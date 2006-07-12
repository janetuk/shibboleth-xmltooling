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
 * @file SignatureValidator.h
 * 
 * Validator for signatures based on an externally-supplied key 
 */

#if !defined(__xmltooling_sigval_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sigval_h__

#include <xmltooling/signature/KeyResolver.h>
#include <xmltooling/signature/Signature.h>
#include <xmltooling/validation/Validator.h>

namespace xmlsignature {

    /**
     * Validator for signatures based on a KeyResolver
     */
    class XMLTOOL_API SignatureValidator : public virtual xmltooling::Validator
    {
    public:
        /**
         * Constructor
         * 
         * @param resolver   the key resolver to use, will be freed by Validator
         */
        SignatureValidator(KeyResolver* resolver) : m_resolver(resolver) {
        }
        
        virtual ~SignatureValidator() {
            delete m_resolver;
        }

        void validate(const xmltooling::XMLObject* xmlObject) const;

        virtual void validate(const Signature* signature) const;
        
        /**
         * Replace the current KeyResolver, if any, with a new one.
         * 
         * @param resolver  the KeyResolver to attach 
         */
        void setKeyResolver(KeyResolver* resolver) {
            delete m_resolver;
            m_resolver=resolver;
        }
    
    protected:
        KeyResolver* m_resolver;
    };

};

#endif /* __xmltooling_sigval_h__ */
