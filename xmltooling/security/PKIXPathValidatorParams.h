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
 * @file xmltooling/security/PKIXPathValidatorParams.h
 * 
 * PKIX-specific parameters to a PathValidator.
 */

#if !defined(__xmltooling_pkixvalparam_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_pkixvalparam_h__

#include <xmltooling/security/PathValidator.h>

#include <set>
#include <string>

namespace xmltooling {

    class XMLTOOL_API XSECCryptoX509CRL;

    /**
     * PKIX-specific parameters to a PathValidator.
     */
    class XMLTOOL_API PKIXPathValidatorParams : public PathValidator::PathValidatorParams
    {
    protected:
        PKIXPathValidatorParams();

    public:
        virtual ~PKIXPathValidatorParams();

        /**
         * Returns the allowable trust chain verification depth.
         * 
         * @return  allowable trust chain verification depth
         */
        virtual int getVerificationDepth() const=0;

        /**
         * Checks whether the any policy OID should be processed
         * if it is included in a certificate.
         *
         * @return true iff the any policy OID should *not* be processed
         */
        virtual bool isAnyPolicyInhibited() const=0;

        /**
         * Checks if policy mapping is inhibited.
         *
         * @return true iff policy mapping should not be allowed
         */
        virtual bool isPolicyMappingInhibited() const=0;

        /**
         * Returns a set of policy OIDs.
         *
         * @return set of policy OIDs
         */
        virtual const std::set<std::string>& getPolicies() const=0;

        /**
         * Returns a set of trust anchors.
         * 
         * @return  set of trust anchors
         */
        virtual const std::vector<XSECCryptoX509*>& getTrustAnchors() const=0;

        enum revocation_t {
            REVOCATION_OFF = 0,
            REVOCATION_ENTITYONLY = 1,
            REVOCATION_FULLCHAIN = 2
        };

        /**
         * Returns the type of revocation checking to perform.
         *
         * @return  revocation checking option
         */
        virtual revocation_t getRevocationChecking() const=0;

        /**
         * Returns a set of CRLs.
         * 
         * @return  set of CRLs
         */
        virtual const std::vector<XSECCryptoX509CRL*>& getCRLs() const=0;
    };
};

#endif /* __xmltooling_pkixvalparam_h__ */
