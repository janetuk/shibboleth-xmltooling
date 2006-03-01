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
 * @file UnknownElement.h
 * 
 * Basic implementations suitable for use as defaults for unrecognized content
 */

#if !defined(__xmltooling_unkelement_h__)
#define __xmltooling_unkelement_h__

#include "internal.h"
#include "AbstractDOMCachingXMLObject.h"
#include "XMLObjectBuilder.h"
#include "io/Marshaller.h"
#include "io/Unmarshaller.h"

#include <string>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Implementation class for unrecognized DOM elements.
     * Purpose is to wrap the DOM and do any necessary caching/reconstruction
     * when a DOM has to cross into a new document.
     */
    class XMLTOOL_DLLLOCAL UnknownElementImpl : public AbstractDOMCachingXMLObject
    {
    public:
        UnknownElementImpl() {}
        virtual ~UnknownElementImpl() {}

        /**
         * Overridden to ensure XML content of DOM isn't lost.
         * 
         * @see DOMCachingXMLObject::releaseDOM()
         */
        void releaseDOM();

        /**
          * @see XMLObject::clone()
          */
        XMLObject* clone() const;

        /**
         * @see XMLObject::hasChildren()
         */
        bool hasChildren() const {
            return false;
        }

        /**
         * @see XMLObject::getOrderedChildren()
         */
        size_t getOrderedChildren(std::vector<XMLObject*>& v) const {
            return 0;
        }

    protected:
        /**
         * When needed, we can serialize the DOM into XML form and preserve it here.
         */
        std::string m_xml;

    private:
        void serialize(std::string& s) const;
        friend class XMLTOOL_API UnknownElementMarshaller;
    };

    /**
     * Factory for UnknownElementImpl objects
     */
    class XMLTOOL_DLLLOCAL UnknownElementBuilder : public virtual XMLObjectBuilder
    {
    public:
        UnknownElementBuilder() {}
        virtual ~UnknownElementBuilder() {}
    
        /**
         * @see XMLObjectBuilder::buildObject()
         */
        XMLObject* buildObject() const {
            return new UnknownElementImpl();
        }
    };

    /**
     * Marshaller for UnknownElementImpl objects
     */
    class XMLTOOL_DLLLOCAL UnknownElementMarshaller : public virtual Marshaller
    {
    public:
        UnknownElementMarshaller() {}
        virtual ~UnknownElementMarshaller() {}
    
        /**
         * @see Marshaller::marshall(XMLObject*,DOMDocument*)
         */
        DOMElement* marshall(XMLObject* xmlObject, DOMDocument* document=NULL) const;

        /**
         * @see Marshaller::marshall(XMLObject*,DOMElement*)
         */
        DOMElement* marshall(XMLObject* xmlObject, DOMElement* parentElement) const;
        
    protected:
        void setDocumentElement(DOMDocument* document, DOMElement* element) const {
            DOMElement* documentRoot = document->getDocumentElement();
            if (documentRoot)
                document->replaceChild(documentRoot, element);
            else
                document->appendChild(element);
        }
    };

    /**
     * Marshaller for UnknownElementImpl objects
     */
    class XMLTOOL_DLLLOCAL UnknownElementUnmarshaller : public virtual Unmarshaller
    {
    public:
        UnknownElementUnmarshaller() {}
        virtual ~UnknownElementUnmarshaller() {}
    
        /**
         * @see Unmarshaller::unmarshall()
         */
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false) const;
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_unkelement_h__ */
