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
#include <xmltooling/util/DateTime.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * An abstract implementation of XMLObject.
     * This is the primary concrete base class, and supplies basic namespace,
     * type, and parent handling. Most implementation classes should not
     * directly inherit from this class, but rather from the various mixins
     * that supply the rest of the XMLObject interface, as required.
     */
    class XMLTOOL_API AbstractXMLObject : public virtual XMLObject
    {
    public:
        virtual ~AbstractXMLObject() {
            delete m_typeQname;
            XMLString::release(&m_schemaLocation);
        }

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
         * A helper function for derived classes, for assignment of strings.
         *
         * This 'normalizes' newString, and then if it is different from oldString,
         * it invalidates the DOM, frees the old string, and returns the new.
         * If not different, it frees the new string and just returns the old value.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value
         * 
         * @return the value that should be assigned
         */
        XMLCh* prepareForAssignment(XMLCh* oldValue, const XMLCh* newValue);

        /**
         * A helper function for derived classes, for assignment of date/time data.
         *
         * It invalidates the DOM, frees the old object, and returns the new.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value
         * 
         * @return the value that should be assigned
         */
        DateTime* prepareForAssignment(DateTime* oldValue, const DateTime* newValue);

        /**
         * A helper function for derived classes, for assignment of date/time data.
         *
         * It invalidates the DOM, frees the old object, and returns the new.
         * 
         * @param oldValue - the current value
         * @param newValue - the epoch to assign as the new value
         * 
         * @return the value that should be assigned
         */
        DateTime* prepareForAssignment(DateTime* oldValue, time_t newValue);

        /**
         * A helper function for derived classes, for assignment of date/time data.
         *
         * It invalidates the DOM, frees the old object, and returns the new.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value in string form
         * 
         * @return the value that should be assigned
         */
        DateTime* prepareForAssignment(DateTime* oldValue, const XMLCh* newValue);

        /**
         * A helper function for derived classes, for assignment of QName data.
         *
         * It invalidates the DOM, frees the old object, and returns the new.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value
         * 
         * @return the value that should be assigned
         */
        QName* prepareForAssignment(QName* oldValue, const QName* newValue);

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
         * Set of namespaces associated with the object.
         */
        mutable std::set<Namespace> m_namespaces;

        /**
         * Logging object.
         */
        void* m_log;

        /**
         * Stores off xsi:schemaLocation attribute.
         */
        XMLCh* m_schemaLocation;

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
