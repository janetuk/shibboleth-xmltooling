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
 * StorageService.cpp
 * 
 * Generic data storage interface.
 */

#include "internal.h"
#include "util/StorageService.h"

using namespace xmltooling;
using namespace std;

namespace {
    static const XMLTOOL_DLLLOCAL StorageService::Capabilities g_ssCaps(255, 255, 255);
};

namespace xmltooling {
    XMLTOOL_DLLLOCAL PluginManager<StorageService,string,const xercesc::DOMElement*>::Factory MemoryStorageServiceFactory; 
};

void XMLTOOL_API xmltooling::registerStorageServices()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.StorageServiceManager.registerFactory(MEMORY_STORAGE_SERVICE, MemoryStorageServiceFactory);
}

StorageService::StorageService()
{
}

StorageService::~StorageService()
{
}

const StorageService::Capabilities& StorageService::getCapabilities() const
{
    return g_ssCaps;
}

StorageService::Capabilities::Capabilities(unsigned int contextSize, unsigned int keySize, unsigned int stringSize)
    : m_contextSize(contextSize), m_keySize(keySize), m_stringSize(stringSize)
{
}

StorageService::Capabilities::~Capabilities()
{
}

unsigned int StorageService::Capabilities::getContextSize() const
{
    return m_contextSize;
}

unsigned int StorageService::Capabilities::getKeySize() const
{
    return m_keySize;
}

unsigned int StorageService::Capabilities::getStringSize() const
{
    return m_stringSize;
}
