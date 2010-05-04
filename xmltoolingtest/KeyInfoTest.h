/*
 *  Copyright 2001-2010 Internet2
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
#include <xmltooling/validation/ValidatorSuite.h>

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
        string path=data_path + "KeyInfo1.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getValidatingParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<KeyInfo> kiObject(dynamic_cast<KeyInfo*>(b->buildFromDocument(doc)));
        TS_ASSERT(kiObject.get()!=nullptr);
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            4, kiObject->getOrderedChildren().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getKeyValues().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getX509Datas().front()->getX509Certificates().size());

        auto_ptr_XMLCh expected("Public Key for CN=xmldap.org, OU=Domain Control Validated, O=xmldap.org");
        TSM_ASSERT("KeyName was not expected value", XMLString::equals(expected.get(), kiObject->getKeyNames().front()->getName()));

        SchemaValidators.validate(kiObject.get());
    }

    void testKeyInfo2() {
        string path=data_path + "KeyInfo2.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getValidatingParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<KeyInfo> kiObject(dynamic_cast<KeyInfo*>(b->buildFromDocument(doc)));
        TS_ASSERT(kiObject.get()!=nullptr);
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            2, kiObject->getOrderedChildren().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            1, kiObject->getRetrievalMethods().size());
        TSM_ASSERT_EQUALS("Number of child elements was not expected value",
            2, kiObject->getSPKIDatas().front()->getSPKISexps().size());

        SchemaValidators.validate(kiObject.get());
    }

    void testKeyInfo3() {
        string path=data_path + "KeyInfo3.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<KeyInfo> kiObject(dynamic_cast<KeyInfo*>(b->buildFromDocument(doc)));
        TS_ASSERT(kiObject.get()!=nullptr);
        TS_ASSERT_THROWS(SchemaValidators.validate(kiObject.get()),ValidationException);
    }
};
