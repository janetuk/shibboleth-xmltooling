/*
 *  Licensed to Internet2 under one or more contributor license agreements.
 *  See the NOTICE file distributed with this work for additional information
 *  regarding copyright ownership. Internet2 licenses this file to you under
 *  the Apache License, Version 2.0 (the "License"); you may not use this
 *  file except in compliance with the License.  You may obtain a copy of the
 *  License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

#include "XMLObjectBaseTestCase.h"

#include <fstream>
#include <xmltooling/soap/SOAP.h>
#include <xmltooling/validation/ValidatorSuite.h>

using namespace soap11;

class SOAPTest : public CxxTest::TestSuite {
public:
    SOAPTest() {}

    void testSOAPFault() {
        string path=data_path + "SOAPFault.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getValidatingParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<Envelope> envObject(dynamic_cast<Envelope*>(b->buildFromDocument(doc)));
        TS_ASSERT(envObject.get()!=nullptr);
        TSM_ASSERT("SOAP Envelope missing Body", envObject->getBody() != nullptr);
        TSM_ASSERT_EQUALS("SOAP Body missing Fault", 1, envObject->getBody()->getOrderedChildren().size());

        SchemaValidators.validate(envObject.get());
    }
};
