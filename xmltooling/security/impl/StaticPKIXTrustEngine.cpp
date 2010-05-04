/*
 *  Copyright 2001-2010 Internet2
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
 * PKIXTrustEngine.cpp
 * 
 * Shibboleth-specific PKIX-validation TrustEngine.
 */

#include "internal.h"

#include "logging.h"
#include "XMLToolingConfig.h"
#include "security/AbstractPKIXTrustEngine.h"
#include "security/CredentialResolver.h"
#include "security/X509Credential.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

namespace xmltooling {

    static const XMLCh _CredentialResolver[] =  UNICODE_LITERAL_18(C,r,e,d,e,n,t,i,a,l,R,e,s,o,l,v,e,r);
    static const XMLCh type[] =                 UNICODE_LITERAL_4(t,y,p,e);
    static const XMLCh certificate[] =          UNICODE_LITERAL_11(c,e,r,t,i,f,i,c,a,t,e);
    static const XMLCh Certificate[] =          UNICODE_LITERAL_11(C,e,r,t,i,f,i,c,a,t,e);
    static const XMLCh Path[] =                 UNICODE_LITERAL_4(P,a,t,h);
    static const XMLCh verifyDepth[] =          UNICODE_LITERAL_11(v,e,r,i,f,y,D,e,p,t,h);

    class XMLTOOL_DLLLOCAL StaticPKIXTrustEngine : public AbstractPKIXTrustEngine
    {
    public:
        StaticPKIXTrustEngine(const DOMElement* e=nullptr);

        virtual ~StaticPKIXTrustEngine() {
            delete m_credResolver;
        }
        
        AbstractPKIXTrustEngine::PKIXValidationInfoIterator* getPKIXValidationInfoIterator(
            const CredentialResolver& pkixSource, CredentialCriteria* criteria=nullptr
            ) const;

        const KeyInfoResolver* getKeyInfoResolver() const {
            return m_keyInfoResolver ? m_keyInfoResolver : XMLToolingConfig::getConfig().getKeyInfoResolver();
        }

    private:
        int m_depth;
        CredentialResolver* m_credResolver;
        friend class XMLTOOL_DLLLOCAL StaticPKIXIterator;
    };
    
    TrustEngine* XMLTOOL_DLLLOCAL StaticPKIXTrustEngineFactory(const DOMElement* const & e)
    {
        return new StaticPKIXTrustEngine(e);
    }

    class XMLTOOL_DLLLOCAL StaticPKIXIterator : public AbstractPKIXTrustEngine::PKIXValidationInfoIterator
    {
    public:
        StaticPKIXIterator(const StaticPKIXTrustEngine& engine) : m_engine(engine), m_done(false) {
            // Merge together all X509Credentials we can resolve.
            m_engine.m_credResolver->lock();
            try {
                vector<const Credential*> creds;
                m_engine.m_credResolver->resolve(creds);
                for (vector<const Credential*>::const_iterator i = creds.begin(); i != creds.end(); ++i) {
                    const X509Credential* xcred = dynamic_cast<const X509Credential*>(*i);
                    if (xcred) {
                        m_certs.insert(m_certs.end(), xcred->getEntityCertificateChain().begin(), xcred->getEntityCertificateChain().end());
                        m_crls.insert(m_crls.end(), xcred->getCRLs().begin(), xcred->getCRLs().end());
                    }
                }
            }
            catch (exception& ex) {
                logging::Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.StaticPKIX").error(ex.what());
            }
        }

        virtual ~StaticPKIXIterator() {
            m_engine.m_credResolver->unlock();
        }

        bool next() {
            if (m_done)
                return false;
            m_done = true;
            return true;
        }

        int getVerificationDepth() const {
            return m_engine.m_depth;
        }
        
        const vector<XSECCryptoX509*>& getTrustAnchors() const {
            return m_certs;
        }

        const vector<XSECCryptoX509CRL*>& getCRLs() const {
            return m_crls;
        }
    
    private:
        const StaticPKIXTrustEngine& m_engine;
        vector<XSECCryptoX509*> m_certs;
        vector<XSECCryptoX509CRL*> m_crls;
        bool m_done;
    };
};

StaticPKIXTrustEngine::StaticPKIXTrustEngine(const DOMElement* e) : AbstractPKIXTrustEngine(e), m_depth(1), m_credResolver(nullptr)
{
    const XMLCh* depth = e ? e->getAttributeNS(nullptr, verifyDepth) : nullptr;
    if (depth && *depth)
        m_depth = XMLString::parseInt(depth);

    if (e && e->hasAttributeNS(nullptr,certificate)) {
        // Simple File resolver config rooted here.
        m_credResolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(FILESYSTEM_CREDENTIAL_RESOLVER,e);
    }
    else {
        e = e ? XMLHelper::getFirstChildElement(e, _CredentialResolver) : nullptr;
        auto_ptr_char t(e ? e->getAttributeNS(nullptr,type) : nullptr);
        if (t.get()) {
            m_credResolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(t.get(),e);
        }
        else
            throw XMLSecurityException("Missing <CredentialResolver> element, or no type attribute found");
    }
}

AbstractPKIXTrustEngine::PKIXValidationInfoIterator* StaticPKIXTrustEngine::getPKIXValidationInfoIterator(
    const CredentialResolver& pkixSource, CredentialCriteria* criteria
    ) const
{
    return new StaticPKIXIterator(*this);
}
