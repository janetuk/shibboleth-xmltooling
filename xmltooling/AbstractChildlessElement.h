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
 * @file AbstractChildlessElement.h
 * 
 * AbstractXMLObject mixin that blocks children
 */

#ifndef __xmltooling_absnokids_h__
#define __xmltooling_absnokids_h__

#include <xmltooling/AbstractXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that blocks children.
     * Inherit from this class to implement a childless element.
     */
    class XMLTOOL_API AbstractChildlessElement : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractChildlessElement() {}
        
        bool hasChildren() const {
            return false;
        }

        const std::list<XMLObject*>& getOrderedChildren() const {
            return m_no_children;
        }

    protected:
        AbstractChildlessElement() {}
        
        /** Copy constructor. */
        AbstractChildlessElement(const AbstractChildlessElement& src) {}

    private:
        static std::list<XMLObject*> m_no_children;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_absnokids_h__ */
