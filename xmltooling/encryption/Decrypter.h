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
 * @file xmltooling/encryption/Decrypter.h
 * 
 * Wrapper API for XML Decryption functionality.
 */

#if !defined(__xmltooling_decrypter_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_decrypter_h__

#include <xmltooling/encryption/Encryption.h>

#include <xsec/xenc/XENCCipher.hpp>

namespace xmltooling {
    class XMLTOOL_API CredentialResolver;
    class XMLTOOL_API KeyResolver;
};

namespace xmlencryption {

    /**
     * Wrapper API for XML Decryption functionality.
     */
    class XMLTOOL_API Decrypter
    {
    public:
        /**
         * Constructor.
         * 
         * @param KEKresolver   locked credential resolver to supply key decryption key
         * @param resolver      directly or indirectly resolves the data decryption key
         */
        Decrypter(const xmltooling::CredentialResolver* KEKresolver=NULL, const xmltooling::KeyResolver* resolver=NULL)
            : m_cipher(NULL), m_KEKresolver(KEKresolver), m_resolver(resolver) {
        }

        ~Decrypter();
        
        /**
         * Replace the current data encryption KeyResolver interface, if any, with a new one.
         * 
         * @param resolver  the KeyResolver to attach 
         */
        void setKeyResolver(const xmltooling::KeyResolver* resolver) {
            m_resolver=resolver;
        }

        /**
         * Replace the current key encryption CredentialResolver interface, if any, with a new one.
         * 
         * @param resolver  the locked CredentialResolver to attach 
         */
        void setKEKResolver(const xmltooling::CredentialResolver* resolver) {
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
         * @param encryptedData the data to decrypt
         * @return  the decrypted DOM fragment
         */
        DOMDocumentFragment* decryptData(EncryptedData& encryptedData);
        
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
        XSECCryptoKey* decryptKey(EncryptedKey& encryptedKey, const XMLCh* algorithm);
        
    private:
        XENCCipher* m_cipher;
        const xmltooling::CredentialResolver* m_KEKresolver;
        const xmltooling::KeyResolver* m_resolver;
    };

    DECL_XMLTOOLING_EXCEPTION(DecryptionException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmlencryption,xmltooling::XMLToolingException,Exceptions in decryption processing);

};

#endif /* __xmltooling_decrypter_h__ */
