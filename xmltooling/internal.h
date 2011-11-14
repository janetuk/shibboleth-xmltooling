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

/*
 *  internal.h - internally visible classes
 */

#ifndef __xmltooling_internal_h__
#define __xmltooling_internal_h__

#ifdef WIN32
# define _CRT_SECURE_NO_DEPRECATE 1
# define _CRT_NONSTDC_NO_DEPRECATE 1
# define _SCL_SECURE_NO_WARNINGS 1
#endif

// Export public APIs.
#define XMLTOOLING_EXPORTS

// eventually we might be able to support autoconf via cygwin...
#if defined (_MSC_VER) || defined(__BORLANDC__)
# include "config_win32.h"
#else
# include "config.h"
#endif

#include "base.h"
#include "XMLToolingConfig.h"
#include "util/ParserPool.h"

#include <map>
#include <string>
#include <vector>

#ifndef XMLTOOLING_NO_XMLSEC
    #include <xsec/framework/XSECProvider.hpp>
#endif

#define XMLTOOLING_LOGCAT "XMLTooling"

// Macros for path and directory separators.
#if defined __CYGWIN32__ && !defined __CYGWIN__
   /* For backwards compatibility with Cygwin b19 and
      earlier, we define __CYGWIN__ here, so that
      we can rely on checking just for that macro. */
#  define __CYGWIN__  __CYGWIN32__
#endif

#if defined _WIN32 && !defined __CYGWIN__
   /* Use Windows separators on all _WIN32 defining
      environments, except Cygwin. */
#  define DIR_SEPARATOR_CHAR        '\\'
#  define DIR_SEPARATOR_STR         "\\"
#  define PATH_SEPARATOR_CHAR       ';'
#  define PATH_SEPARATOR_STR        ";"
#endif
#ifndef DIR_SEPARATOR_CHAR
   /* Assume that not having this is an indicator that all
      are missing. */
#  define DIR_SEPARATOR_CHAR        '/'
#  define DIR_SEPARATOR_STR         "/"
#  define PATH_SEPARATOR_CHAR       ':'
#  define PATH_SEPARATOR_STR        ":"
#endif /* !DIR_SEPARATOR_CHAR */

namespace xmltooling {
    
    /// @cond OFF
    class XMLTOOL_DLLLOCAL XMLToolingInternalConfig : public XMLToolingConfig
    {
    public:
        XMLToolingInternalConfig();
        ~XMLToolingInternalConfig();

        static XMLToolingInternalConfig& getInternalConfig();

        // global per-process setup and shutdown of runtime
        bool init();
        void term();

        // global mutex available to library applications
        Lockable* lock();
        void unlock();

        // named mutexes to limit lock scope
        Mutex& getNamedMutex(const char* name);

        // configuration
        bool load_library(const char* path, void* context=nullptr);
        bool log_config(const char* config=nullptr);

        // parser access
        ParserPool& getParser() const {
            return *m_parserPool;
        }

        ParserPool& getValidatingParser() const {
            return *m_validatingPool;
        }

#ifndef XMLTOOLING_NO_XMLSEC
        XSECCryptoX509CRL* X509CRL() const;
        std::pair<const char*,unsigned int> mapXMLAlgorithmToKeyAlgorithm(const XMLCh* xmlAlgorithm) const;
        void registerXMLAlgorithm(
            const XMLCh* xmlAlgorithm, const char* keyAlgorithm, unsigned int size=0, XMLSecurityAlgorithmType type=ALGTYPE_UNK
            );
        bool isXMLAlgorithmSupported(const XMLCh* xmlAlgorithm, XMLSecurityAlgorithmType type=ALGTYPE_UNK);
        void registerXMLAlgorithms();

        XSECProvider* m_xsecProvider;
    private:
        typedef std::map<XMLSecurityAlgorithmType, std::map< xstring,std::pair<std::string,unsigned int> > > algmap_t;
        algmap_t m_algorithmMap;
#endif

    private:
        int m_initCount;
        Mutex* m_lock;
        std::map<std::string,Mutex*> m_namedLocks;
        std::vector<void*> m_libhandles;
        ParserPool* m_parserPool;
        ParserPool* m_validatingPool;
    };
    
#ifndef XMLTOOLING_NO_XMLSEC
    void log_openssl();
#endif
    
    /// @endcond

};

#endif /* __xmltooling_internal_h__ */
