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
 * AbstractPKIXTrustEngine.cpp
 * 
 * A trust engine that uses X.509 trust anchors and CRLs associated with a KeyInfoSource
 * to perform PKIX validation of signatures and certificates.
 */

#include "internal.h"
#include "security/AbstractPKIXTrustEngine.h"
#include "signature/KeyInfo.h"

#include <log4cpp/Category.hh>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <xmltooling/security/CredentialCriteria.h>
#include <xmltooling/security/CredentialResolver.h>
#include <xmltooling/security/KeyInfoResolver.h>
#include <xmltooling/security/OpenSSLCryptoX509CRL.h>
#include <xmltooling/security/X509Credential.h>
#include <xmltooling/signature/SignatureValidator.h>
#include <xmltooling/util/NDC.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;


namespace {
    static int XMLTOOL_DLLLOCAL error_callback(int ok, X509_STORE_CTX* ctx)
    {
        if (!ok)
            Category::getInstance("OpenSSL").error("path validation failure: %s", X509_verify_cert_error_string(ctx->error));
        return ok;
    }

    static bool XMLTOOL_DLLLOCAL validate(
        X509* EE, STACK_OF(X509)* untrusted, AbstractPKIXTrustEngine::PKIXValidationInfoIterator* pkixInfo
        )
    {
        Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
        // First we build a stack of CA certs. These objects are all referenced in place.
        log.debug("building CA list from PKIX Validation information");
    
        // We need this for CRL support.
        X509_STORE* store=X509_STORE_new();
        if (!store) {
            log_openssl();
            return false;
        }
    #if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
        X509_STORE_set_flags(store,X509_V_FLAG_CRL_CHECK_ALL);
    #endif
    
        STACK_OF(X509)* CAstack = sk_X509_new_null();
        
        // This contains the state of the validate operation.
        X509_STORE_CTX ctx;
        
        const vector<XSECCryptoX509*>& CAcerts = pkixInfo->getTrustAnchors();
        for (vector<XSECCryptoX509*>::const_iterator i=CAcerts.begin(); i!=CAcerts.end(); ++i) {
            if ((*i)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                sk_X509_push(CAstack,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
            }
        }

        const vector<XSECCryptoX509CRL*>& crls = pkixInfo->getCRLs();
        for (vector<XSECCryptoX509CRL*>::const_iterator j=crls.begin(); j!=crls.end(); ++j) {
            if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                // owned by store
                X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
            }
        }
     
        // AFAICT, EE and untrusted are passed in but not owned by the ctx.
    #if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
        if (X509_STORE_CTX_init(&ctx,store,EE,untrusted)!=1) {
            log_openssl();
            log.error("unable to initialize X509_STORE_CTX");
            sk_X509_free(CAstack);
            X509_STORE_free(store);
            return false;
        }
    #else
        X509_STORE_CTX_init(&ctx,store,EE,untrusted);
    #endif
    
        // Seems to be most efficient to just pass in the CA stack.
        X509_STORE_CTX_trusted_stack(&ctx,CAstack);
        X509_STORE_CTX_set_depth(&ctx,100);    // we check the depth down below
        X509_STORE_CTX_set_verify_cb(&ctx,error_callback);
        
        int ret=X509_verify_cert(&ctx);
        if (ret==1) {
            // Now see if the depth was acceptable by counting the number of intermediates.
            int depth=sk_X509_num(ctx.chain)-2;
            if (pkixInfo->getVerificationDepth() < depth) {
                log.error(
                    "certificate chain was too long (%d intermediates, only %d allowed)",
                    (depth==-1) ? 0 : depth,
                    pkixInfo->getVerificationDepth()
                    );
                ret=0;
            }
        }
        
        // Clean up...
        X509_STORE_CTX_cleanup(&ctx);
        X509_STORE_free(store);
        sk_X509_free(CAstack);
    
        if (ret==1) {
            log.debug("successfully validated certificate chain");
            return true;
        }
        
        return false;
    }
};

bool AbstractPKIXTrustEngine::checkEntityNames(
    X509* certEE, const CredentialResolver& credResolver, const CredentialCriteria& criteria
    ) const
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");

    vector<const Credential*> creds;
    credResolver.resolve(creds,&criteria);

    // Build a list of acceptable names.
    vector<string> keynames(1,criteria.getPeerName());
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred)
        (*cred)->getKeyNames(keynames);

    char buf[256];
    X509_NAME* subject=X509_get_subject_name(certEE);
    if (subject) {
        // One way is a direct match to the subject DN.
        // Seems that the way to do the compare is to write the X509_NAME into a BIO.
        BIO* b = BIO_new(BIO_s_mem());
        BIO* b2 = BIO_new(BIO_s_mem());
        BIO_set_mem_eof_return(b, 0);
        BIO_set_mem_eof_return(b2, 0);
        // The flags give us LDAP order instead of X.500, with a comma separator.
        int len=X509_NAME_print_ex(b,subject,0,XN_FLAG_RFC2253);
        string subjectstr,subjectstr2;
        BIO_flush(b);
        while ((len = BIO_read(b, buf, 255)) > 0) {
            buf[len] = '\0';
            subjectstr+=buf;
        }
        log.debugStream() << "certificate subject: " << subjectstr << CategoryStream::ENDLINE;
        // The flags give us LDAP order instead of X.500, with a comma plus space separator.
        len=X509_NAME_print_ex(b2,subject,0,XN_FLAG_RFC2253 + XN_FLAG_SEP_CPLUS_SPC - XN_FLAG_SEP_COMMA_PLUS);
        BIO_flush(b2);
        while ((len = BIO_read(b2, buf, 255)) > 0) {
            buf[len] = '\0';
            subjectstr2+=buf;
        }
        
        // Check each keyname.
        for (vector<string>::const_iterator n=keynames.begin(); n!=keynames.end(); n++) {
#ifdef HAVE_STRCASECMP
            if (!strcasecmp(n->c_str(),subjectstr.c_str()) || !strcasecmp(n->c_str(),subjectstr2.c_str())) {
#else
            if (!stricmp(n->c_str(),subjectstr.c_str()) || !stricmp(n->c_str(),subjectstr2.c_str())) {
#endif
                log.debug("matched full subject DN to a key name (%s)", n->c_str());
                BIO_free(b);
                BIO_free(b2);
                return true;
            }
        }
        BIO_free(b);
        BIO_free(b2);

        log.debug("unable to match DN, trying TLS subjectAltName match");
        STACK_OF(GENERAL_NAME)* altnames=(STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i(certEE, NID_subject_alt_name, NULL, NULL);
        if (altnames) {
            int numalts = sk_GENERAL_NAME_num(altnames);
            for (int an=0; an<numalts; an++) {
                const GENERAL_NAME* check = sk_GENERAL_NAME_value(altnames, an);
                if (check->type==GEN_DNS || check->type==GEN_URI) {
                    const char* altptr = (char*)ASN1_STRING_data(check->d.ia5);
                    const int altlen = ASN1_STRING_length(check->d.ia5);
                    
                    for (vector<string>::const_iterator n=keynames.begin(); n!=keynames.end(); n++) {
#ifdef HAVE_STRCASECMP
                        if ((check->type==GEN_DNS && !strncasecmp(altptr,n->c_str(),altlen))
#else
                        if ((check->type==GEN_DNS && !strnicmp(altptr,n->c_str(),altlen))
#endif
                                || (check->type==GEN_URI && !strncmp(altptr,n->c_str(),altlen))) {
                            log.debug("matched DNS/URI subjectAltName to a key name (%s)", n->c_str());
                            GENERAL_NAMES_free(altnames);
                            return true;
                        }
                    }
                }
            }
        }
        GENERAL_NAMES_free(altnames);
            
        log.debug("unable to match subjectAltName, trying TLS CN match");
        memset(buf,0,sizeof(buf));
        if (X509_NAME_get_text_by_NID(subject,NID_commonName,buf,255)>0) {
            for (vector<string>::const_iterator n=keynames.begin(); n!=keynames.end(); n++) {
#ifdef HAVE_STRCASECMP
                if (!strcasecmp(buf,n->c_str())) {
#else
                if (!stricmp(buf,n->c_str())) {
#endif
                    log.debug("matched subject CN to a key name (%s)", n->c_str());
                    return true;
                }
            }
        }
        else
            log.warn("no common name in certificate subject");
    }
    else
        log.error("certificate has no subject?!");
    
    return false;
}

bool AbstractPKIXTrustEngine::validate(
    X509* certEE,
    STACK_OF(X509)* certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");

    if (!certEE) {
        log.error("X.509 credential was NULL, unable to perform validation");
        return false;
    }

    if (criteria && criteria->getPeerName() && *(criteria->getPeerName())) {
        log.debug("checking that the certificate name is acceptable");
        if (criteria->getUsage()==CredentialCriteria::UNSPECIFIED_CREDENTIAL)
            criteria->setUsage(CredentialCriteria::SIGNING_CREDENTIAL);
        if (!checkEntityNames(certEE,credResolver,*criteria)) {
            log.error("certificate name was not acceptable");
            return false;
        }
    }
    
    log.debug("performing certificate path validation...");

    auto_ptr<PKIXValidationInfoIterator> pkix(getPKIXValidationInfoIterator(credResolver, criteria));
    while (pkix->next()) {
        if (::validate(certEE,certChain,pkix.get())) {
            return true;
        }
    }

    log.debug("failed to validate certificate chain using supplied PKIX information");
    return false;
}

bool AbstractPKIXTrustEngine::validate(
    XSECCryptoX509* certEE,
    const vector<XSECCryptoX509*>& certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
#ifdef _DEBUG
        NDC ndc("validate");
#endif
    if (!certEE) {
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine").error("X.509 credential was NULL, unable to perform validation");
        return false;
    }
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine").error("only the OpenSSL XSEC provider is supported");
        return false;
    }

    STACK_OF(X509)* untrusted=sk_X509_new_null();
    for (vector<XSECCryptoX509*>::const_iterator i=certChain.begin(); i!=certChain.end(); ++i)
        sk_X509_push(untrusted,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());

    bool ret = validate(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(), untrusted, credResolver, criteria);
    sk_X509_free(untrusted);
    return ret;
}

bool AbstractPKIXTrustEngine::validate(
    Signature& sig,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");

    const KeyInfoResolver* inlineResolver = m_keyInfoResolver;
    if (!inlineResolver)
        inlineResolver = XMLToolingConfig::getConfig().getKeyInfoResolver();
    if (!inlineResolver) {
        log.error("unable to perform PKIX validation, no KeyInfoResolver available");
        return false;
    }

    // Pull the certificate chain out of the signature.
    X509Credential* x509cred;
    auto_ptr<Credential> cred(inlineResolver->resolve(&sig,X509Credential::RESOLVE_CERTS));
    if (!cred.get() || !(x509cred=dynamic_cast<X509Credential*>(cred.get()))) {
        log.error("unable to perform PKIX validation, signature does not contain any certificates");
        return false;
    }
    const vector<XSECCryptoX509*>& certs = x509cred->getEntityCertificateChain();
    if (certs.empty()) {
        log.error("unable to perform PKIX validation, signature does not contain any certificates");
        return false;
    }

    log.debug("validating signature using certificate from within the signature");

    // Find and save off a pointer to the certificate that unlocks the object.
    // Most of the time, this will be the first one anyway.
    XSECCryptoX509* certEE=NULL;
    SignatureValidator keyValidator;
    for (vector<XSECCryptoX509*>::const_iterator i=certs.begin(); !certEE && i!=certs.end(); ++i) {
        try {
            auto_ptr<XSECCryptoKey> key((*i)->clonePublicKey());
            keyValidator.setKey(key.get());
            keyValidator.validate(&sig);
            log.debug("signature verified with key inside signature, attempting certificate validation...");
            certEE=(*i);
        }
        catch (ValidationException& ex) {
            log.debug(ex.what());
        }
    }
    
    if (certEE)
        return validate(certEE,certs,credResolver,criteria);
        
    log.debug("failed to verify signature with embedded certificates");
    return false;
}

bool AbstractPKIXTrustEngine::validate(
    const XMLCh* sigAlgorithm,
    const char* sig,
    KeyInfo* keyInfo,
    const char* in,
    unsigned int in_len,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");

    if (!keyInfo) {
        log.error("unable to perform PKIX validation, KeyInfo not present");
        return false;
    }

    const KeyInfoResolver* inlineResolver = m_keyInfoResolver;
    if (!inlineResolver)
        inlineResolver = XMLToolingConfig::getConfig().getKeyInfoResolver();
    if (!inlineResolver) {
        log.error("unable to perform PKIX validation, no KeyInfoResolver available");
        return false;
    }

    // Pull the certificate chain out of the signature.
    X509Credential* x509cred;
    auto_ptr<Credential> cred(inlineResolver->resolve(keyInfo,X509Credential::RESOLVE_CERTS));
    if (!cred.get() || !(x509cred=dynamic_cast<X509Credential*>(cred.get()))) {
        log.error("unable to perform PKIX validation, KeyInfo does not contain any certificates");
        return false;
    }
    const vector<XSECCryptoX509*>& certs = x509cred->getEntityCertificateChain();
    if (certs.empty()) {
        log.error("unable to perform PKIX validation, KeyInfo does not contain any certificates");
        return false;
    }

    log.debug("validating signature using certificate from within KeyInfo");

    // Find and save off a pointer to the certificate that unlocks the object.
    // Most of the time, this will be the first one anyway.
    XSECCryptoX509* certEE=NULL;
    for (vector<XSECCryptoX509*>::const_iterator i=certs.begin(); !certEE && i!=certs.end(); ++i) {
        try {
            auto_ptr<XSECCryptoKey> key((*i)->clonePublicKey());
            if (Signature::verifyRawSignature(key.get(), sigAlgorithm, sig, in, in_len)) {
                log.debug("signature verified with key inside signature, attempting certificate validation...");
                certEE=(*i);
            }
        }
        catch (SignatureException& ex) {
            log.debug(ex.what());
        }
    }
    
    if (certEE)
        return validate(certEE,certs,credResolver,criteria);
        
    log.debug("failed to verify signature with embedded certificates");
    return false;
}
