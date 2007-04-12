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
 * @file xmltooling/security/KeyInfoResolver.h
 * 
 * Resolves credentials from KeyInfo information.
 */

#if !defined(__xmltooling_keyres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keyres_h__

#include <xsec/dsig/DSIGKeyInfoList.hpp>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
    class XMLTOOL_API Signature;
};

namespace xmltooling {

    class XMLTOOL_API Credential;
    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API KeyInfoCredentialContext;

    /**
     * Resolves credentials from KeyInfo information.
     *
     * <p>Credential-specific bitmasks can be provided to control what to resolve.
     *
     * <p>Implementations should only establish KeyNames on the basis of explicit names
     * within the KeyInfo object, never by extracting names out of physical credentials
     * found within it.
     */
    class XMLTOOL_API KeyInfoResolver {
        MAKE_NONCOPYABLE(KeyInfoResolver);
    protected:
        KeyInfoResolver() {}
    public:
        virtual ~KeyInfoResolver() {}
        
        /**
         * Returns a credential based on the supplied KeyInfo information.
         * The caller must release the credential when done with it.
         * 
         * @param keyInfo   the key information
         * @param types     types of credentials to resolve, or 0 for any/all
         * @return  the resolved credential, or NULL
         */
        virtual Credential* resolve(const xmlsignature::KeyInfo* keyInfo, int types=0) const=0;

        /**
         * Returns a credential based on the supplied KeyInfo information.
         * The caller must release the credential when done with it.
         * 
         * @param keyInfo   the key information
         * @param types     types of credentials to resolve, or 0 for any/all
         * @return  the resolved credential, or NULL
         */
        virtual Credential* resolve(DSIGKeyInfoList* keyInfo, int types=0) const=0;

        /**
         * Returns a credential based on the KeyInfo information in the supplied
         * context. The caller must release the credential when done with it.
         *
         * <p>The context object will be owned by the Credential and freed with it.
         * 
         * @param context   context containing the key information
         * @param types types of credentials to resolve, or 0 for any/all
         * @return  the resolved credential, or NULL
         */
        virtual Credential* resolve(KeyInfoCredentialContext* context, int types=0) const=0;

        /**
         * Returns a credential based on the supplied KeyInfo information.
         * The caller must release the credential when done with it.
         * 
         * @param sig   signature containing the key information
         * @param types types of credentials to resolve, or 0 for any/all
         * @return  the resolved credential, or NULL
         */
        Credential* resolve(const xmlsignature::Signature* sig, int types=0) const;

        /**
         * Returns a credential based on the KeyInfo information in the supplied
         * criteria. The caller must release the credential when done with it.
         * 
         * @param criteria   criteria containing the key information
         * @param types types of credentials to resolve, or 0 for any/all
         * @return  the resolved credential, or NULL
         */
        Credential* resolve(const CredentialCriteria& criteria, int types=0) const;
    };

    /**
     * Registers KeyInfoResolver classes into the runtime.
     */
    void XMLTOOL_API registerKeyInfoResolvers();

    /** KeyInfoResolver based on extracting by value directly out of a KeyInfo */
    #define INLINE_KEYINFO_RESOLVER  "Inline"
};

#endif /* __xmltooling_keyres_h__ */
