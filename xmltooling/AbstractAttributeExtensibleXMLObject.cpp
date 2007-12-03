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
 * AbstractAttributeExtensibleXMLObject.cpp
 * 
 * Extension of AbstractDOMCachingXMLObject that implements an AttributeExtensibleXMLObject. 
 */

#include "internal.h"
#include "AbstractAttributeExtensibleXMLObject.h"

#include <algorithm>
#include <functional>

using namespace xmltooling;
using namespace std;

set<QName> AttributeExtensibleXMLObject::m_idAttributeSet;

AbstractAttributeExtensibleXMLObject::~AbstractAttributeExtensibleXMLObject()
{
    for (map<QName,XMLCh*>::iterator i=m_attributeMap.begin(); i!=m_attributeMap.end(); i++)
        XMLString::release(&(i->second));
}

AbstractAttributeExtensibleXMLObject::AbstractAttributeExtensibleXMLObject(const AbstractAttributeExtensibleXMLObject& src)
    : AbstractXMLObject(src)
{
    m_idAttribute = m_attributeMap.end();
    for (map<QName,XMLCh*>::const_iterator i=src.m_attributeMap.begin(); i!=src.m_attributeMap.end(); i++) {
        m_attributeMap[i->first] = XMLString::replicate(i->second);
    }
    if (src.m_idAttribute != src.m_attributeMap.end()) {
        m_idAttribute = m_attributeMap.find(src.m_idAttribute->first);
    }
}

void AbstractAttributeExtensibleXMLObject::setAttribute(const QName& qualifiedName, const XMLCh* value, bool ID)
{
    map<QName,XMLCh*>::iterator i=m_attributeMap.find(qualifiedName);
    if (i!=m_attributeMap.end()) {
        releaseThisandParentDOM();
        XMLString::release(&(i->second));
        if (value && *value) {
            i->second=XMLString::replicate(value);
            if (ID)
                m_idAttribute=i;
        }
        else {
            if (m_idAttribute==i)
                m_idAttribute=m_attributeMap.end();
            m_attributeMap.erase(i);
        }
    }
    else if (value && *value) {
        releaseThisandParentDOM();
        m_attributeMap[qualifiedName]=XMLString::replicate(value);
        if (ID)
            m_idAttribute = m_attributeMap.find(qualifiedName);
    }
}

void AbstractAttributeExtensibleXMLObject::unmarshallExtensionAttribute(const DOMAttr* attribute)
{
    QName q(attribute->getNamespaceURI(),attribute->getLocalName(),attribute->getPrefix());
    bool ID = attribute->isId() || isRegisteredIDAttribute(q);
    setAttribute(q,attribute->getNodeValue(),ID);
    if (ID) {
        attribute->getOwnerElement()->setIdAttributeNode(attribute);
    }
}

void AbstractAttributeExtensibleXMLObject::marshallExtensionAttributes(DOMElement* domElement) const
{
    for (map<QName,XMLCh*>::const_iterator i=m_attributeMap.begin(); i!=m_attributeMap.end(); i++) {
        DOMAttr* attr=domElement->getOwnerDocument()->createAttributeNS(i->first.getNamespaceURI(),i->first.getLocalPart());
        if (i->first.hasPrefix())
            attr->setPrefix(i->first.getPrefix());
        attr->setNodeValue(i->second);
        domElement->setAttributeNodeNS(attr);
        if (m_idAttribute==i)
            domElement->setIdAttributeNode(attr);
    }
}
