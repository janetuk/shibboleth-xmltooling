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

#include <xmltooling/io/Marshaller.h>

namespace xmltooling {

    /**
     * A thread-safe abstract marshaller.
     */
    class XMLTOOL_API AbstractXMLObjectMarshaller : public virtual Marshaller
    {
    public:
        virtual ~AbstractXMLObjectMarshaller() {}

        /**
         * @see Marshaller::marshall()
         */
        DOMElement* marshall(XMLObject* xmlObject, DOMDocument* document=NULL) const;
    
        
    protected:
        /**
         * Constructor.
         * 
         * @param targetNamespaceURI the namespace URI of either the schema type QName or element QName of the elements this
         *            marshaller operates on
         * @param targetLocalName the local name of either the schema type QName or element QName of the elements this
         *            marshaller operates on
         */
        AbstractXMLObjectMarshaller(const XMLCh* targetNamespaceURI, const XMLCh* targetLocalName);

        /**
         * Creates an xsi:type attribute, corresponding to the given type of the XMLObject, on the DOM element.
         * 
         * @param xmlObject the XMLObject
         * @param domElement the DOM element
         * 
         * @throws MarshallingException thrown if the type on the XMLObject is doesn't contain
         * a local name, prefix, and namespace URI
         */
        void marshallElementType(XMLObject* xmlObject, DOMElement* domElement) const;

        /**
         * Creates the xmlns attributes for any namespaces set on the given XMLObject.
         * 
         * @param xmlObject the XMLObject
         * @param domElement the DOM element the namespaces will be added to
         */
        void marshallNamespaces(const XMLObject* xmlObject, DOMElement* domElement) const;
    
        /**
         * Marshalls the child elements of the given XMLObject.
         * 
         * @param xmlObject the XMLObject whose children will be marshalled
         * @param domElement the DOM element that will recieved the marshalled children
         * 
         * @throws MarshallingException thrown if there is a problem marshalling a child element
         */
        void marshallChildElements(const XMLObject* xmlObject, DOMElement* domElement) const;

        /**
         * Marshalls the attributes from the given XMLObject into the given DOM element.
         * The XMLObject passed to this method is guaranteed to be of the target name
         * specified during this marshaller's construction.
         * 
         * @param xmlObject the XMLObject being marshalled
         * @param domElement the DOM Element into which attributes will be marshalled
         * 
         * @throws UnmarshallingException thrown if there is a problem unmarshalling an attribute
         */
        virtual void marshallAttributes(const XMLObject* xmlObject, DOMElement* domElement) const=0;

        /**
         * Marshalls data from the XMLObject into content of the DOM Element.
         * 
         * @param xmlObject the XMLObject
         * @param domElement the DOM element recieving the content
         */
        virtual void marshallElementContent(const XMLObject* xmlObject, DOMElement* domElement) const=0;

        void* m_log;
    private:
        QName m_targetQName;
    };
    
};

#endif /* __xmltooling_xmlmarshaller_h__ */
