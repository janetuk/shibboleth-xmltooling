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

AbstractXMLObjectUnmarshaller::AbstractXMLObjectUnmarshaller(const XMLCh* targetNamespaceURI, const XMLCh* targetLocalName)
        : m_targetQName(targetNamespaceURI, targetLocalName),
        m_log(&Category::getInstance(XMLTOOLING_LOGCAT".Unmarshaller")) {
    if (!targetLocalName || !*targetLocalName)
        throw UnmarshallingException("targetLocalName cannot be null or empty");
}

XMLObject* AbstractXMLObjectUnmarshaller::unmarshall(DOMElement* element, bool bindDocument) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshall");
#endif

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(element->getLocalName());
        XT_log.debug("unmarshalling DOM element %s", dname.get());
    }

#ifdef _DEBUG
    checkElementIsTarget(element);
#endif

    XMLObject* xmlObject = buildXMLObject(element);

    if (element->hasAttributes()) {
        unmarshallAttributes(element, xmlObject);
    }

    if (element->getTextContent()) {
        processElementContent(xmlObject, element->getTextContent());
    }

    unmarshallChildElements(element, xmlObject);

    /* TODO: Signing
    if (xmlObject instanceof SignableXMLObject) {
        verifySignature(domElement, xmlObject);
    }
    */

    DOMCachingXMLObject* dc=dynamic_cast<DOMCachingXMLObject*>(xmlObject);
    if (dc)
        dc->setDOM(element,bindDocument);
        
    return xmlObject;
}

void AbstractXMLObjectUnmarshaller::checkElementIsTarget(const DOMElement* domElement) const
{
    auto_ptr<QName> elementName(XMLHelper::getNodeQName(domElement));

    XT_log.debug("checking that root element meets target criteria");

    auto_ptr<QName> type(XMLHelper::getXSIType(domElement));

    if (type.get() && m_targetQName==*(type.get())) {
        XT_log.debug("schema type of element matches target");
        return;
    }
    else {
        if (m_targetQName==*(elementName.get())) {
            XT_log.debug("element name matches target");
            return;
        }
        else {
            XT_log.errorStream() << "unmarshaller for (" << m_targetQName.toString()
                << ") passed (" << elementName->toString() << ")" << CategoryStream::ENDLINE;
            throw UnmarshallingException("Incorrect element type passed to unmarshaller.");
        }
    }
}

XMLObject* AbstractXMLObjectUnmarshaller::buildXMLObject(const DOMElement* domElement) const
{
    const XMLObjectBuilder* xmlObjectBuilder = XMLObjectBuilder::getBuilder(domElement);
    if (xmlObjectBuilder)
        return xmlObjectBuilder->buildObject();
    throw UnmarshallingException("Failed to locate XMLObjectBuilder for element.");
}

void AbstractXMLObjectUnmarshaller::unmarshallAttributes(const DOMElement* domElement, XMLObject* xmlObject) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshallAttributes");
#endif
    static const XMLCh type[]={chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(domElement->getLocalName());
        XT_log.debug("unmarshalling attributes for DOM element %s", dname.get());
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
        if (!XMLString::compareString(nsuri,XMLConstants::XMLNS_NS)) {
            XT_log.debug("found namespace declaration, adding it to the list of namespaces on the XMLObject");
            xmlObject->addNamespace(Namespace(attribute->getValue(), attribute->getLocalName()));
            continue;
        }
        else if (!XMLString::compareString(nsuri,XMLConstants::XSI_NS) &&
                    !XMLString::compareString(attribute->getLocalName(),type)) {
            XT_log.debug("found xsi:type declaration, setting the schema type of the XMLObject");
            auto_ptr<QName> xsitype(XMLHelper::getAttributeValueAsQName(attribute));
            xmlObject->setSchemaType(xsitype.get());
            continue;
        }

        XT_log.debug("processing generic attribute");
        processAttribute(xmlObject, attribute);
    }
}

void AbstractXMLObjectUnmarshaller::unmarshallChildElements(const DOMElement* domElement, XMLObject* xmlObject) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("unmarshallChildElements");
#endif

    if (XT_log.isDebugEnabled()) {
        auto_ptr_char dname(domElement->getLocalName());
        XT_log.debug("unmarshalling child elements of DOM element %s", dname.get());
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
                if (config.ignoreUnknownElements) {
                    unmarshaller=Unmarshaller::getDefaultUnmarshaller();
                    if (!unmarshaller) {
                        auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                        XT_log.error("no default unmarshaller installed, detected unknown child element %s", cname->toString().c_str());
                        throw UnmarshallingException("Unmarshaller detected unknown child element, but no default unmarshaller was found.");
                    }
                    else {
                        XT_log.debug("using default unmarshaller");
                    }
                }
                else {
                    auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                    XT_log.error("detected unknown child element %s", cname->toString().c_str());
                    throw UnknownElementException("Unmarshaller detected unknown child element.");
                }
            }

            if (XT_log.isDebugEnabled()) {
                auto_ptr<QName> cname(XMLHelper::getNodeQName(childNode));
                XT_log.debug("unmarshalling child element %s", cname->toString().c_str());
            }
            processChildElement(xmlObject, unmarshaller->unmarshall(static_cast<DOMElement*>(childNode)));
        }
    }
}
