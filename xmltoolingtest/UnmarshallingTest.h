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
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithAttribute.xml");
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const Unmarshaller* u = Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<SimpleXMLObject> sxObject(dynamic_cast<SimpleXMLObject*>(u->unmarshall(doc->getDocumentElement(),true)));
        TS_ASSERT(sxObject.get()!=NULL);

        auto_ptr_XMLCh expectedId("Firefly");
        TSM_ASSERT_SAME_DATA("ID was not expected value", expectedId.get(), sxObject->getId(), XMLString::stringLen(expectedId.get()));
    }
};
