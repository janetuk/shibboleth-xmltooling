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
 * TrustEngine.cpp
 * 
 * Registration of factories for built-in engines
 */

#include "internal.h"
#include "security/KeyInfoResolver.h"
#include "security/SignatureTrustEngine.h"
#include "security/OpenSSLTrustEngine.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace std;

using xercesc::DOMElement;

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<TrustEngine,string,const DOMElement*>::Factory ExplicitKeyTrustEngineFactory;
    XMLTOOL_DLLLOCAL PluginManager<TrustEngine,string,const DOMElement*>::Factory StaticPKIXTrustEngineFactory;
    XMLTOOL_DLLLOCAL PluginManager<TrustEngine,string,const DOMElement*>::Factory ChainingTrustEngineFactory;
};

void XMLTOOL_API xmltooling::registerTrustEngines()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.TrustEngineManager.registerFactory(EXPLICIT_KEY_TRUSTENGINE, ExplicitKeyTrustEngineFactory);
    conf.TrustEngineManager.registerFactory(STATIC_PKIX_TRUSTENGINE, StaticPKIXTrustEngineFactory);
    conf.TrustEngineManager.registerFactory(CHAINING_TRUSTENGINE, ChainingTrustEngineFactory);
}

static const XMLCh _KeyInfoResolver[] = UNICODE_LITERAL_15(K,e,y,I,n,f,o,R,e,s,o,l,v,e,r);
static const XMLCh type[] =             UNICODE_LITERAL_4(t,y,p,e);

TrustEngine::TrustEngine(const DOMElement* e) : m_keyInfoResolver(NULL)
{
    DOMElement* child = e ? XMLHelper::getFirstChildElement(e,_KeyInfoResolver) : NULL;
    if (child) {
        auto_ptr_char t(child->getAttributeNS(NULL,type));
        if (t.get())
            m_keyInfoResolver = XMLToolingConfig::getConfig().KeyInfoResolverManager.newPlugin(t.get(),child);
        else
            throw UnknownExtensionException("<KeyInfoResolver> element found with no type attribute");
    }
}

TrustEngine::~TrustEngine()
{
    delete m_keyInfoResolver;
}

SignatureTrustEngine::SignatureTrustEngine(const DOMElement* e) : TrustEngine(e)
{
}

SignatureTrustEngine::~SignatureTrustEngine()
{
}

X509TrustEngine::X509TrustEngine(const DOMElement* e) : TrustEngine(e)
{
}

X509TrustEngine::~X509TrustEngine()
{
}

OpenSSLTrustEngine::OpenSSLTrustEngine(const DOMElement* e) : X509TrustEngine(e)
{
}

OpenSSLTrustEngine::~OpenSSLTrustEngine()
{
}
