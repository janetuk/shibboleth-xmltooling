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
 * AbstractXMLObject.cpp
 *
 * An abstract implementation of XMLObject.
 */

#include "internal.h"
#include "AbstractXMLObject.h"
#include "exceptions.h"

#include <algorithm>

using namespace xmltooling;

using xercesc::XMLString;

AbstractXMLObject::AbstractXMLObject(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
    : m_log(logging::Category::getInstance(XMLTOOLING_LOGCAT".XMLObject")),
    	m_schemaLocation(NULL), m_noNamespaceSchemaLocation(NULL), m_nil(xmlconstants::XML_BOOL_NULL),
        m_parent(NULL), m_elementQname(nsURI, localName, prefix), m_typeQname(NULL)
{
    addNamespace(Namespace(nsURI, prefix));
    if (schemaType) {
        m_typeQname = new QName(*schemaType);
        addNamespace(Namespace(m_typeQname->getNamespaceURI(), m_typeQname->getPrefix()));
    }
}

AbstractXMLObject::AbstractXMLObject(const AbstractXMLObject& src)
    : m_namespaces(src.m_namespaces), m_log(src.m_log), m_schemaLocation(XMLString::replicate(src.m_schemaLocation)),
        m_noNamespaceSchemaLocation(XMLString::replicate(src.m_noNamespaceSchemaLocation)), m_nil(src.m_nil),
        m_parent(NULL), m_elementQname(src.m_elementQname), m_typeQname(NULL)
{
    if (src.m_typeQname)
        m_typeQname=new QName(*src.m_typeQname);
}

AbstractXMLObject::~AbstractXMLObject()
{
    delete m_typeQname;
    xercesc::XMLString::release(&m_schemaLocation);
    xercesc::XMLString::release(&m_noNamespaceSchemaLocation);
}

void XMLObject::setNil(const XMLCh* value) {
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
            Namespace newNamespace(newValue->getNamespaceURI(), newValue->getPrefix());
            addNamespace(newNamespace);
            return new QName(*newValue);
        }
        return NULL;
    }

    delete oldValue;
    releaseThisandParentDOM();
    if (newValue) {
        Namespace newNamespace(newValue->getNamespaceURI(), newValue->getPrefix());
        addNamespace(newNamespace);
        return new QName(*newValue);
    }
    return NULL;
}

DateTime* AbstractXMLObject::prepareForAssignment(DateTime* oldValue, const DateTime* newValue)
{
    if (!oldValue) {
        if (newValue) {
            releaseThisandParentDOM();
            return new DateTime(*newValue);
        }
        return NULL;
    }

    delete oldValue;
    releaseThisandParentDOM();
    return newValue ? new DateTime(*newValue) : NULL;
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

void AbstractXMLObject::detach()
{
    if (!getParent())
        return;
    else if (getParent()->hasParent())
        throw XMLObjectException("Cannot detach an object whose parent is itself a child.");

    // Pull ourselves out of the parent and then blast him.
    getParent()->removeChild(this);
    delete m_parent;
    m_parent = NULL;
}
