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
 * @file xmltooling/encryption/Decrypter.h
 * 
 * Wrapper API for XML Decryption functionality.
 */

#if !defined(__xmltooling_decrypter_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_decrypter_h__

#include <xmltooling/exceptions.h>

class XENCCipher;
class XSECCryptoKey;

namespace xmltooling {
    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API CredentialResolver;
};

namespace xmlencryption {

    class XMLTOOL_API EncryptedData;
    class XMLTOOL_API EncryptedKey;
    class XMLTOOL_API EncryptedKeyResolver;

    /**
     * Wrapper API for XML Decryption functionality.
     */
    class XMLTOOL_API Decrypter
    {
    public:
        /**
         * Constructor.
         * 
         * @param credResolver  locked credential resolver to supply decryption keys
         * @param criteria      optional external criteria to use with resolver
         * @param EKResolver    locates an EncryptedKey pertaining to the EncryptedData
         */
        Decrypter(
            const xmltooling::CredentialResolver* credResolver=nullptr,
            xmltooling::CredentialCriteria* criteria=nullptr,
            const EncryptedKeyResolver* EKResolver=nullptr
            );

        virtual ~Decrypter();
        
        /**
         * Replace the current EncryptedKeyResolver interface, if any, with a new one.
         * 
         * @param EKResolver  the EncryptedKeyResolver to attach 
         */
        void setEncryptedKeyResolver(const EncryptedKeyResolver* EKResolver);

        /**
         * Replace the current CredentialResolver interface, if any, with a new one.
         * 
         * @param resolver  the locked CredentialResolver to attach, or nullptr to clear
         * @param criteria  optional external criteria to use with resolver
         */
        void setKEKResolver(const xmltooling::CredentialResolver* resolver, xmltooling::CredentialCriteria* criteria);

        /**
         * Decrypts the supplied information using the supplied key, and returns
         * the resulting as a DOM fragment owned by the document associated with the
         * marshalled EncryptedData object.
         * 
         * Note that the DOM nodes will be invalidated once that document
         * is released. The caller should therefore process the DOM fragment as
         * required and drop all references to it before that happens. The usual
         * approach should be to unmarshall the DOM and then release it, or the
         * DOM can also be imported into a separately owned document.
         * 
         * @param encryptedData the data to decrypt
         * @param key           the decryption key to use (it will not be freed internally)
         * @return  the decrypted DOM fragment
         */
        xercesc::DOMDocumentFragment* decryptData(const EncryptedData& encryptedData, XSECCryptoKey* key);

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
         * @param recipient identifier of decrypting entity for use in identifying multi-cast keys
         * @return  the decrypted DOM fragment
         */
        xercesc::DOMDocumentFragment* decryptData(const EncryptedData& encryptedData, const XMLCh* recipient=nullptr);
        
        /**
         * Decrypts the supplied information to an output stream.
         *
         * @param out           output stream to receive the decrypted data 
         * @param encryptedData the data to decrypt
         * @param key           the decryption key to use (it will not be freed internally)
         */
        void decryptData(std::ostream& out, const EncryptedData& encryptedData, XSECCryptoKey* key);

        /**
         * Decrypts the supplied information to an output stream.
         *
         * @param out           output stream to receive the decrypted data 
         * @param encryptedData the data to decrypt
         * @param recipient     identifier of decrypting entity for use in identifying multi-cast keys
         */
        void decryptData(std::ostream& out, const EncryptedData& encryptedData, const XMLCh* recipient=nullptr);

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
        XSECCryptoKey* decryptKey(const EncryptedKey& encryptedKey, const XMLCh* algorithm);
        
    private:
        XENCCipher* m_cipher;
        const xmltooling::CredentialResolver* m_credResolver;
        xmltooling::CredentialCriteria* m_criteria;
        const EncryptedKeyResolver* m_EKResolver;
    };

    DECL_XMLTOOLING_EXCEPTION(DecryptionException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmlencryption,xmltooling::XMLToolingException,Exceptions in decryption processing);

};

#endif /* __xmltooling_decrypter_h__ */
