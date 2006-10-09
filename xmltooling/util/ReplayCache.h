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
 * @file xmltooling/util/ReplayCache.h
 * 
 * Helper class on top of StorageService for detecting message replay.
 */

#ifndef __xmltooling_replay_h__
#define __xmltooling_replay_h__

#include <xmltooling/util/StorageService.h>

namespace xmltooling {

    /**
     * Helper class on top of StorageService for detecting message replay.
     */
    class XMLTOOL_API ReplayCache
    {
        MAKE_NONCOPYABLE(ReplayCache);
    public:
        
        /**
         * Creates a replay cache on top of a particular StorageService.
         * 
         * @param storage       pointer to a StorageService, or NULL to keep cache in memory
         */
        ReplayCache(StorageService* storage=NULL);

        virtual ~ReplayCache();
        
        /**
         * Returns true iff the check value is not found in the cache, and stores it.
         * 
         * @param context   a context label to subdivide the cache
         * @param s         value to check
         * @param expires   time for disposal of value from cache
         */
        virtual bool check(const char* context, const char* s, time_t expires);
    
        bool check(const char* context, const XMLCh* str, time_t expires) {
            auto_ptr_char temp(str);
            return check(context, temp.get(), expires);
        }
        
    private:
        StorageService* m_storage;
    };
};

#endif /* __xmltooling_replay_h__ */
