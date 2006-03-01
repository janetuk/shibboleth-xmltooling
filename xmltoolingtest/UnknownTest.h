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
#include <xmltooling/io/Marshaller.h>
#include <xmltooling/io/Unmarshaller.h>


class UnknownTest : public CxxTest::TestSuite {
public:

    void testUnknown() {
        ifstream fs("../xmltoolingtest/data/SimpleXMLObjectWithChildren.xml");
        DOMDocument* doc=nonvalidatingPool->parse(fs);
        TS_ASSERT(doc!=NULL);

        string buf1;
        XMLHelper::serialize(doc->getDocumentElement(), buf1);

        const Unmarshaller* u=Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<XMLObject> xmlObject(u->unmarshall(doc->getDocumentElement(),true)); // bind document
        TS_ASSERT(xmlObject.get()!=NULL);

        auto_ptr<XMLObject> clonedObject(xmlObject->clone());
        TS_ASSERT(clonedObject.get()!=NULL);

        const Marshaller* m=Marshaller::getMarshaller(clonedObject.get());
        TS_ASSERT(m!=NULL);

        DOMElement* rootElement=m->marshall(clonedObject.get());
        TS_ASSERT(rootElement!=NULL);

        rootElement=m->marshall(clonedObject.get());    // should reuse DOM
        TS_ASSERT(rootElement!=NULL);

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

        const Unmarshaller* u=Unmarshaller::getUnmarshaller(doc->getDocumentElement());
        TS_ASSERT(u!=NULL);

        auto_ptr<XMLObject> xmlObject(u->unmarshall(doc->getDocumentElement(),true)); // bind document
        TS_ASSERT(xmlObject.get()!=NULL);

        const Marshaller* m=Marshaller::getMarshaller(xmlObject.get());
        TS_ASSERT(m!=NULL);

        DOMDocument* newDoc=nonvalidatingPool->newDocument();
        DOMElement* rootElement=m->marshall(xmlObject.get(), newDoc);
        TS_ASSERT(rootElement!=NULL);

        string buf2;
        XMLHelper::serialize(rootElement, buf2);
        TS_ASSERT_EQUALS(buf1,buf2);

        newDoc->release();
    }
};
