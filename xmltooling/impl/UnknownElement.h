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
 * @file UnknownElement.h
 * 
 * Basic implementation suitable for use as default for unrecognized content
 */

#if !defined(__xmltooling_unkelement_h__)
#define __xmltooling_unkelement_h__

#include <xmltooling/AbstractChildlessElement.h>
#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>

#include <string>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /// @cond off
    class XMLTOOL_DLLLOCAL UnknownElementImpl : public AbstractChildlessElement, public AbstractDOMCachingXMLObject
    {
    public:
        UnknownElementImpl(const XMLCh* namespaceURI=NULL, const XMLCh* elementLocalName=NULL, const XMLCh* namespacePrefix=NULL)
            : AbstractXMLObject(namespaceURI, elementLocalName, namespacePrefix) {}
    
        void releaseDOM();

        XMLObject* clone() const;

        DOMElement* marshall(DOMDocument* document=NULL, MarshallingContext* ctx=NULL) const;
        DOMElement* marshall(DOMElement* parentElement, MarshallingContext* ctx=NULL) const;
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false);
        
    protected:
        void setDocumentElement(DOMDocument* document, DOMElement* element) const {
            DOMElement* documentRoot = document->getDocumentElement();
            if (documentRoot)
                document->replaceChild(documentRoot, element);
            else
                document->appendChild(element);
        }

        mutable std::string m_xml;

        void serialize(std::string& s) const;
    };
    /// @endcond
    
    /**
     * Builder for UnknownElementImpl objects.
     * Use as the default builder when you want unknown DOM content treated as raw/ignored XML.
     */
    class XMLTOOL_API UnknownElementBuilder : public XMLObjectBuilder
    {
    public:
        XMLObject* buildObject(
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL
            ) const {
            return new UnknownElementImpl(nsURI,localName,prefix);
        }
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_unkelement_h__ */
