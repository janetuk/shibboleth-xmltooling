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
        
        StorageHandle* createHandle();
        
        void createString(StorageHandle* handle, const char* key, const char* value, time_t expiration);
        bool readString(StorageHandle* handle, const char* key, string& value, time_t modifiedSince=0);
        bool updateString(StorageHandle* handle, const char* key, const char* value=NULL, time_t expiration=0);
        bool deleteString(StorageHandle* handle, const char* key);
        
        void createText(StorageHandle* handle, const char* key, const char* value, time_t expiration) {
            return createString(handle, key, value, expiration);
        }
        bool readText(StorageHandle* handle, const char* key, string& value, time_t modifiedSince=0) {
            return readString(handle, key, value, modifiedSince);
        }
        bool updateText(StorageHandle* handle, const char* key, const char* value=NULL, time_t expiration=0) {
            return updateString(handle, key, value, expiration);
        }
        bool deleteText(StorageHandle* handle, const char* key) {
            return deleteString(handle, key);
        }
        
        void reap(StorageHandle* handle);

        void removeHandle(StorageHandle* handle);

    private:
        void cleanup();
    
        struct XMLTOOL_DLLLOCAL Record {
            Record() : modified(0), expiration(0) {}
            Record(string s, time_t t1, time_t t2) : data(s), modified(t1), expiration(t2) {}
            string data;
            time_t modified, expiration;
        };
        
        struct XMLTOOL_DLLLOCAL MemoryHandle : public StorageHandle {
            MemoryHandle(StorageService* storage) : StorageHandle(storage), m_lock(RWLock::create()) {}
            virtual ~MemoryHandle() {
                delete m_lock;
                static_cast<MemoryStorageService*>(m_storage)->removeHandle(this);
            }
            map<string,Record> m_dataMap;
            multimap<time_t,string> m_expMap;
            RWLock* m_lock;
            unsigned long reap();
        };
        
        vector<MemoryHandle*> m_handles;
        Mutex* mutex;
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
    : mutex(NULL), shutdown_wait(NULL), cleanup_thread(NULL), shutdown(false), m_cleanupInterval(0),
        m_log(Category::getInstance(XMLTOOLING_LOGCAT".StorageService"))
{
    mutex = Mutex::create();
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

    delete shutdown_wait;
    delete mutex;
    for_each(m_handles.begin(), m_handles.end(), xmltooling::cleanup<MemoryHandle>());
}

StorageService::StorageHandle* MemoryStorageService::createHandle()
{
    Lock wrapper(mutex);
    MemoryHandle* ret = new MemoryHandle(this);
    m_handles.push_back(ret);
    return ret;
}

void MemoryStorageService::removeHandle(StorageHandle* handle)
{
    Lock wrapper(mutex);
    for (vector<MemoryHandle*>::iterator i=m_handles.begin(); i!=m_handles.end(); ++i) {
        if (*i == handle) {
            m_handles.erase(i);
            return;
        }
    }
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
        
        unsigned long count=0;
        for (vector<MemoryHandle*>::iterator i=m_handles.begin(); i!=m_handles.end(); ++i)
            count += (*i)->reap();
        
        if (count)
            m_log.info("purged %d record(s) from storage", count);
    }

    m_log.info("cleanup thread finished");

    mutex->unlock();
    delete mutex;
    Thread::exit(NULL);
}

void MemoryStorageService::reap(StorageHandle* handle)
{
    if (!isValid(handle))
        throw IOException("Invalid storage handle.");
    static_cast<MemoryHandle*>(handle)->reap();
}

unsigned long MemoryStorageService::MemoryHandle::reap()
{
    // Lock the "database".
    m_lock->wrlock();
    SharedLock wrapper(m_lock, false);
    
    // Garbage collect any expired entries.
    unsigned long count=0;
    time_t now=time(NULL)-XMLToolingConfig::getConfig().clock_skew_secs;
    multimap<time_t,string>::iterator stop=m_expMap.upper_bound(now);
    for (multimap<time_t,string>::iterator i=m_expMap.begin(); i!=stop; m_expMap.erase(i++)) {
        m_dataMap.erase(i->second);
        ++count;
    }

    return count;
}

void MemoryStorageService::createString(StorageHandle* handle, const char* key, const char* value, time_t expiration)
{
    if (!isValid(handle))
        throw IOException("Invalid storage handle.");
    MemoryHandle* h = static_cast<MemoryHandle*>(handle);

    // Lock the maps.
    h->m_lock->wrlock();
    SharedLock wrapper(h->m_lock, false);
    
    // Check for a duplicate.
    map<string,Record>::iterator i=h->m_dataMap.find(key);
    if (i!=h->m_dataMap.end())
        throw IOException("attempted to insert a record with duplicate key ($1)", params(1,key));
    
    h->m_dataMap[key]=Record(value,time(NULL),expiration);
    h->m_expMap.insert(multimap<time_t,string>::value_type(expiration,key));
    
    m_log.debug("inserted record (%s)", key);
}

bool MemoryStorageService::readString(StorageHandle* handle, const char* key, string& value, time_t modifiedSince)
{
    if (!isValid(handle))
        throw IOException("Invalid storage handle.");
    MemoryHandle* h = static_cast<MemoryHandle*>(handle);

    SharedLock wrapper(h->m_lock);
    map<string,Record>::iterator i=h->m_dataMap.find(key);
    if (i==h->m_dataMap.end())
        return false;
    else if (modifiedSince >= i->second.modified)
        return false;
    value = i->second.data;
    return true;
}

bool MemoryStorageService::updateString(StorageHandle* handle, const char* key, const char* value, time_t expiration)
{
    if (!isValid(handle))
        throw IOException("Invalid storage handle.");
    MemoryHandle* h = static_cast<MemoryHandle*>(handle);

    // Lock the maps.
    h->m_lock->wrlock();
    SharedLock wrapper(h->m_lock, false);

    map<string,Record>::iterator i=h->m_dataMap.find(key);
    if (i==h->m_dataMap.end())
        return false;
        
    if (value)
        i->second.data = value;
        
    if (expiration && expiration != i->second.expiration) {
        // Update secondary map.
        pair<multimap<time_t,string>::iterator,multimap<time_t,string>::iterator> range =
            h->m_expMap.equal_range(i->second.expiration);
        for (; range.first != range.second; ++range.first) {
            if (range.first->second == i->first) {
                h->m_expMap.erase(range.first);
                break;
            }
        }
        i->second.expiration = expiration;
       h->m_expMap.insert(multimap<time_t,string>::value_type(expiration,key));
    }

    i->second.modified = time(NULL);
    m_log.debug("updated record (%s)", key);
    return true;
}

bool MemoryStorageService::deleteString(StorageHandle* handle, const char* key)
{
    if (!isValid(handle))
        throw IOException("Invalid storage handle.");
    MemoryHandle* h = static_cast<MemoryHandle*>(handle);

    // Lock the maps.
    h->m_lock->wrlock();
    SharedLock wrapper(h->m_lock, false);
    
    // Find the record.
    map<string,Record>::iterator i=h->m_dataMap.find(key);
    if (i!=h->m_dataMap.end()) {
        // Now find the reversed index of expiration to key, so we can clear it.
        pair<multimap<time_t,string>::iterator,multimap<time_t,string>::iterator> range =
            h->m_expMap.equal_range(i->second.expiration);
        for (; range.first != range.second; ++range.first) {
            if (range.first->second == i->first) {
                h->m_expMap.erase(range.first);
                break;
            }
        }
        // And finally delete the record itself.
        h->m_dataMap.erase(i);
        m_log.debug("deleted record (%s)", key);
        return true;
    }

    m_log.debug("deleting record (%s)....not found", key);
    return false;
}
