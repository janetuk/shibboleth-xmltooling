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
 * Extension of AbstractXMLObject that adds DOM caching methods
 */

#if !defined(__xmltooling_abstractdomxmlobj_h__)
#define __xmltooling_abstractdomxmlobj_h__

#include <xmltooling/AbstractXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Extension of AbstractXMLObject that adds DOM caching methods
     */
    class XMLTOOL_API AbstractDOMCachingXMLObject : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractDOMCachingXMLObject();
        
        DOMElement* getDOM() const {
            return m_dom;
        }
        
        void setDOM(DOMElement* dom, bool bindDocument=false) const;
        
        void setDocument(DOMDocument* doc) const {
            if (m_document)
                m_document->release();
            m_document=doc;
        }
    
        virtual void releaseDOM() const;
        
        virtual void releaseParentDOM(bool propagateRelease=true) const;
        
        virtual void releaseChildrenDOM(bool propagateRelease=true) const;
    
        XMLObject* clone() const;

     protected:
        AbstractDOMCachingXMLObject() : m_dom(NULL), m_document(NULL) {}

        /** Copy constructor. */
        AbstractDOMCachingXMLObject(const AbstractDOMCachingXMLObject& src)
            : AbstractXMLObject(src), m_dom(NULL), m_document(NULL) {}

        /**
         * If a DOM representation exists, this clones it into a new document.
         * 
         * @param doc   the document to clone into, or NULL, in which case a new document is created
         * @return  the cloned DOM
         */
        DOMElement* cloneDOM(DOMDocument* doc=NULL) const;

    private:
        mutable DOMElement* m_dom;
        mutable DOMDocument* m_document;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abstractdomxmlobj_h__ */
