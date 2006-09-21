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
 * @file XMLToolingConfig.h
 * 
 * Library configuration 
 */

#ifndef __xmltooling_config_h__
#define __xmltooling_config_h__

#include <xmltooling/Lockable.h>
#include <xmltooling/PluginManager.h>
#include <xmltooling/util/ParserPool.h>

#ifndef XMLTOOLING_NO_XMLSEC
namespace xmlsignature {
    class XMLTOOL_API CredentialResolver;
    class XMLTOOL_API KeyResolver;
};
#endif

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

namespace xmltooling {
    
    class XMLTOOL_API StorageService;
    class XMLTOOL_API TrustEngine;
    class XMLTOOL_API XSECCryptoX509CRL;

    /**
     * Singleton object that manages library startup/shutdown.configuration.
     * 
     * A locking interface is supplied as a convenience for code that wants to
     * obtain a global system lock, but the actual configuration itself is not
     * synchronized.
     */
    class XMLTOOL_API XMLToolingConfig : public Lockable
    {
        MAKE_NONCOPYABLE(XMLToolingConfig);
    protected:
        XMLToolingConfig() : clock_skew_secs(180) {}
    public:
        virtual ~XMLToolingConfig() {}

        /**
         * Returns the global configuration object for the library.
         * 
         * @return reference to the global library configuration object
         */
        static XMLToolingConfig& getConfig();
        
        /**
         * Initializes library
         * 
         * Each process using the library MUST call this function exactly once
         * before using any library classes except for the LogConfig method.
         * 
         * @return true iff initialization was successful 
         */
        virtual bool init()=0;
        
        /**
         * Shuts down library
         * 
         * Each process using the library SHOULD call this function exactly once
         * before terminating itself
         */
        virtual void term()=0;

        /**
         * Loads a shared/dynamic library extension.
         * 
         * Extension libraries are managed using a pair of "C" linkage functions:<br>
         *      extern "C" int xmltooling_extension_init(void* context);<br>
         *      extern "C" void xmltooling_extension_term();
         * 
         * This method is internally synchronized.
         * 
         * @param path      pathname of shared library to load into process
         * @param context   arbitrary data to pass to library initialization hook
         * @return true iff library was loaded successfully
         */
        virtual bool load_library(const char* path, void* context=NULL)=0;
        
        /**
         * Configure logging system.
         * 
         * May be called first, before initializing the library. Other calls to it
         * must be externally synchronized. 
         * 
         * @param config    either a logging configuration file, or a level from the set
         *                  (DEBUG, INFO, NOTICE, WARN, ERROR, CRIT, ALERT, FATAL, EMERG)
         * @return true iff configuration was successful
         */
        virtual bool log_config(const char* config=NULL)=0;

        /**
         * Obtains a non-validating parser pool.
         * Library must be initialized first.
         *
         * @return reference to a non-validating parser pool.
         */
        virtual ParserPool& getParser() const=0;

        /**
         * Obtains a validating parser pool.
         * Library must be initialized first. Schema/catalog registration must be
         * externally synchronized.
         *
         * @return reference to a validating parser pool.
         */
        virtual ParserPool& getValidatingParser() const=0;
        
        /**
         * List of catalog files to load into validating parser pool at initialization time.
         * Like other path settings, the separator depends on the platform
         * (semicolon on Windows, colon otherwise). 
         */
        std::string catalog_path;
        
        /**
         * Adjusts any clock comparisons to be more liberal/permissive by the
         * indicated number of seconds.
         */
        unsigned int clock_skew_secs;

#ifndef XMLTOOLING_NO_XMLSEC
        /**
         * Returns an X.509 CRL implementation object.
         */
        virtual XSECCryptoX509CRL* X509CRL() const=0;

        /**
         * Manages factories for KeyResolver plugins.
         */
        PluginManager<xmlsignature::KeyResolver,const DOMElement*> KeyResolverManager;

        /**
         * Manages factories for CredentialResolver plugins.
         */
        PluginManager<xmlsignature::CredentialResolver,const DOMElement*> CredentialResolverManager;

        /**
         * Manages factories for TrustEngine plugins.
         */
        PluginManager<TrustEngine,const DOMElement*> TrustEngineManager;
#endif

        /**
         * Manages factories for StorageService plugins.
         */
        PluginManager<StorageService,const DOMElement*> StorageServiceManager;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_config_h__ */
