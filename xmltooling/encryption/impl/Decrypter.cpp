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
#include "encryption/Decrypter.h"
#include "encryption/EncryptedKeyResolver.h"

#include <log4cpp/Category.hh>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/framework/XSECAlgorithmMapper.hpp>
#include <xsec/framework/XSECAlgorithmHandler.hpp>
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
    delete m_resolver;
    delete m_KEKresolver;
}

DOMDocumentFragment* Decrypter::decryptData(EncryptedData* encryptedData)
{
    if (encryptedData->getDOM()==NULL)
        throw DecryptionException("The object must be marshalled before decryption.");
    
    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedData->getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher)
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedData->getDOM()->getOwnerDocument());
    
    try {
        // Resolve decryption key.
        XSECCryptoKey* key=NULL;
        if (m_resolver)
            key=m_resolver->resolveKey(encryptedData->getKeyInfo());

        if (!key && m_KEKresolver) {
            // See if there's an encrypted key available. We'll need the algorithm...
            const XMLCh* algorithm=
                encryptedData->getEncryptionMethod() ? encryptedData->getEncryptionMethod()->getAlgorithm() : NULL;
            if (!algorithm)
                throw DecryptionException("No EncryptionMethod/@Algorithm set, key decryption cannot proceed.");
            
            if (encryptedData->getKeyInfo()) {
                const vector<XMLObject*>& others=const_cast<const KeyInfo*>(encryptedData->getKeyInfo())->getUnknownXMLObjects();
                for (vector<XMLObject*>::const_iterator i=others.begin(); i!=others.end(); i++) {
                    EncryptedKey* encKey=dynamic_cast<EncryptedKey*>(*i);
                    if (encKey) {
                        try {
                            key=decryptKey(encKey, algorithm);
                        }
                        catch (DecryptionException& e) {
                            log4cpp::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(e.what());
                        }
                    }
                }
            }
            
            if (!key) {
                // Check for a non-trivial resolver.
                EncryptedKeyResolver* ekr=dynamic_cast<EncryptedKeyResolver*>(m_resolver);
                if (ekr) {
                    EncryptedKey* encKey=ekr->resolveKey(encryptedData);
                    if (encKey) {
                        try {
                            key=decryptKey(encKey, algorithm);
                        }
                        catch (DecryptionException& e) {
                            log4cpp::Category::getInstance(XMLTOOLING_LOGCAT".Decrypter").warn(e.what());
                        }
                    }
                }
            }
        }

        if (!key)
            throw DecryptionException("Unable to resolve a decryption key.");
        
        m_cipher->setKey(key);
        DOMNode* ret=m_cipher->decryptElementDetached(encryptedData->getDOM());
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

XSECCryptoKey* Decrypter::decryptKey(EncryptedKey* encryptedKey, const XMLCh* algorithm)
{
    if (encryptedKey->getDOM()==NULL)
        throw DecryptionException("The object must be marshalled before decryption.");
    
    // We can reuse the cipher object if the document hasn't changed.

    if (m_cipher && m_cipher->getDocument()!=encryptedKey->getDOM()->getOwnerDocument()) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseCipher(m_cipher);
        m_cipher=NULL;
    }
    
    if (!m_cipher)
        m_cipher=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newCipher(encryptedKey->getDOM()->getOwnerDocument());
    
    try {
        // Resolve key decryption key.
        XSECCryptoKey* key=NULL;
        if (m_KEKresolver)
            key=m_KEKresolver->resolveKey(encryptedKey->getKeyInfo());
        if (!key)
            throw DecryptionException("Unable to resolve a key decryption key.");
        m_cipher->setKEK(key);
        
        XMLByte buffer[1024];
        int keySize = m_cipher->decryptKey(encryptedKey->getDOM(), buffer, 1024);
        if (keySize > 0) {
            // Try to map the key.
            XSECAlgorithmHandler* handler = XSECPlatformUtils::g_algorithmMapper->mapURIToHandler(algorithm);
            if (handler != NULL)
                return handler->createKeyForURI(algorithm, buffer, keySize);
            throw DecryptionException("Unrecognized algorithm, could not build object around decrypted key.");
        }
        throw DecryptionException("Unable to decrypt key.");
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw DecryptionException(string("XMLSecurity exception while decrypting: ") + e.getMsg());
    }
}
