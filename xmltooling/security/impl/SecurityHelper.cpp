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
 * SecurityHelper.cpp
 *
 * A helper class for working with keys, certificates, etc.
 */

#include "internal.h"
#include "logging.h"
#include "io/HTTPResponse.h"
#include "security/OpenSSLCryptoX509CRL.h"
#include "security/SecurityHelper.h"
#include "security/X509Credential.h"
#include "soap/HTTPSOAPTransport.h"
#include "util/NDC.h"

#include <fstream>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

// OpenSSL password callback...
static int passwd_callback(char* buf, int len, int verify, void* passwd)
{
    if(!verify)
    {
        if(passwd && len > strlen(reinterpret_cast<char*>(passwd)))
        {
            strcpy(buf,reinterpret_cast<char*>(passwd));
            return strlen(buf);
        }
    }
    return 0;
}

const char* SecurityHelper::guessEncodingFormat(const char* pathname)
{
    const char* format=NULL;
    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in, pathname)>0) {
        const int READSIZE = 1;
        char buf[READSIZE];
        int mark;

        // Examine the first byte.
        try {
            if ((mark = BIO_tell(in)) < 0)
                throw XMLSecurityException("Error loading file: BIO_tell() can't get the file position.");
            if (BIO_read(in, buf, READSIZE) <= 0)
                throw XMLSecurityException("Error loading file: BIO_read() can't read from the stream.");
            if (BIO_seek(in, mark) < 0)
                throw XMLSecurityException("Error loading file: BIO_seek() can't reset the file position.");
        }
        catch (exception&) {
            log_openssl();
            BIO_free(in);
            throw;
        }

        // Check the first byte of the file.  If it's some kind of DER-encoded structure
        // (including PKCS12), it will begin with ASCII 048. Otherwise, assume it's PEM.
        if (buf[0] != 48) {
            format = "PEM";
        }
        else {
            // Here we know it's DER-encoded, now try to parse it as a PKCS12 ASN.1 structure.
            // If it fails, must be another kind of DER-encoded structure.
            PKCS12* p12;
            if ((p12=d2i_PKCS12_bio(in, NULL)) == NULL) {
                format = "DER";
            }
            else {
                format = "PKCS12";
                PKCS12_free(p12);
            }
        }
    }
    if (in)
        BIO_free(in);
    if (format)
        return format;
    throw XMLSecurityException("Unable to determine encoding for file ($1).", params(1,pathname));
}

XSECCryptoKey* SecurityHelper::loadKeyFromFile(const char* pathname, const char* format, const char* password)
{
#ifdef _DEBUG
    NDC ndc("loadKeyFromFile");
#endif
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper");
    log.info("loading private key from file (%s)", pathname);

    // Native objects.
    PKCS12* p12=NULL;
    EVP_PKEY* pkey=NULL;

    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in, pathname)>0) {
        // If the format isn't set, try and guess it.
        if (!format || !*format) {
            const int READSIZE = 1;
            char buf[READSIZE];
            int mark;

            // Examine the first byte.
            try {
                if ((mark = BIO_tell(in)) < 0)
                    throw XMLSecurityException("Error loading key: BIO_tell() can't get the file position.");
                if (BIO_read(in, buf, READSIZE) <= 0)
                    throw XMLSecurityException("Error loading key: BIO_read() can't read from the stream.");
                if (BIO_seek(in, mark) < 0)
                    throw XMLSecurityException("Error loading key: BIO_seek() can't reset the file position.");
            }
            catch (exception&) {
                log_openssl();
                BIO_free(in);
                throw;
            }

            // Check the first byte of the file.  If it's some kind of DER-encoded structure
            // (including PKCS12), it will begin with ASCII 048. Otherwise, assume it's PEM.
            if (buf[0] != 48) {
                format = "PEM";
            }
            else {
                // Here we know it's DER-encoded, now try to parse it as a PKCS12 ASN.1 structure.
                // If it fails, must be another kind of DER-encoded structure.
                if ((p12=d2i_PKCS12_bio(in, NULL)) == NULL) {
                    format = "DER";
                    if (BIO_seek(in, mark) < 0) {
                        log_openssl();
                        BIO_free(in);
                        throw XMLSecurityException("Error loading key: BIO_seek() can't reset the file position.");
                    }
                }
                else {
                    format = "PKCS12";
                }
            }
            log.debug("key encoding format for (%s) dynamically resolved as (%s)", pathname, format);
        }

        // The format should be known, so parse accordingly.
        if (!strcmp(format, "PEM")) {
            pkey = PEM_read_bio_PrivateKey(in, NULL, passwd_callback, const_cast<char*>(password));
        }
        else if (!strcmp(format, "DER")) {
            pkey=d2i_PrivateKey_bio(in, NULL);
        }
        else if (!strcmp(format, "PKCS12")) {
            if (!p12)
                p12 = d2i_PKCS12_bio(in, NULL);
            if (p12) {
                X509* x=NULL;
                PKCS12_parse(p12, const_cast<char*>(password), &pkey, &x, NULL);
                PKCS12_free(p12);
                X509_free(x);
            }
        }
        else {
            log.error("unknown key encoding format (%s)", format);
        }
    }
    if (in)
        BIO_free(in);

    // Now map it to an XSEC wrapper.
    if (pkey) {
        XSECCryptoKey* ret=NULL;
        switch (pkey->type) {
            case EVP_PKEY_RSA:
                ret=new OpenSSLCryptoKeyRSA(pkey);
                break;

            case EVP_PKEY_DSA:
                ret=new OpenSSLCryptoKeyDSA(pkey);
                break;

            default:
                log.error("unsupported private key type");
        }
        EVP_PKEY_free(pkey);
        if (ret)
            return ret;
    }

    log_openssl();
    throw XMLSecurityException("Unable to load private key from file ($1).", params(1, pathname));
}

vector<XSECCryptoX509*>::size_type SecurityHelper::loadCertificatesFromFile(
    vector<XSECCryptoX509*>& certs, const char* pathname, const char* format, const char* password
    )
{
#ifdef _DEBUG
    NDC ndc("loadCertificatesFromFile");
#endif
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper");
    log.info("loading certificate(s) from file (%s)", pathname);

    vector<XSECCryptoX509*>::size_type count = certs.size();

    // Native objects.
    X509* x=NULL;
    PKCS12* p12=NULL;

    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in, pathname)>0) {
        // If the format isn't set, try and guess it.
        if (!format || !*format) {
            const int READSIZE = 1;
            char buf[READSIZE];
            int mark;

            // Examine the first byte.
            try {
                if ((mark = BIO_tell(in)) < 0)
                    throw XMLSecurityException("Error loading certificate: BIO_tell() can't get the file position.");
                if (BIO_read(in, buf, READSIZE) <= 0)
                    throw XMLSecurityException("Error loading certificate: BIO_read() can't read from the stream.");
                if (BIO_seek(in, mark) < 0)
                    throw XMLSecurityException("Error loading certificate: BIO_seek() can't reset the file position.");
            }
            catch (exception&) {
                log_openssl();
                BIO_free(in);
                throw;
            }

            // Check the first byte of the file.  If it's some kind of DER-encoded structure
            // (including PKCS12), it will begin with ASCII 048. Otherwise, assume it's PEM.
            if (buf[0] != 48) {
                format = "PEM";
            }
            else {
                // Here we know it's DER-encoded, now try to parse it as a PKCS12 ASN.1 structure.
                // If it fails, must be another kind of DER-encoded structure.
                if ((p12=d2i_PKCS12_bio(in, NULL)) == NULL) {
                    format = "DER";
                    if (BIO_seek(in, mark) < 0) {
                        log_openssl();
                        BIO_free(in);
                        throw XMLSecurityException("Error loading certificate: BIO_seek() can't reset the file position.");
                    }
                }
                else {
                    format = "PKCS12";
                }
            }
        }

        // The format should be known, so parse accordingly.
        if (!strcmp(format, "PEM")) {
            while (x=PEM_read_bio_X509(in, NULL, NULL, NULL)) {
                certs.push_back(new OpenSSLCryptoX509(x));
                X509_free(x);
            }
        }
        else if (!strcmp(format, "DER")) {
            x=d2i_X509_bio(in, NULL);
            if (x) {
                certs.push_back(new OpenSSLCryptoX509(x));
                X509_free(x);
            }
        }
        else if (!strcmp(format, "PKCS12")) {
            if (!p12)
                p12 = d2i_PKCS12_bio(in, NULL);
            if (p12) {
                EVP_PKEY* pkey=NULL;
                STACK_OF(X509)* CAstack = sk_X509_new_null();
                PKCS12_parse(p12, const_cast<char*>(password), &pkey, &x, &CAstack);
                PKCS12_free(p12);
                EVP_PKEY_free(pkey);
                if (x) {
                    certs.push_back(new OpenSSLCryptoX509(x));
                    X509_free(x);
                }
                x = sk_X509_pop(CAstack);
                while (x) {
                    certs.push_back(new OpenSSLCryptoX509(x));
                    X509_free(x);
                    x = sk_X509_pop(CAstack);
                }
                sk_X509_free(CAstack);
            }
        }
    }
    if (in)
        BIO_free(in);

    if (certs.size() == count) {
        log_openssl();
        throw XMLSecurityException("Unable to load certificate(s) from file ($1).", params(1, pathname));
    }

    return certs.size();
}

vector<XSECCryptoX509CRL*>::size_type SecurityHelper::loadCRLsFromFile(
    vector<XSECCryptoX509CRL*>& crls, const char* pathname, const char* format
    )
{
#ifdef _DEBUG
    NDC ndc("loadCRLsFromFile");
#endif
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper");
    log.info("loading CRL(s) from file (%s)", pathname);

    vector<XSECCryptoX509CRL*>::size_type count = crls.size();

    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in, pathname)>0) {
        // If the format isn't set, try and guess it.
        if (!format || !*format) {
            const int READSIZE = 1;
            char buf[READSIZE];
            int mark;

            // Examine the first byte.
            try {
                if ((mark = BIO_tell(in)) < 0)
                    throw XMLSecurityException("Error loading CRL: BIO_tell() can't get the file position.");
                if (BIO_read(in, buf, READSIZE) <= 0)
                    throw XMLSecurityException("Error loading CRL: BIO_read() can't read from the stream.");
                if (BIO_seek(in, mark) < 0)
                    throw XMLSecurityException("Error loading CRL: BIO_seek() can't reset the file position.");
            }
            catch (exception&) {
                log_openssl();
                BIO_free(in);
                throw;
            }

            // Check the first byte of the file.  If it's some kind of DER-encoded structure
            // it will begin with ASCII 048. Otherwise, assume it's PEM.
            if (buf[0] != 48) {
                format = "PEM";
            }
            else {
                format = "DER";
            }
            log.debug("CRL encoding format for (%s) dynamically resolved as (%s)", pathname, format);
        }

        X509_CRL* crl=NULL;
        if (!strcmp(format, "PEM")) {
            while (crl=PEM_read_bio_X509_CRL(in, NULL, NULL, NULL)) {
                crls.push_back(new OpenSSLCryptoX509CRL(crl));
                X509_CRL_free(crl);
            }
        }
        else if (!strcmp(format, "DER")) {
            crl=d2i_X509_CRL_bio(in, NULL);
            if (crl) {
                crls.push_back(new OpenSSLCryptoX509CRL(crl));
                X509_CRL_free(crl);
            }
        }
        else {
            log.error("unknown CRL encoding format (%s)", format);
        }
    }
    if (in)
        BIO_free(in);

    if (crls.size() == count) {
        log_openssl();
        throw XMLSecurityException("Unable to load CRL(s) from file ($1).", params(1, pathname));
    }

    return crls.size();
}

XSECCryptoKey* SecurityHelper::loadKeyFromURL(SOAPTransport& transport, const char* backing, const char* format, const char* password)
{
    // Fetch the data.
    transport.send();
    istream& msg = transport.receive();

    // Check for "not modified" status.
    if (dynamic_cast<HTTPSOAPTransport*>(&transport) && transport.getStatusCode() == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED)
        throw (long)HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED;

    // Dump to output file.
    ofstream out(backing, fstream::trunc|fstream::binary);
    out << msg.rdbuf();
    out.close();

    return loadKeyFromFile(backing, format, password);
}

vector<XSECCryptoX509*>::size_type SecurityHelper::loadCertificatesFromURL(
    vector<XSECCryptoX509*>& certs, SOAPTransport& transport, const char* backing, const char* format, const char* password
    )
{
    transport.send();
    istream& msg = transport.receive();

    // Check for "not modified" status.
    if (dynamic_cast<HTTPSOAPTransport*>(&transport) && transport.getStatusCode() == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED)
        throw (long)HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED;

    // Dump to output file.
    ofstream out(backing, fstream::trunc|fstream::binary);
    out << msg.rdbuf();
    out.close();

    return loadCertificatesFromFile(certs, backing, format, password);
}

vector<XSECCryptoX509CRL*>::size_type SecurityHelper::loadCRLsFromURL(
    vector<XSECCryptoX509CRL*>& crls, SOAPTransport& transport, const char* backing, const char* format
    )
{
    // Fetch the data.
    transport.send();
    istream& msg = transport.receive();

    // Check for "not modified" status.
    if (dynamic_cast<HTTPSOAPTransport*>(&transport) && transport.getStatusCode() == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED)
        throw (long)HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED;

    // Dump to output file.
    ofstream out(backing, fstream::trunc|fstream::binary);
    out << msg.rdbuf();
    out.close();

    return loadCRLsFromFile(crls, backing, format);
}

bool SecurityHelper::matches(const XSECCryptoKey& key1, const XSECCryptoKey& key2)
{
    if (key1.getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL ||
        key2.getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("comparison of non-OpenSSL keys not supported");
        return false;
    }

    // If one key is public or both, just compare the public key half.
    if (key1.getKeyType()==XSECCryptoKey::KEY_RSA_PUBLIC || key1.getKeyType()==XSECCryptoKey::KEY_RSA_PAIR) {
        if (key2.getKeyType()!=XSECCryptoKey::KEY_RSA_PUBLIC && key2.getKeyType()!=XSECCryptoKey::KEY_RSA_PAIR)
            return false;
        const RSA* rsa1 = static_cast<const OpenSSLCryptoKeyRSA&>(key1).getOpenSSLRSA();
        const RSA* rsa2 = static_cast<const OpenSSLCryptoKeyRSA&>(key2).getOpenSSLRSA();
        return (rsa1 && rsa2 && BN_cmp(rsa1->n,rsa2->n) == 0 && BN_cmp(rsa1->e,rsa2->e) == 0);
    }

    // For a private key, compare the private half.
    if (key1.getKeyType()==XSECCryptoKey::KEY_RSA_PRIVATE) {
        if (key2.getKeyType()!=XSECCryptoKey::KEY_RSA_PRIVATE && key2.getKeyType()!=XSECCryptoKey::KEY_RSA_PAIR)
            return false;
        const RSA* rsa1 = static_cast<const OpenSSLCryptoKeyRSA&>(key1).getOpenSSLRSA();
        const RSA* rsa2 = static_cast<const OpenSSLCryptoKeyRSA&>(key2).getOpenSSLRSA();
        return (rsa1 && rsa2 && BN_cmp(rsa1->n,rsa2->n) == 0 && BN_cmp(rsa1->d,rsa2->d) == 0);
    }

    // If one key is public or both, just compare the public key half.
    if (key1.getKeyType()==XSECCryptoKey::KEY_DSA_PUBLIC || key1.getKeyType()==XSECCryptoKey::KEY_DSA_PAIR) {
        if (key2.getKeyType()!=XSECCryptoKey::KEY_DSA_PUBLIC && key2.getKeyType()!=XSECCryptoKey::KEY_DSA_PAIR)
            return false;
        const DSA* dsa1 = static_cast<const OpenSSLCryptoKeyDSA&>(key1).getOpenSSLDSA();
        const DSA* dsa2 = static_cast<const OpenSSLCryptoKeyDSA&>(key2).getOpenSSLDSA();
        return (dsa1 && dsa2 && BN_cmp(dsa1->pub_key,dsa2->pub_key) == 0);
    }

    // For a private key, compare the private half.
    if (key1.getKeyType()==XSECCryptoKey::KEY_DSA_PRIVATE) {
        if (key2.getKeyType()!=XSECCryptoKey::KEY_DSA_PRIVATE && key2.getKeyType()!=XSECCryptoKey::KEY_DSA_PAIR)
            return false;
        const DSA* dsa1 = static_cast<const OpenSSLCryptoKeyDSA&>(key1).getOpenSSLDSA();
        const DSA* dsa2 = static_cast<const OpenSSLCryptoKeyDSA&>(key2).getOpenSSLDSA();
        return (dsa1 && dsa2 && BN_cmp(dsa1->priv_key,dsa2->priv_key) == 0);
    }

    Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("unsupported key type for comparison");
    return false;
}

string SecurityHelper::doHash(const char* hashAlg, const char* buf, unsigned long buflen, bool toHex)
{
    static char DIGITS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    string ret;

    const EVP_MD* md = EVP_get_digestbyname(hashAlg);
    if (!md) {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").error("hash algorithm (%s) not available", hashAlg);
        return ret;
    }

    BIO* chain = BIO_new(BIO_s_mem());
    BIO* b = BIO_new(BIO_f_md());
    BIO_set_md(b, md);
    chain = BIO_push(b, chain);
    BIO_write(chain, buf, buflen);
    BIO_flush(chain);

    char digest[EVP_MAX_MD_SIZE];
    int len = BIO_gets(chain, digest, EVP_MD_size(md));
    BIO_free_all(chain);
    if (len != EVP_MD_size(md)) {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").error(
            "hash result length (%d) did not match expected value (%d)", len, EVP_MD_size(md)
            );
        return ret;
    }
    if (toHex) {
        for (int i=0; i < len; ++i) {
            ret += (DIGITS[((unsigned char)(0xF0 & digest[i])) >> 4 ]);
            ret += (DIGITS[0x0F & digest[i]]);
        }
    }
    else {
        for (int i=0; i < len; ++i) {
            ret += digest[i];
        }
    }
    return ret;
}

string SecurityHelper::getDEREncoding(const XSECCryptoKey& key, const char* hash, bool nowrap)
{
    string ret;

    if (key.getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("encoding of non-OpenSSL keys not supported");
        return ret;
    }

    if (key.getKeyType() == XSECCryptoKey::KEY_RSA_PUBLIC || key.getKeyType() == XSECCryptoKey::KEY_RSA_PAIR) {
        const RSA* rsa = static_cast<const OpenSSLCryptoKeyRSA&>(key).getOpenSSLRSA();
        if (!rsa) {
            Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("key was not populated");
            return ret;
        }
        const EVP_MD* md=NULL;
        if (hash) {
            md = EVP_get_digestbyname(hash);
            if (!md) {
                Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").error("hash algorithm (%s) not available", hash);
                return ret;
            }
        }
        BIO* chain = BIO_new(BIO_s_mem());
        BIO* b = BIO_new(BIO_f_base64());
        if (nowrap)
            BIO_set_flags(b, BIO_FLAGS_BASE64_NO_NL);
        chain = BIO_push(b, chain);
        if (md) {
            b = BIO_new(BIO_f_md());
            BIO_set_md(b, md);
            chain = BIO_push(b, chain);
        }
        i2d_RSA_PUBKEY_bio(chain, const_cast<RSA*>(rsa));
        BIO_flush(chain);
        if (md) {
            char digest[EVP_MAX_MD_SIZE];
            int len = BIO_gets(chain, digest, EVP_MD_size(md));
            if (len != EVP_MD_size(md)) {
                BIO_free_all(chain);
                return ret;
            }
            b = BIO_pop(chain);
            BIO_free(chain);
            chain = b;
            BIO_reset(chain);
            BIO_write(chain, digest, len);
            BIO_flush(chain);
        }
        BUF_MEM* bptr=NULL;
        BIO_get_mem_ptr(chain, &bptr);
        if (bptr && bptr->length > 0)
            ret.append(bptr->data, bptr->length);
        BIO_free_all(chain);
    }
    else if (key.getKeyType() == XSECCryptoKey::KEY_DSA_PUBLIC || key.getKeyType() == XSECCryptoKey::KEY_DSA_PAIR) {
        const DSA* dsa = static_cast<const OpenSSLCryptoKeyDSA&>(key).getOpenSSLDSA();
        if (!dsa) {
            Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("key was not populated");
            return ret;
        }
        const EVP_MD* md=NULL;
        if (hash) {
            md = EVP_get_digestbyname(hash);
            if (!md) {
                Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").error("hash algorithm (%s) not available", hash);
                return ret;
            }
        }
        BIO* chain = BIO_new(BIO_s_mem());
        BIO* b = BIO_new(BIO_f_base64());
        if (nowrap)
            BIO_set_flags(b, BIO_FLAGS_BASE64_NO_NL);
        chain = BIO_push(b, chain);
        if (md) {
            b = BIO_new(BIO_f_md());
            BIO_set_md(b, md);
            chain = BIO_push(b, chain);
        }
        i2d_DSA_PUBKEY_bio(chain, const_cast<DSA*>(dsa));
        BIO_flush(chain);
        if (md) {
            char digest[EVP_MAX_MD_SIZE];
            int len = BIO_gets(chain, digest, EVP_MD_size(md));
            if (len != EVP_MD_size(md)) {
                BIO_free_all(chain);
                return ret;
            }
            b = BIO_pop(chain);
            BIO_free(chain);
            chain = b;
            BIO_reset(chain);
            BIO_write(chain, digest, len);
            BIO_flush(chain);
        }
        BUF_MEM* bptr=NULL;
        BIO_get_mem_ptr(chain, &bptr);
        if (bptr && bptr->length > 0)
            ret.append(bptr->data, bptr->length);
        BIO_free_all(chain);
    }
    else {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("encoding of non-RSA/DSA public keys not supported");
    }
    return ret;
}

string SecurityHelper::getDEREncoding(const XSECCryptoX509& cert, const char* hash, bool nowrap)
{
    string ret;

    if (cert.getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").warn("encoding of non-OpenSSL keys not supported");
        return ret;
    }

    const EVP_MD* md=NULL;
    if (hash) {
        md = EVP_get_digestbyname(hash);
        if (!md) {
            Category::getInstance(XMLTOOLING_LOGCAT".SecurityHelper").error("hash algorithm (%s) not available", hash);
            return ret;
        }
    }

    const X509* x = static_cast<const OpenSSLCryptoX509&>(cert).getOpenSSLX509();
    EVP_PKEY* key = X509_get_pubkey(const_cast<X509*>(x));

    BIO* chain = BIO_new(BIO_s_mem());
    BIO* b = BIO_new(BIO_f_base64());
    if (nowrap)
        BIO_set_flags(b, BIO_FLAGS_BASE64_NO_NL);
    chain = BIO_push(b, chain);
    if (md) {
        b = BIO_new(BIO_f_md());
        BIO_set_md(b, md);
        chain = BIO_push(b, chain);
    }
    i2d_PUBKEY_bio(chain, key);
    EVP_PKEY_free(key);
    BIO_flush(chain);
    if (md) {
        char digest[EVP_MAX_MD_SIZE];
        int len = BIO_gets(chain, digest, EVP_MD_size(md));
        if (len != EVP_MD_size(md)) {
            BIO_free_all(chain);
            return ret;
        }
        b = BIO_pop(chain);
        BIO_free(chain);
        chain = b;
        BIO_reset(chain);
        BIO_write(chain, digest, len);
        BIO_flush(chain);
    }
    BUF_MEM* bptr=NULL;
    BIO_get_mem_ptr(chain, &bptr);
    if (bptr && bptr->length > 0)
        ret.append(bptr->data, bptr->length);
    BIO_free_all(chain);
    return ret;
}

string SecurityHelper::getDEREncoding(const Credential& cred, const char* hash, bool nowrap)
{
    const X509Credential* x509 = dynamic_cast<const X509Credential*>(&cred);
    if (x509 && !x509->getEntityCertificateChain().empty())
        return getDEREncoding(*(x509->getEntityCertificateChain().front()), hash, nowrap);
    else if (cred.getPublicKey())
        return getDEREncoding(*(cred.getPublicKey()), hash, nowrap);
    return "";
}

string SecurityHelper::getDEREncoding(const XSECCryptoKey& key, bool hash, bool nowrap)
{
    return getDEREncoding(key, hash ? "SHA1" : NULL, nowrap);
}

string SecurityHelper::getDEREncoding(const XSECCryptoX509& cert, bool hash, bool nowrap)
{
    return getDEREncoding(cert, hash ? "SHA1" : NULL, nowrap);
}

string SecurityHelper::getDEREncoding(const Credential& cred, bool hash, bool nowrap)
{
    return getDEREncoding(cred, hash ? "SHA1" : NULL, nowrap);
}
