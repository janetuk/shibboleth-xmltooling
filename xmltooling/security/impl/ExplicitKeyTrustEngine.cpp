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
 * ExplicitKeyTrustEngine.cpp
 * 
 * TrustEngine based on explicit knowledge of peer key information.
 */

#include "internal.h"
#include "security/X509TrustEngine.h"
#include "signature/SignatureValidator.h"
#include "util/NDC.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {
    class XMLTOOL_DLLLOCAL ExplicitKeyTrustEngine : public X509TrustEngine
    {
    public:
        ExplicitKeyTrustEngine(const DOMElement* e) : X509TrustEngine(e) {}
        virtual ~ExplicitKeyTrustEngine() {}

        virtual bool validate(
            Signature& sig,
            TrustEngine::KeyInfoIterator& keyInfoSource,
            const KeyResolver* keyResolver=NULL
            ) const;
        virtual bool validate(
            XSECCryptoX509* certEE,
            const vector<XSECCryptoX509*>& certChain,
            TrustEngine::KeyInfoIterator& keyInfoSource,
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
    TrustEngine::KeyInfoIterator& keyInfoSource,
    const KeyResolver* keyResolver
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
    if (!keyInfoSource.hasNext()) {
        log.warn("unable to validate signature, no key information available for peer");
        return false;
    }
    
    log.debug("attempting to validate signature with the key information for peer");
    SignatureValidator sigValidator;
    while (keyInfoSource.hasNext()) {
        XSECCryptoKey* key = (keyResolver ? keyResolver : m_keyResolver)->resolveKey(keyInfoSource.next());
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
    XSECCryptoX509* certEE,
    const vector<XSECCryptoX509*>& certChain,
    TrustEngine::KeyInfoIterator& keyInfoSource,
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
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        log.error("only the OpenSSL XSEC provider is supported");
        return false;
    }
    else if (!keyInfoSource.hasNext()) {
        log.warn("unable to validate, no key information available for peer");
        return false;
    }

    // The new "basic" trust implementation relies solely on certificates living within the
    // role interface to verify the EE certificate.

    log.debug("attempting to match key information from peer with end-entity certificate");
    while (keyInfoSource.hasNext()) {
        KeyResolver::ResolvedCertificates resolvedCerts;
        if (0 == (keyResolver ? keyResolver : m_keyResolver)->resolveCertificates(keyInfoSource.next(),resolvedCerts)) {
            log.debug("key information does not resolve to a certificate, skipping it");
            continue;
        }

        log.debug("checking if certificates contained within key information match end-entity certificate");
        if (resolvedCerts.v().front()->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
            log.error("only the OpenSSL XSEC provider is supported");
            continue;
        }
        else if (!X509_cmp(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(),static_cast<OpenSSLCryptoX509*>(resolvedCerts.v().front())->getOpenSSLX509())) {
            log.info("end-entity certificate matches certificate from peer key information");
            return true;
        }
    }

    log.debug("no certificates within this peer's key information matched the given end-entity certificate");
    return false;
}
