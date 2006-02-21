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
 * @file DOMCachingXMLObject.h
 * 
 * An XMLObject that can cached a DOM representation of itself. 
 */

#if !defined(__xmltooling_domxmlobj_h__)
#define __xmltooling_domxmlobj_h__

#include <xercesc/dom/DOM.hpp>
#include <xmltooling/XMLObject.h>

using namespace xercesc;

namespace xmltooling {

    /**
     * An XMLObject that can cached a DOM representation of itself.
     */
    class XMLTOOL_API DOMCachingXMLObject : public virtual XMLObject
    {
    public:
        DOMCachingXMLObject() {}
        virtual ~DOMCachingXMLObject() {}
        
        /**
         * Gets the DOM representation of this XMLObject, if one exists.
         * 
         * @return the DOM representation of this XMLObject
         */
        virtual const DOMElement* getDOM() const=0;
        
        /**
         * Sets the DOM representation of this XMLObject.
         * 
         * @param dom       DOM representation of this XMLObject
         * @param bindDocument  true if the object should take ownership of the associated Document
         */
        virtual void setDOM(DOMElement* dom, bool bindDocument=false)=0;
    
        /**
         * Assigns ownership of a DOM document to the XMLObject.
         * This binds the lifetime of the document to the lifetime of the object.
         * 
         * @param doc DOM document bound to this object 
         */
        virtual DOMDocument* setDocument(DOMDocument* doc)=0;

        /**
         * Releases the DOM representation of this XMLObject, if there is one.
         */
        virtual void releaseDOM()=0;
        
        /**
         * Releases the DOM representation of this XMLObject's parent.
         * 
         * @param propagateRelease true if all ancestors of this element should release their DOM
         */
        virtual void releaseParentDOM(bool propagateRelease=true)=0;
        
        /**
         * Releases the DOM representation of this XMLObject's children.
         * 
         * @param propagateRelease true if all descendants of this element should release their DOM
         */
        virtual void releaseChildrenDOM(bool propagateRelease=true)=0;
    };
    
};

#endif /* __xmltooling_domxmlobj_h__ */
