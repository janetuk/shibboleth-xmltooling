/*
*  Copyright 2001-2007 Internet2
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
 * @file xmltooling/impl/UnknownElement.h
 * 
 * Basic implementation suitable for use as default for unrecognized content
 */

#ifndef __xmltooling_unkelement_h__
#define __xmltooling_unkelement_h__

#include <xmltooling/AbstractSimpleElement.h>
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
    class XMLTOOL_DLLLOCAL UnknownElementImpl : public AbstractSimpleElement, public AbstractDOMCachingXMLObject
    {
    public:
        UnknownElementImpl(const XMLCh* namespaceURI=NULL, const XMLCh* elementLocalName=NULL, const XMLCh* namespacePrefix=NULL)
            : AbstractXMLObject(namespaceURI, elementLocalName, namespacePrefix) {}
    
        void releaseDOM() const;

        XMLObject* clone() const;

        const XMLCh* getTextContent(unsigned int position=0) const {
            throw XMLObjectException("Direct access to content is not permitted.");
        }

        void setTextContent(const XMLCh*, unsigned int position=0) {
            throw XMLObjectException("Direct access to content is not permitted.");
        }

        DOMElement* marshall(
            DOMDocument* document=NULL
#ifndef XMLTOOLING_NO_XMLSEC
            ,const std::vector<xmlsignature::Signature*>* sigs=NULL
#endif
            ) const;

        DOMElement* marshall(
            DOMElement* parentElement
#ifndef XMLTOOLING_NO_XMLSEC
            ,const std::vector<xmlsignature::Signature*>* sigs=NULL
#endif
            ) const;
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false);
        
    protected:
        void setDocumentElement(DOMDocument* document, DOMElement* element) const {
            DOMElement* documentRoot = document->getDocumentElement();
            if (documentRoot)
                document->replaceChild(element, documentRoot);
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
            ) const;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_unkelement_h__ */
