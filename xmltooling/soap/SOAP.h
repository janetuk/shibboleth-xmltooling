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
 * @file xmltooling/soap/SOAP.h
 * 
 * XMLObjects representing SOAP content
 */

#ifndef __xmltooling_soap_h__
#define __xmltooling_soap_h__

#include <xmltooling/ElementProxy.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/util/XMLConstants.h>
#include <xercesc/util/XMLUniDefs.hpp>

/**
 * Macro for declaring SOAP builders.
 * 
 * @param cname name of class being built
 */
#define DECL_SOAP11OBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmlconstants::SOAP11ENV_NS,xmlconstants::SOAP11ENV_PREFIX)

/**
 * @namespace soap11
 * Namespace for SOAP 1.1 schema objects
 */
namespace soap11 {

    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Faultstring,String,SOAP 1.1 faultstring element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,Faultactor,Actor,SOAP 1.1 faultactor element);

    BEGIN_XMLOBJECT(XMLTOOL_API,Faultcode,xmltooling::XMLObject,SOAP 1.1 faultcode element);
        /** Gets the QName content of the element. */
        virtual const xmltooling::QName* getCode() const=0;
        /** Sets the QName content of the element. */
        virtual void setCode(const xmltooling::QName* qname)=0;
        /** Client Fault code. **/
        static xmltooling::QName CLIENT;
        /** Server Fault code. **/
        static xmltooling::QName SERVER;
        /** MustUnderstand Fault code. **/
        static xmltooling::QName MUSTUNDERSTAND;
        /** Version Mismatch Fault code. **/
        static xmltooling::QName VERSIONMISMATCH;
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Detail,xmltooling::ElementProxy,SOAP 1.1 detail element);
        /** detail (type) local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Fault,xmltooling::XMLObject,SOAP 1.1 Fault element);
        DECL_TYPED_CHILD(Faultcode);
        DECL_TYPED_CHILD(Faultstring);
        DECL_TYPED_CHILD(Faultactor);
        DECL_TYPED_CHILD(Detail);
        /** Fault (type) local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Body,xmltooling::ElementProxy,SOAP 1.1 Body element);
        DECL_STRING_ATTRIB(EncodingStyle,ENCODINGSTYLE);
        /** Body (type) local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Header,xmltooling::ElementProxy,SOAP 1.1 Header element);
        DECL_BOOLEAN_ATTRIB(MustUnderstand,MUSTUNDERSTAND,false);
        DECL_STRING_ATTRIB(Actor,ACTOR);
        /** Header (type) local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Envelope,xmltooling::AttributeExtensibleXMLObject,SOAP 1.1 Envelope element);
        DECL_TYPED_CHILD(Header);
        DECL_TYPED_CHILD(Body);
        /** Envelope (type) local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    DECL_SOAP11OBJECTBUILDER(Body);
    DECL_SOAP11OBJECTBUILDER(Envelope);
    DECL_SOAP11OBJECTBUILDER(Fault);
    DECL_SOAP11OBJECTBUILDER(Header);
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,Detail,NULL,NULL);
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,Faultactor,NULL,NULL);
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,Faultcode,NULL,NULL);
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,Faultstring,NULL,NULL);

    /**
     * Registers builders and validators for SOAP 1.1 classes into the runtime.
     */
    void XMLTOOL_API registerSOAPClasses();
};

#endif /* __xmltooling_soap_h__ */
