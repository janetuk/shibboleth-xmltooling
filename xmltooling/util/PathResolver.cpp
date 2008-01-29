/*
 *  Copyright 2001-2007 Internet2
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
 * PathResolver.cpp
 * 
 * Resolves local filenames into absolute pathnames. 
 */

#include "internal.h"
#include "exceptions.h"
#include "util/PathResolver.h"

using namespace xmltooling;
using namespace std;

const string& PathResolver::resolve(string& s, file_type_t filetype, const char* pkgname, const char* prefix) const
{
#ifdef WIN32
    static const char sep = '\\';
#else
    static const char sep = '/';
#endif
    if (!isAbsolute(s.c_str())) {
        switch (filetype) {
            case XMLTOOLING_LIB_FILE:
                s = string(prefix ? prefix : m_defaultPrefix) + sep + "lib" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                break;
                
            case XMLTOOLING_LOG_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + sep + "var" + sep + "log" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                else
                    s = string(sep,1) + "var" + sep + "log" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                break;

            case XMLTOOLING_XML_FILE:
                s = string(prefix ? prefix : m_defaultPrefix) + sep + "share" + sep + "xml" + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                break;

            case XMLTOOLING_RUN_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + sep + "var" + sep + "run" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                else
                    s = string(sep,1) + "var" + sep + "run" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                break;

            case XMLTOOLING_CFG_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + sep + "etc" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                else
                    s = string(sep,1) + "etc" + sep + (pkgname ? pkgname : m_defaultPackage) + sep + s;
                break;
            
            default:
                throw XMLToolingException("Unknown file type to resolve.");
        }
    }
    return s;
}
