/*
*  Copyright 2001-2007 Internet2
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
 * @file xmltooling/io/AbstractXMLObjectUnmarshaller.h
 * 
 * A mix-in to implement object unmarshalling.
 */

#ifndef __xmltooling_xmlunmarshaller_h__
#define __xmltooling_xmlunmarshaller_h__

#include <xmltooling/AbstractDOMCachingXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A mix-in to implement object unmarshalling.
     */
    class XMLTOOL_API AbstractXMLObjectUnmarshaller : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractXMLObjectUnmarshaller() {}

        XMLObject* unmarshall(xercesc::DOMElement* element, bool bindDocument=false);
            
    protected:
        AbstractXMLObjectUnmarshaller() {}

        /**
         * Unmarshalls the attributes from the given DOM Element into the XMLObject. If the attribute
         * is an XML namespace declaration the namespace is added via XMLObject::addNamespace().
         * If it is a schema type (xsi:type) the schema type is added via XMLObject::setSchemaType().
         * All other attributes are passed to the processAttribute hook.
         * 
         * @param domElement the DOM Element whose attributes will be unmarshalled
         * 
         * @throws UnmarshallingException thrown if there is a problem unmarshalling an attribute
         */
        virtual void unmarshallAttributes(const xercesc::DOMElement* domElement);

        /**
         * Unmarshalls a given Element's child nodes. The resulting XMLObject children and content
         * are passed to processChildElement() or processText() for further processing.
         * 
         * @param domElement the DOM Element whose children will be unmarshalled
         * 
         * @throws UnmarshallingException thrown if an error occurs unmarshalling the child elements
         */
        virtual void unmarshallContent(const xercesc::DOMElement* domElement);

        /**
         * Called after a child element has been unmarshalled so that it can be added to the parent XMLObject.
         * 
         * @param child     pointer to the child XMLObject
         * @param childRoot root element of the child (must not be stored, just a hint)
         * 
         * @throws UnmarshallingException thrown if there is a problem adding the child to the parent
         */
        virtual void processChildElement(XMLObject* child, const xercesc::DOMElement* childRoot);
    
        /**
         * Called after an attribute has been unmarshalled so that it can be added to the XMLObject.
         * 
         * @param attribute the attribute being unmarshalled
         * 
         * @throws UnmarshallingException thrown if there is a problem adding the attribute to the XMLObject
         */
        virtual void processAttribute(const xercesc::DOMAttr* attribute);
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlunmarshaller_h__ */
