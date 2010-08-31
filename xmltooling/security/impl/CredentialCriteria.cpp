/*
 *  Copyright 2001-2010 Internet2
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
#include "XMLToolingConfig.h"
#include "security/X509Credential.h"
#include "security/CredentialCriteria.h"
#include "security/KeyInfoResolver.h"
#include "security/SecurityHelper.h"
#include "signature/Signature.h"

#include <openssl/dsa.h>
#include <openssl/rsa.h>
#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>

using xmlsignature::KeyInfo;
using xmlsignature::Signature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

CredentialCriteria::CredentialCriteria()
    : m_keyUsage(Credential::UNSPECIFIED_CREDENTIAL), m_keySize(0), m_maxKeySize(0), m_key(nullptr),
        m_keyInfo(nullptr), m_nativeKeyInfo(nullptr), m_credential(nullptr)
{
}

CredentialCriteria::~CredentialCriteria()
{
    delete m_credential;
}

unsigned int CredentialCriteria::getUsage() const
{
    return m_keyUsage;
}

void CredentialCriteria::setUsage(unsigned int usage)
{
    m_keyUsage = usage;
}

const char* CredentialCriteria::getPeerName() const
{
    return m_peerName.c_str();
}

void CredentialCriteria::setPeerName(const char* peerName)
{
    m_peerName.erase();
    if (peerName)
        m_peerName = peerName;
}

const char* CredentialCriteria::getKeyAlgorithm() const
{
    return m_keyAlgorithm.c_str();
}

void CredentialCriteria::setKeyAlgorithm(const char* keyAlgorithm)
{
    m_keyAlgorithm.erase();
    if (keyAlgorithm)
        m_keyAlgorithm = keyAlgorithm;
}

unsigned int CredentialCriteria::getKeySize() const
{
    return m_keySize;
}

void CredentialCriteria::setKeySize(unsigned int keySize)
{
    m_keySize = keySize;
}

unsigned int CredentialCriteria::getMaxKeySize() const
{
    return m_maxKeySize;
}

void CredentialCriteria::setMaxKeySize(unsigned int keySize)
{
    m_maxKeySize = keySize;
}

void CredentialCriteria::setXMLAlgorithm(const XMLCh* algorithm)
{
    if (algorithm) {
        pair<const char*,unsigned int> mapped = XMLToolingConfig::getConfig().mapXMLAlgorithmToKeyAlgorithm(algorithm);
        setKeyAlgorithm(mapped.first);
        setKeySize(mapped.second);
    }
    else {
        setKeyAlgorithm(nullptr);
        setKeySize(0);
    }
}

const set<string>& CredentialCriteria::getKeyNames() const
{
    return m_keyNames;
}

set<string>& CredentialCriteria::getKeyNames()
{
    return m_keyNames;
}

XSECCryptoKey* CredentialCriteria::getPublicKey() const
{
    return m_key;
}

void CredentialCriteria::setPublicKey(XSECCryptoKey* key)
{
    m_key = key;
}

const KeyInfo* CredentialCriteria::getKeyInfo() const
{
    return m_keyInfo;
}

void CredentialCriteria::setKeyInfo(const KeyInfo* keyInfo, int extraction)
{
    delete m_credential;
    m_credential = nullptr;
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

DSIGKeyInfoList* CredentialCriteria::getNativeKeyInfo() const
{
    return m_nativeKeyInfo;
}

void CredentialCriteria::setNativeKeyInfo(DSIGKeyInfoList* keyInfo, int extraction)
{
    delete m_credential;
    m_credential = nullptr;
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

void CredentialCriteria::setSignature(const Signature& sig, int extraction)
{
    setXMLAlgorithm(sig.getSignatureAlgorithm());
    KeyInfo* k = sig.getKeyInfo();
    if (k)
        return setKeyInfo(k, extraction);
    DSIGSignature* dsig = sig.getXMLSignature();
    if (dsig)
        setNativeKeyInfo(dsig->getKeyInfoList(), extraction);
}

bool CredentialCriteria::matches(const Credential& credential) const
{
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".CredentialCriteria");

    // Usage check, if specified and we have one, compare masks.
    if (getUsage() != Credential::UNSPECIFIED_CREDENTIAL) {
        if (credential.getUsage() != Credential::UNSPECIFIED_CREDENTIAL)
            if ((getUsage() & credential.getUsage()) == 0) {
                if (log.isDebugEnabled())
                    log.debug("usage didn't match (%u != %u)", getUsage(), credential.getUsage());
                return false;
            }
    }

    // Algorithm check, if specified and we have one.
    const char* alg = getKeyAlgorithm();
    if (alg && *alg) {
        const char* alg2 = credential.getAlgorithm();
        if (alg2 && *alg2) {
            if (strcmp(alg,alg2)) {
                if (log.isDebugEnabled())
                    log.debug("key algorithm didn't match ('%s' != '%s')", getKeyAlgorithm(), credential.getAlgorithm());
                return false;
            }
        }
    }

    // KeySize check, if specified and we have one.
    if (credential.getKeySize() > 0) {
        if (m_keySize > 0 && m_maxKeySize == 0) {
            if (credential.getKeySize() != m_keySize) {
                log.debug("key size (%u) didn't match (%u)", credential.getKeySize(), m_keySize);
                return false;
            }
        }
        else if (m_keySize > 0 && credential.getKeySize() < m_keySize) {
            log.debug("key size (%u) smaller than minimum (%u)", credential.getKeySize(), m_keySize);
            return false;
        }
        else if (m_maxKeySize > 0 && credential.getKeySize() > m_maxKeySize) {
            log.debug("key size (%u) larger than maximum (%u)", credential.getKeySize(), m_maxKeySize);
            return false;
        }
    }

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
        if (!found) {
            log.debug("credential name(s) didn't overlap");
            return false;
        }
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

    if (SecurityHelper::matches(*key1, *key2))
        return true;
    
    log.debug("keys didn't match");
    return false;
}
