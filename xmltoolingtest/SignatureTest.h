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
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

class TestContext : public ContentReference
{
    XMLCh* m_uri;
    
public:
    TestContext(const XMLCh* uri) {
        m_uri=XMLString::replicate(uri);
    }
    
    virtual ~TestContext() {
        XMLString::release(&m_uri);
    }

    ContentReference* clone() const {
        return new TestContext(m_uri);
    }

    void createReferences(DSIGSignature* sig) {
        DSIGReference* ref=sig->createReference(m_uri);
        ref->appendEnvelopedSignatureTransform();
        ref->appendCanonicalizationTransform(CANON_C14NE_NOC);
    }
};

class TestValidator : public Validator
{
    XMLCh* m_uri;
    
public:
    TestValidator(const XMLCh* uri) {
        m_uri=XMLString::replicate(uri);
    }
    
    virtual ~TestValidator() {
        XMLString::release(&m_uri);
    }

    Validator* clone() const {
        return new TestValidator(m_uri);
    }

    void validate(const XMLObject* xmlObject) const {
        DSIGSignature* sig=dynamic_cast<const Signature*>(xmlObject)->getXMLSignature();
        if (!sig)
            throw SignatureException("Only a marshalled Signature object can be verified.");
        const XMLCh* uri=sig->getReferenceList()->item(0)->getURI();
        TSM_ASSERT_SAME_DATA("Reference URI does not match.",uri,m_uri,XMLString::stringLen(uri));
        XSECKeyInfoResolverDefault resolver;
        sig->setKeyInfoResolver(&resolver); // It will clone the resolver for us.
        try {
            if (!sig->verify())
                throw SignatureException("Signature did not verify.");
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            throw SignatureException(string("Caught an XMLSecurity exception verifying signature: ") + temp.get());
        }
        catch(XSECCryptoException& e) {
            throw SignatureException(string("Caught an XMLSecurity exception verifying signature: ") + e.getMsg());
        }
    }
};

class _addcert : public std::binary_function<X509Data*,XSECCryptoX509*,void> {
public:
    void operator()(X509Data* bag, XSECCryptoX509* cert) const {
        safeBuffer& buf=cert->getDEREncodingSB();
        X509Certificate* x=X509CertificateBuilder::buildX509Certificate();
        x->setValue(buf.sbStrToXMLCh());
        bag->getX509Certificates().push_back(x);
    }
};

class SignatureTest : public CxxTest::TestSuite {
    XSECCryptoKey* m_key;
    vector<XSECCryptoX509*> m_certs;
public:
    void setUp() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::registerBuilder(qname, new SimpleXMLObjectBuilder());
        XMLObjectBuilder::registerBuilder(qtype, new SimpleXMLObjectBuilder());
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
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::deregisterBuilder(qname);
        XMLObjectBuilder::deregisterBuilder(qtype);
        delete m_key;
        for_each(m_certs.begin(),m_certs.end(),xmltooling::cleanup<XSECCryptoX509>());
    }

    void testSignature() {
        TS_TRACE("testSignature");

        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        const SimpleXMLObjectBuilder* b=dynamic_cast<const SimpleXMLObjectBuilder*>(XMLObjectBuilder::getBuilder(qname));
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
        Signature* sig=SignatureBuilder::buildSignature();
        sxObject->setSignature(sig);
        sig->setContentReference(new TestContext(&chNull));
        sig->setSigningKey(m_key->clone());
        
        // Build KeyInfo.
        KeyInfo* keyInfo=KeyInfoBuilder::buildKeyInfo();
        X509Data* x509Data=X509DataBuilder::buildX509Data();
        keyInfo->getX509Datas().push_back(x509Data);
        for_each(m_certs.begin(),m_certs.end(),bind1st(_addcert(),x509Data));
        sig->setKeyInfo(keyInfo);
        
        // Signing context for the whole document.
        vector<Signature*> sigs(1,sig);
        DOMElement* rootElement = NULL;
        try {
            rootElement=sxObject->marshall((DOMDocument*)NULL,&sigs);
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            throw;
        }
        
        string buf;
        XMLHelper::serialize(rootElement, buf);
        //TS_TRACE(buf.c_str());

        istringstream in(buf);
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        auto_ptr<SimpleXMLObject> sxObject2(dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc)));
        TS_ASSERT(sxObject2.get()!=NULL);
        TS_ASSERT(sxObject2->getSignature()!=NULL);
        sxObject2->getSignature()->registerValidator(new TestValidator(&chNull));
        
        try {
            sxObject2->getSignature()->validate(false);
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            throw;
        }
    }

};
