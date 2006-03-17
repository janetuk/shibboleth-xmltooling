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

#include <fstream>
#include <openssl/pem.h>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGReference.hpp>
#include <xsec/enc/XSECKeyInfoResolverDefault.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>

class TestContext : public SigningContext, public VerifyingContext
{
    XSECCryptoKey* m_key;
    vector<XSECCryptoX509*> m_certs;
    XMLCh* m_uri;
    
public:
    TestContext(const XMLCh* uri) {
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
        
        m_uri=XMLString::replicate(uri);
    }
    
    virtual ~TestContext() {
        delete m_key;
        for_each(m_certs.begin(),m_certs.end(),xmltooling::cleanup<XSECCryptoX509>());
        XMLString::release(&m_uri);
    }

    void createSignature(DSIGSignature* sig) const {
        DSIGReference* ref=sig->createReference(m_uri);
        ref->appendEnvelopedSignatureTransform();
        ref->appendCanonicalizationTransform(CANON_C14NE_NOC);
    }

    void verifySignature(DSIGSignature* sig) const {
        const XMLCh* uri=sig->getReferenceList()->item(0)->getURI();
        TSM_ASSERT_SAME_DATA("Reference URI does not match.",uri,m_uri,XMLString::stringLen(uri));
        XSECKeyInfoResolverDefault resolver;
        sig->setKeyInfoResolver(&resolver); // It will clone the resolver for us.
        sig->verify();
    }
    
    const std::vector<XSECCryptoX509*>& getX509Certificates() const { return m_certs; }
    XSECCryptoKey* getSigningKey() const { return m_key->clone(); }
};

class SignatureTest : public CxxTest::TestSuite {
    QName m_qname;
public:
    SignatureTest() : m_qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME) {}

    void setUp() {
        XMLObjectBuilder::registerBuilder(m_qname, new SimpleXMLObjectBuilder());
    }

    void tearDown() {
        XMLObjectBuilder::deregisterBuilder(m_qname);
    }

    void testSignature() {
        TS_TRACE("testSignature");

        const SimpleXMLObjectBuilder* b=dynamic_cast<const SimpleXMLObjectBuilder*>(XMLObjectBuilder::getBuilder(m_qname));
        TS_ASSERT(b!=NULL);
        
        auto_ptr<SimpleXMLObject> sxObject(b->buildObject());
        TS_ASSERT(sxObject.get()!=NULL);
        VectorOf(SimpleXMLObject) kids=sxObject->getSimpleXMLObjects();
        kids.push_back(b->buildObject());
        kids.push_back(b->buildObject());
        
        // Test some collection stuff
        auto_ptr_XMLCh foo("Foo");
        auto_ptr_XMLCh bar("Bar");
        kids.begin()->setId(foo.get());
        kids[1]->setValue(bar.get());
        
        // Append a Signature.
        Signature* sig=dynamic_cast<Signature*>(XMLObjectBuilder::buildObject(QName(XMLConstants::XMLSIG_NS,Signature::LOCAL_NAME)));
        sxObject->setSignature(sig);
        
        // Signing context for the whole document.
        TestContext tc(&chNull);
        MarshallingContext mctx(sig,&tc);
        DOMElement* rootElement = sxObject->marshall((DOMDocument*)NULL,&mctx);
        
        string buf;
        XMLHelper::serialize(rootElement, buf);
        TS_TRACE(buf.c_str());

        istringstream in(buf);
        DOMDocument* doc=nonvalidatingPool->parse(in);
        auto_ptr<SimpleXMLObject> sxObject2(dynamic_cast<SimpleXMLObject*>(b->buildObject()->unmarshall(doc->getDocumentElement(),true)));
        TS_ASSERT(sxObject2.get()!=NULL);
        TS_ASSERT(sxObject2->getSignature()!=NULL);
        
        try {
            sxObject2->getSignature()->verify(tc);
        }
        catch (SignatureException& e) {
            TS_TRACE(e.what());
            throw;
        }
    }

};
