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
 * BasicX509Credential.cpp
 *
 * Wraps an X.509-based Credential by storing key/cert objects inside.
 */

#include "internal.h"
#include "security/BasicX509Credential.h"
#include "security/KeyInfoCredentialContext.h"
#include "security/OpenSSLCredential.h"
#include "security/XSECCryptoX509CRL.h"
#include "signature/KeyInfo.h"

#include <algorithm>
#include <openssl/x509v3.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

Credential::Credential()
{
}

Credential::~Credential()
{
}

const CredentialContext* Credential::getCredentalContext() const
{
    return NULL;
}

X509Credential::X509Credential()
{
}

X509Credential::~X509Credential()
{
}

OpenSSLCredential::OpenSSLCredential()
{
}

OpenSSLCredential::~OpenSSLCredential()
{
}

CredentialContext::CredentialContext()
{
}

CredentialContext::~CredentialContext()
{
}

KeyInfoCredentialContext::KeyInfoCredentialContext(const KeyInfo* keyInfo) : m_keyInfo(keyInfo), m_nativeKeyInfo(NULL)
{
}

KeyInfoCredentialContext::KeyInfoCredentialContext(DSIGKeyInfoList* keyInfo) : m_keyInfo(NULL), m_nativeKeyInfo(keyInfo)
{
}

KeyInfoCredentialContext::~KeyInfoCredentialContext()
{
}

const KeyInfo* KeyInfoCredentialContext::getKeyInfo() const
{
    return m_keyInfo;
}

DSIGKeyInfoList* KeyInfoCredentialContext::getNativeKeyInfo() const
{
    return m_nativeKeyInfo;
}

BasicX509Credential::BasicX509Credential(bool ownCerts) : m_key(NULL), m_ownCerts(ownCerts), m_keyInfo(NULL), m_compactKeyInfo(NULL)
{
}

BasicX509Credential::BasicX509Credential(XSECCryptoKey* key, const vector<XSECCryptoX509*>& certs, XSECCryptoX509CRL* crl)
    : m_key(key), m_xseccerts(certs), m_ownCerts(true), m_keyInfo(NULL), m_compactKeyInfo(NULL)
{
    if (crl)
        m_crls.push_back(crl);
}

BasicX509Credential::BasicX509Credential(XSECCryptoKey* key, const vector<XSECCryptoX509*>& certs, const vector<XSECCryptoX509CRL*>& crls)
    : m_key(key), m_xseccerts(certs), m_ownCerts(true), m_crls(crls), m_keyInfo(NULL), m_compactKeyInfo(NULL)
{
}

BasicX509Credential::~BasicX509Credential()
{
    delete m_key;
    if (m_ownCerts)
        for_each(m_xseccerts.begin(), m_xseccerts.end(), xmltooling::cleanup<XSECCryptoX509>());
    for_each(m_crls.begin(), m_crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
    delete m_keyInfo;
    delete m_compactKeyInfo;
}

void BasicX509Credential::initKeyInfo(unsigned int types)
{
    delete m_keyInfo;
    m_keyInfo = NULL;
    delete m_compactKeyInfo;
    m_compactKeyInfo = NULL;

    if (types == 0)
        types = KEYINFO_KEY_VALUE | KEYINFO_KEY_NAME | KEYINFO_X509_CERTIFICATE | KEYINFO_X509_SUBJECTNAME | KEYINFO_X509_ISSUERSERIAL;

    if (types & KEYINFO_KEY_NAME) {
        const set<string>& names = getKeyNames();
        if (!names.empty()) {
            m_compactKeyInfo = KeyInfoBuilder::buildKeyInfo();
            VectorOf(KeyName) knames = m_compactKeyInfo->getKeyNames();
            for (set<string>::const_iterator n = names.begin(); n!=names.end(); ++n) {
                if (*n == m_subjectName)
                    continue;
                auto_ptr_XMLCh wide(n->c_str());
                KeyName* kname = KeyNameBuilder::buildKeyName();
                kname->setName(wide.get());
                knames.push_back(kname);
            }
        }
    }

    if (types & KEYINFO_X509_SUBJECTNAME || types & KEYINFO_X509_ISSUERSERIAL) {
        if (!m_subjectName.empty() || (!m_issuerName.empty() && !m_serial.empty())) {
            if (!m_compactKeyInfo)
                m_compactKeyInfo = KeyInfoBuilder::buildKeyInfo();
            X509Data* x509Data=X509DataBuilder::buildX509Data();
            m_compactKeyInfo->getX509Datas().push_back(x509Data);
            if (types & KEYINFO_X509_SUBJECTNAME && !m_subjectName.empty()) {
                X509SubjectName* sn = X509SubjectNameBuilder::buildX509SubjectName();
                auto_ptr_XMLCh wide(m_subjectName.c_str());
                sn->setName(wide.get());
                x509Data->getX509SubjectNames().push_back(sn);
            }

            if (types & KEYINFO_X509_ISSUERSERIAL && !m_issuerName.empty() && !m_serial.empty()) {
                X509IssuerSerial* is = X509IssuerSerialBuilder::buildX509IssuerSerial();
                X509IssuerName* in = X509IssuerNameBuilder::buildX509IssuerName();
                auto_ptr_XMLCh wide(m_issuerName.c_str());
                in->setName(wide.get());
                is->setX509IssuerName(in);
                X509SerialNumber* ser = X509SerialNumberBuilder::buildX509SerialNumber();
                auto_ptr_XMLCh wide2(m_serial.c_str());
                ser->setSerialNumber(wide2.get());
                is->setX509SerialNumber(ser);
                x509Data->getX509IssuerSerials().push_back(is);
            }
        }
    }

    if (types & KEYINFO_X509_CERTIFICATE && !m_xseccerts.empty()) {
        m_keyInfo = m_compactKeyInfo ? m_compactKeyInfo->cloneKeyInfo() : KeyInfoBuilder::buildKeyInfo();
        if (m_keyInfo->getX509Datas().empty())
            m_keyInfo->getX509Datas().push_back(X509DataBuilder::buildX509Data());
        for (vector<XSECCryptoX509*>::const_iterator x = m_xseccerts.begin(); x!=m_xseccerts.end(); ++x) {
            safeBuffer& buf=(*x)->getDEREncodingSB();
            X509Certificate* x509=X509CertificateBuilder::buildX509Certificate();
            x509->setValue(buf.sbStrToXMLCh());
            m_keyInfo->getX509Datas().front()->getX509Certificates().push_back(x509);
        }
    }
}

unsigned int BasicX509Credential::getUsage() const
{
    return UNSPECIFIED_CREDENTIAL;
}

const char* BasicX509Credential::getAlgorithm() const
{
    if (m_key) {
        switch (m_key->getKeyType()) {
            case XSECCryptoKey::KEY_RSA_PRIVATE:
            case XSECCryptoKey::KEY_RSA_PUBLIC:
            case XSECCryptoKey::KEY_RSA_PAIR:
                return "RSA";

            case XSECCryptoKey::KEY_DSA_PRIVATE:
            case XSECCryptoKey::KEY_DSA_PUBLIC:
            case XSECCryptoKey::KEY_DSA_PAIR:
                return "DSA";

            case XSECCryptoKey::KEY_HMAC:
                return "HMAC";

            case XSECCryptoKey::KEY_SYMMETRIC: {
                switch (static_cast<XSECCryptoSymmetricKey*>(m_key)->getSymmetricKeyType()) {
                    case XSECCryptoSymmetricKey::KEY_3DES_192:
                        return "DESede";
                    case XSECCryptoSymmetricKey::KEY_AES_128:
                        return "AES";
                    case XSECCryptoSymmetricKey::KEY_AES_192:
                        return "AES";
                    case XSECCryptoSymmetricKey::KEY_AES_256:
                        return "AES";
                }
            }
        }
    }
    return NULL;
}

unsigned int BasicX509Credential::getKeySize() const
{
    if (m_key) {
        switch (m_key->getKeyType()) {
            case XSECCryptoKey::KEY_RSA_PRIVATE:
            case XSECCryptoKey::KEY_RSA_PUBLIC:
            case XSECCryptoKey::KEY_RSA_PAIR: {
                XSECCryptoKeyRSA* rkey = static_cast<XSECCryptoKeyRSA*>(m_key);
                return rkey->getLength();
            }

            case XSECCryptoKey::KEY_SYMMETRIC: {
                switch (static_cast<XSECCryptoSymmetricKey*>(m_key)->getSymmetricKeyType()) {
                    case XSECCryptoSymmetricKey::KEY_3DES_192:
                        return 192;
                    case XSECCryptoSymmetricKey::KEY_AES_128:
                        return 128;
                    case XSECCryptoSymmetricKey::KEY_AES_192:
                        return 192;
                    case XSECCryptoSymmetricKey::KEY_AES_256:
                        return 256;
                }
            }
        }
    }
    return 0;
}

XSECCryptoKey* BasicX509Credential::getPrivateKey() const
{
    if (m_key) {
        XSECCryptoKey::KeyType type = m_key->getKeyType();
        if (type!=XSECCryptoKey::KEY_RSA_PUBLIC && type!=XSECCryptoKey::KEY_DSA_PUBLIC)
            return m_key;
    }
    return NULL;
}

XSECCryptoKey* BasicX509Credential::getPublicKey() const
{
    if (m_key) {
        XSECCryptoKey::KeyType type = m_key->getKeyType();
        if (type!=XSECCryptoKey::KEY_RSA_PRIVATE && type!=XSECCryptoKey::KEY_DSA_PRIVATE)
            return m_key;
    }
    return NULL;
}

const set<string>& BasicX509Credential::getKeyNames() const
{
    return m_keyNames;
}

KeyInfo* BasicX509Credential::getKeyInfo(bool compact) const
{
    if (compact || !m_keyInfo)
        return m_compactKeyInfo ? m_compactKeyInfo->cloneKeyInfo() : NULL;
    return m_keyInfo->cloneKeyInfo();
}

const vector<XSECCryptoX509*>& BasicX509Credential::getEntityCertificateChain() const
{
    return m_xseccerts;
}

XSECCryptoX509CRL* BasicX509Credential::getCRL() const
{
    return m_crls.empty() ? NULL : m_crls.front();
}

const vector<XSECCryptoX509CRL*>& BasicX509Credential::getCRLs() const
{
    return m_crls;
}

const char* BasicX509Credential::getSubjectName() const
{
    return m_subjectName.c_str();
}

const char* BasicX509Credential::getIssuerName() const
{
    return m_issuerName.c_str();
}

const char* BasicX509Credential::getSerialNumber() const
{
    return m_serial.c_str();
}

void BasicX509Credential::extract()
{
    XSECCryptoX509* x509 = m_xseccerts.empty() ? NULL : m_xseccerts.front();
    if (!x509 || x509->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL)
        return;
    X509* cert = static_cast<OpenSSLCryptoX509*>(x509)->getOpenSSLX509();
    if (!cert)
        return;

    X509_NAME* issuer=X509_get_issuer_name(cert);
    if (issuer) {
        BIO* b = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(b,issuer,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        BUF_MEM* bptr=NULL;
        BIO_get_mem_ptr(b, &bptr);
        m_issuerName.erase();
        m_issuerName.append(bptr->data, bptr->length);
        BIO_free(b);
    }

    ASN1_INTEGER* serialASN = X509_get_serialNumber(cert);
    BIGNUM* serialBN = ASN1_INTEGER_to_BN(serialASN, NULL);
    if (serialBN) {
        char* serial = BN_bn2dec(serialBN);
        if (serial) {
            m_serial = serial;
            free(serial);
        }
        BN_free(serialBN);
    }

    X509_NAME* subject=X509_get_subject_name(cert);
    if (subject) {
        BIO* b = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(b,subject,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        BUF_MEM* bptr=NULL;
        BIO_get_mem_ptr(b, &bptr);
        m_subjectName.erase();
        m_subjectName.append(bptr->data, bptr->length);
        m_keyNames.insert(m_subjectName);
        BIO_free(b);
        
        // Fetch the last CN RDN.
        char* peer_CN = NULL;
        int j,i = -1;
        while ((j=X509_NAME_get_index_by_NID(subject, NID_commonName, i)) >= 0)
            i = j;
        if (i >= 0) {
            ASN1_STRING* tmp = X509_NAME_ENTRY_get_data(X509_NAME_get_entry(subject, i));
            // Copied in from libcurl.
            /* In OpenSSL 0.9.7d and earlier, ASN1_STRING_to_UTF8 fails if the input
               is already UTF-8 encoded. We check for this case and copy the raw
               string manually to avoid the problem. */
            if(tmp && ASN1_STRING_type(tmp) == V_ASN1_UTF8STRING) {
                j = ASN1_STRING_length(tmp);
                if(j >= 0) {
                    peer_CN = (char*)OPENSSL_malloc(j + 1);
                    memcpy(peer_CN, ASN1_STRING_data(tmp), j);
                    peer_CN[j] = '\0';
                }
            }
            else /* not a UTF8 name */ {
                j = ASN1_STRING_to_UTF8(reinterpret_cast<unsigned char**>(&peer_CN), tmp);
            }
            
            if (j > 0)
                m_keyNames.insert(string(peer_CN, j));
            if(peer_CN)
                OPENSSL_free(peer_CN);
        }

        STACK_OF(GENERAL_NAME)* altnames=(STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
        if (altnames) {
            int numalts = sk_GENERAL_NAME_num(altnames);
            for (int an=0; an<numalts; an++) {
                const GENERAL_NAME* check = sk_GENERAL_NAME_value(altnames, an);
                if (check->type==GEN_DNS || check->type==GEN_URI) {
                    const char* altptr = (char*)ASN1_STRING_data(check->d.ia5);
                    const int altlen = ASN1_STRING_length(check->d.ia5);
                    if (altlen > 0)
                        m_keyNames.insert(string(altptr, altlen));
                }
            }
        }
        GENERAL_NAMES_free(altnames);
    }
}
