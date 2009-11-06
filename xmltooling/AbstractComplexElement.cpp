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
 * AbstractComplexElement.cpp
 * 
 * Implementation of AbstractComplexElement.
 */

#include "internal.h"
#include "AbstractComplexElement.h"

#include <algorithm>

using namespace xmltooling;
using namespace std;

using xercesc::XMLString;

namespace {
    bool _nonnull(const XMLObject* ptr) {
        return (ptr!=NULL);
    }
}

AbstractComplexElement::AbstractComplexElement()
{
}

AbstractComplexElement::AbstractComplexElement(const AbstractComplexElement& src)
{
    for (vector<XMLCh*>::const_iterator i=src.m_text.begin(); i!=src.m_text.end(); ++i)
        m_text.push_back(XMLString::replicate(*i));
}

AbstractComplexElement::~AbstractComplexElement() {
    for_each(m_children.begin(), m_children.end(), cleanup<XMLObject>());
    for (vector<XMLCh*>::iterator i=m_text.begin(); i!=m_text.end(); ++i)
        XMLString::release(&(*i));
}

bool AbstractComplexElement::hasChildren() const
{
    if (m_children.empty())
        return false;
    return (find_if(m_children.begin(), m_children.end(), _nonnull) != m_children.end());
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
    return (m_text.size() > position) ? m_text[position] : NULL;
}

void AbstractComplexElement::setTextContent(const XMLCh* value, unsigned int position)
{
    if (position > m_children.size())
        throw XMLObjectException("Can't set text content relative to non-existent child position.");
    vector<XMLCh*>::size_type size = m_text.size();
    while (position >= size) {
        m_text.push_back(NULL);
        ++size;
    }
    m_text[position]=prepareForAssignment(m_text[position],value);
}
