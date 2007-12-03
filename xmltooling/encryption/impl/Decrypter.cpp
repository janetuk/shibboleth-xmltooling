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
 * Decrypter.cpp
 * 
 * Methods for decrypting XMLObjects and other data.
 */

#include "internal.h"
#include "logging.h"
#include "encryption/Decrypter.h"
#include "encryption/EncryptedKeyResolver.h"
#include "security/Credential.h"
#include "security/CredentialCriteria.h"
#include "security/CredentialResolver.h"

#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/framework/XSECAlgorithmMapper.hpp>
#include <xsec/framework/XSECAlgorithmHandler.hpp>
#include <xsec/utils/XSECBinTXFMInputStream.hpp>
#include <xsec/xenc/XENCEncryptedData.hpp>
#include <xsec/xenc/XENCEncryptedKey.hpp>

using namespace xmlencryption;
using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

Decrypter::~Decrypter()
{
    if (m_cipher)
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
}

DOMDocumentFragment* Decrypter::decryptData(const EncryptedData& encryptedData, XSECCryptoKey* key)
{
    if (encryptedData.getDOM()==NULL)
        throw DecryptionException("The object must be marshalled before decryption.");

    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedData.getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher)
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedData.getDOM()->getOwnerDocument());

    try {
        m_cipher->setKey(key->clone());
        DOMNode* ret=m_cipher->decryptElementDetached(encryptedData.getDOM());
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
    int types =
        CredentialCriteria::KEYINFO_EXTRACTION_KEY |
        CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES |
        CredentialCriteria::KEYINFO_EXTRACTION_IMPLICIT_KEYNAMES;
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
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred) {
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
        encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : NULL;
    if (!algorithm)
        throw DecryptionException("No EncryptionMethod/@Algorithm set, key decryption cannot proceed.");
    
    // Check for external resolver.
    const EncryptedKey* encKey=NULL;
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
    if (encryptedData.getDOM()==NULL)
        throw DecryptionException("The object must be marshalled before decryption.");

    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedData.getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher)
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedData.getDOM()->getOwnerDocument());

    try {
        m_cipher->setKey(key->clone());
        auto_ptr<XSECBinTXFMInputStream> in(m_cipher->decryptToBinInputStream(encryptedData.getDOM()));
        
        XMLByte buf[8192];
        unsigned int count = in->readBytes(buf, sizeof(buf));
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
    int types =
        CredentialCriteria::KEYINFO_EXTRACTION_KEY |
        CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES |
        CredentialCriteria::KEYINFO_EXTRACTION_IMPLICIT_KEYNAMES;
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
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred) {
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
        encryptedData.getEncryptionMethod() ? encryptedData.getEncryptionMethod()->getAlgorithm() : NULL;
    if (!algorithm)
        throw DecryptionException("No EncryptionMethod/@Algorithm set, key decryption cannot proceed.");
    
    // Check for external resolver.
    const EncryptedKey* encKey=NULL;
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

    if (encryptedKey.getDOM()==NULL)
        throw DecryptionException("The object must be marshalled before decryption.");

    XSECAlgorithmHandler* handler = XSECPlatformUtils::g_algorithmMapper->mapURIToHandler(algorithm);
    if (!handler)
        throw DecryptionException("Unrecognized algorithm, no way to build object around decrypted key.");
    
    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedKey.getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher)
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedKey.getDOM()->getOwnerDocument());
    
    // Resolve key decryption keys.
    int types =
        CredentialCriteria::KEYINFO_EXTRACTION_KEY |
        CredentialCriteria::KEYINFO_EXTRACTION_KEYNAMES |
        CredentialCriteria::KEYINFO_EXTRACTION_IMPLICIT_KEYNAMES;
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
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred) {
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
    
    throw DecryptionException("Unable to decrypt key.");
}
