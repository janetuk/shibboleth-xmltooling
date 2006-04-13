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
 * Validator.cpp
 * 
 * Rules checking of XMLObjects 
 */

#include "internal.h"
#include "validation/Validator.h"
#include "util/XMLHelper.h"

using namespace xmltooling;
using namespace std;

map< QName, vector<Validator*> > Validator::m_map;

void Validator::checkValidity(const XMLObject* xmlObject)
{
    if (!xmlObject)
        return;

    map< QName, vector<Validator*> >::iterator i;
    if (xmlObject->getSchemaType()) {
        i=m_map.find(*(xmlObject->getSchemaType()));
        if (i!=m_map.end())
            for_each(i->second.begin(),i->second.end(),bind2nd(mem_fun<void,Validator,const XMLObject*>(&Validator::validate),xmlObject));
    }
    i=m_map.find(xmlObject->getElementQName());
    if (i!=m_map.end())
        for_each(i->second.begin(),i->second.end(),bind2nd(mem_fun<void,Validator,const XMLObject*>(&Validator::validate),xmlObject));

    const list<XMLObject*>& kids=xmlObject->getOrderedChildren();
    for_each(kids.begin(),kids.end(),Validator::checkValidity);
}

class _clearvector {
public:
    void operator()(const pair< QName, vector<Validator*> >& p) const {
        for_each(p.second.begin(),p.second.end(),xmltooling::cleanup<Validator>());
    }
};

void Validator::deregisterValidators(const QName& key)
{
    map< QName, vector<Validator*> >::iterator i=m_map.find(key);
    if (i!=m_map.end()) {
        _clearvector f;
        f(*i);
        m_map.erase(i);
    }
}

void Validator::destroyValidators()
{
    for_each(m_map.begin(),m_map.end(),_clearvector());
    m_map.clear();
}
