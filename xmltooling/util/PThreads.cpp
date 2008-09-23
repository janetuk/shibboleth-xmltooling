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
 * PThreads.cpp
 * 
 * Thread and locking wrappers for POSIX platforms
 */

#include "internal.h"
#include "logging.h"
#include "util/Threads.h"

#include <ctime>
#include <signal.h>

#ifdef HAVE_PTHREAD
# include <pthread.h>
# ifndef HAVE_PTHREAD_RWLOCK_INIT
#  include <synch.h>
# endif
#else
# error "This implementation is for POSIX platforms."
#endif

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

namespace xmltooling {

    class XMLTOOL_DLLLOCAL ThreadImpl : public Thread {
        pthread_t thread_id;
    public:
        ThreadImpl(void* (*start_routine)(void*), void* arg);
        virtual ~ThreadImpl() {}
    
        int detach() {
            return pthread_detach(thread_id);
        }
        
        int join(void** thread_return) {
            return pthread_join(thread_id, thread_return);
        }
        
        int kill(int signo) {
            return pthread_kill(thread_id, signo);
        }
    };
    
    class XMLTOOL_DLLLOCAL MutexImpl : public Mutex {
        pthread_mutex_t mutex;
        friend class XMLTOOL_DLLLOCAL CondWaitImpl;
    public:
        MutexImpl();
        virtual ~MutexImpl() {
            pthread_mutex_destroy(&mutex);
        }
    
        int lock() {
            return pthread_mutex_lock(&mutex);
        }
        
        int unlock() {
            return pthread_mutex_unlock(&mutex);
        }
    };
    
    class XMLTOOL_DLLLOCAL CondWaitImpl : public CondWait {
        pthread_cond_t cond;
    public:
        CondWaitImpl();
        virtual ~CondWaitImpl() {
            pthread_cond_destroy(&cond);
        }
    
        int wait(Mutex* mutex) {
            return wait(static_cast<MutexImpl*>(mutex));
        }
        
        int wait(MutexImpl* mutex) {
            return pthread_cond_wait(&cond, &(mutex->mutex));
        }
        
        int timedwait(Mutex* mutex, int delay_seconds) {
            return timedwait(static_cast<MutexImpl*>(mutex), delay_seconds);
        }
        
        int timedwait(MutexImpl* mutex, int delay_seconds) {
            struct timespec ts;
            memset(&ts, 0, sizeof(ts));
            ts.tv_sec = time(NULL) + delay_seconds;
            return pthread_cond_timedwait(&cond, &(mutex->mutex), &ts);
        }
        
        int signal() {
            return pthread_cond_signal(&cond);
        }
        
        int broadcast() {
            return pthread_cond_broadcast(&cond);
        }
    };
    
    class XMLTOOL_DLLLOCAL RWLockImpl : public RWLock {
#ifdef HAVE_PTHREAD_RWLOCK_INIT
        pthread_rwlock_t lock;
    public:
        RWLockImpl();
        virtual ~RWLockImpl() {
            pthread_rwlock_destroy(&lock);
        }
    
        int rdlock() {
            return pthread_rwlock_rdlock(&lock);
        }
        
        int wrlock() {
            return pthread_rwlock_wrlock(&lock);
        }
        
        int unlock() {
            return pthread_rwlock_unlock(&lock);
        }
#else
        rwlock_t lock;
    public:
        RWLockImpl();
        virtual ~RWLockImpl() {
            rwlock_destroy (&lock);
        }
    
        int rdlock() {
            return rw_rdlock(&lock);
        }
        
        int wrlock() {
            return rw_wrlock(&lock);
        }
        
        int unlock() {
            return rw_unlock(&lock);
        }
#endif
    };
    
    class XMLTOOL_DLLLOCAL ThreadKeyImpl : public ThreadKey {
        pthread_key_t key;
    public:
        ThreadKeyImpl(void (*destroy_fcn)(void*));
        virtual ~ThreadKeyImpl() {
            pthread_key_delete(key);
        }
    
        int setData(void* data) {
            return pthread_setspecific(key,data);
        }
        
        void* getData() const {
            return pthread_getspecific(key);
        }
    };

};

ThreadImpl::ThreadImpl(void* (*start_routine)(void*), void* arg)
{
    int rc=pthread_create(&thread_id, NULL, start_routine, arg);
    if (rc) {
#ifdef HAVE_STRERROR_R
        char buf[256];
        strerror_r(rc,buf,sizeof(buf));
        buf[255]=0;
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_create error (%d): %s",rc,buf);
#else
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_create error (%d): %s",rc,strerror(rc));
#endif
        throw ThreadingException("Thread creation failed.");
    }
}

MutexImpl::MutexImpl()
{
    int rc=pthread_mutex_init(&mutex, NULL);
    if (rc) {
#ifdef HAVE_STRERROR_R
        char buf[256];
        strerror_r(rc,buf,sizeof(buf));
        buf[255]=0;
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_mutex_init error (%d): %s",rc,buf);
#else
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_mutex_init error (%d): %s",rc,strerror(rc));
#endif
        throw ThreadingException("Mutex creation failed.");
    }
}

CondWaitImpl::CondWaitImpl()
{
    int rc=pthread_cond_init(&cond, NULL);
    if (rc) {
#ifdef HAVE_STRERROR_R
        char buf[256];
        strerror_r(rc,buf,sizeof(buf));
        buf[255]=0;
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_cond_init error (%d): %s",rc,buf);
#else
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_cond_init error (%d): %s",rc,strerror(rc));
#endif
        throw ThreadingException("Condition variable creation failed.");
    }
}

RWLockImpl::RWLockImpl()
{
#ifdef HAVE_PTHREAD_RWLOCK_INIT
    int rc=pthread_rwlock_init(&lock, NULL);
#else
    int rc=rwlock_init(&lock, USYNC_THREAD, NULL);
#endif
    if (rc) {
#ifdef HAVE_STRERROR_R
        char buf[256];
        strerror_r(rc,buf,sizeof(buf));
        buf[255]=0;
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_rwlock_init error (%d): %s",rc,buf);
#else
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_rwlock_init error (%d): %s",rc,strerror(rc));
#endif
        throw ThreadingException("Shared lock creation failed.");
    }
}

ThreadKeyImpl::ThreadKeyImpl(void (*destroy_fcn)(void*))
{
    int rc=pthread_key_create(&key, destroy_fcn);
    if (rc) {
#ifdef HAVE_STRERROR_R
        char buf[256];
        strerror_r(rc,buf,sizeof(buf));
        buf[255]=0;
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_key_create error (%d): %s",rc,buf);
#else
        Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("pthread_key_create error (%d): %s",rc,strerror(rc));
#endif
        throw ThreadingException("Thread key creation failed.");
    }
}

Thread* Thread::create(void* (*start_routine)(void*), void* arg)
{
    return new ThreadImpl(start_routine, arg);
}

void Thread::exit(void* return_val)
{
    pthread_exit(return_val);
}

void Thread::sleep(int seconds)
{
    sleep(seconds);
}

void Thread::mask_all_signals(void)
{
    sigset_t sigmask;
    sigfillset(&sigmask);
    Thread::mask_signals(SIG_BLOCK, &sigmask, NULL);
}

int Thread::mask_signals(int how, const sigset_t *newmask, sigset_t *oldmask)
{
    return pthread_sigmask(how,newmask,oldmask);
}

Mutex * Mutex::create()
{
    return new MutexImpl();
}

CondWait * CondWait::create()
{
    return new CondWaitImpl();
}

RWLock * RWLock::create()
{
    return new RWLockImpl();
}

ThreadKey* ThreadKey::create (void (*destroy_fcn)(void*))
{
    return new ThreadKeyImpl(destroy_fcn);
}