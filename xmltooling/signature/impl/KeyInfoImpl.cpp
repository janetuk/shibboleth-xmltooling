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
#include "util/NDC.h"
#include "util/XMLHelper.h"
#include "validation/AbstractValidatingXMLObject.h"

#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {
    
    class XMLTOOL_DLLLOCAL KeyInfoImpl : public KeyInfo, public AbstractDOMCachingXMLObject, public AbstractElementProxy,
        public AbstractValidatingXMLObject, public AbstractXMLObjectMarshaller, public AbstractXMLObjectUnmarshaller
    {
    public:
        virtual ~KeyInfoImpl() {}

        KeyInfoImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix)
            : AbstractXMLObject(nsURI, localName, prefix), m_Id(NULL) {}
            
        KeyInfoImpl(const KeyInfoImpl& src) : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src),
            AbstractElementProxy(src), AbstractValidatingXMLObject(src), m_Id(XMLString::replicate(src.m_Id)) {
            for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                getXMLObjects().push_back((*i) ? (*i)->clone() : NULL);
            }
        }
        
        IMPL_XMLOBJECT_ATTRIB(Id);
        IMPL_XMLOBJECT_CLONE(KeyInfo);

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

        void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
            getXMLObjects().push_back(childXMLObject);
        }

        void processAttribute(const DOMAttr* attribute) {
            if (XMLHelper::isNodeNamed(attribute, NULL, ID_ATTRIB_NAME))
                setId(attribute->getValue());
        }

        void processElementContent(const XMLCh* elementContent) {
            setTextContent(elementContent);
        }
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

KeyInfo* KeyInfoBuilderImpl::buildObject(const XMLCh* ns, const XMLCh* name, const XMLCh* prefix) const
{
    return new KeyInfoImpl(ns,name,prefix);
}
