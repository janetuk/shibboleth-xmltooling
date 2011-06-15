/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * @file xmltooling/signature/SignatureValidator.h
 * 
 * Validator for signatures based on an externally-supplied key.
 */

#if !defined(__xmltooling_sigval_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sigval_h__

#include <xmltooling/validation/Validator.h>

class XSECCryptoKey;

namespace xmltooling {
    class XMLTOOL_API Credential;
};

namespace xmlsignature {

    class XMLTOOL_API Signature;

    /**
     * Validator for signatures based on a Credential
     */
    class XMLTOOL_API SignatureValidator : public xmltooling::Validator
    {
    public:
        /**
         * Constructor using a key
         * 
         * @param key the key to use
         */
        SignatureValidator(XSECCryptoKey* key=nullptr);

        /**
         * Constructor using a Credential
         * 
         * @param credential the credential to use
         */
        SignatureValidator(const xmltooling::Credential* credential);

        virtual ~SignatureValidator();

        void validate(const xmltooling::XMLObject* xmlObject) const;

        /**
         * Type-safe validator.
         * 
         * @param signature object to validate
         */
        virtual void validate(const Signature* signature) const;
        
        /**
         * Replace the current key, if any, with a new one.
         * 
         * @param key  the key to attach 
         */
        void setKey(XSECCryptoKey* key);

        /**
         * Replace the current Credential, if any, with a new one.
         * 
         * @param credential  the Credential to attach 
         */
        void setCredential(const xmltooling::Credential* credential);
    
    protected:
        /** Verification key. */
        XSECCryptoKey* m_key;

        /** Verification credential. */
        const xmltooling::Credential* m_credential;
    };

};

#endif /* __xmltooling_sigval_h__ */
