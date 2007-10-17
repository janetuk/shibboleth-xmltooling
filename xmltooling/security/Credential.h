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
 * @file xmltooling/security/Credential.h
 * 
 * Wraps keys and related functionality. 
 */

#if !defined(__xmltooling_cred_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_cred_h__

#include <xmltooling/base.h>
#include <xsec/enc/XSECCryptoKey.hpp>

#include <set>
#include <string>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
};

namespace xmltooling {

    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API CredentialContext;

    /**
     * Wraps keys and related functionality.
     *
     * <p>Shared credential implementations should implement reference counting
     * and honor any locking parameters to ensure appropriate synchronization.
     */
    class XMLTOOL_API Credential
    {
        MAKE_NONCOPYABLE(Credential);
    protected:
        Credential() {}
        
    public:
        virtual ~Credential() {}
        
        /**
         * Bitmask constants for limiting resolution process inside a CredentialResolver. 
         */
        enum ResolveTypes {
            RESOLVE_KEYS = 1,
            RESOLVE_NAMES = 2
        };

        /**
         * Enumeration of use cases for credentials. 
         */
        enum UsageTypes {
            UNSPECIFIED_CREDENTIAL,
            SIGNING_CREDENTIAL,
            TLS_CREDENTIAL,
            ENCRYPTION_CREDENTIAL
        };
        
        /**
         * Get the credential usage type.
         * 
         * @return the usage
         */
        virtual UsageTypes getUsage() const=0;

        /**
         * Returns an algorithm identifier for the Credential.
         *
         * @return  the Credential algorithm, or NULL if indeterminate
         */
        virtual const char* getAlgorithm() const=0;

        /**
         * Returns the size of the key.
         *
         * @return  the key size, or 0 if indeterminate
         */
        virtual unsigned int getKeySize() const=0;

        /**
         * Returns a secret or private key to use for signing or decryption operations.
         * 
         * @return  a secret or private key
         */
        virtual XSECCryptoKey* getPrivateKey() const=0;

        /**
         * Returns a secret or public key to use for verification or encryption operations.
         * 
         * @return  a secret or public key
         */
        virtual XSECCryptoKey* getPublicKey() const=0;
        
        /**
         * Returns names representing the Credential.
         *
         * <p>Names should be unique in the context of the comparisons against CredentialCriteria
         * that deployments expect to see.
         * 
         * @return  a sorted set of names
         */
        virtual const std::set<std::string>& getKeyNames() const=0;
        
        /**
         * Returns a ds:KeyInfo object representing the Credential for use in
         * communicating with other entities.
         * 
         * @param compact   true iff the communication medium is such that only compact forms should be included
         * @return a KeyInfo object, which must be freed by the caller
         */
        virtual xmlsignature::KeyInfo* getKeyInfo(bool compact=false) const=0;

        /**
         * Get the credential context information, which provides additional information
         * specific to the context in which the credential was resolved.
         * 
         * @return resolution context of the credential
         */
        virtual const CredentialContext* getCredentalContext() const {
            return NULL;
        }
    };
};

#endif /* __xmltooling_cred_h__ */
