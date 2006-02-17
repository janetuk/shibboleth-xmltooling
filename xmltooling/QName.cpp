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
 * @file QName.cpp
 * 
 * Representing XML QNames 
 */

#include "internal.h"
#include "QName.h"

using namespace xmltooling;

QName::QName(const XMLCh* uri, const XMLCh* localPart, const XMLCh* prefix)
{
#ifndef HAVE_GOOD_STL
    m_uri=m_prefix=m_local=NULL;
#endif
    setNamespaceURI(uri);
    setLocalPart(localPart);
    setPrefix(prefix);
}

QName::~QName()
{
#ifndef HAVE_GOOD_STL
    XMLString::release(&m_uri);
    XMLString::release(&m_prefix);
    XMLString::release(&m_local);
#endif
}

void QName::setPrefix(const XMLCh* prefix)
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

void QName::setNamespaceURI(const XMLCh* uri)
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

void QName::setLocalPart(const XMLCh* localPart)
{
#ifdef HAVE_GOOD_STL
    if (localPart)
        m_local=localPart;
    else
        m_local.erase();
#else
    if (m_local)
        XMLString::release(&m_local);
    m_local=XMLString::replicate(localPart);
#endif
}

#ifndef HAVE_GOOD_STL
QName::QName(const QName& src)
{
    m_uri=XMLString::replicate(src.getNamespaceURI());
    m_prefix=XMLString::replicate(src.getPrefix());
    m_local=XMLString::replicate(src.getLocalPart());
}

QName& QName::operator=(const QName& src)
{
    m_uri=XMLString::replicate(src.getNamespaceURI());
    m_prefix=XMLString::replicate(src.getPrefix());
    m_local=XMLString::replicate(src.getLocalPart());
    return *this;
}

bool xmltooling::operator==(const QName& op1, const QName& op2)
{
    return (!XMLString::compareString(op1.getNamespaceURI(),op2.getNamespaceURI()) &&
            !XMLString::compareString(op1.getLocalPart(),op2.getLocalPart()));
}
#endif

bool xmltooling::operator<(const QName& op1, const QName& op2)
{
    int i=XMLString::compareString(op1.getNamespaceURI(),op2.getNamespaceURI());
    if (i<0)
        return true;
    else if (i==0)
        return (XMLString::compareString(op1.getLocalPart(),op2.getLocalPart())<0);
    else
        return false;
}
