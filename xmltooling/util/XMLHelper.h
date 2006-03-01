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
 * @file XMLHelper.h
 * 
 * A helper class for working with W3C DOM objects. 
 */

#if !defined(__xmltooling_xmlhelper_h__)
#define __xmltooling_xmlhelper_h__

#include <xmltooling/QName.h>
#include <xercesc/dom/DOM.hpp>

using namespace xercesc;

namespace xmltooling {
    
    /**
     * A helper class for working with W3C DOM objects. 
     */
    class XMLTOOL_API XMLHelper
    {
    public:
        /**
         * Checks if the given element has an xsi:type defined for it
         * 
         * @param e the DOM element
         * @return true if there is a type, false if not
         */
        static bool hasXSIType(const DOMElement* e);

        /**
         * Gets the XSI type for a given element if it has one.
         * 
         * @param e the element
         * @return the type or null
         */
        static QName* getXSIType(const DOMElement* e);

        /**
         * Gets the ID attribute of a DOM element.
         * 
         * @param domElement the DOM element
         * @return the ID attribute or null if there isn't one
         */
        static DOMAttr* getIdAttribute(const DOMElement* domElement);

        /**
         * Gets the QName for the given DOM node.
         * 
         * @param domNode the DOM node
         * @return the QName for the element or null if the element was null
         */
        static QName* getNodeQName(const DOMNode* domNode);

        /**
         * Constructs a QName from an attributes value.
         * 
         * @param attribute the attribute with a QName value
         * @return a QName from an attributes value, or null if the given attribute is null
         */
        static QName* getAttributeValueAsQName(const DOMAttr* attribute);

        /**
         * Appends the child Element to the parent Element,
         * importing the child Element into the parent's Document if needed.
         * 
         * @param parentElement the parent Element
         * @param childElement the child Element
         * @return the child Element that was added (may be an imported copy)
         */
        static DOMElement* appendChildElement(DOMElement* parentElement, DOMElement* childElement);
        
        /**
         * Checks the qualified name of an element.
         * 
         * @param e     element to check
         * @param ns    namespace to compare with
         * @param local local name to compare with
         * @return  true iff the element's qualified name matches the other parameters
         */
        static bool isElementNamed(const DOMElement* e, const XMLCh* ns, const XMLCh* local) {
            return (e && XMLString::equals(ns,e->getNamespaceURI()) && XMLString::equals(local,e->getLocalName()));
        }

        /**
         * Serializes the DOM Element provided into a buffer using UTF-8 encoding and
         * the default XML serializer available. No manipulation or formatting is applied.
         * 
         * @param e     element to serialize
         * @param buf   buffer to serialize element into
         */
        static void serialize(const DOMElement* e, std::string& buf);
    };

};

#endif /* __xmltooling_xmlhelper_h__ */
