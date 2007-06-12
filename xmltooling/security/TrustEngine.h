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
 * Evaluates the trustworthiness and validity of security information against
 * implementation-specific requirements.
 */

#if !defined(__xmltooling_trust_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_trust_h__

#include <xmltooling/base.h>

namespace xmltooling {

    class XMLTOOL_API KeyInfoResolver;

    /**
     * Evaluates the trustworthiness and validity of security information against
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
         *  <li>&lt;KeyInfoResolver&gt; elements with a type attribute
         * </ul>
         * 
         * XML namespaces are ignored in the processing of this content.
         * 
         * @param e DOM to supply configuration for provider
         */
        TrustEngine(const xercesc::DOMElement* e=NULL);
        
        /** Custom KeyInfoResolver instance. */
        KeyInfoResolver* m_keyInfoResolver;
        
    public:
        virtual ~TrustEngine();

        /**
         * Supplies a KeyInfoResolver instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param keyInfoResolver   new KeyInfoResolver instance to use
         */
        void setKeyInfoResolver(KeyInfoResolver* keyInfoResolver);
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
