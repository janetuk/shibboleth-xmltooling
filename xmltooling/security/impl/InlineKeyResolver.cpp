/*
 *  Copyright 2001-2007 Internet2
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * InlineKeyResolver.cpp
 * 
 * Resolves key information directly from recognized KeyInfo structures.
 */

#include "internal.h"
#include "security/BasicX509Credential.h"
#include "security/KeyInfoCredentialContext.h"
#include "security/KeyInfoResolver.h"
#include "signature/KeyInfo.h"
#include "signature/Signature.h"
#include "util/NDC.h"
#include "util/Threads.h"
#include "util/XMLConstants.h"
#include "validation/ValidatorSuite.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/enc/XSECKeyInfoResolverDefault.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {

    class XMLTOOL_DLLLOCAL InlineCredential : public BasicX509Credential
    {
    public:
        InlineCredential(const KeyInfo* keyInfo=NULL) : BasicX509Credential(keyInfo!=NULL), m_credctx(new KeyInfoCredentialContext(keyInfo)) {
        }
        InlineCredential(DSIGKeyInfoList* keyInfo) : BasicX509Credential(false), m_credctx(new KeyInfoCredentialContext(keyInfo)) {
        }
        InlineCredential(KeyInfoCredentialContext* context) : BasicX509Credential(context->getKeyInfo()!=NULL), m_credctx(NULL) {
        }
        virtual ~InlineCredential() {
            delete m_credctx;
        }

        XSECCryptoKey* getPrivateKey() const {
            return NULL;
        }

        const KeyInfo* getKeyInfo(bool compact=false) const {
            return m_credctx->getKeyInfo();
        }
        
        const CredentialContext* getCredentialContext() const {
            return m_credctx;
        }

        void setCredentialContext(KeyInfoCredentialContext* context) {
            m_credctx = context;
        }

        void resolve(const KeyInfo* keyInfo, int types=0);
        void resolve(DSIGKeyInfoList* keyInfo, int types=0);

    private:
        bool resolveCerts(const KeyInfo* keyInfo);
        bool resolveKey(const KeyInfo* keyInfo);
        bool resolveCRL(const KeyInfo* keyInfo);

        KeyInfoCredentialContext* m_credctx;
    };

    class XMLTOOL_DLLLOCAL InlineKeyResolver : public KeyInfoResolver
    {
    public:
        InlineKeyResolver() {}
        virtual ~InlineKeyResolver() {}

        Credential* resolve(const KeyInfo* keyInfo, int types=0) const {
            if (!keyInfo)
                return NULL;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(keyInfo));
            credential->resolve(keyInfo, types);
            return credential.release();
        }
        Credential* resolve(DSIGKeyInfoList* keyInfo, int types=0) const {
            if (!keyInfo)
                return NULL;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(keyInfo));
            credential->resolve(keyInfo, types);
            return credential.release();
        }
        Credential* resolve(KeyInfoCredentialContext* context, int types=0) const {
            if (!context)
                return NULL;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(context));
            if (context->getKeyInfo())
                credential->resolve(context->getKeyInfo(), types);
            else if (context->getNativeKeyInfo())
                credential->resolve(context->getNativeKeyInfo(), types);
            credential->setCredentialContext(context);
            return credential.release();
        }
    };

    KeyInfoResolver* XMLTOOL_DLLLOCAL InlineKeyInfoResolverFactory(const DOMElement* const & e)
    {
        return new InlineKeyResolver();
    }
};

void InlineCredential::resolve(const KeyInfo* keyInfo, int types)
{
#ifdef _DEBUG
    NDC ndc("resolve");
#endif

    if (types & X509Credential::RESOLVE_CERTS)
        resolveCerts(keyInfo);
    
    if (types & Credential::RESOLVE_KEYS) {
        if (types & X509Credential::RESOLVE_CERTS) {
            // If we have a cert, just use it.
            if (!m_xseccerts.empty())
                m_key = m_xseccerts.front()->clonePublicKey();
        }
        // Otherwise try directly for a key and then go for certs if none is found.
        else if (!resolveKey(keyInfo) && resolveCerts(keyInfo)) {
            m_key = m_xseccerts.front()->clonePublicKey();
        }
    }

    if (types & X509Credential::RESOLVE_CRLS)
        resolveCRL(keyInfo);

    keyInfo->extractNames(m_keyNames);
}

bool InlineCredential::resolveKey(const KeyInfo* keyInfo)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver");

    // Check for ds:KeyValue
    const vector<KeyValue*>& keyValues = keyInfo->getKeyValues();
    for (vector<KeyValue*>::const_iterator i=keyValues.begin(); i!=keyValues.end(); ++i) {
        try {
            SchemaValidators.validate(*i);    // see if it's a "valid" key
            RSAKeyValue* rsakv = (*i)->getRSAKeyValue();
            if (rsakv) {
                log.debug("resolving ds:RSAKeyValue");
                auto_ptr_char mod(rsakv->getModulus()->getValue());
                auto_ptr_char exp(rsakv->getExponent()->getValue());
                auto_ptr<XSECCryptoKeyRSA> rsa(XSECPlatformUtils::g_cryptoProvider->keyRSA());
                rsa->loadPublicModulusBase64BigNums(mod.get(), strlen(mod.get()));
                rsa->loadPublicExponentBase64BigNums(exp.get(), strlen(exp.get()));
                m_key = rsa.release();
                return true;
            }
            DSAKeyValue* dsakv = (*i)->getDSAKeyValue();
            if (dsakv) {
                log.debug("resolving ds:DSAKeyValue");
                auto_ptr<XSECCryptoKeyDSA> dsa(XSECPlatformUtils::g_cryptoProvider->keyDSA());
                auto_ptr_char y(dsakv->getY()->getValue());
                dsa->loadYBase64BigNums(y.get(), strlen(y.get()));
                if (dsakv->getP()) {
                    auto_ptr_char p(dsakv->getP()->getValue());
                    dsa->loadPBase64BigNums(p.get(), strlen(p.get()));
                }
                if (dsakv->getQ()) {
                    auto_ptr_char q(dsakv->getQ()->getValue());
                    dsa->loadQBase64BigNums(q.get(), strlen(q.get()));
                }
                if (dsakv->getG()) {
                    auto_ptr_char g(dsakv->getG()->getValue());
                    dsa->loadGBase64BigNums(g.get(), strlen(g.get()));
                }
                m_key = dsa.release();
                return true;
            }
        }
        catch (ValidationException& ex) {
            log.warn("skipping invalid ds:KeyValue (%s)", ex.what());
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            log.error("caught XML-Security exception loading key: %s", temp.get());
        }
        catch(XSECCryptoException& e) {
            log.error("caught XML-Security exception loading key: %s", e.getMsg());
        }
    }

    // Check for RetrievalMethod.
    const XMLCh* fragID=NULL;
    const XMLObject* treeRoot=NULL;
    const vector<RetrievalMethod*>& methods=keyInfo->getRetrievalMethods();
    for (vector<RetrievalMethod*>::const_iterator m=methods.begin(); m!=methods.end(); ++m) {
        if (!XMLString::equals((*m)->getType(),RetrievalMethod::TYPE_RSAKEYVALUE) &&
            !XMLString::equals((*m)->getType(),RetrievalMethod::TYPE_DSAKEYVALUE))
            continue;
        fragID = (*m)->getURI();
        if (!fragID || *fragID != chPound || !*(fragID+1)) {
            log.warn("skipping ds:RetrievalMethod with an empty or non-local reference");
            continue;
        }
        if (!treeRoot) {
            treeRoot = keyInfo;
            while (treeRoot->getParent())
                treeRoot = treeRoot->getParent();
        }
        keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
        if (!keyInfo) {
            log.warn("skipping ds:RetrievalMethod, local reference did not resolve to a ds:KeyInfo");
            continue;
        }
        if (resolveKey(keyInfo))
            return true;
    }
    return false;
}

bool InlineCredential::resolveCerts(const KeyInfo* keyInfo)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver");

    // Check for ds:X509Data
    const vector<X509Data*>& x509Datas=keyInfo->getX509Datas();
    for (vector<X509Data*>::const_iterator j=x509Datas.begin(); m_xseccerts.empty() && j!=x509Datas.end(); ++j) {
        const vector<X509Certificate*> x509Certs=const_cast<const X509Data*>(*j)->getX509Certificates();
        for (vector<X509Certificate*>::const_iterator k=x509Certs.begin(); k!=x509Certs.end(); ++k) {
            try {
                auto_ptr_char x((*k)->getValue());
                if (!x.get()) {
                    log.warn("skipping empty ds:X509Certificate");
                }
                else {
                    log.debug("resolving ds:X509Certificate");
                    auto_ptr<XSECCryptoX509> x509(XSECPlatformUtils::g_cryptoProvider->X509());
                    x509->loadX509Base64Bin(x.get(), strlen(x.get()));
                    m_xseccerts.push_back(x509.release());
                }
            }
            catch(XSECException& e) {
                auto_ptr_char temp(e.getMsg());
                log.error("caught XML-Security exception loading certificate: %s", temp.get());
            }
            catch(XSECCryptoException& e) {
                log.error("caught XML-Security exception loading certificate: %s", e.getMsg());
            }
        }
    }

    if (m_xseccerts.empty()) {
        // Check for RetrievalMethod.
        const XMLCh* fragID=NULL;
        const XMLObject* treeRoot=NULL;
        const vector<RetrievalMethod*> methods=keyInfo->getRetrievalMethods();
        for (vector<RetrievalMethod*>::const_iterator m=methods.begin(); m!=methods.end(); ++m) {
            if (!XMLString::equals((*m)->getType(),RetrievalMethod::TYPE_X509DATA))
                continue;
            fragID = (*m)->getURI();
            if (!fragID || *fragID != chPound || !*(fragID+1)) {
                log.warn("skipping ds:RetrievalMethod with an empty or non-local reference");
                continue;
            }
            if (!treeRoot) {
                treeRoot = keyInfo;
                while (treeRoot->getParent())
                    treeRoot = treeRoot->getParent();
            }
            keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
            if (!keyInfo) {
                log.warn("skipping ds:RetrievalMethod, local reference did not resolve to a ds:KeyInfo");
                continue;
            }
            if (resolveCerts(keyInfo))
                return true;
        }
        return false;
    }
    
    log.debug("resolved %d certificate(s)", m_xseccerts.size());
    return !m_xseccerts.empty();
}

bool InlineCredential::resolveCRL(const KeyInfo* keyInfo)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver");

    // Check for ds:X509Data
    const vector<X509Data*>& x509Datas=keyInfo->getX509Datas();
    for (vector<X509Data*>::const_iterator j=x509Datas.begin(); j!=x509Datas.end(); ++j) {
        const vector<X509CRL*> x509CRLs=const_cast<const X509Data*>(*j)->getX509CRLs();
        for (vector<X509CRL*>::const_iterator k=x509CRLs.begin(); k!=x509CRLs.end(); ++k) {
            try {
                auto_ptr_char x((*k)->getValue());
                if (!x.get()) {
                    log.warn("skipping empty ds:X509CRL");
                }
                else {
                    log.debug("resolving ds:X509CRL");
                    auto_ptr<XSECCryptoX509CRL> crl(XMLToolingConfig::getConfig().X509CRL());
                    crl->loadX509CRLBase64Bin(x.get(), strlen(x.get()));
                    m_crl = crl.release();
                    return true;
                }
            }
            catch(XSECException& e) {
                auto_ptr_char temp(e.getMsg());
                log.error("caught XML-Security exception loading certificate: %s", temp.get());
            }
            catch(XSECCryptoException& e) {
                log.error("caught XML-Security exception loading certificate: %s", e.getMsg());
            }
        }
    }

    // Check for RetrievalMethod.
    const XMLCh* fragID=NULL;
    const XMLObject* treeRoot=NULL;
    const vector<RetrievalMethod*> methods=keyInfo->getRetrievalMethods();
    for (vector<RetrievalMethod*>::const_iterator m=methods.begin(); m!=methods.end(); ++m) {
        if (!XMLString::equals((*m)->getType(),RetrievalMethod::TYPE_X509DATA))
            continue;
        fragID = (*m)->getURI();
        if (!fragID || *fragID != chPound || !*(fragID+1)) {
            log.warn("skipping ds:RetrievalMethod with an empty or non-local reference");
            continue;
        }
        if (!treeRoot) {
            treeRoot = keyInfo;
            while (treeRoot->getParent())
                treeRoot = treeRoot->getParent();
        }
        keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
        if (!keyInfo) {
            log.warn("skipping ds:RetrievalMethod, local reference did not resolve to a ds:KeyInfo");
            continue;
        }
        if (resolveCRL(keyInfo))
            return true;
    }

    return false;
}

void InlineCredential::resolve(DSIGKeyInfoList* keyInfo, int types)
{
#ifdef _DEBUG
    NDC ndc("resolve");
#endif

    if (types & Credential::RESOLVE_KEYS) {
        // Default resolver handles RSA/DSAKeyValue and X509Certificate elements.
        try {
            XSECKeyInfoResolverDefault def;
            m_key = def.resolveKey(keyInfo);
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading certificate: %s", temp.get());
        }
        catch(XSECCryptoException& e) {
            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading certificate: %s", e.getMsg());
        }
    }

	DSIGKeyInfoList::size_type sz = keyInfo->getSize();

    if (types & X509Credential::RESOLVE_CERTS) {
        for (DSIGKeyInfoList::size_type i=0; i<sz; ++i) {
            if (keyInfo->item(i)->getKeyInfoType()==DSIGKeyInfo::KEYINFO_X509) {
                DSIGKeyInfoX509* x509 = static_cast<DSIGKeyInfoX509*>(keyInfo->item(i));
                int count = x509->getCertificateListSize();
                if (count) {
                    for (int j=0; j<count; ++j)
                        m_xseccerts.push_back(x509->getCertificateCryptoItem(j));
                    break;
                }
            }
        }
    }

    if (types & X509Credential::RESOLVE_CRLS) {
        for (DSIGKeyInfoList::size_type i=0; i<sz; ++i) {
            if (keyInfo->item(i)->getKeyInfoType()==DSIGKeyInfo::KEYINFO_X509) {
                auto_ptr_char buf(static_cast<DSIGKeyInfoX509*>(keyInfo->item(i))->getX509CRL());
                if (buf.get()) {
                    try {
                        auto_ptr<XSECCryptoX509CRL> crlobj(XMLToolingConfig::getConfig().X509CRL());
                        crlobj->loadX509CRLBase64Bin(buf.get(), strlen(buf.get()));
                        m_crl = crlobj.release();
                        break;
                    }
                    catch(XSECException& e) {
                        auto_ptr_char temp(e.getMsg());
                        Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading CRL: %s", temp.get());
                    }
                    catch(XSECCryptoException& e) {
                        Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading CRL: %s", e.getMsg());
                    }
                }
            }
        }
    }

    Signature::extractNames(keyInfo, m_keyNames);
}
