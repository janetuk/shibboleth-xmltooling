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
#include <xercesc/util/XMLUniDefs.hpp>

class NonVisibleNamespaceTest : public CxxTest::TestSuite {
public:
    void setUp() {
        xmltooling::QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        xmltooling::QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::registerBuilder(qname, new SimpleXMLObjectBuilder());
        XMLObjectBuilder::registerBuilder(qtype, new SimpleXMLObjectBuilder());
    }

    void tearDown() {
        xmltooling::QName qname(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME);
        xmltooling::QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME);
        XMLObjectBuilder::deregisterBuilder(qname);
        XMLObjectBuilder::deregisterBuilder(qtype);
    }

    void testNamespacesAfterBuilding() {
        xmltooling::QName qtype(SimpleXMLObject::NAMESPACE,SimpleXMLObject::TYPE_NAME,SimpleXMLObject::NAMESPACE_PREFIX);
        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(qtype);
        TS_ASSERT(b!=nullptr);
        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildObject(SimpleXMLObject::NAMESPACE, SimpleXMLObject::LOCAL_NAME, nullptr, &qtype))
            );
        TS_ASSERT(sxObject.get()!=nullptr);
        static_cast<AttributeExtensibleXMLObject*>(sxObject.get())->setAttribute(
            xmltooling::QName(nullptr, "attr1"), xmltooling::QName("http://www.example.org/testObjects/ext", "Value1", "test2")
            );

        static const XMLCh TEST2_PREFIX[] = { chLatin_t, chLatin_e, chLatin_s, chLatin_t, chDigit_2, chNull };

        const set<Namespace>& namespaces = sxObject->getNamespaces();
        bool cond1=false, cond2=false, cond3 = false;
        for (set<Namespace>::const_iterator ns = namespaces.begin(); ns != namespaces.end(); ++ns) {
            if (XMLString::equals(ns->getNamespacePrefix(), SimpleXMLObject::NAMESPACE_PREFIX)) {
                TSM_ASSERT("'test' namespace was visibly used", ns->usage() != Namespace::VisiblyUsed);
                cond1 = true;
            }
            else if (XMLString::equals(ns->getNamespacePrefix(), TEST2_PREFIX)) {
                TSM_ASSERT("'test2' namespace was visibly used", ns->usage() != Namespace::VisiblyUsed);
                cond2 = true;
            }
            else if (XMLString::equals(ns->getNamespacePrefix(), &chNull)) {
                TSM_ASSERT("Default namespace was not visibly used", ns->usage() == Namespace::VisiblyUsed);
                cond3 = true;
            }
        }
        TSM_ASSERT("'test' namespace was missing.", cond1);
        TSM_ASSERT("'test2' namespace was missing.", cond2);
        TSM_ASSERT("Default namespace was missing.", cond3);
    }

    void testNamespacesAfterUnmarshalling() {
        string path=data_path + "SimpleXMLObjectWithNonVisible.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(fs);
        TS_ASSERT(doc!=nullptr);

        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=nullptr);

        auto_ptr<SimpleXMLObject> sxObject(
            dynamic_cast<SimpleXMLObject*>(b->buildFromDocument(doc))
            );
        TS_ASSERT(sxObject.get()!=nullptr);

        const set<Namespace>& namespaces = sxObject->getNamespaces();
        bool cond1=false, cond2=false, cond3=false;
        for (set<Namespace>::const_iterator ns = namespaces.begin(); ns != namespaces.end(); ++ns) {
            if (XMLString::equals(ns->getNamespacePrefix(), SimpleXMLObject::NAMESPACE_PREFIX)) {
                TSM_ASSERT("'test' namespace was visibly used", ns->usage() != Namespace::VisiblyUsed);
                cond1 = true;
            }
            else if (XMLString::equals(ns->getNamespacePrefix(), &chNull)) {
                TSM_ASSERT("Default namespace was not visibly used", ns->usage() == Namespace::VisiblyUsed);
                cond2 = true;
            }
        }
        TSM_ASSERT("Default or 'test' namespace missing.", cond1 && cond2);
        for (set<Namespace>::const_iterator ns = namespaces.begin(); ns != namespaces.end(); ++ns) {
            static const XMLCh TEST2_PREFIX[] = { chLatin_t, chLatin_e, chLatin_s, chLatin_t, chDigit_2, chNull };
            TSM_ASSERT("'test2' namespace was noted during unmarshalling", !XMLString::equals(ns->getNamespacePrefix(), TEST2_PREFIX));
        }
    }
};
