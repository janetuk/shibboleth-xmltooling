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
 * @file Encryption.h
 * 
 * XMLObjects representing XML Encryption content
 */

#ifndef __xmltooling_encrypt_h__
#define __xmltooling_encrypt_h__

#include <xmltooling/ElementProxy.h>
#include <xmltooling/SimpleElement.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/validation/ValidatingXMLObject.h>

#define DECL_XMLENCOBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmltooling::XMLConstants::XMLENC_NS,xmltooling::XMLConstants::XMLENC_PREFIX)

/**
 * @namespace xmlencryption
 * Namespace for XML Encryption schema objects
 */
namespace xmlencryption {

    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,OAEPparams,Name,XML Encryption OAEPparams element);

    BEGIN_XMLOBJECT(XMLTOOL_API,KeySize,xmltooling::SimpleElement,XML Encryption KeySize element);
        DECL_INTEGER_CONTENT(Size);
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptionMethod,xmltooling::XMLObject,XML Encryption EncryptionMethod element);
        DECL_STRING_ATTRIB(Algorithm,ALGORITHM);
        DECL_TYPED_CHILD(KeySize);
        DECL_TYPED_CHILD(OAEPparams);
        DECL_XMLOBJECT_CHILDREN(OtherParameter);
        /** EncryptionMethodType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    DECL_XMLENCOBJECTBUILDER(OAEPparams);
    DECL_XMLENCOBJECTBUILDER(KeySize);
    DECL_XMLENCOBJECTBUILDER(EncryptionMethod);

    /**
     * Registers builders and validators for XML Encryption classes into the runtime.
     */
    void XMLTOOL_API registerEncryptionClasses();
};

#endif /* __xmltooling_encrypt_h__ */
