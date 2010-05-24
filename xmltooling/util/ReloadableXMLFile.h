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
 * @file xmltooling/util/ReloadableXMLFile.h
 * 
 * Base class for file-based XML configuration.
 */

#ifndef __xmltooling_reloadable_h__
#define __xmltooling_reloadable_h__

#include <xmltooling/logging.h>
#include <xmltooling/Lockable.h>

#include <ctime>
#include <string>
#include <xercesc/dom/DOM.hpp>

#ifndef XMLTOOLING_LITE
    namespace xmlsignature {
        class XMLTOOL_API Signature;
    };
#endif

namespace xmltooling {

    class XMLTOOL_API CondWait;
    class XMLTOOL_API RWLock;
    class XMLTOOL_API Thread;

#ifndef XMLTOOLING_LITE
    class XMLTOOL_API CredentialResolver;
    class XMLTOOL_API SignatureTrustEngine;
#endif

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
         *  <dt>reloadInterval or maxRefreshDelay</dt>
         *  <dd>enables periodic refresh of remote file</dd>
         *  <dt>backingFilePath</dt>
         *  <dd>location for backup of remote resource</dd>
         *  <dt>id</dt>
         *  <dd>identifies the plugin instance for logging purposes</dd>
         *  <dt>certificate</dt>
         *  <dd>requires XML be signed with an enveloped signature verifiable with specified key</dd>
         *  <dt>signerName</dt>
         *  <dd>requires XML be signed with an enveloped signature verifiable with &lt;TrustEngine&gt;
         *      by certificate containing this name</dd>
         *  <dt>&lt;CredentialResolver&gt;</dt>
         *  <dd>requires XML be signed with an enveloped signature verifiable with specified key</dd>
         *  <dt>&lt;TrustEngine&gt;</dt>
         *  <dd>requires XML be signed with an enveloped signature verifiable with specified TrustEngine</dd>
         * </dl>
         * 
         * @param e                 DOM to supply configuration
         * @param log               logging object to use
         * @param startReloadThread true iff refresh thread for resources should be started by constructor
         */
        ReloadableXMLFile(const xercesc::DOMElement* e, logging::Category& log, bool startReloadThread=true);
    
        virtual ~ReloadableXMLFile();

        /**
         * Loads configuration material.
         * 
         * <p>This method is called to load configuration material
         * initially and any time a change is detected. The base version
         * performs basic parsing duties and returns the result.
         *
         * <p>This method is not called with the object locked, so actual
         * modification of implementation state requires explicit locking within
         * the method override.
         * 
         * @return a pair consisting of a flag indicating whether to take ownership of
         *      the document, and the root element of the tree to load
         */
        virtual std::pair<bool,xercesc::DOMElement*> background_load();

        /**
         * @deprecated
         * Basic load/parse of configuration material.
         * 
         * <p>The base version performs basic parsing duties and returns the result.
         * Subclasses should override the new background_load() method and perform
         * their own locking in conjunction with use of this method.
         *
         * <p>Subclasses that continue to override this method will function, but
         * a write lock will be acquired and held for the entire operation.
         * 
         * @return a pair consisting of a flag indicating whether to take ownership of
         *      the document, and the root element of the tree to load
         */
        virtual std::pair<bool,xercesc::DOMElement*> load();

        /**
         * Basic load/parse of configuration material.
         *
         * <p>The base version performs basic parsing duties and returns the result.
         * Subclasses should override the new background_load() method and perform
         * their own locking in conjunction with use of this method.
         *
         * <p>This version allows subclasses to explicitly control the use of a
         * backup for remote resources, which allows additional validation to be
         * performed besides just successful XML parsing.
         *
         * @param backup    true iff the backup source should be loaded
         * @return a pair consisting of a flag indicating whether to take ownership of
         *      the document, and the root element of the tree to load
         */
        virtual std::pair<bool,xercesc::DOMElement*> load(bool backup);

        /**
         * Accesses a lock interface protecting use of backup file associated with the
         * object.
         *
         * <p>The lock is <strong>NOT</strong> acquired automatically.
         *
         * @return  pointer to a lock interface, or nullptr if unnecessary
         */
        virtual Lockable* getBackupLock();

        /**
         * Preserves the last remote resource caching identifier in a backup file
         * for use on the next restart.
         */
        void preserveCacheTag();

        /**
         * Starts up reload thread, can be automatically called by constructor, or
         * manually invoked by subclass.
         */
        void startup();

        /**
         * Shuts down reload thread, should be called from subclass destructor.
         */
        void shutdown();

        /** Root of the original DOM element passed into constructor. */
        const xercesc::DOMElement* m_root;
        
        /** Indicates whether resources is local or remote. */
        bool m_local;
        
        /** Use a validating parser when parsing XML. */
        bool m_validate;
        
        /** Resource location, may be a local path or a URI. */
        std::string m_source;

        /** Path to backup copy for remote resource. */
        std::string m_backing;

        /** Last modification of local resource. */
        time_t m_filestamp;

        /** Time in seconds to wait before trying for new copy of remote resource. */
        time_t m_reloadInterval;

        /** Caching tag associated with remote resource. */
        std::string m_cacheTag;

        /** Shared lock for guarding reloads. */
        RWLock* m_lock;
        
        /** Logging object. */
        logging::Category& m_log;

        /** Plugin identifier. */
        std::string m_id;

        /** Indicates whether a usable version of the resource is in place. */
        bool m_loaded;

#ifndef XMLTOOLING_LITE
        /** CredentialResolver for signature verification. */
        CredentialResolver* m_credResolver;

        /** TrustEngine for signature verification. */
        SignatureTrustEngine* m_trust;

        /** Name of signer for signature verification. */
        std::string m_signerName;
#endif

    public:
        Lockable* lock();
        void unlock();

    private:
#ifndef XMLTOOLING_LITE
        void validateSignature(xmlsignature::Signature& sigObj) const;
#endif
        // Used to manage background reload/refresh.
        bool m_shutdown;
        CondWait* m_reload_wait;
        Thread* m_reload_thread;
        static void* reload_fn(void*);
    };

};

#endif /* __xmltooling_reloadable_h__ */
