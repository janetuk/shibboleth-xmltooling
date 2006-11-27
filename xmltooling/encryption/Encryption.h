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
 * @file xmltooling/encryption/Encryption.h
 * 
 * XMLObjects representing XML Encryption content
 */

#ifndef __xmltooling_encryption_h__
#define __xmltooling_encryption_h__

#include <xmltooling/signature/KeyInfo.h>

#define DECL_XMLENCOBJECTBUILDER(cname) \
    DECL_XMLOBJECTBUILDER(XMLTOOL_API,cname,xmlconstants::XMLENC_NS,xmlconstants::XMLENC_PREFIX)

/**
 * @namespace xmlencryption
 * Namespace for XML Encryption schema objects
 */
namespace xmlencryption {

    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,CarriedKeyName,Name,XML Encryption CarriedKeyName element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,CipherValue,Value,XML Encryption CipherValue element);
    DECL_XMLOBJECT_SIMPLE(XMLTOOL_API,OAEPparams,Name,XML Encryption OAEPparams element);

    BEGIN_XMLOBJECT(XMLTOOL_API,KeySize,xmltooling::XMLObject,XML Encryption KeySize element);
        DECL_INTEGER_CONTENT(Size);
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptionMethod,xmltooling::ElementExtensibleXMLObject,XML Encryption EncryptionMethod element);
        DECL_STRING_ATTRIB(Algorithm,ALGORITHM);
        DECL_TYPED_CHILD(KeySize);
        DECL_TYPED_CHILD(OAEPparams);
        /** EncryptionMethodType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,Transforms,xmltooling::XMLObject,XML Encryption Transforms element);
        DECL_TYPED_FOREIGN_CHILDREN(Transform,xmlsignature);
        /** TransformsType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,CipherReference,xmltooling::XMLObject,XML Encryption CipherReference element);
        DECL_STRING_ATTRIB(URI,URI);
        DECL_TYPED_CHILD(Transforms);
        /** CipherReferenceType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,CipherData,xmltooling::XMLObject,XML Encryption CipherData element);
        DECL_TYPED_CHILD(CipherValue);
        DECL_TYPED_CHILD(CipherReference);
        /** CipherDataType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptionProperty,xmltooling::ElementProxy,XML Encryption EncryptionProperty element);
        DECL_STRING_ATTRIB(Target,TARGET);
        DECL_STRING_ATTRIB(Id,ID);
        /** EncryptionPropertyType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptionProperties,xmltooling::XMLObject,XML Encryption EncryptionProperties element);
        DECL_STRING_ATTRIB(Id,ID);
        DECL_TYPED_CHILDREN(EncryptionProperty);
        /** EncryptionPropertiesType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,ReferenceType,xmltooling::ElementExtensibleXMLObject,XML Encryption ReferenceType type);
        DECL_STRING_ATTRIB(URI,URI);
        /** ReferenceType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,DataReference,ReferenceType,XML Encryption DataReference element);
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,KeyReference,ReferenceType,XML Encryption KeyReference element);
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,ReferenceList,xmltooling::XMLObject,XML Encryption ReferenceList element);
        DECL_TYPED_CHILDREN(DataReference);
        DECL_TYPED_CHILDREN(KeyReference);
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptedType,xmltooling::XMLObject,XML Encryption EncryptedType abstract type);
        DECL_STRING_ATTRIB(Id,ID);
        DECL_STRING_ATTRIB(Type,TYPE);
        DECL_STRING_ATTRIB(MimeType,MIMETYPE);
        DECL_STRING_ATTRIB(Encoding,ENCODING);
        DECL_TYPED_CHILD(EncryptionMethod);
        DECL_TYPED_FOREIGN_CHILD(KeyInfo,xmlsignature);
        DECL_TYPED_CHILD(CipherData);
        DECL_TYPED_CHILD(EncryptionProperties);
        /** EncryptedType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptedData,EncryptedType,XML Encryption EncryptedData element);
        /** EncryptedDataType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECT(XMLTOOL_API,EncryptedKey,EncryptedType,XML Encryption EncryptedKey element);
        DECL_STRING_ATTRIB(Recipient,RECIPIENT);
        DECL_TYPED_CHILD(ReferenceList);
        DECL_TYPED_CHILD(CarriedKeyName);
        /** EncryptedKeyType local name */
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    DECL_XMLENCOBJECTBUILDER(CarriedKeyName);
    DECL_XMLENCOBJECTBUILDER(CipherData);
    DECL_XMLENCOBJECTBUILDER(CipherReference);
    DECL_XMLENCOBJECTBUILDER(CipherValue);
    DECL_XMLENCOBJECTBUILDER(DataReference);
    DECL_XMLENCOBJECTBUILDER(EncryptedData);
    DECL_XMLENCOBJECTBUILDER(EncryptedKey);
    DECL_XMLENCOBJECTBUILDER(EncryptionMethod);
    DECL_XMLENCOBJECTBUILDER(EncryptionProperties);
    DECL_XMLENCOBJECTBUILDER(EncryptionProperty);
    DECL_XMLENCOBJECTBUILDER(KeyReference);
    DECL_XMLENCOBJECTBUILDER(KeySize);
    DECL_XMLENCOBJECTBUILDER(OAEPparams);
    DECL_XMLENCOBJECTBUILDER(ReferenceList);
    DECL_XMLENCOBJECTBUILDER(Transforms);

    /**
     * Registers builders and validators for XML Encryption classes into the runtime.
     */
    void XMLTOOL_API registerEncryptionClasses();
};

#endif /* __xmltooling_encryption_h__ */
