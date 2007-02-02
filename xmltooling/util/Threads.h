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
 * @file Threads.h
 * 
 * Thread and locking wrappers 
 */

#ifndef _xmltooling_threads_h
#define _xmltooling_threads_h

#include <xmltooling/base.h>
#include <xmltooling/exceptions.h>

namespace xmltooling
{
    DECL_XMLTOOLING_EXCEPTION(ThreadingException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions during threading/locking operations);
    
    /**
     * A class for manual thread creation and synchronization.
     */
    class XMLTOOL_API Thread
    {
        MAKE_NONCOPYABLE(Thread);
    public:
        Thread() {}
        virtual ~Thread() {}

        /**
         * Disassociate from the thread.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int detach()=0;
        
        /**
         * Join with the thread and wait for its completion.
         * 
         * @param thread_return holds the return value of the thread routine
         * @return 0 for success, non-zero for failure
         */
        virtual int join(void** thread_return)=0;
        
        /**
         * Kill the thread.
         * 
         * @param signo the signal to send to the thread
         * @return 0 for success, non-zero for failure
         */
        virtual int kill(int signo)=0;
        
        /**
         * Creates a new thread object to run the supplied start routine.
         * 
         * @param start_routine the function to execute on the thread
         * @param arg           a parameter for the start routine
         * @return  the created and running thread object 
         */
        static Thread* create(void* (*start_routine)(void*), void* arg);
        
        /**
         * Exits a thread gracefully.
         * 
         * @param return_val    the return value for the thread
         */
        static void exit(void* return_val);

        /**
         * Sleeps the current thread for the specified amount of time.
         * 
         * @param seconds   time to sleep
         */
        static void sleep(int seconds);        
#ifndef WIN32
        /**
         * Masks all signals from a thread. 
         */
        static void mask_all_signals(void);
        
        /**
         * Masks specific signals from a thread.
         * 
         * @param how
         * @param newmask   the new signal mask
         * @param oldmask   the old signal mask
         * @return 0 for success, non-zero for failure
         */
        static int mask_signals(int how, const sigset_t *newmask, sigset_t *oldmask);
#endif
    };

    /**
     * A class for managing Thread Local Storage values.
     */
    class XMLTOOL_API ThreadKey
    {
        MAKE_NONCOPYABLE(ThreadKey);
    public:
        ThreadKey() {}
        virtual ~ThreadKey() {}

        /**
         * Sets the value for a TLS key.
         * 
         * @param data  the value to set
         * @return 0 for success, non-zero for failure
         */
        virtual int setData(void* data)=0;

        /**
         * Returns the value for a TLS key.
         * 
         * @return the value or NULL
         */        
        virtual void* getData() const=0;

        /**
         * Creates a new TLS key.
         * 
         * @param destroy_fn    a functon to cleanup key values
         * @return the new key
         */
        static ThreadKey* create(void (*destroy_fn)(void*));
    };

    /**
     * A class for managing exclusive access to resources.
     */
    class XMLTOOL_API Mutex
    {
        MAKE_NONCOPYABLE(Mutex);
    public:
        Mutex() {}
        virtual ~Mutex() {}

        /**
         * Locks the mutex for exclusive access.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int lock()=0;
        
        /**
         * Unlocks the mutex for exclusive access.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int unlock()=0;

        /**
         * Creates a new mutex object.
         * 
         * @return the new mutex
         */
        static Mutex* create();
    };
    
    /**
     * A class for managing shared and exclusive access to resources.
     */
    class XMLTOOL_API RWLock
    {
        MAKE_NONCOPYABLE(RWLock);
    public:
        RWLock() {}
        virtual ~RWLock() {}

        /**
         * Obtains a shared lock.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int rdlock()=0;
        
        /**
         * Obtains an exclusive lock.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int wrlock()=0;

        /**
         * Unlocks the lock.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int unlock()=0;

        /**
         * Creates a new read/write lock.
         * 
         * @return the new lock
         */
        static RWLock* create();
    };
    
    /**
     * A class for establishing queues on a mutex based on a periodic condition.
     */
    class XMLTOOL_API CondWait
    {
        MAKE_NONCOPYABLE(CondWait);
    public:
        CondWait() {}
        virtual ~CondWait() {}
        
        /**
         * Waits for a condition variable using the supplied mutex as a queue.
         * 
         * @param lock  mutex to queue on
         * @return 0 for success, non-zero for failure
         */
        virtual int wait(Mutex* lock)=0;
        
        /**
         * Waits for a condition variable using the supplied mutex as a queue,
         * but only for a certain time limit.
         * 
         * @param lock          mutex to queue on
         * @param delay_seconds maximum time to wait before waking up
         * @return 0 for success, non-zero for failure
         */
        virtual int timedwait(Mutex* lock, int delay_seconds)=0;
        
        /**
         * Signal a single thread to wake up if a condition changes.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int signal()=0;
        
        /**
         * Signal all threads to wake up if a condition changes.
         * 
         * @return 0 for success, non-zero for failure
         */
        virtual int broadcast()=0;

        /**
         * Creates a new condition variable.
         * 
         * @return the new condition variable
         */
        static CondWait* create();
    };
    
    /**
     * RAII wrapper for a mutex lock.
     */
    class XMLTOOL_API Lock {
        MAKE_NONCOPYABLE(Lock);
    public:
        /**
         * Locks and wraps the designated mutex.
         * 
         * @param mtx mutex to lock 
         */
        Lock(Mutex* mtx) : mutex(mtx) {
            mutex->lock();
        }
        
        /**
         * Unlocks the wrapped mutex.
         */
        ~Lock() {
            mutex->unlock();
        }
    
    private:
        Mutex* mutex;
    };
    
    /**
     * RAII wrapper for a shared lock.
     */
    class XMLTOOL_API SharedLock {
        MAKE_NONCOPYABLE(SharedLock);
    public:
        /**
         * Locks and wraps the designated shared lock.
         * 
         * @param lock      lock to acquire 
         * @param lockit    true if the lock should be acquired here, false if already acquired
         */
        SharedLock(RWLock* lock, bool lockit=true) : rwlock(lock) {
            if (lockit)
                rwlock->rdlock();
        }
        
        /**
         * Unlocks the wrapped shared lock.
         */
        ~SharedLock() {
            rwlock->unlock();
        }
    
    private:
        RWLock* rwlock;
    };

}

#endif /* _xmltooling_threads_h */
