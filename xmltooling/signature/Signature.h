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
 * @file Signature.h
 * 
 * XMLObject representing XML Digital Signature, version 20020212, Signature element. 
 */

#if !defined(__xmltooling_sig_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sig_h__

#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/signature/SigningContext.h>
#include <xmltooling/signature/VerifyingContext.h>

/**
 * @namespace xmlsignature
 * Public namespace of XML Signature classes
 */
namespace xmlsignature {

    /**
     * XMLObject representing XML Digital Signature, version 20020212, Signature element.
     * The default signature settings include Exclusive c14n w/o comments, SHA-1 digests,
     * and RSA-SHA1 signing. 
     */
    class XMLTOOL_API Signature : public virtual xmltooling::XMLObject
    {
    public:
        virtual ~Signature() {}

        /** Element local name */
        static const XMLCh LOCAL_NAME[];

        /**
         * Sets the canonicalization method for the ds:SignedInfo element
         * 
         * @param c14n  the canonicalization method
         */
        virtual void setCanonicalizationMethod(const XMLCh* c14n)=0;
        
        /**
         * Sets the signing algorithm for the signature.
         * 
         * @param sm    the signature algorithm
         */
        virtual void setSignatureAlgorithm(const XMLCh* sm)=0;
        
        /**
         * Applies an XML signature based on the supplied context.
         * 
         * @param ctx   the signing context that determines the signature's content
         * @throws SignatureException   thrown if the signing operation fails
         */
        virtual void sign(const SigningContext& ctx)=0;
        
        /**
         * Verifies an XML signature based on the supplied context.
         * 
         * @param ctx   the verifying context that validates the signature's content
         * @throws SignatureException   thrown if the verifying operation fails
         */
        virtual void verify(const VerifyingContext& ctx) const=0;

    protected:
        Signature() {}
    };

    /**
     * Builder for Signature objects.
     */
    class XMLTOOL_API SignatureBuilder : public xmltooling::XMLObjectBuilder
    {
    public:
        virtual Signature* buildObject(
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const xmltooling::QName* schemaType=NULL
            ) const;
            
        /**
         * Default builder
         * 
         * @return empty Signature object
         */
        virtual Signature* buildObject() const;
    };

    DECL_XMLTOOLING_EXCEPTION(SignatureException,xmlsignature,xmltooling::XMLToolingException,Exceptions in signature processing);

};

#endif /* __xmltooling_sig_h__ */
