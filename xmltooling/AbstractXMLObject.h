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
 * @file AbstractXMLObject.h
 * 
 * An abstract implementation of XMLObject.
 */

#if !defined(__xmltooling_abstractxmlobj_h__)
#define __xmltooling_abstractxmlobj_h__

#include <xmltooling/XMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * An abstract implementation of XMLObject.
     */
    class XMLTOOL_API AbstractXMLObject : public virtual XMLObject
    {
    public:
        virtual ~AbstractXMLObject();

        const QName& getElementQName() const {
            return m_elementQname;
        }

        const std::set<Namespace>& getNamespaces() const {
            return m_namespaces;
        }
    
        void addNamespace(const Namespace& ns) const {
            if (ns.alwaysDeclare() || m_namespaces.find(ns)==m_namespaces.end()) {
                m_namespaces.insert(ns);
            }
        }
    
        void removeNamespace(const Namespace& ns) {
            m_namespaces.erase(ns);
        }
        
        const QName* getSchemaType() const {
            return m_typeQname;
        }
    
        bool hasParent() const {
            return m_parent != NULL;
        }
     
        XMLObject* getParent() const {
            return m_parent;
        }
    
        void setParent(XMLObject* parent) {
            m_parent = parent;
        }

        bool hasChildren() const {
            return !m_children.empty();
        }

        const std::list<XMLObject*>& getOrderedChildren() const {
            return m_children;
        }

     protected:
        /**
         * Constructor
         * 
         * @param nsURI         the namespace of the element
         * @param localName     the local name of the XML element this Object represents
         * @param prefix        the namespace prefix to use
         * @param schemaType    the xsi:type to use
         */
        AbstractXMLObject(
            const XMLCh* nsURI=NULL, const XMLCh* localName=NULL, const XMLCh* prefix=NULL, const QName* schemaType=NULL
            );

        /** Copy constructor. */
        AbstractXMLObject(const AbstractXMLObject& src);
        
        /**
         * A helper function for derived classes.
         * This 'normalizes' newString, and then if it is different from oldString,
         * it invalidates the DOM, frees the old string, and return the new.
         * If not different, it frees the new string and just returns the old value.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value
         * 
         * @return the value that should be assigned
         */
        XMLCh* prepareForAssignment(XMLCh* oldValue, const XMLCh* newValue) {
            XMLCh* newString = XMLString::replicate(newValue);
            XMLString::trim(newString);
            if (!XMLString::equals(oldValue,newValue)) {
                releaseThisandParentDOM();
                XMLString::release(&oldValue);
                return newString;
            }
            XMLString::release(&newString);
            return oldValue;            
        }

        /**
         * A helper function for derived classes, for assignment of (singleton) XML objects.
         * 
         * It is indifferent to whether either the old or the new version of the value is null. 
         * This method will do a safe compare of the objects and will also invalidate the DOM if appropriate.
         * Note that since the new value (even if NULL) is always returned, it may be more efficient
         * to discard the return value and just assign independently if a dynamic cast would be involved.
         * 
         * @param oldValue - current value
         * @param newValue - proposed new value
         * @return the new value 
         * 
         * @throws XMLObjectException if the new child already has a parent.
         */
        XMLObject* prepareForAssignment(XMLObject* oldValue, XMLObject* newValue);

        /**
         * Underlying list of child objects.
         * Manages the lifetime of the children.
         */
        std::list<XMLObject*> m_children;

        /**
         * Set of namespaces associated with the object.
         */
        mutable std::set<Namespace> m_namespaces;

        /**
         * Logging object.
         */
        void* m_log;

    private:
        XMLObject* m_parent;
        QName m_elementQname;
        QName* m_typeQname;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abstractxmlobj_h__ */
