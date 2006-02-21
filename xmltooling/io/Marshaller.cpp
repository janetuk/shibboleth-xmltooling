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
 * Marshaller.cpp
 * 
 * Transforms XMLObjects into DOM trees 
 */

#include "internal.h"
#include "Marshaller.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

map<QName,Marshaller*> Marshaller::m_map;

Marshaller* Marshaller::m_default=NULL;

const Marshaller* Marshaller::getMarshaller(const DOMElement* domElement)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("getMarshaller");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Marshaller");
 
    auto_ptr<QName> schemaType(XMLHelper::getXSIType(domElement));
    const Marshaller* m = getMarshaller(*(schemaType.get()));
    if (m) {
        if (log.isDebugEnabled()) {
            log.debug("Located Marshaller for schema type: %s", schemaType->toString().c_str());
        }
        return m;
    }
    
    auto_ptr<QName> elementName(XMLHelper::getNodeQName(domElement));
    m = getMarshaller(*(elementName.get()));
    if (m) {
        if (log.isDebugEnabled()) {
            log.debug("Located Marshaller for element name: %s", elementName->toString().c_str());
        }
        return m;
    }

    log.error("No Marshaller was registered for element: %s", elementName->toString().c_str());
    return NULL;
}

void Marshaller::destroyMarshallers()
{
    for_each(m_map.begin(),m_map.end(),cleanup_pair<QName,Marshaller>());
    m_map.clear();
    deregisterDefaultMarshaller();
}
