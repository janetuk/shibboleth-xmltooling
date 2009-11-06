/*
 *  Copyright 2001-2009 Internet2
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
 * StorageService.cpp
 * 
 * Generic data storage interface.
 */

#include "internal.h"
#include "util/StorageService.h"

using namespace xmltooling;
using namespace std;

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
