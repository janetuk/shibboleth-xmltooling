/*
 *  Copyright 2001-2005 Internet2
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

#include "XMLObjectBaseTestCase.h"

#include <xmltooling/encryption/Decrypter.h>
#include <xmltooling/encryption/Encrypter.h>

#include <fstream>
#include <openssl/pem.h>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGReference.hpp>
#include <xsec/enc/XSECKeyInfoResolverDefault.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmlencryption;

class _addcert : public std::binary_function<X509Data*,XSECCryptoX509*,void> {
public:
    void operator()(X509Data* bag, XSECCryptoX509* cert) const {
        safeBuffer& buf=cert->getDEREncodingSB();
        X509Certificate* x=X509CertificateBuilder::buildX509Certificate();
        x->setValue(buf.sbStrToXMLCh());
        bag->getX509Certificates().push_back(x);
    }
};

class EncryptionTest : public CxxTest::TestSuite {
    XSECCryptoKey* m_key;
    vector<XSECCryptoX509*> m_certs;
public:
    void setUp() {
        string keypath=data_path + "key.pem";
        BIO* in=BIO_new(BIO_s_file_internal());
        if (in && BIO_read_filename(in,keypath.c_str())>0) {
            EVP_PKEY* pkey=PEM_read_bio_PrivateKey(in, NULL, NULL, NULL);
            if (pkey) {
                m_key=new OpenSSLCryptoKeyRSA(pkey);
                EVP_PKEY_free(pkey);
            }
        }
        if (in) BIO_free(in);
        TS_ASSERT(m_key!=NULL);

        string certpath=data_path + "cert.pem";
        in=BIO_new(BIO_s_file_internal());
        if (in && BIO_read_filename(in,certpath.c_str())>0) {
            X509* x=NULL;
            while (x=PEM_read_bio_X509(in,NULL,NULL,NULL)) {
                m_certs.push_back(new OpenSSLCryptoX509(x));
                X509_free(x);
            }
        }
        if (in) BIO_free(in);
        TS_ASSERT(m_certs.size()>0);
        
    }

    void tearDown() {
        delete m_key;
        for_each(m_certs.begin(),m_certs.end(),xmltooling::cleanup<XSECCryptoX509>());
    }

    void testBasic() {
        TS_TRACE("testBasic");

        string path=data_path + "ComplexXMLObject.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        try {
            Encrypter encrypter;
            Encrypter::EncryptionParams ep;
            Encrypter::KeyEncryptionParams kep(DSIGConstants::s_unicodeStrURIRSA_1_5,m_key->clone());
            auto_ptr<EncryptedData> encData(encrypter.encryptElement(doc->getDocumentElement(),ep,&kep));

            string buf;
            XMLHelper::serialize(encData->marshall(), buf);
            istringstream is(buf);
            DOMDocument* doc2=XMLToolingConfig::getConfig().getValidatingParser().parse(is);
            auto_ptr<EncryptedData> encData2(
                dynamic_cast<EncryptedData*>(XMLObjectBuilder::buildOneFromElement(doc2->getDocumentElement(),true))
                );

            Decrypter decrypter(new KeyResolver(m_key->clone()));
            DOMDocumentFragment* frag = decrypter.decryptData(encData2.get());
            XMLHelper::serialize(static_cast<DOMElement*>(frag->getFirstChild()), buf);
            TS_TRACE(buf.c_str());
            TS_ASSERT(doc->getDocumentElement()->isEqualNode(frag->getFirstChild()));
            frag->release();
            doc->release();
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            doc->release();
            throw;
        }
    }

};
