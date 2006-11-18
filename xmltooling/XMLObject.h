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
 * @file XMLObject.h
 * 
 * Abstract interface to objects that can be manipulated in and out of XML form. 
 */

#ifndef __xmltooling_xmlobj_h__
#define __xmltooling_xmlobj_h__

#include <xmltooling/QName.h>
#include <xmltooling/Namespace.h>

#include <set>
#include <list>
#include <vector>
#include <xercesc/dom/DOM.hpp>

using namespace xercesc;

#ifndef XMLTOOLING_NO_XMLSEC
namespace xmlsignature {
    class XMLTOOL_API Signature;
};
#endif

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Object that represents an XML Element that has been unmarshalled into this C++ object.
     */
    class XMLTOOL_API XMLObject
    {
    public:
        virtual ~XMLObject() {}
        
        /**
         * Creates a copy of the object, along with all of its children.
         * 
         * The new object tree will be completely distinct and independent of
         * the original in all respects.
         */
        virtual XMLObject* clone() const=0;
        
        /**
         * Specialized function for detaching a child object from its parent
         * <strong>while disposing of the parent</strong>.
         *
         * This is not a generic way of detaching any child object, but only of
         * pruning a single child from the root of an XMLObject tree. If the
         * detached XMLObject's parent is itself a child, an exception will be
         * thrown. It's mainly useful for turning a child into the new root of
         * the tree without having to clone the child.
         */
        virtual void detach()=0;

        /**
         * Gets the QName for this element.  This QName <strong>MUST</strong> 
         * contain the namespace URI, namespace prefix, and local element name.
         * 
         * @return constant reference to the QName for this object
         */
        virtual const QName& getElementQName() const=0;
        
        /**
         * Gets the namespaces that are scoped to this element.
         * 
         * The caller MUST NOT modify the set returned, but may use any
         * non-modifying operations or algorithms on it. Iterators will
         * remain valid unless the set member referenced is removed using
         * the removeNamespace method.
         * 
         * @return the namespaces that are scoped to this element
         */
        virtual const std::set<Namespace>& getNamespaces() const=0;
        
        /**
         * Adds a namespace to the ones already scoped to this element
         * 
         * @param ns the namespace to add
         */
        virtual void addNamespace(const Namespace& ns) const=0;
        
        /**
         * Removes a namespace from this element
         * 
         * @param ns the namespace to remove
         */
        virtual void removeNamespace(const Namespace& ns)=0;
        
        /**
         * Gets the XML schema type of this element.  This translates to contents the xsi:type
         * attribute for the element.
         * 
         * @return XML schema type of this element
         */
        virtual const QName* getSchemaType() const=0;
        
        /**
         * Gets the value of the ID attribute set on this object, if any.
         * 
         * @return an ID value or NULL 
         */
        virtual const XMLCh* getXMLID() const=0;
        
        /**
         * Checks to see if this object has a parent.
         * 
         * @return true if the object has a parent, false if not
         */
        virtual bool hasParent() const=0;
        
        /**
         * Gets the parent of this element or null if there is no parent.
         * 
         * @return the parent of this element or null
         */
        virtual XMLObject* getParent() const=0;
        
        /**
         * Sets the parent of this element.
         * 
         * @param parent the parent of this element
         */
        virtual void setParent(XMLObject* parent)=0;
        
        /**
         * Checks if this XMLObject has children.
         * 
         * @return true if this XMLObject has children, false if not
         */
        virtual bool hasChildren() const=0;
        
        /**
         * Returns an unmodifiable list of child objects in the order that they
         * should appear in the serialized representation.
         * 
         * The validity of the returned list is not maintained if any non-const
         * operations are performed on the parent object. 
         * 
         * @return the list of children
         */
        virtual const std::list<XMLObject*>& getOrderedChildren() const=0;

        /**
         * Used by a child's detach method to isolate the child from
         * this parent object in preparation for destroying the parent
         * (this object).
         * 
         * @param child the child object to remove
         */
        virtual void removeChild(XMLObject* child)=0;

        /**
         * Returns the text content at the specified position relative to
         * any child elements. A zero represents leading text, 1 comes after
         * the first child, and so forth.
         *
         * @param position  the relative child element position of the text  
         * @return the designated text value
         */
        virtual const XMLCh* getTextContent(unsigned int position=0) const=0;

        /**
         * Sets (or clears) text content relative to a child element's position. 
         * 
         * @param value         value to set, or NULL to clear
         * @param position      position relative to child element 
         */
        virtual void setTextContent(const XMLCh* value, unsigned int position=0)=0;

        /**
         * Gets the DOM representation of this XMLObject, if one exists.
         * 
         * @return the DOM representation of this XMLObject
         */
        virtual DOMElement* getDOM() const=0;
        
        /**
         * Sets the DOM representation of this XMLObject.
         * 
         * @param dom       DOM representation of this XMLObject
         * @param bindDocument  true if the object should take ownership of the associated Document
         */
        virtual void setDOM(DOMElement* dom, bool bindDocument=false) const=0;
    
        /**
         * Assigns ownership of a DOM document to the XMLObject.
         * This binds the lifetime of the document to the lifetime of the object.
         * 
         * @param doc DOM document bound to this object 
         */
        virtual void setDocument(DOMDocument* doc) const=0;

        /**
         * Releases the DOM representation of this XMLObject, if there is one.
         */
        virtual void releaseDOM() const=0;
        
        /**
         * Releases the DOM representation of this XMLObject's parent.
         * 
         * @param propagateRelease true if all ancestors of this element should release their DOM
         */
        virtual void releaseParentDOM(bool propagateRelease=true) const=0;
        
        /**
         * Releases the DOM representation of this XMLObject's children.
         * 
         * @param propagateRelease true if all descendants of this element should release their DOM
         */
        virtual void releaseChildrenDOM(bool propagateRelease=true) const=0;

        /**
         * A convenience method that is equal to calling releaseDOM() then releaseParentDOM(true).
         */
        void releaseThisandParentDOM() const {
            if (getDOM()) {
                releaseDOM();
                releaseParentDOM(true);
            }
        }
    
        /**
         * A convenience method that is equal to calling releaseChildrenDOM(true) then releaseDOM().
         */
        void releaseThisAndChildrenDOM() const {
            if (getDOM()) {
                releaseChildrenDOM(true);
                releaseDOM();
            }
        }

        /**
         * Marshalls the XMLObject, and its children, into a DOM element.
         * If a document is supplied, then it will be used to create the resulting elements.
         * If the document does not have a Document Element set, then the resulting
         * element will be set as the Document Element. If no document is supplied, then
         * a new document will be created and bound to the lifetime of the root object being
         * marshalled, unless an existing DOM can be reused without creating a new document. 
         * 
         * @param document  the DOM document the marshalled element will be placed in, or NULL
         * @param sigs      ordered array of signatures to create after marshalling is complete
         * @return the DOM element representing this XMLObject
         * 
         * @throws MarshallingException thrown if there is a problem marshalling the given object
         * @throws SignatureException thrown if a problem occurs during signature creation 
         */
        virtual DOMElement* marshall(
            DOMDocument* document=NULL
#ifndef XMLTOOLING_NO_XMLSEC
            ,const std::vector<xmlsignature::Signature*>* sigs=NULL
#endif
            ) const=0;
        
        /**
         * Marshalls the XMLObject and appends it as a child of the given parent element.
         * 
         * <strong>NOTE:</strong> The given Element must be within a DOM tree rooted in 
         * the Document owning the given Element.
         * 
         * @param parentElement the parent element to append the resulting DOM tree
         * @param sigs          ordered array of signatures to create after marshalling is complete
         * @return the marshalled element tree

         * @throws MarshallingException thrown if the given XMLObject can not be marshalled.
         * @throws SignatureException thrown if a problem occurs during signature creation 
         */
        virtual DOMElement* marshall(
            DOMElement* parentElement
#ifndef XMLTOOLING_NO_XMLSEC
            ,const std::vector<xmlsignature::Signature*>* sigs=NULL
#endif
            ) const=0;

        /**
         * Unmarshalls the given W3C DOM element into the XMLObject.
         * The root of a given XML construct should be unmarshalled with the bindDocument parameter
         * set to true.
         * 
         * @param element       the DOM element to unmarshall
         * @param bindDocument  true iff the resulting XMLObject should take ownership of the DOM's Document 
         * 
         * @return the unmarshalled XMLObject
         * 
         * @throws UnmarshallingException thrown if an error occurs unmarshalling the DOM element into the XMLObject
         */
        virtual XMLObject* unmarshall(DOMElement* element, bool bindDocument=false)=0;

    protected:
        XMLObject() {}
    private:
        XMLObject& operator=(const XMLObject& src);
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlobj_h__ */
