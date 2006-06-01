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
 * AbstractXMLObjectUnmarshaller.cpp
 * 
 * A thread-safe abstract unmarshaller.
 */

#include "internal.h"
#include "exceptions.h"
#include "XMLObjectBuilder.h"
#include "io/AbstractXMLObjectUnmarshaller.h"
#include "util/NDC.h"
#include "util/XMLConstants.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>
#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

#define XT_log (*static_cast<Category*>(m_log))

XMLObject* AbstractXMLObjectUnmarshaller::unmarshall(DOMElement* element, bool bindDocument)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshall");
#endif

    if (getDOM() || hasParent())
        throw UnmarshallingException("Object already contains data, it cannot be unmarshalled at this stage.");

    if (!XMLString::equals(element->getNamespaceURI(),getElementQName().getNamespaceURI()) ||
        !XMLString::equals(element->getLocalName(),getElementQName().getLocalPart())) {
        throw UnmarshallingException("Unrecognized element supplied to implementation for unmarshalling.");
    }

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(element->getNodeName());
        XT_log.debug("unmarshalling DOM element (%s)", dname.get());
    }

    if (element->hasAttributes()) {
        unmarshallAttributes(element);
    }

    unmarshallChildElements(element);

    setDOM(element,bindDocument);
    return this;
}

void AbstractXMLObjectUnmarshaller::unmarshallAttributes(const DOMElement* domElement)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshallAttributes");
#endif

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(domElement->getNodeName());
        XT_log.debug("unmarshalling attributes for DOM element (%s)", dname.get());
    }

    DOMNamedNodeMap* attributes = domElement->getAttributes();
    if (!attributes) {
        XT_log.debug("no attributes to unmarshall");
        return;
    }

    DOMNode* childNode;
    DOMAttr* attribute;
    for (XMLSize_t i=0; i<attributes->getLength(); i++) {
        childNode = attributes->item(i);

        // The child node should always be an attribute, but just in case
        if (childNode->getNodeType() != DOMNode::ATTRIBUTE_NODE) {
            XT_log.debug("encountered child node of type %d in attribute list, ignoring it", childNode->getNodeType());
            continue;
        }

        attribute = static_cast<DOMAttr*>(childNode);
        
        const XMLCh* nsuri=attribute->getNamespaceURI();
        if (XMLString::equals(nsuri,XMLConstants::XMLNS_NS)) {
            if (XMLString::equals(attribute->getLocalName(),XMLConstants::XMLNS_PREFIX)) {
                XT_log.debug("found default namespace declaration, adding it to the list of namespaces on the XMLObject");
                addNamespace(Namespace(attribute->getValue(), NULL, true));
                continue;
            }
            else {
                XT_log.debug("found namespace declaration, adding it to the list of namespaces on the XMLObject");
                addNamespace(Namespace(attribute->getValue(), attribute->getLocalName(), true));
                continue;
            }
        }
        else if (XMLString::equals(nsuri,XMLConstants::XSI_NS)) {
            static const XMLCh type[]= UNICODE_LITERAL_4(t,y,p,e);
            static const XMLCh schemaLocation[]= UNICODE_LITERAL_14(s,c,h,e,m,a,L,o,c,a,t,i,o,n);
            if (XMLString::equals(attribute->getLocalName(),type)) {
                XT_log.debug("skipping xsi:type declaration");
                continue;
            }
            else if (XMLString::equals(attribute->getLocalName(),schemaLocation)) {
                XT_log.debug("storing off xsi:schemaLocation attribute");
                if (m_schemaLocation)
                    XMLString::release(&m_schemaLocation);
                m_schemaLocation=XMLString::replicate(attribute->getValue());
                continue;
            }
        }
        else if (nsuri && !XMLString::equals(nsuri,XMLConstants::XML_NS)) {
            XT_log.debug("found namespace-qualified attribute, adding prefix to the list of namespaces on the XMLObject");
            addNamespace(Namespace(nsuri, attribute->getPrefix()));
        }

        XT_log.debug("processing generic attribute");
        processAttribute(attribute);
    }
}

void AbstractXMLObjectUnmarshaller::unmarshallChildElements(const DOMElement* domElement)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshallChildElements");
#endif

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(domElement->getNodeName());
        XT_log.debug("unmarshalling child elements of DOM element (%s)", dname.get());
    }

    DOMNodeList* childNodes = domElement->getChildNodes();
    DOMNode* childNode;
    if (!childNodes || childNodes->getLength()==0) {
        XT_log.debug("element had no children");
        return;
    }

    for (XMLSize_t i = 0; i < childNodes->getLength(); i++) {
        childNode = childNodes->item(i);
        if (childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            const XMLObjectBuilder* builder = XMLObjectBuilder::getBuilder(static_cast<DOMElement*>(childNode));
            if (!builder) {
                auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                XT_log.error("no default builder installed, found unknown child element (%s)", cname->toString().c_str());
                throw UnmarshallingException("Unmarshaller found unknown child element, but no default builder was found.");
            }

            if (XT_log.isDebugEnabled()) {
                auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                XT_log.debug("unmarshalling child element (%s)", cname->toString().c_str());
            }

            // Retain ownership of the unmarshalled child until it's processed by the parent.
            auto_ptr<XMLObject> childObject(builder->buildFromElement(static_cast<DOMElement*>(childNode)));
            processChildElement(childObject.get(), static_cast<DOMElement*>(childNode));
            childObject.release();
        }
        else if (childNode->getNodeType() == DOMNode::TEXT_NODE && !XMLString::isAllWhiteSpace(childNode->getNodeValue())) {
            XT_log.debug("processing element content");
            processElementContent(childNode->getNodeValue());
        }
    }
}

void AbstractXMLObjectUnmarshaller::processChildElement(XMLObject* child, const DOMElement* childRoot)
{
    throw UnmarshallingException("Invalid child element: $1",params(1,child->getElementQName().toString().c_str()));
}

void AbstractXMLObjectUnmarshaller::processAttribute(const DOMAttr* attribute)
{
    auto_ptr<QName> q(XMLHelper::getNodeQName(attribute));
    throw UnmarshallingException("Invalid attribute: $1",params(1,q->toString().c_str()));
}

void AbstractXMLObjectUnmarshaller::processElementContent(const XMLCh* elementContent)
{
    throw UnmarshallingException("Invalid text content in element."); 
}
