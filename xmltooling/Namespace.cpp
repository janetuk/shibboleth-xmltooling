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
 * Namespace.cpp
 * 
 * Representing XML namespace attributes 
 */

#include "internal.h"
#include "Namespace.h"

using namespace xmltooling;

Namespace::Namespace(const XMLCh* uri, const XMLCh* prefix, bool alwaysDeclare) : m_pinned(alwaysDeclare)
{
#ifndef HAVE_GOOD_STL
    m_uri=m_prefix=NULL;
#endif
    setNamespaceURI(uri);
    setNamespacePrefix(prefix);
}

Namespace::~Namespace()
{
#ifndef HAVE_GOOD_STL
    XMLString::release(&m_uri);
    XMLString::release(&m_prefix);
#endif
}

void Namespace::setNamespacePrefix(const XMLCh* prefix)
{
#ifdef HAVE_GOOD_STL
    if (prefix)
        m_prefix=prefix;
    else
        m_prefix.erase();
#else
    if (m_prefix)
        XMLString::release(&m_prefix);
    m_prefix=XMLString::replicate(prefix);
#endif
}

void Namespace::setNamespaceURI(const XMLCh* uri)
{
#ifdef HAVE_GOOD_STL
    if (uri)
        m_uri=uri;
    else
        m_uri.erase();
#else
    if (m_uri)
        XMLString::release(&m_uri);
    m_uri=XMLString::replicate(uri);
#endif
}

#ifndef HAVE_GOOD_STL
Namespace::Namespace(const Namespace& src)
{
    m_uri=XMLString::replicate(src.getNamespaceURI());
    m_prefix=XMLString::replicate(src.getNamespacePrefix());
    m_pinned=src.getAlwaysDeclare();
}

Namespace& Namespace::operator=(const Namespace& src)
{
    m_uri=XMLString::replicate(src.getNamespaceURI());
    m_prefix=XMLString::replicate(src.getNamespacePrefix());
    m_pinned=src.getAlwaysDeclare();
    return *this;
}

bool xmltooling::operator==(const Namespace& op1, const Namespace& op2)
{
    return (XMLString::equals(op1.getNamespaceURI(),op2.getNamespaceURI()) &&
            XMLString::equals(op1.getNamespacePrefix(),op2.getNamespacePrefix()));
}
#endif

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
