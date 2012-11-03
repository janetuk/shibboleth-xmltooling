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
 * @file ReloadableXMLFile.cpp
 *
 * Base class for file-based XML configuration.
 */

#include "internal.h"
#include "io/HTTPResponse.h"
#ifndef XMLTOOLING_LITE
# include "security/Credential.h"
# include "security/CredentialCriteria.h"
# include "security/CredentialResolver.h"
# include "security/SignatureTrustEngine.h"
# include "signature/Signature.h"
# include "signature/SignatureValidator.h"
#endif
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

#include <memory>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#ifndef XMLTOOLING_LITE
# include <xsec/dsig/DSIGReference.hpp>
# include <xsec/dsig/DSIGTransformList.hpp>
using namespace xmlsignature;
#endif

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
static const XMLCh maxRefreshDelay[] =  UNICODE_LITERAL_15(m,a,x,R,e,f,r,e,s,h,D,e,l,a,y);
static const XMLCh backingFilePath[] =  UNICODE_LITERAL_15(b,a,c,k,i,n,g,F,i,l,e,P,a,t,h);
static const XMLCh type[] =             UNICODE_LITERAL_4(t,y,p,e);
static const XMLCh certificate[] =      UNICODE_LITERAL_11(c,e,r,t,i,f,i,c,a,t,e);
static const XMLCh signerName[] =       UNICODE_LITERAL_10(s,i,g,n,e,r,N,a,m,e);
static const XMLCh _TrustEngine[] =     UNICODE_LITERAL_11(T,r,u,s,t,E,n,g,i,n,e);
static const XMLCh _CredentialResolver[] = UNICODE_LITERAL_18(C,r,e,d,e,n,t,i,a,l,R,e,s,o,l,v,e,r);


ReloadableXMLFile::ReloadableXMLFile(const DOMElement* e, Category& log, bool startReloadThread)
    : m_root(e), m_local(true), m_validate(false), m_filestamp(0), m_reloadInterval(0),
      m_lock(nullptr), m_log(log), m_loaded(false),
#ifndef XMLTOOLING_LITE
      m_credResolver(nullptr), m_trust(nullptr),
#endif
      m_shutdown(false), m_reload_wait(nullptr), m_reload_thread(nullptr)
{
#ifdef _DEBUG
    NDC ndc("ReloadableXMLFile");
#endif

    // Establish source of data...
    const XMLCh* source=e->getAttributeNS(nullptr,uri);
    if (!source || !*source) {
        source=e->getAttributeNS(nullptr,url);
        if (!source || !*source) {
            source=e->getAttributeNS(nullptr,path);
            if (!source || !*source) {
                source=e->getAttributeNS(nullptr,pathname);
                if (!source || !*source) {
                    source=e->getAttributeNS(nullptr,file);
                    if (!source || !*source) {
                        source=e->getAttributeNS(nullptr,filename);
                    }
                }
            }
        }
        else {
            m_local=false;
        }
    }
    else {
        m_local=false;
    }

    if (source && *source) {
        m_validate = XMLHelper::getAttrBool(e, false, validate);

        auto_ptr_char temp(source);
        m_source = temp.get();

        if (!m_local && !strstr(m_source.c_str(),"://")) {
            log.warn("deprecated usage of uri/url attribute for a local resource, use path instead");
            m_local = true;
        }

#ifndef XMLTOOLING_LITE
        // Check for signature bits.
        if (e->hasAttributeNS(nullptr, certificate)) {
            // Use a file-based credential resolver rooted here.
            m_credResolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(FILESYSTEM_CREDENTIAL_RESOLVER, e);
        }
        else {
            const DOMElement* sub = XMLHelper::getFirstChildElement(e, _CredentialResolver);
            string t(XMLHelper::getAttrString(sub, nullptr, type));
            if (!t.empty()) {
                m_credResolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(t.c_str(), sub);
            }
            else {
                sub = XMLHelper::getFirstChildElement(e, _TrustEngine);
                t = XMLHelper::getAttrString(sub, nullptr, type);
                if (!t.empty()) {
                    TrustEngine* trust = XMLToolingConfig::getConfig().TrustEngineManager.newPlugin(t.c_str(), sub);
                    if (!(m_trust = dynamic_cast<SignatureTrustEngine*>(trust))) {
                        delete trust;
                        throw XMLToolingException("TrustEngine-based ReloadableXMLFile requires a SignatureTrustEngine plugin.");
                    }

                    m_signerName = XMLHelper::getAttrString(e, nullptr, signerName);
                }
            }
        }
#endif

        if (m_local) {
            XMLToolingConfig::getConfig().getPathResolver()->resolve(m_source, PathResolver::XMLTOOLING_CFG_FILE);

            bool flag = XMLHelper::getAttrBool(e, true, reloadChanges);
            if (flag) {
#ifdef WIN32
                struct _stat stat_buf;
                if (_stat(m_source.c_str(), &stat_buf) == 0)
#else
                struct stat stat_buf;
                if (stat(m_source.c_str(), &stat_buf) == 0)
#endif
                    m_filestamp = stat_buf.st_mtime;
                else
                    throw IOException("Unable to access local file ($1)", params(1,m_source.c_str()));
                m_lock = RWLock::create();
            }
            FILE* cfile = fopen(m_source.c_str(), "r");
            if (cfile)
                fclose(cfile);
            else
                throw IOException("Unable to access local file ($1)", params(1,m_source.c_str()));
            log.debug("using local resource (%s), will %smonitor for changes", m_source.c_str(), m_lock ? "" : "not ");
        }
        else {
            log.debug("using remote resource (%s)", m_source.c_str());
            m_backing = XMLHelper::getAttrString(e, nullptr, backingFilePath);
            if (!m_backing.empty()) {
                XMLToolingConfig::getConfig().getPathResolver()->resolve(m_backing, PathResolver::XMLTOOLING_CACHE_FILE);
                log.debug("backup remote resource to (%s)", m_backing.c_str());
                try {
                    string tagname = m_backing + ".tag";
                    ifstream backer(tagname.c_str());
                    if (backer) {
                        char cachebuf[256];
                        if (backer.getline(cachebuf, 255)) {
                            m_cacheTag = cachebuf;
                            log.debug("loaded initial cache tag (%s)", m_cacheTag.c_str());
                        }
                    }
                }
                catch (exception&) {
                }
            }
            m_reloadInterval = XMLHelper::getAttrInt(e, 0, reloadInterval);
            if (m_reloadInterval == 0)
                m_reloadInterval = XMLHelper::getAttrInt(e, 0, maxRefreshDelay);
            if (m_reloadInterval > 0) {
                m_log.debug("will reload remote resource at most every %d seconds", m_reloadInterval);
                m_lock = RWLock::create();
            }
            m_filestamp = time(nullptr);   // assume it gets loaded initially
        }

        if (startReloadThread)
            startup();
    }
    else {
        log.debug("no resource uri/path/name supplied, will load inline configuration");
    }

    m_id = XMLHelper::getAttrString(e, nullptr, id);
}

ReloadableXMLFile::~ReloadableXMLFile()
{
    shutdown();
    delete m_lock;
}

void ReloadableXMLFile::startup()
{
    if (m_lock && !m_reload_thread) {
        m_reload_wait = CondWait::create();
        m_reload_thread = Thread::create(&reload_fn, this);
    }
}

void ReloadableXMLFile::shutdown()
{
    if (m_reload_thread) {
        // Shut down the reload thread and let it know.
        m_shutdown = true;
        m_reload_wait->signal();
        m_reload_thread->join(nullptr);
        delete m_reload_thread;
        delete m_reload_wait;
        m_reload_thread = nullptr;
        m_reload_wait = nullptr;
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

    return nullptr;
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
        if (m_reload_wait) {
            m_log.info("change detected, signaling reload thread...");
            m_reload_wait->signal();
        }
        else {
            m_log.warn("change detected, but reload thread not started");
        }
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
                m_log.info("using local backup of remote resource");
            else
                m_log.debug("loading configuration from external resource...");

            DOMDocument* doc=nullptr;
            if (m_local || backup) {
                auto_ptr_XMLCh widenit(backup ? m_backing.c_str() : m_source.c_str());
                // Use library-wide lock for now, nothing else is using it anyway.
                Locker locker(backup ? getBackupLock() : nullptr);
                LocalFileInputSource src(widenit.get());
                Wrapper4InputSource dsrc(&src, false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
            }
            else {
                URLInputSource src(m_root, nullptr, &m_cacheTag);
                Wrapper4InputSource dsrc(&src, false);
                if (m_validate)
                    doc=XMLToolingConfig::getConfig().getValidatingParser().parse(dsrc);
                else
                    doc=XMLToolingConfig::getConfig().getParser().parse(dsrc);

                // Check for a response code signal.
                if (XMLHelper::isNodeNamed(doc->getDocumentElement(), xmlconstants::XMLTOOLING_NS, URLInputSource::utf16StatusCodeElementName)) {
                    int responseCode = XMLString::parseInt(doc->getDocumentElement()->getFirstChild()->getNodeValue());
                    doc->release();
                    if (responseCode == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED)
                        throw (long)responseCode; // toss out as a "known" case to handle gracefully
                    else {
                        m_log.warn("remote resource fetch returned atypical status code (%d)", responseCode);
                        throw IOException("remote resource fetch failed, check log for status code of response");
                    }
                }
            }

            m_log.infoStream() << "loaded XML resource (" << (backup ? m_backing : m_source) << ")" << logging::eol;
#ifndef XMLTOOLING_LITE
            if (m_credResolver || m_trust) {
                m_log.debug("checking signature on XML resource");
                try {
                    DOMElement* sigel = XMLHelper::getFirstChildElement(doc->getDocumentElement(), xmlconstants::XMLSIG_NS, Signature::LOCAL_NAME);
                    if (!sigel)
                        throw XMLSecurityException("Signature validation required, but no signature found.");

                    // Wrap and unmarshall the signature for the duration of the check.
                    auto_ptr<Signature> sigobj(dynamic_cast<Signature*>(SignatureBuilder::buildOneFromElement(sigel)));    // don't bind to document
                    validateSignature(*sigobj.get());
                }
                catch (exception&) {
                    doc->release();
                    throw;
                }

            }
#endif
            return make_pair(true, doc->getDocumentElement());
        }
    }
    catch (XMLException& e) {
        auto_ptr_char msg(e.getMessage());
        m_log.errorStream() << "Xerces error while loading resource (" << (backup ? m_backing : m_source) << "): "
            << msg.get() << logging::eol;
        throw XMLParserException(msg.get());
    }
    catch (exception& e) {
        m_log.errorStream() << "error while loading resource ("
            << (m_source.empty() ? "inline" : (backup ? m_backing : m_source)) << "): " << e.what() << logging::eol;
        throw;
    }
}

pair<bool,DOMElement*> ReloadableXMLFile::load()
{
    // If this method is used, we're responsible for managing failover to a
    // backup of a remote resource (if available), and for backing up remote
    // resources.
    try {
        pair<bool,DOMElement*> ret = load(false);
        if (!m_backing.empty()) {
            m_log.debug("backing up remote resource to (%s)", m_backing.c_str());
            try {
                Locker locker(getBackupLock());
                ofstream backer(m_backing.c_str());
                backer << *(ret.second->getOwnerDocument());
                preserveCacheTag();
            }
            catch (exception& ex) {
                m_log.crit("exception while backing up resource: %s", ex.what());
            }
        }
        return ret;
    }
    catch (long& responseCode) {
        // If there's an HTTP error or the document hasn't changed,
        // use the backup iff we have no "valid" resource in place.
        // That prevents reload of the backup copy any time the document
        // hasn't changed.
        if (responseCode == HTTPResponse::XMLTOOLING_HTTP_STATUS_NOTMODIFIED)
            m_log.info("remote resource (%s) unchanged from cached version", m_source.c_str());
        if (!m_loaded && !m_backing.empty())
            return load(true);
        throw;
    }
    catch (exception&) {
        // Same as above, but for general load/parse errors.
        if (!m_loaded && !m_backing.empty())
            return load(true);
        throw;
    }
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

void ReloadableXMLFile::preserveCacheTag()
{
    if (!m_cacheTag.empty() && !m_backing.empty()) {
        try {
            string tagname = m_backing + ".tag";
            ofstream backer(tagname.c_str());
            backer << m_cacheTag;
        }
        catch (exception&) {
        }
    }
}

#ifndef XMLTOOLING_LITE

void ReloadableXMLFile::validateSignature(Signature& sigObj) const
{
    DSIGSignature* sig=sigObj.getXMLSignature();
    if (!sig)
        throw XMLSecurityException("Signature does not exist yet.");

    // Make sure the whole document was signed.
    bool valid=false;
    DSIGReferenceList* refs=sig->getReferenceList();
    if (refs && refs->getSize()==1) {
        DSIGReference* ref=refs->item(0);
        if (ref) {
            const XMLCh* URI=ref->getURI();
            if (URI==nullptr || *URI==0) {
                DSIGTransformList* tlist=ref->getTransforms();
                if (tlist->getSize() <= 2) { 
                    for (unsigned int i=0; tlist && i<tlist->getSize(); i++) {
                        if (tlist->item(i)->getTransformType()==TRANSFORM_ENVELOPED_SIGNATURE)
                            valid=true;
                        else if (tlist->item(i)->getTransformType()!=TRANSFORM_EXC_C14N &&
                                 tlist->item(i)->getTransformType()!=TRANSFORM_C14N
#ifdef XMLTOOLING_XMLSEC_C14N11
                                 && tlist->item(i)->getTransformType()!=TRANSFORM_C14N11
#endif
                                 ) {
                            valid=false;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if (!valid)
        throw XMLSecurityException("Invalid signature profile for signed configuration resource.");

    // Set up criteria.
    CredentialCriteria cc;
    cc.setUsage(Credential::SIGNING_CREDENTIAL);
    cc.setSignature(sigObj, CredentialCriteria::KEYINFO_EXTRACTION_KEY);
    if (!m_signerName.empty())
        cc.setPeerName(m_signerName.c_str());

    if (m_credResolver) {
        Locker locker(m_credResolver);
        vector<const Credential*> creds;
        if (m_credResolver->resolve(creds, &cc)) {
            SignatureValidator sigValidator;
            for (vector<const Credential*>::const_iterator i = creds.begin(); i != creds.end(); ++i) {
                try {
                    sigValidator.setCredential(*i);
                    sigValidator.validate(&sigObj);
                    return; // success!
                }
                catch (exception&) {
                }
            }
            throw XMLSecurityException("Unable to verify signature with supplied key(s).");
        }
        else {
            throw XMLSecurityException("CredentialResolver did not supply any candidate keys.");
        }
    }
    else if (m_trust) {
        auto_ptr<CredentialResolver> dummy(
            XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(DUMMY_CREDENTIAL_RESOLVER, nullptr)
            );
        if (m_trust->validate(sigObj, *(dummy.get()), &cc))
            return;
        throw XMLSecurityException("TrustEngine unable to verify signature.");
    }

    throw XMLSecurityException("Unable to verify signature.");
}

#endif
