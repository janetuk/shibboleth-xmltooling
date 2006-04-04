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
 * @file AbstractComplexElement.h
 * 
 * AbstractXMLObject mixin that implements children
 */

#ifndef __xmltooling_abscomplexel_h__
#define __xmltooling_abscomplexel_h__

#include <xmltooling/AbstractXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements children.
     * Inherit from this class to implement an element with child objects.
     * No unprotected access to them is supplied here.
     */
    class XMLTOOL_API AbstractComplexElement : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractComplexElement();
        
        bool hasChildren() const {
            return !m_children.empty();
        }

        const std::list<XMLObject*>& getOrderedChildren() const {
            return m_children;
        }

    protected:
        AbstractComplexElement() {}
        
        /** Copy constructor. */
        AbstractComplexElement(const AbstractComplexElement& src) {}

        /**
         * Underlying list of child objects.
         * Manages the lifetime of the children.
         */
        std::list<XMLObject*> m_children;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abscomplexel_h__ */
