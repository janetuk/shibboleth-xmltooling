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
 * @file VerifyingContext.h
 * 
 * Interface to signature verification process supplied by a relying party 
 */

#if !defined(__xmltooling_verctx_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_verctx_h__

#include <xsec/dsig/DSIGSignature.hpp>

namespace xmltooling {

    /**
     * Interface to signature verification process supplied by a relying party
     */
    class XMLTOOL_API VerifyingContext
    {
        MAKE_NONCOPYABLE(VerifyingContext);
    public:
        virtual ~VerifyingContext() {}

        /**
         * Given a native signature, asks the context to verify the signature
         * in accordance with the relying party's requirements.
         * 
         * @param sig   native signature object
         * 
         * @throws SignatureException   raised if signature is invalid
         */
        virtual void verifySignature(DSIGSignature* sig) const=0;
        
    protected:
        VerifyingContext() {}
    };

};

#endif /* __xmltooling_verctx_h__ */
