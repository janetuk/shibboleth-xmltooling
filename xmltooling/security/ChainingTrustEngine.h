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
 * @file xmltooling/security/ChainingTrustEngine.h
 * 
 * OpenSSLTrustEngine that uses multiple engines in sequence.
 */

#if !defined(__xmltooling_chaintrust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_chaintrust_h__

#include <xmltooling/security/OpenSSLTrustEngine.h>

namespace xmltooling {

    /**
     * OpenSSLTrustEngine that uses multiple engines in sequence.
     */
    class XMLTOOL_API ChainingTrustEngine : public OpenSSLTrustEngine {
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
        ChainingTrustEngine(const DOMElement* e=NULL);
        
        /**
         * Destructor will delete any embedded engines.
         */
        virtual ~ChainingTrustEngine();

        /**
         * Adds a trust engine for future calls.
         * 
         * @param newEngine trust engine to add
         */
        void addTrustEngine(TrustEngine* newEngine) {
            m_engines.push_back(newEngine);
        }

        /**
         * Removes a trust engine. The caller must delete the engine if necessary.
         * 
         * @param oldEngine trust engine to remove
         * @return  the old engine
         */
        TrustEngine* removeTrustEngine(TrustEngine* oldEngine) {
            for (std::vector<TrustEngine*>::iterator i=m_engines.begin(); i!=m_engines.end(); i++) {
                if (oldEngine==(*i)) {
                    m_engines.erase(i);
                    return oldEngine;
                }
            }
            return NULL;
        }

        bool validate(
            xmlsignature::Signature& sig,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=NULL
            ) const;
        bool validate(
            const XMLCh* sigAlgorithm,
            const char* sig,
            xmlsignature::KeyInfo* keyInfo,
            const char* in,
            unsigned int in_len,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=NULL
            ) const;
        bool validate(
            XSECCryptoX509* certEE,
            const std::vector<XSECCryptoX509*>& certChain,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=NULL
            ) const;
        bool validate(
            X509* certEE,
            STACK_OF(X509)* certChain,
            const CredentialResolver& credResolver,
            CredentialCriteria* criteria=NULL
            ) const;
    private:
        std::vector<TrustEngine*> m_engines;
    };
    
};

#endif /* __xmltooling_chaintrust_h__ */
