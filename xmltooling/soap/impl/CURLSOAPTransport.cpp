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
 * CURLSOAPTransport.cpp
 *
 * libcurl-based SOAPTransport implementation
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "security/CredentialCriteria.h"
#include "security/OpenSSLTrustEngine.h"
#include "security/OpenSSLCredential.h"
#include "soap/HTTPSOAPTransport.h"
#include "soap/OpenSSLSOAPTransport.h"
#include "util/NDC.h"
#include "util/Threads.h"

#include <list>
#include <curl/curl.h>
#include <openssl/x509_vfy.h>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

namespace xmltooling {

    // Manages cache of socket connections via CURL handles.
    class XMLTOOL_DLLLOCAL CURLPool
    {
    public:
        CURLPool() : m_size(0), m_lock(Mutex::create()),
            m_log(Category::getInstance(XMLTOOLING_LOGCAT".SOAPTransport.CURL")) {}
        ~CURLPool();

        CURL* get(const SOAPTransport::Address& addr);
        void put(const char* from, const char* to, const char* endpoint, CURL* handle);

    private:
        typedef map<string,vector<CURL*> > poolmap_t;
        poolmap_t m_bindingMap;
        list< vector<CURL*>* > m_pools;
        long m_size;
        Mutex* m_lock;
        Category& m_log;
    };

    static XMLTOOL_DLLLOCAL CURLPool* g_CURLPool = NULL;

    class XMLTOOL_DLLLOCAL CURLSOAPTransport : public HTTPSOAPTransport, public OpenSSLSOAPTransport
    {
    public:
        CURLSOAPTransport(const Address& addr)
            : m_sender(addr.m_from ? addr.m_from : ""), m_peerName(addr.m_to ? addr.m_to : ""), m_endpoint(addr.m_endpoint),
                m_handle(NULL), m_headers(NULL),
#ifndef XMLTOOLING_NO_XMLSEC
                    m_cred(NULL), m_trustEngine(NULL), m_peerResolver(NULL), m_mandatory(false),
#endif
                    m_openssl_ops(SSL_OP_ALL|SSL_OP_NO_SSLv2), m_ssl_callback(NULL), m_ssl_userptr(NULL),
                    m_chunked(true), m_authenticated(false), m_cacheTag(NULL) {
            m_handle = g_CURLPool->get(addr);
            curl_easy_setopt(m_handle,CURLOPT_URL,addr.m_endpoint);
            curl_easy_setopt(m_handle,CURLOPT_CONNECTTIMEOUT,15);
            curl_easy_setopt(m_handle,CURLOPT_TIMEOUT,30);
            curl_easy_setopt(m_handle,CURLOPT_HTTPAUTH,0);
            curl_easy_setopt(m_handle,CURLOPT_USERPWD,NULL);
            curl_easy_setopt(m_handle,CURLOPT_SSL_VERIFYHOST,2);
            curl_easy_setopt(m_handle,CURLOPT_HEADERDATA,this);
            m_headers=curl_slist_append(m_headers,"Content-Type: text/xml");
        }

        virtual ~CURLSOAPTransport() {
            curl_slist_free_all(m_headers);
            curl_easy_setopt(m_handle,CURLOPT_ERRORBUFFER,NULL);
            curl_easy_setopt(m_handle,CURLOPT_PRIVATE,m_authenticated ? "secure" : NULL); // Save off security "state".
            g_CURLPool->put(m_sender.c_str(), m_peerName.c_str(), m_endpoint.c_str(), m_handle);
        }

        bool isConfidential() const {
            return m_endpoint.find("https")==0;
        }

        bool setConnectTimeout(long timeout) {
            return (curl_easy_setopt(m_handle,CURLOPT_CONNECTTIMEOUT,timeout)==CURLE_OK);
        }

        bool setTimeout(long timeout) {
            return (curl_easy_setopt(m_handle,CURLOPT_TIMEOUT,timeout)==CURLE_OK);
        }

        bool setAuth(transport_auth_t authType, const char* username=NULL, const char* password=NULL);

        bool setVerifyHost(bool verify) {
            return (curl_easy_setopt(m_handle,CURLOPT_SSL_VERIFYHOST,verify ? 2 : 0)==CURLE_OK);
        }

#ifndef XMLTOOLING_NO_XMLSEC
        bool setCredential(const Credential* cred=NULL) {
            const OpenSSLCredential* down = dynamic_cast<const OpenSSLCredential*>(cred);
            if (!down) {
                m_cred = NULL;
                return (cred==NULL);
            }
            m_cred = down;
            return true;
        }

        bool setTrustEngine(
            const X509TrustEngine* trustEngine=NULL,
            const CredentialResolver* peerResolver=NULL,
            CredentialCriteria* criteria=NULL,
            bool mandatory=true
            ) {
            const OpenSSLTrustEngine* down = dynamic_cast<const OpenSSLTrustEngine*>(trustEngine);
            if (!down) {
                m_trustEngine = NULL;
                m_peerResolver = NULL;
                m_criteria = NULL;
                return (trustEngine==NULL);
            }
            m_trustEngine = down;
            m_peerResolver = peerResolver;
            m_criteria = criteria;
            m_mandatory = mandatory;
            return true;
        }

#endif

        bool useChunkedEncoding(bool chunked=true) {
            m_chunked = chunked;
            return true;
        }

        bool setCacheTag(string* cacheTag) {
            m_cacheTag = cacheTag;
            return true;
        }

        bool setProviderOption(const char* provider, const char* option, const char* value);

        void send(istream& in) {
            send(&in);
        }

        void send(istream* in=NULL);

        istream& receive() {
            return m_stream;
        }

        bool isAuthenticated() const {
            return m_authenticated;
        }

        void setAuthenticated(bool auth) {
            m_authenticated = auth;
        }

        string getContentType() const;
        long getStatusCode() const;

        bool setRequestHeader(const char* name, const char* val) {
            string temp(name);
            temp=temp + ": " + val;
            m_headers=curl_slist_append(m_headers,temp.c_str());
            return true;
        }

        const vector<string>& getResponseHeader(const char* val) const;

        bool setSSLCallback(ssl_ctx_callback_fn fn, void* userptr=NULL) {
            m_ssl_callback=fn;
            m_ssl_userptr=userptr;
            return true;
        }

    private:
        // per-call state
        string m_sender,m_peerName,m_endpoint,m_simplecreds;
        CURL* m_handle;
        stringstream m_stream;
        struct curl_slist* m_headers;
        map<string,vector<string> > m_response_headers;
        vector<string> m_saved_options;
#ifndef XMLTOOLING_NO_XMLSEC
        const OpenSSLCredential* m_cred;
        const OpenSSLTrustEngine* m_trustEngine;
        const CredentialResolver* m_peerResolver;
        CredentialCriteria* m_criteria;
        bool m_mandatory;
#endif
        int m_openssl_ops;
        ssl_ctx_callback_fn m_ssl_callback;
        void* m_ssl_userptr;
        bool m_chunked;
        bool m_authenticated;
        string* m_cacheTag;

        friend size_t XMLTOOL_DLLLOCAL curl_header_hook(void* ptr, size_t size, size_t nmemb, void* stream);
        friend CURLcode XMLTOOL_DLLLOCAL xml_ssl_ctx_callback(CURL* curl, SSL_CTX* ssl_ctx, void* userptr);
        friend int XMLTOOL_DLLLOCAL verify_callback(X509_STORE_CTX* x509_ctx, void* arg);
    };

    // libcurl callback functions
    size_t XMLTOOL_DLLLOCAL curl_header_hook(void* ptr, size_t size, size_t nmemb, void* stream);
    size_t XMLTOOL_DLLLOCAL curl_write_hook(void* ptr, size_t size, size_t nmemb, void* stream);
    size_t XMLTOOL_DLLLOCAL curl_read_hook( void *ptr, size_t size, size_t nmemb, void *stream);
    int XMLTOOL_DLLLOCAL curl_debug_hook(CURL* handle, curl_infotype type, char* data, size_t len, void* ptr);
    CURLcode XMLTOOL_DLLLOCAL xml_ssl_ctx_callback(CURL* curl, SSL_CTX* ssl_ctx, void* userptr);
#ifndef XMLTOOLING_NO_XMLSEC
    int XMLTOOL_DLLLOCAL verify_callback(X509_STORE_CTX* x509_ctx, void* arg);
#endif

    SOAPTransport* CURLSOAPTransportFactory(const SOAPTransport::Address& addr)
    {
        return new CURLSOAPTransport(addr);
    }
};

void xmltooling::registerSOAPTransports()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.SOAPTransportManager.registerFactory("http", CURLSOAPTransportFactory);
    conf.SOAPTransportManager.registerFactory("https", CURLSOAPTransportFactory);
}

void xmltooling::initSOAPTransports()
{
    g_CURLPool=new CURLPool();
}

void xmltooling::termSOAPTransports()
{
    delete g_CURLPool;
    g_CURLPool = NULL;
}

OpenSSLSOAPTransport::OpenSSLSOAPTransport()
{
}

OpenSSLSOAPTransport::~OpenSSLSOAPTransport()
{
}

CURLPool::~CURLPool()
{
    for (poolmap_t::iterator i=m_bindingMap.begin(); i!=m_bindingMap.end(); i++) {
        for (vector<CURL*>::iterator j=i->second.begin(); j!=i->second.end(); j++)
            curl_easy_cleanup(*j);
    }
    delete m_lock;
}

CURL* CURLPool::get(const SOAPTransport::Address& addr)
{
#ifdef _DEBUG
    xmltooling::NDC("get");
#endif
    m_log.debug("getting connection handle to %s", addr.m_endpoint);
    string key(addr.m_endpoint);
    if (addr.m_from)
        key = key + '|' + addr.m_from;
    if (addr.m_to)
        key = key + '|' + addr.m_to;
    m_lock->lock();
    poolmap_t::iterator i=m_bindingMap.find(key);

    if (i!=m_bindingMap.end()) {
        // Move this pool to the front of the list.
        m_pools.remove(&(i->second));
        m_pools.push_front(&(i->second));

        // If a free connection exists, return it.
        if (!(i->second.empty())) {
            CURL* handle=i->second.back();
            i->second.pop_back();
            m_size--;
            m_lock->unlock();
            m_log.debug("returning existing connection handle from pool");
            return handle;
        }
    }

    m_lock->unlock();
    m_log.debug("nothing free in pool, returning new connection handle");

    // Create a new connection and set non-varying options.
    CURL* handle=curl_easy_init();
    if (!handle)
        return NULL;
    curl_easy_setopt(handle,CURLOPT_NOPROGRESS,1);
    curl_easy_setopt(handle,CURLOPT_NOSIGNAL,1);
    curl_easy_setopt(handle,CURLOPT_FAILONERROR,1);
    curl_easy_setopt(handle,CURLOPT_SSL_CIPHER_LIST,"ALL:!aNULL:!LOW:!EXPORT:!SSLv2");
    // Verification of the peer is via TrustEngine only.
    curl_easy_setopt(handle,CURLOPT_SSL_VERIFYPEER,0);
    curl_easy_setopt(handle,CURLOPT_CAINFO,NULL);
    curl_easy_setopt(handle,CURLOPT_HEADERFUNCTION,&curl_header_hook);
    curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,&curl_write_hook);
    curl_easy_setopt(handle,CURLOPT_DEBUGFUNCTION,&curl_debug_hook);

    return handle;
}

void CURLPool::put(const char* from, const char* to, const char* endpoint, CURL* handle)
{
    string key(endpoint);
    if (from)
        key = key + '|' + from;
    if (to)
        key = key + '|' + to;
    m_lock->lock();
    poolmap_t::iterator i=m_bindingMap.find(key);
    if (i==m_bindingMap.end())
        m_pools.push_front(&(m_bindingMap.insert(poolmap_t::value_type(key,vector<CURL*>(1,handle))).first->second));
    else
        i->second.push_back(handle);

    CURL* killit=NULL;
    if (++m_size > 256) {
        // Kick a handle out from the back of the bus.
        while (true) {
            vector<CURL*>* corpse=m_pools.back();
            if (!corpse->empty()) {
                killit=corpse->back();
                corpse->pop_back();
                m_size--;
                break;
            }

            // Move an empty pool up to the front so we don't keep hitting it.
            m_pools.pop_back();
            m_pools.push_front(corpse);
        }
    }
    m_lock->unlock();
    if (killit) {
        curl_easy_cleanup(killit);
#ifdef _DEBUG
        xmltooling::NDC("put");
#endif
        m_log.info("conn_pool_max limit reached, dropping an old connection");
    }
}

bool CURLSOAPTransport::setAuth(transport_auth_t authType, const char* username, const char* password)
{
    if (authType==transport_auth_none) {
        if (curl_easy_setopt(m_handle,CURLOPT_HTTPAUTH,0)!=CURLE_OK)
            return false;
        return (curl_easy_setopt(m_handle,CURLOPT_USERPWD,NULL)==CURLE_OK);
    }
    long flag=0;
    switch (authType) {
        case transport_auth_basic:    flag = CURLAUTH_BASIC; break;
        case transport_auth_digest:   flag = CURLAUTH_DIGEST; break;
        case transport_auth_ntlm:     flag = CURLAUTH_NTLM; break;
        case transport_auth_gss:      flag = CURLAUTH_GSSNEGOTIATE; break;
        default:            return false;
    }
    if (curl_easy_setopt(m_handle,CURLOPT_HTTPAUTH,flag)!=CURLE_OK)
        return false;
    m_simplecreds = string(username ? username : "") + ':' + (password ? password : "");
    return (curl_easy_setopt(m_handle,CURLOPT_USERPWD,m_simplecreds.c_str())==CURLE_OK);
}

bool CURLSOAPTransport::setProviderOption(const char* provider, const char* option, const char* value)
{
    if (!provider || !option || !value) {
        return false;
    }
    else if (!strcmp(provider, "OpenSSL")) {
        if (!strcmp(option, "SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION") && (*value=='1' || *value=='t')) {
            // If the new option to enable buggy rengotiation is available, set it.
            // Otherwise, signal false if this is newer than 0.9.8k, because that
            // means it's 0.9.8l, which blocks renegotiation, and therefore will
            // not honor this request. Older versions are buggy, so behave as though
            // the flag was set anyway, so we signal true.
#if defined(SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION)
            m_openssl_ops |= SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION;
            return true;
#elif (OPENSSL_VERSION_NUMBER > 0x009080bfL)
            return false;
#else
            return true;
#endif
        }
        return false;
    }
    else if (strcmp(provider, "CURL")) {
        return false;
    }

    // For libcurl, the option is an enum and the value type depends on the option.
    CURLoption opt = static_cast<CURLoption>(strtol(option, NULL, 10));
    if (opt < CURLOPTTYPE_OBJECTPOINT)
        return (curl_easy_setopt(m_handle, opt, strtol(value, NULL, 10)) == CURLE_OK);
#ifdef CURLOPTTYPE_OFF_T
    else if (opt < CURLOPTTYPE_OFF_T) {
        if (value)
            m_saved_options.push_back(value);
        return (curl_easy_setopt(m_handle, opt, value ? m_saved_options.back().c_str() : NULL) == CURLE_OK);
    }
# ifdef HAVE_CURL_OFF_T
    else if (sizeof(curl_off_t) == sizeof(long))
        return (curl_easy_setopt(m_handle, opt, strtol(value, NULL, 10)) == CURLE_OK);
# else
    else if (sizeof(off_t) == sizeof(long))
        return (curl_easy_setopt(m_handle, opt, strtol(value, NULL, 10)) == CURLE_OK);
# endif
    return false;
#else
    else {
        if (value)
            m_saved_options.push_back(value);
        return (curl_easy_setopt(m_handle, opt, value ? m_saved_options.back().c_str() : NULL) == CURLE_OK);
    }
#endif
}

const vector<string>& CURLSOAPTransport::getResponseHeader(const char* name) const
{
    static vector<string> emptyVector;

    map<string,vector<string> >::const_iterator i=m_response_headers.find(name);
    if (i!=m_response_headers.end())
        return i->second;

    for (map<string,vector<string> >::const_iterator j=m_response_headers.begin(); j!=m_response_headers.end(); j++) {
#ifdef HAVE_STRCASECMP
        if (!strcasecmp(j->first.c_str(), name))
#else
        if (!stricmp(j->first.c_str(), name))
#endif
            return j->second;
    }

    return emptyVector;
}

string CURLSOAPTransport::getContentType() const
{
    char* content_type=NULL;
    curl_easy_getinfo(m_handle,CURLINFO_CONTENT_TYPE,&content_type);
    return content_type ? content_type : "";
}

long CURLSOAPTransport::getStatusCode() const
{
    long code=200;
    if (curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &code) != CURLE_OK)
        code = 200;
    return code;
}

void CURLSOAPTransport::send(istream* in)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("send");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".SOAPTransport.CURL");
    Category& log_curl=Category::getInstance(XMLTOOLING_LOGCAT".libcurl");

    // For this implementation, it's sufficient to check for https as a sign of transport security.
    if (m_mandatory && !isConfidential())
        throw IOException("Blocking unprotected HTTP request, transport authentication by server required.");

    string msg;

    // By this time, the handle has been prepared with the URL to use and the
    // caller should have executed any set functions to manipulate it.

    // Setup standard per-call curl properties.
    curl_easy_setopt(m_handle,CURLOPT_DEBUGDATA,&log_curl);
    curl_easy_setopt(m_handle,CURLOPT_FILE,&m_stream);
    if (m_chunked && in) {
        curl_easy_setopt(m_handle,CURLOPT_POST,1);
        m_headers=curl_slist_append(m_headers,"Transfer-Encoding: chunked");
        curl_easy_setopt(m_handle,CURLOPT_READFUNCTION,&curl_read_hook);
        curl_easy_setopt(m_handle,CURLOPT_READDATA,in);
    }
    else if (in) {
        char buf[1024];
        while (*in) {
            in->read(buf,1024);
            msg.append(buf,in->gcount());
        }
        curl_easy_setopt(m_handle,CURLOPT_POST,1);
        curl_easy_setopt(m_handle,CURLOPT_READFUNCTION,NULL);
        curl_easy_setopt(m_handle,CURLOPT_POSTFIELDS,msg.c_str());
        curl_easy_setopt(m_handle,CURLOPT_POSTFIELDSIZE,msg.length());
    }
    else {
        curl_easy_setopt(m_handle,CURLOPT_HTTPGET,1);
        curl_easy_setopt(m_handle,CURLOPT_FOLLOWLOCATION,1);
        curl_easy_setopt(m_handle,CURLOPT_MAXREDIRS,6);
    }

    char curl_errorbuf[CURL_ERROR_SIZE];
    curl_errorbuf[0]=0;
    curl_easy_setopt(m_handle,CURLOPT_ERRORBUFFER,curl_errorbuf);
    if (log_curl.isDebugEnabled())
        curl_easy_setopt(m_handle,CURLOPT_VERBOSE,1);

    // Check for cache tag.
    if (m_cacheTag && !m_cacheTag->empty()) {
        string hdr("If-None-Match: ");
        hdr += *m_cacheTag;
        m_headers = curl_slist_append(m_headers, hdr.c_str());
    }

    // Set request headers.
    curl_easy_setopt(m_handle,CURLOPT_HTTPHEADER,m_headers);

#ifndef XMLTOOLING_NO_XMLSEC
    if (m_ssl_callback || m_cred || m_trustEngine) {
#else
    if (m_ssl_callback) {
#endif
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_FUNCTION,xml_ssl_ctx_callback);
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_DATA,this);

        // Restore security "state". Necessary because the callback only runs
        // when handshakes occur. Even new TCP connections won't execute it.
        char* priv=NULL;
        curl_easy_getinfo(m_handle,CURLINFO_PRIVATE,&priv);
        if (priv)
            m_authenticated=true;
    }
    else {
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_FUNCTION,NULL);
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_DATA,NULL);
    }

    // Make the call.
    log.debug("sending SOAP message to %s", m_endpoint.c_str());
    if (curl_easy_perform(m_handle) != CURLE_OK) {
        throw IOException(
            string("CURLSOAPTransport failed while contacting SOAP endpoint (") + m_endpoint + "): " +
                (curl_errorbuf[0] ? curl_errorbuf : "no further information available"));
    }

    // Check for outgoing cache tag.
    if (m_cacheTag) {
        const vector<string>& tags = getResponseHeader("ETag");
        if (!tags.empty())
            *m_cacheTag = tags.front();
    }
}

// callback to buffer headers from server
size_t xmltooling::curl_header_hook(void* ptr, size_t size, size_t nmemb, void* stream)
{
    // only handle single-byte data
    if (size!=1)
        return 0;
    CURLSOAPTransport* ctx = reinterpret_cast<CURLSOAPTransport*>(stream);
    char* buf = (char*)malloc(nmemb + 1);
    if (buf) {
        memset(buf,0,nmemb + 1);
        memcpy(buf,ptr,nmemb);
        char* sep=(char*)strchr(buf,':');
        if (sep) {
            *(sep++)=0;
            while (*sep==' ')
                *(sep++)=0;
            char* white=buf+nmemb-1;
            while (isspace(*white))
                *(white--)=0;
            ctx->m_response_headers[buf].push_back(sep);
        }
        free(buf);
        return nmemb;
    }
    return 0;
}

// callback to send data to server
size_t xmltooling::curl_read_hook(void* ptr, size_t size, size_t nmemb, void* stream)
{
    // stream is actually an istream pointer
    istream* buf=reinterpret_cast<istream*>(stream);
    buf->read(reinterpret_cast<char*>(ptr),size*nmemb);
    return buf->gcount();
}

// callback to buffer data from server
size_t xmltooling::curl_write_hook(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t len = size*nmemb;
    reinterpret_cast<stringstream*>(stream)->write(reinterpret_cast<const char*>(ptr),len);
    return len;
}

// callback for curl debug data
int xmltooling::curl_debug_hook(CURL* handle, curl_infotype type, char* data, size_t len, void* ptr)
{
    // *ptr is actually a logging object
    if (!ptr) return 0;
    CategoryStream log=reinterpret_cast<Category*>(ptr)->debugStream();
    for (unsigned char* ch=(unsigned char*)data; len && (isprint(*ch) || isspace(*ch)); len--)
        log << *ch++;
    return 0;
}

#ifndef XMLTOOLING_NO_XMLSEC
int xmltooling::verify_callback(X509_STORE_CTX* x509_ctx, void* arg)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".SOAPTransport.CURL");
    log.debug("invoking custom X.509 verify callback");
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
    CURLSOAPTransport* ctx = reinterpret_cast<CURLSOAPTransport*>(arg);
#else
    // Yes, this sucks. I'd use TLS, but there's no really obvious spot to put the thread key
    // and global variables suck too. We can't access the X509_STORE_CTX depth directly because
    // OpenSSL only copies it into the context if it's >=0, and the unsigned pointer may be
    // negative in the SSL structure's int member.
    CURLSOAPTransport* ctx = reinterpret_cast<CURLSOAPTransport*>(
        SSL_get_verify_depth(
            reinterpret_cast<SSL*>(X509_STORE_CTX_get_ex_data(x509_ctx,SSL_get_ex_data_X509_STORE_CTX_idx()))
            )
        );
#endif

    bool success=false;
    if (ctx->m_criteria) {
        ctx->m_criteria->setUsage(Credential::TLS_CREDENTIAL);
        // Bypass name check (handled for us by curl).
        ctx->m_criteria->setPeerName(NULL);
        success = ctx->m_trustEngine->validate(x509_ctx->cert,x509_ctx->untrusted,*(ctx->m_peerResolver),ctx->m_criteria);
    }
    else {
        // Bypass name check (handled for us by curl).
        CredentialCriteria cc;
        cc.setUsage(Credential::TLS_CREDENTIAL);
        success = ctx->m_trustEngine->validate(x509_ctx->cert,x509_ctx->untrusted,*(ctx->m_peerResolver),&cc);
    }

    if (!success) {
        log.error("supplied TrustEngine failed to validate SSL/TLS server certificate");
        x509_ctx->error=X509_V_ERR_APPLICATION_VERIFICATION;     // generic error, check log for plugin specifics
        ctx->setAuthenticated(false);
        return ctx->m_mandatory ? 0 : 1;
    }

    // Signal success. Hopefully it doesn't matter what's actually in the structure now.
    ctx->setAuthenticated(true);
    return 1;
}
#endif

// callback to invoke a caller-defined SSL callback
CURLcode xmltooling::xml_ssl_ctx_callback(CURL* curl, SSL_CTX* ssl_ctx, void* userptr)
{
    CURLSOAPTransport* conf = reinterpret_cast<CURLSOAPTransport*>(userptr);

    // Default flags manually disable SSLv2 so we're not dependent on libcurl to do it.
    // Also disable the ticket option where implemented, since this breaks a variety
    // of servers. Newer libcurl also does this for us.
#ifdef SSL_OP_NO_TICKET
    SSL_CTX_set_options(ssl_ctx, conf->m_openssl_ops|SSL_OP_NO_TICKET);
#else
    SSL_CTX_set_options(ssl_ctx, conf->m_openssl_ops);
#endif

#ifndef XMLTOOLING_NO_XMLSEC
    if (conf->m_cred)
        conf->m_cred->attach(ssl_ctx);

    if (conf->m_trustEngine) {
        SSL_CTX_set_verify(ssl_ctx,SSL_VERIFY_PEER,NULL);
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
        // With 0.9.7, we can pass a callback argument directly.
        SSL_CTX_set_cert_verify_callback(ssl_ctx,verify_callback,userptr);
#else
        // With 0.9.6, there's no argument, so we're going to use a really embarrassing hack and
        // stuff the argument in the depth property where it will get copied to the context object
        // that's handed to the callback.
        SSL_CTX_set_cert_verify_callback(ssl_ctx,reinterpret_cast<int (*)()>(verify_callback),NULL);
        SSL_CTX_set_verify_depth(ssl_ctx,reinterpret_cast<int>(userptr));
#endif
    }
#endif

    if (conf->m_ssl_callback && !conf->m_ssl_callback(conf, ssl_ctx, conf->m_ssl_userptr))
        return CURLE_SSL_CERTPROBLEM;

    return CURLE_OK;
}
