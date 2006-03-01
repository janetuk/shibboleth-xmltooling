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
 * @file AbstractDOMCachingXMLObject.h
 * 
 * Extension of AbstractXMLObject that implements a DOMCachingXMLObject. 
 */

#if !defined(__xmltooling_abstractdomxmlobj_h__)
#define __xmltooling_abstractdomxmlobj_h__

#include <xmltooling/AbstractXMLObject.h>
#include <xmltooling/DOMCachingXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Extension of AbstractXMLObject that implements a DOMCachingXMLObject.
     */
    class XMLTOOL_API AbstractDOMCachingXMLObject : public virtual AbstractXMLObject, public virtual DOMCachingXMLObject
    {
    public:
        virtual ~AbstractDOMCachingXMLObject();
        
        /**
         * @see DOMCachingXMLObject::getDOM()
         */
        DOMElement* getDOM() const {
            return m_dom;
        }
        
        /**
         * @see DOMCachingXMLObject::setDOM()
         */
        void setDOM(DOMElement* dom, bool bindDocument=false);
        
        /**
         * @see DOMCachingXMLObject::setDocument()
         */
        DOMDocument* setDocument(DOMDocument* doc) {
            DOMDocument* ret=m_document;
            m_document=doc;
            return ret;
        }
    
        /**
         * @see DOMCachingXMLObject::releaseDOM()
         */
        virtual void releaseDOM();
        
        /**
         * @see DOMCachingXMLObject::releaseParentDOM()
         */
        virtual void releaseParentDOM(bool propagateRelease=true);
        
        /**
         * @see DOMCachingXMLObject::releaseChildrenDOM()
         */
        virtual void releaseChildrenDOM(bool propagateRelease=true);
    
        /**
         * A convenience method that is equal to calling releaseDOM() then releaseParentDOM(true).
         */
        void releaseThisandParentDOM() {
            if (m_dom) {
                releaseDOM();
                releaseParentDOM(true);
            }
        }
    
        /**
         * A convenience method that is equal to calling releaseChildrenDOM(true) then releaseDOM().
         */
        void releaseThisAndChildrenDOM() {
            if (m_dom) {
                releaseChildrenDOM(true);
                releaseDOM();
            }
        }
    
     protected:
        /**
         * A helper function for derived classes.
         * This 'normalizes' newString and then if it is different from oldString
         * invalidates the DOM. It returns the normalized value.
         * 
         * @param oldValue - the current value
         * @param newValue - the new value
         * 
         * @return the value that should be assigned
         */
        XMLCh* prepareForAssignment(const XMLCh* oldValue, const XMLCh* newValue) {
            XMLCh* newString = XMLString::replicate(newValue);
            XMLString::trim(newString);

            if (oldValue && !newValue || !oldValue && newValue || XMLString::compareString(oldValue,newValue))
                releaseThisandParentDOM();
    
            return newString;
        }
    
        /**
         * A helper function for derived classes, for assignment of (singleton) XML objects.
         * 
         * It is indifferent to whether either the old or the new version of the value is null. 
         * This method will do a safe compare of the objects and will also invalidate the DOM if appropriate
         * 
         * @param oldValue - current value
         * @param newValue - proposed new value
         * @return The value to assign to the saved Object.
         * 
         * @throws IllegalArgumentException if the child already has a parent.
         */
        XMLObject* prepareForAssignment(const XMLObject* oldValue, XMLObject* newValue);

        AbstractDOMCachingXMLObject() : m_dom(NULL), m_document(NULL) {}

        /**
         * Constructor
         * 
         * @param namespaceURI the namespace the element is in
         * @param elementLocalName the local name of the XML element this Object represents
         */
        AbstractDOMCachingXMLObject(const XMLCh* namespaceURI, const XMLCh* elementLocalName, const XMLCh* namespacePrefix)
            : AbstractXMLObject(namespaceURI,elementLocalName, namespacePrefix), m_dom(NULL), m_document(NULL) {}

    private:
        DOMElement* m_dom;
        DOMDocument* m_document;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abstractdomxmlobj_h__ */
