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

    vector<string> names;
    if (getKeyNames(names)>0) {
        m_compactKeyInfo = KeyInfoBuilder::buildKeyInfo();
        VectorOf(KeyName) knames = m_compactKeyInfo->getKeyNames();
        for (vector<string>::const_iterator n = names.begin(); n!=names.end(); ++n) {
            xmltooling::auto_ptr_XMLCh wide(n->c_str());
            KeyName* kname = KeyNameBuilder::buildKeyName();
            kname->setName(wide.get());
            knames.push_back(kname);
        }
    }
    
    if (!m_xseccerts.empty()) {
        m_keyInfo = m_compactKeyInfo ? m_compactKeyInfo->cloneKeyInfo() : KeyInfoBuilder::buildKeyInfo();
        X509Data* x509Data=X509DataBuilder::buildX509Data();
        m_keyInfo->getX509Datas().push_back(x509Data);
        for (vector<XSECCryptoX509*>::const_iterator x = m_xseccerts.begin(); x!=m_xseccerts.end(); ++x) {
            safeBuffer& buf=(*x)->getDEREncodingSB();
            X509Certificate* x509=X509CertificateBuilder::buildX509Certificate();
            x509->setValue(buf.sbStrToXMLCh());
            x509Data->getX509Certificates().push_back(x509);
        }
    }
}

vector<string>::size_type BasicX509Credential::getKeyNames(vector<string>& results) const
{
    if (m_xseccerts.empty() || m_xseccerts.front()->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL)
        return 0;
    
    X509* cert = static_cast<OpenSSLCryptoX509*>(m_xseccerts.front())->getOpenSSLX509();
    if (!cert)
        return 0;
        
    X509_NAME* subject=X509_get_subject_name(cert);
    if (subject) {
        char buf[256];
        memset(buf,0,sizeof(buf));
        if (X509_NAME_get_text_by_NID(subject,NID_commonName,buf,255)>0)
            results.push_back(buf);

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
                        results.push_back(alt);
                    }
                }
            }
        }
        GENERAL_NAMES_free(altnames);
    }

    return results.size();
}
