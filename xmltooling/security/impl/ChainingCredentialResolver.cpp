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
 * ChainingCredentialResolver.cpp
 * 
 * CredentialResolver with chaining capability.
 */

#include "internal.h"
#include "logging.h"
#include "XMLToolingConfig.h"
#include "security/CredentialResolver.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

using xercesc::DOMElement;

namespace xmltooling {
    class XMLTOOL_DLLLOCAL ChainingCredentialResolver : public CredentialResolver
    {
    public:
        ChainingCredentialResolver(const DOMElement* e);
        virtual ~ChainingCredentialResolver() {
            for_each(m_resolvers.begin(), m_resolvers.end(), xmltooling::cleanup<CredentialResolver>());
        }

        Lockable* lock() {
            for_each(m_resolvers.begin(), m_resolvers.end(), mem_fun(&Lockable::lock));
            return this;
        }
        void unlock() {
            for_each(m_resolvers.begin(), m_resolvers.end(), mem_fun(&Lockable::unlock));
        }
        
        const Credential* resolve(const CredentialCriteria* criteria=NULL) const {
            const Credential* cred = NULL;
            for (vector<CredentialResolver*>::const_iterator cr = m_resolvers.begin(); !cred && cr!=m_resolvers.end(); ++cr)
                cred = (*cr)->resolve(criteria);
            return cred;
        }

        virtual vector<const Credential*>::size_type resolve(
            vector<const Credential*>& results, const CredentialCriteria* criteria=NULL
            ) const {
            for (vector<CredentialResolver*>::const_iterator cr = m_resolvers.begin(); cr!=m_resolvers.end(); ++cr)
                (*cr)->resolve(results, criteria);
            return results.size();
        }

    private:
        vector<CredentialResolver*> m_resolvers;
    };

    CredentialResolver* XMLTOOL_DLLLOCAL ChainingCredentialResolverFactory(const DOMElement* const & e)
    {
        return new ChainingCredentialResolver(e);
    }

    static const XMLCh _CredentialResolver[] =  UNICODE_LITERAL_18(C,r,e,d,e,n,t,i,a,l,R,e,s,o,l,v,e,r);
    static const XMLCh _type[] =                UNICODE_LITERAL_4(t,y,p,e);
};

ChainingCredentialResolver::ChainingCredentialResolver(const DOMElement* e)
{
    XMLToolingConfig& conf = XMLToolingConfig::getConfig();
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."CHAINING_CREDENTIAL_RESOLVER);

    // Load up the chain of resolvers.
    e = e ? XMLHelper::getFirstChildElement(e, _CredentialResolver) : NULL;
    while (e) {
        auto_ptr_char type(e->getAttributeNS(NULL,_type));
        if (type.get() && *(type.get())) {
            log.info("building CredentialResolver of type %s", type.get());
            try {
                m_resolvers.push_back(conf.CredentialResolverManager.newPlugin(type.get(),e));
            }
            catch (exception& ex) {
                log.error("caught exception processing embedded CredentialResolver element: %s", ex.what());
            }
        }
        e = XMLHelper::getNextSiblingElement(e, _CredentialResolver);
    }
}
