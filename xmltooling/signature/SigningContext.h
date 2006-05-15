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
#include <xmltooling/util/CredentialResolver.h>

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
         * appropriate signature transforms, references, etc. The context
         * should return true iff the necessary ds:KeyInfo information was
         * also attached.
         * 
         * @param sig   native signature interface
         * @return      indicator whether ds:KeyInfo was created by context
         */
        virtual bool createSignature(DSIGSignature* sig)=0;
        
        /**
         * Gets a reference to a credential resolver.
         * The resolver's certificates will be included in the signature only
         * if the context returns false when creating the signature and returns
         * NULL from the getKeyInfo() method.
         * 
         * 
         * @return  a resolver to the credentials to sign with
         */
        virtual xmltooling::CredentialResolver& getCredentialResolver()=0;

        /**
         * Gets a KeyInfo structure to embed.
         * Ownership of the object MUST be transferred to the caller.
         * 
         * @return  pointer to a KeyInfo structure, will be freed by caller
         */
        virtual KeyInfo* getKeyInfo()=0;

    protected:
        SigningContext() {}
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_signctx_h__ */
