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
 * AbstractAttributeExtensibleXMLObject.cpp
 * 
 * Extension of AbstractDOMCachingXMLObject that implements an AttributeExtensibleXMLObject. 
 */

#include "internal.h"
#include "AbstractAttributeExtensibleXMLObject.h"
#include "ElementExtensibleXMLObject.h"
#include "ElementProxy.h"

#include <algorithm>
#include <functional>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace xercesc;
using namespace boost::lambda;
using namespace boost;
using namespace std;


ElementExtensibleXMLObject::ElementExtensibleXMLObject()
{
}

ElementExtensibleXMLObject::~ElementExtensibleXMLObject()
{
}

ElementProxy::ElementProxy()
{
}

ElementProxy::~ElementProxy()
{
}

set<xmltooling::QName> AttributeExtensibleXMLObject::m_idAttributeSet;

AttributeExtensibleXMLObject::AttributeExtensibleXMLObject()
{
}

AttributeExtensibleXMLObject::~AttributeExtensibleXMLObject()
{
}

const set<xmltooling::QName>& AttributeExtensibleXMLObject::getRegisteredIDAttributes()
{
    return m_idAttributeSet;
}

bool AttributeExtensibleXMLObject::isRegisteredIDAttribute(const xmltooling::QName& name)
{
    return m_idAttributeSet.find(name)!=m_idAttributeSet.end();
}

void AttributeExtensibleXMLObject::registerIDAttribute(const xmltooling::QName& name)
{
    m_idAttributeSet.insert(name);
}

void AttributeExtensibleXMLObject::deregisterIDAttribute(const xmltooling::QName& name)
{
    m_idAttributeSet.erase(name);
}

void AttributeExtensibleXMLObject::deregisterIDAttributes()
{
    m_idAttributeSet.clear();
}

AbstractAttributeExtensibleXMLObject::AbstractAttributeExtensibleXMLObject()
{
    m_idAttribute = m_attributeMap.end();
}

AbstractAttributeExtensibleXMLObject::AbstractAttributeExtensibleXMLObject(const AbstractAttributeExtensibleXMLObject& src)
    : AbstractXMLObject(src)
{
    m_idAttribute = m_attributeMap.end();
    for (map<xmltooling::QName,XMLCh*>::const_iterator i = src.m_attributeMap.begin(); i != src.m_attributeMap.end(); ++i) {
        m_attributeMap[i->first] = XMLString::replicate(i->second);
    }
    if (src.m_idAttribute != src.m_attributeMap.end()) {
        m_idAttribute = m_attributeMap.find(src.m_idAttribute->first);
    }
}

AbstractAttributeExtensibleXMLObject::~AbstractAttributeExtensibleXMLObject()
{
#ifdef XMLTOOLING_XERCESC_HAS_XMLBYTE_RELEASE
    static void (*release)(XMLCh**) = &XMLString::release;
#else
    static void (*release)(XMLCh**,MemoryManager*) = &XMLString::release;
#endif
    for_each(
        m_attributeMap.begin(), m_attributeMap.end(),
        lambda::bind(
            release,
            &lambda::bind(&map<xmltooling::QName,XMLCh*>::value_type::second, boost::ref(_1))
#ifndef XMLTOOLING_XERCESC_HAS_XMLBYTE_RELEASE
            ,XMLPlatformUtils::fgMemoryManager
#endif
            )
        );
}

const XMLCh* AbstractAttributeExtensibleXMLObject::getAttribute(const xmltooling::QName& qualifiedName) const
{
    map<xmltooling::QName,XMLCh*>::const_iterator i = m_attributeMap.find(qualifiedName);
    return (i != m_attributeMap.end()) ? i->second : nullptr;
}

void AbstractAttributeExtensibleXMLObject::setAttribute(const xmltooling::QName& qualifiedName, const XMLCh* value, bool ID)
{
    map<xmltooling::QName,XMLCh*>::iterator i=m_attributeMap.find(qualifiedName);
    if (i != m_attributeMap.end()) {
        releaseThisandParentDOM();
        XMLString::release(&(i->second));
        if (value && *value) {
            i->second = XMLString::replicate(value);
            if (ID)
                m_idAttribute = i;
        }
        else {
            if (m_idAttribute == i)
                m_idAttribute = m_attributeMap.end();
            m_attributeMap.erase(i);
        }
    }
    else if (value && *value) {
        releaseThisandParentDOM();
        m_attributeMap[qualifiedName] = XMLString::replicate(value);
        if (ID)
            m_idAttribute = m_attributeMap.find(qualifiedName);
        Namespace newNamespace(qualifiedName.getNamespaceURI(), qualifiedName.getPrefix(), false, Namespace::VisiblyUsed);
        addNamespace(newNamespace);
    }
}

void AttributeExtensibleXMLObject::setAttribute(const xmltooling::QName& qualifiedName, const xmltooling::QName& value)
{
    if (!value.hasLocalPart())
        return;

    if (value.hasPrefix()) {
        xstring buf(value.getPrefix());
        buf = buf + chColon + value.getLocalPart();
        setAttribute(qualifiedName, buf.c_str());
    }
    else {
        setAttribute(qualifiedName, value.getLocalPart());
    }

    Namespace newNamespace(value.getNamespaceURI(), value.getPrefix(), false, Namespace::NonVisiblyUsed);
    addNamespace(newNamespace);
}

const map<xmltooling::QName,XMLCh*>& AbstractAttributeExtensibleXMLObject::getExtensionAttributes() const
{
    return m_attributeMap;
}
const XMLCh* AbstractAttributeExtensibleXMLObject::getXMLID() const
{
    return (m_idAttribute != m_attributeMap.end()) ? m_idAttribute->second : nullptr;
}

void AbstractAttributeExtensibleXMLObject::unmarshallExtensionAttribute(const DOMAttr* attribute)
{
    xmltooling::QName q(attribute->getNamespaceURI(), attribute->getLocalName(), attribute->getPrefix());
    bool ID = attribute->isId() || isRegisteredIDAttribute(q);
    setAttribute(q,attribute->getNodeValue(),ID);
    if (ID) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
        attribute->getOwnerElement()->setIdAttributeNode(attribute, true);
#else
        attribute->getOwnerElement()->setIdAttributeNode(attribute);
#endif
    }
}

void AbstractAttributeExtensibleXMLObject::marshallExtensionAttributes(DOMElement* domElement) const
{
    for (map<xmltooling::QName,XMLCh*>::const_iterator i = m_attributeMap.begin(); i != m_attributeMap.end(); ++i) {
        DOMAttr* attr = domElement->getOwnerDocument()->createAttributeNS(i->first.getNamespaceURI(), i->first.getLocalPart());
        if (i->first.hasPrefix())
            attr->setPrefix(i->first.getPrefix());
        attr->setNodeValue(i->second);
        domElement->setAttributeNodeNS(attr);
        if (m_idAttribute == i) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
            domElement->setIdAttributeNode(attr, true);
#else
            domElement->setIdAttributeNode(attr);
#endif
        }
    }
}
