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
 * Encrypter.cpp
 * 
 * Methods for encrypting XMLObjects and other data.
 */

#include "internal.h"
#include "encryption/Encrypter.h"
#include "security/Credential.h"

#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/framework/XSECAlgorithmMapper.hpp>
#include <xsec/framework/XSECAlgorithmHandler.hpp>
#include <xsec/xenc/XENCEncryptedData.hpp>
#include <xsec/xenc/XENCEncryptedKey.hpp>

using namespace xmlencryption;
using namespace xmlsignature;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

Encrypter::~Encrypter()
{
    XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
    memset(m_keyBuffer,0,32);
}

void Encrypter::checkParams(EncryptionParams& encParams, KeyEncryptionParams* kencParams)
{
    if (encParams.m_keyBufferSize==0) {
        if (encParams.m_credential) {
            if (kencParams)
                throw EncryptionException("Generating EncryptedKey inline requires the encryption key in raw form.");
        }
        else {
            if (!kencParams)
                throw EncryptionException("Using a generated encryption key requires a KeyEncryptionParams object.");

            // We're generating a random key. The maximum supported length is AES-256, so we need 32 bytes.
            if (XSECPlatformUtils::g_cryptoProvider->getRandom(m_keyBuffer,32)<32)
                throw EncryptionException("Unable to generate random data; was PRNG seeded?");
            encParams.m_keyBuffer=m_keyBuffer;
            encParams.m_keyBufferSize=32;
        }
    }
    
    XSECCryptoKey* key=NULL;
    if (encParams.m_credential) {
        key = encParams.m_credential->getPrivateKey();
        if (!key)
            throw EncryptionException("Credential in EncryptionParams structure did not supply a private/secret key.");
        // Set the encryption key.
        m_cipher->setKey(key->clone());
    }
    else {
        // We have to have a raw key now, so we need to build a wrapper around it.
        XSECAlgorithmHandler* handler =XSECPlatformUtils::g_algorithmMapper->mapURIToHandler(encParams.m_algorithm);
        if (handler != NULL)
            key = handler->createKeyForURI(
                encParams.m_algorithm,const_cast<unsigned char*>(encParams.m_keyBuffer),encParams.m_keyBufferSize
                );

        if (!key)
            throw EncryptionException("Unable to build wrapper for key, unknown algorithm?");
        // Overwrite the length if known.
        switch (static_cast<XSECCryptoSymmetricKey*>(key)->getSymmetricKeyType()) {
            case XSECCryptoSymmetricKey::KEY_3DES_192:
                encParams.m_keyBufferSize = 192/8;
                break;
            case XSECCryptoSymmetricKey::KEY_AES_128:
                encParams.m_keyBufferSize = 128/8;
                break;
            case XSECCryptoSymmetricKey::KEY_AES_192:
                encParams.m_keyBufferSize = 192/8;
                break;
            case XSECCryptoSymmetricKey::KEY_AES_256:
                encParams.m_keyBufferSize = 256/8;
                break;
        }
        // Set the encryption key.
        m_cipher->setKey(key);
    }
}

EncryptedData* Encrypter::encryptElement(DOMElement* element, EncryptionParams& encParams, KeyEncryptionParams* kencParams)
{
    // We can reuse the cipher object if the document hasn't changed.
    
    if (m_cipher && m_cipher->getDocument()!=element->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher) {
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(element->getOwnerDocument());
        m_cipher->setExclusiveC14nSerialisation(false);
    }
    
    try {
        checkParams(encParams,kencParams);
        m_cipher->encryptElementDetached(element, ENCRYPT_NONE, encParams.m_algorithm);
        return decorateAndUnmarshall(encParams, kencParams);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + e.getMsg());
    }
}

EncryptedData* Encrypter::encryptElementContent(DOMElement* element, EncryptionParams& encParams, KeyEncryptionParams* kencParams)
{
    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=element->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher) {
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(element->getOwnerDocument());
        m_cipher->setExclusiveC14nSerialisation(false);
    }
    
    try {
        checkParams(encParams,kencParams);
        m_cipher->encryptElementContentDetached(element, ENCRYPT_NONE, encParams.m_algorithm);
        return decorateAndUnmarshall(encParams, kencParams);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + e.getMsg());
    }
}

EncryptedData* Encrypter::encryptStream(istream& input, EncryptionParams& encParams, KeyEncryptionParams* kencParams)
{
    // Get a fresh cipher object and document.

    if (m_cipher) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    DOMDocument* doc=NULL;
    try {
        doc=XMLToolingConfig::getConfig().getParser().newDocument();
        XercesJanitor<DOMDocument> janitor(doc);
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(doc);
        m_cipher->setExclusiveC14nSerialisation(false);
        
        checkParams(encParams,kencParams);
        StreamInputSource::StreamBinInputStream xstream(input);
        m_cipher->encryptBinInputStream(&xstream, ENCRYPT_NONE, encParams.m_algorithm);
        return decorateAndUnmarshall(encParams, kencParams);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + e.getMsg());
    }
}

EncryptedData* Encrypter::decorateAndUnmarshall(EncryptionParams& encParams, KeyEncryptionParams* kencParams)
{
    XENCEncryptedData* encData=m_cipher->getEncryptedData();
    if (!encData)
        throw EncryptionException("No EncryptedData element found?");

    // Unmarshall a tooling version of EncryptedData around the DOM.
    EncryptedData* xmlEncData=NULL;
    auto_ptr<XMLObject> xmlObject(XMLObjectBuilder::buildOneFromElement(encData->getElement()));
    if (!(xmlObject.get()) || !(xmlEncData=dynamic_cast<EncryptedData*>(xmlObject.get())))
        throw EncryptionException("Unable to unmarshall into EncryptedData object.");
    
    // Unbind from DOM so we can divorce this from the original document.
    xmlEncData->releaseThisAndChildrenDOM();
    
    // KeyInfo?
    KeyInfo* kinfo = encParams.m_credential ? encParams.m_credential->getKeyInfo(encParams.m_compact) : NULL;
    if (kinfo)
        xmlEncData->setKeyInfo(kinfo);
    
    // Are we doing a key encryption?
    if (kencParams) {
        XSECCryptoKey* kek = kencParams->m_credential.getPublicKey();
        if (!kek)
            throw EncryptionException("Credential in KeyEncryptionParams structure did not supply a public key.");
        if (!kencParams->m_algorithm)
            kencParams->m_algorithm = getKeyTransportAlgorithm(kencParams->m_credential, encParams.m_algorithm);

        m_cipher->setKEK(kek->clone());
        // ownership of this belongs to us, for some reason...
        auto_ptr<XENCEncryptedKey> encKey(
            m_cipher->encryptKey(encParams.m_keyBuffer, encParams.m_keyBufferSize, ENCRYPT_NONE, kencParams->m_algorithm)
            );
        EncryptedKey* xmlEncKey=NULL;
        auto_ptr<XMLObject> xmlObjectKey(XMLObjectBuilder::buildOneFromElement(encKey->getElement()));
        if (!(xmlObjectKey.get()) || !(xmlEncKey=dynamic_cast<EncryptedKey*>(xmlObjectKey.get())))
            throw EncryptionException("Unable to unmarshall into EncryptedKey object.");
        
        xmlEncKey->releaseThisAndChildrenDOM();
        
        // Recipient?
        if (kencParams->m_recipient)
            xmlEncKey->setRecipient(kencParams->m_recipient);
        
        // KeyInfo?
        kinfo = kencParams->m_credential.getKeyInfo(encParams.m_compact);
        if (kinfo)
            xmlEncKey->setKeyInfo(kinfo);
        
        // Add the EncryptedKey inline.
        if (!xmlEncData->getKeyInfo())
            xmlEncData->setKeyInfo(KeyInfoBuilder::buildKeyInfo());
        xmlEncData->getKeyInfo()->getUnknownXMLObjects().push_back(xmlEncKey);
        xmlObjectKey.release();
    }
    
    xmlObject.release();
    return xmlEncData;
}

EncryptedKey* Encrypter::encryptKey(
    const unsigned char* keyBuffer, unsigned int keyBufferSize, KeyEncryptionParams& kencParams, bool compact
    )
{
    // Get a fresh cipher object and document.

    if (m_cipher) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }

    XSECCryptoKey* kek = kencParams.m_credential.getPublicKey();
    if (!kek)
        throw EncryptionException("Credential in KeyEncryptionParams structure did not supply a public key.");

    DOMDocument* doc=NULL;
    try {
        doc=XMLToolingConfig::getConfig().getParser().newDocument();
        XercesJanitor<DOMDocument> janitor(doc);
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(doc);
        m_cipher->setExclusiveC14nSerialisation(false);
        m_cipher->setKEK(kek->clone());
        auto_ptr<XENCEncryptedKey> encKey(m_cipher->encryptKey(keyBuffer, keyBufferSize, ENCRYPT_NONE, kencParams.m_algorithm));
        
        EncryptedKey* xmlEncKey=NULL;
        auto_ptr<XMLObject> xmlObjectKey(XMLObjectBuilder::buildOneFromElement(encKey->getElement()));
        if (!(xmlObjectKey.get()) || !(xmlEncKey=dynamic_cast<EncryptedKey*>(xmlObjectKey.get())))
            throw EncryptionException("Unable to unmarshall into EncryptedKey object.");
        
        xmlEncKey->releaseThisAndChildrenDOM();
        
        // Recipient?
        if (kencParams.m_recipient)
            xmlEncKey->setRecipient(kencParams.m_recipient);

        // KeyInfo?
        KeyInfo* kinfo = kencParams.m_credential.getKeyInfo(compact);
        if (kinfo)
            xmlEncKey->setKeyInfo(kinfo);

        xmlObjectKey.release();
        return xmlEncKey;
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw EncryptionException(string("XMLSecurity exception while encrypting: ") + e.getMsg());
    }
}

const XMLCh* Encrypter::getKeyTransportAlgorithm(const Credential& credential, const XMLCh* encryptionAlg)
{
    const char* alg = credential.getAlgorithm();
    if (!alg || !strcmp(alg, "RSA")) {
        if (XMLString::equals(encryptionAlg,DSIGConstants::s_unicodeStrURI3DES_CBC))
            return DSIGConstants::s_unicodeStrURIRSA_1_5;
        else
            return DSIGConstants::s_unicodeStrURIRSA_OAEP_MGFP1;
    }
    else if (!strcmp(alg, "AES")) {
        switch (credential.getKeySize()) {
            case 128:
                return DSIGConstants::s_unicodeStrURIKW_AES128;
            case 192:
                return DSIGConstants::s_unicodeStrURIKW_AES192;
            case 256:
                return DSIGConstants::s_unicodeStrURIKW_AES256;
        }
    }
    else if (!strcmp(alg, "DESede")) {
        return DSIGConstants::s_unicodeStrURIKW_3DES;
    }

    return NULL;
}
