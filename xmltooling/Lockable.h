/*
 *  Copyright 2001-2010 Internet2
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
 * @file xmltooling/Lockable.h
 * 
 * Locking abstraction.
 */

#ifndef __xmltooling_lockable_h__
#define __xmltooling_lockable_h__

#include <xmltooling/base.h>

namespace xmltooling {

    /**
     * Abstract mixin interface for interfaces that support locking
     */
    class XMLTOOL_API Lockable
    {
    protected:
        Lockable();
    public:
        virtual ~Lockable();
        
        /**
         * Lock the associated object for exclusive access.
         * 
         * @return a pointer to the object being locked
         */
        virtual Lockable* lock()=0;

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
         * Optionally locks an object and stores it for later release.
         * 
         * @param lockee    pointer to an object to hold, and optionally lock
         * @param lock      true iff object is not yet locked
         */
        Locker(Lockable* lockee=nullptr, bool lock=true);

        /**
         * Optionally locks an object and stores it for later release.
         * If an object is already held, it is unlocked and detached.
         * 
         * @param lockee    pointer to an object to hold, and optionally lock
         * @param lock      true iff object is not yet locked
         */
        void assign(Lockable* lockee=nullptr, bool lock=true);
        
        /**
         * Destructor releases lock on held pointer, if any.
         */
        ~Locker();
        
    private:
        Lockable* m_lockee;
    };

};

#endif /* __xmltooling_lockable_h__ */
