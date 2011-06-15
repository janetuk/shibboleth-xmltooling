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
 * EncryptionImpl.cpp
 * 
 * Implementation classes for XML Encryption schema.
 */

#include "internal.h"
#include "AbstractAttributeExtensibleXMLObject.h"
#include "AbstractComplexElement.h"
#include "AbstractSimpleElement.h"
#include "exceptions.h"
#include "encryption/Encryption.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "signature/KeyInfo.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlencryption;
using namespace xmltooling;
using namespace xercesc;
using namespace std;
using xmlconstants::XMLENC_NS;
using xmlconstants::XMLSIG_NS;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlencryption {

    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,CarriedKeyName);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,CipherValue);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,KeySize);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,OAEPparams);

    class XMLTOOL_DLLLOCAL EncryptionMethodImpl : public virtual EncryptionMethod,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Algorithm=nullptr;
            m_KeySize=nullptr;
            m_OAEPparams=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_KeySize=m_children.begin();
            m_pos_OAEPparams=m_pos_KeySize;
            ++m_pos_OAEPparams;
        }
    public:
        virtual ~EncryptionMethodImpl() {
            XMLString::release(&m_Algorithm);
        }

        EncryptionMethodImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptionMethodImpl(const EncryptionMethodImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            setAlgorithm(src.getAlgorithm());
            if (src.getKeySize())
                setKeySize(src.getKeySize()->cloneKeySize());
            if (src.getOAEPparams())
                setOAEPparams(src.getOAEPparams()->cloneOAEPparams());
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptionMethod);
        IMPL_STRING_ATTRIB(Algorithm);
        IMPL_TYPED_CHILD(KeySize);
        IMPL_TYPED_CHILD(OAEPparams);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Algorithm,ALGORITHM,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(KeySize,XMLENC_NS,false);
            PROC_TYPED_CHILD(OAEPparams,XMLENC_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLENC_NS) && nsURI && *nsURI) {
                getUnknownXMLObjects().push_back(childXMLObject);
                return;
            }
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(Algorithm,ALGORITHM,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL TransformsImpl : public virtual Transforms,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~TransformsImpl() {}

        TransformsImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
        }
            
        TransformsImpl(const TransformsImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            VectorOf(xmlsignature::Transform) v=getTransforms();
            for (vector<xmlsignature::Transform*>::const_iterator i=src.m_Transforms.begin(); i!=src.m_Transforms.end(); i++) {
                if (*i) {
                    v.push_back((*i)->cloneTransform());
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(Transforms);
        IMPL_TYPED_FOREIGN_CHILDREN(Transform,xmlsignature,m_children.end());

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_FOREIGN_CHILDREN(Transform,xmlsignature,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL CipherReferenceImpl : public virtual CipherReference,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_URI=nullptr;
            m_Transforms=nullptr;
            m_children.push_back(nullptr);
            m_pos_Transforms=m_children.begin();
        }
    public:
        virtual ~CipherReferenceImpl() {
            XMLString::release(&m_URI);
        }

        CipherReferenceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        CipherReferenceImpl(const CipherReferenceImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            setURI(src.getURI());
            if (src.getTransforms())
                setTransforms(src.getTransforms()->cloneTransforms());
        }
        
        IMPL_XMLOBJECT_CLONE(CipherReference);
        IMPL_STRING_ATTRIB(URI);
        IMPL_TYPED_CHILD(Transforms);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(URI,URI,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(Transforms,XMLENC_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(URI,URI,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL CipherDataImpl : public virtual CipherData,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_CipherValue=nullptr;
            m_CipherReference=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_CipherValue=m_children.begin();
            m_pos_CipherReference=m_pos_CipherValue;
            ++m_pos_CipherReference;
        }
    public:
        virtual ~CipherDataImpl() {}

        CipherDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        CipherDataImpl(const CipherDataImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            if (src.getCipherValue())
                setCipherValue(src.getCipherValue()->cloneCipherValue());
            if (src.getCipherReference())
                setCipherReference(src.getCipherReference()->cloneCipherReference());
        }
        
        IMPL_XMLOBJECT_CLONE(CipherData);
        IMPL_TYPED_CHILD(CipherValue);
        IMPL_TYPED_CHILD(CipherReference);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(CipherValue,XMLENC_NS,false);
            PROC_TYPED_CHILD(CipherReference,XMLENC_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL EncryptionPropertyImpl : public virtual EncryptionProperty,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Id=m_Target=nullptr;
        }
    public:
        virtual ~EncryptionPropertyImpl() {
            XMLString::release(&m_Id);
            XMLString::release(&m_Target);
        }

        EncryptionPropertyImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptionPropertyImpl(const EncryptionPropertyImpl& src)
                : AbstractXMLObject(src),
                    AbstractAttributeExtensibleXMLObject(src),
                    AbstractComplexElement(src),
                    AbstractDOMCachingXMLObject(src) {
            init();
            setId(src.getId());
            setTarget(src.getTarget());
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptionProperty);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_STRING_ATTRIB(Target);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject, m_children.end());

        void setAttribute(const xmltooling::QName& qualifiedName, const XMLCh* value, bool ID=false) {
            if (!qualifiedName.hasNamespaceURI()) {
                if (XMLString::equals(qualifiedName.getLocalPart(),ID_ATTRIB_NAME)) {
                    setId(value);
                    return;
                }
                else if (XMLString::equals(qualifiedName.getLocalPart(),TARGET_ATTRIB_NAME)) {
                    setTarget(value);
                    return;
                }
            }
            AbstractAttributeExtensibleXMLObject::setAttribute(qualifiedName, value, ID);
        }

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
            MARSHALL_STRING_ATTRIB(Target,TARGET,nullptr);
            marshallExtensionAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getUnknownXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            unmarshallExtensionAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL EncryptionPropertiesImpl : public virtual EncryptionProperties,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Id=nullptr;
        }
    public:
        virtual ~EncryptionPropertiesImpl() {
            XMLString::release(&m_Id);
        }

        EncryptionPropertiesImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptionPropertiesImpl(const EncryptionPropertiesImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            setId(src.getId());
            VectorOf(EncryptionProperty) v=getEncryptionPropertys();
            for (vector<EncryptionProperty*>::const_iterator i=src.m_EncryptionPropertys.begin(); i!=src.m_EncryptionPropertys.end(); i++) {
                if (*i) {
                    v.push_back((*i)->cloneEncryptionProperty());
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptionProperties);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_TYPED_CHILDREN(EncryptionProperty,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(EncryptionProperty,XMLENC_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL ReferenceTypeImpl : public virtual ReferenceType,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_URI=nullptr;
        }
        
    protected:
        ReferenceTypeImpl() {
            init();
        }
        
    public:
        virtual ~ReferenceTypeImpl() {
            XMLString::release(&m_URI);
        }

        ReferenceTypeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        ReferenceTypeImpl(const ReferenceTypeImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            setURI(src.getURI());
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(ReferenceType);
        IMPL_STRING_ATTRIB(URI);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(URI,URI,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getUnknownXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(URI,URI,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL DataReferenceImpl : public virtual DataReference, public ReferenceTypeImpl
    {
    public:
        virtual ~DataReferenceImpl() {}

        DataReferenceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {}
            
        DataReferenceImpl(const DataReferenceImpl& src) : AbstractXMLObject(src), ReferenceTypeImpl(src) {}
        
        IMPL_XMLOBJECT_CLONE(DataReference);
        ReferenceType* cloneReferenceType() const {
            return new DataReferenceImpl(*this);
        }
    };

    class XMLTOOL_DLLLOCAL KeyReferenceImpl : public virtual KeyReference, public ReferenceTypeImpl
    {
    public:
        virtual ~KeyReferenceImpl() {}

        KeyReferenceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {}
            
        KeyReferenceImpl(const KeyReferenceImpl& src) : AbstractXMLObject(src), ReferenceTypeImpl(src) {}
        
        IMPL_XMLOBJECT_CLONE(KeyReference);
        ReferenceType* cloneReferenceType() const {
            return new KeyReferenceImpl(*this);
        }
    };

    class XMLTOOL_DLLLOCAL ReferenceListImpl : public virtual ReferenceList,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~ReferenceListImpl() {}

        ReferenceListImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
        }
            
        ReferenceListImpl(const ReferenceListImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                if (*i) {
                    DataReference* data=dynamic_cast<DataReference*>(*i);
                    if (data) {
                        getDataReferences().push_back(data->cloneDataReference());
                        continue;
                    }

                    KeyReference* key=dynamic_cast<KeyReference*>(*i);
                    if (key) {
                        getKeyReferences().push_back(key->cloneKeyReference());
                        continue;
                    }
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(ReferenceList);
        IMPL_TYPED_CHILDREN(DataReference,m_children.end());
        IMPL_TYPED_CHILDREN(KeyReference,m_children.end());

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(DataReference,XMLENC_NS,false);
            PROC_TYPED_CHILDREN(KeyReference,XMLENC_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL EncryptedTypeImpl : public virtual EncryptedType,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Id=m_Type=m_MimeType=m_Encoding=nullptr;
            m_EncryptionMethod=nullptr;
            m_KeyInfo=nullptr;
            m_CipherData=nullptr;
            m_EncryptionProperties=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_EncryptionMethod=m_children.begin();
            m_pos_KeyInfo=m_pos_EncryptionMethod;
            ++m_pos_KeyInfo;
            m_pos_CipherData=m_pos_KeyInfo;
            ++m_pos_CipherData;
            m_pos_EncryptionProperties=m_pos_CipherData;
            ++m_pos_EncryptionProperties;
        }
    protected:
        EncryptedTypeImpl() {
            init();
        }
        
    public:
        virtual ~EncryptedTypeImpl() {
            XMLString::release(&m_Id);
            XMLString::release(&m_Type);
            XMLString::release(&m_MimeType);
            XMLString::release(&m_Encoding);
        }

        EncryptedTypeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptedTypeImpl(const EncryptedTypeImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            setId(src.getId());
            setType(src.getType());
            setMimeType(src.getMimeType());
            setEncoding(src.getEncoding());
            if (src.getEncryptionMethod())
                setEncryptionMethod(src.getEncryptionMethod()->cloneEncryptionMethod());
            if (src.getKeyInfo())
                setKeyInfo(src.getKeyInfo()->cloneKeyInfo());
            if (src.getCipherData())
                setCipherData(src.getCipherData()->cloneCipherData());
            if (src.getEncryptionProperties())
                setEncryptionProperties(src.getEncryptionProperties()->cloneEncryptionProperties());
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptedType);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_STRING_ATTRIB(Type);
        IMPL_STRING_ATTRIB(MimeType);
        IMPL_STRING_ATTRIB(Encoding);
        IMPL_TYPED_CHILD(EncryptionMethod);
        IMPL_TYPED_FOREIGN_CHILD(KeyInfo,xmlsignature);
        IMPL_TYPED_CHILD(CipherData);
        IMPL_TYPED_CHILD(EncryptionProperties);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
            MARSHALL_STRING_ATTRIB(Type,TYPE,nullptr);
            MARSHALL_STRING_ATTRIB(MimeType,MIMETYPE,nullptr);
            MARSHALL_STRING_ATTRIB(Encoding,ENCODING,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(EncryptionMethod,XMLENC_NS,false);
            PROC_TYPED_FOREIGN_CHILD(KeyInfo,xmlsignature,XMLSIG_NS,false);
            PROC_TYPED_CHILD(CipherData,XMLENC_NS,false);
            PROC_TYPED_CHILD(EncryptionProperties,XMLENC_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            PROC_STRING_ATTRIB(Type,TYPE,nullptr);
            PROC_STRING_ATTRIB(MimeType,MIMETYPE,nullptr);
            PROC_STRING_ATTRIB(Encoding,ENCODING,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL EncryptedDataImpl : public virtual EncryptedData, public EncryptedTypeImpl
    {
    public:
        virtual ~EncryptedDataImpl() {}

        EncryptedDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {}
            
        EncryptedDataImpl(const EncryptedDataImpl& src) : AbstractXMLObject(src), EncryptedTypeImpl(src) {}
        
        IMPL_XMLOBJECT_CLONE(EncryptedData);
        EncryptedType* cloneEncryptedType() const {
            return new EncryptedDataImpl(*this);
        }
    };

    class XMLTOOL_DLLLOCAL EncryptedKeyImpl : public virtual EncryptedKey, public EncryptedTypeImpl
    {
        void init() {
            m_Recipient=nullptr;
            m_ReferenceList=nullptr;
            m_CarriedKeyName=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_ReferenceList=m_pos_EncryptionProperties;
            ++m_pos_ReferenceList;
            m_pos_CarriedKeyName=m_pos_ReferenceList;
            ++m_pos_CarriedKeyName;
        }
        
    public:
        virtual ~EncryptedKeyImpl() {
            XMLString::release(&m_Recipient);
        }

        EncryptedKeyImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptedKeyImpl(const EncryptedKeyImpl& src) : AbstractXMLObject(src), EncryptedTypeImpl(src) {
            init();
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptedKey);
        EncryptedType* cloneEncryptedType() const {
            return new EncryptedKeyImpl(*this);
        }
        IMPL_STRING_ATTRIB(Recipient);
        IMPL_TYPED_CHILD(ReferenceList);
        IMPL_TYPED_CHILD(CarriedKeyName);
    
    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Recipient,RECIPIENT,nullptr);
            EncryptedTypeImpl::marshallAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(ReferenceList,XMLENC_NS,false);
            PROC_TYPED_CHILD(CarriedKeyName,XMLENC_NS,false);
            EncryptedTypeImpl::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(Recipient,RECIPIENT,nullptr);
            EncryptedTypeImpl::processAttribute(attribute);
        }
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(CarriedKeyName);
IMPL_XMLOBJECTBUILDER(CipherData);
IMPL_XMLOBJECTBUILDER(CipherReference);
IMPL_XMLOBJECTBUILDER(CipherValue);
IMPL_XMLOBJECTBUILDER(DataReference);
IMPL_XMLOBJECTBUILDER(EncryptedData);
IMPL_XMLOBJECTBUILDER(EncryptedKey);
IMPL_XMLOBJECTBUILDER(EncryptionMethod);
IMPL_XMLOBJECTBUILDER(EncryptionProperties);
IMPL_XMLOBJECTBUILDER(EncryptionProperty);
IMPL_XMLOBJECTBUILDER(KeyReference);
IMPL_XMLOBJECTBUILDER(KeySize);
IMPL_XMLOBJECTBUILDER(OAEPparams);
IMPL_XMLOBJECTBUILDER(ReferenceList);
IMPL_XMLOBJECTBUILDER(Transforms);

// Unicode literals

const XMLCh CarriedKeyName::LOCAL_NAME[] =              UNICODE_LITERAL_14(C,a,r,r,i,e,d,K,e,y,N,a,m,e);
const XMLCh CipherData::LOCAL_NAME[] =                  UNICODE_LITERAL_10(C,i,p,h,e,r,D,a,t,a);
const XMLCh CipherData::TYPE_NAME[] =                   UNICODE_LITERAL_14(C,i,p,h,e,r,D,a,t,a,T,y,p,e);
const XMLCh CipherReference::LOCAL_NAME[] =             UNICODE_LITERAL_15(C,i,p,h,e,r,R,e,f,e,r,e,n,c,e);
const XMLCh CipherReference::TYPE_NAME[] =              UNICODE_LITERAL_19(C,i,p,h,e,r,R,e,f,e,r,e,n,c,e,T,y,p,e);
const XMLCh CipherReference::URI_ATTRIB_NAME[] =        UNICODE_LITERAL_3(U,R,I);
const XMLCh CipherValue::LOCAL_NAME[] =                 UNICODE_LITERAL_11(C,i,p,h,e,r,V,a,l,u,e);
const XMLCh DataReference::LOCAL_NAME[] =               UNICODE_LITERAL_13(D,a,t,a,R,e,f,e,r,e,n,c,e);
const XMLCh EncryptedData::LOCAL_NAME[] =               UNICODE_LITERAL_13(E,n,c,r,y,p,t,e,d,D,a,t,a);
const XMLCh EncryptedData::TYPE_NAME[] =                UNICODE_LITERAL_17(E,n,c,r,y,p,t,e,d,D,a,t,a,T,y,p,e);
const XMLCh EncryptedKey::LOCAL_NAME[] =                UNICODE_LITERAL_12(E,n,c,r,y,p,t,e,d,K,e,y);
const XMLCh EncryptedKey::TYPE_NAME[] =                 UNICODE_LITERAL_16(E,n,c,r,y,p,t,e,d,K,e,y,T,y,p,e);
const XMLCh EncryptedKey::RECIPIENT_ATTRIB_NAME[] =     UNICODE_LITERAL_9(R,e,c,i,p,i,e,n,t);
const XMLCh EncryptedType::LOCAL_NAME[] =               {chNull};
const XMLCh EncryptedType::TYPE_NAME[] =                UNICODE_LITERAL_13(E,n,c,r,y,p,t,e,d,T,y,p,e);
const XMLCh EncryptedType::ID_ATTRIB_NAME[] =           UNICODE_LITERAL_2(I,d);
const XMLCh EncryptedType::TYPE_ATTRIB_NAME[] =         UNICODE_LITERAL_4(T,y,p,e);
const XMLCh EncryptedType::MIMETYPE_ATTRIB_NAME[] =     UNICODE_LITERAL_8(M,i,m,e,T,y,p,e);
const XMLCh EncryptedType::ENCODING_ATTRIB_NAME[] =     UNICODE_LITERAL_8(E,n,c,o,d,i,n,g);
const XMLCh EncryptionMethod::LOCAL_NAME[] =            UNICODE_LITERAL_16(E,n,c,r,y,p,t,i,o,n,M,e,t,h,o,d);
const XMLCh EncryptionMethod::TYPE_NAME[] =             UNICODE_LITERAL_20(E,n,c,r,y,p,t,i,o,n,M,e,t,h,o,d,T,y,p,e);
const XMLCh EncryptionMethod::ALGORITHM_ATTRIB_NAME[] = UNICODE_LITERAL_9(A,l,g,o,r,i,t,h,m);
const XMLCh EncryptionProperties::LOCAL_NAME[] =        UNICODE_LITERAL_20(E,n,c,r,y,p,t,i,o,n,P,r,o,p,e,r,t,i,e,s);
const XMLCh EncryptionProperties::TYPE_NAME[] =         UNICODE_LITERAL_24(E,n,c,r,y,p,t,i,o,n,P,r,o,p,e,r,t,i,e,s,T,y,p,e);
const XMLCh EncryptionProperties::ID_ATTRIB_NAME[] =    UNICODE_LITERAL_2(I,d);
const XMLCh EncryptionProperty::LOCAL_NAME[] =          UNICODE_LITERAL_18(E,n,c,r,y,p,t,i,o,n,P,r,o,p,e,r,t,y);
const XMLCh EncryptionProperty::TYPE_NAME[] =           UNICODE_LITERAL_22(E,n,c,r,y,p,t,i,o,n,P,r,o,p,e,r,t,y,T,y,p,e);
const XMLCh EncryptionProperty::ID_ATTRIB_NAME[] =      UNICODE_LITERAL_2(I,d);
const XMLCh EncryptionProperty::TARGET_ATTRIB_NAME[] =  UNICODE_LITERAL_6(T,a,r,g,e,t);
const XMLCh KeyReference::LOCAL_NAME[] =                UNICODE_LITERAL_12(K,e,y,R,e,f,e,r,e,n,c,e);
const XMLCh KeySize::LOCAL_NAME[] =                     UNICODE_LITERAL_7(K,e,y,S,i,z,e);
const XMLCh OAEPparams::LOCAL_NAME[] =                  UNICODE_LITERAL_10(O,A,E,P,p,a,r,a,m,s);
const XMLCh ReferenceList::LOCAL_NAME[] =               UNICODE_LITERAL_13(R,e,f,e,r,e,n,c,e,L,i,s,t);
const XMLCh ReferenceType::LOCAL_NAME[] =               {chNull};
const XMLCh ReferenceType::TYPE_NAME[] =                UNICODE_LITERAL_13(R,e,f,e,r,e,n,c,e,T,y,p,e);
const XMLCh ReferenceType::URI_ATTRIB_NAME[] =          UNICODE_LITERAL_3(U,R,I);
const XMLCh Transforms::LOCAL_NAME[] =                  UNICODE_LITERAL_10(T,r,a,n,s,f,o,r,m,s);
const XMLCh Transforms::TYPE_NAME[] =                   UNICODE_LITERAL_14(T,r,a,n,s,f,o,r,m,s,T,y,p,e);
