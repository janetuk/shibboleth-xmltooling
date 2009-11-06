/*
 *  Copyright 2001-2009 Internet2
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
 * @file xmltooling/AbstractSimpleElement.h
 * 
 * AbstractXMLObject mixin that implements a simple string-based content model 
 */

#ifndef __xmltooling_abssimpleel_h__
#define __xmltooling_abssimpleel_h__

#include <xmltooling/AbstractXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * AbstractXMLObject mixin that implements a simple string-based content model.
     * Inherit from this class to support string-based element content.
     */
    class XMLTOOL_API AbstractSimpleElement : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractSimpleElement();
        
        // Virtual function overrides.
        bool hasChildren() const;
        const std::list<XMLObject*>& getOrderedChildren() const;
        void removeChild(XMLObject* child);
        const XMLCh* getTextContent(unsigned int position=0) const;
        void setTextContent(const XMLCh* value, unsigned int position=0);
        
    protected:
        AbstractSimpleElement();
        
        /** Copy constructor. */
        AbstractSimpleElement(const AbstractSimpleElement& src);

    private:
        XMLCh* m_value;

        static std::list<XMLObject*> m_no_children;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abssimpleel_h__ */
