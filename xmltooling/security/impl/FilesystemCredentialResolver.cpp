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
 * FilesystemCredentialResolver.cpp
 * 
 * Supplies credentials from local files
 */

#include "internal.h"
#include "logging.h"
#include "security/BasicX509Credential.h"
#include "security/CredentialCriteria.h"
#include "security/CredentialResolver.h"
#include "security/KeyInfoResolver.h"
#include "security/OpenSSLCredential.h"
#include "security/OpenSSLCryptoX509CRL.h"
#include "util/NDC.h"
#include "util/PathResolver.h"
#include "util/XMLHelper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/pkcs12.h>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>

using namespace xmlsignature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

// OpenSSL password callback...
static int passwd_callback(char* buf, int len, int verify, void* passwd)
{
    if(!verify)
    {
        if(passwd && len > strlen(reinterpret_cast<char*>(passwd)))
        {
            strcpy(buf,reinterpret_cast<char*>(passwd));
            return strlen(buf);
        }
    }  
    return 0;
}

namespace xmltooling {

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 )
#endif

    class XMLTOOL_DLLLOCAL FilesystemCredentialResolver;
    class XMLTOOL_DLLLOCAL FilesystemCredential : public OpenSSLCredential, public BasicX509Credential
    {
    public:
        FilesystemCredential(
            FilesystemCredentialResolver* resolver, XSECCryptoKey* key, const std::vector<XSECCryptoX509*>& xseccerts, XSECCryptoX509CRL* crl=NULL
            ) : BasicX509Credential(key, xseccerts, crl), m_resolver(resolver), m_usage(UNSPECIFIED_CREDENTIAL) {
            extract();
            initKeyInfo();
        }
        virtual ~FilesystemCredential() {
        }

        unsigned int getUsage() const {
            return m_usage;
        }

        void setUsage(const XMLCh* usage) {
            if (usage && *usage) {
                auto_ptr_char u(usage);
                if (!strcmp(u.get(), "signing"))
                    m_usage = SIGNING_CREDENTIAL | TLS_CREDENTIAL;
                else if (!strcmp(u.get(), "TLS"))
                    m_usage = TLS_CREDENTIAL;
                else if (!strcmp(u.get(), "encryption"))
                    m_usage = ENCRYPTION_CREDENTIAL;
            }
        }

        void addKeyNames(const DOMElement* e);

        void attach(SSL_CTX* ctx) const;
    
    private:
        FilesystemCredentialResolver* m_resolver;
        unsigned int m_usage;
    };

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

    class XMLTOOL_DLLLOCAL FilesystemCredentialResolver : public CredentialResolver
    {
    public:
        FilesystemCredentialResolver(const DOMElement* e);
        virtual ~FilesystemCredentialResolver() {
            delete m_credential;
            for_each(m_certs.begin(),m_certs.end(),X509_free);
        }

        Lockable* lock() { return this; }
        void unlock() {}
        
        const Credential* resolve(const CredentialCriteria* criteria=NULL) const {
            return (criteria ? (criteria->matches(*m_credential) ? m_credential : NULL) : m_credential);
        }

        virtual vector<const Credential*>::size_type resolve(
            vector<const Credential*>& results, const CredentialCriteria* criteria=NULL
            ) const {
            if (!criteria || criteria->matches(*m_credential)) {
                results.push_back(m_credential);
                return 1;
            }
            return 0;
        }

        void attach(SSL_CTX* ctx) const;

    private:
        XSECCryptoKey* loadKey();
        XSECCryptoX509CRL* loadCRL();
        
        enum format_t { PEM=SSL_FILETYPE_PEM, DER=SSL_FILETYPE_ASN1, _PKCS12, UNKNOWN };
    
        format_t getEncodingFormat(BIO* in) const;
        string formatToString(format_t format) const;
        format_t xmlFormatToFormat(const XMLCh* format_xml) const;
    
        format_t m_keyformat,m_crlformat;
        string m_keypath,m_keypass,m_crlpath;
        vector<X509*> m_certs;
        FilesystemCredential* m_credential;
    };

    CredentialResolver* XMLTOOL_DLLLOCAL FilesystemCredentialResolverFactory(const DOMElement* const & e)
    {
        return new FilesystemCredentialResolver(e);
    }

    static const XMLCh _CredentialResolver[] =  UNICODE_LITERAL_18(C,r,e,d,e,n,t,i,a,l,R,e,s,o,l,v,e,r);
    static const XMLCh CAPath[] =           UNICODE_LITERAL_6(C,A,P,a,t,h);
    static const XMLCh Certificate[] =      UNICODE_LITERAL_11(C,e,r,t,i,f,i,c,a,t,e);
    static const XMLCh _certificate[] =     UNICODE_LITERAL_11(c,e,r,t,i,f,i,c,a,t,e);
    static const XMLCh CRL[] =              UNICODE_LITERAL_3(C,R,L);
    static const XMLCh format[] =           UNICODE_LITERAL_6(f,o,r,m,a,t);
    static const XMLCh Key[] =              UNICODE_LITERAL_3(K,e,y);
    static const XMLCh _key[] =             UNICODE_LITERAL_3(k,e,y);
    static const XMLCh keyName[] =          UNICODE_LITERAL_7(k,e,y,N,a,m,e);
    static const XMLCh Name[] =             UNICODE_LITERAL_4(N,a,m,e);
    static const XMLCh password[] =         UNICODE_LITERAL_8(p,a,s,s,w,o,r,d);
    static const XMLCh Path[] =             UNICODE_LITERAL_4(P,a,t,h);
    static const XMLCh _use[] =             UNICODE_LITERAL_3(u,s,e);
};

FilesystemCredentialResolver::FilesystemCredentialResolver(const DOMElement* e) : m_credential(NULL)
{
#ifdef _DEBUG
    NDC ndc("FilesystemCredentialResolver");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER);

    if (e && (e->hasAttributeNS(NULL,_certificate) || e->hasAttributeNS(NULL,_key))) {
        // Dummy up a simple file resolver config using these attributes.
        DOMElement* dummy = e->getOwnerDocument()->createElementNS(NULL,_CredentialResolver);
        DOMElement* child;
        DOMElement* path;
        if (e->hasAttributeNS(NULL,_key)) {
            child = e->getOwnerDocument()->createElementNS(NULL,Key);
            dummy->appendChild(child);
            path = e->getOwnerDocument()->createElementNS(NULL,Path);
            child->appendChild(path);
            path->appendChild(e->getOwnerDocument()->createTextNode(e->getAttributeNS(NULL,_key)));
            if (e->hasAttributeNS(NULL,password))
                child->setAttributeNS(NULL,password,e->getAttributeNS(NULL,password));
            if (e->hasAttributeNS(NULL,keyName)) {
                path = e->getOwnerDocument()->createElementNS(NULL,Name);
                child->appendChild(path);
                path->appendChild(e->getOwnerDocument()->createTextNode(e->getAttributeNS(NULL,keyName)));
            }
        }
        if (e->hasAttributeNS(NULL,_certificate)) {
            child = e->getOwnerDocument()->createElementNS(NULL,Certificate);
            dummy->appendChild(child);
            path = e->getOwnerDocument()->createElementNS(NULL,Path);
            child->appendChild(path);
            path->appendChild(e->getOwnerDocument()->createTextNode(e->getAttributeNS(NULL,_certificate)));
        }
        e = dummy;  // reset "root" to the dummy config element
    }
    
    const DOMElement* root=e;
    const XMLCh* usage = root->getAttributeNS(NULL,_use);

    XSECCryptoKey* key=NULL;
    vector<XSECCryptoX509*> xseccerts;
    XSECCryptoX509CRL* crl=NULL;

    format_t fformat;
    const XMLCh* format_xml=NULL;
    BIO* in = NULL;
    
    // Move to Key
    const DOMElement* keynode=XMLHelper::getFirstChildElement(root,Key);
    if (keynode) {
        // Get raw format attrib value, but defer processing til later since may need to 
        // determine format dynamically, and we need the Path for that.
        format_xml=keynode->getAttributeNS(NULL,format);
            
        const XMLCh* password_xml=keynode->getAttributeNS(NULL,password);
        if (password_xml) {
            auto_ptr_char kp(password_xml);
            m_keypass=kp.get();
        }
        
        e=XMLHelper::getFirstChildElement(keynode,Path);
        if (e && e->hasChildNodes()) {
            const XMLCh* s=e->getFirstChild()->getNodeValue();
            auto_ptr_char kpath(s);
            m_keypath = kpath.get();
            XMLToolingConfig::getConfig().getPathResolver()->resolve(m_keypath, PathResolver::XMLTOOLING_CFG_FILE);
#ifdef WIN32
            struct _stat stat_buf;
            if (_stat(m_keypath.c_str(), &stat_buf) != 0)
#else
            struct stat stat_buf;
            if (stat(m_keypath.c_str(), &stat_buf) != 0)
#endif
            {
                log.error("key file (%s) can't be opened", m_keypath.c_str());
                throw XMLSecurityException("FilesystemCredentialResolver can't access key file ($1)",params(1,m_keypath.c_str()));
            }
        }
        else {
            log.error("Path element missing inside Key element");
            throw XMLSecurityException("FilesystemCredentialResolver can't access key file, no Path element specified.");
        }

        // Determine the key encoding format dynamically, if not explicitly specified
        if (format_xml && *format_xml) {
            fformat = xmlFormatToFormat(format_xml);
            if (fformat != UNKNOWN) {
                m_keyformat = fformat;
            }
            else {
                auto_ptr_char unknown(format_xml);
                log.error("configuration specifies unknown key encoding format (%s)", unknown.get());
                throw XMLSecurityException("FilesystemCredentialResolver configuration contains unknown key encoding format ($1)",params(1,unknown.get()));
            }
        }
        else {
            in=BIO_new(BIO_s_file_internal());
            if (in && BIO_read_filename(in,m_keypath.c_str())>0) {
                m_keyformat = getEncodingFormat(in);
                log.debug("key encoding format for (%s) dynamically resolved as (%s)", m_keypath.c_str(), formatToString(m_keyformat).c_str());
            }
            else {
                log.error("key file (%s) can't be read to determine encoding format", m_keypath.c_str());
                throw XMLSecurityException("FilesystemCredentialResolver can't read key file ($1) to determine encoding format",params(1,m_keypath.c_str()));
            }
            if (in)
                BIO_free(in);
            in = NULL;    
        }
        
        // Load the key.
        key = loadKey();
    }
    
    // Check for CRL.
    const DOMElement* crlnode=XMLHelper::getFirstChildElement(root,CRL);
    if (crlnode) {
        // Get raw format attrib value, but defer processing til later since may need to 
        // determine format dynamically, and we need the Path for that.
        format_xml=crlnode->getAttributeNS(NULL,format);
            
        e=XMLHelper::getFirstChildElement(crlnode,Path);
        if (e && e->hasChildNodes()) {
            const XMLCh* s=e->getFirstChild()->getNodeValue();
            auto_ptr_char kpath(s);
            m_crlpath=kpath.get();
            XMLToolingConfig::getConfig().getPathResolver()->resolve(m_crlpath, PathResolver::XMLTOOLING_CFG_FILE);
#ifdef WIN32
            struct _stat stat_buf;
            if (_stat(m_crlpath.c_str(), &stat_buf) != 0)
#else
            struct stat stat_buf;
            if (stat(m_crlpath.c_str(), &stat_buf) != 0)
#endif
            {
                log.error("CRL file (%s) can't be opened", m_crlpath.c_str());
                throw XMLSecurityException("FilesystemCredentialResolver can't access CRL file ($1)",params(1,m_crlpath.c_str()));
            }
        }
        else {
            log.error("Path element missing inside CRL element");
            throw XMLSecurityException("FilesystemCredentialResolver can't access CRL file, no Path element specified.");
        }

        // Determine the CRL encoding format dynamically, if not explicitly specified
        if (format_xml && *format_xml) {
            fformat = xmlFormatToFormat(format_xml);
            if (fformat != UNKNOWN) {
                m_crlformat = fformat;
            }
            else {
                auto_ptr_char unknown(format_xml);
                log.error("configuration specifies unknown CRL encoding format (%s)", unknown.get());
                throw XMLSecurityException("FilesystemCredentialResolver configuration contains unknown CRL encoding format ($1)",params(1,unknown.get()));
            }
        }
        else {
            in=BIO_new(BIO_s_file_internal());
            if (in && BIO_read_filename(in,m_crlpath.c_str())>0) {
                m_crlformat = getEncodingFormat(in);
                log.debug("CRL encoding format for (%s) dynamically resolved as (%s)", m_crlpath.c_str(), formatToString(m_crlformat).c_str());
            }
            else {
                log.error("CRL file (%s) can't be read to determine encoding format", m_crlpath.c_str());
                throw XMLSecurityException("FilesystemCredentialResolver can't read CRL file ($1) to determine encoding format",params(1,m_crlpath.c_str()));
            }
            if (in)
                BIO_free(in);
            in = NULL;
        }
        
        // Load the key.
        crl = loadCRL();
    }

    // Check for Certificate
    e=XMLHelper::getFirstChildElement(root,Certificate);
    if (!e) {
        m_credential = new FilesystemCredential(this,key,xseccerts,crl);
        m_credential->addKeyNames(keynode);
        m_credential->setUsage(usage);
        return;
    }
    auto_ptr_char certpass(e->getAttributeNS(NULL,password));
    
    const DOMElement* ep=XMLHelper::getFirstChildElement(e,Path);
    if (!ep || !ep->hasChildNodes()) {
        log.error("Path element missing inside Certificate element or is empty");
        delete key;
        delete crl;
        throw XMLSecurityException("FilesystemCredentialResolver can't access certificate file, missing or empty Path element.");
    }
    
    auto_ptr_char certpath2(ep->getFirstChild()->getNodeValue());
    string certpath(certpath2.get());
    XMLToolingConfig::getConfig().getPathResolver()->resolve(certpath, PathResolver::XMLTOOLING_CFG_FILE);

    format_xml=e->getAttributeNS(NULL,format);
    if (format_xml && *format_xml) {
        fformat = xmlFormatToFormat(format_xml);
        if (fformat == UNKNOWN) {
            auto_ptr_char unknown(format_xml);
            log.error("configuration specifies unknown certificate encoding format (%s)", unknown.get());
            delete key;
            delete crl;
            throw XMLSecurityException("FilesystemCredentialResolver configuration contains unknown certificate encoding format ($1)",params(1,unknown.get()));
        }
    }
    
    try {
        X509* x=NULL;
        PKCS12* p12=NULL;
        in=BIO_new(BIO_s_file_internal());
        if (in && BIO_read_filename(in,certpath.c_str())>0) {
            if (!format_xml || !*format_xml) {
                // Determine the cert encoding format dynamically, if not explicitly specified
                fformat = getEncodingFormat(in);
                log.debug("certificate encoding format for (%s) dynamically resolved as (%s)", certpath.c_str(), formatToString(fformat).c_str());
            }

            Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER).info(
                "loading certificate from file (%s)", certpath.c_str()
                );

            switch(fformat) {
                case PEM:
                    while (x=PEM_read_bio_X509(in,NULL,passwd_callback,const_cast<char*>(certpass.get())))
                        m_certs.push_back(x);
                    break;
                                
                case DER:
                    x=d2i_X509_bio(in,NULL);
                    if (x)
                        m_certs.push_back(x);
                    else {
                        log_openssl();
                        BIO_free(in);
                        throw XMLSecurityException("FilesystemCredentialResolver unable to load DER certificate from file ($1)",params(1,certpath.c_str()));
                    }
                    break;

                case _PKCS12:
                    p12=d2i_PKCS12_bio(in,NULL);
                    if (p12) {
                        PKCS12_parse(p12, certpass.get(), NULL, &x, NULL);
                        PKCS12_free(p12);
                    }
                    if (x) {
                        m_certs.push_back(x);
                        x=NULL;
                    } else {
                        log_openssl();
                        BIO_free(in);
                        throw XMLSecurityException("FilesystemCredentialResolver unable to load PKCS12 certificate from file ($1)",params(1,certpath.c_str()));
                    }
                    break;
            } // end switch

        } else {
            log_openssl();
            if (in) {
                BIO_free(in);
                in=NULL;
            }
            throw XMLSecurityException("FilesystemCredentialResolver unable to load certificate(s) from file ($1)",params(1,certpath.c_str()));
        }
        if (in) {
            BIO_free(in);
            in=NULL;
        }

        if (m_certs.empty())
            throw XMLSecurityException("FilesystemCredentialResolver unable to load any certificate(s)");

        // Load any extra CA files.
        const DOMElement* extra=XMLHelper::getFirstChildElement(e,CAPath);
        while (extra) {
            if (!extra->hasChildNodes()) {
                log.warn("skipping empty CAPath element");
                extra = XMLHelper::getNextSiblingElement(extra,CAPath);
                continue;
            }
            auto_ptr_char capath2(extra->getFirstChild()->getNodeValue());
            string capath(capath2.get());
            XMLToolingConfig::getConfig().getPathResolver()->resolve(capath, PathResolver::XMLTOOLING_CFG_FILE);
            x=NULL;
            p12=NULL;
            in=BIO_new(BIO_s_file_internal());
            if (in && BIO_read_filename(in,capath.c_str())>0) {
                if (!format_xml || !*format_xml) {
                    // Determine the cert encoding format dynamically, if not explicitly specified
                    fformat = getEncodingFormat(in);
                    log.debug("CA certificate encoding format for (%s) dynamically resolved as (%s)", capath.c_str(), formatToString(fformat).c_str());
                }

                Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER).info(
                    "loading CA certificate from file (%s)", capath.c_str()
                    );

                switch (fformat) {
                    case PEM:
                        while (x=PEM_read_bio_X509(in,NULL,NULL,NULL))
                            m_certs.push_back(x);
                        break;

                    case DER:
                        x=d2i_X509_bio(in,NULL);
                        if (x)
                            m_certs.push_back(x);
                        else {
                            log_openssl();
                            BIO_free(in);
                            throw XMLSecurityException("FilesystemCredentialResolver unable to load DER CA certificate from file ($1)",params(1,capath.c_str()));
                        }
                        break;

                    case _PKCS12:
                        p12 = d2i_PKCS12_bio(in, NULL);
                        if (p12) {
                            PKCS12_parse(p12, NULL, NULL, &x, NULL);
                            PKCS12_free(p12);
                        }
                        if (x) {
                            m_certs.push_back(x);
                            x=NULL;
                        }
                        else {
                            log_openssl();
                            BIO_free(in);
                            throw XMLSecurityException("FilesystemCredentialResolver unable to load PKCS12 CA certificate from file ($1)",params(1,capath.c_str()));
                        }
                        break;
                } //end switch

                BIO_free(in);
            }
            else {
                if (in)
                    BIO_free(in);
                log_openssl();
                log.error("CA certificate file (%s) can't be opened", capath.c_str());
                throw XMLSecurityException("FilesystemCredentialResolver can't open CA certificate file ($1)",params(1,capath.c_str()));
            }
            
            extra = XMLHelper::getNextSiblingElement(extra,CAPath);
        }
    }
    catch (XMLToolingException&) {
        delete key;
        delete crl;
        for_each(m_certs.begin(), m_certs.end(), X509_free);
        throw;
    }

    // Reflect certs over to XSEC form and wrap with credential object.
    for (vector<X509*>::iterator j=m_certs.begin(); j!=m_certs.end(); j++)
        xseccerts.push_back(new OpenSSLCryptoX509(*j));
    if (!key && !xseccerts.empty())
        key = xseccerts.front()->clonePublicKey();
    m_credential = new FilesystemCredential(this, key, xseccerts, crl);
    m_credential->addKeyNames(keynode);
    m_credential->setUsage(usage);
}

XSECCryptoKey* FilesystemCredentialResolver::loadKey()
{
#ifdef _DEBUG
    NDC ndc("loadKey");
#endif
    Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER).info(
        "loading private key from file (%s)", m_keypath.c_str()
        );

    // Get a EVP_PKEY.
    EVP_PKEY* pkey=NULL;
    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in,m_keypath.c_str())>0) {
        switch (m_keyformat) {
            case PEM:
                pkey=PEM_read_bio_PrivateKey(in, NULL, passwd_callback, const_cast<char*>(m_keypass.c_str()));
                break;
            
            case DER:
                pkey=d2i_PrivateKey_bio(in, NULL);
                break;
                
            default: {
                PKCS12* p12 = d2i_PKCS12_bio(in, NULL);
                if (p12) {
                    PKCS12_parse(p12, const_cast<char*>(m_keypass.c_str()), &pkey, NULL, NULL);
                    PKCS12_free(p12);
                }
            }
        }
    }
    if (in)
        BIO_free(in);
    
    // Now map it to an XSEC wrapper.
    if (pkey) {
        XSECCryptoKey* ret=NULL;
        switch (pkey->type) {
            case EVP_PKEY_RSA:
                ret=new OpenSSLCryptoKeyRSA(pkey);
                break;
                
            case EVP_PKEY_DSA:
                ret=new OpenSSLCryptoKeyDSA(pkey);
                break;
            
            default:
                Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER).error("unsupported private key type");
        }
        EVP_PKEY_free(pkey);
        if (ret)
            return ret;
    }

    log_openssl();
    throw XMLSecurityException("FilesystemCredentialResolver unable to load private key from file."); 
}

XSECCryptoX509CRL* FilesystemCredentialResolver::loadCRL()
{
#ifdef _DEBUG
    NDC ndc("loadCRL");
#endif
    Category::getInstance(XMLTOOLING_LOGCAT".CredentialResolver."FILESYSTEM_CREDENTIAL_RESOLVER).info(
        "loading CRL from file (%s)", m_crlpath.c_str()
        );

    X509_CRL* crl=NULL;
    BIO* in=BIO_new(BIO_s_file_internal());
    if (in && BIO_read_filename(in,m_crlpath.c_str())>0) {
        switch (m_crlformat) {
            case PEM:
                crl=PEM_read_bio_X509_CRL(in, NULL, NULL, NULL);
                break;
            
            case DER:
                crl=d2i_X509_CRL_bio(in, NULL);
                break;
        }
    }
    if (in)
        BIO_free(in);
    
    // Now map it to an XSEC wrapper.
    if (crl) {
        XSECCryptoX509CRL* ret=new OpenSSLCryptoX509CRL(crl);
        X509_CRL_free(crl);
        return ret;
    }

    log_openssl();
    throw XMLSecurityException("FilesystemCredentialResolver unable to load CRL from file."); 
}

// Used to determine the encoding format of credentials files
// dynamically. Supports: PEM, DER, PKCS12.
FilesystemCredentialResolver::format_t FilesystemCredentialResolver::getEncodingFormat(BIO* in) const
{
    PKCS12* p12 = NULL;
    format_t format;

    const int READSIZE = 1;
    char buf[READSIZE];
    char b1;
    int mark;

    try {
        if ( (mark = BIO_tell(in)) < 0 ) 
            throw XMLSecurityException("getEncodingFormat: BIO_tell() can't get the file position");
        if ( BIO_read(in, buf, READSIZE) <= 0 ) 
            throw XMLSecurityException("getEncodingFormat: BIO_read() can't read from the stream");
        if ( BIO_seek(in, mark) < 0 ) 
            throw XMLSecurityException("getEncodingFormat: BIO_seek() can't reset the file position");
    }
    catch (...) {
        log_openssl();
        throw;
    }

    b1 = buf[0];

    // This is a slight variation of the Java code by Chad La Joie.
    //
    // Check the first byte of the file.  If it's some kind of
    // DER-encoded structure (including PKCS12), it will begin with ASCII 048.
    // Otherwise, assume it's PEM.
    if (b1 !=  48) {
        format = PEM;
    } else {
        // Here we know it's DER-encoded, now try to parse it as a PKCS12
        // ASN.1 structure.  If it fails, must be another kind of DER-encoded
        // key/cert structure.  A little inefficient...but it works.
        if ( (p12=d2i_PKCS12_bio(in,NULL)) == NULL ) {
            format = DER;
        } else {
            format = _PKCS12;
        }
        if (p12)
            PKCS12_free(p12);    
        if ( BIO_seek(in, mark) < 0 ) {
            log_openssl();
            throw XMLSecurityException("getEncodingFormat: BIO_seek() can't reset the file position");
        }
    }

    return format;
}

// Convert key/cert format_t types to a human-meaningful string for debug output
string FilesystemCredentialResolver::formatToString(format_t format) const
{
    switch(format) {
        case PEM:
            return "PEM";
        case DER:
            return "DER";
        case _PKCS12:
            return "PKCS12";
        default:
            return "UNKNOWN";
    }
}

// Convert key/cert raw XML format attribute (XMLCh[]) to format_t type
FilesystemCredentialResolver::format_t FilesystemCredentialResolver::xmlFormatToFormat(const XMLCh* format_xml) const
{
    static const XMLCh cPEM[] = UNICODE_LITERAL_3(P,E,M);
    static const XMLCh cDER[] = UNICODE_LITERAL_3(D,E,R);
    static const XMLCh cPKCS12[] = { chLatin_P, chLatin_K, chLatin_C, chLatin_S, chDigit_1, chDigit_2, chNull };
    format_t format;

    if (!XMLString::compareString(format_xml,cPEM))
        format=PEM;
    else if (!XMLString::compareString(format_xml,cDER))
        format=DER;
    else if (!XMLString::compareString(format_xml,cPKCS12))
        format=_PKCS12;
    else
        format=UNKNOWN;

    return format;
}

void FilesystemCredentialResolver::attach(SSL_CTX* ctx) const
{
#ifdef _DEBUG
    NDC ndc("attach");
#endif

    if (m_keypath.empty())
        throw XMLSecurityException("No key available, unable to attach private key to SSL context.");

    // Attach key.
    SSL_CTX_set_default_passwd_cb(ctx, passwd_callback);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, const_cast<char*>(m_keypass.c_str()));

    int ret=0;
    switch (m_keyformat) {
        case PEM:
            ret=SSL_CTX_use_PrivateKey_file(ctx, m_keypath.c_str(), m_keyformat);
            break;
            
        case DER:
            ret=SSL_CTX_use_RSAPrivateKey_file(ctx, m_keypath.c_str(), m_keyformat);
            break;
            
        default: {
            BIO* in=BIO_new(BIO_s_file_internal());
            if (in && BIO_read_filename(in,m_keypath.c_str())>0) {
                EVP_PKEY* pkey=NULL;
                PKCS12* p12 = d2i_PKCS12_bio(in, NULL);
                if (p12) {
                    PKCS12_parse(p12, const_cast<char*>(m_keypass.c_str()), &pkey, NULL, NULL);
                    PKCS12_free(p12);
                    if (pkey) {
                        ret=SSL_CTX_use_PrivateKey(ctx, pkey);
                        EVP_PKEY_free(pkey);
                    }
                }
            }
            if (in)
                BIO_free(in);
        }
    }
    
    if (ret!=1) {
        log_openssl();
        throw XMLSecurityException("Unable to attach private key to SSL context.");
    }

    // Attach certs.
    for (vector<X509*>::const_iterator i=m_certs.begin(); i!=m_certs.end(); i++) {
        if (i==m_certs.begin()) {
            if (SSL_CTX_use_certificate(ctx, *i) != 1) {
                log_openssl();
                throw XMLSecurityException("Unable to attach client certificate to SSL context.");
            }
        }
        else {
            // When we add certs, they don't get ref counted, so we need to duplicate them.
            X509* dup = X509_dup(*i);
            if (SSL_CTX_add_extra_chain_cert(ctx, dup) != 1) {
                X509_free(dup);
                log_openssl();
                throw XMLSecurityException("Unable to attach CA certificate to SSL context.");
            }
        }
    }
}

void FilesystemCredential::addKeyNames(const DOMElement* e)
{
    e = e ? XMLHelper::getFirstChildElement(e, Name) : NULL;
    while (e) {
        if (e->hasChildNodes()) {
            auto_ptr_char n(e->getFirstChild()->getNodeValue());
            if (n.get() && *n.get())
                m_keyNames.insert(n.get());
        }
        e = XMLHelper::getNextSiblingElement(e, Name);
    }
}

void FilesystemCredential::attach(SSL_CTX* ctx) const
{
    return m_resolver->attach(ctx);
}
