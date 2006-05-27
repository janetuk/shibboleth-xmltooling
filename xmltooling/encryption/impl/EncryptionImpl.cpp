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
 * EncryptionImpl.cpp
 * 
 * Implementation classes for XML Encryption schema
 */

#include "internal.h"
#include "AbstractChildlessElement.h"
#include "AbstractComplexElement.h"
#include "AbstractSimpleElement.h"
#include "exceptions.h"
#include "encryption/Encryption.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "util/XMLHelper.h"
#include "validation/AbstractValidatingXMLObject.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlencryption;
using namespace xmltooling;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlencryption {

    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,KeySize);
    DECL_XMLOBJECTIMPL_SIMPLE(XMLTOOL_DLLLOCAL,OAEPparams);

    class XMLTOOL_DLLLOCAL EncryptionMethodImpl : public virtual EncryptionMethod,
        public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractValidatingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
        void init() {
            m_Algorithm=NULL;
            m_KeySize=NULL;
            m_OAEPparams=NULL;
            m_children.push_back(NULL);
            m_children.push_back(NULL);
            m_pos_KeySize=m_children.begin();
            m_pos_OAEPparams=m_pos_KeySize;
            ++m_pos_OAEPparams;
        }
    public:
        virtual ~EncryptionMethodImpl() {
            XMLString::release(&m_Algorithm);
        }

        EncryptionMethodImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            init();
        }
            
        EncryptionMethodImpl(const EncryptionMethodImpl& src)
                : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
            init();
            setAlgorithm(src.getAlgorithm());
            if (src.getKeySize())
                setKeySize(src.getKeySize()->cloneKeySize());
            if (src.getOAEPparams())
                setOAEPparams(src.getOAEPparams()->cloneOAEPparams());
            VectorOf(XMLObject) v=getOtherParameters();
            for (vector<XMLObject*>::const_iterator i=src.m_OtherParameters.begin(); i!=src.m_OtherParameters.end(); i++) {
                if (*i) {
                    v.push_back((*i)->clone());
                }
            }
        }
        
        IMPL_XMLOBJECT_CLONE(EncryptionMethod);
        IMPL_STRING_ATTRIB(Algorithm);
        IMPL_TYPED_CHILD(KeySize);
        IMPL_TYPED_CHILD(OAEPparams);
        IMPL_XMLOBJECT_CHILDREN(OtherParameter,m_children.end());

    protected:
        void marshallAttributes(DOMElement* domElement) const {
            MARSHALL_STRING_ATTRIB(Algorithm,ALGORITHM,NULL);
        }

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            PROC_TYPED_CHILD(KeySize,XMLConstants::XMLENC_NS,false);
            PROC_TYPED_CHILD(OAEPparams,XMLConstants::XMLENC_NS,false);
            
            // Unknown child.
            const XMLCh* nsURI=root->getNamespaceURI();
            if (!XMLString::equals(nsURI,XMLConstants::XMLENC_NS) && nsURI && *nsURI)
                getOtherParameters().push_back(childXMLObject);
            
            AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
        }

        void processAttribute(const DOMAttr* attribute) {
            PROC_STRING_ATTRIB(Algorithm,ALGORITHM,NULL);
        }
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(KeySize);
IMPL_XMLOBJECTBUILDER(OAEPparams);
IMPL_XMLOBJECTBUILDER(EncryptionMethod);

// Unicode literals

const XMLCh KeySize::LOCAL_NAME[] =                     UNICODE_LITERAL_7(K,e,y,S,i,z,e);
const XMLCh OAEPparams::LOCAL_NAME[] =                  UNICODE_LITERAL_10(O,A,E,P,p,a,r,a,m,s);
const XMLCh EncryptionMethod::LOCAL_NAME[] =            UNICODE_LITERAL_16(E,n,c,r,y,p,t,i,o,n,M,e,t,h,o,d);
const XMLCh EncryptionMethod::TYPE_NAME[] =             UNICODE_LITERAL_20(E,n,c,r,y,p,t,i,o,n,M,e,t,h,o,d,T,y,p,e);
const XMLCh EncryptionMethod::ALGORITHM_ATTRIB_NAME[] = UNICODE_LITERAL_9(A,l,g,o,r,i,t,h,m);
