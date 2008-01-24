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
 * QName.cpp
 * 
 * Representing XML QNames 
 */

#include "internal.h"
#include "QName.h"

using namespace xmltooling;
using namespace std;

QName::QName(const XMLCh* uri, const XMLCh* localPart, const XMLCh* prefix)
{
#ifndef HAVE_GOOD_STL
    m_uri=m_prefix=m_local=NULL;
#endif
    setNamespaceURI(uri);
    setLocalPart(localPart);
    setPrefix(prefix);
}

QName::QName(const char* uri, const char* localPart, const char* prefix)
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

void QName::setPrefix(const char* prefix)
{
#ifdef HAVE_GOOD_STL
    if (prefix) {
        auto_ptr_XMLCh temp(prefix);
        m_prefix=temp.get();
    }
    else
        m_prefix.erase();
#else
    if (m_prefix)
        XMLString::release(&m_prefix);
    m_prefix=XMLString::transcode(prefix);
#endif
}

void QName::setNamespaceURI(const char* uri)
{
#ifdef HAVE_GOOD_STL
    if (uri) {
        auto_ptr_XMLCh temp(uri);
        m_uri=temp.get();
    }
    else
        m_uri.erase();
#else
    if (m_uri)
        XMLString::release(&m_uri);
    m_uri=XMLString::transcode(uri);
#endif
}

void QName::setLocalPart(const char* localPart)
{
#ifdef HAVE_GOOD_STL
    if (localPart) {
        auto_ptr_XMLCh temp(localPart);
        m_local=temp.get();
    }
    else
        m_local.erase();
#else
    if (m_local)
        XMLString::release(&m_local);
    m_local=XMLString::transcode(localPart);
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
#endif

bool xmltooling::operator==(const QName& op1, const QName& op2)
{
    if (&op1 == &op2)
        return true;
    return (!XMLString::compareString(op1.getNamespaceURI(),op2.getNamespaceURI()) &&
            !XMLString::compareString(op1.getLocalPart(),op2.getLocalPart()));
}

bool xmltooling::operator!=(const QName& op1, const QName& op2)
{
    return !(op1==op2);
}

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

string QName::toString() const
{
    if (!hasLocalPart())
        return "";
    auto_ptr_char local(getLocalPart());
    if (hasPrefix()) {
        auto_ptr_char pre(getPrefix());
        return string(pre.get()) + ':' + local.get(); 
    }
    else if (hasNamespaceURI()) {
        auto_ptr_char ns(getNamespaceURI());
        return string("{") + ns.get() + '}' + local.get(); 
    }
    else
        return local.get();
}
