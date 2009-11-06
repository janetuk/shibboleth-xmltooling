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

#include "XMLObjectBaseTestCase.h"

#include <xmltooling/security/Credential.h>
#include <xmltooling/security/CredentialCriteria.h>
#include <xmltooling/security/CredentialResolver.h>
#include <xmltooling/signature/ContentReference.h>
#include <xmltooling/signature/KeyInfo.h>
#include <xmltooling/signature/SignatureValidator.h>

#include <fstream>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGReference.hpp>
#include <xsec/dsig/DSIGSignature.hpp>

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

    void createReferences(DSIGSignature* sig) {
        DSIGReference* ref=sig->createReference(m_uri);
        ref->appendEnvelopedSignatureTransform();
        ref->appendCanonicalizationTransform(CANON_C14NE_NOC);
    }
};

class TestValidator : public SignatureValidator
{
    XMLCh* m_uri;
    
public:
    TestValidator(const XMLCh* uri, const Credential* credential) : SignatureValidator(credential) {
        m_uri=XMLString::replicate(uri);
    }
    
    virtual ~TestValidator() {
        XMLString::release(&m_uri);
    }

    void validate(const Signature* sigObj) const {
        DSIGSignature* sig=sigObj->getXMLSignature();
        if (!sig)
            throw SignatureException("Only a marshalled Signature object can be verified.");
        const XMLCh* uri=sig->getReferenceList()->item(0)->getURI();
        TSM_ASSERT_SAME_DATA("Reference URI does not match.",uri,m_uri,XMLString::stringLen(uri));
        SignatureValidator::validate(sigObj);
    }
};

class SignatureTest : public CxxTest::TestSuite {
    CredentialResolver* m_resolver;
public:
    void setUp() {
        m_resolver=NULL;
        xmltooling::QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        xmltooling::QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::registerBuilder(qname, new SimpleXMLObjectBuilder());
        XMLObjectBuilder::registerBuilder(qtype, new SimpleXMLObjectBuilder());

        string config = data_path + "FilesystemCredentialResolver.xml";
        ifstream in(config.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        XercesJanitor<DOMDocument> janitor(doc);
        m_resolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(
            FILESYSTEM_CREDENTIAL_RESOLVER,doc->getDocumentElement()
            );
    }

    void tearDown() {
        xmltooling::QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        xmltooling::QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::deregisterBuilder(qname);
        XMLObjectBuilder::deregisterBuilder(qtype);
        delete m_resolver;
    }

    void testSignature() {
        xmltooling::QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        const SimpleXMLObjectBuilder* b=dynamic_cast<const SimpleXMLObjectBuilder*>(XMLObjectBuilder::getBuilder(qname));
        TS_ASSERT(b!=NULL);
        
        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        TS_ASSERT(sxObject.get()!=NULL);
        VectorOf(SimpleXMLObject) kids=sxObject->getSimpleXMLObjects();
        kids.push_back(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        kids.push_back(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        
        // Test some collection stuff
        auto_ptr_XMLCh foo("Foo");
        auto_ptr_XMLCh bar("Bar");
        kids.begin()->setId(foo.get());
        kids[1]->setValue(bar.get());
        
        // Append a Signature.
        Signature* sig=SignatureBuilder::buildSignature();
        sxObject->setSignature(sig);
        sig->setContentReference(new TestContext(&chNull));

        CredentialCriteria cc;
        cc.setUsage(Credential::SIGNING_CREDENTIAL);
        Locker locker(m_resolver);
        const Credential* cred = m_resolver->resolve(&cc);
        TSM_ASSERT("Retrieved credential was null", cred!=NULL);
        
        DOMElement* rootElement = NULL;
        try {
            vector<Signature*> sigs(1,sig);
            rootElement=sxObject->marshall((DOMDocument*)NULL,&sigs,cred);
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
        
        try {
            TestValidator tv(&chNull, cred);
            tv.validate(sxObject2->getSignature());
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            throw;
        }
    }

};
