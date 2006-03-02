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
 * Unmarshaller.cpp
 * 
 * Transforms DOM trees into XMLObjects 
 */

#include "internal.h"
#include "Unmarshaller.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

map<QName,Unmarshaller*> Unmarshaller::m_map;

Unmarshaller* Unmarshaller::m_default=NULL;

const Unmarshaller* Unmarshaller::getUnmarshaller(const DOMElement* domElement)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("getUnmarshaller");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Unmarshaller");
 
    auto_ptr<QName> schemaType(XMLHelper::getXSIType(domElement));
    const Unmarshaller* m = schemaType.get() ? getUnmarshaller(*(schemaType.get())) : NULL;
    if (m) {
        if (log.isDebugEnabled()) {
            log.debug("located Unmarshaller for schema type: %s", schemaType->toString().c_str());
        }
        return m;
    }
    
    auto_ptr<QName> elementName(XMLHelper::getNodeQName(domElement));
    m = getUnmarshaller(*(elementName.get()));
    if (m) {
        if (log.isDebugEnabled()) {
            log.debug("located Unmarshaller for element name: %s", elementName->toString().c_str());
        }
        return m;
    }

    if (log.isDebugEnabled()) {
        log.debug("no Unmarshaller registered for element (%s), returning default", elementName->toString().c_str());
    }
    return m_default;
}

void Unmarshaller::destroyUnmarshallers()
{
    for_each(m_map.begin(),m_map.end(),cleanup_pair<QName,Unmarshaller>());
    m_map.clear();
    deregisterDefaultUnmarshaller();
}
