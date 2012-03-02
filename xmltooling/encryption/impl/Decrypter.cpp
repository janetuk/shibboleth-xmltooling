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
 * Decrypter.cpp
 * 
 * Methods for decrypting XMLObjects and other data.
 */

#include "internal.h"
#include "logging.h"
#include "encryption/Decrypter.h"
#include "encryption/EncryptedKeyResolver.h"
#include "encryption/Encryption.h"
#include "security/Credential.h"
#include "security/CredentialCriteria.h"
#include "security/CredentialResolver.h"

#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/framework/XSECAlgorithmMapper.hpp>
#include <xsec/framework/XSECAlgorithmHandler.hpp>
#include <xsec/utils/XSECBinTXFMInputStream.hpp>
#include <xsec/xenc/XENCCipher.hpp>
#include <xsec/xenc/XENCEncryptedData.hpp>
#include <xsec/xenc/XENCEncryptedKey.hpp>

using namespace xmlencryption;
using namespace xmlsignature;
using namespace xmltooling;
using namespace xercesc;
using namespace std;


Decrypter::Decrypter(
    const CredentialResolver* credResolver,
    CredentialCriteria* criteria,
    const EncryptedKeyResolver* EKResolver,
    bool requireAuthenticatedCipher
    ) : m_cipher(nullptr), m_credResolver(credResolver), m_criteria(criteria), m_EKResolver(EKResolver),
        m_requireAuthenticatedCipher(requireAuthenticatedCipher)
{
}

Decrypter::~Decrypter()
{
    if (m_cipher)
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
}

void Decrypter::setEncryptedKeyResolver(const EncryptedKeyResolver* EKResolver)
{
    m_EKResolver=EKResolver;
}

void Decrypter::setKEKResolver(const CredentialResolver* resolver, CredentialCriteria* criteria)
{
    m_credResolver=resolver;
    m_criteria=criteria;
}

DOMDocumentFragment* Decrypter::decryptData(const EncryptedData& encryptedData, XSECCryptoKey* key)
{
    if (encryptedData.getDOM() == nullptr)
        throw DecryptionException("The object must be marshalled before decryption.");

    XMLToolingInternalConfig& xmlconf = XMLToolingInternalConfig::getInternalConfig();
    if (m_requireAuthenticatedCipher) {
        const XMLCh* alg = encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : nullptr;
        if (!alg || !xmlconf.isXMLAlgorithmSupported(alg, XMLToolingConfig::ALGTYPE_AUTHNENCRYPT)) {
            throw DecryptionException("Unauthenticated data encryption algorithm unsupported.");
        }
    }

    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedData.getDOM()->getOwnerDocument()) {
        xmlconf.m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=nullptr;
    }
    
    if (!m_cipher)
        m_cipher = xmlconf.m_xsecProvider->newCipher(encryptedData.getDOM()->getOwnerDocument());

    try {
        m_cipher->setKey(key->clone());
        DOMNode* ret = m_cipher->decryptElementDetached(encryptedData.getDOM());
        if (ret->getNodeType()!=DOMNode::DOCUMENT_FRAGMENT_NODE) {
            ret->release();
            throw DecryptionException("Decryption operation did not result in DocumentFragment.");
        }
        return static_cast<DOMDocumentFragment*>(ret);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + e.getMsg());
    }
}

DOMDocumentFragment* Decrypter::decryptData(const EncryptedData& encryptedData, const XMLCh* recipient)
{
    if (!m_credResolver)
        throw DecryptionException("No CredentialResolver supplied to provide decryption keys.");

    // Resolve a decryption key directly.
    vector<const Credential*> creds;
    int types = CredentialCriteria::KEYINFO_EXTRACTION_KEY | CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES;
    if (m_criteria) {
        m_criteria->setUsage(Credential::ENCRYPTION_CREDENTIAL);
        m_criteria->setKeyInfo(encryptedData.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedData.getEncryptionMethod();
        if (meth)
            m_criteria->setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds, m_criteria);
    }
    else {
        CredentialCriteria criteria;
        criteria.setUsage(Credential::ENCRYPTION_CREDENTIAL);
        criteria.setKeyInfo(encryptedData.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedData.getEncryptionMethod();
        if (meth)
            criteria.setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds, &criteria);
    }

    // Loop over them and try each one.
    XSECCryptoKey* key;
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred != creds.end(); ++cred) {
        try {
            key = (*cred)->getPrivateKey();
            if (!key)
                continue;
            return decryptData(encryptedData, key);
        }
        catch(DecryptionException& ex) {
            logging::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(ex.what());
        }
    }

    // We need to find an encrypted decryption key somewhere. We'll need the underlying algorithm...
    const XMLCh* algorithm=
        encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : nullptr;
    if (!algorithm)
        throw DecryptionException("No EncryptionMethod/@Algorithm set, key decryption cannot proceed.");
    
    // Check for external resolver.
    const EncryptedKey* encKey=nullptr;
    if (m_EKResolver)
        encKey = m_EKResolver->resolveKey(encryptedData, recipient);
    else {
        EncryptedKeyResolver ekr;
        encKey = ekr.resolveKey(encryptedData, recipient);
    }

    if (!encKey)
        throw DecryptionException("Unable to locate an encrypted key.");

    auto_ptr<XSECCryptoKey> keywrapper(decryptKey(*encKey, algorithm));
    if (!keywrapper.get())
        throw DecryptionException("Unable to decrypt the encrypted key.");
    return decryptData(encryptedData, keywrapper.get());
}

void Decrypter::decryptData(ostream& out, const EncryptedData& encryptedData, XSECCryptoKey* key)
{
    if (encryptedData.getDOM() == nullptr)
        throw DecryptionException("The object must be marshalled before decryption.");

    XMLToolingInternalConfig& xmlconf = XMLToolingInternalConfig::getInternalConfig();
    if (m_requireAuthenticatedCipher) {
        const XMLCh* alg = encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : nullptr;
        if (!alg || !xmlconf.isXMLAlgorithmSupported(alg, XMLToolingConfig::ALGTYPE_AUTHNENCRYPT)) {
            throw DecryptionException("Unauthenticated data encryption algorithm unsupported.");
        }
    }

    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument() != encryptedData.getDOM()->getOwnerDocument()) {
        xmlconf.m_xsecProvider->releaseCipher(m_cipher);
        m_cipher = nullptr;
    }
    
    if (!m_cipher)
        m_cipher = xmlconf.m_xsecProvider->newCipher(encryptedData.getDOM()->getOwnerDocument());

    try {
        m_cipher->setKey(key->clone());
        auto_ptr<XSECBinTXFMInputStream> in(m_cipher->decryptToBinInputStream(encryptedData.getDOM()));
        
        XMLByte buf[8192];
        xsecsize_t count = in->readBytes(buf, sizeof(buf));
        while (count > 0)
            out.write(reinterpret_cast<char*>(buf),count);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + e.getMsg());
    }
}

void Decrypter::decryptData(ostream& out, const EncryptedData& encryptedData, const XMLCh* recipient)
{
    if (!m_credResolver)
        throw DecryptionException("No CredentialResolver supplied to provide decryption keys.");

    // Resolve a decryption key directly.
    vector<const Credential*> creds;
    int types = CredentialCriteria::KEYINFO_EXTRACTION_KEY | CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES;
    if (m_criteria) {
        m_criteria->setUsage(Credential::ENCRYPTION_CREDENTIAL);
        m_criteria->setKeyInfo(encryptedData.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedData.getEncryptionMethod();
        if (meth)
            m_criteria->setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds,m_criteria);
    }
    else {
        CredentialCriteria criteria;
        criteria.setUsage(Credential::ENCRYPTION_CREDENTIAL);
        criteria.setKeyInfo(encryptedData.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedData.getEncryptionMethod();
        if (meth)
            criteria.setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds,&criteria);
    }

    // Loop over them and try each one.
    XSECCryptoKey* key;
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred != creds.end(); ++cred) {
        try {
            key = (*cred)->getPrivateKey();
            if (!key)
                continue;
            return decryptData(out, encryptedData, key);
        }
        catch(DecryptionException& ex) {
            logging::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(ex.what());
        }
    }

    // We need to find an encrypted decryption key somewhere. We'll need the underlying algorithm...
    const XMLCh* algorithm=
        encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : nullptr;
    if (!algorithm)
        throw DecryptionException("No EncryptionMethod/@Algorithm set, key decryption cannot proceed.");
    
    // Check for external resolver.
    const EncryptedKey* encKey=nullptr;
    if (m_EKResolver)
        encKey = m_EKResolver->resolveKey(encryptedData, recipient);
    else {
        EncryptedKeyResolver ekr;
        encKey = ekr.resolveKey(encryptedData, recipient);
    }

    if (!encKey)
        throw DecryptionException("Unable to locate an encrypted key.");

    auto_ptr<XSECCryptoKey> keywrapper(decryptKey(*encKey, algorithm));
    if (!keywrapper.get())
        throw DecryptionException("Unable to decrypt the encrypted key.");
    decryptData(out, encryptedData, keywrapper.get());
}

XSECCryptoKey* Decrypter::decryptKey(const EncryptedKey& encryptedKey, const XMLCh* algorithm)
{
    if (!m_credResolver)
        throw DecryptionException("No CredentialResolver supplied to provide decryption keys.");

    if (encryptedKey.getDOM()==nullptr)
        throw DecryptionException("The object must be marshalled before decryption.");

    XSECAlgorithmHandler* handler;
    try {
        handler = XSECPlatformUtils::g_algorithmMapper->mapURIToHandler(algorithm);
        if (!handler)
            throw DecryptionException("Unrecognized algorithm, no way to build object around decrypted key.");
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw DecryptionException(string("XMLSecurity exception while decrypting key: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw DecryptionException(string("XMLSecurity exception while decrypting key: ") + e.getMsg());
    }
    
    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedKey.getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher = nullptr;
    }
    
    if (!m_cipher)
        m_cipher = XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedKey.getDOM()->getOwnerDocument());
    
    // Resolve key decryption keys.
    int types = CredentialCriteria::KEYINFO_EXTRACTION_KEY | CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES;
    vector<const Credential*> creds;
    if (m_criteria) {
        m_criteria->setUsage(Credential::ENCRYPTION_CREDENTIAL);
        m_criteria->setKeyInfo(encryptedKey.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedKey.getEncryptionMethod();
        if (meth)
            m_criteria->setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds, m_criteria);
    }
    else {
        CredentialCriteria criteria;
        criteria.setUsage(Credential::ENCRYPTION_CREDENTIAL);
        criteria.setKeyInfo(encryptedKey.getKeyInfo(), types);
        const EncryptionMethod* meth = encryptedKey.getEncryptionMethod();
        if (meth)
            criteria.setXMLAlgorithm(meth->getAlgorithm());
        m_credResolver->resolve(creds, &criteria);
    }
    if (creds.empty())
        throw DecryptionException("Unable to resolve any key decryption keys.");

    XMLByte buffer[1024];
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred != creds.end(); ++cred) {
        try {
            if (!(*cred)->getPrivateKey())
                throw DecryptionException("Credential did not contain a private key.");
            memset(buffer,0,sizeof(buffer));
            m_cipher->setKEK((*cred)->getPrivateKey()->clone());

            try {
                int keySize = m_cipher->decryptKey(encryptedKey.getDOM(), buffer, 1024);
                if (keySize<=0)
                    throw DecryptionException("Unable to decrypt key.");
        
                // Try to wrap the key.
                return handler->createKeyForURI(algorithm, buffer, keySize);
            }
            catch(XSECException& e) {
                auto_ptr_char temp(e.getMsg());
                throw DecryptionException(string("XMLSecurity exception while decrypting key: ") + temp.get());
            }
            catch(XSECCryptoException& e) {
                throw DecryptionException(string("XMLSecurity exception while decrypting key: ") + e.getMsg());
            }
        }
        catch(DecryptionException& ex) {
            logging::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(ex.what());
        }
    }
    
    // Some algorithms are vulnerable to chosen ciphertext attacks, so we generate a random key
    // to prevent discovery of the validity of the original candidate.
    logging::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(
        "unable to decrypt key, generating random key for defensive purposes"
        );
    pair<const char*,unsigned int> mapped = XMLToolingConfig::getConfig().mapXMLAlgorithmToKeyAlgorithm(algorithm);
    if (!mapped.second)
        mapped.second = 256;
    try {
        if (XSECPlatformUtils::g_cryptoProvider->getRandom(reinterpret_cast<unsigned char*>(buffer),mapped.second) < mapped.second)
            throw DecryptionException("Unable to generate random data; was PRNG seeded?");
        return handler->createKeyForURI(algorithm, buffer, mapped.second);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw DecryptionException(string("XMLSecurity exception while generating key: ") + temp.get());
    }
    catch (XSECCryptoException& e) {
        throw DecryptionException(string("XMLSecurity exception while generating key: ") + e.getMsg());
    }
}
