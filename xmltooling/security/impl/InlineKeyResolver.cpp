/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * InlineKeyResolver.cpp
 * 
 * Resolves key information directly from recognized KeyInfo structures.
 */

#include "internal.h"
#include "logging.h"
#include "security/BasicX509Credential.h"
#include "security/KeyInfoCredentialContext.h"
#include "security/KeyInfoResolver.h"
#include "security/SecurityHelper.h"
#include "security/XSECCryptoX509CRL.h"
#include "signature/KeyInfo.h"
#include "signature/Signature.h"
#include "util/NDC.h"
#include "util/Threads.h"
#include "util/XMLConstants.h"
#include "validation/ValidatorSuite.h"

#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/enc/XSECKeyInfoResolverDefault.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>
#include <xsec/enc/XSECCryptoKeyRSA.hpp>
#include <xsec/enc/XSECCryptoKeyDSA.hpp>
#ifdef XMLTOOLING_XMLSEC_ECC
# include <xsec/enc/XSECCryptoKeyEC.hpp>
#endif
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmlsignature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

namespace xmltooling {

    class XMLTOOL_DLLLOCAL InlineCredential : public BasicX509Credential
    {
    public:
        InlineCredential(const KeyInfo* keyInfo=nullptr) : BasicX509Credential(keyInfo!=nullptr), m_credctx(new KeyInfoCredentialContext(keyInfo)) {
        }
        InlineCredential(DSIGKeyInfoList* keyInfo) : BasicX509Credential(false), m_credctx(new KeyInfoCredentialContext(keyInfo)) {
        }
        InlineCredential(KeyInfoCredentialContext* context) : BasicX509Credential(context->getKeyInfo()!=nullptr), m_credctx(nullptr) {
        }
        virtual ~InlineCredential() {
            delete m_credctx;
        }

        XSECCryptoKey* getPrivateKey() const {
            return nullptr;
        }

        KeyInfo* getKeyInfo(bool compact=false) const {
            KeyInfo* ret = m_credctx->getKeyInfo() ? m_credctx->getKeyInfo()->cloneKeyInfo() : nullptr;
            if (ret) {
                ret->setId(nullptr);
                ret->getRetrievalMethods().clear();
                ret->getKeyInfoReferences().clear();
                if (compact) {
                    ret->getKeyValues().clear();
                    ret->getDEREncodedKeyValues().clear();
                    ret->getSPKIDatas().clear();
                    ret->getPGPDatas().clear();
                    ret->getUnknownXMLObjects().clear();
                    VectorOf(X509Data) x509Datas=ret->getX509Datas();
                    for (VectorOf(X509Data)::size_type pos = 0; pos < x509Datas.size();) {
                        x509Datas[pos]->getX509Certificates().clear();
                        x509Datas[pos]->getX509CRLs().clear();
                        x509Datas[pos]->getOCSPResponses().clear();
                        x509Datas[pos]->getUnknownXMLObjects().clear();
                        if (x509Datas[pos]->hasChildren())
                            ++pos;
                        else
                            x509Datas.erase(x509Datas.begin() + pos);
                    }
                }
            }
            if (!ret->hasChildren()) {
                delete ret;
                ret = nullptr;
            }
            return ret;
        }
        
        const CredentialContext* getCredentalContext() const {
            return m_credctx;
        }

        void setCredentialContext(KeyInfoCredentialContext* context) {
            m_credctx = context;
        }

        void resolve(const KeyInfo* keyInfo, int types=0, bool followRefs=false);
        void resolve(DSIGKeyInfoList* keyInfo, int types=0, bool followRefs=false);

    private:
        bool resolveCerts(const KeyInfo* keyInfo, bool followRefs=false);
        bool resolveKey(const KeyInfo* keyInfo, bool followRefs=false);
        bool resolveCRLs(const KeyInfo* keyInfo, bool followRefs=false);

        KeyInfoCredentialContext* m_credctx;
    };

    static const XMLCh keyInfoReferences[] = UNICODE_LITERAL_17(k,e,y,I,n,f,o,R,e,f,e,r,e,n,c,e,s);

    class XMLTOOL_DLLLOCAL InlineKeyResolver : public KeyInfoResolver
    {
    public:
        InlineKeyResolver(const DOMElement* e)
            : m_followRefs(XMLHelper::getAttrBool(e, false, keyInfoReferences)) {
        }

        virtual ~InlineKeyResolver() {}

        Credential* resolve(const KeyInfo* keyInfo, int types=0) const {
            if (!keyInfo)
                return nullptr;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(keyInfo));
            credential->resolve(keyInfo, types, m_followRefs);
            return credential.release();
        }
        Credential* resolve(DSIGKeyInfoList* keyInfo, int types=0) const {
            if (!keyInfo)
                return nullptr;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(keyInfo));
            credential->resolve(keyInfo, types, m_followRefs);
            return credential.release();
        }
        Credential* resolve(KeyInfoCredentialContext* context, int types=0) const {
            if (!context)
                return nullptr;
            if (types == 0)
                types = Credential::RESOLVE_KEYS|X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS;
            auto_ptr<InlineCredential> credential(new InlineCredential(context));
            if (context->getKeyInfo())
                credential->resolve(context->getKeyInfo(), types, m_followRefs);
            else if (context->getNativeKeyInfo())
                credential->resolve(context->getNativeKeyInfo(), types, m_followRefs);
            credential->setCredentialContext(context);
            return credential.release();
        }

    private:
        bool m_followRefs;
    };

    KeyInfoResolver* XMLTOOL_DLLLOCAL InlineKeyInfoResolverFactory(const DOMElement* const & e)
    {
        return new InlineKeyResolver(e);
    }
};

void InlineCredential::resolve(const KeyInfo* keyInfo, int types, bool followRefs)
{
#ifdef _DEBUG
    NDC ndc("resolve");
#endif

    if (types & X509Credential::RESOLVE_CERTS)
        resolveCerts(keyInfo, followRefs);
    
    if (types & Credential::RESOLVE_KEYS) {
        if (types & X509Credential::RESOLVE_CERTS) {
            // If we have a cert, just use it.
            if (!m_xseccerts.empty())
                m_key = m_xseccerts.front()->clonePublicKey();
            else
                resolveKey(keyInfo, followRefs);
        }
        // Otherwise try directly for a key and then go for certs if none is found.
        else if (!resolveKey(keyInfo, followRefs) && resolveCerts(keyInfo, followRefs)) {
            m_key = m_xseccerts.front()->clonePublicKey();
        }
    }

    if (types & X509Credential::RESOLVE_CRLS)
        resolveCRLs(keyInfo, followRefs);

    const XMLCh* n;
    char* kn;
    const vector<KeyName*>& knames=keyInfo->getKeyNames();
    for (vector<KeyName*>::const_iterator kn_i=knames.begin(); kn_i!=knames.end(); ++kn_i) {
        n=(*kn_i)->getName();
        if (n && *n) {
            kn=toUTF8(n);
            m_keyNames.insert(kn);
            delete[] kn;
        }
    }
    const vector<X509Data*> datas=keyInfo->getX509Datas();
    for (vector<X509Data*>::const_iterator x_i=datas.begin(); x_i!=datas.end(); ++x_i) {
        const vector<X509SubjectName*> snames = const_cast<const X509Data*>(*x_i)->getX509SubjectNames();
        for (vector<X509SubjectName*>::const_iterator sn_i = snames.begin(); sn_i!=snames.end(); ++sn_i) {
            n = (*sn_i)->getName();
            if (n && *n) {
                kn=toUTF8(n);
                m_keyNames.insert(kn);
                m_subjectName = kn;
                delete[] kn;
            }
        }

        const vector<X509IssuerSerial*> inames = const_cast<const X509Data*>(*x_i)->getX509IssuerSerials();
        if (!inames.empty()) {
            const X509IssuerName* iname = inames.front()->getX509IssuerName();
            if (iname) {
                kn = toUTF8(iname->getName());
                if (kn)
                    m_issuerName = kn;
                delete[] kn;
            }

            const X509SerialNumber* ser = inames.front()->getX509SerialNumber();
            if (ser) {
                auto_ptr_char sn(ser->getSerialNumber());
                m_serial = sn.get();
            }
        }
    }
}

bool InlineCredential::resolveKey(const KeyInfo* keyInfo, bool followRefs)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver."INLINE_KEYINFO_RESOLVER);

    // Check for ds:KeyValue
    const vector<KeyValue*>& keyValues = keyInfo->getKeyValues();
    for (vector<KeyValue*>::const_iterator i = keyValues.begin(); i != keyValues.end(); ++i) {
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
#ifdef XMLTOOLING_XMLSEC_ECC
            ECKeyValue* eckv = (*i)->getECKeyValue();
            if (eckv && eckv->getNamedCurve() && eckv->getPublicKey()) {
                log.warn("resolving ds11:ECKeyValue");
                auto_ptr<XSECCryptoKeyEC> ec(XSECPlatformUtils::g_cryptoProvider->keyEC());
                auto_ptr_char uri(eckv->getNamedCurve()->getURI());
                auto_ptr_char val(eckv->getPublicKey()->getValue());
                if (uri.get() && val.get()) {
                    ec->loadPublicKeyBase64(uri.get(), val.get(), XMLString::stringLen(val.get()));
                    m_key = ec.release();
                    return true;
                }
            }
#endif
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

    // Check for ds11:DEREncodedKeyValue
    const vector<DEREncodedKeyValue*>& derValues = keyInfo->getDEREncodedKeyValues();
    for (vector<DEREncodedKeyValue*>::const_iterator j = derValues.begin(); j != derValues.end(); ++j) {
        log.debug("resolving ds11:DEREncodedKeyValue");
        m_key = SecurityHelper::fromDEREncoding((*j)->getValue());
        if (m_key)
            return true;
        log.warn("failed to resolve ds11:DEREncodedKeyValue");
    }


    if (followRefs) {
        // Check for KeyInfoReference.
        const XMLCh* fragID=nullptr;
        const XMLObject* treeRoot=nullptr;
        const vector<KeyInfoReference*>& refs = keyInfo->getKeyInfoReferences();
        for (vector<KeyInfoReference*>::const_iterator ref = refs.begin(); ref != refs.end(); ++ref) {
            fragID = (*ref)->getURI();
            if (!fragID || *fragID != chPound || !*(fragID+1)) {
                log.warn("skipping ds11:KeyInfoReference with an empty or non-local reference");
                continue;
            }
            if (!treeRoot) {
                treeRoot = keyInfo;
                while (treeRoot->getParent())
                    treeRoot = treeRoot->getParent();
            }
            keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
            if (!keyInfo) {
                log.warn("skipping ds11:KeyInfoReference, local reference did not resolve to a ds:KeyInfo");
                continue;
            }
            if (resolveKey(keyInfo, false))
                return true;
        }
    }

    return false;
}

bool InlineCredential::resolveCerts(const KeyInfo* keyInfo, bool followRefs)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver."INLINE_KEYINFO_RESOLVER);

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

    if (followRefs && m_xseccerts.empty()) {
        // Check for KeyInfoReference.
        const XMLCh* fragID=NULL;
        const XMLObject* treeRoot=NULL;
        const vector<KeyInfoReference*>& refs = keyInfo->getKeyInfoReferences();
        for (vector<KeyInfoReference*>::const_iterator ref = refs.begin(); ref != refs.end(); ++ref) {
            fragID = (*ref)->getURI();
            if (!fragID || *fragID != chPound || !*(fragID+1)) {
                log.warn("skipping ds11:KeyInfoReference with an empty or non-local reference");
                continue;
            }
            if (!treeRoot) {
                treeRoot = keyInfo;
                while (treeRoot->getParent())
                    treeRoot = treeRoot->getParent();
            }
            keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
            if (!keyInfo) {
                log.warn("skipping ds11:KeyInfoReference, local reference did not resolve to a ds:KeyInfo");
                continue;
            }
            if (resolveCerts(keyInfo, false))
                return true;
        }
        return false;
    }
    
    log.debug("resolved %d certificate(s)", m_xseccerts.size());
    return !m_xseccerts.empty();
}

bool InlineCredential::resolveCRLs(const KeyInfo* keyInfo, bool followRefs)
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".KeyInfoResolver."INLINE_KEYINFO_RESOLVER);

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
                    m_crls.push_back(crl.release());
                }
            }
            catch(XSECException& e) {
                auto_ptr_char temp(e.getMsg());
                log.error("caught XML-Security exception loading CRL: %s", temp.get());
            }
            catch(XSECCryptoException& e) {
                log.error("caught XML-Security exception loading CRL: %s", e.getMsg());
            }
        }
    }

    if (followRefs && m_crls.empty()) {
        // Check for KeyInfoReference.
        const XMLCh* fragID=NULL;
        const XMLObject* treeRoot=NULL;
        const vector<KeyInfoReference*>& refs = keyInfo->getKeyInfoReferences();
        for (vector<KeyInfoReference*>::const_iterator ref = refs.begin(); ref != refs.end(); ++ref) {
            fragID = (*ref)->getURI();
            if (!fragID || *fragID != chPound || !*(fragID+1)) {
                log.warn("skipping ds11:KeyInfoReference with an empty or non-local reference");
                continue;
            }
            if (!treeRoot) {
                treeRoot = keyInfo;
                while (treeRoot->getParent())
                    treeRoot = treeRoot->getParent();
            }
            keyInfo = dynamic_cast<const KeyInfo*>(XMLHelper::getXMLObjectById(*treeRoot, fragID+1));
            if (!keyInfo) {
                log.warn("skipping ds11:KeyInfoReference, local reference did not resolve to a ds:KeyInfo");
                continue;
            }
            if (resolveCRLs(keyInfo, false))
                return true;
        }
        return false;
    }

    log.debug("resolved %d CRL(s)", m_crls.size());
    return !m_crls.empty();
}

void InlineCredential::resolve(DSIGKeyInfoList* keyInfo, int types, bool followRefs)
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
            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading certificate: %s", temp.get());
        }
        catch(XSECCryptoException& e) {
            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading certificate: %s", e.getMsg());
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
#ifdef XMLTOOLING_XMLSEC_MULTIPLECRL
                DSIGKeyInfoX509* x509 = static_cast<DSIGKeyInfoX509*>(keyInfo->item(i));
                int count = x509->getX509CRLListSize();
                for (int j=0; j<count; ++j) {
                    auto_ptr_char buf(x509->getX509CRLItem(j));
                    if (buf.get()) {
                        try {
                            auto_ptr<XSECCryptoX509CRL> crlobj(XMLToolingConfig::getConfig().X509CRL());
                            crlobj->loadX509CRLBase64Bin(buf.get(), strlen(buf.get()));
                            m_crls.push_back(crlobj.release());
                        }
                        catch(XSECException& e) {
                            auto_ptr_char temp(e.getMsg());
                            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading CRL: %s", temp.get());
                        }
                        catch(XSECCryptoException& e) {
                            Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading CRL: %s", e.getMsg());
                        }
                    }
                }
#else
                // The current xmlsec API is limited to one CRL per KeyInfo.
                // For now, I'm going to process the DOM directly.
                DOMNode* x509Node = keyInfo->item(i)->getKeyInfoDOMNode();
                DOMElement* crlElement = x509Node ? XMLHelper::getFirstChildElement(x509Node, xmlconstants::XMLSIG_NS, X509CRL::LOCAL_NAME) : nullptr;
                while (crlElement) {
                    if (crlElement->hasChildNodes()) {
                        auto_ptr_char buf(crlElement->getFirstChild()->getNodeValue());
                        if (buf.get()) {
                            try {
                                auto_ptr<XSECCryptoX509CRL> crlobj(XMLToolingConfig::getConfig().X509CRL());
                                crlobj->loadX509CRLBase64Bin(buf.get(), strlen(buf.get()));
                                m_crls.push_back(crlobj.release());
                            }
                            catch(XSECException& e) {
                                auto_ptr_char temp(e.getMsg());
                                Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading CRL: %s", temp.get());
                            }
                            catch(XSECCryptoException& e) {
                                Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver."INLINE_KEYINFO_RESOLVER).error("caught XML-Security exception loading CRL: %s", e.getMsg());
                            }
                        }
                    }
                    crlElement = XMLHelper::getNextSiblingElement(crlElement, xmlconstants::XMLSIG_NS, X509CRL::LOCAL_NAME);
                }
#endif
            }
        }
    }

    char* kn;
    const XMLCh* n;

    for (size_t s=0; s<keyInfo->getSize(); s++) {
        DSIGKeyInfo* dki = keyInfo->item(s);
        n=dki->getKeyName();
        if (n && *n) {
            kn=toUTF8(n);
            m_keyNames.insert(kn);
            if (dki->getKeyInfoType() == DSIGKeyInfo::KEYINFO_X509)
                m_subjectName = kn;
            delete[] kn;
        }

        if (dki->getKeyInfoType() == DSIGKeyInfo::KEYINFO_X509) {
            DSIGKeyInfoX509* kix = static_cast<DSIGKeyInfoX509*>(dki);
            n = kix->getX509IssuerName();
            if (n && *n) {
                kn=toUTF8(n);
                m_issuerName = kn;
                delete[] kn;
            }
            n = kix->getX509IssuerSerialNumber();
            if (n && *n) {
                auto_ptr_char sn(n);
                m_serial = sn.get();
            }
        }
    }
}
