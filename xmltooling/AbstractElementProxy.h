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
 * @file AbstractElementProxy.h
 * 
 * AbstractXMLObject mixin that implements an open content model 
 */

#ifndef __xmltooling_abseleproxy_h__
#define __xmltooling_abseleproxy_h__

#include <xmltooling/AbstractComplexElement.h>
#include <xmltooling/AbstractSimpleElement.h>
#include <xmltooling/ElementProxy.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements an open content model.
     * Inherit from this class to merge both simple and complex content
     * and expose the underlying child collection in read/write mode.
     */
    class XMLTOOL_API AbstractElementProxy
        : public virtual ElementProxy, public AbstractSimpleElement, public AbstractComplexElement
    {
    public:
        virtual ~AbstractElementProxy() {}
        
        virtual ListOf(XMLObject) getXMLObjects() {
            return ListOf(XMLObject)(this,m_children,NULL,m_children.end());
        }
    
        virtual const std::list<XMLObject*>& getXMLObjects() const {
            return m_children;
        }

    protected:
        AbstractElementProxy() {}
        
        /** Copy constructor. */
        AbstractElementProxy(const AbstractElementProxy& src)
            : AbstractXMLObject(src), AbstractSimpleElement(src) {}
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abseleproxy_h__ */
