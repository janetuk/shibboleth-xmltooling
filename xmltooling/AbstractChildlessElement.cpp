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
 * AbstractChildlessElement.cpp
 * 
 * Extension of AbstractXMLObject that implements childlessness 
 */

#include "internal.h"
#include "AbstractChildlessElement.h"

using namespace xmltooling;
using namespace std;

// shared "empty" list of children for childless objects

list<XMLObject*> AbstractChildlessElement::m_no_children;

void AbstractChildlessElement::removeChild(XMLObject* child)
{
    throw XMLObjectException("Cannot remove child from a childless object.");
}
