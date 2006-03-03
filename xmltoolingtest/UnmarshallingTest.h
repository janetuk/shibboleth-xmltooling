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

const XMLCh SimpleXMLObject::ID_ATTRIB_NAME[] = {
    chLatin_I, chLatin_d, chNull
};

class UnmarshallingTest : public CxxTest::TestSuite {
    QName m_qname;
public:
    UnmarshallingTest() : m_qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME) {}

    void setUp() {
        XMLObjectBuilder::registerBuilder(m_qname, new SimpleXMLObjectBuilder());
        Marshaller::registerMarshaller(m_qname, new SimpleXMLObjectMarshaller());
        Unmarshaller::registerUnmarshaller(m_qname, new SimpleXMLObjectUnmarshaller());
    }

    void tearDown() {
        XMLObjectBuilder::deregisterBuilder(m_qname);
        Marshaller::deregisterMarshaller(m_qname);
        Unmarshaller::deregisterUnmarshaller(m_qname);
    }

    void testUnmarshallingWithAttributes() {
        TS_TRACE("testUnmarshallingWithAttributes");

        string path=data_path + "SimpleXMLObjectWithAttribute.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const Unmarshaller* u = Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(u->unmarshall(doc->getDocumentElement(),true)));
        TS_ASSERT(sxObject.get()!=NULL);

        auto_ptr_XMLCh expected("Firefly");
        TSM_ASSERT_SAME_DATA("ID was not expected value", expected.get(), sxObject->getId(), XMLString::stringLen(expected.get()));
    }

    void testUnmarshallingWithElementContent() {
        TS_TRACE("testUnmarshallingWithElementContent");

        string path=data_path + "SimpleXMLObjectWithContent.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const Unmarshaller* u = Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(u->unmarshall(doc->getDocumentElement(),true)));
        TS_ASSERT(sxObject.get()!=NULL);

        auto_ptr_XMLCh expected("Sample Content");
        TSM_ASSERT_SAME_DATA("Element content was not expected value", expected.get(), sxObject->getValue(), XMLString::stringLen(expected.get()));
    }

    void testUnmarshallingWithChildElements() {
        TS_TRACE("testUnmarshallingWithChildElements");

        string path=data_path + "SimpleXMLObjectWithChildren.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const Unmarshaller* u = Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(u->unmarshall(doc->getDocumentElement(),true)));
        TS_ASSERT(sxObject.get()!=NULL);

        ListOf(SimpleXMLObject) kids=sxObject->getSimpleXMLObjects();
        TSM_ASSERT_EQUALS("Number of child elements was not expected value", 2, kids.size());
    }

    void testUnmarshallingWithUnknownChild() {
        TS_TRACE("testUnmarshallingWithUnknownChild");

        string path=data_path + "SimpleXMLObjectWithUnknownChild.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const Unmarshaller* u = Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        TS_ASSERT_THROWS(u->unmarshall(doc->getDocumentElement(),true),UnmarshallingException);
        doc->release();
    }
};
