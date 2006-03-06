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
 * AbstractExtensibleXMLObject.cpp
 * 
 * Extension of AbstractDOMCachingXMLObject that implements an ExtensibleXMLObject. 
 */

#include "internal.h"
#include "AbstractExtensibleXMLObject.h"

using namespace xmltooling;
using namespace std;

void AbstractExtensibleXMLObject::setTextContent(const XMLCh* value)
{
    m_value=prepareForAssignment(m_value,value);
}

ListOf(XMLObject) AbstractExtensibleXMLObject::getXMLObjects()
{
    return ListOf(XMLObject)(this,m_children,NULL,m_children.end());
}
