/*
 *  Copyright 2001-2010 Internet2
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
 * XMLObjectBuilder.cpp
 * 
 * Factory interface for XMLObjects 
 */

#include "internal.h"
#include "logging.h"
#include "ConcreteXMLObjectBuilder.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

using xercesc::DOMDocument;
using xercesc::DOMElement;

map<QName,XMLObjectBuilder*> XMLObjectBuilder::m_map;
XMLObjectBuilder* XMLObjectBuilder::m_default = nullptr;

XMLObjectBuilder::XMLObjectBuilder()
{
}

XMLObjectBuilder::~XMLObjectBuilder()
{
}

XMLObject* XMLObjectBuilder::buildFromQName(const QName& q) const
{
    return buildObject(q.getNamespaceURI(),q.getLocalPart(),q.getPrefix());
}

XMLObject* XMLObjectBuilder::buildFromElement(DOMElement* element, bool bindDocument) const
{
    auto_ptr<XMLObject> ret(
        buildObject(element->getNamespaceURI(),element->getLocalName(),element->getPrefix(),XMLHelper::getXSIType(element))
        );
    ret->unmarshall(element,bindDocument);
    return ret.release();
}

XMLObject* XMLObjectBuilder::buildFromDocument(DOMDocument* doc, bool bindDocument) const
{
    return buildFromElement(doc->getDocumentElement(),bindDocument);
}

XMLObject* XMLObjectBuilder::buildOneFromElement(xercesc::DOMElement* element, bool bindDocument)
{
    const XMLObjectBuilder* b=getBuilder(element);
    return b ? b->buildFromElement(element,bindDocument) : nullptr;
}

const XMLObjectBuilder* XMLObjectBuilder::getBuilder(const QName& key)
{
    map<QName,XMLObjectBuilder*>::const_iterator i=m_map.find(key);
    return (i==m_map.end()) ? nullptr : i->second;
}

const XMLObjectBuilder* XMLObjectBuilder::getBuilder(const DOMElement* domElement)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("getBuilder");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLObject.Builder");
 
    auto_ptr<QName> schemaType(XMLHelper::getXSIType(domElement));
    const XMLObjectBuilder* xmlObjectBuilder = schemaType.get() ? getBuilder(*(schemaType.get())) : nullptr;
    if (xmlObjectBuilder) {
        if (log.isDebugEnabled()) {
            log.debug("located XMLObjectBuilder for schema type: %s", schemaType->toString().c_str());
        }
        return xmlObjectBuilder;
    }
    
    auto_ptr<QName> elementName(XMLHelper::getNodeQName(domElement));
    xmlObjectBuilder = getBuilder(*(elementName.get()));
    if (xmlObjectBuilder) {
        if (log.isDebugEnabled()) {
            log.debug("located XMLObjectBuilder for element name: %s", elementName->toString().c_str());
        }
        return xmlObjectBuilder;
    }

    if (log.isDebugEnabled()) {
        log.debug("no XMLObjectBuilder registered for element (%s), returning default", elementName->toString().c_str());
    }
    return m_default;
}

const XMLObjectBuilder* XMLObjectBuilder::getDefaultBuilder()
{
    return m_default;
}

const map<QName,XMLObjectBuilder*>& XMLObjectBuilder::getBuilders()
{
    return m_map;
}

void XMLObjectBuilder::registerBuilder(const QName& builderKey, XMLObjectBuilder* builder)
{
    deregisterBuilder(builderKey);
    m_map[builderKey]=builder;
}

void XMLObjectBuilder::registerDefaultBuilder(XMLObjectBuilder* builder)
{
    deregisterDefaultBuilder();
    m_default=builder;
}

void XMLObjectBuilder::deregisterBuilder(const QName& builderKey)
{
    delete getBuilder(builderKey);
    m_map.erase(builderKey);
}

void XMLObjectBuilder::deregisterDefaultBuilder()
{
    delete m_default;
    m_default = nullptr;
}

void XMLObjectBuilder::destroyBuilders()
{
    for_each(m_map.begin(),m_map.end(),cleanup_pair<QName,XMLObjectBuilder>());
    m_map.clear();
    deregisterDefaultBuilder();
}

ConcreteXMLObjectBuilder::ConcreteXMLObjectBuilder()
{
}

ConcreteXMLObjectBuilder::~ConcreteXMLObjectBuilder()
{
}
