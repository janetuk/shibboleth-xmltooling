/*
*  Copyright 2001-2006 Internet2
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

/**
 * SOAPSchemaValidators.cpp
 * 
 * Schema validators for SOAP schema
 */

#include "internal.h"
#include "exceptions.h"
#include "soap/SOAP.h"

using namespace soap11;
using namespace xmltooling;
using namespace std;

namespace {

    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Faultstring);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Faultactor);

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Faultcode);
        XMLOBJECTVALIDATOR_REQUIRE(Faultcode,Code);
    END_XMLOBJECTVALIDATOR;
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Fault);
        XMLOBJECTVALIDATOR_REQUIRE(Fault,Faultcode);
        XMLOBJECTVALIDATOR_REQUIRE(Fault,Faultstring);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Envelope);
        XMLOBJECTVALIDATOR_REQUIRE(Envelope,Body);
    END_XMLOBJECTVALIDATOR;
    
};

#define REGISTER_ELEMENT(namespaceURI,cname) \
    q=QName(namespaceURI,cname::LOCAL_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    SOAPSchemaValidators.registerValidator(q,new cname##SchemaValidator())
    
#define REGISTER_TYPE(namespaceURI,cname) \
    q=QName(namespaceURI,cname::TYPE_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    SOAPSchemaValidators.registerValidator(q,new cname##SchemaValidator())

#define REGISTER_ELEMENT_NOVAL(namespaceURI,cname) \
    q=QName(namespaceURI,cname::LOCAL_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder());
    
#define REGISTER_TYPE_NOVAL(namespaceURI,cname) \
    q=QName(namespaceURI,cname::TYPE_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder());

ValidatorSuite soap11::SOAPSchemaValidators("SOAPSchemaValidators");

void soap11::registerSOAPClasses()
{
    QName q;
    REGISTER_ELEMENT_NOVAL(XMLConstants::SOAP11ENV_NS,Body);
    REGISTER_ELEMENT_NOVAL(XMLConstants::SOAP11ENV_NS,Detail);
    REGISTER_ELEMENT(XMLConstants::SOAP11ENV_NS,Envelope);
    REGISTER_ELEMENT(XMLConstants::SOAP11ENV_NS,Fault);
    REGISTER_ELEMENT(XMLConstants::SOAP11ENV_NS,Faultactor);
    REGISTER_ELEMENT(XMLConstants::SOAP11ENV_NS,Faultcode);
    REGISTER_ELEMENT(XMLConstants::SOAP11ENV_NS,Faultstring);
    REGISTER_ELEMENT_NOVAL(XMLConstants::SOAP11ENV_NS,Header);
    REGISTER_TYPE_NOVAL(XMLConstants::SOAP11ENV_NS,Body);
    REGISTER_TYPE_NOVAL(XMLConstants::SOAP11ENV_NS,Detail);
    REGISTER_TYPE(XMLConstants::SOAP11ENV_NS,Envelope);
    REGISTER_TYPE(XMLConstants::SOAP11ENV_NS,Fault);
    REGISTER_TYPE_NOVAL(XMLConstants::SOAP11ENV_NS,Header);
}
