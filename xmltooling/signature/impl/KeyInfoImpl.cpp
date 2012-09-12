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
 * KeyInfoImpl.cpp
 * 
 * Implementation classes for KeyInfo schema.
 */

#include "internal.h"
#include "AbstractComplexElement.h"
#include "AbstractSimpleElement.h"
#include "exceptions.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "signature/KeyInfo.h"
#include "util/XMLHelper.h"

#include <boost/lambda/bind.hpp>
#include <boost/lambda/casts.hpp>
#include <boost/lambda/if.hpp>
#include <boost/lambda/lambda.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace xercesc;
using namespace std;
using xmlconstants::XMLSIG_NS;
using xmlconstants::XMLSIG11_NS;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlsignature {
    
    class XMLTOOL_DLLLOCAL DSAKeyValueImpl : public virtual DSAKeyValue,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_P=nullptr;
            m_Q=nullptr;
            m_G=nullptr;
            m_Y=nullptr;
            m_J=nullptr;
            m_Seed=nullptr;
            m_PgenCounter=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
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

    public:
        virtual ~DSAKeyValueImpl() {}

        DSAKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        DSAKeyValueImpl(const DSAKeyValueImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_TYPED_CHILD(P);
            IMPL_CLONE_TYPED_CHILD(Q);
            IMPL_CLONE_TYPED_CHILD(G);
            IMPL_CLONE_TYPED_CHILD(Y);
            IMPL_CLONE_TYPED_CHILD(J);
            IMPL_CLONE_TYPED_CHILD(Seed);
            IMPL_CLONE_TYPED_CHILD(PgenCounter);
        }
                
        IMPL_XMLOBJECT_CLONE(DSAKeyValue);
        IMPL_TYPED_CHILD(P);
        IMPL_TYPED_CHILD(Q);
        IMPL_TYPED_CHILD(G);
        IMPL_TYPED_CHILD(Y);
        IMPL_TYPED_CHILD(J);
        IMPL_TYPED_CHILD(Seed);
        IMPL_TYPED_CHILD(PgenCounter);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(P,XMLSIG_NS,false);
            PROC_TYPED_CHILD(Q,XMLSIG_NS,false);
            PROC_TYPED_CHILD(G,XMLSIG_NS,false);
            PROC_TYPED_CHILD(Y,XMLSIG_NS,false);
            PROC_TYPED_CHILD(J,XMLSIG_NS,false);
            PROC_TYPED_CHILD(Seed,XMLSIG_NS,false);
            PROC_TYPED_CHILD(PgenCounter,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL RSAKeyValueImpl : public virtual RSAKeyValue,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Modulus=nullptr;
            m_Exponent=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_Modulus=m_children.begin();
            m_pos_Exponent=m_pos_Modulus;
            ++m_pos_Exponent;
        }
        
    public:
        virtual ~RSAKeyValueImpl() {}

        RSAKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        RSAKeyValueImpl(const RSAKeyValueImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_TYPED_CHILD(Modulus);
            IMPL_CLONE_TYPED_CHILD(Exponent);
        }
        
        IMPL_XMLOBJECT_CLONE(RSAKeyValue);
        IMPL_TYPED_CHILD(Modulus);
        IMPL_TYPED_CHILD(Exponent);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(Modulus,XMLSIG_NS,false);
            PROC_TYPED_CHILD(Exponent,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL NamedCurveImpl : public virtual NamedCurve,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~NamedCurveImpl() {
            XMLString::release(&m_URI);
        }

        NamedCurveImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_URI(nullptr) {
        }

        NamedCurveImpl(const NamedCurveImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src), m_URI(nullptr) {
            IMPL_CLONE_ATTRIB(URI);
        }

        IMPL_XMLOBJECT_CLONE(NamedCurve);
        IMPL_STRING_ATTRIB(URI);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(URI,URI,nullptr);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(URI,URI,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL ECKeyValueImpl : public virtual ECKeyValue,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Id=nullptr;
            m_ECParameters=nullptr;
            m_NamedCurve=nullptr;
            m_PublicKey=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_ECParameters=m_children.begin();
            m_pos_NamedCurve=m_pos_ECParameters;
            ++m_pos_NamedCurve;
            m_pos_PublicKey=m_pos_NamedCurve;
            ++m_pos_PublicKey;
        }
        
    public:
        virtual ~ECKeyValueImpl() {
            XMLString::release(&m_Id);
        }

        ECKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        ECKeyValueImpl(const ECKeyValueImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_ATTRIB(Id);
            IMPL_CLONE_XMLOBJECT_CHILD(ECParameters);
            IMPL_CLONE_TYPED_CHILD(NamedCurve);
            IMPL_CLONE_TYPED_CHILD(PublicKey);
        }
        
        IMPL_XMLOBJECT_CLONE(ECKeyValue);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_XMLOBJECT_CHILD(ECParameters);
        IMPL_TYPED_CHILD(NamedCurve);
        IMPL_TYPED_CHILD(PublicKey);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(NamedCurve,XMLSIG11_NS,false);
            PROC_TYPED_CHILD(PublicKey,XMLSIG11_NS,false);

            // Not really "unknown", but currently unwrapped.
            static const XMLCh _ECParameters[] = UNICODE_LITERAL_12(E,C,P,a,r,a,m,e,t,e,r,s);
            if (XMLString::equals(root->getLocalName(), _ECParameters) && XMLString::equals(root->getNamespaceURI(), XMLSIG11_NS)) {
                setECParameters(childXMLObject);
                return;
            }

            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL KeyValueImpl : public virtual KeyValue,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_DSAKeyValue=nullptr;
            m_RSAKeyValue=nullptr;
            m_ECKeyValue=nullptr;
            m_UnknownXMLObject=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_DSAKeyValue=m_children.begin();
            m_pos_RSAKeyValue=m_pos_DSAKeyValue;
            ++m_pos_RSAKeyValue;
            m_pos_ECKeyValue=m_pos_RSAKeyValue;
            ++m_pos_ECKeyValue;
            m_pos_UnknownXMLObject=m_pos_ECKeyValue;
            ++m_pos_UnknownXMLObject;
        }

    public:
        virtual ~KeyValueImpl() {}

        KeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        KeyValueImpl(const KeyValueImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_TYPED_CHILD(DSAKeyValue);
            IMPL_CLONE_TYPED_CHILD(RSAKeyValue);
            IMPL_CLONE_TYPED_CHILD(ECKeyValue);
            IMPL_CLONE_XMLOBJECT_CHILD(UnknownXMLObject);
        }
                
        IMPL_XMLOBJECT_CLONE(KeyValue);
        IMPL_TYPED_CHILD(DSAKeyValue);
        IMPL_TYPED_CHILD(RSAKeyValue);
        IMPL_TYPED_CHILD(ECKeyValue);
        IMPL_XMLOBJECT_CHILD(UnknownXMLObject);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(DSAKeyValue,XMLSIG_NS,false);
            PROC_TYPED_CHILD(RSAKeyValue,XMLSIG_NS,false);
            PROC_TYPED_CHILD(ECKeyValue,XMLSIG11_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
                setUnknownXMLObject(childXMLObject);
                return;
            }
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL DEREncodedKeyValueImpl : public virtual DEREncodedKeyValue,
        public AbstractSimpleElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~DEREncodedKeyValueImpl() {
            XMLString::release(&m_Id);
        }

        DEREncodedKeyValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Id(nullptr) {
        }

        DEREncodedKeyValueImpl(const DEREncodedKeyValueImpl& src)
                : AbstractXMLObject(src), AbstractSimpleElement(src), AbstractDOMCachingXMLObject(src), m_Id(nullptr) {
            IMPL_CLONE_ATTRIB(Id);
        }

        IMPL_XMLOBJECT_CLONE(DEREncodedKeyValue);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL TransformImpl : public virtual Transform,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~TransformImpl() {
            XMLString::release(&m_Algorithm);
        }

        TransformImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Algorithm(nullptr) {
        }
            
        TransformImpl(const TransformImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src), m_Algorithm(nullptr) {
            IMPL_CLONE_ATTRIB(Algorithm);
            IMPL_CLONE_CHILDBAG_BEGIN;
                IMPL_CLONE_TYPED_CHILD_IN_BAG(XPath);
                IMPL_CLONE_XMLOBJECT_CHILD_IN_BAG(UnknownXMLObject);
            IMPL_CLONE_CHILDBAG_END;
        }
        
        IMPL_XMLOBJECT_CLONE(Transform);
        IMPL_STRING_ATTRIB(Algorithm);
        IMPL_TYPED_CHILDREN(XPath,m_children.end());
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Algorithm,ALGORITHM,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(XPath,XMLSIG_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
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
            IMPL_CLONE_TYPED_CHILDREN(Transform);
        }
        
        IMPL_XMLOBJECT_CLONE(Transforms);
        IMPL_TYPED_CHILDREN(Transform,m_children.end());

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(Transform,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL RetrievalMethodImpl : public virtual RetrievalMethod,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_URI=m_Type=nullptr;
            m_Transforms=nullptr;
            m_children.push_back(nullptr);
            m_pos_Transforms=m_children.begin();
        }
        
    public:
        virtual ~RetrievalMethodImpl() {
            XMLString::release(&m_URI);
            XMLString::release(&m_Type);
        }

        RetrievalMethodImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        RetrievalMethodImpl(const RetrievalMethodImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_ATTRIB(URI);
            IMPL_CLONE_ATTRIB(Type);
            IMPL_CLONE_TYPED_CHILD(Transforms);
        }
        
        IMPL_XMLOBJECT_CLONE(RetrievalMethod);
        IMPL_STRING_ATTRIB(URI);
        IMPL_STRING_ATTRIB(Type);
        IMPL_TYPED_CHILD(Transforms);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(URI,URI,nullptr);
            MARSHALL_STRING_ATTRIB(Type,TYPE,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(Transforms,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(URI,URI,nullptr);
            PROC_STRING_ATTRIB(Type,TYPE,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL X509IssuerSerialImpl : public virtual X509IssuerSerial,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_X509IssuerName=nullptr;
            m_X509SerialNumber=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_X509IssuerName=m_children.begin();
            m_pos_X509SerialNumber=m_pos_X509IssuerName;
            ++m_pos_X509SerialNumber;
        }
        
    public:
        virtual ~X509IssuerSerialImpl() {}

        X509IssuerSerialImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        X509IssuerSerialImpl(const X509IssuerSerialImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_TYPED_CHILD(X509IssuerName);
            IMPL_CLONE_TYPED_CHILD(X509SerialNumber);
        }
        
        IMPL_XMLOBJECT_CLONE(X509IssuerSerial);
        IMPL_TYPED_CHILD(X509IssuerName);
        IMPL_TYPED_CHILD(X509SerialNumber);

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(X509IssuerName,XMLSIG_NS,false);
            PROC_TYPED_CHILD(X509SerialNumber,XMLSIG_NS,false);
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL X509DigestImpl : public virtual X509Digest,
        public AbstractSimpleElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~X509DigestImpl() {
            XMLString::release(&m_Algorithm);
        }

        X509DigestImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Algorithm(nullptr) {
        }

        X509DigestImpl(const X509DigestImpl& src)
                : AbstractXMLObject(src), AbstractSimpleElement(src), AbstractDOMCachingXMLObject(src), m_Algorithm(nullptr) {
            IMPL_CLONE_ATTRIB(Algorithm);
        }

        IMPL_XMLOBJECT_CLONE(X509Digest);
        IMPL_STRING_ATTRIB(Algorithm);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Algorithm,ALGORITHM,nullptr);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(Algorithm,ALGORITHM,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };


    class XMLTOOL_DLLLOCAL X509DataImpl : public virtual X509Data,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~X509DataImpl() {}

        X509DataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
        }
            
        X509DataImpl(const X509DataImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            IMPL_CLONE_CHILDBAG_BEGIN;
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509Certificate);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509CRL);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509SubjectName);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509IssuerSerial);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509SKI);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509Digest);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(OCSPResponse);
                IMPL_CLONE_XMLOBJECT_CHILD_IN_BAG(UnknownXMLObject);
            IMPL_CLONE_CHILDBAG_END;
        }
        
        IMPL_XMLOBJECT_CLONE(X509Data);
        IMPL_TYPED_CHILDREN(X509IssuerSerial,m_children.end());
        IMPL_TYPED_CHILDREN(X509SKI,m_children.end());
        IMPL_TYPED_CHILDREN(X509SubjectName,m_children.end());
        IMPL_TYPED_CHILDREN(X509Certificate,m_children.end());
        IMPL_TYPED_CHILDREN(X509CRL,m_children.end());
        IMPL_TYPED_CHILDREN(X509Digest,m_children.end());
        IMPL_TYPED_CHILDREN(OCSPResponse,m_children.end());
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(X509IssuerSerial,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(X509SKI,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(X509SubjectName,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(X509Certificate,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(X509CRL,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(X509Digest,XMLSIG11_NS,false);
            PROC_TYPED_CHILDREN(OCSPResponse,XMLSIG11_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
                getUnknownXMLObjects().push_back(childXMLObject);
                return;
            }
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL SPKIDataImpl : public virtual SPKIData,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        vector< pair<SPKISexp*,XMLObject*> > m_SPKISexps;

    public:
        virtual ~SPKIDataImpl() {}

        SPKIDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
        }
            
        SPKIDataImpl(const SPKIDataImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            for (vector< pair<SPKISexp*,XMLObject*> >::const_iterator i = src.m_SPKISexps.begin(); i != src.m_SPKISexps.end(); ++i) {
                if (i->first) {
                    getSPKISexps().push_back(make_pair(i->first->cloneSPKISexp(),(i->second ? i->second->clone() : (XMLObject*)nullptr)));
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(SPKIData);

    public:
        VectorOfPairs(SPKISexp,XMLObject) getSPKISexps() {
            return VectorOfPairs(SPKISexp,XMLObject)(this, m_SPKISexps, &m_children, m_children.end());
        }
        
        const vector< pair<SPKISexp*,XMLObject*> >& getSPKISexps() const {
            return m_SPKISexps;
        }
        
    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            if (XMLHelper::isNodeNamed(root,XMLSIG_NS,SPKISexp::LOCAL_NAME)) {
                SPKISexp* typesafe=dynamic_cast<SPKISexp*>(childXMLObject);
                if (typesafe) {
                    getSPKISexps().push_back(make_pair(typesafe,(XMLObject*)nullptr));
                    return;
                }
            }

            // Unknown child (has to be paired with the last SPKISexp processed.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
                // Update second half of pair in vector, and in master list.
                if (!m_SPKISexps.empty() && m_SPKISexps.back().second==nullptr) {
                    m_SPKISexps.back().second=childXMLObject;
                    m_children.back()=childXMLObject;
                    return;
                }
                else
                    throw UnmarshallingException("Extension element must follow ds:SPKISexp element.");
            }
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL PGPDataImpl : public virtual PGPData,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_PGPKeyID=nullptr;
            m_PGPKeyPacket=nullptr;
            m_children.push_back(nullptr);
            m_children.push_back(nullptr);
            m_pos_PGPKeyID=m_children.begin();
            m_pos_PGPKeyPacket=m_pos_PGPKeyID;
            ++m_pos_PGPKeyPacket;
        }
        
    public:
        virtual ~PGPDataImpl() {}

        PGPDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        PGPDataImpl(const PGPDataImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_TYPED_CHILD(PGPKeyID);
            IMPL_CLONE_TYPED_CHILD(PGPKeyPacket);
            IMPL_CLONE_XMLOBJECT_CHILDREN(UnknownXMLObject);
        }
        
        IMPL_XMLOBJECT_CLONE(PGPData);
        IMPL_TYPED_CHILD(PGPKeyID);
        IMPL_TYPED_CHILD(PGPKeyPacket);
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(PGPKeyID,XMLSIG_NS,false);
            PROC_TYPED_CHILD(PGPKeyPacket,XMLSIG_NS,false);

            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
                getUnknownXMLObjects().push_back(childXMLObject);
                return;
            }

            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }
    };

    class XMLTOOL_DLLLOCAL KeyInfoReferenceImpl : public virtual KeyInfoReference,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Id=m_URI=nullptr;
        }

    public:
        virtual ~KeyInfoReferenceImpl() {
            XMLString::release(&m_Id);
            XMLString::release(&m_URI);
        }

        KeyInfoReferenceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }

        KeyInfoReferenceImpl(const KeyInfoReferenceImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
            init();
            IMPL_CLONE_ATTRIB(Id);
            IMPL_CLONE_ATTRIB(URI);
        }

        IMPL_XMLOBJECT_CLONE(KeyInfoReference);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_STRING_ATTRIB(URI);

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
            MARSHALL_STRING_ATTRIB(URI,URI,nullptr);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            PROC_STRING_ATTRIB(URI,URI,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };

    class XMLTOOL_DLLLOCAL KeyInfoImpl : public virtual KeyInfo,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~KeyInfoImpl() {
            XMLString::release(&m_Id);
        }

        KeyInfoImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Id(nullptr) {
        }
            
        KeyInfoImpl(const KeyInfoImpl& src)
                : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src), m_Id(nullptr) {
            IMPL_CLONE_ATTRIB(Id);
            IMPL_CLONE_CHILDBAG_BEGIN;
                IMPL_CLONE_TYPED_CHILD_IN_BAG(X509Data);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(KeyName);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(KeyValue);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(DEREncodedKeyValue);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(RetrievalMethod);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(MgmtData);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(SPKIData);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(PGPData);
                IMPL_CLONE_TYPED_CHILD_IN_BAG(KeyInfoReference);
                IMPL_CLONE_XMLOBJECT_CHILD_IN_BAG(UnknownXMLObject);
            IMPL_CLONE_CHILDBAG_END;
        }
        
        IMPL_XMLOBJECT_CLONE(KeyInfo);
        IMPL_ID_ATTRIB_EX(Id,ID,nullptr);
        IMPL_TYPED_CHILDREN(KeyName,m_children.end());
        IMPL_TYPED_CHILDREN(KeyValue,m_children.end());
        IMPL_TYPED_CHILDREN(DEREncodedKeyValue,m_children.end());
        IMPL_TYPED_CHILDREN(RetrievalMethod,m_children.end());
        IMPL_TYPED_CHILDREN(X509Data,m_children.end());
        IMPL_TYPED_CHILDREN(MgmtData,m_children.end());
        IMPL_TYPED_CHILDREN(SPKIData,m_children.end());
        IMPL_TYPED_CHILDREN(PGPData,m_children.end());
        IMPL_TYPED_CHILDREN(KeyInfoReference,m_children.end());
        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_ID_ATTRIB(Id,ID,nullptr);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILDREN(X509Data,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(KeyName,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(KeyValue,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(DEREncodedKeyValue,XMLSIG11_NS,false);
            PROC_TYPED_CHILDREN(RetrievalMethod,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(MgmtData,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(SPKIData,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(PGPData,XMLSIG_NS,false);
            PROC_TYPED_CHILDREN(KeyInfoReference,XMLSIG11_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLSIG_NS) && nsURI && *nsURI) {
                getUnknownXMLObjects().push_back(childXMLObject);
                return;
            }
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_ID_ATTRIB(Id,ID,nullptr);
            AbstractXMLObjectUnmarshaller::processAttribute(attribute);
        }
    };
   
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,KeyName);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,MgmtData);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Modulus);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Exponent);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Seed);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,PgenCounter);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,P);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Q);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,G);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,Y);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,J);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,XPath);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509IssuerName);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509SerialNumber);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509SKI);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509SubjectName);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509Certificate);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,X509CRL);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,SPKISexp);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyID);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,PGPKeyPacket);

    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,OCSPResponse);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,PublicKey);
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(X509IssuerSerial);
IMPL_XMLOBJECTBUILDER(X509IssuerName);
IMPL_XMLOBJECTBUILDER(X509SerialNumber);
IMPL_XMLOBJECTBUILDER(X509SKI);
IMPL_XMLOBJECTBUILDER(X509SubjectName);
IMPL_XMLOBJECTBUILDER(X509Certificate);
IMPL_XMLOBJECTBUILDER(X509CRL);
IMPL_XMLOBJECTBUILDER(X509Data);
IMPL_XMLOBJECTBUILDER(XPath);
IMPL_XMLOBJECTBUILDER(Transform);
IMPL_XMLOBJECTBUILDER(Transforms);
IMPL_XMLOBJECTBUILDER(RetrievalMethod);
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
IMPL_XMLOBJECTBUILDER(SPKISexp);
IMPL_XMLOBJECTBUILDER(SPKIData);
IMPL_XMLOBJECTBUILDER(PGPKeyID);
IMPL_XMLOBJECTBUILDER(PGPKeyPacket);
IMPL_XMLOBJECTBUILDER(PGPData);

IMPL_XMLOBJECTBUILDER(DEREncodedKeyValue);
IMPL_XMLOBJECTBUILDER(ECKeyValue);
IMPL_XMLOBJECTBUILDER(KeyInfoReference);
IMPL_XMLOBJECTBUILDER(NamedCurve);
IMPL_XMLOBJECTBUILDER(OCSPResponse);
IMPL_XMLOBJECTBUILDER(PublicKey);
IMPL_XMLOBJECTBUILDER(X509Digest);

// Unicode literals

const XMLCh KeyInfo::LOCAL_NAME[] =                 UNICODE_LITERAL_7(K,e,y,I,n,f,o);
const XMLCh KeyInfo::TYPE_NAME[] =                  UNICODE_LITERAL_11(K,e,y,I,n,f,o,T,y,p,e);
const XMLCh KeyInfo::ID_ATTRIB_NAME[] =             UNICODE_LITERAL_2(I,d);
const XMLCh KeyValue::LOCAL_NAME[] =                UNICODE_LITERAL_8(K,e,y,V,a,l,u,e);
const XMLCh KeyValue::TYPE_NAME[] =                 UNICODE_LITERAL_12(K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh DSAKeyValue::LOCAL_NAME[] =             UNICODE_LITERAL_11(D,S,A,K,e,y,V,a,l,u,e);
const XMLCh DSAKeyValue::TYPE_NAME[] =              UNICODE_LITERAL_15(D,S,A,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh RSAKeyValue::LOCAL_NAME[] =             UNICODE_LITERAL_11(R,S,A,K,e,y,V,a,l,u,e);
const XMLCh RSAKeyValue::TYPE_NAME[] =              UNICODE_LITERAL_15(R,S,A,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh MgmtData::LOCAL_NAME[] =                UNICODE_LITERAL_8(M,g,m,t,D,a,t,a);
const XMLCh KeyName::LOCAL_NAME[] =                 UNICODE_LITERAL_7(K,e,y,N,a,m,e);
const XMLCh Modulus::LOCAL_NAME[] =                 UNICODE_LITERAL_7(M,o,d,u,l,u,s);
const XMLCh Exponent::LOCAL_NAME[] =                UNICODE_LITERAL_8(E,x,p,o,n,e,n,t);
const XMLCh Seed::LOCAL_NAME[] =                    UNICODE_LITERAL_4(S,e,e,d);
const XMLCh PgenCounter::LOCAL_NAME[] =             UNICODE_LITERAL_11(P,g,e,n,C,o,u,n,t,e,r);
const XMLCh P::LOCAL_NAME[] =                       UNICODE_LITERAL_1(P);
const XMLCh Q::LOCAL_NAME[] =                       UNICODE_LITERAL_1(Q);
const XMLCh G::LOCAL_NAME[] =                       UNICODE_LITERAL_1(G);
const XMLCh Y::LOCAL_NAME[] =                       UNICODE_LITERAL_1(Y);
const XMLCh J::LOCAL_NAME[] =                       UNICODE_LITERAL_1(J);
const XMLCh XPath::LOCAL_NAME[] =                   UNICODE_LITERAL_5(X,P,a,t,h);
const XMLCh Transform::LOCAL_NAME[] =               UNICODE_LITERAL_9(T,r,a,n,s,f,o,r,m);
const XMLCh Transform::TYPE_NAME[] =                UNICODE_LITERAL_13(T,r,a,n,s,f,o,r,m,T,y,p,e);
const XMLCh Transform::ALGORITHM_ATTRIB_NAME[] =    UNICODE_LITERAL_9(A,l,g,o,r,i,t,h,m);
const XMLCh Transforms::LOCAL_NAME[] =              UNICODE_LITERAL_10(T,r,a,n,s,f,o,r,m,s);
const XMLCh Transforms::TYPE_NAME[] =               UNICODE_LITERAL_14(T,r,a,n,s,f,o,r,m,s,T,y,p,e);
const XMLCh RetrievalMethod::LOCAL_NAME[] =         UNICODE_LITERAL_15(R,e,t,r,i,e,v,a,l,M,e,t,h,o,d);
const XMLCh RetrievalMethod::TYPE_NAME[] =          UNICODE_LITERAL_19(R,e,t,r,i,e,v,a,l,M,e,t,h,o,d,T,y,p,e);
const XMLCh RetrievalMethod::URI_ATTRIB_NAME[] =    UNICODE_LITERAL_3(U,R,I);
const XMLCh RetrievalMethod::TYPE_ATTRIB_NAME[] =   UNICODE_LITERAL_4(T,y,p,e);
const XMLCh SPKISexp::LOCAL_NAME[] =                UNICODE_LITERAL_8(S,P,K,I,S,e,x,p);
const XMLCh SPKIData::LOCAL_NAME[] =                UNICODE_LITERAL_8(S,P,K,I,D,a,t,a);
const XMLCh SPKIData::TYPE_NAME[] =                 UNICODE_LITERAL_12(S,P,K,I,D,a,t,a,T,y,p,e);
const XMLCh PGPKeyID::LOCAL_NAME[] =                UNICODE_LITERAL_8(P,G,P,K,e,y,I,D);
const XMLCh PGPKeyPacket::LOCAL_NAME[] =            UNICODE_LITERAL_12(P,G,P,K,e,y,P,a,c,k,e,t);
const XMLCh PGPData::LOCAL_NAME[] =                 UNICODE_LITERAL_7(P,G,P,D,a,t,a);
const XMLCh PGPData::TYPE_NAME[] =                  UNICODE_LITERAL_11(P,G,P,D,a,t,a,T,y,p,e);

const XMLCh DEREncodedKeyValue::LOCAL_NAME[] =      UNICODE_LITERAL_18(D,E,R,E,n,c,o,d,e,d,K,e,y,V,a,l,u,e);
const XMLCh DEREncodedKeyValue::TYPE_NAME[] =       UNICODE_LITERAL_22(D,E,R,E,n,c,o,d,e,d,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh DEREncodedKeyValue::ID_ATTRIB_NAME[] =  UNICODE_LITERAL_2(I,d);
const XMLCh ECKeyValue::LOCAL_NAME[] =              UNICODE_LITERAL_10(E,C,K,e,y,V,a,l,u,e);
const XMLCh ECKeyValue::TYPE_NAME[] =               UNICODE_LITERAL_14(E,C,K,e,y,V,a,l,u,e,T,y,p,e);
const XMLCh ECKeyValue::ID_ATTRIB_NAME[] =          UNICODE_LITERAL_2(I,d);
const XMLCh KeyInfoReference::LOCAL_NAME[] =        UNICODE_LITERAL_16(K,e,y,I,n,f,o,R,e,f,e,r,e,n,c,e);
const XMLCh KeyInfoReference::TYPE_NAME[] =         UNICODE_LITERAL_20(K,e,y,I,n,f,o,R,e,f,e,r,e,n,c,e,T,y,p,e);
const XMLCh KeyInfoReference::ID_ATTRIB_NAME[] =    UNICODE_LITERAL_2(I,d);
const XMLCh KeyInfoReference::URI_ATTRIB_NAME[] =   UNICODE_LITERAL_3(U,R,I);
const XMLCh NamedCurve::LOCAL_NAME[] =              UNICODE_LITERAL_10(N,a,m,e,d,C,u,r,v,e);
const XMLCh NamedCurve::TYPE_NAME[] =               UNICODE_LITERAL_14(N,a,m,e,d,C,u,r,v,e,T,y,p,e);
const XMLCh NamedCurve::URI_ATTRIB_NAME[] =         UNICODE_LITERAL_3(U,R,I);
const XMLCh OCSPResponse::LOCAL_NAME[] =            UNICODE_LITERAL_12(O,C,S,P,R,e,s,p,o,n,s,e);
const XMLCh PublicKey::LOCAL_NAME[] =               UNICODE_LITERAL_9(P,u,b,l,i,c,K,e,y);
const XMLCh X509Digest::ALGORITHM_ATTRIB_NAME[] =   UNICODE_LITERAL_9(A,l,g,o,r,i,t,h,m);

#define XCH(ch) chLatin_##ch
#define XNUM(d) chDigit_##d

const XMLCh X509Data::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(D), XCH(a), XCH(t), XCH(a), chNull
    };
const XMLCh X509Data::TYPE_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(D), XCH(a), XCH(t), XCH(a), XCH(T), XCH(y), XCH(p), XCH(e), chNull
    };
const XMLCh X509IssuerSerial::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(I), XCH(s), XCH(s), XCH(u), XCH(e), XCH(r),
    XCH(S), XCH(e), XCH(r), XCH(i), XCH(a), XCH(l), chNull
    };
const XMLCh X509IssuerSerial::TYPE_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(I), XCH(s), XCH(s), XCH(u), XCH(e), XCH(r),
    XCH(S), XCH(e), XCH(r), XCH(i), XCH(a), XCH(l), XCH(T), XCH(y), XCH(p), XCH(e), chNull
    };
const XMLCh X509IssuerName::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(I), XCH(s), XCH(s), XCH(u), XCH(e), XCH(r),
    XCH(N), XCH(a), XCH(m), XCH(e), chNull
    };
const XMLCh X509SerialNumber::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(S), XCH(e), XCH(r), XCH(i), XCH(a), XCH(l),
    XCH(N), XCH(u), XCH(m), XCH(b), XCH(e), XCH(r), chNull
    };
const XMLCh X509SKI::LOCAL_NAME[] = { XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(S), XCH(K), XCH(I), chNull };
const XMLCh X509SubjectName::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(S), XCH(u), XCH(b), XCH(j), XCH(e), XCH(c), XCH(t),
    XCH(N), XCH(a), XCH(m), XCH(e), chNull
    };
const XMLCh X509Certificate::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9),
    XCH(C), XCH(e), XCH(r), XCH(t), XCH(i), XCH(f), XCH(i), XCH(c), XCH(a), XCH(t), XCH(e), chNull
    };
const XMLCh X509CRL::LOCAL_NAME[] = { XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(C), XCH(R), XCH(L), chNull };
const XMLCh X509Digest::LOCAL_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9), XCH(D), XCH(i), XCH(g), XCH(e), XCH(s), XCH(t), chNull
    };
const XMLCh X509Digest::TYPE_NAME[] = {
    XCH(X), XNUM(5), XNUM(0), XNUM(9),  XCH(D), XCH(i), XCH(g), XCH(e), XCH(s), XCH(t), XCH(T), XCH(y), XCH(p), XCH(e), chNull
    };

const XMLCh RetrievalMethod::TYPE_DSAKEYVALUE[] = {
    chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
    chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
    chDigit_2, chDigit_0, chDigit_0, chDigit_0, chForwardSlash, chDigit_0, chDigit_9, chForwardSlash,
    chLatin_x, chLatin_m, chLatin_l, chLatin_d, chLatin_s, chLatin_i, chLatin_g, chPound,
    chLatin_D, chLatin_S, chLatin_A, chLatin_K, chLatin_e, chLatin_y, chLatin_V, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull
    };

const XMLCh RetrievalMethod::TYPE_RSAKEYVALUE[] = {
    chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
    chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
    chDigit_2, chDigit_0, chDigit_0, chDigit_0, chForwardSlash, chDigit_0, chDigit_9, chForwardSlash,
    chLatin_x, chLatin_m, chLatin_l, chLatin_d, chLatin_s, chLatin_i, chLatin_g, chPound,
    chLatin_R, chLatin_S, chLatin_A, chLatin_K, chLatin_e, chLatin_y, chLatin_V, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull
    };

const XMLCh RetrievalMethod::TYPE_X509DATA[] = {
    chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
    chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
    chDigit_2, chDigit_0, chDigit_0, chDigit_0, chForwardSlash, chDigit_0, chDigit_9, chForwardSlash,
    chLatin_x, chLatin_m, chLatin_l, chLatin_d, chLatin_s, chLatin_i, chLatin_g, chPound,
    chLatin_X, chDigit_5, chDigit_0, chDigit_9, chLatin_D, chLatin_a, chLatin_t, chLatin_a, chNull
    };
