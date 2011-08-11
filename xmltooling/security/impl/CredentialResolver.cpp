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
 * CredentialResolver.cpp
 * 
 * An API for resolving keys and certificates based on application criteria.
 */

#include "internal.h"
#include "security/CredentialResolver.h"

using namespace xmltooling;

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<CredentialResolver,std::string,const xercesc::DOMElement*>::Factory FilesystemCredentialResolverFactory;
    XMLTOOL_DLLLOCAL PluginManager<CredentialResolver,std::string,const xercesc::DOMElement*>::Factory DummyCredentialResolverFactory;
    XMLTOOL_DLLLOCAL PluginManager<CredentialResolver,std::string,const xercesc::DOMElement*>::Factory ChainingCredentialResolverFactory; 
};

void XMLTOOL_API xmltooling::registerCredentialResolvers()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.CredentialResolverManager.registerFactory(FILESYSTEM_CREDENTIAL_RESOLVER, FilesystemCredentialResolverFactory);
    conf.CredentialResolverManager.registerFactory(DUMMY_CREDENTIAL_RESOLVER, DummyCredentialResolverFactory);
    conf.CredentialResolverManager.registerFactory(CHAINING_CREDENTIAL_RESOLVER, ChainingCredentialResolverFactory);
}

CredentialResolver::CredentialResolver()
{
}

CredentialResolver::~CredentialResolver()
{
}
