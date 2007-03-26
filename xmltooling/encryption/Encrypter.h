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
 * @file xmltooling/encryption/Encrypter.h
 * 
 * Methods for encrypting XMLObjects and other data.
 */

#if !defined(__xmltooling_encrypter_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_encrypter_h__

#include <xmltooling/encryption/Encryption.h>

#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/xenc/XENCCipher.hpp>

namespace xmltooling {
    class XMLTOOL_API Credential;
};

namespace xmlencryption {

    /**
     * Wrapper API for XML Encryption functionality.
     * Designed to allow both external and internal key generation as follows:
     * 
     * If no keying material is supplied, then the algorithm MAY be recognized
     * and a key can be generated internally. This is only done if a KeyEncryptionParams
     * structure is also supplied to the operation (otherwise the key would be lost).
     * 
     * If an XSECCryptoKey is supplied, then it is used directly, but if KeyEncryptionParams
     * are supplied, an exception will result unless the raw key buffer is also supplied.
     * 
     * If a raw key is provided, then a key object can also be created internally if the
     * algorithm is recognized.
     * 
     * Summing up, if KeyEncryptionParams are used, a raw key must be available or the
     * key can be generated when the encryption algorithm itself is a standard one. If
     * no KeyEncryptionParams are supplied, then the key must be supplied either in raw
     * or object form. 
     */
    class XMLTOOL_API Encrypter
    {
    public:

        /**
         * Structure to collect encryption requirements.
         */
        struct XMLTOOL_API EncryptionParams {
            
            /**
             * Constructor.
             *
             * The algorithm constant and key buffer <strong>MUST</strong> be accessible for the life of
             * the structure.
             * 
             * @param algorithm     the XML Encryption algorithm constant
             * @param keyBuffer     buffer containing the raw key information
             * @param keyBufferSize the size of the raw key buffer in bytes  
             * @param credential    optional Credential supplying the encryption key
             * @param compact       true iff the encrypted representation should be made as small as possible
             */
            EncryptionParams(
                const XMLCh* algorithm=DSIGConstants::s_unicodeStrURIAES256_CBC,
                const unsigned char* keyBuffer=NULL,
                unsigned int keyBufferSize=0,
                const xmltooling::Credential* credential=NULL,
                bool compact=false
                ) :  m_algorithm(algorithm), m_keyBuffer(keyBuffer), m_keyBufferSize(keyBufferSize),
                    m_credential(credential), m_compact(compact) {
            }

            ~EncryptionParams() {}
        private:
            const XMLCh* m_algorithm;
            const unsigned char* m_keyBuffer;
            unsigned int m_keyBufferSize;
            const xmltooling::Credential* m_credential;
            bool m_compact;
            
            friend class Encrypter;
        };
        
        /**
         * Structure to collect key wrapping/transport requirements.
         */
        struct XMLTOOL_API KeyEncryptionParams {
            
            /**
             * Constructor.
             * 
             * @param credential    a Credential supplying the key encryption key
             * @param algorithm     the XML Encryption key wrapping or transport algorithm constant
             * @param recipient     optional name of recipient of encrypted key
             */
            KeyEncryptionParams(
                const xmltooling::Credential& credential,
                const XMLCh* algorithm,
                const XMLCh* recipient=NULL
                ) : m_credential(credential), m_algorithm(algorithm), m_recipient(recipient) {
            }
        
            ~KeyEncryptionParams() {}
        private:
            const xmltooling::Credential& m_credential;
            const XMLCh* m_algorithm;
            const XMLCh* m_recipient;
            
            friend class Encrypter;
        };
    
        Encrypter() : m_cipher(NULL) {}

        ~Encrypter();
        
        /**
         * Encrypts the supplied element and returns the resulting object.
         * 
         * If an encryption algorithm is set, but no key, a random key will be
         * generated iff kencParams is non-NULL and the algorithm is known.
         * 
         * If key encryption parameters are supplied, then the encryption key
         * is wrapped and the result placed into an EncryptedKey object in the
         * KeyInfo of the returned EncryptedData.
         * 
         * @param element       the DOM element to encrypt
         * @param encParams     primary encryption settings
         * @param kencParams    key encryption settings, or NULL
         * @return a stand-alone EncryptedData object, unconnected to the source DOM 
         */
        EncryptedData* encryptElement(DOMElement* element, EncryptionParams& encParams, KeyEncryptionParams* kencParams=NULL);

        /**
         * Encrypts the supplied element's children and returns the resulting object.
         * 
         * If an encryption algorithm is set, but no key, a random key will be
         * generated iff kencParams is non-NULL and the algorithm is known.

         * If key encryption parameters are supplied, then the encryption key
         * is wrapped and the result placed into an EncryptedKey object in the
         * KeyInfo of the returned EncryptedData.
         * 
         * @param element       parent element of children to encrypt
         * @param encParams     primary encryption settings
         * @param kencParams    key encryption settings, or NULL
         * @return a stand-alone EncryptedData object, unconnected to the source DOM 
         */
        EncryptedData* encryptElementContent(DOMElement* element, EncryptionParams& encParams, KeyEncryptionParams* kencParams=NULL);

        /**
         * Encrypts the supplied input stream and returns the resulting object.
         * 
         * If an encryption algorithm is set, but no key, a random key will be
         * generated iff kencParams is non-NULL and the algorithm is known.

         * If key encryption parameters are supplied, then the encryption key
         * is wrapped and the result placed into an EncryptedKey object in the
         * KeyInfo of the returned EncryptedData.
         * 
         * @param input         the stream to encrypt
         * @param encParams     primary encryption settings
         * @param kencParams    key encryption settings, or NULL
         * @return a stand-alone EncryptedData object, unconnected to any DOM 
         */
        EncryptedData* encryptStream(std::istream& input, EncryptionParams& encParams, KeyEncryptionParams* kencParams=NULL);
        
        /**
         * Encrypts the supplied key and returns the resulting object.
         * 
         * @param keyBuffer     raw key material to encrypt
         * @param keyBufferSize size in bytes of raw key material
         * @param kencParams    key encryption settings
         * @param compact       true iff the encrypted representation should be made as small as possible
         * @return a stand-alone EncryptedKey object, unconnected to any DOM 
         */
        EncryptedKey* encryptKey(const unsigned char* keyBuffer, unsigned int keyBufferSize, KeyEncryptionParams& kencParams, bool compact=false);
        
    private:
        void checkParams(EncryptionParams& encParams, KeyEncryptionParams* kencParams);
        EncryptedData* decorateAndUnmarshall(EncryptionParams& encParams, KeyEncryptionParams* kencParams);
    
        XENCCipher* m_cipher;
        unsigned char m_keyBuffer[32];
    };

    DECL_XMLTOOLING_EXCEPTION(EncryptionException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmlencryption,xmltooling::XMLSecurityException,Exceptions in encryption processing);

};

#endif /* __xmltooling_encrypter_h__ */
