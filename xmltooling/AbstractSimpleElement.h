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
 * @file AbstractSimpleElement.h
 * 
 * AbstractXMLObject mixin that implements a simple string-based content model 
 */

#ifndef __xmltooling_abssimpleel_h__
#define __xmltooling_abssimpleel_h__

#include <xmltooling/AbstractXMLObject.h>
#include <xmltooling/SimpleElement.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements a simple string-based content model.
     * Inherit from this class to support string-based element content.
     */
    class XMLTOOL_API AbstractSimpleElement : public virtual SimpleElement, public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractSimpleElement() {
            XMLString::release(&m_value);
        }
        
        virtual const XMLCh* getTextContent() const {
            return m_value;
        }
        
        virtual void setTextContent(const XMLCh* value) {
            m_value=prepareForAssignment(m_value,value);
        }
        
    protected:
        AbstractSimpleElement() : m_value(NULL) {}
        
        /** Copy constructor. */
        AbstractSimpleElement(const AbstractSimpleElement& src)
            : AbstractXMLObject(src), m_value(XMLString::replicate(src.m_value)) {}

    private:
        XMLCh* m_value;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abssimpleel_h__ */
