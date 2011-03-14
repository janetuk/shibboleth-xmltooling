/*
 *  Copyright 2001-2010 Internet2
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
 * @file xmltooling/security/OpenSSLTrustEngine.h
 * 
 * Extended TrustEngine interface that adds validation of X.509 credentials
 * using OpenSSL data types directly for efficiency.
 */

#if !defined(__xmltooling_openssltrust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_openssltrust_h__

#include <xmltooling/security/X509TrustEngine.h>

#include <openssl/x509.h>

namespace xmltooling {

    /**
     * Extended TrustEngine interface that adds validation of X.509 credentials
     * using OpenSSL data types directly for efficiency.
     */
    class XMLTOOL_API OpenSSLTrustEngine : public X509TrustEngine {
    protected:
        /**
         * Constructor.
         * 
         * @param e DOM to supply configuration for provider
         */
        OpenSSLTrustEngine(const xercesc::DOMElement* e=nullptr);
        
    public:
        virtual ~OpenSSLTrustEngine();

        using X509TrustEngine::validate;

        /**
         * Determines whether an X.509 credential is valid with respect to the
         * source of credentials supplied.
         * 
         * <p>It is the responsibility of the application to ensure that the credentials
         * supplied are in fact associated with the peer who presented the credential.
         * 
         * <p>If criteria with a peer name are supplied, the "name" of the EE certificate
         * may also be checked to ensure that it identifies the intended peer.
         * The peer name itself or implementation-specific rules based on the content of the
         * peer credentials may be applied. Implementations may omit this check if they
         * deem it unnecessary.
         * 
         * @param certEE        end-entity certificate to validate
         * @param certChain     the complete set of certificates presented for validation (includes certEE)
         * @param credResolver  a locked resolver to supply trusted peer credentials to the TrustEngine
         * @param criteria      criteria for selecting peer credentials
         */
        virtual bool validate(
            X509* certEE, STACK_OF(X509)* certChain,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const=0;
    };
    
};

#endif /* __xmltooling_openssltrust_h__ */
