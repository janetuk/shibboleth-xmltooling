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
        
        void createString(const char* context, const char* key, const char* value, time_t expiration);
        int readString(const char* context, const char* key, string* pvalue=NULL, time_t* pexpiration=NULL, int version=0);
        int updateString(const char* context, const char* key, const char* value=NULL, time_t expiration=0, int version=0);
        bool deleteString(const char* context, const char* key);
        
        void createText(const char* context, const char* key, const char* value, time_t expiration) {
            return createString(context, key, value, expiration);
        }
        int readText(const char* context, const char* key, string* pvalue=NULL, time_t* pexpiration=NULL, int version=0) {
            return readString(context, key, pvalue, pexpiration, version);
        }
        int updateText(const char* context, const char* key, const char* value=NULL, time_t expiration=0, int version=0) {
            return updateString(context, key, value, expiration, version);
        }
        bool deleteText(const char* context, const char* key) {
            return deleteString(context, key);
        }
        
        void reap(const char* context);
        void updateContext(const char* context, time_t expiration);
        void deleteContext(const char* context) {
            Lock wrapper(contextLock);
            m_contextMap.erase(context);
        }

    private:
        void cleanup();
    
        struct XMLTOOL_DLLLOCAL Record {
            Record() : expiration(0), version(1) {}
            Record(const string& s, time_t t) : data(s), expiration(t), version(1) {}
            string data;
            time_t expiration;
            int version;
        };
        
        struct XMLTOOL_DLLLOCAL Context {
            Context() : m_lock(RWLock::create()) {}
            Context(const Context& src) {
                m_dataMap = src.m_dataMap;
                m_lock = RWLock::create();
            }
            ~Context() { delete m_lock; }
            map<string,Record> m_dataMap;
            RWLock* m_lock;
            unsigned long reap(time_t exp);
        };

        Context& getContext(const char* context) {
            Lock wrapper(contextLock);
            return m_contextMap[context];
        }

        map<string,Context> m_contextMap;
        Mutex* contextLock;
        CondWait* shutdown_wait;
        Thread* cleanup_thread;
        static void* cleanup_fn(void*);
        bool shutdown;
        int m_cleanupInterval;
        Category& m_log;

        friend class _expcheck;
    };

    StorageService* XMLTOOL_DLLLOCAL MemoryStorageServiceFactory(const DOMElement* const & e)
    {
        return new MemoryStorageService(e);
    }
};

static const XMLCh cleanupInterval[] = UNICODE_LITERAL_15(c,l,e,a,n,u,p,I,n,t,e,r,v,a,l);

MemoryStorageService::MemoryStorageService(const DOMElement* e)
    : contextLock(NULL), shutdown_wait(NULL), cleanup_thread(NULL), shutdown(false), m_cleanupInterval(0),
        m_log(Category::getInstance(XMLTOOLING_LOGCAT".StorageService"))
{
    const XMLCh* tag=e ? e->getAttributeNS(NULL,cleanupInterval) : NULL;
    if (tag && *tag) {
        m_cleanupInterval = XMLString::parseInt(tag);
    }
    if (!m_cleanupInterval)
        m_cleanupInterval=900;

    contextLock = Mutex::create();
    shutdown_wait = CondWait::create();
    cleanup_thread = Thread::create(&cleanup_fn, (void*)this);
}

MemoryStorageService::~MemoryStorageService()
{
    // Shut down the cleanup thread and let it know...
    shutdown = true;
    shutdown_wait->signal();
    cleanup_thread->join(NULL);

    delete shutdown_wait;
    delete contextLock;
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

    auto_ptr<Mutex> mutex(Mutex::create());
    mutex->lock();

    m_log.info("cleanup thread started...running every %d seconds", m_cleanupInterval);

    while (!shutdown) {
        shutdown_wait->timedwait(mutex.get(), m_cleanupInterval);
        if (shutdown)
            break;
        
        unsigned long count=0;
        time_t now = time(NULL);
        Lock wrapper(contextLock);
        for (map<string,Context>::iterator i=m_contextMap.begin(); i!=m_contextMap.end(); ++i)
            count += i->second.reap(now);
        
        if (count)
            m_log.info("purged %d expired record(s) from storage", count);
    }

    m_log.info("cleanup thread finished");

    mutex->unlock();
    Thread::exit(NULL);
}

void MemoryStorageService::reap(const char* context)
{
    getContext(context).reap(time(NULL));
}

unsigned long MemoryStorageService::Context::reap(time_t exp)
{
    // Lock the "database".
    m_lock->wrlock();
    SharedLock wrapper(m_lock, false);
    
    // Garbage collect any expired entries.
    unsigned long count=0;
    map<string,Record>::iterator cur = m_dataMap.begin();
    map<string,Record>::iterator stop = m_dataMap.end();
    while (cur != stop) {
        if (cur->second.expiration <= exp) {
            map<string,Record>::iterator tmp = cur++;
            m_dataMap.erase(tmp);
            ++count;
        }
        else {
            cur++;
        }
    }
    return count;
}

void MemoryStorageService::createString(const char* context, const char* key, const char* value, time_t expiration)
{
    Context& ctx = getContext(context);

    // Lock the maps.
    ctx.m_lock->wrlock();
    SharedLock wrapper(ctx.m_lock, false);
    
    // Check for a duplicate.
    map<string,Record>::iterator i=ctx.m_dataMap.find(key);
    if (i!=ctx.m_dataMap.end()) {
        // Not yet expired?
        if (time(NULL) < i->second.expiration)
            throw IOException("attempted to insert a record with duplicate key ($1)", params(1,key));
        // It's dead, so we can just remove it now and create the new record.
        ctx.m_dataMap.erase(i);
    }
    
    ctx.m_dataMap[key]=Record(value,expiration);
    
    m_log.debug("inserted record (%s) in context (%s)", key, context);
}

int MemoryStorageService::readString(const char* context, const char* key, string* pvalue, time_t* pexpiration, int version)
{
    Context& ctx = getContext(context);

    SharedLock wrapper(ctx.m_lock);
    map<string,Record>::iterator i=ctx.m_dataMap.find(key);
    if (i==ctx.m_dataMap.end())
        return 0;
    else if (time(NULL) >= i->second.expiration)
        return 0;
    if (pexpiration)
        *pexpiration = i->second.expiration;
    if (i->second.version == version)
        return version; // nothing's changed, so just echo back the version
    if (pvalue)
        *pvalue = i->second.data;
    return i->second.version;
}

int MemoryStorageService::updateString(const char* context, const char* key, const char* value, time_t expiration, int version)
{
    Context& ctx = getContext(context);

    // Lock the map.
    ctx.m_lock->wrlock();
    SharedLock wrapper(ctx.m_lock, false);

    map<string,Record>::iterator i=ctx.m_dataMap.find(key);
    if (i==ctx.m_dataMap.end())
        return 0;
    else if (time(NULL) >= i->second.expiration)
        return 0;
    
    if (version > 0 && version != i->second.version)
        return -1;  // caller's out of sync

    if (value) {
        i->second.data = value;
        ++(i->second.version);
    }
        
    if (expiration && expiration != i->second.expiration)
        i->second.expiration = expiration;

    m_log.debug("updated record (%s) in context (%s)", key, context);
    return i->second.version;
}

bool MemoryStorageService::deleteString(const char* context, const char* key)
{
    Context& ctx = getContext(context);

    // Lock the map.
    ctx.m_lock->wrlock();
    SharedLock wrapper(ctx.m_lock, false);
    
    // Find the record.
    map<string,Record>::iterator i=ctx.m_dataMap.find(key);
    if (i!=ctx.m_dataMap.end()) {
        ctx.m_dataMap.erase(i);
        m_log.debug("deleted record (%s) in context (%s)", key, context);
        return true;
    }

    m_log.debug("deleting record (%s) in context (%s)....not found", key, context);
    return false;
}

void MemoryStorageService::updateContext(const char* context, time_t expiration)
{
    Context& ctx = getContext(context);

    // Lock the map.
    ctx.m_lock->wrlock();
    SharedLock wrapper(ctx.m_lock, false);

    time_t now = time(NULL);
    map<string,Record>::iterator stop=ctx.m_dataMap.end();
    for (map<string,Record>::iterator i = ctx.m_dataMap.begin(); i!=stop; ++i) {
        if (now >= i->second.expiration)
            i->second.expiration = expiration;
    }

    m_log.debug("updated expiration of valid records in context (%s)", context);
}
