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
 * AbstractXMLObject.cpp
 * 
 * An abstract implementation of XMLObject.
 */

#include "internal.h"
#include "AbstractXMLObject.h"

#include <algorithm>
#include <log4cpp/Category.hh>

using namespace xmltooling;

AbstractXMLObject::~AbstractXMLObject() {
    delete m_typeQname;
    std::for_each(m_children.begin(), m_children.end(), cleanup<XMLObject>());
}

AbstractXMLObject::AbstractXMLObject(const XMLCh* namespaceURI, const XMLCh* elementLocalName, const XMLCh* namespacePrefix)
    : m_elementQname(namespaceURI,elementLocalName, namespacePrefix), m_typeQname(NULL), m_parent(NULL),
        m_log(&log4cpp::Category::getInstance(XMLTOOLING_LOGCAT".XMLObject"))
{
    addNamespace(Namespace(namespaceURI, namespacePrefix));
}
