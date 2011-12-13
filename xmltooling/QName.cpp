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
 * QName.cpp
 * 
 * Representing XML QNames 
 */

#include "internal.h"
#include "QName.h"

using namespace xmltooling;
using namespace std;

using xercesc::XMLString;

QName::QName(const XMLCh* uri, const XMLCh* localPart, const XMLCh* prefix)
{
    setNamespaceURI(uri);
    setLocalPart(localPart);
    setPrefix(prefix);
}

QName::QName(const char* uri, const char* localPart, const char* prefix)
{
    setNamespaceURI(uri);
    setLocalPart(localPart);
    setPrefix(prefix);
}

QName::~QName()
{
}

void QName::setPrefix(const XMLCh* prefix)
{
    if (prefix)
        m_prefix=prefix;
    else
        m_prefix.erase();
}

void QName::setNamespaceURI(const XMLCh* uri)
{
    if (uri)
        m_uri=uri;
    else
        m_uri.erase();
}

void QName::setLocalPart(const XMLCh* localPart)
{
    if (localPart)
        m_local=localPart;
    else
        m_local.erase();
}

void QName::setPrefix(const char* prefix)
{
    if (prefix) {
        auto_ptr_XMLCh temp(prefix);
        m_prefix=temp.get();
    }
    else
        m_prefix.erase();
}

void QName::setNamespaceURI(const char* uri)
{
    if (uri) {
        auto_ptr_XMLCh temp(uri);
        m_uri=temp.get();
    }
    else
        m_uri.erase();
}

void QName::setLocalPart(const char* localPart)
{
    if (localPart) {
        auto_ptr_XMLCh temp(localPart);
        m_local=temp.get();
    }
    else
        m_local.erase();
}

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
