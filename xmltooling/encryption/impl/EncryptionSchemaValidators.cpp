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
 * EncryptionSchemaValidators.cpp
 * 
 * Schema validators for XML Encryption schema
 */

#include "internal.h"
#include "exceptions.h"
#include "encryption/Encryption.h"

using namespace xmlencryption;
using namespace xmltooling;
using namespace std;

namespace xmlencryption {

    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,KeySize);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,OAEPparams);
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,EncryptionMethod);
        XMLOBJECTVALIDATOR_REQUIRE(EncryptionMethod,Algorithm);
    END_XMLOBJECTVALIDATOR;

};

#define REGISTER_ELEMENT(namespaceURI,cname) \
    q=QName(namespaceURI,cname::LOCAL_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    Validator::registerValidator(q,new cname##SchemaValidator())
    
#define REGISTER_TYPE(namespaceURI,cname) \
    q=QName(namespaceURI,cname::TYPE_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    Validator::registerValidator(q,new cname##SchemaValidator())

void xmlencryption::registerEncryptionClasses()
{
    QName q;
    REGISTER_ELEMENT(XMLConstants::XMLENC_NS,KeySize);
    REGISTER_ELEMENT(XMLConstants::XMLENC_NS,OAEPparams);
    REGISTER_ELEMENT(XMLConstants::XMLENC_NS,EncryptionMethod);
    REGISTER_TYPE(XMLConstants::XMLENC_NS,EncryptionMethod);
}
