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

/**
 * EncryptionSchemaValidators.cpp
 * 
 * Schema validators for XML Encryption schema
 */

#include "internal.h"
#include "exceptions.h"
#include "encryption/Encryption.h"
#include "validation/Validator.h"
#include "validation/ValidatorSuite.h"

using namespace xmlencryption;
using namespace xmltooling;
using namespace std;
using xmlconstants::XMLENC_NS;
using xmlconstants::XMLENC11_NS;

namespace xmlencryption {

    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,CarriedKeyName);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,CipherValue);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,KeySize);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,OAEPparams);
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,EncryptionMethod);
        XMLOBJECTVALIDATOR_REQUIRE(EncryptionMethod,Algorithm);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Transforms);
        XMLOBJECTVALIDATOR_NONEMPTY(Transforms,Transform);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,CipherReference);
        XMLOBJECTVALIDATOR_REQUIRE(CipherReference,URI);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,CipherData);
        XMLOBJECTVALIDATOR_ONLYONEOF(CipherData,CipherValue,CipherReference);
    END_XMLOBJECTVALIDATOR;

    class XMLTOOL_DLLLOCAL checkWildcardNS {
    public:
        void operator()(const XMLObject* xmlObject) const {
            const XMLCh* ns=xmlObject->getElementQName().getNamespaceURI();
            if (XMLString::equals(ns,XMLENC_NS) || !ns || !*ns) {
                throw ValidationException(
                    "Object contains an illegal extension child element ($1).",
                    params(1,xmlObject->getElementQName().toString().c_str())
                    );
            }
        }
    };

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,EncryptionProperty);
        if (!ptr->hasChildren())
            throw ValidationException("EncryptionProperty must have at least one child element.");
        const vector<XMLObject*>& anys=ptr->getUnknownXMLObjects();
        for_each(anys.begin(),anys.end(),checkWildcardNS());
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,EncryptionProperties);
        XMLOBJECTVALIDATOR_NONEMPTY(EncryptionProperties,EncryptionProperty);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,ReferenceType);
        XMLOBJECTVALIDATOR_REQUIRE(DataReference,URI);
        const vector<XMLObject*>& anys=ptr->getUnknownXMLObjects();
        for_each(anys.begin(),anys.end(),checkWildcardNS());
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR_SUB(XMLTOOL_DLLLOCAL,DataReference,ReferenceType);
        ReferenceTypeSchemaValidator::validate(xmlObject);
    END_XMLOBJECTVALIDATOR;
    
    BEGIN_XMLOBJECTVALIDATOR_SUB(XMLTOOL_DLLLOCAL,KeyReference,ReferenceType);
        ReferenceTypeSchemaValidator::validate(xmlObject);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,ReferenceList);
        if (!ptr->hasChildren())
            throw ValidationException("ReferenceList must have at least one child element.");
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,EncryptedType);
        XMLOBJECTVALIDATOR_REQUIRE(EncryptedType,CipherData);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR_SUB(XMLTOOL_DLLLOCAL,EncryptedData,EncryptedType);
        EncryptedTypeSchemaValidator::validate(xmlObject);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR_SUB(XMLTOOL_DLLLOCAL,EncryptedKey,EncryptedType);
        EncryptedTypeSchemaValidator::validate(xmlObject);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,MGF);
        XMLOBJECTVALIDATOR_REQUIRE(MGF,Algorithm);
    END_XMLOBJECTVALIDATOR;

};

#define REGISTER_ELEMENT(namespaceURI,cname) \
    q=QName(namespaceURI,cname::LOCAL_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    SchemaValidators.registerValidator(q,new cname##SchemaValidator())
    
#define REGISTER_TYPE(namespaceURI,cname) \
    q=QName(namespaceURI,cname::TYPE_NAME); \
    XMLObjectBuilder::registerBuilder(q,new cname##Builder()); \
    SchemaValidators.registerValidator(q,new cname##SchemaValidator())

void xmlencryption::registerEncryptionClasses()
{
    QName q;
    REGISTER_ELEMENT(XMLENC_NS,CarriedKeyName);
    REGISTER_ELEMENT(XMLENC_NS,CipherData);
    REGISTER_ELEMENT(XMLENC_NS,CipherReference);
    REGISTER_ELEMENT(XMLENC_NS,CipherValue);
    REGISTER_ELEMENT(XMLENC_NS,DataReference);
    REGISTER_ELEMENT(XMLENC_NS,EncryptedData);
    REGISTER_ELEMENT(XMLENC_NS,EncryptedKey);
    REGISTER_ELEMENT(XMLENC_NS,EncryptionMethod);
    REGISTER_ELEMENT(XMLENC_NS,EncryptionProperties);
    REGISTER_ELEMENT(XMLENC_NS,EncryptionProperty);
    REGISTER_ELEMENT(XMLENC_NS,KeyReference);
    REGISTER_ELEMENT(XMLENC_NS,KeySize);
    REGISTER_ELEMENT(XMLENC_NS,OAEPparams);
    REGISTER_ELEMENT(XMLENC_NS,ReferenceList);
    REGISTER_ELEMENT(XMLENC_NS,Transforms);
    REGISTER_TYPE(XMLENC_NS,CipherData);
    REGISTER_TYPE(XMLENC_NS,CipherReference);
    REGISTER_TYPE(XMLENC_NS,EncryptionMethod);
    REGISTER_TYPE(XMLENC_NS,EncryptionProperties);
    REGISTER_TYPE(XMLENC_NS,EncryptionProperty);
    REGISTER_TYPE(XMLENC_NS,Transforms);

    REGISTER_ELEMENT(XMLENC11_NS,MGF);
    REGISTER_TYPE(XMLENC11_NS,MGF);
}
