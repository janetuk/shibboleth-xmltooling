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
 * @file XMLSecSignature.h
 * 
 * Signature classes for XMLSec-based signature-handling
 */

#if !defined(__xmltooling_xmlsecsig_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_xmlsecsig_h__

#include "internal.h"
#include "impl/UnknownElement.h"
#include "signature/Signature.h"
#include "util/XMLConstants.h"

#include <string>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * XMLObject representing XML Digital Signature, version 20020212, Signature element.
     * Manages an Apache XML Signature object and the associated DOM.  
     */
    class XMLTOOL_DLLLOCAL XMLSecSignatureImpl : public UnknownElementImpl, public virtual Signature
    {
    public:
        XMLSecSignatureImpl() : UnknownElementImpl(XMLConstants::XMLSIG_NS, Signature::LOCAL_NAME),
            m_signature(NULL), m_c14n(NULL), m_sm(NULL) {}
        virtual ~XMLSecSignatureImpl();
        
        void releaseDOM();
        XMLObject* clone() const;

        // Getters
        const XMLCh* getCanonicalizationMethod() const { return m_c14n ? m_c14n : DSIGConstants::s_unicodeStrURIEXC_C14N_NOC; }
        const XMLCh* getSignatureAlgorithm() const { return m_sm ? m_sm : DSIGConstants::s_unicodeStrURIRSA_SHA1; }
        const DSIGKeyInfoList* getKeyInfo() const;

        // Setters
        void setCanonicalizationMethod(const XMLCh* c14n) { m_c14n = prepareForAssignment(m_c14n,c14n); }
        void setSignatureAlgorithm(const XMLCh* sm) { m_sm = prepareForAssignment(m_sm,sm); }

        void sign(const SigningContext* ctx);

    private:
        DSIGSignature* m_signature;
        XMLCh* m_c14n;
        XMLCh* m_sm;

        friend class XMLTOOL_DLLLOCAL XMLSecSignatureMarshaller;
        friend class XMLTOOL_DLLLOCAL XMLSecSignatureUnmarshaller;
    };

    /**
     * Factory for XMLSecSignatureImpl objects
     */
    class XMLTOOL_DLLLOCAL XMLSecSignatureBuilder : public virtual XMLObjectBuilder
    {
    public:
        /**
         * @see XMLObjectBuilder::buildObject()
         */
        XMLObject* buildObject() const {
            return new XMLSecSignatureImpl();
        }
    };

    /**
     * Marshaller for XMLSecSignatureImpl objects
     */
    class XMLTOOL_DLLLOCAL XMLSecSignatureMarshaller : public virtual Marshaller
    {
    public:
        /**
         * @see Marshaller::marshall(XMLObject*,DOMDocument*, const MarshallingContext*)
         */
        DOMElement* marshall(XMLObject* xmlObject, DOMDocument* document=NULL, MarshallingContext* ctx=NULL) const;

        /**
         * @see Marshaller::marshall(XMLObject*,DOMElement*, const MarshallingContext* ctx)
         */
        DOMElement* marshall(XMLObject* xmlObject, DOMElement* parentElement, MarshallingContext* ctx=NULL) const;
        
    protected:
        void setDocumentElement(DOMDocument* document, DOMElement* element) const {
            DOMElement* documentRoot = document->getDocumentElement();
            if (documentRoot)
                document->replaceChild(documentRoot, element);
            else
                document->appendChild(element);
        }
    };

    /**
     * Unmarshaller for XMLSecSignatureImpl objects
     */
    class XMLTOOL_DLLLOCAL XMLSecSignatureUnmarshaller : public virtual Unmarshaller
    {
    public:
        /**
         * @see Unmarshaller::unmarshall()
         */
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false) const;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlsecsig_h__ */
