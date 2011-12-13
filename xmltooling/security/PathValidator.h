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
 * @file xmltooling/security/PathValidator.h
 * 
 * Plugin interface to certificate path validation.
 */

#if !defined(__xmltooling_pathval_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_pathval_h__

#include <vector>

class XSECCryptoX509;

namespace xmltooling {

    /**
     * Plugin interface to certificate path validation, independent of context.
     * <p>This interface assumes that the end-entity certificate is "correctly"
     * bound to a party, and solely addresses the validity of that certificate.
     */
    class XMLTOOL_API PathValidator
    {
        MAKE_NONCOPYABLE(PathValidator);
    protected:
        PathValidator();

    public:
        virtual ~PathValidator();

        /**
         * Marker interface for plugin-specific parameters into the validation
         * process.
         */
        class XMLTOOL_API PathValidatorParams {
            MAKE_NONCOPYABLE(PathValidatorParams);
        protected:
            PathValidatorParams();
            
        public:
            virtual ~PathValidatorParams();
        };
        
        /**
         * Validates an end-entity certificate.
         * 
         * @param certEE    end-entity certificate
         * @param certChain the complete untrusted certificate chain
         * @param params    plugin-specific parameters to the validation process
         * @return  true iff validaton succeeds
         */
        virtual bool validate(
            XSECCryptoX509* certEE,
            const std::vector<XSECCryptoX509*>& certChain,
            const PathValidatorParams& params
            ) const=0;
    };

    /**
     * Registers PathValidator classes into the runtime.
     */
    void XMLTOOL_API registerPathValidators();

    /** PathValidator based on PKIX. */
    #define PKIX_PATHVALIDATOR  "PKIX"
};

#endif /* __xmltooling_pathval_h__ */
