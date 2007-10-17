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

#include "XMLObjectBaseTestCase.h"

#include <fstream>

class MarshallingTest : public CxxTest::TestSuite {
public:
    void setUp() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::registerBuilder(qname, new SimpleXMLObjectBuilder());
        XMLObjectBuilder::registerBuilder(qtype, new SimpleXMLObjectBuilder());
    }

    void tearDown() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::deregisterBuilder(qname);
        XMLObjectBuilder::deregisterBuilder(qtype);
    }

    void testMarshallingWithAttributes() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        auto_ptr<SimpleXMLObject> sxObject(SimpleXMLObjectBuilder::buildSimpleXMLObject());
        TS_ASSERT(sxObject.get()!=NULL);
        auto_ptr_XMLCh expected("Firefly");
        sxObject->setId(expected.get());
        
        DOMElement* rootElement = sxObject->marshall();

        string path=data_path + "SimpleXMLObjectWithAttribute.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        TS_ASSERT(rootElement->isEqualNode(doc->getDocumentElement()));
        doc->release();
    }

    void testMarshallingWithElementContent() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        auto_ptr<SimpleXMLObject> sxObject(SimpleXMLObjectBuilder::buildSimpleXMLObject());
        TS_ASSERT(sxObject.get()!=NULL);
        auto_ptr_XMLCh expected("Sample Content");
        sxObject->setValue(expected.get());
        
        DOMElement* rootElement = sxObject->marshall();

        string path=data_path + "SimpleXMLObjectWithContent.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        TS_ASSERT(rootElement->isEqualNode(doc->getDocumentElement()));
        doc->release();
    }

    void testMarshallingWithChildElements() {
        QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        const SimpleXMLObjectBuilder* b=dynamic_cast<const SimpleXMLObjectBuilder*>(XMLObjectBuilder::getBuilder(qname));
        TS_ASSERT(b!=NULL);
        
        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        TS_ASSERT(sxObject.get()!=NULL);
        VectorOf(SimpleXMLObject) kids=sxObject->getSimpleXMLObjects();
        kids.push_back(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        kids.push_back(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        kids.push_back(dynamic_cast<SimpleXMLObject*>(b->buildObject()));
        
        // Test some collection stuff
        auto_ptr_XMLCh foo("Foo");
        auto_ptr_XMLCh bar("Bar");
        auto_ptr_XMLCh baz("Baz");
        kids.begin()->setId(foo.get());
        kids.at(2)->setValue(bar.get());
        kids.erase(kids.begin()+1);
        TS_ASSERT(XMLString::equals(kids.back()->getValue(), bar.get()));
        
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME,SimpleXMLObject::NAMESPACE_PREFIX);
        kids.push_back(
            dynamic_cast<SimpleXMLObject*>(
                b->buildObject(SimpleXMLObject::NAMESPACE,SimpleXMLObject::DERIVED_NAME,SimpleXMLObject::NAMESPACE_PREFIX,&qtype)
                )
            );
        kids.back()->setValue(baz.get());
        
        DOMElement* rootElement = sxObject->marshall();

        string path=data_path + "SimpleXMLObjectWithChildren.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        TS_ASSERT(rootElement->isEqualNode(doc->getDocumentElement()));
        doc->release();
    }

};
