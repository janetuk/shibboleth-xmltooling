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
 * XMLToolingConfig.cpp
 *
 * Library configuration
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "XMLToolingConfig.h"
#include "encryption/Encryption.h"
#include "encryption/Encrypter.h"
#include "io/HTTPRequest.h"
#include "io/HTTPResponse.h"
#include "impl/UnknownElement.h"
#include "security/TrustEngine.h"
#include "security/OpenSSLCryptoX509CRL.h"
#include "security/CredentialResolver.h"
#include "security/KeyInfoResolver.h"
#include "signature/Signature.h"
#include "soap/SOAP.h"
#include "soap/SOAPTransport.h"
#include "util/NDC.h"
#include "util/PathResolver.h"
#include "util/ReplayCache.h"
#include "util/StorageService.h"
#include "util/TemplateEngine.h"
#include "util/URLEncoder.h"
#include "util/XMLConstants.h"
#include "validation/ValidatorSuite.h"

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#include <stdexcept>
#if defined(XMLTOOLING_LOG4SHIB)
# include <log4shib/PropertyConfigurator.hh>
# include <log4shib/OstreamAppender.hh>
#elif defined(XMLTOOLING_LOG4CPP)
# include <log4cpp/PropertyConfigurator.hh>
# include <log4cpp/OstreamAppender.hh>
#endif
#include <xercesc/util/PlatformUtils.hpp>
#ifndef XMLTOOLING_NO_XMLSEC
# include <curl/curl.h>
# include <openssl/err.h>
# include <xsec/framework/XSECProvider.hpp>
#endif

using namespace soap11;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

using xercesc::XMLPlatformUtils;

DECL_XMLTOOLING_EXCEPTION_FACTORY(XMLParserException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(XMLObjectException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(MarshallingException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(UnmarshallingException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(UnknownElementException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(UnknownAttributeException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(UnknownExtensionException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(ValidationException,xmltooling);
DECL_XMLTOOLING_EXCEPTION_FACTORY(IOException,xmltooling);

#ifndef XMLTOOLING_NO_XMLSEC
using namespace xmlencryption;
using namespace xmlsignature;
    DECL_XMLTOOLING_EXCEPTION_FACTORY(XMLSecurityException,xmltooling);
    DECL_XMLTOOLING_EXCEPTION_FACTORY(SignatureException,xmlsignature);
    DECL_XMLTOOLING_EXCEPTION_FACTORY(EncryptionException,xmlencryption);
#endif

namespace xmltooling {
    static XMLToolingInternalConfig g_config;
#ifndef XMLTOOLING_NO_XMLSEC
    static vector<Mutex*> g_openssl_locks;

    extern "C" void openssl_locking_callback(int mode,int n,const char *file,int line)
    {
        if (mode & CRYPTO_LOCK)
            g_openssl_locks[n]->lock();
        else
            g_openssl_locks[n]->unlock();
    }

# ifndef WIN32
    extern "C" unsigned long openssl_thread_id(void)
    {
        return (unsigned long)(pthread_self());
    }
# endif
#endif

#ifdef WIN32
    BOOL LogEvent(
        LPCSTR  lpUNCServerName,
        WORD  wType,
        DWORD  dwEventID,
        PSID  lpUserSid,
        LPCSTR  message)
    {
        LPCSTR  messages[] = {message, NULL};

        HANDLE hElog = RegisterEventSource(lpUNCServerName, "OpenSAML XMLTooling Library");
        BOOL res = ReportEvent(hElog, wType, 0, dwEventID, lpUserSid, 1, 0, messages, NULL);
        return (DeregisterEventSource(hElog) && res);
    }
#endif
}

XMLToolingConfig& XMLToolingConfig::getConfig()
{
    return g_config;
}

XMLToolingInternalConfig& XMLToolingInternalConfig::getInternalConfig()
{
    return g_config;
}

bool XMLToolingInternalConfig::log_config(const char* config)
{
    try {
        if (!config || !*config)
            config=getenv("XMLTOOLING_LOG_CONFIG");
        if (!config || !*config)
            config="WARN";

        bool level=false;
        Category& root = Category::getRoot();
        if (!strcmp(config,"DEBUG")) {
            root.setPriority(Priority::DEBUG);
            level=true;
        }
        else if (!strcmp(config,"INFO")) {
            root.setPriority(Priority::INFO);
            level=true;
        }
        else if (!strcmp(config,"NOTICE")) {
            root.setPriority(Priority::NOTICE);
            level=true;
        }
        else if (!strcmp(config,"WARN")) {
            root.setPriority(Priority::WARN);
            level=true;
        }
        else if (!strcmp(config,"ERROR")) {
            root.setPriority(Priority::ERROR);
            level=true;
        }
        else if (!strcmp(config,"CRIT")) {
            root.setPriority(Priority::CRIT);
            level=true;
        }
        else if (!strcmp(config,"ALERT")) {
            root.setPriority(Priority::ALERT);
            level=true;
        }
        else if (!strcmp(config,"EMERG")) {
            root.setPriority(Priority::EMERG);
            level=true;
        }
        else if (!strcmp(config,"FATAL")) {
            root.setPriority(Priority::FATAL);
            level=true;
        }
        if (level) {
            root.setAppender(new OstreamAppender("default",&cerr));
        }
        else {
            string path(config);
            PropertyConfigurator::configure(m_pathResolver ? m_pathResolver->resolve(path, PathResolver::XMLTOOLING_CFG_FILE) : path);
        }
    }
    catch (const ConfigureFailure& e) {
        string msg = string("failed to configure logging: ") + e.what();
        Category::getInstance(XMLTOOLING_LOGCAT".Logging").crit(msg);
#ifdef WIN32
        LogEvent(NULL, EVENTLOG_ERROR_TYPE, 2100, NULL, msg.c_str());
#endif
        return false;
    }

    return true;
}

#ifndef XMLTOOLING_LITE
void XMLToolingConfig::setReplayCache(ReplayCache* replayCache)
{
    delete m_replayCache;
    m_replayCache = replayCache;
}
#endif

void XMLToolingConfig::setPathResolver(PathResolver* pathResolver)
{
    delete m_pathResolver;
    m_pathResolver = pathResolver;
}

void XMLToolingConfig::setTemplateEngine(TemplateEngine* templateEngine)
{
    delete m_templateEngine;
    m_templateEngine = templateEngine;
}

void XMLToolingConfig::setURLEncoder(URLEncoder* urlEncoder)
{
    delete m_urlEncoder;
    m_urlEncoder = urlEncoder;
}

bool XMLToolingInternalConfig::init()
{
#ifdef _DEBUG
    xmltooling::NDC ndc("init");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLToolingConfig");
    try {
        log.debug("library initialization started");

#ifndef XMLTOOLING_NO_XMLSEC
        if (curl_global_init(CURL_GLOBAL_ALL)) {
            log.fatal("failed to initialize libcurl, OpenSSL, or Winsock");
            return false;
        }
        log.debug("libcurl %s initialization complete", LIBCURL_VERSION);
#endif

        XMLPlatformUtils::Initialize();
        log.debug("Xerces %s initialization complete", XERCES_FULLVERSIONDOT);

#ifndef XMLTOOLING_NO_XMLSEC
        XSECPlatformUtils::Initialise();
        m_xsecProvider=new XSECProvider();
        log.debug("XML-Security %s initialization complete", XSEC_FULLVERSIONDOT);
#endif

        m_parserPool=new ParserPool();
        m_validatingPool=new ParserPool(true,true);
        m_lock=XMLPlatformUtils::makeMutex();

        // Load catalogs from path.
        if (!catalog_path.empty()) {
            char* catpath=strdup(catalog_path.c_str());
            char* sep=NULL;
            char* start=catpath;
            while (start && *start) {
                sep=strchr(start,PATH_SEPARATOR_CHAR);
                if (sep)
                    *sep=0;
                auto_ptr_XMLCh temp(start);
                m_validatingPool->loadCatalog(temp.get());
                start = sep ? sep + 1 : NULL;
            }
            free(catpath);
        }

        // default registrations
        XMLObjectBuilder::registerDefaultBuilder(new UnknownElementBuilder());

        registerSOAPClasses();

        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(XMLParserException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(XMLObjectException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(MarshallingException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(UnmarshallingException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(UnknownElementException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(UnknownAttributeException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(ValidationException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(IOException,xmltooling);

#ifndef XMLTOOLING_NO_XMLSEC
        XMLObjectBuilder::registerBuilder(QName(xmlconstants::XMLSIG_NS,Signature::LOCAL_NAME),new SignatureBuilder());
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(XMLSecurityException,xmltooling);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(SignatureException,xmlsignature);
        REGISTER_XMLTOOLING_EXCEPTION_FACTORY(EncryptionException,xmlencryption);
        registerKeyInfoClasses();
        registerEncryptionClasses();
        registerKeyInfoResolvers();
        registerCredentialResolvers();
        registerTrustEngines();
        registerXMLAlgorithms();
        registerSOAPTransports();
        initSOAPTransports();
        registerStorageServices();
        m_keyInfoResolver = KeyInfoResolverManager.newPlugin(INLINE_KEYINFO_RESOLVER,NULL);
#endif

        m_pathResolver = new PathResolver();
        m_urlEncoder = new URLEncoder();

        // Register xml:id as an ID attribute.
        static const XMLCh xmlid[] = UNICODE_LITERAL_2(i,d);
        AttributeExtensibleXMLObject::registerIDAttribute(QName(xmlconstants::XML_NS, xmlid));
    }
    catch (const xercesc::XMLException&) {
        log.fatal("caught exception while initializing Xerces");
#ifndef XMLTOOLING_NO_XMLSEC
        curl_global_cleanup();
#endif
        return false;
    }

#ifndef XMLTOOLING_NO_XMLSEC
    // Set up OpenSSL locking.
    for (int i=0; i<CRYPTO_num_locks(); i++)
        g_openssl_locks.push_back(Mutex::create());
    CRYPTO_set_locking_callback(openssl_locking_callback);
# ifndef WIN32
    CRYPTO_set_id_callback(openssl_thread_id);
# endif
#endif

    log.info("%s library initialization complete", PACKAGE_STRING);
    return true;
}

void XMLToolingInternalConfig::term()
{
#ifndef XMLTOOLING_NO_XMLSEC
    CRYPTO_set_locking_callback(NULL);
    for_each(g_openssl_locks.begin(), g_openssl_locks.end(), xmltooling::cleanup<Mutex>());
    g_openssl_locks.clear();
#endif

    SchemaValidators.destroyValidators();
    XMLObjectBuilder::destroyBuilders();
    XMLToolingException::deregisterFactories();
    AttributeExtensibleXMLObject::deregisterIDAttributes();

#ifndef XMLTOOLING_NO_XMLSEC
    StorageServiceManager.deregisterFactories();
    termSOAPTransports();
    SOAPTransportManager.deregisterFactories();
    TrustEngineManager.deregisterFactories();
    CredentialResolverManager.deregisterFactories();
    KeyInfoResolverManager.deregisterFactories();
    m_algorithmMap.clear();

    delete m_keyInfoResolver;
    m_keyInfoResolver = NULL;

    delete m_replayCache;
    m_replayCache = NULL;
#endif

    delete m_pathResolver;
    m_pathResolver = NULL;

    delete m_templateEngine;
    m_templateEngine = NULL;

    delete m_urlEncoder;
    m_urlEncoder = NULL;

    for (vector<void*>::reverse_iterator i=m_libhandles.rbegin(); i!=m_libhandles.rend(); i++) {
#if defined(WIN32)
        FARPROC fn=GetProcAddress(static_cast<HMODULE>(*i),"xmltooling_extension_term");
        if (fn)
            fn();
        FreeLibrary(static_cast<HMODULE>(*i));
#elif defined(HAVE_DLFCN_H)
        void (*fn)()=(void (*)())dlsym(*i,"xmltooling_extension_term");
        if (fn)
            fn();
        dlclose(*i);
#else
# error "Don't know about dynamic loading on this platform!"
#endif
    }
    m_libhandles.clear();

    delete m_parserPool;
    m_parserPool=NULL;
    delete m_validatingPool;
    m_validatingPool=NULL;

#ifndef XMLTOOLING_NO_XMLSEC
    delete m_xsecProvider;
    m_xsecProvider=NULL;
    XSECPlatformUtils::Terminate();
#endif

    XMLPlatformUtils::closeMutex(m_lock);
    m_lock=NULL;
    XMLPlatformUtils::Terminate();

#ifndef XMLTOOLING_NO_XMLSEC
    curl_global_cleanup();
#endif
#ifdef _DEBUG
    xmltooling::NDC ndc("term");
#endif
   Category::getInstance(XMLTOOLING_LOGCAT".XMLToolingConfig").info("%s library shutdown complete", PACKAGE_STRING);
}

Lockable* XMLToolingInternalConfig::lock()
{
    xercesc::XMLPlatformUtils::lockMutex(m_lock);
    return this;
}

void XMLToolingInternalConfig::unlock()
{
    xercesc::XMLPlatformUtils::unlockMutex(m_lock);
}

bool XMLToolingInternalConfig::load_library(const char* path, void* context)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("LoadLibrary");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLToolingConfig");
    log.info("loading extension: %s", path);

    Locker locker(this);

    string resolved(path);
    m_pathResolver->resolve(resolved, PathResolver::XMLTOOLING_LIB_FILE);

#if defined(WIN32)
    HMODULE handle=NULL;
    for (string::iterator i = resolved.begin(); i != resolved.end(); ++i)
        if (*i == '/')
            *i = '\\';

    UINT em=SetErrorMode(SEM_FAILCRITICALERRORS);
    try {
        handle=LoadLibraryEx(resolved.c_str(),NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!handle)
             handle=LoadLibraryEx(resolved.c_str(),NULL,0);
        if (!handle)
            throw runtime_error(string("unable to load extension library: ") + resolved);
        FARPROC fn=GetProcAddress(handle,"xmltooling_extension_init");
        if (!fn)
            throw runtime_error(string("unable to locate xmltooling_extension_init entry point: ") + resolved);
        if (reinterpret_cast<int(*)(void*)>(fn)(context)!=0)
            throw runtime_error(string("detected error in xmltooling_extension_init: ") + resolved);
        SetErrorMode(em);
    }
    catch(exception&) {
        if (handle)
            FreeLibrary(handle);
        SetErrorMode(em);
        throw;
    }

#elif defined(HAVE_DLFCN_H)
    void* handle=dlopen(resolved.c_str(),RTLD_LAZY);
    if (!handle)
        throw runtime_error(string("unable to load extension library '") + resolved + "': " + dlerror());
    int (*fn)(void*)=(int (*)(void*))(dlsym(handle,"xmltooling_extension_init"));
    if (!fn) {
        dlclose(handle);
        throw runtime_error(
            string("unable to locate xmltooling_extension_init entry point in '") + resolved + "': " +
                (dlerror() ? dlerror() : "unknown error")
            );
    }
    try {
        if (fn(context)!=0)
            throw runtime_error(string("detected error in xmltooling_extension_init in ") + resolved);
    }
    catch(exception&) {
        if (handle)
            dlclose(handle);
        throw;
    }
#else
# error "Don't know about dynamic loading on this platform!"
#endif
    m_libhandles.push_back(handle);
    log.info("loaded extension: %s", resolved.c_str());
    return true;
}

#ifndef XMLTOOLING_NO_XMLSEC
void xmltooling::log_openssl()
{
    const char* file;
    const char* data;
    int flags,line;

    unsigned long code=ERR_get_error_line_data(&file,&line,&data,&flags);
    while (code) {
        Category& log=Category::getInstance("OpenSSL");
        log.errorStream() << "error code: " << code << " in " << file << ", line " << line << logging::eol;
        if (data && (flags & ERR_TXT_STRING))
            log.errorStream() << "error data: " << data << logging::eol;
        code=ERR_get_error_line_data(&file,&line,&data,&flags);
    }
}

XSECCryptoX509CRL* XMLToolingInternalConfig::X509CRL() const
{
    return new OpenSSLCryptoX509CRL();
}

void XMLToolingInternalConfig::registerXMLAlgorithms()
{
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_MD5, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_SHA1, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_SHA224, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_SHA256, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_SHA384, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_SHA512, "RSA", 0);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_1_5, "RSA", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIRSA_OAEP_MGFP1, "RSA", 0);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIDSA_SHA1, "DSA", 0);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIHMAC_SHA1, "HMAC", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIHMAC_SHA224, "HMAC", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIHMAC_SHA256, "HMAC", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIHMAC_SHA384, "HMAC", 0);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIHMAC_SHA512, "HMAC", 0);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURI3DES_CBC, "DESede", 192);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIKW_3DES, "DESede", 192);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIAES128_CBC, "AES", 128);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIKW_AES128, "AES", 128);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIAES192_CBC, "AES", 192);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIKW_AES192, "AES", 192);

    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIAES256_CBC, "AES", 256);
    registerXMLAlgorithm(DSIGConstants::s_unicodeStrURIKW_AES256, "AES", 256);
}
#endif

#ifdef WIN32

extern "C" __declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_THREAD_DETACH || fdwReason == DLL_PROCESS_DETACH)
        ThreadKey::onDetach();
    return TRUE;
}

#endif
