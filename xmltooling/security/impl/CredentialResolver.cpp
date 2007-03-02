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
 * CredentialResolver.cpp
 * 
 * Registration of factories for built-in resolvers
 */

#include "internal.h"
#include "security/CredentialResolver.h"

using namespace xmltooling;

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<CredentialResolver,const DOMElement*>::Factory FilesystemCredentialResolverFactory; 
};

void XMLTOOL_API xmltooling::registerCredentialResolvers()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.CredentialResolverManager.registerFactory(FILESYSTEM_CREDENTIAL_RESOLVER, FilesystemCredentialResolverFactory);
}
