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
 * ExplicitKeyTrustEngine.cpp
 * 
 * TrustEngine based on explicit knowledge of peer key information.
 */

#include "internal.h"
#include "security/OpenSSLTrustEngine.h"
#include "signature/SignatureValidator.h"
#include "util/NDC.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {
    class XMLTOOL_DLLLOCAL ExplicitKeyTrustEngine : public OpenSSLTrustEngine
    {
    public:
        ExplicitKeyTrustEngine(const DOMElement* e) : OpenSSLTrustEngine(e) {}
        virtual ~ExplicitKeyTrustEngine() {}

        virtual bool validate(
            Signature& sig,
            const KeyInfoSource& keyInfoSource,
            const KeyResolver* keyResolver=NULL
            ) const;
        virtual bool validate(
            const XMLCh* sigAlgorithm,
            const char* sig,
            KeyInfo* keyInfo,
            const char* in,
            unsigned int in_len,
            const KeyInfoSource& keyInfoSource,
            const KeyResolver* keyResolver=NULL
            ) const;
        virtual bool validate(
            XSECCryptoX509* certEE,
            const vector<XSECCryptoX509*>& certChain,
            const KeyInfoSource& keyInfoSource,
            bool checkName=true,
            const KeyResolver* keyResolver=NULL
            ) const;
        virtual bool validate(
            X509* certEE,
            STACK_OF(X509)* certChain,
            const KeyInfoSource& keyInfoSource,
            bool checkName=true,
            const KeyResolver* keyResolver=NULL
            ) const;
    };

    TrustEngine* XMLTOOL_DLLLOCAL ExplicitKeyTrustEngineFactory(const DOMElement* const & e)
    {
        return new ExplicitKeyTrustEngine(e);
    }
};

bool ExplicitKeyTrustEngine::validate(
    Signature& sig,
    const KeyInfoSource& keyInfoSource,
    const KeyResolver* keyResolver
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
    auto_ptr<KeyInfoIterator> keyInfoIter(keyInfoSource.getKeyInfoIterator());
    if (!keyInfoIter->hasNext()) {
        log.warn("unable to validate signature, no key information available for peer");
        return false;
    }
    
    log.debug("attempting to validate signature with the key information for peer");
    SignatureValidator sigValidator;
    while (keyInfoIter->hasNext()) {
        XSECCryptoKey* key = (keyResolver ? keyResolver : m_keyResolver)->resolveKey(keyInfoIter->next());
        if (key) {
            log.debug("attempting to validate signature with public key...");
            try {
                sigValidator.setKey(key);   // key now owned by validator
                sigValidator.validate(&sig);
                log.info("signature validated with public key");
                return true;
            }
            catch (ValidationException& e) {
                if (log.isDebugEnabled()) {
                    log.debug("public key did not validate signature: %s", e.what());
                }
            }
        }
        else {
            log.debug("key information does not resolve to a public key, skipping it");
        }
    }

    log.error("no peer key information validated the signature");
    return false;
}

bool ExplicitKeyTrustEngine::validate(
    const XMLCh* sigAlgorithm,
    const char* sig,
    KeyInfo* keyInfo,
    const char* in,
    unsigned int in_len,
    const KeyInfoSource& keyInfoSource,
    const KeyResolver* keyResolver
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
    auto_ptr<KeyInfoIterator> keyInfoIter(keyInfoSource.getKeyInfoIterator());
    if (!keyInfoIter->hasNext()) {
        log.warn("unable to validate signature, no key information available for peer");
        return false;
    }
    
    log.debug("attempting to validate signature with the key information for peer");
    while (keyInfoIter->hasNext()) {
        auto_ptr<XSECCryptoKey> key((keyResolver ? keyResolver : m_keyResolver)->resolveKey(keyInfoIter->next()));
        if (key.get()) {
            log.debug("attempting to validate signature with public key...");
            try {
                if (Signature::verifyRawSignature(key.get(), sigAlgorithm, sig, in, in_len)) {
                    log.info("signature validated with public key");
                    return true;
                }
            }
            catch (SignatureException& e) {
                if (log.isDebugEnabled()) {
                    log.debug("public key did not validate signature: %s", e.what());
                }
            }
        }
        else {
            log.debug("key information does not resolve to a public key, skipping it");
        }
    }

    log.error("no peer key information validated the signature");
    return false;
}

bool ExplicitKeyTrustEngine::validate(
    XSECCryptoX509* certEE,
    const vector<XSECCryptoX509*>& certChain,
    const KeyInfoSource& keyInfoSource,
    bool checkName,
    const KeyResolver* keyResolver
    ) const
{
    if (!certEE) {
#ifdef _DEBUG
        NDC ndc("validate");
#endif
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine").error("unable to validate, end-entity certificate was null");
        return false;
    }
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
#ifdef _DEBUG
        NDC ndc("validate");
#endif
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine").error("only the OpenSSL XSEC provider is supported");
        return false;
    }

    return validate(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(), NULL, keyInfoSource, checkName, keyResolver);
}

bool ExplicitKeyTrustEngine::validate(
    X509* certEE,
    STACK_OF(X509)* certChain,
    const KeyInfoSource& keyInfoSource,
    bool checkName,
    const KeyResolver* keyResolver
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
    if (!certEE) {
        log.error("unable to validate, end-entity certificate was null");
        return false;
    }

    auto_ptr<KeyInfoIterator> keyInfoIter(keyInfoSource.getKeyInfoIterator());
    if (!keyInfoIter->hasNext()) {
        log.warn("unable to validate, no key information available for peer");
        return false;
    }

    // The "explicit" trust implementation relies solely on keys living within the
    // peer interface to verify the EE certificate.

    log.debug("attempting to match key information from peer with end-entity certificate");
    while (keyInfoIter->hasNext()) {
        auto_ptr<XSECCryptoKey> key((keyResolver ? keyResolver : m_keyResolver)->resolveKey(keyInfoIter->next()));
        if (key.get()) {
            log.debug("checking if peer key matches end-entity certificate");
            if (key->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
                log.error("only the OpenSSL XSEC provider is supported");
                continue;
            }
            switch (key->getKeyType()) {
                case XSECCryptoKey::KEY_RSA_PUBLIC:
                {
                    RSA* rsa = static_cast<OpenSSLCryptoKeyRSA*>(key.get())->getOpenSSLRSA();
                    EVP_PKEY* evp = certEE->cert_info->key->pkey;
                    if (rsa && evp && evp->type == EVP_PKEY_RSA &&
                            BN_cmp(rsa->n,evp->pkey.rsa->n) == 0 && BN_cmp(rsa->e,evp->pkey.rsa->e) != 0) {
                        log.info("end-entity certificate matches peer RSA key information");
                        return true;
                    }
                    break;
                }
                
                case XSECCryptoKey::KEY_DSA_PUBLIC:
                {
                    DSA* dsa = static_cast<OpenSSLCryptoKeyDSA*>(key.get())->getOpenSSLDSA();
                    EVP_PKEY* evp = certEE->cert_info->key->pkey;
                    if (dsa && evp && evp->type == EVP_PKEY_DSA && BN_cmp(dsa->pub_key,evp->pkey.dsa->pub_key) == 0) {
                        log.info("end-entity certificate matches peer DSA key information");
                        return true;
                    }
                    break;
                }

                default:
                    log.warn("unknown peer key type, skipping...");
            }
        }
        else {
            log.debug("key information does not resolve to a public key, skipping it");
        }
    }

    log.debug("no keys within this peer's key information matched the given end-entity certificate");
    return false;
}
