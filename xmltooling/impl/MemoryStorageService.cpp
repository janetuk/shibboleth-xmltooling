/*
 *  Copyright 2001-2005 Internet2
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
 * MemoryStorageService.cpp
 * 
 * In-memory "persistent" storage, suitable for simple applications.
 */

#include "internal.h"
#include "util/NDC.h"
#include "util/StorageService.h"
#include "util/Threads.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {
    class XMLTOOL_DLLLOCAL MemoryStorageService : public StorageService
    {
    public:
        MemoryStorageService(const DOMElement* e);
        virtual ~MemoryStorageService();
        
        void createString(const char* key, const char* value, time_t expiration);
        bool readString(const char* key, string& value, time_t modifiedSince=0);
        bool updateString(const char* key, const char* value=NULL, time_t expiration=0);
        bool deleteString(const char* key);
        
        void createText(const char* key, const char* value, time_t expiration) {
            return createString(key, value, expiration);
        }
        bool readText(const char* key, string& value, time_t modifiedSince=0) {
            return readString(key, value, modifiedSince);
        }
        bool updateText(const char* key, const char* value=NULL, time_t expiration=0) {
            return updateString(key, value, expiration);
        }
        bool deleteText(const char* key) {
            return deleteString(key);
        }
        
        void reap() {
            shutdown_wait->signal();
        }

    private:
        void cleanup();
    
        struct XMLTOOL_DLLLOCAL Record {
            Record() : modified(0), expiration(0) {}
            Record(string s, time_t t1, time_t t2) : data(s), modified(t1), expiration(t2) {}
            string data;
            time_t modified, expiration;
        };
        
        map<string,Record> m_dataMap;
        multimap<time_t,string> m_expMap;
        RWLock* m_lock;
        CondWait* shutdown_wait;
        Thread* cleanup_thread;
        static void* cleanup_fn(void*);
        bool shutdown;
        int m_cleanupInterval;
        Category& m_log;
    };

    StorageService* XMLTOOL_DLLLOCAL MemoryStorageServiceFactory(const DOMElement* const & e)
    {
        return new MemoryStorageService(e);
    }

};

static const XMLCh cleanupInterval[] = UNICODE_LITERAL_15(c,l,e,a,n,u,p,I,n,t,e,r,v,a,l);

MemoryStorageService::MemoryStorageService(const DOMElement* e)
    : m_lock(NULL), shutdown_wait(NULL), cleanup_thread(NULL), shutdown(false), m_cleanupInterval(0),
        m_log(Category::getInstance(XMLTOOLING_LOGCAT".StorageService"))
{
    m_lock = RWLock::create();
    shutdown_wait = CondWait::create();
    cleanup_thread = Thread::create(&cleanup_fn, (void*)this);

    const XMLCh* tag=e ? e->getAttributeNS(NULL,cleanupInterval) : NULL;
    if (tag && *tag) {
        m_cleanupInterval = XMLString::parseInt(tag);
    }
    if (!m_cleanupInterval)
        m_cleanupInterval=300;
}

MemoryStorageService::~MemoryStorageService()
{
    // Shut down the cleanup thread and let it know...
    shutdown = true;
    shutdown_wait->signal();
    cleanup_thread->join(NULL);

    delete m_lock;
    delete shutdown_wait;
}

void* MemoryStorageService::cleanup_fn(void* cache_p)
{
    MemoryStorageService* cache = reinterpret_cast<MemoryStorageService*>(cache_p);

#ifndef WIN32
    // First, let's block all signals 
    Thread::mask_all_signals();
#endif

    // Now run the cleanup process.
    cache->cleanup();
    return NULL;
}

void MemoryStorageService::cleanup()
{
#ifdef _DEBUG
    NDC ndc("cleanup");
#endif
    

    Mutex* mutex = Mutex::create();
    mutex->lock();

    m_log.info("cleanup thread started...running every %d seconds", m_cleanupInterval);

    while (!shutdown) {
        shutdown_wait->timedwait(mutex, m_cleanupInterval);
        if (shutdown)
            break;

        // Lock the "database".
        m_lock->wrlock();
        
        // Garbage collect any expired entries.
        unsigned int count=0;
        time_t now=time(NULL)-XMLToolingConfig::getConfig().clock_skew_secs;
        multimap<time_t,string>::iterator stop=m_expMap.upper_bound(now);
        for (multimap<time_t,string>::iterator i=m_expMap.begin(); i!=stop; m_expMap.erase(i++)) {
            m_dataMap.erase(i->second);
            ++count;
        }
        
        m_lock->unlock();
        
        if (count)
            m_log.info("purged %d record(s) from storage", count);
    }

    m_log.info("cleanup thread finished");

    mutex->unlock();
    delete mutex;
    Thread::exit(NULL);
}

void MemoryStorageService::createString(const char* key, const char* value, time_t expiration)
{
    // Lock the maps.
    m_lock->wrlock();
    SharedLock wrapper(m_lock, false);
    
    // Check for a duplicate.
    map<string,Record>::iterator i=m_dataMap.find(key);
    if (i!=m_dataMap.end())
        throw IOException("attempted to insert a record with duplicate key ($1)", params(1,key));
    
    m_dataMap[key]=Record(value,time(NULL),expiration);
    m_expMap.insert(multimap<time_t,string>::value_type(expiration,key));
    
    m_log.debug("inserted record (%s)", key);
}

bool MemoryStorageService::readString(const char* key, string& value, time_t modifiedSince)
{
    SharedLock wrapper(m_lock);
    map<string,Record>::iterator i=m_dataMap.find(key);
    if (i==m_dataMap.end())
        return false;
    else if (modifiedSince >= i->second.modified)
        return false;
    value = i->second.data;
    return true;
}

bool MemoryStorageService::updateString(const char* key, const char* value, time_t expiration)
{
    // Lock the maps.
    m_lock->wrlock();
    SharedLock wrapper(m_lock, false);

    map<string,Record>::iterator i=m_dataMap.find(key);
    if (i==m_dataMap.end())
        return false;
        
    if (value)
        i->second.data = value;
        
    if (expiration && expiration != i->second.expiration) {
        // Update secondary map.
        pair<multimap<time_t,string>::iterator,multimap<time_t,string>::iterator> range=m_expMap.equal_range(i->second.expiration);
        for (; range.first != range.second; ++range.first) {
            if (range.first->second == i->first) {
                m_expMap.erase(range.first);
                break;
            }
        }
        i->second.expiration = expiration;
        m_expMap.insert(multimap<time_t,string>::value_type(expiration,key));
    }

    i->second.modified = time(NULL);
    m_log.debug("updated record (%s)", key);
    return true;
}

bool MemoryStorageService::deleteString(const char* key)
{
    // Lock the maps.
    m_lock->wrlock();
    SharedLock wrapper(m_lock, false);
    
    // Find the record.
    map<string,Record>::iterator i=m_dataMap.find(key);
    if (i!=m_dataMap.end()) {
        // Now find the reversed index of expiration to key, so we can clear it.
        pair<multimap<time_t,string>::iterator,multimap<time_t,string>::iterator> range=m_expMap.equal_range(i->second.expiration);
        for (; range.first != range.second; ++range.first) {
            if (range.first->second == i->first) {
                m_expMap.erase(range.first);
                break;
            }
        }
        // And finally delete the record itself.
        m_dataMap.erase(i);
        m_log.debug("deleted record (%s)", key);
        return true;
    }

    m_log.debug("deleting record (%s)....not found", key);
    return false;
}
