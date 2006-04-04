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
 * AbstractValidatingXMLObject.cpp
 * 
 * Extension of AbstractXMLObject that implements a ValidatingXMLObject. 
 */

#include "internal.h"
#include "exceptions.h"
#include "validation/AbstractValidatingXMLObject.h"

#include <algorithm>
#include <functional>

using namespace xmltooling;
using namespace std;

AbstractValidatingXMLObject::ValidatorWrapper::~ValidatorWrapper()
{
    for_each(v.begin(),v.end(),cleanup<Validator>());
}

AbstractValidatingXMLObject::AbstractValidatingXMLObject(const AbstractValidatingXMLObject& src) : AbstractXMLObject(src)
{
    if (src.m_validators) {
        m_validators=new ValidatorWrapper();
        xmltooling::clone(src.m_validators->v,m_validators->v);
    }
}

AbstractValidatingXMLObject::~AbstractValidatingXMLObject()
{
    delete m_validators;
}

void AbstractValidatingXMLObject::registerValidator(Validator* validator) const
{
    if (!m_validators)
        m_validators=new ValidatorWrapper();
    m_validators->v.push_back(validator);
}

void AbstractValidatingXMLObject::deregisterValidator(Validator* validator) const
{
    if (m_validators) {
        for (std::vector<Validator*>::iterator i=m_validators->v.begin(); i!=m_validators->v.end(); i++) {
            if ((*i)==validator)
                m_validators->v.erase(i);
                return;
        }
    }
}

class _validate : public binary_function<const XMLObject*,bool,void> {
public:
    void operator()(const XMLObject* obj, bool propagate) const {
        const ValidatingXMLObject* val = dynamic_cast<const ValidatingXMLObject*>(obj);
        if (val) {
            val->validate(propagate);
        }
    }
};

void AbstractValidatingXMLObject::validate(bool validateDescendants) const
{
    if (m_validators) {
        for_each(
            m_validators->v.begin(),m_validators->v.end(),
            bind2nd(mem_fun<void,Validator,const XMLObject*>(&Validator::validate),this)
            );
    }
    
    if (validateDescendants && hasChildren()) {
        const list<XMLObject*>& children=getOrderedChildren();
        for_each(children.begin(),children.end(),bind2nd(_validate(),validateDescendants));
    }
}
