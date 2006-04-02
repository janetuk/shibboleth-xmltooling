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
 * KeyInfoImpl.cpp
 * 
 * Implementation classes for KeyInfo schema
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
    
    class XMLTOOL_DLLLOCAL DSAKeyValueImpl : public DSAKeyValue,
        public AbstractDOMCachingXMLObject,
        public AbstractValidatingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~DSAKeyValueImpl() {}

        DSAKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        DSAKeyValueImpl(const DSAKeyValueImpl& src)
                : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
            init();
            setP(src.getP());
            setQ(src.getQ());
            setG(src.getG());
            setY(src.getY());
            setJ(src.getJ());
            setSeed(src.getSeed());
            setPgenCounter(src.getPgenCounter());
        }
        
        void init() {
            m_P=NULL;
            m_Q=NULL;
            m_G=NULL;
            m_Y=NULL;
            m_J=NULL;
            m_Seed=NULL;
            m_PgenCounter=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_P=m_children.begin();
            m_pos_Q=m_pos_P;
            ++m_pos_Q;
            m_pos_G=m_pos_Q;
            ++m_pos_G;
            m_pos_Y=m_pos_G;
            ++m_pos_Y;
            m_pos_J=m_pos_Y;
            ++m_pos_J;
            m_pos_Seed=m_pos_J;
            ++m_pos_Seed;
            m_pos_PgenCounter=m_pos_Seed;
            ++m_pos_PgenCounter;
        }
        
        IMPL_XMLOBJECT_CLONE(DSAKeyValue);
        IMPL_XMLOBJECT_CHILD(P);
        IMPL_XMLOBJECT_CHILD(Q);
        IMPL_XMLOBJECT_CHILD(G);
        IMPL_XMLOBJECT_CHILD(Y);
        IMPL_XMLOBJECT_CHILD(J);
        IMPL_XMLOBJECT_CHILD(Seed);
        IMPL_XMLOBJECT_CHILD(PgenCounter);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_XMLOBJECT_CHILD(P,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(Q,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(G,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(Y,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(J,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(Seed,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(PgenCounter,XMLConstants::XMLSIG_NS);
            throw UnmarshallingException("Invalid child element: $1",params(1,childXMLObject->getElementQName().toString().c_str()));
        }
    };

    class XMLTOOL_DLLLOCAL RSAKeyValueImpl : public RSAKeyValue,
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
                : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
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
            m_pos_Exponent=m_pos_Modulus;
            ++m_pos_Exponent;
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

    class XMLTOOL_DLLLOCAL KeyValueImpl : public KeyValue,
        public AbstractDOMCachingXMLObject,
        public AbstractValidatingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~KeyValueImpl() {}

        KeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        KeyValueImpl(const KeyValueImpl& src)
                : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
            init();
            setDSAKeyValue(src.getDSAKeyValue());
            setRSAKeyValue(src.getRSAKeyValue());
            setXMLObject(src.getXMLObject());
            setTextContent(src.getTextContent());
        }
        
        void init() {
            m_TextContent=NULL;
            m_DSAKeyValue=NULL;
            m_RSAKeyValue=NULL;
            m_XMLObject=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_DSAKeyValue=m_children.begin();
            m_pos_RSAKeyValue=m_pos_DSAKeyValue;
            ++m_pos_RSAKeyValue;
            m_pos_XMLObject=m_pos_RSAKeyValue;
            ++m_pos_XMLObject;
        }
        
        IMPL_XMLOBJECT_CLONE(KeyValue);
        IMPL_XMLOBJECT_CHILD(DSAKeyValue);
        IMPL_XMLOBJECT_CHILD(RSAKeyValue);
        IMPL_XMLOBJECT_CHILD(XMLObject);
        IMPL_XMLOBJECT_CONTENT(TextContent);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_XMLOBJECT_CHILD(DSAKeyValue,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILD(RSAKeyValue,XMLConstants::XMLSIG_NS);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLConstants::XMLSIG_NS) && nsURI && *nsURI)
                setXMLObject(childXMLObject);
            
            throw UnmarshallingException("Invalid child element: $1",params(1,childXMLObject->getElementQName().toString().c_str()));
        }
    };
    
    class XMLTOOL_DLLLOCAL KeyInfoImpl : public KeyInfo,
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
                : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractElementProxy(src),
                    AbstractValidatingXMLObject(src), m_Id(XMLString::replicate(src.m_Id)) {
            for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                if (*i) {
                    KeyName* kn=dynamic_cast<KeyName*>(*i);
                    if (kn) {
                        getKeyNames().push_back(kn->cloneKeyName());
                        continue;
                    }
                    KeyValue* kv=dynamic_cast<KeyValue*>(*i);
                    if (kv) {
                        getKeyValues().push_back(kv->cloneKeyValue());
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
        IMPL_XMLOBJECT_CHILDREN(KeyValue,m_children.end());
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
            PROC_XMLOBJECT_CHILDREN(KeyValue,XMLConstants::XMLSIG_NS);
            PROC_XMLOBJECT_CHILDREN(MgmtData,XMLConstants::XMLSIG_NS);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
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
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Seed,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,PgenCounter,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,P,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Q,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,G,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Y,Value);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,J,Value);
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(KeyName);
IMPL_XMLOBJECTBUILDER(MgmtData);
IMPL_XMLOBJECTBUILDER(Modulus);
IMPL_XMLOBJECTBUILDER(Exponent);
IMPL_XMLOBJECTBUILDER(Seed);
IMPL_XMLOBJECTBUILDER(PgenCounter);
IMPL_XMLOBJECTBUILDER(P);
IMPL_XMLOBJECTBUILDER(Q);
IMPL_XMLOBJECTBUILDER(G);
IMPL_XMLOBJECTBUILDER(Y);
IMPL_XMLOBJECTBUILDER(J);
IMPL_XMLOBJECTBUILDER(DSAKeyValue);
IMPL_XMLOBJECTBUILDER(RSAKeyValue);
IMPL_XMLOBJECTBUILDER(KeyValue);
IMPL_XMLOBJECTBUILDER(KeyInfo);

const XMLCh KeyInfo::LOCAL_NAME[] =         UNICODE_LITERAL_7(K,e,y,I,n,f,o);
const XMLCh KeyInfo::TYPE_NAME[] =          UNICODE_LITERAL_11(K,e,y,I,n,f,o,T,y,p,e);
const XMLCh KeyInfo::ID_ATTRIB_NAME[] =     UNICODE_LITERAL_2(I,d);
const XMLCh KeyValue::LOCAL_NAME[] =        UNICODE_LITERAL_8(K,e,y,V,a,l,u,e);
const XMLCh KeyValue::TYPE_NAME[] =         UNICODE_LITERAL_12(K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh DSAKeyValue::LOCAL_NAME[] =     UNICODE_LITERAL_11(D,S,A,K,e,y,V,a,l,u,e);
const XMLCh DSAKeyValue::TYPE_NAME[] =      UNICODE_LITERAL_15(D,S,A,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh RSAKeyValue::LOCAL_NAME[] =     UNICODE_LITERAL_11(R,S,A,K,e,y,V,a,l,u,e);
const XMLCh RSAKeyValue::TYPE_NAME[] =      UNICODE_LITERAL_15(R,S,A,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh MgmtData::LOCAL_NAME[] =        UNICODE_LITERAL_8(M,g,m,t,D,a,t,a);
const XMLCh KeyName::LOCAL_NAME[] =         UNICODE_LITERAL_7(K,e,y,N,a,m,e);
const XMLCh Modulus::LOCAL_NAME[] =         UNICODE_LITERAL_7(M,o,d,u,l,u,s);
const XMLCh Exponent::LOCAL_NAME[] =        UNICODE_LITERAL_8(E,x,p,o,n,e,n,t);
const XMLCh Seed::LOCAL_NAME[] =            UNICODE_LITERAL_4(S,e,e,d);
const XMLCh PgenCounter::LOCAL_NAME[] =     UNICODE_LITERAL_11(P,g,e,n,C,o,u,n,t,e,r);
const XMLCh P::LOCAL_NAME[] =               UNICODE_LITERAL_1(P);
const XMLCh Q::LOCAL_NAME[] =               UNICODE_LITERAL_1(Q);
const XMLCh G::LOCAL_NAME[] =               UNICODE_LITERAL_1(G);
const XMLCh Y::LOCAL_NAME[] =               UNICODE_LITERAL_1(Y);
const XMLCh J::LOCAL_NAME[] =               UNICODE_LITERAL_1(J);
