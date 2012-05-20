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
    setCacheDir("/var/cache");
}

PathResolver::~PathResolver()
{
}

void PathResolver::setDefaultPackageName(const char* pkgname)
{
    m_defaultPackage = pkgname;
}

void PathResolver::setDefaultPrefix(const char* prefix)
{
    m_defaultPrefix = prefix;
}

void PathResolver::setLibDir(const char* dir)
{
    m_lib = dir;
}

void PathResolver::setLogDir(const char* dir)
{
    m_log = dir;
}

void PathResolver::setXMLDir(const char* dir)
{
    m_xml = dir;
}

void PathResolver::setRunDir(const char* dir)
{
    m_run = dir;
}

void PathResolver::setCfgDir(const char* dir)
{
    m_cfg = dir;
}

void PathResolver::setCacheDir(const char* dir)
{
    m_cache = dir;
}

bool PathResolver::isAbsolute(const char* s) const
{
    switch (*s) {
        case 0:
            return false;
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
#ifdef WIN32
    // Check for possible environment variable(s).
    if (s.find('%') != string::npos) {
        char expbuf[MAX_PATH + 2];
        DWORD cnt = ExpandEnvironmentStrings(s.c_str(), expbuf, sizeof(expbuf));
        if (cnt != 0 && cnt <= sizeof(expbuf))
            s = expbuf;
    }
#endif

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

            case XMLTOOLING_CACHE_FILE:
                s = m_cache + '/' + (pkgname ? pkgname : m_defaultPackage) + '/' + s;
                if (!isAbsolute(m_cache.c_str())) {
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
