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

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
};

namespace xmltooling {

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
        
        enum ResolveTypes {
            RESOLVE_KEYS = 1
        };

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
         * Returns names representing the Credential, generally when the Credential itself merely
         * points to a Credential rather than containing one.
         * 
         * @param results   array to populate with names
         * @return  the number of names returned
         */
        virtual std::vector<std::string>::size_type getKeyNames(std::vector<std::string>& results) const=0;
        
        /**
         * Returns a ds:KeyInfo object representing the Credential for use in
         * communicating with other entities.
         * 
         * @param compact   true iff the communication medium is such that only compact forms should be included
         * @return reference to a KeyInfo object
         */
        virtual const xmlsignature::KeyInfo* getKeyInfo(bool compact=false) const=0;

        /**
         * Compares the public key inside the Credential to a second public key.
         *
         * @param key   the public key to compare
         * @return true iff the keys are equal
         */
        virtual bool isEqual(XSECCryptoKey& key) const;
    };
};

#endif /* __xmltooling_cred_h__ */
