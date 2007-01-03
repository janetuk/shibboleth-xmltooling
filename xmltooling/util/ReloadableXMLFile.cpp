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

/**
 * @file ReloadableXMLFile.cpp
 * 
 * Base class for file-based XML configuration.
 */

#include "internal.h"
#include "util/NDC.h"
#include "util/ReloadableXMLFile.h"
#include "util/XMLConstants.h"
#include "util/XMLHelper.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <log4cpp/Category.hh>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

static const XMLCh uri[] =          UNICODE_LITERAL_3(u,r,i);
static const XMLCh url[] =          UNICODE_LITERAL_3(u,r,l);
static const XMLCh path[] =         UNICODE_LITERAL_4(p,a,t,h);
static const XMLCh pathname[] =     UNICODE_LITERAL_8(p,a,t,h,n,a,m,e);
static const XMLCh file[] =         UNICODE_LITERAL_4(f,i,l,e);
static const XMLCh filename[] =     UNICODE_LITERAL_8(f,i,l,e,n,a,m,e);
static const XMLCh validate[] =     UNICODE_LITERAL_8(v,a,l,i,d,a,t,e);
static const XMLCh reloadChanges[] =UNICODE_LITERAL_13(r,e,l,o,a,d,C,h,a,n,g,e,s);

ReloadableXMLFile::ReloadableXMLFile(const DOMElement* e)
    : m_root(e), m_local(true), m_validate(false), m_filestamp(0), m_lock(NULL)
{
#ifdef _DEBUG
    NDC ndc("ReloadableXMLFile");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ReloadableXMLFile");

    // Establish source of data...
    const XMLCh* source=e->getAttributeNS(NULL,uri);
    if (!source || !*source) {
        source=e->getAttributeNS(NULL,url);
        if (!source || !*source) {
            source=e->getAttributeNS(NULL,path);
            if (!source || !*source) {
                source=e->getAttributeNS(NULL,pathname);
                if (!source || !*source) {
                    source=e->getAttributeNS(NULL,file);
                    if (!source || !*source) {
                        source=e->getAttributeNS(NULL,filename);
                    }
                }
            }
        }
        else
            m_local=false;
    }
    else
        m_local=false;
    
    if (source && *source) {
        const XMLCh* flag=e->getAttributeNS(NULL,validate);
        m_validate=(XMLString::equals(flag,xmlconstants::XML_TRUE) || XMLString::equals(flag,xmlconstants::XML_ONE));

        auto_ptr_char temp(source);
        m_source=temp.get();
        
        if (!m_local && !strstr(m_source.c_str(),"://")) {
            log.warn("deprecated usage of uri/url attribute for a local resource, use path instead");
            m_local=true;
        }

        flag=e->getAttributeNS(NULL,reloadChanges);
        if (!XMLString::equals(flag,xmlconstants::XML_FALSE) && !XMLString::equals(flag,xmlconstants::XML_ZERO)) {
            if (m_local) {
#ifdef WIN32
                struct _stat stat_buf;
                if (_stat(m_source.c_str(), &stat_buf) == 0)
                    m_filestamp=stat_buf.st_mtime;
                else
                    m_local=false;
#else
                struct stat stat_buf;
                if (stat(m_source.c_str(), &stat_buf) == 0)
                    m_filestamp=stat_buf.st_mtime;
                else
                    m_local=false;
#endif
            }
            m_lock=RWLock::create();
        }

        log.debug("using external resource (%s), will %smonitor for changes", m_source.c_str(), m_lock ? "" : "not ");
    }
    else
        log.debug("no resource uri/path/name supplied, will load inline configuration");
}

pair<bool,DOMElement*> ReloadableXMLFile::load()
{
#ifdef _DEBUG
    NDC ndc("init");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ReloadableXMLFile");

    try {
        if (m_source.empty()) {
            // Data comes from the DOM we were handed.
            log.debug("loading inline configuration...");
            return make_pair(false,XMLHelper::getFirstChildElement(m_root));
        }
        else {
            // Data comes from a file we have to parse.
            log.debug("loading configuration from external resource...");

            DOMDocument* doc=NULL;
            auto_ptr_XMLCh widenit(m_source.c_str());
            if (m_local) {
                LocalFileInputSource src(widenit.get());
                Wrapper4InputSource dsrc(&src,false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
            }
            else {
                URLInputSource src(widenit.get());
                Wrapper4InputSource dsrc(&src,false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
            }

            log.infoStream() << "loaded XML resource (" << m_source << ")" << CategoryStream::ENDLINE;
            return make_pair(true,doc->getDocumentElement());
        }
    }
    catch (XMLException& e) {
        auto_ptr_char msg(e.getMessage());
        log.errorStream() << "Xerces error while loading resource (" << m_source << "): "
            << msg.get() << CategoryStream::ENDLINE;
        throw XMLParserException(msg.get());
    }
    catch (XMLToolingException& e) {
        log.errorStream() << "error while loading configuration from ("
            << (m_source.empty() ? "inline" : m_source) << "): " << e.what() << CategoryStream::ENDLINE;
        throw;
    }
}

Lockable* ReloadableXMLFile::lock()
{
    if (!m_lock)
        return this;
        
    m_lock->rdlock();

    // Check if we need to refresh.
    if (m_local) {
#ifdef WIN32
        struct _stat stat_buf;
        if (_stat(m_source.c_str(), &stat_buf) != 0)
            return this;
#else
        struct stat stat_buf;
        if (stat(m_source.c_str(), &stat_buf) != 0)
            return this;
#endif
        if (m_filestamp>=stat_buf.st_mtime)
            return this;
        
        // Elevate lock and recheck.
        m_lock->unlock();
        m_lock->wrlock();
        if (m_filestamp>=stat_buf.st_mtime) {
            // Somebody else handled it, just downgrade.
            m_lock->unlock();
            m_lock->rdlock();
            return this;
        }

        // Update the timestamp regardless. No point in repeatedly trying.
        m_filestamp=stat_buf.st_mtime;
    }
    else {
        if (isValid())
            return this;

        // Elevate lock and recheck.
        m_lock->unlock();
        m_lock->wrlock();
        if (isValid()) {
            // Somebody else handled it, just downgrade.
            m_lock->unlock();
            m_lock->rdlock();
            return this;
        }
    }
    
    // Do this once...
    do {
        // At this point we're holding the write lock, so make sure we pop it.
        SharedLock lockwrap(m_lock,false);
        pair<bool,DOMElement*> ret=load();
        if (ret.first)
            ret.second->getOwnerDocument()->release();
    } while(0);
    
    // If we made it here, the swap may or may not have worked, but we need to relock.
    m_lock->rdlock();
    return this;
}
