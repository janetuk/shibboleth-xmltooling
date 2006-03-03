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

#if !defined(__xmltooling_xmlobj_h__)
#define __xmltooling_xmlobj_h__

#include <set>
#include <list>
#include <xmltooling/QName.h>
#include <xmltooling/Namespace.h>

namespace xmltooling {

    /**
     * Object that represents an XML Element that has been unmarshalled into this C++ object.
     */
    class XMLTOOL_API XMLObject
    {
        MAKE_NONCOPYABLE(XMLObject);
    public:
        XMLObject() {}
        virtual ~XMLObject() {}
        
        /**
         * Creates a copy of the object, along with all of its children.
         * 
         * The new object tree will be completely distinct and independent of
         * the original in all respects.
         */
        virtual XMLObject* clone() const=0;
        
        /**
         * Gets the QName for this element.  This QName <strong>MUST</strong> 
         * contain the namespace URI, namespace prefix, and local element name.
         * 
         * @return constant reference to the QName for this object
         */
        virtual const QName& getElementQName() const=0;
        
        /**
         * Sets the namespace prefix for this element.
         * 
         * @param prefix the prefix for this element
         */
        virtual void setElementNamespacePrefix(const XMLCh* prefix)=0;
        
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
        virtual void addNamespace(const Namespace& ns)=0;
        
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
         * Sets the XML schema type of this element.  This translates to contents the xsi:type
         * attribute for the element.
         * 
         * @param type XML schema type of this element
         */
        virtual void setSchemaType(const QName* type)=0;
        
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
    };

};

#endif /* __xmltooling_xmlobj_h__ */
