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

class ComplexXMLObjectTest : public CxxTest::TestSuite {
public:
    ComplexXMLObjectTest() {}

    void setUp() {
        XMLObjectBuilder::registerDefaultBuilder(new WildcardXMLObjectBuilder());
    }

    void tearDown() {
        //XMLObjectBuilder::deregisterDefaultBuilder();
    }

    void testComplexUnmarshalling() {
        TS_TRACE("testComplexUnmarshalling");

        string path=data_path + "ComplexXMLObject.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<WildcardXMLObject> wcObject(
            dynamic_cast<WildcardXMLObject*>(b->buildObject(doc->getDocumentElement())->unmarshall(doc->getDocumentElement(),true))
            );
        TS_ASSERT(wcObject.get()!=NULL);

        ListOf(XMLObject) kids=wcObject->getXMLObjects();
        TSM_ASSERT_EQUALS("Number of child elements was not expected value", 2, kids.size());
        
        WildcardXMLObject* wc1=dynamic_cast<WildcardXMLObject*>(*(++kids.begin()));
        WildcardXMLObject* wc2=dynamic_cast<WildcardXMLObject*>(*(++(wc1->getXMLObjects().begin())));
        TSM_ASSERT_EQUALS("Number of child elements was not expected value", 3, wc2->getXMLObjects().size());

        static const XMLCh html[] = {chLatin_h, chLatin_t, chLatin_m, chLatin_l, chNull};
        static const XMLCh div[] = {chLatin_d, chLatin_i, chLatin_v, chNull};
        auto_ptr_XMLCh htmlns("http://www.w3.org/1999/xhtml");
        QName q(htmlns.get(),div,html);
        ListOf(XMLObject)::const_iterator it=wc2->getXMLObjects().begin();
        ++it; ++it;
        TSM_ASSERT_EQUALS("Element QName unexpected", it->getElementQName(),q);
    }

};
