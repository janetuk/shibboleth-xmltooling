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
 * SOAPImpl.cpp
 * 
 * Implementation classes for SOAP schema
 */

#include "internal.h"
#include "AbstractAttributeExtensibleXMLObject.h"
#include "AbstractComplexElement.h"
#include "AbstractSimpleElement.h"
#include "exceptions.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "soap/SOAP.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace soap11;
using namespace xmltooling;
using namespace std;
using xmlconstants::SOAP11ENV_NS;
using xmlconstants::SOAP11ENV_PREFIX;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace {

    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Faultstring);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Faultactor);

    class XMLTOOL_DLLLOCAL FaultcodeImpl : public virtual Faultcode,
        public AbstractSimpleElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        QName* m_qname;
    public:
        virtual ~FaultcodeImpl() {
            delete m_qname;
        }

        FaultcodeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_qname(NULL) {
        }
            
        FaultcodeImpl(const FaultcodeImpl& src)
                : AbstractXMLObject(src), AbstractSimpleElement(src), AbstractDOMCachingXMLObject(src), m_qname(NULL) {
            setCode(src.getCode());
        }
        
        const QName* getCode() const {
            return m_qname;
        }
        
        void setCode(const QName* qname) {
            m_qname=prepareForAssignment(m_qname,qname);
            if (m_qname) {
                auto_ptr_XMLCh temp(m_qname->toString().c_str());
                setTextContent(temp.get());
            }
            else
                setTextContent(NULL);
        }
        
        IMPL_XMLOBJECT_CLONE(Faultcode);
    };

    class XMLTOOL_DLLLOCAL DetailImpl : public virtual Detail,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~DetailImpl() {}

        DetailImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
        }
            
        DetailImpl(const DetailImpl& src)
                : AbstractXMLObject(src),
                    AbstractAttributeExtensibleXMLObject(src),
                    AbstractComplexElement(src),
                    AbstractDOMCachingXMLObject(src) {
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(Detail);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject, m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            marshallExtensionAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getUnknownXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            unmarshallExtensionAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL FaultImpl : public virtual Fault,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Faultcode=NULL;
            m_Faultstring=NULL;
            m_Faultactor=NULL;
            m_Detail=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_Faultcode=m_children.begin();
            m_pos_Faultstring=m_pos_Faultcode;
            ++m_pos_Faultstring;
            m_pos_Faultactor=m_pos_Faultstring;
            ++m_pos_Faultactor;
            m_pos_Detail=m_pos_Faultactor;
            ++m_pos_Detail;
        }
    protected:
        FaultImpl() {
            init();
        }
        
    public:
        virtual ~FaultImpl() {}

        FaultImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        FaultImpl(const FaultImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            if (src.getFaultcode())
                setFaultcode(src.getFaultcode()->cloneFaultcode());
            if (src.getFaultstring())
                setFaultstring(src.getFaultstring()->cloneFaultstring());
            if (src.getFaultactor())
                setFaultactor(src.getFaultactor()->cloneFaultactor());
            if (src.getDetail())
                setDetail(src.getDetail()->cloneDetail());
        }
        
        IMPL_XMLOBJECT_CLONE(Fault);
        IMPL_TYPED_CHILD(Faultcode);
        IMPL_TYPED_CHILD(Faultstring);
        IMPL_TYPED_CHILD(Faultactor);
        IMPL_TYPED_CHILD(Detail);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(Faultcode,SOAP11ENV_NS,false);
            PROC_TYPED_CHILD(Faultstring,SOAP11ENV_NS,false);
            PROC_TYPED_CHILD(Faultactor,SOAP11ENV_NS,false);
            PROC_TYPED_CHILD(Detail,SOAP11ENV_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL BodyImpl : public virtual Body,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_EncodingStyle=NULL;
        }
    public:
        virtual ~BodyImpl() {
            XMLString::release(&m_EncodingStyle);
        }

        BodyImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        BodyImpl(const BodyImpl& src)
                : AbstractXMLObject(src),
                    AbstractAttributeExtensibleXMLObject(src),
                    AbstractComplexElement(src),
                    AbstractDOMCachingXMLObject(src) {
            init();
            setEncodingStyle(src.getEncodingStyle());
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(Body);
        IMPL_STRING_ATTRIB(EncodingStyle);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject, m_children.end());

        void setAttribute(QName& qualifiedName, const XMLCh* value, bool ID=false) {
            if (qualifiedName.hasNamespaceURI() && XMLString::equals(qualifiedName.getNamespaceURI(),SOAP11ENV_NS)) {
                if (XMLString::equals(qualifiedName.getLocalPart(),ENCODINGSTYLE_ATTRIB_NAME)) {
                    setEncodingStyle(value);
                    return;
                }
            }
            AbstractAttributeExtensibleXMLObject::setAttribute(qualifiedName, value, ID);
        }

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(EncodingStyle,ENCODINGSTYLE,SOAP11ENV_NS);
            marshallExtensionAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getUnknownXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            unmarshallExtensionAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL HeaderImpl : public virtual Header,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Actor=NULL;
            m_MustUnderstand=xmlconstants::XML_BOOL_NULL;
        }
    public:
        virtual ~HeaderImpl() {
            XMLString::release(&m_Actor);
        }

        HeaderImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        HeaderImpl(const HeaderImpl& src)
                : AbstractXMLObject(src),
                    AbstractAttributeExtensibleXMLObject(src),
                    AbstractComplexElement(src),
                    AbstractDOMCachingXMLObject(src) {
            init();
            setActor(src.getActor());
            MustUnderstand(m_MustUnderstand);
            VectorOf(XMLObject) v=getUnknownXMLObjects();
            for (vector<XMLObject*>::const_iterator i=src.m_UnknownXMLObjects.begin(); i!=src.m_UnknownXMLObjects.end(); ++i)
                v.push_back((*i)->clone());
        }
        
        IMPL_XMLOBJECT_CLONE(Header);
        IMPL_STRING_ATTRIB(Actor);
        IMPL_BOOLEAN_ATTRIB(MustUnderstand);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject, m_children.end());

        void setAttribute(QName& qualifiedName, const XMLCh* value, bool ID=false) {
            if (qualifiedName.hasNamespaceURI() && XMLString::equals(qualifiedName.getNamespaceURI(),SOAP11ENV_NS)) {
                if (XMLString::equals(qualifiedName.getLocalPart(),MUSTUNDERSTAND_ATTRIB_NAME)) {
                    setMustUnderstand(value);
                    return;
                }
                else if (XMLString::equals(qualifiedName.getLocalPart(),ACTOR_ATTRIB_NAME)) {
                    setActor(value);
                    return;
                }
            }
            AbstractAttributeExtensibleXMLObject::setAttribute(qualifiedName, value, ID);
        }

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Actor,ACTOR,SOAP11ENV_NS);
            MARSHALL_BOOLEAN_ATTRIB(MustUnderstand,MUSTUNDERSTAND,SOAP11ENV_NS);
            marshallExtensionAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getUnknownXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            unmarshallExtensionAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL EnvelopeImpl : public virtual Envelope,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Header=NULL;
            m_Body=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_Header=m_children.begin();
            m_pos_Body=m_pos_Header;
            ++m_pos_Body;
        }
    public:
        virtual ~EnvelopeImpl() {}

        EnvelopeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EnvelopeImpl(const EnvelopeImpl& src)
                : AbstractXMLObject(src), AbstractAttributeExtensibleXMLObject(src),
                    AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            if (src.getHeader())
                setHeader(src.getHeader()->cloneHeader());
            if (src.getBody())
                setBody(src.getBody()->cloneBody());
        }
        
        IMPL_TYPED_CHILD(Header);
        IMPL_TYPED_CHILD(Body);
        IMPL_XMLOBJECT_CLONE(Envelope);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            marshallExtensionAttributes(domElement);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(Header,SOAP11ENV_NS,false);
            PROC_TYPED_CHILD(Body,SOAP11ENV_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            unmarshallExtensionAttribute(attribute);
        }
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(Body);
IMPL_XMLOBJECTBUILDER(Detail);
IMPL_XMLOBJECTBUILDER(Envelope);
IMPL_XMLOBJECTBUILDER(Fault);
IMPL_XMLOBJECTBUILDER(Faultactor);
IMPL_XMLOBJECTBUILDER(Faultcode);
IMPL_XMLOBJECTBUILDER(Faultstring);
IMPL_XMLOBJECTBUILDER(Header);

// Unicode literals

const XMLCh Body::LOCAL_NAME[] =                        UNICODE_LITERAL_4(B,o,d,y);
const XMLCh Body::TYPE_NAME[] =                         UNICODE_LITERAL_4(B,o,d,y);
const XMLCh Body::ENCODINGSTYLE_ATTRIB_NAME[] =         UNICODE_LITERAL_13(e,n,c,o,d,i,n,g,S,t,y,l,e);
const XMLCh Detail::LOCAL_NAME[] =                      UNICODE_LITERAL_6(d,e,t,a,i,l);
const XMLCh Detail::TYPE_NAME[] =                       UNICODE_LITERAL_6(d,e,t,a,i,l);
const XMLCh Envelope::LOCAL_NAME[] =                    UNICODE_LITERAL_8(E,n,v,e,l,o,p,e);
const XMLCh Envelope::TYPE_NAME[] =                     UNICODE_LITERAL_8(E,n,v,e,l,o,p,e);
const XMLCh Fault::LOCAL_NAME[] =                       UNICODE_LITERAL_5(F,a,u,l,t);
const XMLCh Fault::TYPE_NAME[] =                        UNICODE_LITERAL_5(F,a,u,l,t);
const XMLCh Faultactor::LOCAL_NAME[] =                  UNICODE_LITERAL_10(F,a,u,l,t,a,c,t,o,r);
const XMLCh Faultcode::LOCAL_NAME[] =                   UNICODE_LITERAL_9(F,a,u,l,t,c,o,d,e);
const XMLCh Faultstring::LOCAL_NAME[] =                 UNICODE_LITERAL_11(F,a,u,l,t,s,t,r,i,n,g);
const XMLCh Header::LOCAL_NAME[] =                      UNICODE_LITERAL_6(H,e,a,d,e,r);
const XMLCh Header::TYPE_NAME[] =                       UNICODE_LITERAL_6(H,e,a,d,e,r);
const XMLCh Header::ACTOR_ATTRIB_NAME[] =               UNICODE_LITERAL_5(a,c,t,o,r);
const XMLCh Header::MUSTUNDERSTAND_ATTRIB_NAME[] =      UNICODE_LITERAL_14(m,u,s,t,U,n,d,e,r,s,t,a,n,d);

static const XMLCh _CLIENT[] =                          UNICODE_LITERAL_6(C,l,i,e,n,t);
static const XMLCh _SERVER[] =                          UNICODE_LITERAL_6(S,e,r,v,e,r);
static const XMLCh _MUSTUNDERSTAND[] =                  UNICODE_LITERAL_14(M,u,s,t,U,n,d,e,r,s,t,a,n,d);
static const XMLCh _VERSIONMISMATCH[] =                 UNICODE_LITERAL_15(V,e,r,s,i,o,n,M,i,s,m,a,t,c,h);
 
QName Faultcode::CLIENT(SOAP11ENV_NS,_CLIENT,SOAP11ENV_PREFIX);
QName Faultcode::SERVER(SOAP11ENV_NS,_SERVER,SOAP11ENV_PREFIX);
QName Faultcode::MUSTUNDERSTAND(SOAP11ENV_NS,_MUSTUNDERSTAND,SOAP11ENV_PREFIX);
QName Faultcode::VERSIONMISMATCH(SOAP11ENV_NS,_VERSIONMISMATCH,SOAP11ENV_PREFIX);
