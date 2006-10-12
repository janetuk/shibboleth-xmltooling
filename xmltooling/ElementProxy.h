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
 * @file xmltooling/ElementProxy.h
 * 
 * An XMLObject with an open content model 
 */

#ifndef __xmltooling_eleproxy_h__
#define __xmltooling_eleproxy_h__

#include <xmltooling/XMLObject.h>
#include <xmltooling/util/XMLObjectChildrenList.h>

using namespace xercesc;

namespace xmltooling {

    /**
     * An XMLObject that exposes its children via mutable list.
     */
    class XMLTOOL_API ElementProxy : public virtual XMLObject
    {
    public:
        ElementProxy() {}
        virtual ~ElementProxy() {}
        
        /**
         * Gets a mutable list of child objects
         * 
         * @return  mutable list of child objects
         */
        virtual ListOf(XMLObject) getXMLObjects()=0;

        /**
         * Gets an immutable list of child objects
         * 
         * @return  immutable list of child objects
         */
        virtual const std::list<XMLObject*>& getXMLObjects() const=0;
    };
    
};

#endif /* __xmltooling_eleproxy_h__ */
