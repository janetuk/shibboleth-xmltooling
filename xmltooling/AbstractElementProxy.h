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
 * An abstract implementation of a DOM-caching ElementProxy 
 */

#if !defined(__xmltooling_abseleproxy_h__)
#define __xmltooling_abseleproxy_h__

#include <xmltooling/AbstractDOMCachingXMLObject.h>
#include <xmltooling/ElementProxy.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * An abstract implementation of a DOM-caching ExtensibleXMLObject.
     */
    class XMLTOOL_API AbstractElementProxy : public virtual ElementProxy, public virtual AbstractDOMCachingXMLObject
    {
    public:
        virtual ~AbstractElementProxy() {}
        
        /**
         * @see ElementProxy::getTextContent()
         */
        virtual const XMLCh* getTextContent() const {
            return m_value;
        }
        
        /**
         * @see ElementProxy::setTextContent()
         */
        virtual void setTextContent(const XMLCh* value);
        

        /**
         * @see ElementProxy::getXMLObjects()
         */
        virtual ListOf(XMLObject) getXMLObjects();
    
     protected:
        AbstractElementProxy() : m_value(NULL) {}

    private:
        XMLCh* m_value;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abseleproxy_h__ */
