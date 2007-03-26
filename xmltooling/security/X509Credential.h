/*
 *  Copyright 2001-2007 Internet2
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
 * @file xmltooling/security/X509Credential.h
 * 
 * Wraps an X.509-based Credential. 
 */

#if !defined(__xmltooling_x509cred_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_x509cred_h__

#include <xmltooling/security/Credential.h>
#include <xmltooling/security/XSECCryptoX509CRL.h>

#include <vector>
#include <xsec/enc/XSECCryptoX509.hpp>

namespace xmltooling {

    /**
     * Wraps an X.509-based Credential.
     */
    class XMLTOOL_API X509Credential : public virtual Credential
    {
    protected:
        X509Credential() {}
        
    public:
        virtual ~X509Credential() {}

        enum ResolveTypes {
            RESOLVE_CERTS = 2,
            RESOLVE_CRLS = 4
        };

        /**
         * Gets an immutable collection of certificates in the entity's trust chain. The entity certificate is contained
         * within this list. No specific ordering of the certificates is guaranteed.
         * 
         * @return a certificate chain
         */
        virtual const std::vector<XSECCryptoX509*>& getEntityCertificateChain() const=0;

        /**
         * Gets a CRL associated with the credential.
         * 
         * @return CRL associated with the credential
         */
        virtual XSECCryptoX509CRL* getCRL() const=0;
    };
};

#endif /* __xmltooling_x509cred_h__ */
