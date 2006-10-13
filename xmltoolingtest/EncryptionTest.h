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
#include <xmltooling/signature/CredentialResolver.h>

#include <fstream>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGReference.hpp>

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
    CredentialResolver* m_resolver;
public:
    void setUp() {
        m_resolver=NULL;
        string config = data_path + "FilesystemCredentialResolver.xml";
        ifstream in(config.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        XercesJanitor<DOMDocument> janitor(doc);
        m_resolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(
            FILESYSTEM_CREDENTIAL_RESOLVER,doc->getDocumentElement()
            );
    }

    void tearDown() {
        delete m_resolver;
    }

    void testEncryption() {
        string path=data_path + "ComplexXMLObject.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        try {
            Locker locker(m_resolver);
            Encrypter encrypter;
            Encrypter::EncryptionParams ep;
            Encrypter::KeyEncryptionParams kep(DSIGConstants::s_unicodeStrURIRSA_1_5,m_resolver->getKey());
            auto_ptr<EncryptedData> encData(encrypter.encryptElement(doc->getDocumentElement(),ep,&kep));

            string buf;
            XMLHelper::serialize(encData->marshall(), buf);
            istringstream is(buf);
            DOMDocument* doc2=XMLToolingConfig::getConfig().getValidatingParser().parse(is);
            auto_ptr<EncryptedData> encData2(
                dynamic_cast<EncryptedData*>(XMLObjectBuilder::buildOneFromElement(doc2->getDocumentElement(),true))
                );

            Decrypter decrypter(new KeyResolver(m_resolver->getKey()));
            DOMDocumentFragment* frag = decrypter.decryptData(encData2.get());
            XMLHelper::serialize(static_cast<DOMElement*>(frag->getFirstChild()), buf);
            //TS_TRACE(buf.c_str());
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
