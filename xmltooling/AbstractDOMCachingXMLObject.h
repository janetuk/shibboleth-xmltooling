/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * @file xmltooling/AbstractDOMCachingXMLObject.h
 * 
 * AbstractXMLObject mixin that implements DOM caching.
 */

#ifndef __xmltooling_abstractdomxmlobj_h__
#define __xmltooling_abstractdomxmlobj_h__

#include <xmltooling/AbstractXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements DOM caching.
     * Inherit from this class to implement standard DOM caching behavior.
     */
    class XMLTOOL_API AbstractDOMCachingXMLObject : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractDOMCachingXMLObject();

        // Virtual function overrides.
        xercesc::DOMElement* getDOM() const;
        void setDOM(xercesc::DOMElement* dom, bool bindDocument=false) const;
        void setDocument(xercesc::DOMDocument* doc) const;
        void releaseDOM() const;
        void releaseParentDOM(bool propagateRelease=true) const;
        void releaseChildrenDOM(bool propagateRelease=true) const;
        XMLObject* clone() const;
        void detach();

     protected:
        AbstractDOMCachingXMLObject();

        /** Copy constructor. */
        AbstractDOMCachingXMLObject(const AbstractDOMCachingXMLObject& src);

        /**
         * If a DOM representation exists, this clones it into a new document.
         * 
         * @param doc   the document to clone into, or nullptr, in which case a new document is created
         * @return  the cloned DOM
         */
        xercesc::DOMElement* cloneDOM(xercesc::DOMDocument* doc=nullptr) const;

    private:
        mutable xercesc::DOMElement* m_dom;
        mutable xercesc::DOMDocument* m_document;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abstractdomxmlobj_h__ */
