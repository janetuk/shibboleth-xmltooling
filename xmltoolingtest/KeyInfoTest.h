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
#include <xmltooling/signature/KeyInfo.h>

using namespace xmlsignature;

class KeyInfoTest : public CxxTest::TestSuite {
public:
    KeyInfoTest() {}

    void setUp() {
        XMLObjectBuilder::registerDefaultBuilder(new AnyElementBuilder());
    }

    void tearDown() {
        XMLObjectBuilder::deregisterDefaultBuilder();
    }

    void testKeyInfo1() {
        TS_TRACE("testKeyInfo1");

        string path=data_path + "KeyInfo1.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=validatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<KeyInfo> kiObject(
            dynamic_cast<KeyInfo*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(kiObject.get()!=NULL);
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            3, kiObject->getOrderedChildren().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getKeyValues().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getX509Datas().front()->getX509Certificates().size());

        auto_ptr_XMLCh expected("Public Key for CN=xmldap.org, OU=Domain Control Validated, O=xmldap.org");
        TSM_ASSERT_SAME_DATA("KeyName was not expected value",
            expected.get(), kiObject->getKeyNames().front()->getName(), XMLString::stringLen(expected.get()));

        Validator::checkValidity(kiObject.get());
    }

    void testKeyInfo2() {
        TS_TRACE("testKeyInfo2");

        string path=data_path + "KeyInfo2.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=validatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<KeyInfo> kiObject(
            dynamic_cast<KeyInfo*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(kiObject.get()!=NULL);
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            2, kiObject->getOrderedChildren().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getRetrievalMethods().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            2, kiObject->getSPKIDatas().front()->getSPKISexps().size());

        Validator::checkValidity(kiObject.get());
    }

    void testKeyInfo3() {
        TS_TRACE("testKeyInfo3");

        string path=data_path + "KeyInfo3.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<KeyInfo> kiObject(
            dynamic_cast<KeyInfo*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(kiObject.get()!=NULL);
        TS_ASSERT_THROWS(Validator::checkValidity(kiObject.get()),ValidationException);
    }
};
