/*
 *  Copyright 2001-2006 Internet2
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
 * @file Encrypter.h
 * 
 * Methods for encrypting XMLObjects and other data.
 */

#if !defined(__xmltooling_decrypter_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_decrypter_h__

#include <xmltooling/encryption/Encryption.h>
#include <xmltooling/signature/KeyResolver.h>

#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/xenc/XENCCipher.hpp>

namespace xmlencryption {

    /**
     * Wrapper API for XML Decryption functionality.
     */
    class XMLTOOL_API Decrypter
    {
    public:
        /**
         * Constructor.
         * Resolvers will be deleted when Decrypter is.
         * 
         * @param KEKresolver   resolves key decryption key
         * @param resolver      resolves data decryption key
         */
        Decrypter(xmlsignature::KeyResolver* KEKresolver=NULL, xmlsignature::KeyResolver* resolver=NULL)
            : m_cipher(NULL), m_resolver(resolver), m_KEKresolver(KEKresolver) {
        }

        ~Decrypter();
        
        /**
         * Replace the current data encryption KeyResolver interface, if any, with a new one.
         * 
         * @param resolver  the KeyResolver to attach 
         */
        void setKeyResolver(xmlsignature::KeyResolver* resolver) {
            delete m_resolver;
            m_resolver=resolver;
        }

        /**
         * Replace the current key encryption KeyResolver interface, if any, with a new one.
         * 
         * @param resolver  the KeyResolver to attach 
         */
        void setKEKResolver(xmlsignature::KeyResolver* resolver) {
            delete m_KEKresolver;
            m_KEKresolver=resolver;
        }

        /**
         * Decrypts the supplied information and returns the resulting as a DOM
         * fragment owned by the document associated with the marshalled EncryptedData
         * object.
         * 
         * Note that the DOM nodes will be invalidated once that document
         * is released. The caller should therefore process the DOM fragment as
         * required and drop all references to it before that happens. The usual
         * approach should be to unmarshall the DOM and then release it, or the
         * DOM can also be imported into a separately owned document.
         * 
         * @param encryptedData the encrypted data to decrypt
         * @return  the decrypted DOM fragment
         */
        DOMDocumentFragment* decryptData(EncryptedData* encryptedData);
        
        /**
         * Decrypts the supplied information and returns the resulting key.
         * The caller is responsible for deleting the key. The algorithm of the
         * key must be supplied by the caller based on knowledge of the associated
         * EncryptedData information.
         * 
         * @param encryptedKey  the encrypted/wrapped key to decrypt
         * @param algorithm     the algorithm associated with the decrypted key
         * @return  the decrypted key
         */
        XSECCryptoKey* decryptKey(EncryptedKey* encryptedKey, const XMLCh* algorithm);
        
    private:
        XENCCipher* m_cipher;
        xmlsignature::KeyResolver* m_resolver;
        xmlsignature::KeyResolver* m_KEKresolver;
    };

    DECL_XMLTOOLING_EXCEPTION(DecryptionException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmlencryption,xmltooling::XMLToolingException,Exceptions in decryption processing);

};

#endif /* __xmltooling_decrypter_h__ */
