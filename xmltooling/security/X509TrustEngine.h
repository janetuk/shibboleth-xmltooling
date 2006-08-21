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
 * @file xmltooling/security/X509TrustEngine.h
 * 
 * Extended TrustEngine interface that adds validation of X.509 credentials.
 */

#if !defined(__xmltooling_x509trust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_x509trust_h__

#include <xmltooling/security/TrustEngine.h>

namespace xmltooling {

    /**
     * Extended TrustEngine interface that adds validation of X.509 credentials.
     */
    class XMLTOOL_API X509TrustEngine : public TrustEngine {
    protected:
        /**
         * Constructor.
         * 
         * If a DOM is supplied, the following XML content is supported:
         * 
         * <ul>
         *  <li>&lt;KeyResolver&gt; elements with a type attribute
         * </ul>
         * 
         * XML namespaces are ignored in the processing of this content.
         * 
         * @param e DOM to supply configuration for provider
         */
        X509TrustEngine(const DOMElement* e=NULL) : TrustEngine(e) {}
        
    public:
        virtual ~X509TrustEngine() {}
        
        /**
         * Determines whether an X.509 credential is valid with respect to the
         * KeyInfo data supplied. It is the responsibility of the application to
         * ensure that the KeyInfo information supplied is in fact associated with
         * the peer who presented the signature. 
         * 
         * A custom KeyResolver can be supplied from outside the TrustEngine.
         * Alternatively, one may be specified to the plugin constructor.
         * A non-caching, inline resolver will be used as a fallback.
         * 
         * @param certEE        end-entity certificate to validate
         * @param certChain     the complete set of certificates presented for validation (includes certEE)
         * @param keyInfoSource supplies KeyInfo objects to the TrustEngine
         * @param checkName     true iff certificate subject/name checking has <b>NOT</b> already occurred
         * @param keyResolver   optional externally supplied KeyResolver, or NULL
         */
        virtual bool validate(
            XSECCryptoX509* certEE,
            const std::vector<XSECCryptoX509*>& certChain,
            TrustEngine::KeyInfoIterator& keyInfoSource,
            bool checkName=true,
            const xmlsignature::KeyResolver* keyResolver=NULL
            )=0;
    };
    
};

#endif /* __xmltooling_x509trust_h__ */
