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
 * Win32Threads.cpp
 *
 * Thread and locking wrappers for Win32 platforms
 */

#include "internal.h"
#include "logging.h"
#include "util/Threads.h"

#ifndef WIN32
# error "This implementation is for WIN32 platforms."
#endif

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

// base error code for a routine to return on failure
#define THREAD_ERROR_TIMEOUT 	(1)
#define THREAD_ERROR_WAKE_OTHER (2)
#define THREAD_ERROR 		    (3)

// windows returns non zero for success pthreads returns zero
static int XMLTOOL_DLLLOCAL map_windows_error_status_to_pthreads(int rc=0) {
    if(rc!=0)  // success?
        return 0;
    Category::getInstance(XMLTOOLING_LOGCAT".Threads").error("error from thread operation (%d)", GetLastError());
    return THREAD_ERROR;
}

namespace xmltooling {

    // two levels of classes are needed here
    // in case InitializeCriticalSection
    // throws an exception we can keep from
    // calling the critical_section destructor
    // on unitilized data, or it could be done with a flag
    struct XMLTOOL_DLLLOCAL critical_section_data {
        CRITICAL_SECTION cs;
        critical_section_data(){
            InitializeCriticalSection(&cs);
        }
    };

    class XMLTOOL_DLLLOCAL critical_section {
    private:
        critical_section_data	cse;
    public:
        critical_section(){}
        ~critical_section(){
            DeleteCriticalSection (&cse.cs);
        }
        void enter(void) {
            EnterCriticalSection(&cse.cs);
        }
        void leave(void) {
            LeaveCriticalSection(&cse.cs);
        }
    };

    // hold a critical section over the lifetime of this object
    // used to make a stack variable that unlocks automaticly
    // on return/throw
    class XMLTOOL_DLLLOCAL with_crit_section {
    private:
        critical_section& cs;
    public:
        with_crit_section(critical_section& acs):cs(acs){
            cs.enter();
        }
        ~with_crit_section(){
            cs.leave();
        }
    };

    class XMLTOOL_DLLLOCAL ThreadImpl : public Thread {
    private:
        HANDLE thread_id;
    public:
        ThreadImpl(void* (*start_routine)(void*), void* arg) : thread_id(0) {
            thread_id=CreateThread(
                0, // security attributes
                0, // use default stack size, maybe this should be setable
                (LPTHREAD_START_ROUTINE ) start_routine,
                arg,
                0, // flags, default is ignore stacksize and don't create suspended which is what we want
                0);
            if (thread_id==0) {
                map_windows_error_status_to_pthreads();
                throw ThreadingException("Thread creation failed.");
            }
        }

        ~ThreadImpl() {
            (void)detach();
        }

        int detach() {
            if (thread_id==0)
                return THREAD_ERROR;
            int rc=map_windows_error_status_to_pthreads(CloseHandle(thread_id));
            thread_id=0;
            return rc;
        }

        int join(void** thread_return) {
            if (thread_id==0)
                return THREAD_ERROR;
            if (thread_return!=0)
                *thread_return=0;
            int rc=WaitForSingleObject(thread_id,INFINITE);
            switch(rc) {
                case WAIT_OBJECT_0:
                    if (thread_return)
                        map_windows_error_status_to_pthreads(GetExitCodeThread(thread_id,(unsigned long*)thread_return));
                default:
                    return THREAD_ERROR;
            }
            return 0;
        }

        int kill(int signo) {
            if (thread_id==0)
                return THREAD_ERROR;
            return map_windows_error_status_to_pthreads(TerminateThread(thread_id,signo));
        }
    };

    class XMLTOOL_DLLLOCAL MutexImpl : public Mutex {
    private:
        HANDLE mhandle;
    public:
        MutexImpl() : mhandle(CreateMutex(0,false,0)) {
            if (mhandle==0) {
                map_windows_error_status_to_pthreads();
                throw ThreadingException("Mutex creation failed.");
            }
        }

        ~MutexImpl() {
            if((mhandle!=0) && (!CloseHandle(mhandle)))
                map_windows_error_status_to_pthreads();
        }

        int lock() {
            int rc=WaitForSingleObject(mhandle,INFINITE);
            switch(rc) {
                case WAIT_ABANDONED:
                case WAIT_OBJECT_0:
                    return 0;
                default:
                    return map_windows_error_status_to_pthreads();
            }
        }

        int unlock() {
            return map_windows_error_status_to_pthreads(ReleaseMutex(mhandle));
        }
    };

    class XMLTOOL_DLLLOCAL CondWaitImpl : public CondWait {
    private:
        HANDLE cond;

    public:
        CondWaitImpl() : cond(CreateEvent(0,false,false,0)) {
            if(cond==0) {
                map_windows_error_status_to_pthreads();
    	        throw ThreadingException("Event creation failed.");
            }
        };

        ~CondWaitImpl() {
            if((cond!=0) && (!CloseHandle(cond)))
                map_windows_error_status_to_pthreads();
        }

        int wait(Mutex* mutex) {
            return timedwait(mutex,INFINITE);
        }

        int signal() {
            if(!SetEvent(cond))
                return map_windows_error_status_to_pthreads();
            return 0;
        }

        int broadcast() {
            throw ThreadingException("Broadcast not implemented on Win32 platforms.");
        }

        // wait for myself to signal and this mutex or the timeout
        int timedwait(Mutex* mutex, int delay_seconds) {
            int rc=mutex->unlock();
            if(rc!=0)
                return rc;

            int delay_ms=delay_seconds;
            if(delay_seconds!=INFINITE)
                delay_ms*=1000;
            rc=WaitForSingleObject(cond,delay_ms);
            int rc2=mutex->lock();
            if(rc2!=0)
                return rc2;
            switch(rc) {
                case WAIT_ABANDONED:
                case WAIT_OBJECT_0:
                case WAIT_TIMEOUT:
                    return 0;
                default:
                    return map_windows_error_status_to_pthreads();
            }
            return 0;
        }
    };

    class XMLTOOL_DLLLOCAL RWLockImpl : public RWLock {
    private:
        // used to protect read or write to the data below
        critical_section cs;
        // event handle threads wait on when the lock they want is busy
        // normally set to signaled all the time, if some thread can't get what
        // they want they reset it and sleep.  on releasing a lock set it to
        // signaled if someone may have wanted what you just released
        HANDLE wake_waiters;
        // number of threads holding a read lock
        int num_readers;
        // true iff there a writer has our lock
        bool have_writer;

    public:
        RWLockImpl() : wake_waiters(0), num_readers(0), have_writer(true) {
            with_crit_section acs(cs);
            wake_waiters=CreateEvent(0,true,true,0);
            have_writer=false;
            if (wake_waiters==0) {
                map_windows_error_status_to_pthreads();
                throw ThreadingException("Event creation for shared lock failed.");
            }
        }

        ~RWLockImpl() {
            with_crit_section acs(cs);
            if ((wake_waiters!=0) && (!CloseHandle(wake_waiters)))
                map_windows_error_status_to_pthreads();
        }

        int rdlock() {
            while(1) {
                // wait for the lock maybe being availible
                // we will find out for sure inside the critical section
                if (WaitForSingleObject(wake_waiters,INFINITE)!=WAIT_OBJECT_0)
                    return map_windows_error_status_to_pthreads();

                with_crit_section alock(cs);
                // invariant not locked for reading and writing
                if ((num_readers!=0) && (have_writer))
                    return THREAD_ERROR;
                // if no writer we can join any existing readers
                if (!have_writer) {
                    num_readers++;
                    return 0;
                }

                // have a writer, mark the synchronization object
                // so everyone waits, when the writer unlocks it will wake us
                if (!ResetEvent(wake_waiters))
                    return map_windows_error_status_to_pthreads();
            }
            return THREAD_ERROR;
        }

        int wrlock() {
            while(1) {
                // wait for the lock maybe being availible
                // we will find out for sure inside the critical section
                if (WaitForSingleObject(wake_waiters,INFINITE)!=WAIT_OBJECT_0)
                    return map_windows_error_status_to_pthreads();

                with_crit_section bla(cs);
                // invariant not locked for reading and writing
                if ((num_readers!=0) && (have_writer))
        	       return THREAD_ERROR;

                // if no writer and no readers we can become the writer
                if ((num_readers==0) && (!have_writer)) {
                    have_writer=true;
                    return 0;
                }

                // lock is busy, the unlocker will wake us
                if (!ResetEvent(wake_waiters))
                    return map_windows_error_status_to_pthreads();
            }
            return THREAD_ERROR;
        }

        int unlock() {
            with_crit_section mumble(cs);
            // invariant not locked for reading and writing
            if ((num_readers!=0) && (have_writer))
                return THREAD_ERROR;

            // error if nothing locked
            if ((num_readers==0) && (!have_writer))
                return THREAD_ERROR;

            // if there was a writer it has to be us so unlock write lock
            have_writer=false;

            // if there where any reades there is one less now
            if(num_readers>0)
                num_readers--;

            // if no readers left wake up any readers/writers waiting
            // to have a go at it
            if (num_readers==0)
                if (!SetEvent(wake_waiters))
                    return map_windows_error_status_to_pthreads();
            return 0;
        }
    };

    typedef void (*destroy_hook_type)(void*);

    class XMLTOOL_DLLLOCAL ThreadKeyImpl : public ThreadKey {
    private:
        //destroy_hook_type destroy_hook;
        DWORD key;

    public:
        ThreadKeyImpl(void (*destroy_fcn)(void*)) { // : destroy_hook(destroy_fcn) {
            if (destroy_fcn)
                throw ThreadingException("TLS destructor function not supported.");
            key=TlsAlloc();
        };

        virtual ~ThreadKeyImpl() {
            //if (destroy_hook)
            //    destroy_hook(TlsGetValue(key));
            TlsFree(key);
        }

        int setData(void* data) {
            TlsSetValue(key,data);
            return 0;
        }

        void* getData() const {
            return TlsGetValue(key);
        }
    };

};

//
// public "static" creation functions
//

Thread* Thread::create(void* (*start_routine)(void*), void* arg)
{
    return new ThreadImpl(start_routine, arg);
}

void Thread::exit(void* return_val)
{
    ExitThread((DWORD)return_val);
}

void Thread::sleep(int seconds)
{
    Sleep(seconds * 1000);
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
