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
 * @file AbstractXMLObjectMarshaller.h
 * 
 * A thread-safe abstract marshaller.
 */

#if !defined(__xmltooling_xmlmarshaller_h__)
#define __xmltooling_xmlmarshaller_h__

#include <xmltooling/AbstractDOMCachingXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A thread-safe abstract marshaller.
     */
    class XMLTOOL_API AbstractXMLObjectMarshaller : public virtual AbstractDOMCachingXMLObject
    {
    public:
        virtual ~AbstractXMLObjectMarshaller() {}

        /**
         * @see XMLObject::marshall(DOMDocument*,const MarshallingContext*)
         */
        DOMElement* marshall(DOMDocument* document=NULL, MarshallingContext* ctx=NULL) const;

        /**
         * @see XMLObject::marshall(DOMElement*,const MarshallingContext*)
         */
        DOMElement* marshall(DOMElement* parentElement, MarshallingContext* ctx=NULL) const;
        
    protected:
        AbstractXMLObjectMarshaller() {}

        /**
         * Sets the given element as the Document Element of the given Document.
         * If the document already has a Document Element it is replaced by the given element.
         * 
         * @param document the document
         * @param element the Element that will serve as the Document Element
         */
        void setDocumentElement(DOMDocument* document, DOMElement* element) const {
            DOMElement* documentRoot = document->getDocumentElement();
            if (documentRoot)
                document->replaceChild(documentRoot, element);
            else
                document->appendChild(element);
        }
    
        /**
         * Marshalls the XMLObject into the given DOM Element.
         * The DOM Element must be within a DOM tree rooted in the owning Document.
         * 
         * @param targetElement the Element into which the XMLObject is marshalled into
         * @param ctx           optional marshalling context
         * 
         * @throws MarshallingException thrown if there is a problem marshalling the object
         * @throws SignatureException thrown if a problem occurs during signature creation 
         */
        void marshallInto(DOMElement* targetElement, MarshallingContext* ctx) const;
    
        /**
         * Creates an xsi:type attribute, corresponding to the given type of the XMLObject, on the DOM element.
         * 
         * @param domElement the DOM element
         * 
         * @throws MarshallingException thrown if the type on the XMLObject is doesn't contain
         * a local name, prefix, and namespace URI
         */
        void marshallElementType(DOMElement* domElement) const;

        /**
         * Creates the xmlns attributes for any namespaces set on the XMLObject.
         * 
         * @param domElement the DOM element the namespaces will be added to
         */
        void marshallNamespaces(DOMElement* domElement) const;
    
        /**
         * Marshalls the child elements of the XMLObject.
         * 
         * @param domElement the DOM element that will recieved the marshalled children
         * 
         * @throws MarshallingException thrown if there is a problem marshalling a child element
         */
        void marshallChildElements(DOMElement* domElement) const;

        /**
         * Marshalls the attributes from the XMLObject into the given DOM element.
         * 
         * @param domElement the DOM Element into which attributes will be marshalled
         * 
         * @throws MarshallingException thrown if there is a problem marshalling an attribute
         */
        virtual void marshallAttributes(DOMElement* domElement) const=0;

        /**
         * Marshalls data from the XMLObject into content of the DOM Element.
         * 
         * @param domElement the DOM element recieving the content
         */
        virtual void marshallElementContent(DOMElement* domElement) const=0;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlmarshaller_h__ */
