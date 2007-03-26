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
 * @file xmltooling/security/CredentialResolver.h
 * 
 * An API for resolving keys and certificates based on application criteria.
 */

#if !defined(__xmltooling_credres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_credres_h__

#include <xmltooling/Lockable.h>

namespace xmltooling {

    class XMLTOOL_API Credential;
    class XMLTOOL_API CredentialCriteria;

    /**
     * An API for resolving keys and certificates based on application criteria.
     */
    class XMLTOOL_API CredentialResolver : public virtual Lockable
    {
        MAKE_NONCOPYABLE(CredentialResolver);
    protected:
        CredentialResolver() {}
        
    public:
        virtual ~CredentialResolver() {}
        
        /**
         * Returns a single Credential according to the supplied criteria.
         * 
         * @param criteria   an optional CredentialCriteria object 
         * @return  a Credential, or NULL if none could be found
         */
        virtual const Credential* resolve(const CredentialCriteria* criteria=NULL) const=0;

        /**
         * Returns all matching Credentials according to the supplied criteria.
         * 
         * @param results   array to store matching Credentials
         * @param criteria  an optional CredentialCriteria object 
         * @return  number of credentials found
         */
        virtual std::vector<const Credential*>::size_type resolve(
            std::vector<const Credential*>& results, const CredentialCriteria* criteria=NULL
            ) const=0;
    };

    /**
     * Registers CredentialResolver classes into the runtime.
     */
    void XMLTOOL_API registerCredentialResolvers();

    /** CredentialResolver based on local files with no criteria support. */
    #define FILESYSTEM_CREDENTIAL_RESOLVER  "File"
};

#endif /* __xmltooling_credres_h__ */
