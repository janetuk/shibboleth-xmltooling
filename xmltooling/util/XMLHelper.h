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

#ifndef __xmltooling_xmlhelper_h__
#define __xmltooling_xmlhelper_h__

#include <xmltooling/XMLObject.h>
#include <xercesc/dom/DOM.hpp>

#include <iostream>

using namespace xercesc;

namespace xmltooling {
    
    /**
     * RAII wrapper for Xerces resources.
     */
    template<class T> class XercesJanitor
    {
        MAKE_NONCOPYABLE(XercesJanitor);
        T* m_held;
    public:
        XercesJanitor(T* resource) : m_held(resource) {}
        
        ~XercesJanitor() {
            if (m_held)
                m_held->release();
        }
        
        /**
         * Returns resource held by this object and releases it to the caller.
         * 
         * @return  the resource held or NULL
         */
        T* release() {
            T* ret=m_held;
            m_held=NULL;
            return ret;
        }
    };
    
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
         * Attempts to locate an XMLObject from this point downward in the tree whose
         * XML ID matches the supplied value.
         * 
         * @param tree  root of tree to search
         * @param id    ID value to locate
         * @return XMLObject in the tree with a matching ID value, or NULL
         */
        static const XMLObject* getXMLObjectById(const XMLObject& tree, const XMLCh* id);
        

        /**
         * Gets the QName for the given DOM node.
         * 
         * @param domNode the DOM node
         * @return the QName for the element or null if the element was null
         */
        static QName* getNodeQName(const DOMNode* domNode);

        /**
         * Constructs a QName from an attribute's value.
         * 
         * @param attribute the attribute with a QName value
         * @return a QName from an attribute's value, or null if the given attribute is null
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
         * Checks the qualified name of a node.
         * 
         * @param n     node to check
         * @param ns    namespace to compare with
         * @param local local name to compare with
         * @return  true iff the node's qualified name matches the other parameters
         */
        static bool isNodeNamed(const DOMNode* n, const XMLCh* ns, const XMLCh* local) {
            return (n && XMLString::equals(local,n->getLocalName()) && XMLString::equals(ns,n->getNamespaceURI()));
        }

        /**
         * Returns the first matching child element of the node if any.
         * 
         * @param n         node to check
         * @param localName local name to compare with or NULL for any match
         * @return  the first matching child node of type Element, or NULL
         */
        static DOMElement* getFirstChildElement(const DOMNode* n, const XMLCh* localName=NULL);
        
        /**
         * Returns the last matching child element of the node if any.
         * 
         * @param n     node to check
         * @param localName local name to compare with or NULL for any match
         * @return  the last matching child node of type Element, or NULL
         */
        static DOMElement* getLastChildElement(const DOMNode* n, const XMLCh* localName=NULL);
        
        /**
         * Returns the next matching sibling element of the node if any.
         * 
         * @param n     node to check
         * @param localName local name to compare with or NULL for any match
         * @return  the next matching sibling node of type Element, or NULL
         */
        static DOMElement* getNextSiblingElement(const DOMNode* n, const XMLCh* localName=NULL);
        
        /**
         * Returns the previous matching sibling element of the node if any.
         * 
         * @param n     node to check
         * @param localName local name to compare with or NULL for any match
         * @return  the previous matching sibling node of type Element, or NULL
         */
        static DOMElement* getPreviousSiblingElement(const DOMNode* n, const XMLCh* localName=NULL);
        
        /**
         * Returns the first matching child element of the node if any.
         * 
         * @param n         node to check
         * @param ns        namespace to compare with
         * @param localName local name to compare with
         * @return  the first matching child node of type Element, or NULL
         */
        static DOMElement* getFirstChildElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName);
        
        /**
         * Returns the last matching child element of the node if any.
         * 
         * @param n         node to check
         * @param ns        namespace to compare with
         * @param localName local name to compare with
         * @return  the last matching child node of type Element, or NULL
         */
        static DOMElement* getLastChildElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName);
        
        /**
         * Returns the next matching sibling element of the node if any.
         * 
         * @param n         node to check
         * @param ns        namespace to compare with
         * @param localName local name to compare with
         * @return  the next matching sibling node of type Element, or NULL
         */
        static DOMElement* getNextSiblingElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName);
        
        /**
         * Returns the previous matching sibling element of the node if any.
         * 
         * @param n         node to check
         * @param ns        namespace to compare with
         * @param localName local name to compare with
         * @return  the previous matching sibling node of type Element, or NULL
         */
        static DOMElement* getPreviousSiblingElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName);

        /**
         * Returns the content of the first Text node found in the element, if any.
         * This is roughly similar to the DOM getTextContent function, but only
         * examples the immediate children of the element.
         *
         * @param e     element to examine
         * @return the content of the first Text node found, or NULL
         */
        static const XMLCh* getTextContent(const DOMElement* e);

        /**
         * Serializes the DOM node provided into a buffer using UTF-8 encoding and
         * the default XML serializer available. No manipulation or formatting is applied.
         * 
         * @param n     node to serialize
         * @param buf   buffer to serialize element into
         */
        static void serialize(const DOMNode* n, std::string& buf);

        /**
         * Serializes the DOM node provided to a stream using UTF-8 encoding and
         * the default XML serializer available. No manipulation or formatting is applied.
         * 
         * @param n     node to serialize
         * @param out   stream to serialize element into
         */
        static void serialize(const DOMNode* n, std::ostream& out);
    };

    /**
     * Serializes the DOM node provided to a stream using UTF-8 encoding and
     * the default XML serializer available. No manipulation or formatting is applied.
     * 
     * @param n      node to serialize
     * @param ostr   stream to serialize element into
     * @return reference to output stream
     */
    extern XMLTOOL_API std::ostream& operator<<(std::ostream& ostr, const DOMNode& n);

    /**
     * Marshalls and serializes the XMLObject provided to a stream using UTF-8 encoding and
     * the default XML serializer available. No manipulation or formatting is applied.
     * 
     * <p>The marshaller operation takes no parameters.
     * 
     * @param obj    object to serialize
     * @param ostr   stream to serialize object into
     * @return reference to output stream
     */
    extern XMLTOOL_API std::ostream& operator<<(std::ostream& ostr, const XMLObject& obj);
};

#endif /* __xmltooling_xmlhelper_h__ */
