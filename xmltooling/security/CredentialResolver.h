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
 * @file xmltooling/signature/CredentialResolver.h
 * 
 * Resolves keys and certificates "owned" by an entity 
 */

#if !defined(__xmltooling_credres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_credres_h__

#include <xmltooling/Lockable.h>

#include <vector>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

namespace xmltooling {

    /**
     * An API for resolving local/owned keys and certificates
     */
    class XMLTOOL_API CredentialResolver : public Lockable
    {
        MAKE_NONCOPYABLE(CredentialResolver);
    protected:
        CredentialResolver() {}
        
    public:
        virtual ~CredentialResolver() {}
        
        /**
         * Returns a secret or private key to use for signing operations.
         * The caller is responsible for deleting the key when finished with it.
         * 
         * @return  a secret or private key
         */
        virtual XSECCryptoKey* getKey() const=0;
        
        /**
         * Returns a set of certificates to publish during signing operations.
         * The certificates must be cloned if kept beyond the scope of a lock.
         * 
         * @return  a set of certificates
         */
        virtual const std::vector<XSECCryptoX509*>& getCertificates() const=0;
    };

    /**
     * Registers CredentialResolver classes into the runtime.
     */
    void XMLTOOL_API registerCredentialResolvers();

    /** CredentialResolver based on local files */
    #define FILESYSTEM_CREDENTIAL_RESOLVER  "File"
};

#endif /* __xmltooling_credres_h__ */
