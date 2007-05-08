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
 * ChainingTrustEngine.cpp
 * 
 * TrustEngine that uses multiple engines in sequence.
 */

#include "internal.h"
#include "exceptions.h"
#include "security/ChainingTrustEngine.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {
    TrustEngine* XMLTOOL_DLLLOCAL ChainingTrustEngineFactory(const DOMElement* const & e)
    {
        return new ChainingTrustEngine(e);
    }
};

static const XMLCh _TrustEngine[] =                 UNICODE_LITERAL_11(T,r,u,s,t,E,n,g,i,n,e);
static const XMLCh type[] =                         UNICODE_LITERAL_4(t,y,p,e);

ChainingTrustEngine::ChainingTrustEngine(const DOMElement* e) : OpenSSLTrustEngine(e) {
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    e = e ? XMLHelper::getFirstChildElement(e, _TrustEngine) : NULL;
    while (e) {
        try {
            auto_ptr_char temp(e->getAttributeNS(NULL,type));
            if (temp.get() && *temp.get()) {
                log.info("building TrustEngine of type %s", temp.get());
                TrustEngine* engine = XMLToolingConfig::getConfig().TrustEngineManager.newPlugin(temp.get(), e);
                m_engines.push_back(engine);
            }
        }
        catch (exception& ex) {
            log.error("error building TrustEngine: %s", ex.what());
        }
        e = XMLHelper::getNextSiblingElement(e, _TrustEngine);
    }
}

ChainingTrustEngine::~ChainingTrustEngine() {
    for_each(m_engines.begin(), m_engines.end(), xmltooling::cleanup<TrustEngine>());
}

bool ChainingTrustEngine::validate(Signature& sig, const CredentialResolver& credResolver, CredentialCriteria* criteria) const
{
    for (vector<TrustEngine*>::const_iterator i=m_engines.begin(); i!=m_engines.end(); ++i) {
        if ((*i)->validate(sig,credResolver,criteria))
            return true;
    }
    return false;
}

bool ChainingTrustEngine::validate(
    const XMLCh* sigAlgorithm,
    const char* sig,
    KeyInfo* keyInfo,
    const char* in,
    unsigned int in_len,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
    for (vector<TrustEngine*>::const_iterator i=m_engines.begin(); i!=m_engines.end(); ++i) {
        if ((*i)->validate(sigAlgorithm, sig, keyInfo, in, in_len, credResolver, criteria))
            return true;
    }
    return false;
}

bool ChainingTrustEngine::validate(
    XSECCryptoX509* certEE,
    const vector<XSECCryptoX509*>& certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
    X509TrustEngine* down;
    for (vector<TrustEngine*>::const_iterator i=m_engines.begin(); i!=m_engines.end(); ++i) {
        if ((down = dynamic_cast<X509TrustEngine*>(*i)) &&
                down->validate(certEE,certChain,credResolver,criteria))
            return true;
    }
    return false;
}

bool ChainingTrustEngine::validate(
    X509* certEE,
    STACK_OF(X509)* certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
    OpenSSLTrustEngine* down;
    for (vector<TrustEngine*>::const_iterator i=m_engines.begin(); i!=m_engines.end(); ++i) {
        if ((down = dynamic_cast<OpenSSLTrustEngine*>(*i)) && down->validate(certEE,certChain,credResolver,criteria))
            return true;
    }
    return false;
}
