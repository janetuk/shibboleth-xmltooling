/*
 *  Copyright 2001-2009 Internet2
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
 * Namespace.cpp
 * 
 * Representing XML namespace attributes 
 */

#include "internal.h"
#include "Namespace.h"

using namespace xmltooling;

using xercesc::XMLString;

Namespace::Namespace(const XMLCh* uri, const XMLCh* prefix, bool alwaysDeclare) : m_pinned(alwaysDeclare)
{
    setNamespaceURI(uri);
    setNamespacePrefix(prefix);
}

Namespace::~Namespace()
{
}

void Namespace::setNamespacePrefix(const XMLCh* prefix)
{
    if (prefix)
        m_prefix=prefix;
    else
        m_prefix.erase();
}

void Namespace::setNamespaceURI(const XMLCh* uri)
{
    if (uri)
        m_uri=uri;
    else
        m_uri.erase();
}

bool xmltooling::operator<(const Namespace& op1, const Namespace& op2)
{
    int i=XMLString::compareString(op1.getNamespaceURI(),op2.getNamespaceURI());
    if (i<0)
        return true;
    else if (i==0)
        return (XMLString::compareString(op1.getNamespacePrefix(),op2.getNamespacePrefix())<0);
    else
        return false;
}
