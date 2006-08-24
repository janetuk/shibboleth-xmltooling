/*
 *  Copyright 2001-2005 Internet2
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
#include "signature/CachingKeyResolver.h"
#include "util/NDC.h"
#include "util/Threads.h"

#include <algorithm>
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

namespace xmlsignature {
    class XMLTOOL_DLLLOCAL InlineKeyResolver : public CachingKeyResolver
    {
    public:
        InlineKeyResolver(const DOMElement* e);
        virtual ~InlineKeyResolver();

        XSECCryptoKey* resolveKey(const KeyInfo* keyInfo) const;
        XSECCryptoKey* resolveKey(DSIGKeyInfoList* keyInfo) const;
        vector<XSECCryptoX509*>::size_type resolveCertificates(const KeyInfo* keyInfo, ResolvedCertificates& certs) const;
        vector<XSECCryptoX509*>::size_type resolveCertificates(DSIGKeyInfoList* keyInfo, ResolvedCertificates& certs) const;
        XSECCryptoX509CRL* resolveCRL(const KeyInfo* keyInfo) const;
        XSECCryptoX509CRL* resolveCRL(DSIGKeyInfoList* keyInfo) const;
        
        void clearCache() {
            if (m_lock)
                m_lock->wrlock();
            m_cache.clear();
            if (m_lock)
                m_lock->unlock();
        }
        
    private:
        struct XMLTOOL_DLLLOCAL CacheEntry {
            CacheEntry() : m_key(NULL), m_crl(NULL) {}
            ~CacheEntry() {
                delete m_key;
                for_each(m_certs.begin(),m_certs.end(),xmltooling::cleanup<XSECCryptoX509>());
                delete m_crl;
            }
            XSECCryptoKey* m_key;
            vector<XSECCryptoX509*> m_certs;
            XSECCryptoX509CRL* m_crl;
        };

        void _resolve(const KeyInfo* keyInfo, CacheEntry& entry) const;
        XSECCryptoKey* _resolveKey(const KeyInfo* keyInfo) const;
        vector<XSECCryptoX509*>::size_type _resolveCertificates(const KeyInfo* keyInfo, vector<XSECCryptoX509*>& certs) const;
        XSECCryptoX509CRL* _resolveCRL(const KeyInfo* keyInfo) const;

        RWLock* m_lock;
        mutable map<const KeyInfo*,CacheEntry> m_cache;
    };

    KeyResolver* XMLTOOL_DLLLOCAL InlineKeyResolverFactory(const DOMElement* const & e)
    {
        return new InlineKeyResolver(e);
    }
};

static const XMLCh cache[] = UNICODE_LITERAL_5(c,a,c,h,e);

InlineKeyResolver::InlineKeyResolver(const DOMElement* e) : m_lock(NULL)
{
    const XMLCh* flag = e ? e->getAttributeNS(NULL,cache) : NULL;
    if (flag && XMLString::equals(flag,XMLConstants::XML_TRUE) || XMLString::equals(flag,XMLConstants::XML_ONE))
        m_lock=RWLock::create();
}

InlineKeyResolver::~InlineKeyResolver()
{
    clearCache();
    delete m_lock;
}

void InlineKeyResolver::_resolve(const KeyInfo* keyInfo, CacheEntry& entry) const
{
    if (_resolveCertificates(keyInfo, entry.m_certs)>0)
        entry.m_key = entry.m_certs.front()->clonePublicKey();
    else
        entry.m_key = _resolveKey(keyInfo);
    entry.m_crl = _resolveCRL(keyInfo);
}

XSECCryptoKey* InlineKeyResolver::_resolveKey(const KeyInfo* keyInfo) const
{
#ifdef _DEBUG
    NDC ndc("_resolveKey");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver");

    if (!keyInfo)
        return NULL;

    // Check for ds:X509Data
    const vector<X509Data*>& x509Datas=keyInfo->getX509Datas();
    for (vector<X509Data*>::const_iterator j=x509Datas.begin(); j!=x509Datas.end(); ++j) {
        try {
            const vector<X509Certificate*> x509Certs=const_cast<const X509Data*>(*j)->getX509Certificates();
            if (!x509Certs.empty()) {
                auto_ptr_char x(x509Certs.front()->getValue());
                if (!x.get()) {
                    log.warn("skipping empty ds:X509Certificate");
                }
                else {
                    log.debug("resolving ds:X509Certificate");
                    auto_ptr<XSECCryptoX509> x509(XSECPlatformUtils::g_cryptoProvider->X509());
                    x509->loadX509Base64Bin(x.get(), strlen(x.get()));
                    return x509->clonePublicKey();
                }
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

    // Check for ds:KeyValue
    const vector<KeyValue*>& keyValues = keyInfo->getKeyValues();
    for (vector<KeyValue*>::const_iterator i=keyValues.begin(); i!=keyValues.end(); ++i) {
        try {
            KeyInfoSchemaValidators.validate(*i);    // see if it's a "valid" key
            RSAKeyValue* rsakv = (*i)->getRSAKeyValue();
            if (rsakv) {
                log.debug("resolving ds:RSAKeyValue");
                auto_ptr_char mod(rsakv->getModulus()->getValue());
                auto_ptr_char exp(rsakv->getExponent()->getValue());
                auto_ptr<XSECCryptoKeyRSA> rsa(XSECPlatformUtils::g_cryptoProvider->keyRSA());
                rsa->loadPublicModulusBase64BigNums(mod.get(), strlen(mod.get()));
                rsa->loadPublicExponentBase64BigNums(exp.get(), strlen(exp.get()));
                return rsa.release();
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
                return dsa.release();
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

    log.warn("unable to resolve key");
    return NULL;
}

vector<XSECCryptoX509*>::size_type InlineKeyResolver::_resolveCertificates(
    const KeyInfo* keyInfo, vector<XSECCryptoX509*>& certs
    ) const
{
#ifdef _DEBUG
    NDC ndc("_resolveCertificates");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver");

    if (!keyInfo)
        return 0;

    // Check for ds:X509Data
    const vector<X509Data*>& x509Datas=keyInfo->getX509Datas();
    for (vector<X509Data*>::const_iterator j=x509Datas.begin(); certs.empty() && j!=x509Datas.end(); ++j) {
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
                    certs.push_back(x509.release());
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
    if (log.isDebugEnabled()) {
        log.debug("resolved %d certificate%s", certs.size(), certs.size()==1 ? "" : "s");
    }
    return certs.size();
}

XSECCryptoX509CRL* InlineKeyResolver::_resolveCRL(const KeyInfo* keyInfo) const
{
#ifdef _DEBUG
    NDC ndc("_resolveCRL");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver");

    if (!keyInfo)
        return NULL;

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
                    return crl.release();
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
    return NULL;
}

XSECCryptoKey* InlineKeyResolver::resolveKey(const KeyInfo* keyInfo) const
{
    // Caching?
    if (m_lock) {
        // Get read lock.
        m_lock->rdlock();
        map<const KeyInfo*,CacheEntry>::iterator i=m_cache.find(keyInfo);
        if (i != m_cache.end()) {
            // Found in cache, so just return the results.
            SharedLock locker(m_lock,false);
            return i->second.m_key ? i->second.m_key->clone() : NULL;
        }
        else {
            // Elevate lock.
            m_lock->unlock();
            m_lock->wrlock();
            SharedLock locker(m_lock,false);
            // Recheck cache.
            i=m_cache.find(keyInfo);
            if (i == m_cache.end()) {
                i = m_cache.insert(make_pair(keyInfo,CacheEntry())).first;
                _resolve(i->first, i->second);
            }
            return i->second.m_key ? i->second.m_key->clone() : NULL;
        }
    }
    return _resolveKey(keyInfo);
}

XSECCryptoX509CRL* InlineKeyResolver::resolveCRL(const KeyInfo* keyInfo) const
{
    // Caching?
    if (m_lock) {
        // Get read lock.
        m_lock->rdlock();
        map<const KeyInfo*,CacheEntry>::iterator i=m_cache.find(keyInfo);
        if (i != m_cache.end()) {
            // Found in cache, so just return the results.
            SharedLock locker(m_lock,false);
            return i->second.m_crl ? i->second.m_crl->clone() : NULL;
        }
        else {
            // Elevate lock.
            m_lock->unlock();
            m_lock->wrlock();
            SharedLock locker(m_lock,false);
            // Recheck cache.
            i=m_cache.find(keyInfo);
            if (i == m_cache.end()) {
                i = m_cache.insert(make_pair(keyInfo,CacheEntry())).first;
                _resolve(i->first, i->second);
            }
            return i->second.m_crl ? i->second.m_crl->clone() : NULL;
        }
    }
    return _resolveCRL(keyInfo);
}

vector<XSECCryptoX509*>::size_type InlineKeyResolver::resolveCertificates(
    const KeyInfo* keyInfo, ResolvedCertificates& certs
    ) const
{
    // Caching?
    if (m_lock) {
        // Get read lock.
        m_lock->rdlock();
        map<const KeyInfo*,CacheEntry>::iterator i=m_cache.find(keyInfo);
        if (i != m_cache.end()) {
            // Found in cache, so just return the results.
            SharedLock locker(m_lock,false);
            accessCertificates(certs).assign(i->second.m_certs.begin(), i->second.m_certs.end());
            accessOwned(certs) = false;
            return accessCertificates(certs).size();
        }
        else {
            // Elevate lock.
            m_lock->unlock();
            m_lock->wrlock();
            SharedLock locker(m_lock,false);
            // Recheck cache.
            i=m_cache.find(keyInfo);
            if (i == m_cache.end()) {
                i = m_cache.insert(make_pair(keyInfo,CacheEntry())).first;
                _resolve(i->first, i->second);
            }
            accessCertificates(certs).assign(i->second.m_certs.begin(), i->second.m_certs.end());
            accessOwned(certs) = false;
            return accessCertificates(certs).size();
        }
    }
    accessOwned(certs) = true;
    return _resolveCertificates(keyInfo, accessCertificates(certs));
}

XSECCryptoKey* InlineKeyResolver::resolveKey(DSIGKeyInfoList* keyInfo) const
{
#ifdef _DEBUG
    NDC ndc("resolveKey");
#endif

    if (!keyInfo)
        return NULL;

    // Default resolver handles RSA/DSAKeyValue and X509Certificate elements.
    try {
        XSECKeyInfoResolverDefault def;
        return def.resolveKey(keyInfo);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading certificate: %s", temp.get());
    }
    catch(XSECCryptoException& e) {
        Category::getInstance(XMLTOOLING_LOGCAT".KeyResolver").error("caught XML-Security exception loading certificate: %s", e.getMsg());
    }
    return NULL;
}

vector<XSECCryptoX509*>::size_type InlineKeyResolver::resolveCertificates(
    DSIGKeyInfoList* keyInfo, ResolvedCertificates& certs
    ) const
{
    accessCertificates(certs).clear();
    accessOwned(certs) = false;

    if (!keyInfo)
        return 0;

	DSIGKeyInfoList::size_type sz = keyInfo->getSize();
    for (DSIGKeyInfoList::size_type i=0; accessCertificates(certs).empty() && i<sz; ++i) {
        if (keyInfo->item(i)->getKeyInfoType()==DSIGKeyInfo::KEYINFO_X509) {
            DSIGKeyInfoX509* x509 = static_cast<DSIGKeyInfoX509*>(keyInfo->item(i));
            int count = x509->getCertificateListSize();
            for (int j=0; j<count; ++j) {
                accessCertificates(certs).push_back(x509->getCertificateCryptoItem(j));
            }
        }
    }
    return accessCertificates(certs).size();
}

XSECCryptoX509CRL* InlineKeyResolver::resolveCRL(DSIGKeyInfoList* keyInfo) const
{
#ifdef _DEBUG
    NDC ndc("resolveCRL");
#endif

    if (!keyInfo)
        return NULL;

    DSIGKeyInfoList::size_type sz = keyInfo->getSize();
    for (DSIGKeyInfoList::size_type i=0; i<sz; ++i) {
        if (keyInfo->item(i)->getKeyInfoType()==DSIGKeyInfo::KEYINFO_X509) {
            auto_ptr_char buf(static_cast<DSIGKeyInfoX509*>(keyInfo->item(i))->getX509CRL());
            if (buf.get()) {
                try {
                    auto_ptr<XSECCryptoX509CRL> crlobj(XMLToolingConfig::getConfig().X509CRL());
                    crlobj->loadX509CRLBase64Bin(buf.get(), strlen(buf.get()));
                    return crlobj.release();
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
    return NULL;
}
