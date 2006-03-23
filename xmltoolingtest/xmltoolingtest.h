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
#include <cxxtest/GlobalFixture.h>
#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/util/ParserPool.h>

ParserPool* validatingPool=NULL;
ParserPool* nonvalidatingPool=NULL;
std::string data_path = "../xmltoolingtest/data/";

class ToolingFixture : public CxxTest::GlobalFixture
{
public:
    bool setUpWorld() {
        XMLToolingConfig::getConfig().log_config();
        if (!XMLToolingConfig::getConfig().init())
            return false;
        validatingPool = new ParserPool(true,true);
        nonvalidatingPool = new ParserPool();
        if (getenv("XMLTOOLINGTEST_DATA"))
            data_path=std::string(getenv("XMLTOOLINGTEST_DATA")) + "/";
        return true;
    }
    bool tearDownWorld() {
        delete validatingPool;
        delete nonvalidatingPool;
        XMLToolingConfig::getConfig().term();
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

    void testCatalog(void) {
        std::string path=data_path + "catalog.xml";
        auto_ptr_XMLCh temp(path.c_str());
        TS_ASSERT(validatingPool->loadCatalog(temp.get()));
    }

    void testUnknown() {
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithChildren.xml");
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        string buf1;
        XMLHelper::serialize(doc->getDocumentElement(), buf1);

        const XMLObjectBuilder* b=XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<XMLObject> xmlObject(b->buildFromDocument(doc)); // bind document
        TS_ASSERT(xmlObject.get()!=NULL);

        auto_ptr<XMLObject> clonedObject(xmlObject->clone());
        TS_ASSERT(clonedObject.get()!=NULL);

        DOMElement* rootElement=clonedObject->marshall();
        TS_ASSERT(rootElement!=NULL);

        // should reuse DOM
        TS_ASSERT(rootElement==clonedObject->marshall());

        string buf2;
        XMLHelper::serialize(rootElement, buf2);
        TS_ASSERT_EQUALS(buf1,buf2);
    }

    void testUnknownWithDocChange() {
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithChildren.xml");
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        string buf1;
        XMLHelper::serialize(doc->getDocumentElement(), buf1);

        const XMLObjectBuilder* b=XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);

        auto_ptr<XMLObject> xmlObject(b->buildFromDocument(doc)); // bind document
        TS_ASSERT(xmlObject.get()!=NULL);

        DOMDocument* newDoc=nonvalidatingPool->newDocument();
        DOMElement* rootElement=xmlObject->marshall(newDoc);
        TS_ASSERT(rootElement!=NULL);

        string buf2;
        XMLHelper::serialize(rootElement, buf2);
        TS_ASSERT_EQUALS(buf1,buf2);

        newDoc->release();
    }
};

