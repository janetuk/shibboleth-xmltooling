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
    class XMLTOOL_API AbstractAttributeExtensibleXMLObject : public virtual AttributeExtensibleXMLObject, public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractAttributeExtensibleXMLObject();
        
        virtual const XMLCh* getAttribute(QName& qualifiedName) const {
            std::map<QName,XMLCh*>::const_iterator i=m_attributeMap.find(qualifiedName);
            return (i==m_attributeMap.end()) ? NULL : i->second;
        }
        
        virtual void setAttribute(QName& qualifiedName, const XMLCh* value);
    
     protected:
        AbstractAttributeExtensibleXMLObject() {}

        /** Copy constructor. */
        AbstractAttributeExtensibleXMLObject(const AbstractAttributeExtensibleXMLObject& src);

        /** Map of arbitrary attributes. */
        std::map<QName,XMLCh*> m_attributeMap;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_absattrextxmlobj_h__ */
