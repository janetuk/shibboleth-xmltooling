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
 * XMLSecSignature.h
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

    class XMLTOOL_DLLLOCAL XMLSecSignatureImpl : public UnknownElementImpl, public virtual Signature
    {
    public:
        XMLSecSignatureImpl() : UnknownElementImpl(XMLConstants::XMLSIG_NS, Signature::LOCAL_NAME),
            m_signature(NULL), m_c14n(NULL), m_sm(NULL) {}
        virtual ~XMLSecSignatureImpl();
        
        void releaseDOM();
        XMLObject* clone() const;

        DOMElement* marshall(DOMDocument* document=NULL, MarshallingContext* ctx=NULL) const;
        DOMElement* marshall(DOMElement* parentElement, MarshallingContext* ctx=NULL) const;
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false);
        
        // Getters
        const XMLCh* getCanonicalizationMethod() const { return m_c14n ? m_c14n : DSIGConstants::s_unicodeStrURIEXC_C14N_NOC; }
        const XMLCh* getSignatureAlgorithm() const { return m_sm ? m_sm : DSIGConstants::s_unicodeStrURIRSA_SHA1; }

        // Setters
        void setCanonicalizationMethod(const XMLCh* c14n) { m_c14n = prepareForAssignment(m_c14n,c14n); }
        void setSignatureAlgorithm(const XMLCh* sm) { m_sm = prepareForAssignment(m_sm,sm); }

        void sign(const SigningContext& ctx);
        void verify(const VerifyingContext& ctx) const;

    private:
        mutable DSIGSignature* m_signature;
        XMLCh* m_c14n;
        XMLCh* m_sm;
    };

    class XMLTOOL_DLLLOCAL XMLSecSignatureBuilder : public virtual XMLObjectBuilder
    {
    public:
        XMLSecSignatureImpl* buildObject() const {
            return new XMLSecSignatureImpl();
        }
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlsecsig_h__ */
