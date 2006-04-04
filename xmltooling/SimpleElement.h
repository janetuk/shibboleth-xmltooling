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
 * @file SimpleElement.h
 * 
 * An XMLObject with a simple content model. 
 */

#ifndef __xmltooling_simpleel_h__
#define __xmltooling_simpleel_h__

#include <xmltooling/XMLObject.h>
#include <xmltooling/util/XMLObjectChildrenList.h>

using namespace xercesc;

namespace xmltooling {

    /**
     * An XMLObject with a simple content model.
     */
    class XMLTOOL_API SimpleElement : public virtual XMLObject
    {
    public:
        SimpleElement() {}
        virtual ~SimpleElement() {}
        
        /**
         * Gets the text content of the object
         * 
         * @return the text content, or NULL
         */
        virtual const XMLCh* getTextContent() const=0;
        
        /**
         * Sets (or clears) the text content of the object 
         * 
         * @param value         value to set, or NULL to clear
         */
        virtual void setTextContent(const XMLCh* value)=0;
    };
    
};

#endif /* __xmltooling_simpleel_h__ */
