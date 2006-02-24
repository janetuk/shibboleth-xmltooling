/*
 *  Copyright 2001-2005 Internet2
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

/*
 *  internal.h - internally visible classes
 */

#ifndef __xmltooling_internal_h__
#define __xmltooling_internal_h__

#ifdef WIN32
# define _CRT_SECURE_NO_DEPRECATE 1
# define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

// eventually we might be able to support autoconf via cygwin...
#if defined (_MSC_VER) || defined(__BORLANDC__)
# include "config_win32.h"
#else
# include "config.h"
#endif

#include "base.h"
#include "XMLToolingConfig.h"
#include "util/ParserPool.h"

#include <vector>

#define XMLTOOLING_LOGCAT "XMLTooling"

namespace xmltooling {
    
    class XMLToolingInternalConfig : public xmltooling::XMLToolingConfig
    {
    public:
        XMLToolingInternalConfig() : m_lock(NULL), m_parserPool(NULL) {}

        static XMLToolingInternalConfig& getInternalConfig();

        // global per-process setup and shutdown of runtime
        bool init();
        void term();

        // global mutex available to library applications
        xmltooling::ILockable& lock();
        void unlock();

        // configuration
        bool load_library(const char* path, void* context=NULL);
        bool log_config(const char* config=NULL);

        // internal parser pool
        xmltooling::ParserPool* m_parserPool;

    private:
        std::vector<void*> m_libhandles;
        void* m_lock;
        //XSECProvider* m_xsec;
        //PlugManager m_plugMgr;
    };

    // Template cleanup functors for use with for_each algorithm
    template<class T> struct cleanup
    {
        void operator()(T* ptr) {delete ptr;}
        void operator()(const T* ptr) {delete const_cast<T*>(ptr);}
    };

    template<class A,class B> struct cleanup_pair
    {
        void operator()(const std::pair<A,B*>& p) {delete p.second;}
    };
};

#endif /* __xmltooling_internal_h__ */
