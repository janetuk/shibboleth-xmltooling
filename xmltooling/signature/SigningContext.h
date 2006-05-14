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
 * @file SigningContext.h
 * 
 * Interface to signing process supplied by a signing application 
 */

#if !defined(__xmltooling_signctx_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_signctx_h__

#include <xmltooling/signature/KeyInfo.h>

#include <vector>
#include <xsec/dsig/DSIGSignature.hpp>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlsignature {

    /**
     * Interface to signing process supplied by a signing application
     */
    class XMLTOOL_API SigningContext
    {
        MAKE_NONCOPYABLE(SigningContext);
    public:
        virtual ~SigningContext() {}

        /**
         * Given a "blank" native signature, asks the context to define the
         * appropriate signature transforms, references, etc.
         * This method MAY attach ds:KeyInfo information, or a set of X.509
         * certificates can be returned from the SigningContext::getX509Certificates()
         * method instead.
         * 
         * @param sig   native signature interface
         */
        virtual void createSignature(DSIGSignature* sig) const=0;
        
        /**
         * Gets a reference to a collection of certificates to append to
         * the ds:KeyInfo element in a ds:X509Data chain.
         * The certificate corresponding to the signing key SHOULD be
         * first, followed by any additional intermediates to append. 
         * 
         * @return  an immutable collection of certificates to embed
         */
        virtual const std::vector<XSECCryptoX509*>* getX509Certificates() const=0;

        /**
         * Gets a KeyInfo structure to embed.
         * Ownership of the object MUST be transferred to the caller.
         * This method will only be called if no certificates are returned from
         * the getX509Certificates() method.
         * 
         * @return  pointer to a KeyInfo structure, will be freed by caller
         */
        virtual KeyInfo* getKeyInfo() const=0;

        /**
         * Gets the signing key to use.
         * Must be compatible with the intended signature algorithm. Ownership of the key
         * MUST be transferred to the caller.
         * 
         * @return  pointer to a signing key, will be freed by caller
         */
        virtual XSECCryptoKey* getSigningKey() const=0;
        
    protected:
        SigningContext() {}
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_signctx_h__ */
