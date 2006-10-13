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
 * ValidatorSuite.cpp
 * 
 * Groups of rule checkers of XMLObjects based on type or element name. 
 */

#include "internal.h"
#include "validation/ValidatorSuite.h"
#include "util/XMLHelper.h"

using namespace xmltooling;
using namespace std;

ValidatorSuite xmltooling::SchemaValidators("SchemaValidators");

void ValidatorSuite::deregisterValidators(const QName& key)
{
    pair<multimap<QName,Validator*>::iterator,multimap<QName,Validator*>::iterator> range=m_map.equal_range(key);
    for_each(range.first, range.second, xmltooling::cleanup_pair<QName,Validator>());
    m_map.erase(range.first, range.second);
}

void ValidatorSuite::destroyValidators()
{
    for_each(m_map.begin(),m_map.end(),xmltooling::cleanup_pair<QName,Validator>());
    m_map.clear();
}

void ValidatorSuite::validate(const XMLObject* xmlObject) const
{
    if (!xmlObject)
        return;

    pair<multimap<QName,Validator*>::const_iterator,multimap<QName,Validator*>::const_iterator> range;
    if (xmlObject->getSchemaType()) {
        range=m_map.equal_range(*(xmlObject->getSchemaType()));
        while (range.first!=range.second) {
            range.first->second->validate(xmlObject);
            ++range.first;
        }
    }
    range=m_map.equal_range(xmlObject->getElementQName());
    while (range.first!=range.second) {
        range.first->second->validate(xmlObject);
        ++range.first;
    }

    const list<XMLObject*>& kids=xmlObject->getOrderedChildren();
    for (list<XMLObject*>::const_iterator j=kids.begin(); j!=kids.end(); j++)
        validate(*j);
}
