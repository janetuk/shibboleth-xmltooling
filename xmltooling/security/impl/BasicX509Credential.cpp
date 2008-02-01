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
 * BasicX509Credential.cpp
 * 
 * Wraps an X.509-based Credential by storing key/cert objects inside. 
 */

#include "internal.h"
#include "security/BasicX509Credential.h"
#include "signature/KeyInfo.h"

#include <algorithm>
#include <openssl/x509v3.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

BasicX509Credential::~BasicX509Credential()
{
    delete m_key;
    if (m_ownCerts)
        for_each(m_xseccerts.begin(), m_xseccerts.end(), xmltooling::cleanup<XSECCryptoX509>());
    delete m_crl;
    delete m_keyInfo;
    delete m_compactKeyInfo;
}

void BasicX509Credential::initKeyInfo()
{
    delete m_keyInfo;
    m_keyInfo = NULL;
    delete m_compactKeyInfo;
    m_compactKeyInfo = NULL;

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

    if (!m_subjectName.empty() || (!m_issuerName.empty() && m_serial >= 0)) {
        if (!m_compactKeyInfo)
            m_compactKeyInfo = KeyInfoBuilder::buildKeyInfo();
        X509Data* x509Data=X509DataBuilder::buildX509Data();
        m_compactKeyInfo->getX509Datas().push_back(x509Data);
        if (!m_subjectName.empty()) {
            X509SubjectName* sn = X509SubjectNameBuilder::buildX509SubjectName();
            auto_ptr_XMLCh wide(m_subjectName.c_str());
            sn->setName(wide.get());
            x509Data->getX509SubjectNames().push_back(sn);
        }
        
        if (!m_issuerName.empty() && m_serial >= 0) {
            X509IssuerSerial* is = X509IssuerSerialBuilder::buildX509IssuerSerial();
            X509IssuerName* in = X509IssuerNameBuilder::buildX509IssuerName();
            auto_ptr_XMLCh wide(m_issuerName.c_str());
            in->setName(wide.get());
            is->setX509IssuerName(in);
            X509SerialNumber* ser = X509SerialNumberBuilder::buildX509SerialNumber();
            char buf[64];
            sprintf(buf,"%d",m_serial);
            auto_ptr_XMLCh wide2(buf);
            ser->setSerialNumber(wide2.get());
            is->setX509SerialNumber(ser);
            x509Data->getX509IssuerSerials().push_back(is);
        }
    }
    
    if (!m_xseccerts.empty()) {
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

void BasicX509Credential::extract()
{
    XSECCryptoX509* x509 = m_xseccerts.empty() ? NULL : m_xseccerts.front();
    if (!x509 || x509->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL)
        return;
    X509* cert = static_cast<OpenSSLCryptoX509*>(x509)->getOpenSSLX509();
    if (!cert)
        return;

    BIO* b;
    int len;
    char buf[256];

    X509_NAME* issuer=X509_get_issuer_name(cert);
    if (issuer) {
        memset(buf,0,sizeof(buf));
        b = BIO_new(BIO_s_mem());
        BIO_set_mem_eof_return(b, 0);
        len=X509_NAME_print_ex(b,issuer,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        m_issuerName.erase();
        while ((len = BIO_read(b, buf, 255)) > 0) {
            buf[len] = '\0';
            m_issuerName+=buf;
        }
        BIO_free(b);
    }

    ASN1_INTEGER* serialASN = X509_get_serialNumber(cert);
    BIGNUM* serialBN = ASN1_INTEGER_to_BN(serialASN, NULL);
    if (serialBN) {
        char* serial = BN_bn2dec(serialBN);
        if (serial) {
            m_serial = atoi(serial);
            free(serial);
        }
        BN_free(serialBN);
    }
    
    X509_NAME* subject=X509_get_subject_name(cert);
    if (subject) {
        memset(buf,0,sizeof(buf));
        b = BIO_new(BIO_s_mem());
        BIO_set_mem_eof_return(b, 0);
        len=X509_NAME_print_ex(b,subject,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        m_subjectName.erase();
        while ((len = BIO_read(b, buf, 255)) > 0) {
            buf[len] = '\0';
            m_subjectName+=buf;
        }
        m_keyNames.insert(m_subjectName);
        BIO_free(b);

        memset(buf,0,sizeof(buf));
        if (X509_NAME_get_text_by_NID(subject,NID_commonName,buf,255)>0)
            m_keyNames.insert(buf);

        STACK_OF(GENERAL_NAME)* altnames=(STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
        if (altnames) {
            string alt;
            int numalts = sk_GENERAL_NAME_num(altnames);
            for (int an=0; an<numalts; an++) {
                const GENERAL_NAME* check = sk_GENERAL_NAME_value(altnames, an);
                if (check->type==GEN_DNS || check->type==GEN_URI) {
                    const char* altptr = (char*)ASN1_STRING_data(check->d.ia5);
                    const int altlen = ASN1_STRING_length(check->d.ia5);
                    if (altlen>0) {
                        alt.erase();
                        alt.append(altptr,altlen);
                        m_keyNames.insert(alt);
                    }
                }
            }
        }
        GENERAL_NAMES_free(altnames);
    }
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
