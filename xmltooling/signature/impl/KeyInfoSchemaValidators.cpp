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
 * KeyInfoSchemaValidators.cpp
 * 
 * Schema validators for KeyInfo schema
 */

#include "internal.h"
#include "exceptions.h"
#include "signature/KeyInfo.h"
#include "validation/ValidatorSuite.h"

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;
using xmlconstants::XMLSIG_NS;

namespace xmlsignature {

    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,KeyName);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,MgmtData);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Modulus);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Exponent);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Seed);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PgenCounter);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,P);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Q);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,G);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Y);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,J);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,XPath);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509IssuerName);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SerialNumber);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SKI);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SubjectName);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509Certificate);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509CRL);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,SPKISexp);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyID);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyPacket);
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,RSAKeyValue);
        XMLOBJECTVALIDATOR_REQUIRE(RSAKeyValue,Modulus);
        XMLOBJECTVALIDATOR_REQUIRE(RSAKeyValue,Exponent);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,DSAKeyValue);
        XMLOBJECTVALIDATOR_REQUIRE(DSAKeyValue,Y);
        XMLOBJECTVALIDATOR_NONEORBOTH(DSKeyValue,P,Q);
        XMLOBJECTVALIDATOR_NONEORBOTH(DSKeyValue,Seed,PgenCounter);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,KeyValue);
        XMLOBJECTVALIDATOR_ONLYONEOF3(KeyValue,DSAKeyValue,RSAKeyValue,OtherKeyValue);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Transform);
        XMLOBJECTVALIDATOR_REQUIRE(Transform,Algorithm);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Transforms);
        XMLOBJECTVALIDATOR_NONEMPTY(Transforms,Transform);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,RetrievalMethod);
        XMLOBJECTVALIDATOR_REQUIRE(RetrievalMethod,URI);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,X509IssuerSerial);
        XMLOBJECTVALIDATOR_REQUIRE(X509IssuerSerial,X509IssuerName);
        XMLOBJECTVALIDATOR_REQUIRE(X509IssuerSerial,X509SerialNumber);
    END_XMLOBJECTVALIDATOR;

    class XMLTOOL_DLLLOCAL checkWildcardNS {
    public:
        void operator()(const XMLObject* xmlObject) const {
            const XMLCh* ns=xmlObject->getElementQName().getNamespaceURI();
            if (XMLString::equals(ns,XMLSIG_NS) || !ns || !*ns) {
                throw ValidationException(
                    "Object contains an illegal extension child element ($1).",
                    params(1,xmlObject->getElementQName().toString().c_str())
                    );
            }
        }
    };
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,X509Data);
        if (!ptr->hasChildren())
            throw ValidationException("X509Data must have at least one child element.");
        const vector<XMLObject*>& anys=ptr->getOtherX509Datas();
        for_each(anys.begin(),anys.end(),checkWildcardNS());
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,SPKIData);
        XMLOBJECTVALIDATOR_NONEMPTY(SPKIData,SPKISexp);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,PGPData);
        XMLOBJECTVALIDATOR_ONEOF(PGPData,PGPKeyID,PGPKeyPacket);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,KeyInfo);
        if (!ptr->hasChildren())
            throw ValidationException("KeyInfo must have at least one child element.");
        const vector<XMLObject*>& anys=ptr->getOthers();
        for_each(anys.begin(),anys.end(),checkWildcardNS());
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

void xmlsignature::registerKeyInfoClasses()
{
    QName q;
    REGISTER_ELEMENT(XMLSIG_NS,KeyInfo);
    REGISTER_ELEMENT(XMLSIG_NS,KeyName);
    REGISTER_ELEMENT(XMLSIG_NS,KeyValue);
    REGISTER_ELEMENT(XMLSIG_NS,MgmtData);
    REGISTER_ELEMENT(XMLSIG_NS,DSAKeyValue);
    REGISTER_ELEMENT(XMLSIG_NS,RSAKeyValue);
    REGISTER_ELEMENT(XMLSIG_NS,Exponent);
    REGISTER_ELEMENT(XMLSIG_NS,Modulus);
    REGISTER_ELEMENT(XMLSIG_NS,P);
    REGISTER_ELEMENT(XMLSIG_NS,Q);
    REGISTER_ELEMENT(XMLSIG_NS,G);
    REGISTER_ELEMENT(XMLSIG_NS,Y);
    REGISTER_ELEMENT(XMLSIG_NS,J);
    REGISTER_ELEMENT(XMLSIG_NS,Seed);
    REGISTER_ELEMENT(XMLSIG_NS,PgenCounter);
    REGISTER_ELEMENT(XMLSIG_NS,XPath);
    REGISTER_ELEMENT(XMLSIG_NS,Transform);
    REGISTER_ELEMENT(XMLSIG_NS,Transforms);
    REGISTER_ELEMENT(XMLSIG_NS,RetrievalMethod);
    REGISTER_ELEMENT(XMLSIG_NS,X509IssuerSerial);
    REGISTER_ELEMENT(XMLSIG_NS,X509IssuerName);
    REGISTER_ELEMENT(XMLSIG_NS,X509SerialNumber);
    REGISTER_ELEMENT(XMLSIG_NS,X509SKI);
    REGISTER_ELEMENT(XMLSIG_NS,X509SubjectName);
    REGISTER_ELEMENT(XMLSIG_NS,X509Certificate);
    REGISTER_ELEMENT(XMLSIG_NS,X509CRL);
    REGISTER_ELEMENT(XMLSIG_NS,X509Data);
    REGISTER_ELEMENT(XMLSIG_NS,SPKISexp);
    REGISTER_ELEMENT(XMLSIG_NS,SPKIData);
    REGISTER_ELEMENT(XMLSIG_NS,PGPKeyID);
    REGISTER_ELEMENT(XMLSIG_NS,PGPKeyPacket);
    REGISTER_ELEMENT(XMLSIG_NS,PGPData);
    REGISTER_TYPE(XMLSIG_NS,KeyInfo);
    REGISTER_TYPE(XMLSIG_NS,KeyValue);
    REGISTER_TYPE(XMLSIG_NS,DSAKeyValue);
    REGISTER_TYPE(XMLSIG_NS,RSAKeyValue);
    REGISTER_TYPE(XMLSIG_NS,Transform);
    REGISTER_TYPE(XMLSIG_NS,Transforms);
    REGISTER_TYPE(XMLSIG_NS,RetrievalMethod);
    REGISTER_TYPE(XMLSIG_NS,X509IssuerSerial);
    REGISTER_TYPE(XMLSIG_NS,X509Data);
    REGISTER_TYPE(XMLSIG_NS,SPKIData);
    REGISTER_TYPE(XMLSIG_NS,PGPData);
}
