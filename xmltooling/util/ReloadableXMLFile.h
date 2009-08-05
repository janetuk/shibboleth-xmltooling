/*
 *  Copyright 2001-2009 Internet2
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

#include <xmltooling/logging.h>
#include <xmltooling/Lockable.h>
#include <xmltooling/util/Threads.h>

#include <ctime>
#include <string>
#include <xercesc/dom/DOM.hpp>

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
         *  <dd>enables monitoring of local file for changes</dd>
         *  <dt>reloadInterval</dt>
         *  <dd>enables periodic refresh of remote file</dd>
         *  <dt>backingFilePath</dt>
         *  <dd>location for backup of remote resource</dd>
         * </dl>
         * 
         * @param e     DOM to supply configuration
         * @param log   logging object to use
         */
        ReloadableXMLFile(const xercesc::DOMElement* e, logging::Category& log);
    
        virtual ~ReloadableXMLFile() {
            delete m_lock;
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
        virtual std::pair<bool,xercesc::DOMElement*> load() {
            return load(false);
        }
        
        /** Root of the original DOM element passed into constructor. */
        const xercesc::DOMElement* m_root;
        
        /** Indicates whether resources is local or remote. */
        bool m_local;
        
        /** Use a validating parser when parsing XML. */
        bool m_validate;
        
        /** Resource location, may be a local path or a URI. */
        std::string m_source;

        /** Path to backup copy for remote resources. */
        std::string m_backing;
        
        /** Last modification of local resource or reload of remote resource. */
        time_t m_filestamp;

        /** Time in seconds to wait before trying for new copy of remote resource. */
        time_t m_reloadInterval;

        /** Shared lock for guarding reloads. */
        RWLock* m_lock;
        
        /** Logging object. */
        logging::Category& m_log;

    public:
        Lockable* lock();

        void unlock() {
            if (m_lock)
                m_lock->unlock();
        }

    private:
        std::pair<bool,xercesc::DOMElement*> load(bool backup);
    };

};

#endif /* __xmltooling_reloadable_h__ */
