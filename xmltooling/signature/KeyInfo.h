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

#include <typeinfo.h>

#define DECL_XMLSIGOBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,XMLConstants::XMLSIG_NS,XMLConstants::XMLSIG_PREFIX)

namespace xmltooling {

    /**
     * XMLObject representing XML Digital Signature, version 20020212, KeyName element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,KeyName,XMLObject);
        DECL_XMLOBJECT_CONTENT(Name);
    END_XMLOBJECT;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, MgmtData element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,MgmtData,XMLObject);
        DECL_XMLOBJECT_CONTENT(Data);
    END_XMLOBJECT;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, Modulus element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,Modulus,XMLObject);
        DECL_XMLOBJECT_CONTENT(Value);
    END_XMLOBJECT;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, Exponent element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,Exponent,XMLObject);
        DECL_XMLOBJECT_CONTENT(Value);
    END_XMLOBJECT;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, RSAKeyValue element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,RSAKeyValue,XMLObject);
        DECL_XMLOBJECT_CHILD(Modulus);
        DECL_XMLOBJECT_CHILD(Exponent);
        /** RSAKeyValueType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, KeyInfo element.
     */
    BEGIN_XMLOBJECT(XMLTOOL_API,KeyInfo,ElementProxy);
        DECL_XMLOBJECT_ATTRIB(Id,ID);
        DECL_XMLOBJECT_CHILDREN(KeyName);
        DECL_XMLOBJECT_CHILDREN(MgmtData);
        /** KeyInfoType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    DECL_XMLSIGOBJECTBUILDER(KeyName);
    DECL_XMLSIGOBJECTBUILDER(MgmtData);
    DECL_XMLSIGOBJECTBUILDER(Modulus);
    DECL_XMLSIGOBJECTBUILDER(Exponent);
    DECL_XMLSIGOBJECTBUILDER(RSAKeyValue);
    DECL_XMLSIGOBJECTBUILDER(KeyInfo);
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,KeyName);
        XMLOBJECTVALIDATOR_REQUIRE(KeyName,Name);
    END_XMLOBJECTVALIDATOR;
    
    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,MgmtData);
        XMLOBJECTVALIDATOR_REQUIRE(MgmtData,Data);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Modulus);
        XMLOBJECTVALIDATOR_REQUIRE(Modulus,Value);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,Exponent);
        XMLOBJECTVALIDATOR_REQUIRE(Exponent,Value);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,RSAKeyValue);
        XMLOBJECTVALIDATOR_REQUIRE(RSAKeyValue,Modulus);
        XMLOBJECTVALIDATOR_REQUIRE(RSAKeyValue,Exponent);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(XMLTOOL_DLLLOCAL,KeyInfo);
        XMLOBJECTVALIDATOR_CHECKEMPTY(KeyInfo,XMLObject);
    END_XMLOBJECTVALIDATOR;

};

#endif /* __xmltooling_keyinfo_h__ */
