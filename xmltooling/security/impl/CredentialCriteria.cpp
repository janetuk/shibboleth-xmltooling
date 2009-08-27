/*
 *  Copyright 2001-2009 Internet2
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
 * CredentialCriteria.cpp
 *
 * Class for specifying criteria by which a CredentialResolver should resolve credentials.
 */

#include "internal.h"
#include "logging.h"
#include "security/X509Credential.h"
#include "security/CredentialCriteria.h"
#include "security/KeyInfoResolver.h"
#include "security/SecurityHelper.h"

#include <openssl/dsa.h>
#include <openssl/rsa.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>

using namespace xmltooling;
using namespace std;

void CredentialCriteria::setKeyInfo(const xmlsignature::KeyInfo* keyInfo, int extraction)
{
    delete m_credential;
    m_credential = NULL;
    m_keyInfo = keyInfo;
    if (!keyInfo || !extraction)
        return;

    int types = (extraction & KEYINFO_EXTRACTION_KEY) ? Credential::RESOLVE_KEYS : 0;
    types |= (extraction & KEYINFO_EXTRACTION_KEYNAMES) ? X509Credential::RESOLVE_CERTS : 0;
    m_credential = XMLToolingConfig::getConfig().getKeyInfoResolver()->resolve(keyInfo,types);

    // Ensure any key names have been sucked out for later if desired.
    if (extraction & KEYINFO_EXTRACTION_KEYNAMES) {
        X509Credential* xcred = dynamic_cast<X509Credential*>(m_credential);
        if (xcred)
            xcred->extract();
    }
} 

void CredentialCriteria::setNativeKeyInfo(DSIGKeyInfoList* keyInfo, int extraction)
{
    delete m_credential;
    m_credential = NULL;
    m_nativeKeyInfo = keyInfo;
    if (!keyInfo || !extraction)
        return;

    int types = (extraction & KEYINFO_EXTRACTION_KEY) ? Credential::RESOLVE_KEYS : 0;
    types |= (extraction & KEYINFO_EXTRACTION_KEYNAMES) ? X509Credential::RESOLVE_CERTS : 0;
    m_credential = XMLToolingConfig::getConfig().getKeyInfoResolver()->resolve(keyInfo,types);

    // Ensure any key names have been sucked out for later if desired.
    if (extraction & KEYINFO_EXTRACTION_KEYNAMES) {
        X509Credential* xcred = dynamic_cast<X509Credential*>(m_credential);
        if (xcred)
            xcred->extract();
    }
}

void CredentialCriteria::setSignature(const xmlsignature::Signature& sig, int extraction)
{
    setXMLAlgorithm(sig.getSignatureAlgorithm());
    xmlsignature::KeyInfo* k = sig.getKeyInfo();
    if (k)
        return setKeyInfo(k, extraction);
    DSIGSignature* dsig = sig.getXMLSignature();
    if (dsig)
        setNativeKeyInfo(dsig->getKeyInfoList(), extraction);
}

bool CredentialCriteria::matches(const Credential& credential) const
{
    // Usage check, if specified and we have one, compare masks.
    if (getUsage() != Credential::UNSPECIFIED_CREDENTIAL) {
        if (credential.getUsage() != Credential::UNSPECIFIED_CREDENTIAL)
            if ((getUsage() & credential.getUsage()) == 0)
                return false;
    }

    // Algorithm check, if specified and we have one.
    const char* alg = getKeyAlgorithm();
    if (alg && *alg) {
        const char* alg2 = credential.getAlgorithm();
        if (alg2 && *alg2)
            if (strcmp(alg,alg2))
                return false;
    }

    // KeySize check, if specified and we have one.
    if (credential.getKeySize()>0 && getKeySize()>0 && credential.getKeySize() != getKeySize())
        return false;

    // See if we can test key names.
    set<string> critnames = getKeyNames();
    if (m_credential)
        critnames.insert(m_credential->getKeyNames().begin(), m_credential->getKeyNames().end());
    const set<string>& crednames = credential.getKeyNames();
    if (!critnames.empty() && !crednames.empty()) {
        bool found = false;
        for (set<string>::const_iterator n = critnames.begin(); n!=critnames.end(); ++n) {
            if (crednames.count(*n)>0) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }

    // See if we have to match a specific key.
    const XSECCryptoKey* key1 = getPublicKey();
    if (!key1 && m_credential)
        key1 = m_credential->getPublicKey();
    if (!key1)
        return true;    // no key to compare against, so we're done

    const XSECCryptoKey* key2 = credential.getPublicKey();
    if (!key2)
        return true;   // no key here, so we can't test it

    return SecurityHelper::matches(*key1, *key2);
}
