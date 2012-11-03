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
 * @file xmltooling/util/PathResolver.h
 *
 * Resolves local filenames into absolute pathnames.
 */

#if !defined(__xmltooling_pathres_h__)
#define __xmltooling_pathres_h__

#include <xmltooling/base.h>

#include <string>

namespace xmltooling {
    /**
     * Resolves local filenames into absolute pathnames.
     */
    class XMLTOOL_API PathResolver
    {
        MAKE_NONCOPYABLE(PathResolver);
    public:
        PathResolver();
        virtual ~PathResolver();

        /** Types of file resources to resolve. */
        enum file_type_t {
            XMLTOOLING_LIB_FILE,
            XMLTOOLING_LOG_FILE,
            XMLTOOLING_XML_FILE,
            XMLTOOLING_RUN_FILE,
            XMLTOOLING_CFG_FILE,
            XMLTOOLING_CACHE_FILE
        };

        /**
         * Set the default package to use when resolving files.
         *
         * @param pkgname name of default package to use
         */
        virtual void setDefaultPackageName(const char* pkgname);

        /**
         * Set the default installation prefix to use when resolving files.
         *
         * @param prefix name of default prefix to use
         */
        virtual void setDefaultPrefix(const char* prefix);

        /**
         * Set the lib directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the library directory to use
         */
        virtual void setLibDir(const char* dir);

        /**
         * Set the log directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the log directory to use
         */
        virtual void setLogDir(const char* dir);

        /**
         * Set the XML directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the XML directory to use
         */
        virtual void setXMLDir(const char* dir);

        /**
         * Set the run directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the run directory to use
         */
        virtual void setRunDir(const char* dir);

        /**
         * Set the config directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the config directory to use
         */
        virtual void setCfgDir(const char* dir);

        /**
         * Set the cache directory to use when resolving files.
         * <p>If relative, the default prefix will be prepended.
         *
         * @param dir    the cache directory to use
         */
        virtual void setCacheDir(const char* dir);

        /**
         * Changes the input filename into an absolute pathname to the same file.
         *
         * @param s         filename to resolve
         * @param filetype  type of file being resolved
         * @param pkgname   application package name to use in resolving the file (or nullptr for the default)
         * @param prefix    installation prefix to use in resolving the file (or nullptr for the default)
         *
         * @return a const reference to the input string
         */
        virtual const std::string& resolve(std::string& s, file_type_t filetype, const char* pkgname=nullptr, const char* prefix=nullptr) const;

    private:
        bool isAbsolute(const char* s) const;

        std::string m_defaultPackage,m_defaultPrefix,m_lib,m_log,m_xml,m_run,m_cfg,m_cache;
    };
};

#endif /* __xmltooling_pathres_h__ */
