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
 * @file xmltooling/ElementExtensibleXMLObject.h
 * 
 * An XMLObject that exposes arbitrary children via a mutable vector. 
 */

#ifndef __xmltooling_eleextxmlobj_h__
#define __xmltooling_eleextxmlobj_h__

#include <xmltooling/XMLObject.h>
#include <xmltooling/util/XMLObjectChildrenList.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * An XMLObject that exposes arbitrary children via a mutable vector.
     */
    class XMLTOOL_API ElementExtensibleXMLObject : public virtual XMLObject
    {
    protected:
        ElementExtensibleXMLObject() {}
    
    public:
        virtual ~ElementExtensibleXMLObject() {}
        
        /**
         * Gets a mutable list of child objects
         * 
         * @return  mutable list of child objects
         */
        virtual VectorOf(XMLObject) getUnknownXMLObjects()=0;

        /**
         * Gets an immutable list of child objects
         * 
         * @return  immutable list of child objects
         */
        virtual const std::vector<XMLObject*>& getUnknownXMLObjects() const=0;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_eleextxmlobj_h__ */
