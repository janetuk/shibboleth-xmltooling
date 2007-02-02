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
 * @file AbstractAttributeExtensibleXMLObject.h
 * 
 * AbstractXMLObject mixin that implements AttributeExtensibleXMLObject
 */

#ifndef __xmltooling_absattrextxmlobj_h__
#define __xmltooling_absattrextxmlobj_h__

#include <map>
#include <xmltooling/AbstractXMLObject.h>
#include <xmltooling/AttributeExtensibleXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements AttributeExtensibleXMLObject.
     * Inherit from this class to add support for attribute wildcarding.
     */
    class XMLTOOL_API AbstractAttributeExtensibleXMLObject
        : public virtual AttributeExtensibleXMLObject, public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractAttributeExtensibleXMLObject();
        
        const XMLCh* getAttribute(const QName& qualifiedName) const {
            std::map<QName,XMLCh*>::const_iterator i=m_attributeMap.find(qualifiedName);
            return (i==m_attributeMap.end()) ? NULL : i->second;
        }
        
        void setAttribute(const QName& qualifiedName, const XMLCh* value, bool ID=false);
    
        const std::map<QName,XMLCh*>& getExtensionAttributes() const {
            return m_attributeMap;
        }
        
        const XMLCh* getXMLID() const {
            return (m_idAttribute == m_attributeMap.end()) ? NULL : m_idAttribute->second;
        }
    
     protected:
        AbstractAttributeExtensibleXMLObject() {
            m_idAttribute = m_attributeMap.end();
        }

        /** Copy constructor. */
        AbstractAttributeExtensibleXMLObject(const AbstractAttributeExtensibleXMLObject& src);

        /**
         * Assists in the unmarshalling of extension attributes.
         * 
         * @param attribute the DOM attribute node being unmarshalled
         */
        void unmarshallExtensionAttribute(const DOMAttr* attribute);

        /**
         * Assists in the marshalling of extension attributes.
         * 
         * @param domElement    the DOM element against which to marshall the attributes
         */
        void marshallExtensionAttributes(DOMElement* domElement) const;
    
    private:
        /** Map of arbitrary attributes. */
        std::map<QName,XMLCh*> m_attributeMap;
        
        /** Points to the last attribute designated as an XML ID. */
        std::map<QName,XMLCh*>::const_iterator m_idAttribute;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_absattrextxmlobj_h__ */
