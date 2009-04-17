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
 * @file xmltooling/security/SecurityHelper.h
 *
 * A helper class for working with keys, certificates, etc.
 */

#if !defined(__xmltooling_sechelper_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sechelper_h__

#include <xmltooling/security/XSECCryptoX509CRL.h>
#include <xmltooling/soap/SOAPTransport.h>

#include <vector>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

namespace xmltooling {
    class XMLTOOL_API Credential;

    /**
     * A helper class for working with keys, certificates, etc.
     */
    class XMLTOOL_API SecurityHelper
    {
    public:
        /**
         * Access a file to try and guess the encoding format used.
         *
         * @param pathname  path to file
         * @return  constant identifying encoding format
         */
        static const char* guessEncodingFormat(const char* pathname);

        /**
         * Loads a private key from a local file.
         *
         * @param pathname  path to file containing key
         * @param format    optional constant identifying key encoding format
         * @param password  optional password to decrypt key
         * @return  a populated key object
         */
        static XSECCryptoKey* loadKeyFromFile(const char* pathname, const char* format=NULL, const char* password=NULL);

        /**
         * Loads certificate(s) from a local file.
         *
         * @param certs     array to populate with certificate(s)
         * @param pathname  path to file containing certificate(s)
         * @param format    optional constant identifying certificate encoding format
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509*>::size_type loadCertificatesFromFile(
            std::vector<XSECCryptoX509*>& certs, const char* pathname, const char* format=NULL, const char* password=NULL
            );

        /**
         * Loads CRL(s) from a local file.
         *
         * @param crls      array to populate with CRL(s)
         * @param pathname  path to file containing CRL(s)
         * @param format    optional constant identifying CRL encoding format
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509CRL*>::size_type loadCRLsFromFile(
            std::vector<XSECCryptoX509CRL*>& crls, const char* pathname, const char* format=NULL
            );

        /**
         * Loads a private key from a URL.
         *
         * @param transport object to use to acquire key
         * @param backing   backing file for key (written to or read from if download fails)
         * @param format    optional constant identifying key encoding format
         * @param password  optional password to decrypt key
         * @return  a populated key object
         */
        static XSECCryptoKey* loadKeyFromURL(SOAPTransport& transport, const char* backing, const char* format=NULL, const char* password=NULL);

        /**
         * Loads certificate(s) from a URL.
         *
         * @param certs     array to populate with certificate(s)
         * @param transport object to use to acquire certificate(s)
         * @param backing   backing file for certificate(s) (written to or read from if download fails)
         * @param format    optional constant identifying certificate encoding format
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509*>::size_type loadCertificatesFromURL(
            std::vector<XSECCryptoX509*>& certs, SOAPTransport& transport, const char* backing, const char* format=NULL, const char* password=NULL
            );

        /**
         * Loads CRL(s) from a URL.
         *
         * @param crls      array to populate with CRL(s)
         * @param transport object to use to acquire CRL(s)
         * @param backing   backing file for CRL(s) (written to or read from if download fails)
         * @param format    optional constant identifying CRL encoding format
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509CRL*>::size_type loadCRLsFromURL(
            std::vector<XSECCryptoX509CRL*>& crls, SOAPTransport& transport, const char* backing, const char* format=NULL
            );

        /**
         * Compares two keys for equality.
         *
         * @param key1 first key to compare
         * @param key2 second key to compare
         * @return  true iff the keys match
         */
        static bool matches(const XSECCryptoKey* key1, const XSECCryptoKey* key2);

        /**
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         *
         * @param key   the credential containing the key to encode
         * @return  the base64 encoded key value in a malloc'd string
         */
        static char* getDEREncoding(const Credential& cred);

        /**
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         *
         * @param key   the key to encode
         * @return  the base64 encoded key value in a malloc'd string
         */
        static char* getDEREncoding(const XSECCryptoKey& key);

        /**
         * Returns the base64-encoded DER encoding of a certifiate's public key in SubjectPublicKeyInfo format.
         *
         * @param cert   the certificate's key to encode
         * @return  the base64 encoded key value in a malloc'd string
         */
        static char* getDEREncoding(const XSECCryptoX509& cert);
    };
};

#endif /* __xmltooling_sechelper_h__ */
