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
    if (!isAbsolute(s.c_str())) {
        switch (filetype) {
            case XMLTOOLING_LIB_FILE:
                s = string(prefix ? prefix : m_defaultPrefix) + "/lib/" + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                break;
                
            case XMLTOOLING_LOG_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + "/var/log/" + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                else
                    s = string("/var/log/") + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                break;

            case XMLTOOLING_XML_FILE:
                s = string(prefix ? prefix : m_defaultPrefix) + "/share/xml/" + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                break;

            case XMLTOOLING_RUN_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + "/var/run/" + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                else
                    s = string("/var/run/") + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                break;

            case XMLTOOLING_CFG_FILE:
                if (prefix || m_defaultPrefix != "/usr")
                    s = string(prefix ? prefix : m_defaultPrefix) + "/etc/" + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                else
                    s = string("/etc/") + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                break;
            
            default:
                throw XMLToolingException("Unknown file type to resolve.");
        }
    }
    return s;
}
