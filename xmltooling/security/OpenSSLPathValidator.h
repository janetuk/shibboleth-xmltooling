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
 * @file xmltooling/security/OpenSSLPathValidator.h
 * 
 * Extended PathValidator interface that adds validation
 * using OpenSSL data types directly for efficiency.
 */

#if !defined(__xmltooling_opensslpathval_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_opensslpathval_h__

#include <xmltooling/security/PathValidator.h>

#include <openssl/x509.h>


namespace xmltooling {

    /**
     * Extended PathValidator interface that adds validation
     * using OpenSSL data types directly for efficiency.
     */
    class XMLTOOL_API OpenSSLPathValidator : public PathValidator
    {
        MAKE_NONCOPYABLE(OpenSSLPathValidator);
    protected:
        OpenSSLPathValidator();

    public:
        virtual ~OpenSSLPathValidator();

        /**
         * Validates an end-entity certificate.
         * 
         * @param certEE    end-entity certificate
         * @param certChain the complete untrusted certificate chain
         * @param params    plugin-specific parameters to the validation process
         * @return  true iff validaton succeeds
         */
        virtual bool validate(
            X509* certEE,
            STACK_OF(X509)* certChain,
            const PathValidatorParams& params
            ) const=0;

    };
};

#endif /* __xmltooling_opensslpathval_h__ */
