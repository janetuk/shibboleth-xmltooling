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
 * AbstractXMLObject.cpp
 *
 * An abstract implementation of XMLObject.
 */

#include "internal.h"
#include "exceptions.h"
#include "AbstractXMLObject.h"
#include "util/DateTime.h"

#include <algorithm>

using namespace xmltooling;
using std::set;

using xercesc::XMLString;

XMLObject::XMLObject()
{
}

XMLObject::~XMLObject()
{
}

void XMLObject::releaseThisandParentDOM() const
{
    releaseDOM();
    releaseParentDOM(true);
}

void XMLObject::releaseThisAndChildrenDOM() const
{
    releaseChildrenDOM(true);
    releaseDOM();
}

AbstractXMLObject::AbstractXMLObject(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
    : m_log(logging::Category::getInstance(XMLTOOLING_LOGCAT".XMLObject")),
    	m_schemaLocation(nullptr), m_noNamespaceSchemaLocation(nullptr), m_nil(xmlconstants::XML_BOOL_NULL),
        m_parent(nullptr), m_elementQname(nsURI, localName, prefix)
{
    addNamespace(Namespace(nsURI, prefix, false, Namespace::VisiblyUsed));
    if (schemaType) {
        m_typeQname.reset(new QName(*schemaType));
        addNamespace(Namespace(m_typeQname->getNamespaceURI(), m_typeQname->getPrefix(), false, Namespace::NonVisiblyUsed));
    }
}

AbstractXMLObject::AbstractXMLObject(const AbstractXMLObject& src)
    : m_namespaces(src.m_namespaces), m_log(src.m_log), m_schemaLocation(XMLString::replicate(src.m_schemaLocation)),
        m_noNamespaceSchemaLocation(XMLString::replicate(src.m_noNamespaceSchemaLocation)), m_nil(src.m_nil),
        m_parent(nullptr), m_elementQname(src.m_elementQname),
        m_typeQname(src.m_typeQname.get() ? new QName(*src.m_typeQname) : nullptr)
{
}

AbstractXMLObject::~AbstractXMLObject()
{
    xercesc::XMLString::release(&m_schemaLocation);
    xercesc::XMLString::release(&m_noNamespaceSchemaLocation);
}

void AbstractXMLObject::detach()
{
    if (!getParent())
        return;
    else if (getParent()->hasParent())
        throw XMLObjectException("Cannot detach an object whose parent is itself a child.");

    // Pull ourselves out of the parent and then blast him.
    getParent()->removeChild(this);
    delete m_parent;
    m_parent = nullptr;
}

const QName& AbstractXMLObject::getElementQName() const
{
    return m_elementQname;
}

const set<Namespace>& AbstractXMLObject::getNamespaces() const
{
    return m_namespaces;
}

void XMLObject::setNil(const XMLCh* value)
{
    if (value) {
        switch (*value) {
            case xercesc::chLatin_t:
                nil(xmlconstants::XML_BOOL_TRUE);
                break;
            case xercesc::chLatin_f:
                nil(xmlconstants::XML_BOOL_FALSE);
                break;
            case xercesc::chDigit_1:
                nil(xmlconstants::XML_BOOL_ONE);
                break;
            case xercesc::chDigit_0:
                nil(xmlconstants::XML_BOOL_ZERO);
                break;
            default:
                nil(xmlconstants::XML_BOOL_NULL);
        }
    }
    else {
        nil(xmlconstants::XML_BOOL_NULL);
    }
}

void AbstractXMLObject::addNamespace(const Namespace& ns) const
{
    for (set<Namespace>::const_iterator n = m_namespaces.begin(); n != m_namespaces.end(); ++n) {
        // Look for the prefix in the existing set.
        if (XMLString::equals(ns.getNamespacePrefix(), n->getNamespacePrefix())) {
            // See if it's the same declaration, and overlay various properties if so.
            if (XMLString::equals(ns.getNamespaceURI(), n->getNamespaceURI())) {
                if (ns.alwaysDeclare())
                    const_cast<Namespace&>(*n).setAlwaysDeclare(true);
                switch (ns.usage()) {
                    case Namespace::Indeterminate:
                        break;
                    case Namespace::VisiblyUsed:
                        const_cast<Namespace&>(*n).setUsage(Namespace::VisiblyUsed);
                        break;
                    case Namespace::NonVisiblyUsed:
                        if (n->usage() == Namespace::Indeterminate)
                            const_cast<Namespace&>(*n).setUsage(Namespace::NonVisiblyUsed);
                        break;
                }
            }
            return;
        }
    }

    // If the prefix is now, go ahead and add it.
    m_namespaces.insert(ns);
}

void AbstractXMLObject::removeNamespace(const Namespace& ns)
{
    m_namespaces.erase(ns);
}

const QName* AbstractXMLObject::getSchemaType() const
{
    return m_typeQname.get();
}

const XMLCh* AbstractXMLObject::getXMLID() const
{
    return nullptr;
}

xmlconstants::xmltooling_bool_t AbstractXMLObject::getNil() const
{
    return m_nil;
}

void AbstractXMLObject::nil(xmlconstants::xmltooling_bool_t value)
{
    if (m_nil != value) {
        releaseThisandParentDOM();
        m_nil = value;
    }
}

bool AbstractXMLObject::hasParent() const
{
    return m_parent != nullptr;
}

XMLObject* AbstractXMLObject::getParent() const
{
    return m_parent;
}

void AbstractXMLObject::setParent(XMLObject* parent)
{
    m_parent = parent;
}

XMLCh* AbstractXMLObject::prepareForAssignment(XMLCh* oldValue, const XMLCh* newValue)
{
    if (!XMLString::equals(oldValue,newValue)) {
        releaseThisandParentDOM();
        XMLCh* newString = XMLString::replicate(newValue);
        XMLString::release(&oldValue);
        return newString;
    }
    return oldValue;
}

QName* AbstractXMLObject::prepareForAssignment(QName* oldValue, const QName* newValue)
{
    if (!oldValue) {
        if (newValue) {
            releaseThisandParentDOM();
            addNamespace(Namespace(newValue->getNamespaceURI(), newValue->getPrefix(), false, Namespace::NonVisiblyUsed));
            return new QName(*newValue);
        }
        return nullptr;
    }

    delete oldValue;
    releaseThisandParentDOM();
    if (newValue) {
        // Attach a non-visibly used namespace.
        addNamespace(Namespace(newValue->getNamespaceURI(), newValue->getPrefix(), false, Namespace::NonVisiblyUsed));
        return new QName(*newValue);
    }
    return nullptr;
}

DateTime* AbstractXMLObject::prepareForAssignment(DateTime* oldValue, const DateTime* newValue)
{
    if (!oldValue) {
        if (newValue) {
            releaseThisandParentDOM();
            return new DateTime(*newValue);
        }
        return nullptr;
    }

    delete oldValue;
    releaseThisandParentDOM();
    return newValue ? new DateTime(*newValue) : nullptr;
}

DateTime* AbstractXMLObject::prepareForAssignment(DateTime* oldValue, time_t newValue, bool duration)
{
    delete oldValue;
    releaseThisandParentDOM();
    DateTime* ret = new DateTime(newValue, duration);
    if (duration)
        ret->parseDuration();
    else
        ret->parseDateTime();
    return ret;
}

DateTime* AbstractXMLObject::prepareForAssignment(DateTime* oldValue, const XMLCh* newValue, bool duration)
{
    delete oldValue;
    releaseThisandParentDOM();
    if (!newValue || !*newValue)
        return nullptr;
    DateTime* ret = new DateTime(newValue);
    if (duration)
        ret->parseDuration();
    else
        ret->parseDateTime();
    return ret;
}

XMLObject* AbstractXMLObject::prepareForAssignment(XMLObject* oldValue, XMLObject* newValue)
{
    if (newValue && newValue->hasParent())
        throw XMLObjectException("child XMLObject cannot be added - it is already the child of another XMLObject");

    if (!oldValue) {
        if (newValue) {
            releaseThisandParentDOM();
            newValue->setParent(this);
        }
        return newValue;
    }

    if (oldValue != newValue) {
        delete oldValue;
        releaseThisandParentDOM();
        if (newValue)
            newValue->setParent(this);
    }

    return newValue;
}
