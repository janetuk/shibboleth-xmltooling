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
 * @file xmltooling/security/SignatureTrustEngine.h
 * 
 * TrustEngine interface that adds validation of digital signatures.
 */

#if !defined(__xmltooling_sigtrust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sigtrust_h__

#include <xmltooling/security/TrustEngine.h>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
    class XMLTOOL_API Signature;
};

namespace xmltooling {

    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API CredentialResolver;

    /**
     * TrustEngine interface that adds validation of digital signatures.
     */
    class XMLTOOL_API SignatureTrustEngine : public virtual TrustEngine {
    protected:
        /**
         * Constructor.
         * 
         * If a DOM is supplied, the following XML content is supported:
         * 
         * <ul>
         *  <li>&lt;KeyInfoResolver&gt; elements with a type attribute
         * </ul>
         * 
         * XML namespaces are ignored in the processing of this content.
         * 
         * @param e DOM to supply configuration for provider
         */
        SignatureTrustEngine(const xercesc::DOMElement* e=nullptr);
        
    public:
        virtual ~SignatureTrustEngine();

        /**
         * Determines whether an XML signature is correct and valid with respect to
         * the source of credentials supplied.
         * 
         * <p>It is the responsibility of the application to ensure that the credentials
         * supplied are in fact associated with the peer who created the signature.
         * 
         * <p>If criteria with a peer name are supplied, the "name" of the Credential that verifies
         * the signature may also be checked to ensure that it identifies the intended peer.
         * The peer name itself or implementation-specific rules based on the content of the
         * peer credentials may be applied. Implementations may omit this check if they
         * deem it unnecessary.
         * 
         * @param sig           reference to a signature object to validate
         * @param credResolver  a locked resolver to supply trusted peer credentials to the TrustEngine
         * @param criteria      criteria for selecting peer credentials
         * @return  true iff the signature validates
         */
        virtual bool validate(
            xmlsignature::Signature& sig,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const=0;

        /**
         * Determines whether a raw signature is correct and valid with respect to
         * the source of credentials supplied.
         * 
         * <p>It is the responsibility of the application to ensure that the Credentials
         * supplied are in fact associated with the peer who created the signature.
         * 
         * <p>If criteria with a peer name are supplied, the "name" of the Credential that verifies
         * the signature may also be checked to ensure that it identifies the intended peer.
         * The peer name itself or implementation-specific rules based on the content of the
         * peer credentials may be applied. Implementations may omit this check if they
         * deem it unnecessary.
         *
         * <p>Note that the keyInfo parameter is not part of the implicitly trusted
         * set of information supplied via the CredentialResolver, but rather advisory
         * data that may have accompanied the signature itself.
         * 
         * @param sigAlgorithm  XML Signature identifier for the algorithm used
         * @param sig           null-terminated base64-encoded signature value
         * @param keyInfo       KeyInfo object accompanying the signature, if any
         * @param in            the input data over which the signature was created
         * @param in_len        size of input data in bytes
         * @param credResolver  a locked resolver to supply trusted peer credentials to the TrustEngine
         * @param criteria      criteria for selecting peer credentials
         * @return  true iff the signature validates
         */
        virtual bool validate(
            const XMLCh* sigAlgorithm,
            const char* sig,
            xmlsignature::KeyInfo* keyInfo,
            const char* in,
            unsigned int in_len,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const=0;
    };
};

#endif /* __xmltooling_sigtrust_h__ */
