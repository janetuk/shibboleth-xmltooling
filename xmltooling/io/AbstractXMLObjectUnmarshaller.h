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
 * @file AbstractXMLObjectUnmarshaller.h
 * 
 * A thread-safe abstract unmarshaller.
 */

#if !defined(__xmltooling_xmlunmarshaller_h__)
#define __xmltooling_xmlunmarshaller_h__

#include <xmltooling/io/Unmarshaller.h>

namespace xmltooling {

    /**
     * A thread-safe abstract unmarshaller.
     */
    class XMLTOOL_API AbstractXMLObjectUnmarshaller : public virtual Unmarshaller
    {
    public:
        virtual ~AbstractXMLObjectUnmarshaller() {}

        /**
         * @see Unmarshaller::unmarshall()
         */
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false) const;
            
    protected:
        AbstractXMLObjectUnmarshaller();

        /**
         * Constructs the XMLObject that the given DOM Element will be unmarshalled into. If the DOM element has an XML
         * Schema type defined this method will attempt to retrieve an XMLObjectBuilder using the schema type. If no
         * schema type is present or no builder is registered for the schema type, the element's QName is used. Once
         * the builder is found the XMLObject is created by invoking XMLObjectBuilder::buildObject().
         * Extending classes may wish to override this logic if more than just schema type or element name
         * (e.g. element attributes or content) need to be used to determine how to create the XMLObject.
         * 
         * @param domElement the DOM Element the created XMLObject will represent
         * @return the empty XMLObject that DOM Element can be unmarshalled into
         * 
         * @throws UnmarshallingException thrown if there is now XMLObjectBuilder registered for the given DOM Element
         */
        virtual XMLObject* buildXMLObject(const DOMElement* domElement) const;
        
        /**
         * Unmarshalls the attributes from the given DOM Element into the given XMLObject. If the attribute is an XML
         * namespace declaration the namespace is added to the given element via XMLObject::addNamespace().
         * If it is a schema type (xsi:type) the schema type is added to the element via XMLObject::setSchemaType().
         * All other attributes are passed to the processAttribute hook.
         * 
         * @param domElement the DOM Element whose attributes will be unmarshalled
         * @param xmlObject the XMLObject that will recieve information from the DOM attribute
         * 
         * @throws UnmarshallingException thrown if there is a problem unmarshalling an attribute
         */
        virtual void unmarshallAttributes(const DOMElement* domElement, XMLObject& xmlObject) const;

        /**
         * Unmarshalls a given Element's children. For each child an unmarshaller is retrieved using
         * getUnmarshaller(). The unmarshaller is then used to unmarshall the child element and the
         * resulting XMLObject is passed to processChildElement() for further processing.
         * 
         * @param domElement the DOM Element whose children will be unmarshalled
         * @param xmlObject the parent object of the unmarshalled children
         * 
         * @throws UnmarshallingException thrown if an error occurs unmarshalling the child elements
         */
        virtual void unmarshallChildElements(const DOMElement* domElement, XMLObject& xmlObject) const;

        /**
         * Called after a child element has been unmarshalled so that it can be added to the parent XMLObject.
         * 
         * @param parent the parent XMLObject
         * @param child pointer to the child XMLObject
         * 
         * @throws UnmarshallingException thrown if there is a problem adding the child to the parent
         */
        virtual void processChildElement(XMLObject& parent, XMLObject* child) const=0;
    
        /**
         * Called after an attribute has been unmarshalled so that it can be added to the XMLObject.
         * 
         * @param xmlObject the XMLObject
         * @param attribute the attribute being unmarshalled
         * 
         * @throws UnmarshallingException thrown if there is a problem adding the attribute to the XMLObject
         */
        virtual void processAttribute(XMLObject& xmlObject, const DOMAttr* attribute) const=0;
    
        /**
         * Called if the element being unmarshalled contained textual content so that it can be added to the XMLObject.
         * 
         * @param xmlObject XMLObject the content will be given to
         * @param elementContent the Element's text content
         */
        virtual void processElementContent(XMLObject& xmlObject, const XMLCh* elementContent) const=0;

        void* m_log;
    };
    
};

#endif /* __xmltooling_xmlunmarshaller_h__ */
