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
 * KeyInfoResolver.cpp
 * 
 * Resolves credentials from KeyInfo information.
 */

#include "internal.h"
#include "security/CredentialCriteria.h"
#include "security/KeyInfoResolver.h"
#include "signature/Signature.h"

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<KeyInfoResolver,string,const xercesc::DOMElement*>::Factory InlineKeyInfoResolverFactory;
};

void XMLTOOL_API xmltooling::registerKeyInfoResolvers()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.KeyInfoResolverManager.registerFactory(INLINE_KEYINFO_RESOLVER, InlineKeyInfoResolverFactory);
}

KeyInfoResolver::KeyInfoResolver()
{
}

KeyInfoResolver::~KeyInfoResolver()
{
}

Credential* KeyInfoResolver::resolve(const Signature* sig, int types) const
{
    const KeyInfo* keyInfo = sig->getKeyInfo();
    if (keyInfo)
        return resolve(keyInfo, types);
    DSIGSignature* native = sig->getXMLSignature();
    return resolve(native ? native->getKeyInfoList() : (DSIGKeyInfoList*)nullptr, types);
}

Credential* KeyInfoResolver::resolve(const CredentialCriteria& criteria, int types) const
{
    const KeyInfo* keyInfo = criteria.getKeyInfo();
    if (keyInfo)
        return resolve(keyInfo, types);
    DSIGKeyInfoList* native = criteria.getNativeKeyInfo();
    return native ? resolve(native, types) : nullptr;
}
