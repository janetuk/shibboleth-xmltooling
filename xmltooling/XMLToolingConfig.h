/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * @file xmltooling/XMLToolingConfig.h
 * 
 * Library configuration.
 */

#ifndef __xmltooling_config_h__
#define __xmltooling_config_h__

#include <xmltooling/Lockable.h>
#include <xmltooling/PluginManager.h>
#include <xmltooling/soap/SOAPTransport.h>

#include <string>
#include <xercesc/dom/DOM.hpp>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

namespace xmltooling {
    
    class XMLTOOL_API Mutex;
    class XMLTOOL_API ParserPool;
    class XMLTOOL_API PathResolver;
    class XMLTOOL_API TemplateEngine;
    class XMLTOOL_API URLEncoder;
#ifndef XMLTOOLING_LITE
    class XMLTOOL_API ReplayCache;
    class XMLTOOL_API StorageService;
#endif
#ifndef XMLTOOLING_NO_XMLSEC
    class XMLTOOL_API CredentialResolver;
    class XMLTOOL_API KeyInfoResolver;
    class XMLTOOL_API PathValidator;
    class XMLTOOL_API TrustEngine;
    class XMLTOOL_API XSECCryptoX509CRL;
#endif

    /**
     * Singleton object that manages library startup/shutdown.configuration.
     * 
     * <p>A locking interface is supplied as a convenience for code that wants to
     * obtain a global system lock, but the actual configuration itself is not
     * synchronized.
     */
    class XMLTOOL_API XMLToolingConfig : public virtual Lockable
    {
        MAKE_NONCOPYABLE(XMLToolingConfig);
    protected:
        XMLToolingConfig();

#ifndef XMLTOOLING_NO_XMLSEC
        /** Global KeyInfoResolver instance. */
        KeyInfoResolver* m_keyInfoResolver;

        /** Global ReplayCache instance. */
        ReplayCache* m_replayCache;
#endif

        /** Global PathResolver instance. */
        PathResolver* m_pathResolver;
        
        /** Global TemplateEngine instance. */
        TemplateEngine* m_templateEngine;

        /** Global URLEncoder instance for use by URL-related functions. */
        URLEncoder* m_urlEncoder;

    public:
        virtual ~XMLToolingConfig();

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
         * <p>Each process using the library SHOULD call this function exactly once
         * before terminating itself
         */
        virtual void term()=0;

        /**
         * Loads a shared/dynamic library extension.
         *
         * <p>Extension libraries are managed using a pair of "C" linkage functions:<br>
         *      extern "C" int xmltooling_extension_init(void* context);<br>
         *      extern "C" void xmltooling_extension_term();
         *
         * <p>This method is internally synchronized.
         * 
         * @param path      pathname of shared library to load into process
         * @param context   arbitrary data to pass to library initialization hook
         * @return true iff library was loaded successfully
         */
        virtual bool load_library(const char* path, void* context=nullptr)=0;
        
        /**
         * Configure logging system.
         * <p>May be called first, before initializing the library. Other calls to it
         * must be externally synchronized. 
         * 
         * @param config    either a logging configuration file, or a level from the set
         *                  (DEBUG, INFO, NOTICE, WARN, ERROR, CRIT, ALERT, FATAL, EMERG)
         * @return true iff configuration was successful
         */
        virtual bool log_config(const char* config=nullptr)=0;

        /**
         * Obtains a non-validating parser pool.
         * <p>Library must be initialized first.
         *
         * @return reference to a non-validating parser pool.
         */
        virtual ParserPool& getParser() const=0;

        /**
         * Obtains a validating parser pool.
         * <p>Library must be initialized first. Schema/catalog registration must be
         * externally synchronized.
         *
         * @return reference to a validating parser pool.
         */
        virtual ParserPool& getValidatingParser() const=0;

        /**
         * Returns a reference to a named mutex.
         * <p>The first access to a given name will create the object.
         *
         * @param name  name of mutex to access
         * @return  reference to a mutex object
         */
        virtual Mutex& getNamedMutex(const char* name)=0;

#ifndef XMLTOOLING_NO_XMLSEC
        /**
         * Returns the global KeyInfoResolver instance.
         * 
         * @return  global KeyInfoResolver or nullptr
         */
        const KeyInfoResolver* getKeyInfoResolver() const;

        /**
         * Returns the global ReplayCache instance.
         * 
         * @return  global ReplayCache or nullptr
         */
        ReplayCache* getReplayCache() const;

        /**
         * Sets the global KeyInfoResolver instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param keyInfoResolver   new KeyInfoResolver instance to store
         */
        void setKeyInfoResolver(KeyInfoResolver* keyInfoResolver);

        /**
         * Sets the global ReplayCache instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param replayCache   new ReplayCache instance to store
         */
        void setReplayCache(ReplayCache* replayCache);
#endif

        /**
         * Returns the global PathResolver instance.
         * 
         * @return  global PathResolver or nullptr
         */
        PathResolver* getPathResolver() const;
        
        /**
         * Returns the global TemplateEngine instance.
         * 
         * @return  global TemplateEngine or nullptr
         */
        TemplateEngine* getTemplateEngine() const;

        /**
         * Returns the global URLEncoder instance.
         * 
         * @return  global URLEncoder or nullptr
         */
        const URLEncoder* getURLEncoder() const;

        /**
         * Sets the global PathResolver instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param pathResolver   new PathResolver instance to store
         */
        void setPathResolver(PathResolver* pathResolver);
        
        /**
         * Sets the global TemplateEngine instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param templateEngine   new TemplateEngine instance to store
         */
        void setTemplateEngine(TemplateEngine* templateEngine);

        /**
         * Sets the global URLEncoder instance.
         * <p>This method must be externally synchronized with any code that uses the object.
         * Any previously set object is destroyed.
         * 
         * @param urlEncoder   new URLEncoder instance to store
         */
        void setURLEncoder(URLEncoder* urlEncoder);
        
        /**
         * @deprecated
         * List of catalog files to load into validating parser pool at initialization time.
         * <p>Like other path settings, the separator depends on the platform
         * (semicolon on Windows, colon otherwise). 
         */
        std::string catalog_path;

        /** A User-Agent header to include in HTTP client requests. */
        std::string user_agent;

        /**
         * Adjusts any clock comparisons to be more liberal/permissive by the
         * indicated number of seconds.
         */
        unsigned int clock_skew_secs;

#ifndef XMLTOOLING_LITE
        /**
         * Manages factories for StorageService plugins.
         */
        PluginManager<StorageService,std::string,const xercesc::DOMElement*> StorageServiceManager;
#endif

#ifndef XMLTOOLING_NO_XMLSEC
        /**
         * Returns an X.509 CRL implementation object.
         */
        virtual XSECCryptoX509CRL* X509CRL() const=0;

        /**
         * Manages factories for CredentialResolver plugins.
         */
        PluginManager<CredentialResolver,std::string,const xercesc::DOMElement*> CredentialResolverManager;

        /**
         * Manages factories for KeyInfoResolver plugins.
         */
        PluginManager<KeyInfoResolver,std::string,const xercesc::DOMElement*> KeyInfoResolverManager;

        /**
         * Manages factories for PathValidator plugins.
         */
        PluginManager<PathValidator,std::string,const xercesc::DOMElement*> PathValidatorManager;

        /**
         * Manages factories for TrustEngine plugins.
         */
        PluginManager<TrustEngine,std::string,const xercesc::DOMElement*> TrustEngineManager;

        /**
         * Maps an XML Signature/Encryption algorithm identifier to a library-specific
         * key algorithm and size for use in resolving credentials.
         *
         * @param xmlAlgorithm  XML Signature/Encryption algorithm identifier
         * @return  a general key algorithm and key size (or 0 if the size is irrelevant)
         */
        virtual std::pair<const char*,unsigned int> mapXMLAlgorithmToKeyAlgorithm(const XMLCh* xmlAlgorithm) const=0;

        /**
         * Types of XML Security algorithms.
         */
        enum XMLSecurityAlgorithmType {
            ALGTYPE_UNK,
            ALGTYPE_DIGEST,
            ALGTYPE_SIGN,
            ALGTYPE_ENCRYPT,
            ALGTYPE_KEYENCRYPT,
            ALGTYPE_KEYAGREE,
            ALGTYPE_AUTHNENCRYPT
        };

        /**
         * Registers an XML Signature/Encryption algorithm identifier against a library-specific
         * key algorithm and size for use in resolving credentials.
         *
         * @param xmlAlgorithm  XML Signature/Encryption algorithm identifier
         * @param keyAlgorithm  a key algorithm
         * @param size          a key size (or 0 if the size is irrelevant)
         * @param type          type of algorithm, if known
         */
        virtual void registerXMLAlgorithm(
            const XMLCh* xmlAlgorithm, const char* keyAlgorithm, unsigned int size=0, XMLSecurityAlgorithmType type=ALGTYPE_UNK
            )=0;

        /**
         * Checks for implementation support of a particular XML Security algorithm.
         *
         * @param xmlAlgorithm  XML Signature/Encryption algorithm identifier
         * @param type          type of algorithm, or ALGTYPE_UNK to ignore
         * @return  true iff the algorithm is supported by the underlying libraries
         */
        virtual bool isXMLAlgorithmSupported(const XMLCh* xmlAlgorithm, XMLSecurityAlgorithmType type=ALGTYPE_UNK)=0;
#endif

        /**
         * Manages factories for SOAPTransport plugins.
         * 
         * <p>The factory interface takes a peer name/endpoint pair.
         */
        PluginManager<SOAPTransport,std::string,SOAPTransport::Address> SOAPTransportManager;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_config_h__ */
