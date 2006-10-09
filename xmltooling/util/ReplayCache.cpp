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
 * ReplayCache.cpp
 * 
 * Helper class on top of StorageService for detecting message replay. 
 */

#include "internal.h"
#include "util/ReplayCache.h"

using namespace xmltooling;
using namespace std;

ReplayCache::ReplayCache(StorageService* storage) : m_storage(storage)
{
    if (!m_storage)
        m_storage = XMLToolingConfig::getConfig().StorageServiceManager.newPlugin(MEMORY_STORAGE_SERVICE, NULL);
}

ReplayCache::~ReplayCache()
{
    delete m_storage;
}

bool ReplayCache::check(const char* context, const char* s, time_t expires)
{
    // In storage already?
    if (m_storage->readString(context, s))
        return false;
    m_storage->createText(context, s, "x", expires);
    return true;
}
