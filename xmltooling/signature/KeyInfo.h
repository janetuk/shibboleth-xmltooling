/*
 *  Copyright 2001-2007 Internet2
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
 * @file xmltooling/signature/KeyInfo.h
 * 
 * XMLObjects representing XML Digital Signature, version 20020212, KeyInfo element
 * and related content.
 */

#if !defined(__xmltooling_keyinfo_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keyinfo_h__

#include <xmltooling/ConcreteXMLObjectBuilder.h>
#include <xmltooling/ElementProxy.h>
#include <xmltooling/util/XMLConstants.h>

/**
 * Macro for declaring signature builders.
 * 
 * @param cname name of class being built
 */
#define DECL_XMLSIGOBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmlconstants::XMLSIG_NS,xmlconstants::XMLSIG_PREFIX)

/**
 * Macro for declaring signature builders.
 * 
 * @param cname name of class being built
 */
#define DECL_XMLSIG11OBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmlconstants::XMLSIG11_NS,xmlconstants::XMLSIG11_PREFIX)

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
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,OCSPResponse,Response,XML Digital Signature version 1.1 OCSPResponse element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,SPKISexp,Value,XML Digital Signature version 20020212 SPKISexp element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,PGPKeyID,ID,XML Digital Signature version 20020212 PGPKeyID element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,PGPKeyPacket,Packet,XML Digital Signature version 20020212 PGPKeyPacket element);

    BEGIN_XMLOBJECT(XMLTOOL_API,DSAKeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 DSAKeyValue element);
        DECL_TYPED_CHILD(P);
        DECL_TYPED_CHILD(Q);
        DECL_TYPED_CHILD(G);
        DECL_TYPED_CHILD(Y);
        DECL_TYPED_CHILD(J);
        DECL_TYPED_CHILD(Seed);
        DECL_TYPED_CHILD(PgenCounter);
        /** DSAKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,RSAKeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 RSAKeyValue element);
        DECL_TYPED_CHILD(Modulus);
        DECL_TYPED_CHILD(Exponent);
        /** RSAKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyValue,xmltooling::XMLObject,XML Digital Signature version 20020212 KeyValue element);
        DECL_TYPED_CHILD(DSAKeyValue);
        DECL_TYPED_CHILD(RSAKeyValue);
        DECL_XMLOBJECT_CHILD(UnknownXMLObject);
        /** KeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,DEREncodedKeyValue,xmltooling::XMLObject,XML Digital Signature version 1.1 DEREncodedKeyValue element);
        DECL_STRING_ATTRIB(Id,ID);
        DECL_SIMPLE_CONTENT(Value);
        /** DEREncodedKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Transform,xmltooling::ElementExtensibleXMLObject,XML Digital Signature version 20020212 Transform element);
        DECL_STRING_ATTRIB(Algorithm,ALGORITHM);
        DECL_TYPED_CHILDREN(XPath);
        /** TransformType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Transforms,xmltooling::XMLObject,XML Digital Signature version 20020212 Transforms element);
        DECL_TYPED_CHILDREN(Transform);
        /** TransformsType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,RetrievalMethod,xmltooling::XMLObject,XML Digital Signature version 20020212 RetrievalMethod element);
        DECL_STRING_ATTRIB(URI,URI);
        DECL_STRING_ATTRIB(Type,TYPE);
        DECL_TYPED_CHILD(Transforms);
        /** RetrievalMethodType local name */
        static const XMLCh TYPE_NAME[];
        /** DSAKeyValue RetrievalMethod Type */
        static const XMLCh TYPE_DSAKEYVALUE[];
        /** RSAKeyValue RetrievalMethod Type */
        static const XMLCh TYPE_RSAKEYVALUE[];
        /** X509Data RetrievalMethod Type */
        static const XMLCh TYPE_X509DATA[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,X509IssuerSerial,xmltooling::XMLObject,XML Digital Signature version 20020212 X509IssuerSerial element);
        DECL_TYPED_CHILD(X509IssuerName);
        DECL_TYPED_CHILD(X509SerialNumber);
        /** X509IssuerSerialType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,X509Data,xmltooling::ElementExtensibleXMLObject,XML Digital Signature version 20020212 X509Data element);
        DECL_TYPED_CHILDREN(X509IssuerSerial);
        DECL_TYPED_CHILDREN(X509SKI);
        DECL_TYPED_CHILDREN(X509SubjectName);
        DECL_TYPED_CHILDREN(X509Certificate);
        DECL_TYPED_CHILDREN(X509CRL);
        DECL_TYPED_CHILDREN(OCSPResponse);
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

    BEGIN_XMLOBJECT(XMLTOOL_API,PGPData,xmltooling::ElementExtensibleXMLObject,XML Digital Signature version 20020212 PGPData element);
        DECL_TYPED_CHILD(PGPKeyID);
        DECL_TYPED_CHILD(PGPKeyPacket);
        /** PGPDataType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyInfoReference,xmltooling::XMLObject,XML Digital Signature version 1.1 KeyInfoReference element);
        DECL_STRING_ATTRIB(Id,ID);
        DECL_STRING_ATTRIB(URI,URI);
        /** KeyInfoReferenceType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyInfo,xmltooling::ElementExtensibleXMLObject,XML Digital Signature version 20020212 KeyInfo element);
        DECL_STRING_ATTRIB(Id,ID);
        DECL_TYPED_CHILDREN(X509Data);
        DECL_TYPED_CHILDREN(KeyName);
        DECL_TYPED_CHILDREN(KeyValue);
        DECL_TYPED_CHILDREN(DEREncodedKeyValue);
        DECL_TYPED_CHILDREN(RetrievalMethod);
        DECL_TYPED_CHILDREN(MgmtData);
        DECL_TYPED_CHILDREN(PGPData);
        DECL_TYPED_CHILDREN(SPKIData);
        DECL_TYPED_CHILDREN(KeyInfoReference);
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

    DECL_XMLSIG11OBJECTBUILDER(DEREncodedKeyValue);
    DECL_XMLSIG11OBJECTBUILDER(KeyInfoReference);
    DECL_XMLSIG11OBJECTBUILDER(OCSPResponse);

    /**
     * Registers builders and validators for KeyInfo classes into the runtime.
     */
    void XMLTOOL_API registerKeyInfoClasses();

};

#endif /* __xmltooling_keyinfo_h__ */
