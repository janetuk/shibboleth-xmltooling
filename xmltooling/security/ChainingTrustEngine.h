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
 * @file xmltooling/security/ChainingTrustEngine.h
 * 
 * OpenSSLTrustEngine that uses multiple engines in sequence.
 */

#if !defined(__xmltooling_chaintrust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_chaintrust_h__

#include <xmltooling/security/OpenSSLTrustEngine.h>
#include <xmltooling/security/SignatureTrustEngine.h>

#include <boost/ptr_container/ptr_vector.hpp>

namespace xmltooling {

    /**
     * OpenSSLTrustEngine that uses multiple engines in sequence.
     */
    class XMLTOOL_API ChainingTrustEngine : public SignatureTrustEngine, public OpenSSLTrustEngine {
    public:
        /**
         * Constructor.
         * 
         * If a DOM is supplied, the following XML content is supported:
         * 
         * <ul>
         *  <li>&lt;TrustEngine&gt; elements with a type attribute
         * </ul>
         * 
         * XML namespaces are ignored in the processing of this content.
         * 
         * @param e DOM to supply configuration for provider
         */
        ChainingTrustEngine(const xercesc::DOMElement* e=nullptr);
        
        /**
         * Destructor will delete any embedded engines.
         */
        virtual ~ChainingTrustEngine();

        /**
         * Adds a trust engine for future calls.
         * 
         * @param newEngine trust engine to add
         */
        void addTrustEngine(TrustEngine* newEngine);

        /**
         * Removes a trust engine. The caller must delete the engine if necessary.
         * 
         * @param oldEngine trust engine to remove
         * @return  the old engine
         */
        TrustEngine* removeTrustEngine(TrustEngine* oldEngine);

        bool validate(
            xmlsignature::Signature& sig,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const;
        bool validate(
            const XMLCh* sigAlgorithm,
            const char* sig,
            xmlsignature::KeyInfo* keyInfo,
            const char* in,
            unsigned int in_len,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const;
        bool validate(
            XSECCryptoX509* certEE,
            const std::vector<XSECCryptoX509*>& certChain,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const;
        bool validate(
            X509* certEE,
            STACK_OF(X509)* certChain,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=nullptr
            ) const;
    private:
        boost::ptr_vector<TrustEngine> m_engines;
        boost::ptr_vector<SignatureTrustEngine,boost::view_clone_allocator> m_sigEngines;
        boost::ptr_vector<X509TrustEngine,boost::view_clone_allocator> m_x509Engines;
        boost::ptr_vector<OpenSSLTrustEngine,boost::view_clone_allocator> m_osslEngines;
    };
    
};

#endif /* __xmltooling_chaintrust_h__ */
