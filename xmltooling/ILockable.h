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
 * @file ILockable.h
 * 
 * Locking abstraction 
 */

#if !defined(__xmltooling_lockable_h__)
#define __xmltooling_lockable_h__

#include <xmltooling/base.h>

namespace xmltooling {

    /**
     * Singleton object that manages library startup/shutdown.configuration.
     */
    struct XMLTOOL_API ILockable
    {
        virtual ~ILockable() {}
        
        /**
         * Lock the associated object for exclusive access.
         * 
         * @return a reference to the object being locked
         */
        virtual ILockable& lock()=0;

        /**
         * Unlock the associated object from exclusive access.
         */
        virtual void unlock()=0;
    };

    /**
     * RAII wrapper for lockable objects to ensure lock release
     */
    class XMLTOOL_API Locker
    {
    MAKE_NONCOPYABLE(Locker);
    public:
        /**
         * Locks an object and stores it for later release.
         * 
         * @param lockee    Pointer to an object to lock and hold
         */
        Locker(ILockable* lockee) : m_lockee(lockee->lock()) {}
        
        /**
         * Locks an object and stores it for later release.
         * 
         * @param lockee    Reference to an object to lock and hold
         */
        Locker(ILockable& lockee) : m_lockee(lockee.lock()) {}

        /**
         * Releases lock on held pointer, if any.
         */
        ~Locker() {m_lockee.unlock();}
    private:
        ILockable& m_lockee;
    };

};

#endif /* __xmltooling_lockable_h__ */
