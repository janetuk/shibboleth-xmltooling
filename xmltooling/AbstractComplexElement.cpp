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
 * AbstractComplexElement.cpp
 * 
 * Implementation of AbstractComplexElement.
 */

#include "internal.h"
#include "AbstractComplexElement.h"

#include <algorithm>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

using namespace xmltooling;
using namespace xercesc;
using namespace boost::lambda;
using namespace boost;
using namespace std;


AbstractComplexElement::AbstractComplexElement()
{
}

AbstractComplexElement::AbstractComplexElement(const AbstractComplexElement& src)
{
    static void (vector<XMLCh*>::* push_back)(XMLCh* const&) = &vector<XMLCh*>::push_back;
    static XMLCh* (*replicate)(const XMLCh*,MemoryManager*) = &XMLString::replicate;

    for_each(
        src.m_text.begin(), src.m_text.end(),
        lambda::bind(push_back, boost::ref(m_text), lambda::bind(replicate, _1, XMLPlatformUtils::fgMemoryManager))
        );
}

AbstractComplexElement::~AbstractComplexElement() {
    static void (*release)(XMLCh**,MemoryManager*) = &XMLString::release;

    for_each(m_children.begin(), m_children.end(), cleanup<XMLObject>());
    for_each(m_text.begin(), m_text.end(), lambda::bind(release, &_1, XMLPlatformUtils::fgMemoryManager));
}

bool AbstractComplexElement::hasChildren() const
{
    if (m_children.empty())
        return false;
    return (find_if(m_children.begin(), m_children.end(), (_1 != ((XMLObject*)nullptr))) != m_children.end());
}

const list<XMLObject*>& AbstractComplexElement::getOrderedChildren() const
{
    return m_children;
}

void AbstractComplexElement::removeChild(XMLObject* child)
{
    m_children.erase(remove(m_children.begin(), m_children.end(), child), m_children.end());
}

const XMLCh* AbstractComplexElement::getTextContent(unsigned int position) const
{
    return (m_text.size() > position) ? m_text[position] : nullptr;
}

void AbstractComplexElement::setTextContent(const XMLCh* value, unsigned int position)
{
    if (position > m_children.size())
        throw XMLObjectException("Can't set text content relative to non-existent child position.");
    vector<XMLCh*>::size_type size = m_text.size();
    while (position >= size) {
        m_text.push_back(nullptr);
        ++size;
    }
    m_text[position] = prepareForAssignment(m_text[position], value);
}
