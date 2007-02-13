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
 * CURLSOAPTransport.cpp
 * 
 * libcurl-based SOAPTransport implementation
 */

#include "internal.h"
#include "exceptions.h"
#include "security/OpenSSLTrustEngine.h"
#include "signature/OpenSSLCredentialResolver.h"
#include "soap/HTTPSOAPTransport.h"
#include "soap/OpenSSLSOAPTransport.h"
#include "util/NDC.h"
#include "util/Threads.h"

#include <list>
#include <curl/curl.h>
#include <log4cpp/Category.hh>
#include <openssl/x509_vfy.h>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace xmltooling {

    // Manages cache of socket connections via CURL handles.
    class XMLTOOL_DLLLOCAL CURLPool
    {
    public:
        CURLPool() : m_size(256), m_lock(Mutex::create()),
            m_log(Category::getInstance(XMLTOOLING_LOGCAT".SOAPTransport.CURLPool")) {}
        ~CURLPool();
        
        CURL* get(const string& to, const char* endpoint);
        void put(const string& to, const char* endpoint, CURL* handle);
    
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
        CURLSOAPTransport(const KeyInfoSource& peer, const char* endpoint)
                : m_peer(peer), m_endpoint(endpoint), m_handle(NULL), m_headers(NULL),
#ifndef XMLTOOLING_NO_XMLSEC
                    m_credResolver(NULL), m_trustEngine(NULL), m_mandatory(false), m_keyResolver(NULL),
#endif
                    m_ssl_callback(NULL), m_ssl_userptr(NULL), m_secure(false) {
            m_handle = g_CURLPool->get(peer.getName(), endpoint);
            curl_easy_setopt(m_handle,CURLOPT_URL,endpoint);
            curl_easy_setopt(m_handle,CURLOPT_CONNECTTIMEOUT,15);
            curl_easy_setopt(m_handle,CURLOPT_TIMEOUT,30);
            curl_easy_setopt(m_handle,CURLOPT_HTTPAUTH,0);
            curl_easy_setopt(m_handle,CURLOPT_USERPWD,NULL);
            curl_easy_setopt(m_handle,CURLOPT_HEADERDATA,this);
            m_headers=curl_slist_append(m_headers,"Content-Type: text/xml");
            m_headers=curl_slist_append(m_headers,"Transport-Encoding: chunked");
        }
        
        virtual ~CURLSOAPTransport() {
            curl_slist_free_all(m_headers);
            curl_easy_setopt(m_handle,CURLOPT_ERRORBUFFER,NULL);
            curl_easy_setopt(m_handle,CURLOPT_PRIVATE,m_secure ? "secure" : NULL); // Save off security "state".
            g_CURLPool->put(m_peer.getName(), m_endpoint.c_str(), m_handle);
        }

        bool setConnectTimeout(long timeout) const {
            return (curl_easy_setopt(m_handle,CURLOPT_CONNECTTIMEOUT,timeout)==CURLE_OK);
        }
        
        bool setTimeout(long timeout) const {
            return (curl_easy_setopt(m_handle,CURLOPT_TIMEOUT,timeout)==CURLE_OK);
        }
        
        bool setAuth(transport_auth_t authType, const char* username=NULL, const char* password=NULL) const;
        
#ifndef XMLTOOLING_NO_XMLSEC
        bool setCredentialResolver(const CredentialResolver* credResolver) const {
            const OpenSSLCredentialResolver* down = dynamic_cast<const OpenSSLCredentialResolver*>(credResolver);
            if (!down) {
                m_credResolver = NULL;
                return (credResolver==NULL);
            }
            m_credResolver = down;
            return true;
        }
        
        bool setTrustEngine(const X509TrustEngine* trustEngine, bool mandatory=true, const KeyResolver* keyResolver=NULL) const {
            const OpenSSLTrustEngine* down = dynamic_cast<const OpenSSLTrustEngine*>(trustEngine);
            if (!down) {
                m_trustEngine = NULL;
                m_keyResolver = NULL;
                return (trustEngine==NULL);
            }
            m_trustEngine = down;
            m_keyResolver = keyResolver;
            m_mandatory = mandatory;
            return true;
        }
        
#endif
        
        void send(istream& in);
        
        istream& receive() {
            return m_stream;
        }
        
        bool isSecure() const {
            return m_secure;
        }

        void setSecure(bool secure) {
            m_secure = secure;
        }

        string getContentType() const;
        
        bool setRequestHeader(const char* name, const char* val) const {
            string temp(name);
            temp=temp + ": " + val;
            m_headers=curl_slist_append(m_headers,temp.c_str());
            return true;
        }
        
        const vector<string>& getResponseHeader(const char* val) const;
        
        bool setSSLCallback(ssl_ctx_callback_fn fn, void* userptr=NULL) const {
            m_ssl_callback=fn;
            m_ssl_userptr=userptr;
            return true;
        }

    private:        
        // per-call state
        const KeyInfoSource& m_peer;
        string m_endpoint;
        CURL* m_handle;
        stringstream m_stream;
        mutable struct curl_slist* m_headers;
        map<string,vector<string> > m_response_headers;
#ifndef XMLTOOLING_NO_XMLSEC
        mutable const OpenSSLCredentialResolver* m_credResolver;
        mutable const OpenSSLTrustEngine* m_trustEngine;
        mutable bool m_mandatory;
        mutable const KeyResolver* m_keyResolver;
#endif
        mutable ssl_ctx_callback_fn m_ssl_callback;
        mutable void* m_ssl_userptr;
        bool m_secure;
        
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

    SOAPTransport* CURLSOAPTransportFactory(const pair<const KeyInfoSource*,const char*>& dest)
    {
        return new CURLSOAPTransport(*dest.first, dest.second);
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

CURLPool::~CURLPool()
{
    for (poolmap_t::iterator i=m_bindingMap.begin(); i!=m_bindingMap.end(); i++) {
        for (vector<CURL*>::iterator j=i->second.begin(); j!=i->second.end(); j++)
            curl_easy_cleanup(*j);
    }
    delete m_lock;
}

CURL* CURLPool::get(const string& to, const char* endpoint)
{
#ifdef _DEBUG
    xmltooling::NDC("get");
#endif
    m_log.debug("getting connection handle to %s", endpoint);
    m_lock->lock();
    poolmap_t::iterator i=m_bindingMap.find(to + "|" + endpoint);
    
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
    curl_easy_setopt(handle,CURLOPT_SSLVERSION,3);
    curl_easy_setopt(handle,CURLOPT_SSL_VERIFYHOST,2);
    curl_easy_setopt(handle,CURLOPT_HEADERFUNCTION,&curl_header_hook);
    curl_easy_setopt(handle,CURLOPT_READFUNCTION,&curl_read_hook);
    curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,&curl_write_hook);
    curl_easy_setopt(handle,CURLOPT_DEBUGFUNCTION,&curl_debug_hook);

    return handle;
}

void CURLPool::put(const string& to, const char* endpoint, CURL* handle)
{
    string key = to + "|" + endpoint;
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

bool CURLSOAPTransport::setAuth(transport_auth_t authType, const char* username, const char* password) const
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
    string creds = string(username ? username : "") + ':' + (password ? password : "");
    return (curl_easy_setopt(m_handle,CURLOPT_USERPWD,creds.c_str())==CURLE_OK);
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

void CURLSOAPTransport::send(istream& in)
{
#ifdef _DEBUG
    xmltooling::NDC ndc("send");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".SOAPTransport");
    Category& log_curl=Category::getInstance(XMLTOOLING_LOGCAT".libcurl");

    // By this time, the handle has been prepared with the URL to use and the
    // caller should have executed any set functions to manipulate it.

    // Setup standard per-call curl properties.
    curl_easy_setopt(m_handle,CURLOPT_POST,1);
    curl_easy_setopt(m_handle,CURLOPT_READDATA,&in);
    curl_easy_setopt(m_handle,CURLOPT_FILE,&m_stream);
    curl_easy_setopt(m_handle,CURLOPT_DEBUGDATA,&log_curl);

    char curl_errorbuf[CURL_ERROR_SIZE];
    curl_errorbuf[0]=0;
    curl_easy_setopt(m_handle,CURLOPT_ERRORBUFFER,curl_errorbuf);
    if (log_curl.isDebugEnabled())
        curl_easy_setopt(m_handle,CURLOPT_VERBOSE,1);

    // Set request headers (possibly appended by hooks).
    curl_easy_setopt(m_handle,CURLOPT_HTTPHEADER,m_headers);

    if (m_ssl_callback || m_credResolver || m_trustEngine) {
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_FUNCTION,xml_ssl_ctx_callback);
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_DATA,this);

        // Restore security "state". Necessary because the callback only runs
        // when handshakes occur. Even new TCP connections won't execute it.
        char* priv=NULL;
        curl_easy_getinfo(m_handle,CURLINFO_PRIVATE,&priv);
        if (priv)
            m_secure=true;
    }
    else {
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_FUNCTION,NULL);
        curl_easy_setopt(m_handle,CURLOPT_SSL_CTX_DATA,NULL);
    }
    
    // Verification of the peer is via TrustEngine only.
    curl_easy_setopt(m_handle,CURLOPT_SSL_VERIFYPEER,0);

    // Make the call.
    log.debug("sending SOAP message to %s", m_endpoint.c_str());
    if (curl_easy_perform(m_handle) != CURLE_OK) {
        throw IOException(
            string("CURLSOAPTransport failed while contacting SOAP responder: ") +
                (curl_errorbuf[0] ? curl_errorbuf : "no further information available"));
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
    // *stream is actually an istream object
    istream& buf=*(reinterpret_cast<istream*>(stream));
    buf.read(reinterpret_cast<char*>(ptr),size*nmemb);
    return buf.gcount();
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
    for (char* ch=data; len && (isprint(*ch) || isspace(*ch)); len--)
        log << *ch++;
    log << CategoryStream::ENDLINE;
    return 0;
}

#ifndef XMLTOOLING_NO_XMLSEC
int xmltooling::verify_callback(X509_STORE_CTX* x509_ctx, void* arg)
{
    Category& log = Category::getInstance("OpenSSL");
    log.debug("invoking X509 verify callback");
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

     // Bypass name check (handled for us by curl).
    if (!ctx->m_trustEngine->validate(x509_ctx->cert,x509_ctx->untrusted,ctx->m_peer,false,ctx->m_keyResolver)) {
        log.error("supplied TrustEngine failed to validate SSL/TLS server certificate");
        x509_ctx->error=X509_V_ERR_APPLICATION_VERIFICATION;     // generic error, check log for plugin specifics
        ctx->setSecure(false);
        return ctx->m_mandatory ? 0 : 1;
    }
    
    // Signal success. Hopefully it doesn't matter what's actually in the structure now.
    ctx->setSecure(true);
    return 1;
}
#endif

// callback to invoke a caller-defined SSL callback
CURLcode xmltooling::xml_ssl_ctx_callback(CURL* curl, SSL_CTX* ssl_ctx, void* userptr)
{
    CURLSOAPTransport* conf = reinterpret_cast<CURLSOAPTransport*>(userptr);

#ifndef XMLTOOLING_NO_XMLSEC
    if (conf->m_credResolver)
        conf->m_credResolver->attach(ssl_ctx);

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
