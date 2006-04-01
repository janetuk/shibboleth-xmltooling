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
#include "signature/KeyInfo.h"
#include "util/XMLHelper.h"
#include "validation/AbstractValidatingXMLObject.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {
    
    class XMLTOOL_DLLLOCAL RSAKeyValueImpl
        : public RSAKeyValue,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~RSAKeyValueImpl() {}

        RSAKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        RSAKeyValueImpl(const RSAKeyValueImpl& src)
            : AbstractXMLObject(src),
                AbstractDOMCachingXMLObject(src),
                AbstractValidatingXMLObject(src) {
            init();
            setModulus(src.getModulus());
            setExponent(src.getExponent());
        }
        
        void init() {
            m_Modulus=NULL;
            m_Exponent=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_Modulus=m_children.begin();
            m_pos_Exponent=m_children.begin();
            m_pos_Exponent++;
        }
        
        IMPL_XMLOBJECT_CLONE(RSAKeyValue);
        IMPL_XMLOBJECT_CHILD(Modulus);
        IMPL_XMLOBJECT_CHILD(Exponent);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_XMLOBJECT_CHILD(Modulus,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(Exponent,XMLConstants::XMLSIG_NS);
            
            throw UnmarshallingException("Invalid child element: $1",params(1,childXMLObject->getElementQName().toString().c_str()));
        }
    };
    
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
                if (*i) {
                    KeyName* kn=dynamic_cast<KeyName*>(*i);
                    if (kn) {
                        getKeyNames().push_back(kn->cloneKeyName());
                        continue;
                    }
                    MgmtData* md=dynamic_cast<MgmtData*>(*i);
                    if (md) {
                        getMgmtDatas().push_back(md->cloneMgmtData());
                        continue;
                    }
                    getXMLObjects().push_back((*i)->clone());
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(KeyInfo);
        IMPL_XMLOBJECT_ATTRIB(Id);
        IMPL_XMLOBJECT_CHILDREN(KeyName,m_children.end());
        IMPL_XMLOBJECT_CHILDREN(MgmtData,m_children.end());

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
            PROC_XMLOBJECT_CHILDREN(KeyName,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILDREN(MgmtData,XMLConstants::XMLSIG_NS);
            
            // Unknown child.
            const XMLCh* nsURI=childXMLObject->getElementQName().getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLConstants::XMLSIG_NS) && nsURI && *nsURI)
                getXMLObjects().push_back(childXMLObject);
            
            throw UnmarshallingException("Invalid child element: $1",params(1,childXMLObject->getElementQName().toString().c_str()));
        }

        void processAttribute(const DOMAttr* attribute) {
            if (XMLHelper::isNodeNamed(attribute, NULL, ID_ATTRIB_NAME)) {
                setId(attribute->getValue());
                static_cast<DOMElement*>(attribute->getParentNode())->setIdAttributeNode(attribute);
            }
        }
    };
    
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,KeyName,Name);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,MgmtData,Data);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Modulus,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Exponent,Value);
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(KeyInfo);
IMPL_XMLOBJECTBUILDER(KeyName);
IMPL_XMLOBJECTBUILDER(MgmtData);
IMPL_XMLOBJECTBUILDER(Modulus);
IMPL_XMLOBJECTBUILDER(Exponent);
IMPL_XMLOBJECTBUILDER(RSAKeyValue);

const XMLCh KeyInfo::LOCAL_NAME[] =         UNICODE_LITERAL_7(K,e,y,I,n,f,o);
const XMLCh KeyInfo::TYPE_NAME[] =          UNICODE_LITERAL_11(K,e,y,I,n,f,o,T,y,p,e);
const XMLCh KeyInfo::ID_ATTRIB_NAME[] =     UNICODE_LITERAL_2(I,d);
const XMLCh MgmtData::LOCAL_NAME[] =        UNICODE_LITERAL_8(M,g,m,t,D,a,t,a);
const XMLCh KeyName::LOCAL_NAME[] =         UNICODE_LITERAL_7(K,e,y,N,a,m,e);
const XMLCh Modulus::LOCAL_NAME[] =         UNICODE_LITERAL_7(M,o,d,u,l,u,s);
const XMLCh Exponent::LOCAL_NAME[] =        UNICODE_LITERAL_8(E,x,p,o,n,e,n,t);
const XMLCh RSAKeyValue::LOCAL_NAME[] =     UNICODE_LITERAL_11(R,S,A,K,e,y,V,a,l,u,e);
const XMLCh RSAKeyValue::TYPE_NAME[] =      UNICODE_LITERAL_15(R,S,A,K,e,y,V,a,l,u,e,T,y,p,e);
