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
 * XMLSecSignatureImpl.cpp
 * 
 * Signature class for XMLSec-based signature-handling
 */

#include "internal.h"
#include "AbstractElementProxy.h"
#include "exceptions.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "signature/impl/KeyInfoImpl.h"
#include "util/XMLHelper.h"
#include "validation/AbstractValidatingXMLObject.h"

#include <typeinfo.h>

using namespace xmltooling;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {
    
    class XMLTOOL_DLLLOCAL KeyInfoImpl
        : public KeyInfo,
            public AbstractDOMCachingXMLObject,
            public AbstractElementProxy,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~KeyInfoImpl() {}

        KeyInfoImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Id(NULL) {
        }
            
        KeyInfoImpl(const KeyInfoImpl& src)
            : AbstractXMLObject(src),
                AbstractDOMCachingXMLObject(src),
                AbstractElementProxy(src),
                AbstractValidatingXMLObject(src),
                m_Id(XMLString::replicate(src.m_Id)) {
                    
            for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                getXMLObjects().push_back((*i) ? (*i)->clone() : NULL);
            }
        }
        
        IMPL_XMLOBJECT_CLONE(KeyInfo);
        IMPL_XMLOBJECT_ATTRIB(Id);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            if(getId()) {
                domElement->setAttributeNS(NULL, ID_ATTRIB_NAME, getId());
                domElement->setIdAttributeNS(NULL, ID_ATTRIB_NAME);
            }
        }

        void marshallElementContent(DOMElement* domElement) const {
            if(getTextContent()) {
                domElement->appendChild(domElement->getOwnerDocument()->createTextNode(getTextContent()));
            }
        }

        void processElementContent(const XMLCh* elementContent) {
            setTextContent(elementContent);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            if (XMLHelper::isNodeNamed(attribute, NULL, ID_ATTRIB_NAME)) {
                setId(attribute->getValue());
                static_cast<DOMElement*>(attribute->getParentNode())->setIdAttributeNode(attribute);
            }
        }
    };
    
    class XMLTOOL_DLLLOCAL KeyNameImpl
        : public KeyName,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~KeyNameImpl() {}

        KeyNameImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Name(NULL) {
        }
            
        KeyNameImpl(const KeyNameImpl& src)
            : AbstractXMLObject(src),
                AbstractDOMCachingXMLObject(src),
                AbstractValidatingXMLObject(src),
                m_Name(XMLString::replicate(src.m_Name)) {
        }
        
        IMPL_XMLOBJECT_CLONE(KeyName);
        IMPL_XMLOBJECT_CONTENT(Name);
    };

    class XMLTOOL_DLLLOCAL MgmtDataImpl
        : public MgmtData,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~MgmtDataImpl() {}

        MgmtDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Data(NULL) {
        }
            
        MgmtDataImpl(const MgmtDataImpl& src)
            : AbstractXMLObject(src),
                AbstractDOMCachingXMLObject(src),
                AbstractValidatingXMLObject(src),
                m_Data(XMLString::replicate(src.m_Data)) {
        }
        
        IMPL_XMLOBJECT_CLONE(MgmtData);
        IMPL_XMLOBJECT_CONTENT(Data);
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(KeyInfo);
IMPL_XMLOBJECTBUILDER(KeyName);
IMPL_XMLOBJECTBUILDER(MgmtData);

// Validators

void KeyInfoSchemaValidator::validate(const XMLObject* xmlObject) const
{
    const KeyInfo* ptr=dynamic_cast<const KeyInfo*>(xmlObject);
    if (!ptr)
        throw ValidationException("KeyInfoSchemaValidator: unsupported object type ($1)",params(1,typeid(xmlObject).name()));
    if (!ptr->hasChildren())
        throw ValidationException("KeyInfo is empty");
}

void KeyNameSchemaValidator::validate(const XMLObject* xmlObject) const
{
    const KeyName* ptr=dynamic_cast<const KeyName*>(xmlObject);
    if (!ptr)
        throw ValidationException("KeyNameSchemaValidator: unsupported object type ($1)",params(1,typeid(xmlObject).name()));
    if (XMLString::stringLen(ptr->getName())==0)
        throw ValidationException("KeyName is empty");
}

void MgmtDataSchemaValidator::validate(const XMLObject* xmlObject) const
{
    const MgmtData* ptr=dynamic_cast<const MgmtData*>(xmlObject);
    if (!ptr)
        throw ValidationException("MgmtDataSchemaValidator: unsupported object type ($1)",params(1,typeid(xmlObject).name()));
    if (XMLString::stringLen(ptr->getData())==0)
        throw ValidationException("MgmtData is empty");
}
