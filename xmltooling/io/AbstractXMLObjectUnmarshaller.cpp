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
#include "DOMCachingXMLObject.h"
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

AbstractXMLObjectUnmarshaller::AbstractXMLObjectUnmarshaller()
    : m_log(&Category::getInstance(XMLTOOLING_LOGCAT".Unmarshaller")) {}

XMLObject* AbstractXMLObjectUnmarshaller::unmarshall(DOMElement* element, bool bindDocument) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshall");
#endif

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(element->getNodeName());
        XT_log.debug("unmarshalling DOM element (%s)", dname.get());
    }

    auto_ptr<XMLObject> xmlObject(buildXMLObject(element));

    if (element->hasAttributes()) {
        unmarshallAttributes(element, *(xmlObject.get()));
    }

    unmarshallChildElements(element, *(xmlObject.get()));

    /* TODO: Signing
    if (xmlObject instanceof SignableXMLObject) {
        verifySignature(domElement, xmlObject);
    }
    */

    DOMCachingXMLObject* dc=dynamic_cast<DOMCachingXMLObject*>(xmlObject.get());
    if (dc)
        dc->setDOM(element,bindDocument);
    else if (bindDocument)
        throw UnmarshallingException("Unable to bind document to non-DOM caching XMLObject instance.");
        
    return xmlObject.release();
}

XMLObject* AbstractXMLObjectUnmarshaller::buildXMLObject(const DOMElement* domElement) const
{
    const XMLObjectBuilder* xmlObjectBuilder = XMLObjectBuilder::getBuilder(domElement);
    if (xmlObjectBuilder)
        return xmlObjectBuilder->buildObject();
    throw UnmarshallingException("Failed to locate XMLObjectBuilder for element.");
}

void AbstractXMLObjectUnmarshaller::unmarshallAttributes(const DOMElement* domElement, XMLObject& xmlObject) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshallAttributes");
#endif
    static const XMLCh type[]={chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};

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
                xmlObject.addNamespace(Namespace(attribute->getValue(), NULL, true));
                continue;
            }
            else {
                XT_log.debug("found namespace declaration, adding it to the list of namespaces on the XMLObject");
                xmlObject.addNamespace(Namespace(attribute->getValue(), attribute->getLocalName(), true));
                continue;
            }
        }
        else if (XMLString::equals(nsuri,XMLConstants::XSI_NS) && XMLString::equals(attribute->getLocalName(),type)) {
            XT_log.debug("found xsi:type declaration, setting the schema type of the XMLObject");
            auto_ptr<QName> xsitype(XMLHelper::getAttributeValueAsQName(attribute));
            xmlObject.setSchemaType(xsitype.get());
            continue;
        }
        else if (nsuri && !XMLString::equals(nsuri,XMLConstants::XML_NS)) {
            XT_log.debug("found namespace-qualified attribute, adding prefix to the list of namespaces on the XMLObject");
            xmlObject.addNamespace(Namespace(nsuri, attribute->getPrefix()));
        }

        XT_log.debug("processing generic attribute");
        processAttribute(xmlObject, attribute);
    }
}

void AbstractXMLObjectUnmarshaller::unmarshallChildElements(const DOMElement* domElement, XMLObject& xmlObject) const
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
    const Unmarshaller* unmarshaller;
    if (!childNodes || childNodes->getLength()==0) {
        XT_log.debug("element had no children");
        return;
    }

    XMLToolingConfig& config=XMLToolingConfig::getConfig();
    for (XMLSize_t i = 0; i < childNodes->getLength(); i++) {
        childNode = childNodes->item(i);
        if (childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
            unmarshaller = Unmarshaller::getUnmarshaller(static_cast<DOMElement*>(childNode));
            if (!unmarshaller) {
                auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                XT_log.error(
                    "no default unmarshaller installed, found unknown child element (%s)", cname->toString().c_str()
                    );
                throw UnmarshallingException("Unmarshaller found unknown child element, but no default unmarshaller was found.");
            }

            if (XT_log.isDebugEnabled()) {
                auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                XT_log.debug("unmarshalling child element (%s)", cname->toString().c_str());
            }

            // Retain ownership of the unmarshalled child until it's processed by the parent.
            auto_ptr<XMLObject> childObject(unmarshaller->unmarshall(static_cast<DOMElement*>(childNode)));
            processChildElement(xmlObject, childObject.get(), static_cast<DOMElement*>(childNode));
            childObject.release();
        }
        else if (childNode->getNodeType() == DOMNode::TEXT_NODE) {
            XT_log.debug("processing element content");
            processElementContent(xmlObject, childNode->getNodeValue());
        }
    }
}
