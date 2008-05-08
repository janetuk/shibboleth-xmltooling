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
#include <xercesc/util/XMLUniDefs.hpp>

const XMLCh SimpleXMLObject::NAMESPACE[] = {
    chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
    chLatin_w, chLatin_w, chLatin_w, chPeriod,
    chLatin_e, chLatin_x, chLatin_a, chLatin_m, chLatin_p, chLatin_l, chLatin_e, chPeriod,
    chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
    chLatin_t, chLatin_e, chLatin_s, chLatin_t,
    chLatin_O, chLatin_b, chLatin_j, chLatin_e, chLatin_c, chLatin_t, chLatin_s, chNull
};

const XMLCh SimpleXMLObject::NAMESPACE_PREFIX[] = {
    chLatin_t, chLatin_e, chLatin_s, chLatin_t, chNull
};

const XMLCh SimpleXMLObject::LOCAL_NAME[] = {
    chLatin_S, chLatin_i, chLatin_m, chLatin_p, chLatin_l, chLatin_e,
    chLatin_E, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh SimpleXMLObject::DERIVED_NAME[] = {
    chLatin_D, chLatin_e, chLatin_r, chLatin_i, chLatin_v, chLatin_e, chLatin_d,
    chLatin_E, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh SimpleXMLObject::TYPE_NAME[] = {
    chLatin_S, chLatin_i, chLatin_m, chLatin_p, chLatin_l, chLatin_e,
    chLatin_E, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, 
    chLatin_T, chLatin_y, chLatin_p, chLatin_e, chNull
};

const XMLCh SimpleXMLObject::ID_ATTRIB_NAME[] = {
    chLatin_I, chLatin_d, chNull
};

class UnmarshallingTest : public CxxTest::TestSuite {
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

    void testUnmarshallingWithAttributes() {
        string path=data_path + "SimpleXMLObjectWithAttribute.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(sxObject.get()!=NULL);

        auto_ptr_XMLCh expected("Firefly");
        TSM_ASSERT("ID was not expected value", XMLString::equals(expected.get(), sxObject->getId()));
    }

    void testUnmarshallingWithElementContent() {
        string path=data_path + "SimpleXMLObjectWithContent.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(sxObject.get()!=NULL);

        auto_ptr_XMLCh expected("Sample Content");
        TSM_ASSERT("Element content was not expected value", XMLString::equals(expected.get(), sxObject->getValue()));
    }

    void testUnmarshallingWithChildElements() {
        string path=data_path + "SimpleXMLObjectWithChildren.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(sxObject.get()!=NULL);

        VectorOf(SimpleXMLObject) kids=sxObject->getSimpleXMLObjects();
        TSM_ASSERT_EQUALS("Number of child elements was not expected value", 3, kids.size());
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        TSM_ASSERT_EQUALS("Child's schema type was not expected value", qtype, *(kids.back()->getSchemaType()));
    }

    void testUnmarshallingWithClone() {
        string path=data_path + "SimpleXMLObjectWithChildren.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(sxObject.get()!=NULL);

        sxObject->releaseThisAndChildrenDOM();
        auto_ptr<SimpleXMLObject> clonedObject(dynamic_cast<SimpleXMLObject*>(sxObject->clone()));

        VectorOf(SimpleXMLObject) kids=clonedObject->getSimpleXMLObjects();
        TSM_ASSERT_EQUALS("Number of child elements was not expected value", 3, kids.size());
        QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        TSM_ASSERT_EQUALS("Child's schema type was not expected value", qtype, *(kids.back()->getSchemaType()));
    }

    void testUnmarshallingWithUnknownChild() {
        string path=data_path + "SimpleXMLObjectWithUnknownChild.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        TS_ASSERT_THROWS(b->buildFromDocument(doc),UnmarshallingException);
        doc->release();
    }
};
