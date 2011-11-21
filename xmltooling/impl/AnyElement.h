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
 * @file xmltooling/impl/AnyElement.h
 * 
 * Advanced anyType implementation suitable for deep processing of unknown content.
 */

#ifndef __xmltooling_anyelement_h__
#define __xmltooling_anyelement_h__

#include <xmltooling/ElementProxy.h>
#include <xmltooling/AbstractAttributeExtensibleXMLObject.h>
#include <xmltooling/AbstractComplexElement.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Implements a smart wrapper around unknown or arbitrary DOM content.
     */
    class XMLTOOL_API AnyElementImpl : public virtual ElementProxy,
        public AbstractDOMCachingXMLObject,
        public AbstractComplexElement,
        public AbstractAttributeExtensibleXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
    {
    /// @cond OFF
    public:
        virtual ~AnyElementImpl();

        AnyElementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=nullptr, const QName* schemaType=nullptr);
        
        XMLObject* clone() const;
        
    protected:
        AnyElementImpl();
        AnyElementImpl(const AnyElementImpl& src);
        
        /**
         * Copies the content of a source object into a newly constructed instance.
         * <p>Used to solve compiler problems that limit calling virtual functions
         * from the actual copy constructor.
         *
         * @param src source to clone
         */
        void _clone(const AnyElementImpl& src);

        IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());
        
        void marshallAttributes(xercesc::DOMElement* domElement) const;
        void processChildElement(XMLObject* childXMLObject, const xercesc::DOMElement* childRoot);
        void processAttribute(const xercesc::DOMAttr* attribute);
    };
    /// @endcond

    /**
     * Builder for AnyElementImpl objects.
     * Use as the default builder when you want to wrap each unknown element and
     * process the DOM content through xmltooling interfaces. 
     */
    class XMLTOOL_API AnyElementBuilder : public XMLObjectBuilder
    {
    public:
        XMLObject* buildObject(
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=nullptr, const QName* schemaType=nullptr
            ) const;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_anyelement_h__ */
