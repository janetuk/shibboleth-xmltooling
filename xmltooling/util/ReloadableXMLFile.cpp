/*
 *  Copyright 2001-2010 Internet2
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
#include "io/HTTPResponse.h"
#include "util/NDC.h"
#include "util/PathResolver.h"
#include "util/ReloadableXMLFile.h"
#include "util/Threads.h"
#include "util/XMLConstants.h"
#include "util/XMLHelper.h"

#if defined(XMLTOOLING_LOG4SHIB)
# include <log4shib/NDC.hh>
#elif defined(XMLTOOLING_LOG4CPP)
# include <log4cpp/NDC.hh>
#endif

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

static const XMLCh id[] =               UNICODE_LITERAL_2(i,d);
static const XMLCh uri[] =              UNICODE_LITERAL_3(u,r,i);
static const XMLCh url[] =              UNICODE_LITERAL_3(u,r,l);
static const XMLCh path[] =             UNICODE_LITERAL_4(p,a,t,h);
static const XMLCh pathname[] =         UNICODE_LITERAL_8(p,a,t,h,n,a,m,e);
static const XMLCh file[] =             UNICODE_LITERAL_4(f,i,l,e);
static const XMLCh filename[] =         UNICODE_LITERAL_8(f,i,l,e,n,a,m,e);
static const XMLCh validate[] =         UNICODE_LITERAL_8(v,a,l,i,d,a,t,e);
static const XMLCh reloadChanges[] =    UNICODE_LITERAL_13(r,e,l,o,a,d,C,h,a,n,g,e,s);
static const XMLCh reloadInterval[] =   UNICODE_LITERAL_14(r,e,l,o,a,d,I,n,t,e,r,v,a,l);
static const XMLCh backingFilePath[] =  UNICODE_LITERAL_15(b,a,c,k,i,n,g,F,i,l,e,P,a,t,h);


ReloadableXMLFile::ReloadableXMLFile(const DOMElement* e, Category& log)
    : m_root(e), m_local(true), m_validate(false), m_backupIndicator(true), m_filestamp(0), m_reloadInterval(0), m_lock(NULL), m_log(log),
        m_shutdown(false), m_reload_wait(NULL), m_reload_thread(NULL)
{
#ifdef _DEBUG
    NDC ndc("ReloadableXMLFile");
#endif

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

        if (m_local) {
            XMLToolingConfig::getConfig().getPathResolver()->resolve(m_source, PathResolver::XMLTOOLING_CFG_FILE);

            flag=e->getAttributeNS(NULL,reloadChanges);
            if (!XMLString::equals(flag,xmlconstants::XML_FALSE) && !XMLString::equals(flag,xmlconstants::XML_ZERO)) {
#ifdef WIN32
                struct _stat stat_buf;
                if (_stat(m_source.c_str(), &stat_buf) == 0)
#else
                struct stat stat_buf;
                if (stat(m_source.c_str(), &stat_buf) == 0)
#endif
                    m_filestamp=stat_buf.st_mtime;
                else
                    throw IOException("Unable to access local file ($1)", params(1,m_source.c_str()));
                m_lock=RWLock::create();
            }
            log.debug("using local resource (%s), will %smonitor for changes", m_source.c_str(), m_lock ? "" : "not ");
        }
        else {
            log.debug("using remote resource (%s)", m_source.c_str());
            source = e->getAttributeNS(NULL,backingFilePath);
            if (source && *source) {
                auto_ptr_char temp2(source);
                m_backing=temp2.get();
                XMLToolingConfig::getConfig().getPathResolver()->resolve(m_backing, PathResolver::XMLTOOLING_RUN_FILE);
                log.debug("backup remote resource to (%s)", m_backing.c_str());
            }
            source = e->getAttributeNS(NULL,reloadInterval);
            if (source && *source) {
                m_reloadInterval = XMLString::parseInt(source);
                if (m_reloadInterval > 0) {
                    m_log.debug("will reload remote resource at most every %d seconds", m_reloadInterval);
                    m_lock=RWLock::create();
                }
            }
            m_filestamp = time(NULL);   // assume it gets loaded initially
        }

        if (m_lock) {
            m_reload_wait = CondWait::create();
            m_reload_thread = Thread::create(&reload_fn, this);
        }
    }
    else {
        log.debug("no resource uri/path/name supplied, will load inline configuration");
    }

    source = e->getAttributeNS(NULL, id);
    if (source && *source) {
        auto_ptr_char tempid(source);
        m_id = tempid.get();
    }
}

ReloadableXMLFile::~ReloadableXMLFile()
{
    shutdown();
    delete m_lock;
}

void ReloadableXMLFile::shutdown()
{
    if (m_reload_thread) {
        // Shut down the reload thread and let it know.
        m_shutdown = true;
        m_reload_wait->signal();
        m_reload_thread->join(NULL);
        delete m_reload_thread;
        delete m_reload_wait;
        m_reload_thread = NULL;
        m_reload_wait = NULL;
    }
}

void* ReloadableXMLFile::reload_fn(void* pv)
{
    ReloadableXMLFile* r = reinterpret_cast<ReloadableXMLFile*>(pv);

#ifndef WIN32
    // First, let's block all signals
    Thread::mask_all_signals();
#endif

    if (!r->m_id.empty()) {
        string threadid("[");
        threadid += r->m_id + ']';
        logging::NDC::push(threadid);
    }

#ifdef _DEBUG
    NDC ndc("reload");
#endif

    auto_ptr<Mutex> mutex(Mutex::create());
    mutex->lock();

    if (r->m_local)
        r->m_log.info("reload thread started...running when signaled");
    else
        r->m_log.info("reload thread started...running every %d seconds", r->m_reloadInterval);

    while (!r->m_shutdown) {
        if (r->m_local)
            r->m_reload_wait->wait(mutex.get());
        else
            r->m_reload_wait->timedwait(mutex.get(), r->m_reloadInterval);
        if (r->m_shutdown)
            break;

        try {
            r->m_log.info("reloading %s resource...", r->m_local ? "local" : "remote");
            pair<bool,DOMElement*> ret = r->background_load();
            if (ret.first)
                ret.second->getOwnerDocument()->release();
        }
        catch (long& ex) {
            if (ex == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED) {
                r->m_log.info("remote resource (%s) unchanged from cached version", r->m_source.c_str());
            }
            else {
                // Shouldn't happen, we should only get codes intended to be gracefully handled.
                r->m_log.crit("maintaining existing configuration, remote resource fetch returned atypical status code (%d)", ex);
            }
        }
        catch (exception& ex) {
            r->m_log.crit("maintaining existing configuration, error reloading resource (%s): %s", r->m_source.c_str(), ex.what());
        }
    }

    r->m_log.info("reload thread finished");

    mutex->unlock();

    if (!r->m_id.empty()) {
        logging::NDC::pop();
    }

    return NULL;
}

Lockable* ReloadableXMLFile::lock()
{
    if (!m_lock)
        return this;

    m_lock->rdlock();

    if (m_local) {
    // Check if we need to refresh.
#ifdef WIN32
        struct _stat stat_buf;
        if (_stat(m_source.c_str(), &stat_buf) != 0)
            return this;
#else
        struct stat stat_buf;
        if (stat(m_source.c_str(), &stat_buf) != 0)
            return this;
#endif
        if (m_filestamp >= stat_buf.st_mtime)
            return this;

        // Elevate lock and recheck.
        m_log.debug("timestamp of local resource changed, elevating to a write lock");
        m_lock->unlock();
        m_lock->wrlock();
        if (m_filestamp >= stat_buf.st_mtime) {
            // Somebody else handled it, just downgrade.
            m_log.debug("update of local resource handled by another thread, downgrading lock");
            m_lock->unlock();
            m_lock->rdlock();
            return this;
        }

        // Update the timestamp regardless.
        m_filestamp = stat_buf.st_mtime;
        m_log.info("change detected, signaling reload thread...");
        m_reload_wait->signal();
    }

    return this;
}

void ReloadableXMLFile::unlock()
{
    if (m_lock)
        m_lock->unlock();
}

pair<bool,DOMElement*> ReloadableXMLFile::load(bool backup)
{
#ifdef _DEBUG
    NDC ndc("load");
#endif

    try {
        if (m_source.empty()) {
            // Data comes from the DOM we were handed.
            m_log.debug("loading inline configuration...");
            return make_pair(false, XMLHelper::getFirstChildElement(m_root));
        }
        else {
            // Data comes from a file we have to parse.
            if (backup)
                m_log.warn("using local backup of remote resource");
            else
                m_log.debug("loading configuration from external resource...");

            DOMDocument* doc=NULL;
            if (m_local || backup) {
                auto_ptr_XMLCh widenit(backup ? m_backing.c_str() : m_source.c_str());
                // Use library-wide lock for now, nothing else is using it anyway.
                Locker locker(backup ? getBackupLock() : NULL);
                LocalFileInputSource src(widenit.get());
                Wrapper4InputSource dsrc(&src, false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
            }
            else {
                URLInputSource src(m_root, NULL, &m_cacheTag);
                Wrapper4InputSource dsrc(&src, false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);

                // Check for a response code signal.
                if (XMLHelper::isNodeNamed(doc->getDocumentElement(), xmlconstants::XMLTOOLING_NS, URLInputSource::utf16StatusCodeElementName)) {
                    int responseCode = XMLString::parseInt(doc->getDocumentElement()->getFirstChild()->getNodeValue());
                    doc->release();
                    if (responseCode == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED) {
                        throw (long)responseCode; // toss out as a "known" case to handle gracefully
                    }
                    else {
                        m_log.warn("remote resource fetch returned atypical status code (%d)", responseCode);
                        throw IOException("remote resource fetch failed, check log for status code of response");
                    }
                }
            }

            m_log.infoStream() << "loaded XML resource (" << (backup ? m_backing : m_source) << ")" << logging::eol;

            if (!backup && !m_backing.empty()) {
                // If the indicator is true, we're responsible for the backup.
                if (m_backupIndicator) {
                    m_log.debug("backing up remote resource to (%s)", m_backing.c_str());
                    try {
                        Locker locker(getBackupLock());
                        ofstream backer(m_backing.c_str());
                        backer << *doc;
                    }
                    catch (exception& ex) {
                        m_log.crit("exception while backing up resource: %s", ex.what());
                    }
                }
                else {
                    // If the indicator was false, set true to signal that a backup is needed.
                    // The caller will presumably flip it back to false once that's done.
                    m_backupIndicator = true;
                }
            }

            return make_pair(true, doc->getDocumentElement());
        }
    }
    catch (XMLException& e) {
        auto_ptr_char msg(e.getMessage());
        m_log.errorStream() << "Xerces error while loading resource (" << (backup ? m_backing : m_source) << "): "
            << msg.get() << logging::eol;
        if (!backup && !m_backing.empty())
            return load(true);
        throw XMLParserException(msg.get());
    }
    catch (exception& e) {
        m_log.errorStream() << "error while loading resource ("
            << (m_source.empty() ? "inline" : (backup ? m_backing : m_source)) << "): " << e.what() << logging::eol;
        if (!backup && !m_backing.empty())
            return load(true);
        throw;
    }
}

pair<bool,DOMElement*> ReloadableXMLFile::load()
{
    return load(false);
}

pair<bool,DOMElement*> ReloadableXMLFile::background_load()
{
    // If this method isn't overridden, we acquire a write lock
    // and just call the old override.
    if (m_lock)
        m_lock->wrlock();
    SharedLock locker(m_lock, false);
    return load();
}

Lockable* ReloadableXMLFile::getBackupLock()
{
    return &XMLToolingConfig::getConfig();
}
