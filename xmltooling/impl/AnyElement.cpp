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
 * AnyElement.cpp
 * 
 * Advanced anyType implementation suitable for deep processing of unknown content.
 */

#include "internal.h"
#include "exceptions.h"
#include "impl/AnyElement.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

XMLObject* AnyElementImpl::clone() const {
    auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone());
    AnyElementImpl* ret=dynamic_cast<AnyElementImpl*>(domClone.get());
    if (ret) {
        domClone.release();
        return ret;
    }

    return new AnyElementImpl(*this);
}

AnyElementImpl::AnyElementImpl(const AnyElementImpl& src) : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src),
    AbstractElementProxy(src), AbstractAttributeExtensibleXMLObject(src) {
    for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
        getXMLObjects().push_back((*i) ? (*i)->clone() : NULL);
    }
}       

void AnyElementImpl::marshallAttributes(DOMElement* domElement) const {
    for (map<QName,XMLCh*>::const_iterator i=m_attributeMap.begin(); i!=m_attributeMap.end(); i++) {
        DOMAttr* attr=domElement->getOwnerDocument()->createAttributeNS(i->first.getNamespaceURI(),i->first.getLocalPart());
        if (i->first.hasPrefix())
            attr->setPrefix(i->first.getPrefix());
        attr->setNodeValue(i->second);
        domElement->setAttributeNode(attr);
    }
}

void AnyElementImpl::marshallElementContent(DOMElement* domElement) const {
    if(getTextContent()) {
        domElement->appendChild(domElement->getOwnerDocument()->createTextNode(getTextContent()));
    }
}

void AnyElementImpl::processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
    getXMLObjects().push_back(childXMLObject);
}

void AnyElementImpl::processAttribute(const DOMAttr* attribute) {
    QName q(attribute->getNamespaceURI(),attribute->getLocalName(),attribute->getPrefix()); 
    setAttribute(q,attribute->getNodeValue());
}

void AnyElementImpl::processElementContent(const XMLCh* elementContent) {
    setTextContent(elementContent);
}

XMLObject* AnyElementBuilder::buildObject(
    const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType
    ) const {
    return new AnyElementImpl(nsURI, localName, prefix, schemaType);
}
