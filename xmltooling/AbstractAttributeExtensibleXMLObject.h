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
 * @file AbstractAttributeExtensibleXMLObject.h
 * 
 * An abstract implementation of a DOM-caching AttributeExtensibleXMLObject 
 */

#if !defined(__xmltooling_absattrextxmlobj_h__)
#define __xmltooling_absattrextxmlobj_h__

#include <map>
#include <xmltooling/AbstractDOMCachingXMLObject.h>
#include <xmltooling/AttributeExtensibleXMLObject.h>

using namespace xercesc;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * An abstract implementation of a DOM-caching AttributeExtensibleXMLObject.
     */
    class XMLTOOL_API AbstractAttributeExtensibleXMLObject : public virtual AbstractDOMCachingXMLObject
    {
    public:
        virtual ~AbstractAttributeExtensibleXMLObject();
        
        /**
         * @see AttributeExtensibleXMLObject::getAttribute()
         */
        virtual const XMLCh* getAttribute(QName& qualifiedName) const {
            std::map<QName,XMLCh*>::const_iterator i=m_attributeMap.find(qualifiedName);
            return (i==m_attributeMap.end()) ? NULL : i->second;
        }
        
        /**
         * @see AttributeExtensibleXMLObject::setAttribute()
         */
        virtual void setAttribute(QName& qualifiedName, const XMLCh* value);
    
     protected:
        /**
         * Constructor
         * 
         * @param namespaceURI the namespace the element is in
         * @param elementLocalName the local name of the XML element this Object represents
         * @param namespacePrefix the namespace prefix to use
         */
        AbstractAttributeExtensibleXMLObject(
            const XMLCh* namespaceURI=NULL, const XMLCh* elementLocalName=NULL, const XMLCh* namespacePrefix=NULL
            ) : AbstractDOMCachingXMLObject(namespaceURI,elementLocalName, namespacePrefix) {}

        std::map<QName,XMLCh*> m_attributeMap;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_absattrextxmlobj_h__ */
