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
 * XMLToolingConfig.cpp
 * 
 * Library configuration 
 */

#include "internal.h"
#include "exceptions.h"
#include "XMLToolingConfig.h"
#include "impl/UnknownElement.h"
#include "signature/impl/XMLSecSignature.h"
#include "util/NDC.h"

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/OstreamAppender.hh>
#include <xercesc/util/PlatformUtils.hpp>
#ifndef XMLTOOLING_NO_XMLSEC
    #include <xsec/framework/XSECProvider.hpp>
#endif

#include <stdexcept>

using namespace log4cpp;
using namespace xmltooling;
using namespace std;

DECL_EXCEPTION_FACTORY(XMLParserException);
DECL_EXCEPTION_FACTORY(XMLObjectException);
DECL_EXCEPTION_FACTORY(MarshallingException);
DECL_EXCEPTION_FACTORY(UnmarshallingException);
DECL_EXCEPTION_FACTORY(UnknownElementException);
DECL_EXCEPTION_FACTORY(UnknownAttributeException);
DECL_EXCEPTION_FACTORY(ValidationException);
DECL_EXCEPTION_FACTORY(SignatureException);

namespace {
   XMLToolingInternalConfig g_config;
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
        if (level)
            root.setAppender(new OstreamAppender("default",&cerr));
        else
            PropertyConfigurator::configure(config);
    }
    catch (const ConfigureFailure& e) {
        Category::getInstance(XMLTOOLING_LOGCAT".Logging").crit("failed to initialize log4cpp: %s", e.what());
        return false;
    }
    
    return true;
}

bool XMLToolingInternalConfig::init()
{
#ifdef _DEBUG
    xmltooling::NDC ndc("init");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLToolingConfig");
    try {
        log.debug("library initialization started");

        xercesc::XMLPlatformUtils::Initialize();
        log.debug("Xerces initialization complete");

#ifndef XMLTOOLING_NO_XMLSEC
        XSECPlatformUtils::Initialise();
        m_xsecProvider=new XSECProvider();
        log.debug("XMLSec initialization complete");
#endif

        m_parserPool=new ParserPool();
        m_lock=xercesc::XMLPlatformUtils::makeMutex();

        // default registrations
        XMLObjectBuilder::registerDefaultBuilder(new UnknownElementBuilder());
#ifndef XMLTOOLING_NO_XMLSEC
        XMLObjectBuilder::registerBuilder(QName(XMLConstants::XMLSIG_NS,Signature::LOCAL_NAME),new XMLSecSignatureBuilder());
#endif
        REGISTER_EXCEPTION_FACTORY(XMLParserException);
        REGISTER_EXCEPTION_FACTORY(XMLObjectException);
        REGISTER_EXCEPTION_FACTORY(MarshallingException);
        REGISTER_EXCEPTION_FACTORY(UnmarshallingException);
        REGISTER_EXCEPTION_FACTORY(UnknownElementException);
        REGISTER_EXCEPTION_FACTORY(UnknownAttributeException);
        REGISTER_EXCEPTION_FACTORY(ValidationException);
        REGISTER_EXCEPTION_FACTORY(SignatureException);
    }
    catch (const xercesc::XMLException&) {
        log.fatal("caught exception while initializing Xerces");
        return false;
    }

    log.info("library initialization complete");
    return true;
}

void XMLToolingInternalConfig::term()
{
    XMLObjectBuilder::destroyBuilders();

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

#ifndef XMLTOOLING_NO_XMLSEC
    delete m_xsecProvider;
    m_xsecProvider=NULL;
    XSECPlatformUtils::Terminate();
#endif

    xercesc::XMLPlatformUtils::closeMutex(m_lock);
    m_lock=NULL;
    xercesc::XMLPlatformUtils::Terminate();

 #ifdef _DEBUG
    xmltooling::NDC ndc("term");
#endif
   Category::getInstance(XMLTOOLING_LOGCAT".XMLToolingConfig").info("library shutdown complete");
}

ILockable& XMLToolingInternalConfig::lock()
{
    xercesc::XMLPlatformUtils::lockMutex(m_lock);
    return *this;
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

#if defined(WIN32)
    HMODULE handle=NULL;
    char* fixed=const_cast<char*>(path);
    if (strchr(fixed,'/')) {
        fixed=strdup(path);
        char* p=fixed;
        while (p=strchr(p,'/'))
            *p='\\';
    }

    UINT em=SetErrorMode(SEM_FAILCRITICALERRORS);
    try {
        handle=LoadLibraryEx(fixed,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!handle)
             handle=LoadLibraryEx(fixed,NULL,0);
        if (!handle)
            throw runtime_error(string("unable to load extension library: ") + fixed);
        FARPROC fn=GetProcAddress(handle,"xmltooling_extension_init");
        if (!fn)
            throw runtime_error(string("unable to locate xmltooling_extension_init entry point: ") + fixed);
        if (reinterpret_cast<int(*)(void*)>(fn)(context)!=0)
            throw runtime_error(string("detected error in xmltooling_extension_init: ") + fixed);
        if (fixed!=path)
            free(fixed);
        SetErrorMode(em);
    }
    catch(runtime_error& e) {
        log.error(e.what());
        if (handle)
            FreeLibrary(handle);
        SetErrorMode(em);
        if (fixed!=path)
            free(fixed);
        return false;
    }

#elif defined(HAVE_DLFCN_H)
    void* handle=dlopen(path,RTLD_LAZY);
    if (!handle)
        throw runtime_error(string("unable to load extension library '") + path + "': " + dlerror());
    int (*fn)(void*)=(int (*)(void*))(dlsym(handle,"xmltooling_extension_init"));
    if (!fn) {
        dlclose(handle);
        throw runtime_error(
            string("unable to locate xmltooling_extension_init entry point in '") + path + "': " +
                (dlerror() ? dlerror() : "unknown error")
            );
    }
    try {
        if (fn(context)!=0)
            throw runtime_error(string("detected error in xmltooling_extension_init in ") + path);
    }
    catch(runtime_error& e) {
        log.error(e.what());
        if (handle)
            dlclose(handle);
        return false;
    }
#else
# error "Don't know about dynamic loading on this platform!"
#endif
    m_libhandles.push_back(handle);
    log.info("loaded extension: %s", path);
    return true;
}
