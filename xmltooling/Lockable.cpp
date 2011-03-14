/*
 *  Copyright 2009-2010 Internet2
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
 * Lockable.cpp
 * 
 * Locking abstraction.
 */

#include "internal.h"
#include "Lockable.h"

using namespace xmltooling;

Lockable::Lockable()
{
}

Lockable::~Lockable()
{
}
        
Locker::Locker(Lockable* lockee, bool lock)
{
    if (lockee && lock)
        m_lockee = lockee->lock();
    else
        m_lockee = lockee;
}

void Locker::assign(Lockable* lockee, bool lock)
{
    if (m_lockee)
        m_lockee->unlock();
    m_lockee = nullptr;
    if (lockee && lock)
        m_lockee = lockee->lock();
    else
        m_lockee = lockee;
}

Locker::~Locker()
{
    if (m_lockee)
        m_lockee->unlock();
}
