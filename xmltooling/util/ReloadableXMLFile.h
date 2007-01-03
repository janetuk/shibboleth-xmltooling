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
 * @file xmltooling/util/ReloadableXMLFile.h
 * 
 * Base class for file-based XML configuration.
 */

#ifndef __xmltooling_reloadable_h__
#define __xmltooling_reloadable_h__

#include <xmltooling/Lockable.h>
#include <xmltooling/util/Threads.h>

#include <ctime>
#include <string>

namespace xmltooling {

    /**
     * Base class for file-based XML configuration.
     */
    class XMLTOOL_API ReloadableXMLFile : protected virtual Lockable
    {
        MAKE_NONCOPYABLE(ReloadableXMLFile);
        
    protected:
        /**
         * Constructor taking a DOM element supporting the following content:
         * 
         * <dl>
         *  <dt>file | filename | path | pathname</dt>
         *  <dd>identifies a local file</dd>
         *  <dt>uri | url</dt>
         *  <dd>identifies a remote resource</dd>
         *  <dt>validate</dt>
         *  <dd>use a validating parser</dd>
         *  <dt>reloadChanges</dt>
         *  <dd>enables monitoring of resources for changes</dd>
         * </dl>
         * 
         * @param e DOM to supply configuration
         */
        ReloadableXMLFile(const DOMElement* e);
    
        virtual ~ReloadableXMLFile() {
            delete m_lock;
        }

    public:
        Lockable* lock();

        void unlock() {
            if (m_lock)
                m_lock->unlock();
        }

        /**
         * Loads configuration material.
         * 
         * <p>This method is called to load configuration material
         * initially and any time a change is detected. The base version
         * performs basic parsing duties and returns the result.
         * 
         * @return a pair consisting of a flag indicating whether to take ownership of
         *      the document, and the root element of the tree to load
         */
        virtual std::pair<bool,DOMElement*> load();
        
        /**
         * Overrideable method to determine whether a remote resource remains valid.
         * 
         * @return  true iff the resource remains valid and should not be reloaded
         */
        virtual bool isValid() const {
            return true;
        }

    private:
        const DOMElement* m_root;
        bool m_local, m_validate;
        std::string m_source;
        time_t m_filestamp;
        RWLock* m_lock;
    };

};

#endif /* __xmltooling_reloadable_h__ */
