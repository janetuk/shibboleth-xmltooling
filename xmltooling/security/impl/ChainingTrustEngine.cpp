/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * ChainingTrustEngine.cpp
 * 
 * OpenSSLTrustEngine that uses multiple engines in sequence.
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "security/ChainingTrustEngine.h"
#include "security/CredentialCriteria.h"
#include "util/XMLHelper.h"

#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmlsignature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace boost::lambda;
using namespace boost;
using namespace std;

using xercesc::DOMElement;

namespace xmltooling {
    TrustEngine* XMLTOOL_DLLLOCAL ChainingTrustEngineFactory(const DOMElement* const & e)
    {
        return new ChainingTrustEngine(e);
    }
};

static const XMLCh _TrustEngine[] =                 UNICODE_LITERAL_11(T,r,u,s,t,E,n,g,i,n,e);
static const XMLCh type[] =                         UNICODE_LITERAL_4(t,y,p,e);

ChainingTrustEngine::ChainingTrustEngine(const DOMElement* e) : TrustEngine(e)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine."CHAINING_TRUSTENGINE);
    e = e ? XMLHelper::getFirstChildElement(e, _TrustEngine) : nullptr;
    while (e) {
        try {
            string t = XMLHelper::getAttrString(e, nullptr, type);
            if (!t.empty()) {
                log.info("building TrustEngine of type %s", t.c_str());
                addTrustEngine(XMLToolingConfig::getConfig().TrustEngineManager.newPlugin(t.c_str(), e));
            }
        }
        catch (exception& ex) {
            log.error("error building TrustEngine: %s", ex.what());
        }
        e = XMLHelper::getNextSiblingElement(e, _TrustEngine);
    }
}

ChainingTrustEngine::~ChainingTrustEngine()
{
}

void ChainingTrustEngine::addTrustEngine(TrustEngine* newEngine)
{
    m_engines.push_back(newEngine);
    SignatureTrustEngine* sig = dynamic_cast<SignatureTrustEngine*>(newEngine);
    if (sig)
        m_sigEngines.push_back(sig);
    X509TrustEngine* x509 = dynamic_cast<X509TrustEngine*>(newEngine);
    if (x509)
        m_x509Engines.push_back(x509);
    OpenSSLTrustEngine* ossl = dynamic_cast<OpenSSLTrustEngine*>(newEngine);
    if (ossl)
        m_osslEngines.push_back(ossl);
}

TrustEngine* ChainingTrustEngine::removeTrustEngine(TrustEngine* oldEngine)
{
    ptr_vector<TrustEngine>::iterator i =
        find_if(m_engines.begin(), m_engines.end(), (&_1 == oldEngine));
    if (i != m_engines.end()) {
        SignatureTrustEngine* sig = dynamic_cast<SignatureTrustEngine*>(oldEngine);
        if (sig) {
            ptr_vector<SignatureTrustEngine>::iterator s =
                find_if(m_sigEngines.begin(), m_sigEngines.end(), (&_1 == sig));
            if (s != m_sigEngines.end())
                m_sigEngines.erase(s);
        }

        X509TrustEngine* x509 = dynamic_cast<X509TrustEngine*>(oldEngine);
        if (x509) {
            ptr_vector<X509TrustEngine>::iterator x =
                find_if(m_x509Engines.begin(), m_x509Engines.end(), (&_1 == x509));
            if (x != m_x509Engines.end())
                m_x509Engines.erase(x);
        }

        OpenSSLTrustEngine* ossl = dynamic_cast<OpenSSLTrustEngine*>(oldEngine);
        if (ossl) {
            ptr_vector<OpenSSLTrustEngine>::iterator o =
                find_if(m_osslEngines.begin(), m_osslEngines.end(), (&_1 == ossl));
            if (o != m_osslEngines.end())
                m_osslEngines.erase(o);
        }

        return (m_engines.release(i)).release();
    }
    return nullptr;
}

bool ChainingTrustEngine::validate(Signature& sig, const CredentialResolver& credResolver, CredentialCriteria* criteria) const
{
    unsigned int usage = criteria ? criteria->getUsage() : 0;
    for (ptr_vector<SignatureTrustEngine>::const_iterator i=m_sigEngines.begin(); i!=m_sigEngines.end(); ++i) {
        if (i->validate(sig,credResolver,criteria))
            return true;
        if (criteria) {
            criteria->reset();
            criteria->setUsage(usage);
        }
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
    unsigned int usage = criteria ? criteria->getUsage() : 0;
    for (ptr_vector<SignatureTrustEngine>::const_iterator i=m_sigEngines.begin(); i!=m_sigEngines.end(); ++i) {
        if (i->validate(sigAlgorithm, sig, keyInfo, in, in_len, credResolver, criteria))
            return true;
        if (criteria) {
            criteria->reset();
            criteria->setUsage(usage);
        }
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
    unsigned int usage = criteria ? criteria->getUsage() : 0;
    for (ptr_vector<X509TrustEngine>::const_iterator i=m_x509Engines.begin(); i!=m_x509Engines.end(); ++i) {
        if (i->validate(certEE,certChain,credResolver,criteria))
            return true;
        if (criteria) {
            criteria->reset();
            criteria->setUsage(usage);
        }
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
    unsigned int usage = criteria ? criteria->getUsage() : 0;
    for (ptr_vector<OpenSSLTrustEngine>::const_iterator i=m_osslEngines.begin(); i!=m_osslEngines.end(); ++i) {
        if (i->validate(certEE,certChain,credResolver,criteria))
            return true;
        if (criteria) {
            criteria->reset();
            criteria->setUsage(usage);
        }
    }
    return false;
}
