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
 * PathResolver.cpp
 *
 * Resolves local filenames into absolute pathnames.
 */

#include "internal.h"
#include "exceptions.h"
#include "util/PathResolver.h"

using namespace xmltooling;
using namespace std;

PathResolver::PathResolver() : m_defaultPackage(PACKAGE_NAME), m_defaultPrefix("/usr")
{
    setLibDir("/usr/lib");
    setLogDir("/var/log");
    setXMLDir("/usr/share/xml");
    setRunDir("/var/run");
    setCfgDir("/etc");
}

bool PathResolver::isAbsolute(const char* s) const
{
    switch (*s) {
        case '/':
        case '\\':
            return true;
        case '.':
            return (*(s+1) == '.' || *(s+1) == '/' || *(s+1) == '\\');
    }
    return *(s+1) == ':';
}

const string& PathResolver::resolve(string& s, file_type_t filetype, const char* pkgname, const char* prefix) const
{
    if (!isAbsolute(s.c_str())) {
        switch (filetype) {
            case XMLTOOLING_LIB_FILE:
                s = m_lib + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_lib.c_str()))
                    s = string(prefix ? prefix : m_defaultPrefix) + '/' + s;
                break;

            case XMLTOOLING_LOG_FILE:
                s = m_log + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_log.c_str())) {
                    if (prefix || m_defaultPrefix != "/usr")
                        s = string(prefix ? prefix : m_defaultPrefix) + '/' + s;
                    else
                        s = string("/") + s;
                }
                break;

            case XMLTOOLING_XML_FILE:
                s = m_xml + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_xml.c_str()))
                    s = string(prefix ? prefix : m_defaultPrefix) + '/' + s;
                break;

            case XMLTOOLING_RUN_FILE:
                s = m_run + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_run.c_str())) {
                    if (prefix || m_defaultPrefix != "/usr")
                        s = string(prefix ? prefix : m_defaultPrefix) + '/' + s;
                    else
                        s = string("/") + s;
                }
                break;

            case XMLTOOLING_CFG_FILE:
                s = m_cfg + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_cfg.c_str())) {
                    if (prefix || m_defaultPrefix != "/usr")
                        s = string(prefix ? prefix : m_defaultPrefix) + '/' + s;
                    else
                        s = string("/") + s;
                }
                break;

            default:
                throw XMLToolingException("Unknown file type to resolve.");
        }
    }
    return s;
}
