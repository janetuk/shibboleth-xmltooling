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
#include <sstream>
#include <xmltooling/util/TemplateEngine.h>

class TemplateEngineTest : public CxxTest::TestSuite {
public:
    void setUp() {
    }
    
    void tearDown() {
    }

    void testTemplateEngine() {
        auto_ptr<TemplateEngine> engine(new TemplateEngine());

        TemplateEngine::TemplateParameters p;
        p.m_map["foo1"] = "bar1";
        p.m_map["foo3"] = "bar3";
        p.m_map["encoded"] = "http://www.example.org/foo/bar#foobar";
        multimap<string,string>& submap = p.m_collectionMap["sub"];
        submap.insert(pair<const string,string>("subfoo1", "subbar1"));
        submap.insert(pair<const string,string>("subfoo2", "subbar2"));
        
        string path = data_path + "template.in";
        ifstream in(path.c_str());
        ostringstream out;
        
        engine->run(in,out,p);
        
        string compstr;
        path = data_path + "template.out";
        ifstream compfile(path.c_str());
        while (getline(compfile,path))
            compstr += path + '\n';
            
        TSM_ASSERT_EQUALS("Template output did not match.", out.str(), compstr);
    }
};
