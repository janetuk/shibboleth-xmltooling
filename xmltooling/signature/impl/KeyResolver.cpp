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
 * KeyResolver.cpp
 * 
 * Registration of factories for built-in resolvers
 */

#include "internal.h"
#include "signature/KeyResolver.h"

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

namespace xmlsignature {
    XMLTOOL_DLLLOCAL PluginManager<KeyResolver,const DOMElement*>::Factory FilesystemKeyResolverFactory;
    XMLTOOL_DLLLOCAL PluginManager<KeyResolver,const DOMElement*>::Factory InlineKeyResolverFactory;
};

void XMLTOOL_API xmlsignature::registerKeyResolvers()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.KeyResolverManager.registerFactory(FILESYSTEM_KEY_RESOLVER, FilesystemKeyResolverFactory);
    conf.KeyResolverManager.registerFactory(INLINE_KEY_RESOLVER, InlineKeyResolverFactory);
}

vector<XSECCryptoX509*>::size_type KeyResolver::resolveCertificates(
    const KeyInfo* keyInfo, vector<XSECCryptoX509*>& certs
    ) const
{
    return 0;
}

vector<XSECCryptoX509*>::size_type KeyResolver::resolveCertificates(
    DSIGKeyInfoList* keyInfo, vector<XSECCryptoX509*>& certs
    ) const
{
    return 0;
}

XSECCryptoX509CRL* KeyResolver::resolveCRL(const KeyInfo* keyInfo) const
{
    return NULL;
}

XSECCryptoX509CRL* KeyResolver::resolveCRL(DSIGKeyInfoList* keyInfo) const
{
    return NULL;
}
