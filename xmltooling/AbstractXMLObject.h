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

#include <algorithm>
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
        virtual ~AbstractXMLObject() {
            delete m_typeQname;
            std::for_each(m_children.begin(), m_children.end(), cleanup<XMLObject>());
        }

        /**
         * @see XMLObject::getElementQName()
         */
        const QName& getElementQName() const {
            return m_elementQname;
        }

        /**
         * @see XMLObject::setElementNamespacePrefix()
         */
        void setElementNamespacePrefix(const XMLCh* prefix) {
            m_elementQname.setPrefix(prefix);
        }

        /**
         * @see XMLObject::getNamespaces()
         */
        const std::set<Namespace>& getNamespaces() const {
            return m_namespaces;
        }
    
        /**
         * @see XMLObject::addNamespace()
         */
        void addNamespace(const Namespace& ns) {
            if (ns.alwaysDeclare() || m_namespaces.find(ns)==m_namespaces.end()) {
                m_namespaces.insert(ns);
            }
        }
    
        /**
         * @see XMLObject::removeNamespace()
         */
        void removeNamespace(const Namespace& ns) {
            m_namespaces.erase(ns);
        }
        
        /**
         * @see XMLObject::getSchemaType()
         */
        const QName* getSchemaType() const {
            return m_typeQname;
        }
    
        /**
         * @see XMLObject::setSchemaType()
         */
        void setSchemaType(const QName* type) {
            delete m_typeQname;
            m_typeQname = NULL;
            if (type) {
                m_typeQname = new QName(*type);
                addNamespace(Namespace(type->getNamespaceURI(), type->getPrefix()));
            }
        }
    
        /**
         * @see XMLObject::hasParent()
         */
        bool hasParent() const {
            return m_parent != NULL;
        }
     
        /**
         * @see XMLObject::getParent()
         */
        XMLObject* getParent() const {
            return m_parent;
        }
    
        /**
         * @see XMLObject::setParent()
         */
        void setParent(XMLObject* parent) {
            m_parent = parent;
        }

        /**
         * @see XMLObject::hasChildren()
         */
        bool hasChildren() const {
            return !m_children.empty();
        }

        /**
         * @see XMLObject::getOrderedChildren()
         */
        const std::list<XMLObject*>& getOrderedChildren() const {
            return m_children;
        }

     protected:
        /**
         * Constructor
         * 
         * @param namespaceURI the namespace the element is in
         * @param elementLocalName the local name of the XML element this Object represents
         * @param namespacePrefix the namespace prefix to use
         */
        AbstractXMLObject(const XMLCh* namespaceURI=NULL, const XMLCh* elementLocalName=NULL, const XMLCh* namespacePrefix=NULL)
            : m_elementQname(namespaceURI,elementLocalName, namespacePrefix), m_typeQname(NULL), m_parent(NULL) {
            addNamespace(Namespace(namespaceURI, namespacePrefix));
        }

        /**
         * Underlying list of child objects.
         * Manages the lifetime of the children.
         */
        std::list<XMLObject*> m_children;

        /**
         * Set of namespaces associated with the object.
         */
        std::set<Namespace> m_namespaces;
        
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
