/*
 *  Copyright 2001-2006 Internet2
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
#include "security/TrustEngine.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace std;

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<TrustEngine,const DOMElement*>::Factory ExplicitKeyTrustEngineFactory;
    XMLTOOL_DLLLOCAL PluginManager<TrustEngine,const DOMElement*>::Factory ChainingTrustEngineFactory;
};

void XMLTOOL_API xmltooling::registerTrustEngines()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.TrustEngineManager.registerFactory(EXPLICIT_KEY_TRUSTENGINE, ExplicitKeyTrustEngineFactory);
    conf.TrustEngineManager.registerFactory(CHAINING_TRUSTENGINE, ChainingTrustEngineFactory);
}

static const XMLCh GenericKeyResolver[] =           UNICODE_LITERAL_11(K,e,y,R,e,s,o,l,v,e,r);
static const XMLCh type[] =                         UNICODE_LITERAL_4(t,y,p,e);

TrustEngine::TrustEngine(const DOMElement* e) : m_keyResolver(NULL)
{
    DOMElement* child = e ? XMLHelper::getFirstChildElement(e,GenericKeyResolver) : NULL;
    if (child) {
        auto_ptr_char t(child->getAttributeNS(NULL,type));
        if (t.get())
            m_keyResolver = XMLToolingConfig::getConfig().KeyResolverManager.newPlugin(t.get(),child);
        else
            throw UnknownExtensionException("<KeyResolver> element found with no type attribute");
    }
    else if (!m_keyResolver) {
        m_keyResolver = XMLToolingConfig::getConfig().KeyResolverManager.newPlugin(INLINE_KEY_RESOLVER, child);
    }
}

TrustEngine::~TrustEngine()
{
    delete m_keyResolver;
}
