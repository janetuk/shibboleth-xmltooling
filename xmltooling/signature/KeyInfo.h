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
 * @file KeyInfo.h
 * 
 * XMLObjects representing XML Digital Signature, version 20020212, KeyInfo element
 * and related content.
 */

#ifndef __xmltooling_keyinfo_h__
#define __xmltooling_keyinfo_h__

#include <xmltooling/ElementProxy.h>
#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/validation/ValidatingXMLObject.h>

#include <typeinfo>

#define DECL_XMLSIGOBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmltooling::XMLConstants::XMLSIG_NS,xmltooling::XMLConstants::XMLSIG_PREFIX)

namespace xmlsignature {

    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,KeyName,Name,XML Digital Signature version 20020212 KeyName element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,MgmtData,Data,XML Digital Signature version 20020212 MgmtData element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Modulus,Value,XML Digital Signature version 20020212 Modulus element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Exponent,Value,XML Digital Signature version 20020212 Exponent element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Seed,Value,XML Digital Signature version 20020212 Seed element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,PgenCounter,Value,XML Digital Signature version 20020212 PgenCounter element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,P,Value,XML Digital Signature version 20020212 P element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Q,Value,XML Digital Signature version 20020212 Q element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,G,Value,XML Digital Signature version 20020212 G element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Y,Value,XML Digital Signature version 20020212 Y element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,J,Value,XML Digital Signature version 20020212 J element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,XPath,Expression,XML Digital Signature version 20020212 XPath element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509IssuerName,Name,XML Digital Signature version 20020212 X509IssuerName element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509SerialNumber,SerialNumber,XML Digital Signature version 20020212 X509SerialNumber element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509SKI,Value,XML Digital Signature version 20020212 X509SKI element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509SubjectName,Name,XML Digital Signature version 20020212 X509SubjectName element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509Certificate,Value,XML Digital Signature version 20020212 X509Certificate element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,X509CRL,Value,XML Digital Signature version 20020212 X509CRL element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,SPKISexp,Value,XML Digital Signature version 20020212 SPKISexp element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,PGPKeyID,ID,XML Digital Signature version 20020212 PGPKeyID element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,PGPKeyPacket,Packet,XML Digital Signature version 20020212 PGPKeyPacket element);

    BEGIN_XMLOBJECT(XMLTOOL_API,DSAKeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 DSAKeyValue element);
        DECL_XMLOBJECT_CHILD(P);
        DECL_XMLOBJECT_CHILD(Q);
        DECL_XMLOBJECT_CHILD(G);
        DECL_XMLOBJECT_CHILD(Y);
        DECL_XMLOBJECT_CHILD(J);
        DECL_XMLOBJECT_CHILD(Seed);
        DECL_XMLOBJECT_CHILD(PgenCounter);
        /** DSAKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,RSAKeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 RSAKeyValue element);
        DECL_XMLOBJECT_CHILD(Modulus);
        DECL_XMLOBJECT_CHILD(Exponent);
        /** RSAKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 KeyValue element);
        DECL_XMLOBJECT_CHILD(DSAKeyValue);
        DECL_XMLOBJECT_CHILD(RSAKeyValue);
        DECL_XMLOBJECT_CHILD(XMLObject);
        DECL_XMLOBJECT_CONTENT(TextContent);
        /** KeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Transform,xmltooling::ElementProxy,XML Digital Signature version 20020212 Transform element);
        DECL_XMLOBJECT_ATTRIB(Algorithm,ALGORITHM);
        DECL_XMLOBJECT_CHILDREN(XPath);
        /** TransformType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Transforms,xmltooling::XMLObject,XML Digital Signature version 20020212 Transforms element);
        DECL_XMLOBJECT_CHILDREN(Transform);
        /** TransformsType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,RetrievalMethod,xmltooling::XMLObject,XML Digital Signature version 20020212 RetrievalMethod element);
        DECL_XMLOBJECT_ATTRIB(URI,URI);
        DECL_XMLOBJECT_ATTRIB(Type,TYPE);
        DECL_XMLOBJECT_CHILD(Transforms);
        /** RetrievalMethodType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,X509IssuerSerial,xmltooling::XMLObject,XML Digital Signature version 20020212 X509IssuerSerial element);
        DECL_XMLOBJECT_CHILD(X509IssuerName);
        DECL_XMLOBJECT_CHILD(X509SerialNumber);
        /** X509IssuerSerialType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,X509Data,xmltooling::XMLObject,XML Digital Signature version 20020212 X509Data element);
        DECL_XMLOBJECT_CHILDREN(X509IssuerSerial);
        DECL_XMLOBJECT_CHILDREN(X509SKI);
        DECL_XMLOBJECT_CHILDREN(X509SubjectName);
        DECL_XMLOBJECT_CHILDREN(X509Certificate);
        DECL_XMLOBJECT_CHILDREN(X509CRL);
        DECL_XMLOBJECT_CHILDREN(XMLObject);
        /** X509DataType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,SPKIData,xmltooling::XMLObject,XML Digital Signature version 20020212 SPKIData element);
        /** SPKIDataType local name */
        static const XMLCh TYPE_NAME[];
        
        /** Returns modifiable collection of SPKIsexp/XMLObject pairs. */
        virtual VectorOfPairs(SPKISexp,xmltooling::XMLObject) getSPKISexps()=0;
        
        /** Returns reference to immutable collection of SPKIsexp/XMLObject pairs. */
        virtual const std::vector< std::pair<SPKISexp*,xmltooling::XMLObject*> >& getSPKISexps() const=0;
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,PGPData,xmltooling::XMLObject,XML Digital Signature version 20020212 PGPData element);
        DECL_XMLOBJECT_CHILD(PGPKeyID);
        DECL_XMLOBJECT_CHILD(PGPKeyPacket);
        DECL_XMLOBJECT_CHILDREN(XMLObject);
        /** PGPDataType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyInfo,xmltooling::ElementProxy,XML Digital Signature version 20020212 KeyInfo element);
        DECL_XMLOBJECT_ATTRIB(Id,ID);
        DECL_XMLOBJECT_CHILDREN(X509Data);
        DECL_XMLOBJECT_CHILDREN(KeyName);
        DECL_XMLOBJECT_CHILDREN(KeyValue);
        DECL_XMLOBJECT_CHILDREN(RetrievalMethod);
        DECL_XMLOBJECT_CHILDREN(MgmtData);
        DECL_XMLOBJECT_CHILDREN(PGPData);
        DECL_XMLOBJECT_CHILDREN(SPKIData);
        /** KeyInfoType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    DECL_XMLSIGOBJECTBUILDER(PGPData);
    DECL_XMLSIGOBJECTBUILDER(PGPKeyID);
    DECL_XMLSIGOBJECTBUILDER(PGPKeyPacket);
    DECL_XMLSIGOBJECTBUILDER(SPKIData);
    DECL_XMLSIGOBJECTBUILDER(SPKISexp);
    DECL_XMLSIGOBJECTBUILDER(X509IssuerSerial);
    DECL_XMLSIGOBJECTBUILDER(X509IssuerName);
    DECL_XMLSIGOBJECTBUILDER(X509SerialNumber);
    DECL_XMLSIGOBJECTBUILDER(X509SKI);
    DECL_XMLSIGOBJECTBUILDER(X509SubjectName);
    DECL_XMLSIGOBJECTBUILDER(X509Certificate);
    DECL_XMLSIGOBJECTBUILDER(X509CRL);
    DECL_XMLSIGOBJECTBUILDER(X509Data);
    DECL_XMLSIGOBJECTBUILDER(XPath);
    DECL_XMLSIGOBJECTBUILDER(Transform);
    DECL_XMLSIGOBJECTBUILDER(Transforms);
    DECL_XMLSIGOBJECTBUILDER(RetrievalMethod);
    DECL_XMLSIGOBJECTBUILDER(KeyName);
    DECL_XMLSIGOBJECTBUILDER(MgmtData);
    DECL_XMLSIGOBJECTBUILDER(Modulus);
    DECL_XMLSIGOBJECTBUILDER(Exponent);
    DECL_XMLSIGOBJECTBUILDER(Seed);
    DECL_XMLSIGOBJECTBUILDER(PgenCounter);
    DECL_XMLSIGOBJECTBUILDER(P);
    DECL_XMLSIGOBJECTBUILDER(Q);
    DECL_XMLSIGOBJECTBUILDER(G);
    DECL_XMLSIGOBJECTBUILDER(Y);
    DECL_XMLSIGOBJECTBUILDER(J);
    DECL_XMLSIGOBJECTBUILDER(DSAKeyValue);
    DECL_XMLSIGOBJECTBUILDER(RSAKeyValue);
    DECL_XMLSIGOBJECTBUILDER(KeyValue);
    DECL_XMLSIGOBJECTBUILDER(KeyInfo);

#ifdef XMLTOOLING_DECLARE_VALIDATORS
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,KeyName,Name);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,MgmtData,Data);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Modulus,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Exponent,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Seed,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PgenCounter,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,P,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Q,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,G,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,Y,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,J,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,XPath,Expression);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509IssuerName,Name);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SerialNumber,SerialNumber);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SKI,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509SubjectName,Name);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509Certificate,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,X509CRL,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,SPKISexp,Value);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyID,ID);
    XMLOBJECTVALIDATOR_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyPacket,Packet);
    
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
        XMLOBJECTVALIDATOR_ONEOF3(KeyValue,DSAKeyValue,RSAKeyValue,XMLObject);
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

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,X509Data);
        if (!ptr->hasChildren())
            throw xmltooling::ValidationException("X509Data must have at least one child element.");
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,SPKIData);
        XMLOBJECTVALIDATOR_NONEMPTY(SPKIData,SPKISexp);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,PGPData);
        XMLOBJECTVALIDATOR_ONEOF(PGPData,PGPKeyID,PGPKeyPacket);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,KeyInfo);
        XMLOBJECTVALIDATOR_NONEMPTY(KeyInfo,XMLObject);
    END_XMLOBJECTVALIDATOR;
#endif /* XMLTOOLING_DECLARE_VALIDATORS */

};

#endif /* __xmltooling_keyinfo_h__ */
