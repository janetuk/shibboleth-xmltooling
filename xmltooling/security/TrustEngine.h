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
 * @file xmltooling/security/TrustEngine.h
 * 
 * Evaluates the trustworthiness and validity of XML Signatures against
 * implementation-specific requirements.
 */

#if !defined(__xmltooling_trust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_trust_h__

#include <xmltooling/security/KeyInfoSource.h>
#include <xmltooling/signature/KeyResolver.h>
#include <xmltooling/signature/Signature.h>

namespace xmltooling {

    /**
     * Evaluates the trustworthiness and validity of XML or raw Signatures against
     * implementation-specific requirements.
     */
    class XMLTOOL_API TrustEngine {
        MAKE_NONCOPYABLE(TrustEngine);
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
        TrustEngine(const DOMElement* e=NULL);
        
        /** Default KeyResolver instance. */
        xmlsignature::KeyResolver* m_keyResolver;
        
    public:
        virtual ~TrustEngine();
        
        /**
         * Determines whether an XML signature is correct and valid with respect to
         * the source of KeyInfo data supplied. It is the responsibility of the
         * application to ensure that the KeyInfo information supplied is in fact
         * associated with the peer who created the signature. 
         * 
         * <p>A custom KeyResolver can be supplied from outside the TrustEngine.
         * Alternatively, one may be specified to the plugin constructor.
         * A non-caching, inline resolver will be used as a fallback.
         * 
         * @param sig           reference to a signature object to validate
         * @param keyInfoSource supplies KeyInfo objects to the TrustEngine
         * @param keyResolver   optional externally supplied KeyResolver, or NULL
         * @return  true iff the signature validates
         */
        virtual bool validate(
            xmlsignature::Signature& sig,
            const KeyInfoSource& keyInfoSource,
            const xmlsignature::KeyResolver* keyResolver=NULL
            ) const=0;

        /**
         * Determines whether a raw signature is correct and valid with respect to
         * the source of KeyInfo data supplied. It is the responsibility of the
         * application to ensure that the KeyInfo information supplied is in fact
         * associated with the peer who created the signature.
         * 
         * <p>A custom KeyResolver can be supplied from outside the TrustEngine.
         * Alternatively, one may be specified to the plugin constructor.
         * A non-caching, inline resolver will be used as a fallback.
         * 
         * <p>Note that the keyInfo parameter is not part of the implicitly trusted
         * set of key information supplied via the KeyInfoSource, but rather advisory
         * data that may have accompanied the signature itself.
         * 
         * @param sigAlgorithm  XML Signature identifier for the algorithm used
         * @param sig           null-terminated base64-encoded signature value
         * @param keyInfo       KeyInfo object accompanying the signature, if any
         * @param in            the input data over which the signature was created
         * @param in_len        size of input data in bytes
         * @param keyInfoSource supplies KeyInfo objects to the TrustEngine
         * @param keyResolver   optional externally supplied KeyResolver, or NULL
         * @return  true iff the signature validates
         */
        virtual bool validate(
            const XMLCh* sigAlgorithm,
            const char* sig,
            xmlsignature::KeyInfo* keyInfo,
            const char* in,
            unsigned int in_len,
            const KeyInfoSource& keyInfoSource,
            const xmlsignature::KeyResolver* keyResolver=NULL
            ) const=0;
    };

    /**
     * Registers TrustEngine classes into the runtime.
     */
    void XMLTOOL_API registerTrustEngines();

    /** TrustEngine based on explicit knowledge of peer key information. */
    #define EXPLICIT_KEY_TRUSTENGINE  "ExplicitKey"
    
    /** TrustEngine that tries multiple engines in sequence. */
    #define CHAINING_TRUSTENGINE  "Chaining"
    
};

#endif /* __xmltooling_trust_h__ */
