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
 * @file xmltooling/security/SecurityHelper.h
 *
 * A helper class for working with keys, certificates, etc.
 */

#if !defined(__xmltooling_sechelper_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sechelper_h__

#include <xmltooling/base.h>

#include <string>
#include <vector>

class XSECCryptoKey;
class XSECCryptoX509;

namespace xmltooling {
    class XMLTOOL_API Credential;
    class XMLTOOL_API SOAPTransport;
    class XMLTOOL_API XSECCryptoX509CRL;

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
        static XSECCryptoKey* loadKeyFromFile(const char* pathname, const char* format=nullptr, const char* password=nullptr);

        /**
         * Loads certificate(s) from a local file.
         *
         * @param certs     array to populate with certificate(s)
         * @param pathname  path to file containing certificate(s)
         * @param format    optional constant identifying certificate encoding format
         * @param password  optional password to decrypt certificate(s)
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509*>::size_type loadCertificatesFromFile(
            std::vector<XSECCryptoX509*>& certs, const char* pathname, const char* format=nullptr, const char* password=nullptr
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
            std::vector<XSECCryptoX509CRL*>& crls, const char* pathname, const char* format=nullptr
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
        static XSECCryptoKey* loadKeyFromURL(SOAPTransport& transport, const char* backing, const char* format=nullptr, const char* password=nullptr);

        /**
         * Loads certificate(s) from a URL.
         *
         * @param certs     array to populate with certificate(s)
         * @param transport object to use to acquire certificate(s)
         * @param backing   backing file for certificate(s) (written to or read from if download fails)
         * @param format    optional constant identifying certificate encoding format
         * @param password  optional password to decrypt certificate(s)
         * @return  size of the resulting array
         */
        static std::vector<XSECCryptoX509*>::size_type loadCertificatesFromURL(
            std::vector<XSECCryptoX509*>& certs, SOAPTransport& transport, const char* backing, const char* format=nullptr, const char* password=nullptr
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
            std::vector<XSECCryptoX509CRL*>& crls, SOAPTransport& transport, const char* backing, const char* format=nullptr
            );

        /**
         * Compares two keys for equality.
         *
         * @param key1 first key to compare
         * @param key2 second key to compare
         * @return  true iff the keys match
         */
        static bool matches(const XSECCryptoKey& key1, const XSECCryptoKey& key2);

        /**
         * Performs a hash operation over the supplied data.
         *
         * @param hashAlg   name of hash algorithm, syntax specific to crypto provider
         * @param buf       input data to hash
         * @param buflen    length of input data
         * @param toHex     if true, hex-encodes the resulting raw bytes
         * @return  result of hash operation, or an empty string
         */
        static std::string doHash(const char* hashAlg, const char* buf, unsigned long buflen, bool toHex=true);

        /**
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         * <p>If a hash algorithm is provided, the data is digested before being base64-encoded.
         *
         * @param cred      the credential containing the key to encode
         * @param hash      optional name of hash algorithm, syntax specific to crypto provider
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const Credential& cred, const char* hash, bool nowrap=true);

        /**
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         * <p>If a hash algorithm is provided, the data is digested before being base64-encoded.
         *
         * @param key       the key to encode
         * @param hash      optional name of hash algorithm, syntax specific to crypto provider
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const XSECCryptoKey& key, const char* hash, bool nowrap=true);

        /**
         * Returns the base64-encoded DER encoding of a certifiate's public key in SubjectPublicKeyInfo format.
         * <p>If a hash algorithm is provided, the data is digested before being base64-encoded.
         *
         * @param cert      the certificate's key to encode
         * @param hash      optional name of hash algorithm, syntax specific to crypto provider
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const XSECCryptoX509& cert, const char* hash, bool nowrap=true);

        /**
         * @deprecated
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         *
         * @param cred      the credential containing the key to encode
         * @param hash      if true, the DER encoded data is hashed with SHA-1 before base64 encoding
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const Credential& cred, bool hash=false, bool nowrap=true);

        /**
         * @deprecated
         * Returns the base64-encoded DER encoding of a public key in SubjectPublicKeyInfo format.
         *
         * @param key       the key to encode
         * @param hash      if true, the DER encoded data is hashed with SHA-1 before base64 encoding
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const XSECCryptoKey& key, bool hash=false, bool nowrap=true);

        /**
         * @deprecated
         * Returns the base64-encoded DER encoding of a certificate's public key in SubjectPublicKeyInfo format.
         *
         * @param cert      the certificate's key to encode
         * @param hash      if true, the DER encoded data is hashed with SHA-1 before base64 encoding
         * @param nowrap    if true, any linefeeds will be stripped from the result
         * @return  the base64 encoded key value
         */
        static std::string getDEREncoding(const XSECCryptoX509& cert, bool hash=false, bool nowrap=true);

        /**
         * Decodes a DER-encoded public key.
         *
         * @param buf       DER encoded data
         * @param buflen    length of data in bytes
         * @param base64    true iff DER is base64-encoded
         * @return  the decoded public key, or nullptr
         */
        static XSECCryptoKey* fromDEREncoding(const char* buf, unsigned long buflen, bool base64=true);

        /**
         * Decodes a base64-encoded and DER-encoded public key.
         *
         * @param buf       base64 and DER encoded data
         * @return  the decoded public key, or nullptr
         */
        static XSECCryptoKey* fromDEREncoding(const XMLCh* buf);
    };
};

#endif /* __xmltooling_sechelper_h__ */
