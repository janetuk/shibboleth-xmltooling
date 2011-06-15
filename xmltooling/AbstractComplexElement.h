/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * @file xmltooling/AbstractComplexElement.h
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
     * Inherit from this class to implement an element with child objects and mixed content.
     */
    class XMLTOOL_API AbstractComplexElement : public virtual AbstractXMLObject
    {
    public:
        virtual ~AbstractComplexElement();

        // Virtual function overrides.
        bool hasChildren() const;
        const std::list<XMLObject*>& getOrderedChildren() const;
        void removeChild(XMLObject* child);
        const XMLCh* getTextContent(unsigned int position=0) const;
        void setTextContent(const XMLCh* value, unsigned int position=0);

    protected:
        AbstractComplexElement();
        
        /** Copy constructor. */
        AbstractComplexElement(const AbstractComplexElement& src);

        /**
         * Underlying list of child objects.
         * Manages the lifetime of the children.
         */
        std::list<XMLObject*> m_children;
        
        /**
         * Interstitial text nodes.
         * Needed to support mixed content, and preserve DOM whitespace across rebuilds.
         */
        std::vector<XMLCh*> m_text;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abscomplexel_h__ */
