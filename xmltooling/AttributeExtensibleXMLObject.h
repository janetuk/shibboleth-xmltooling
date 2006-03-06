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
 * @file AttributeExtensibleXMLObject.h
 * 
 * An XMLObject that supports arbitrary attributes 
 */

#if !defined(__xmltooling_attrextxmlobj_h__)
#define __xmltooling_attrextxmlobj_h__

#include <xmltooling/XMLObject.h>

using namespace xercesc;

namespace xmltooling {

    /**
     * An XMLObject that supports arbitrary attributes.
     */
    class XMLTOOL_API AttributeExtensibleXMLObject : public virtual XMLObject
    {
    public:
        AttributeExtensibleXMLObject() {}
        virtual ~AttributeExtensibleXMLObject() {}
        
        /**
         * Gets the value of an XML attribute of the object
         * 
         * @param   qualifiedName   qualified name of the attribute   
         * @return the attribute value, or NULL
         */
        virtual const XMLCh* getAttribute(QName& qualifiedName) const=0;
        
        /**
         * Sets (or clears) an XML attribute of the object 
         * 
         * @param qualifiedName qualified name of the attribute   
         * @param value         value to set, or NULL to clear
         */
        virtual void setAttribute(QName& qualifiedName, const XMLCh* value)=0;
    };
    
};

#endif /* __xmltooling_attrextxmlobj_h__ */
