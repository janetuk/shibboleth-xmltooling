/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include "XMLObjectBaseTestCase.h"

#include <fstream>
#include <cxxtest/GlobalFixture.h>
#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/util/ParserPool.h>

//#define XMLTOOLINGTEST_LEAKCHECK

std::string data_path = "../xmltoolingtest/data/";

class ToolingFixture : public CxxTest::GlobalFixture
{
public:
    bool setUpWorld() {
        XMLToolingConfig::getConfig().log_config();

        if (getenv("XMLTOOLINGTEST_DATA"))
            data_path=std::string(getenv("XMLTOOLINGTEST_DATA")) + "/";
        XMLToolingConfig::getConfig().catalog_path = data_path + "catalog.xml";

        if (!XMLToolingConfig::getConfig().init())
            return false;
        if (!XMLToolingConfig::getConfig().init())  // should be a no-op
            return false;
        
        return true;
    }
    bool tearDownWorld() {
        XMLToolingConfig::getConfig().term();       // should be a no-op
        XMLToolingConfig::getConfig().term();
        XMLToolingConfig::getConfig().term();       // shouldn't break anything
#if defined(_MSC_VER ) && defined(XMLTOOLINGTEST_LEAKCHECK)
       _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
       _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
       _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
       _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
       _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
       _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
       _CrtDumpMemoryLeaks();
#endif
        return true;
    }
    //bool setUp() { printf( "</test>" ); return true; }
    //bool tearDown() { printf( "</test>" ); return true; }
};

static ToolingFixture globalFixture;

class GlobalTest : public CxxTest::TestSuite
{
public:
    void setUp() {
        XMLObjectBuilder::registerDefaultBuilder(new UnknownElementBuilder());
    }

    void tearDown() {
        XMLObjectBuilder::deregisterDefaultBuilder();
    }

    void testUnknown() {
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithChildren.xml");
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        string buf1;
        XMLHelper::serialize(doc->getDocumentElement(), buf1);

        const XMLObjectBuilder* b=XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<XMLObject> xmlObject(b->buildFromDocument(doc)); // bind document
        TS_ASSERT(xmlObject.get()!=nullptr);

        auto_ptr<XMLObject> clonedObject(xmlObject->clone());
        TS_ASSERT(clonedObject.get()!=nullptr);

        DOMElement* rootElement=clonedObject->marshall();
        TS_ASSERT(rootElement!=nullptr);

        // should reuse DOM
        TS_ASSERT(rootElement==clonedObject->marshall());

        string buf2;
        XMLHelper::serialize(rootElement, buf2);
        TS_ASSERT_EQUALS(buf1,buf2);
    }

    void testUnknownWithDocChange() {
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithChildren.xml");
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        string buf1;
        XMLHelper::serialize(doc->getDocumentElement(), buf1);

        const XMLObjectBuilder* b=XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<XMLObject> xmlObject(b->buildFromDocument(doc)); // bind document
        TS_ASSERT(xmlObject.get()!=nullptr);

        DOMDocument* newDoc=XMLToolingConfig::getConfig().getParser().newDocument();
        DOMElement* rootElement=xmlObject->marshall(newDoc);
        TS_ASSERT(rootElement!=nullptr);

        string buf2;
        XMLHelper::serialize(rootElement, buf2);
        TS_ASSERT_EQUALS(buf1,buf2);

        newDoc->release();
    }
};

