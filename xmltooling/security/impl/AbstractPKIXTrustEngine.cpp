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
 * AbstractPKIXTrustEngine.cpp
 * 
 * A trust engine that uses X.509 trust anchors and CRLs associated with a KeyInfoSource
 * to perform PKIX validation of signatures and certificates.
 */

#include "internal.h"
#include "logging.h"
#include "security/AbstractPKIXTrustEngine.h"
#include "signature/KeyInfo.h"
#include "signature/Signature.h"
#include "security/CredentialCriteria.h"
#include "security/CredentialResolver.h"
#include "security/KeyInfoResolver.h"
#include "security/OpenSSLCryptoX509CRL.h"
#include "security/X509Credential.h"
#include "signature/SignatureValidator.h"
#include "util/NDC.h"

#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;


namespace {
    static int XMLTOOL_DLLLOCAL error_callback(int ok, X509_STORE_CTX* ctx)
    {
        if (!ok)
            Category::getInstance("OpenSSL").error("path validation failure: %s", X509_verify_cert_error_string(ctx->error));
        return ok;
    }

    static bool XMLTOOL_DLLLOCAL validate(
        X509* EE,
        STACK_OF(X509)* untrusted,
        AbstractPKIXTrustEngine::PKIXValidationInfoIterator* pkixInfo,
		bool useCRL,
        bool fullCRLChain,
        const vector<XSECCryptoX509CRL*>* inlineCRLs=nullptr
        )
    {
        Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine");
    
        // First we build a stack of CA certs. These objects are all referenced in place.
        log.debug("supplying PKIX Validation information");
    
        // We need this for CRL support.
        X509_STORE* store=X509_STORE_new();
        if (!store) {
            log_openssl();
            return false;
        }
    
        STACK_OF(X509)* CAstack = sk_X509_new_null();
        
        // This contains the state of the validate operation.
        int count=0;
        X509_STORE_CTX ctx;
        
        const vector<XSECCryptoX509*>& CAcerts = pkixInfo->getTrustAnchors();
        for (vector<XSECCryptoX509*>::const_iterator i=CAcerts.begin(); i!=CAcerts.end(); ++i) {
            if ((*i)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                sk_X509_push(CAstack,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
                ++count;
            }
        }

        log.debug("supplied (%d) CA certificate(s)", count);

        if (useCRL) {
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
            count=0;
            if (inlineCRLs) {
                for (vector<XSECCryptoX509CRL*>::const_iterator j=inlineCRLs->begin(); j!=inlineCRLs->end(); ++j) {
                    if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                        // owned by store
                        X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                        ++count;
                    }
                }
            }
            const vector<XSECCryptoX509CRL*>& crls = pkixInfo->getCRLs();
            for (vector<XSECCryptoX509CRL*>::const_iterator j=crls.begin(); j!=crls.end(); ++j) {
                if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                    // owned by store
                    X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                    ++count;
                }
            }
            log.debug("supplied (%d) CRL(s)", count);
            if (count > 0) {
                X509_STORE_set_flags(store, fullCRLChain ? (X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL) : (X509_V_FLAG_CRL_CHECK));
		    }
		    else {
			    log.warn("CRL checking is enabled, but none were supplied");
                sk_X509_free(CAstack);
                X509_STORE_free(store);
                return false;
		    }
#else
			log.warn("CRL checking is enabled, but OpenSSL version is too old");
            sk_X509_free(CAstack);
            X509_STORE_free(store);
            return false;
#endif
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

    static XMLCh fullCRLChain[] =		UNICODE_LITERAL_12(f,u,l,l,C,R,L,C,h,a,i,n);
	static XMLCh checkRevocation[] =	UNICODE_LITERAL_15(c,h,e,c,k,R,e,v,o,c,a,t,i,o,n);
};

AbstractPKIXTrustEngine::PKIXValidationInfoIterator::PKIXValidationInfoIterator()
{
}

AbstractPKIXTrustEngine::PKIXValidationInfoIterator::~PKIXValidationInfoIterator()
{
}

AbstractPKIXTrustEngine::AbstractPKIXTrustEngine(const xercesc::DOMElement* e)
	: TrustEngine(e),
		m_fullCRLChain(XMLHelper::getAttrBool(e, false, fullCRLChain)),
		m_checkRevocation(XMLHelper::getAttrString(e, nullptr, checkRevocation))
{
    if (m_fullCRLChain) {
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX").warn(
            "fullCRLChain option is deprecated, setting checkRevocation to \"fullChain\""
            );
        m_checkRevocation = "fullChain";
    }
    else if (m_checkRevocation == "fullChain") {
        m_fullCRLChain = true; // in case anything's using this
    }
}

AbstractPKIXTrustEngine::~AbstractPKIXTrustEngine()
{
}

bool AbstractPKIXTrustEngine::checkEntityNames(
    X509* certEE, const CredentialResolver& credResolver, const CredentialCriteria& criteria
    ) const
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX");

    // We resolve to a set of trusted credentials.
    vector<const Credential*> creds;
    credResolver.resolve(creds,&criteria);

    // Build a list of acceptable names.
    set<string> trustednames;
    trustednames.insert(criteria.getPeerName());
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred)
        trustednames.insert((*cred)->getKeyNames().begin(), (*cred)->getKeyNames().end());

    X509_NAME* subject=X509_get_subject_name(certEE);
    if (subject) {
        // One way is a direct match to the subject DN.
        // Seems that the way to do the compare is to write the X509_NAME into a BIO.
        BIO* b = BIO_new(BIO_s_mem());
        BIO* b2 = BIO_new(BIO_s_mem());
        // The flags give us LDAP order instead of X.500, with a comma separator.
        X509_NAME_print_ex(b,subject,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        // The flags give us LDAP order instead of X.500, with a comma plus space separator.
        X509_NAME_print_ex(b2,subject,0,XN_FLAG_RFC2253 + XN_FLAG_SEP_CPLUS_SPC - XN_FLAG_SEP_COMMA_PLUS);
        BIO_flush(b2);

        BUF_MEM* bptr=nullptr;
        BUF_MEM* bptr2=nullptr;
        BIO_get_mem_ptr(b, &bptr);
        BIO_get_mem_ptr(b2, &bptr2);

        if (bptr && bptr->length > 0 && log.isDebugEnabled()) {
            string subjectstr(bptr->data, bptr->length);
            log.debug("certificate subject: %s", subjectstr.c_str());
        }
        
        // Check each keyname.
        for (set<string>::const_iterator n=trustednames.begin(); bptr && bptr2 && n!=trustednames.end(); n++) {
#ifdef HAVE_STRCASECMP
            if ((n->length() == bptr->length && !strncasecmp(n->c_str(), bptr->data, bptr->length)) ||
                (n->length() == bptr2->length && !strncasecmp(n->c_str(), bptr2->data, bptr2->length))) {
#else
            if ((n->length() == bptr->length && !strnicmp(n->c_str(), bptr->data, bptr->length)) ||
                (n->length() == bptr2->length && !strnicmp(n->c_str(), bptr2->data, bptr2->length))) {
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
        STACK_OF(GENERAL_NAME)* altnames=(STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i(certEE, NID_subject_alt_name, nullptr, nullptr);
        if (altnames) {
            int numalts = sk_GENERAL_NAME_num(altnames);
            for (int an=0; an<numalts; an++) {
                const GENERAL_NAME* check = sk_GENERAL_NAME_value(altnames, an);
                if (check->type==GEN_DNS || check->type==GEN_URI) {
                    const char* altptr = (char*)ASN1_STRING_data(check->d.ia5);
                    const int altlen = ASN1_STRING_length(check->d.ia5);
                    for (set<string>::const_iterator n=trustednames.begin(); n!=trustednames.end(); n++) {
#ifdef HAVE_STRCASECMP
                        if ((check->type==GEN_DNS && n->length()==altlen && !strncasecmp(altptr,n->c_str(),altlen))
#else
                        if ((check->type==GEN_DNS && n->length()==altlen && !strnicmp(altptr,n->c_str(),altlen))
#endif
                                || (check->type==GEN_URI && n->length()==altlen && !strncmp(altptr,n->c_str(),altlen))) {
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

        // Fetch the last CN RDN.
        char* peer_CN = nullptr;
        int j,i = -1;
        while ((j=X509_NAME_get_index_by_NID(subject, NID_commonName, i)) >= 0)
            i = j;
        if (i >= 0) {
            ASN1_STRING* tmp = X509_NAME_ENTRY_get_data(X509_NAME_get_entry(subject, i));
            // Copied in from libcurl.
            /* In OpenSSL 0.9.7d and earlier, ASN1_STRING_to_UTF8 fails if the input
               is already UTF-8 encoded. We check for this case and copy the raw
               string manually to avoid the problem. */
            if(tmp && ASN1_STRING_type(tmp) == V_ASN1_UTF8STRING) {
                j = ASN1_STRING_length(tmp);
                if(j >= 0) {
                    peer_CN = (char*)OPENSSL_malloc(j + 1);
                    memcpy(peer_CN, ASN1_STRING_data(tmp), j);
                    peer_CN[j] = '\0';
                }
            }
            else /* not a UTF8 name */ {
                j = ASN1_STRING_to_UTF8(reinterpret_cast<unsigned char**>(&peer_CN), tmp);
            }

            for (set<string>::const_iterator n=trustednames.begin(); n!=trustednames.end(); n++) {
#ifdef HAVE_STRCASECMP
                if (n->length() == j && !strncasecmp(peer_CN, n->c_str(), j)) {
#else
                if (n->length() == j && !strnicmp(peer_CN, n->c_str(), j)) {
#endif
                    log.debug("matched subject CN to a key name (%s)", n->c_str());
                    if(peer_CN)
                        OPENSSL_free(peer_CN);
                    return true;
                }
            }
            if(peer_CN)
                OPENSSL_free(peer_CN);
        }
        else {
            log.warn("no common name in certificate subject");
        }
    }
    else {
        log.error("certificate has no subject?!");
    }
    
    return false;
}

bool AbstractPKIXTrustEngine::validateWithCRLs(
    X509* certEE,
    STACK_OF(X509)* certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria,
    const std::vector<XSECCryptoX509CRL*>* inlineCRLs
    ) const
{
#ifdef _DEBUG
    NDC ndc("validateWithCRLs");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX");

    if (!certEE) {
        log.error("X.509 credential was NULL, unable to perform validation");
        return false;
    }

    if (criteria && criteria->getPeerName() && *(criteria->getPeerName())) {
        log.debug("checking that the certificate name is acceptable");
        if (criteria->getUsage()==Credential::UNSPECIFIED_CREDENTIAL)
            criteria->setUsage(Credential::SIGNING_CREDENTIAL);
        if (!checkEntityNames(certEE,credResolver,*criteria)) {
            log.error("certificate name was not acceptable");
            return false;
        }
    }
    
    log.debug("performing certificate path validation...");

    auto_ptr<PKIXValidationInfoIterator> pkix(getPKIXValidationInfoIterator(credResolver, criteria));
    while (pkix->next()) {
        if (::validate(
				certEE,
				certChain,
				pkix.get(),
				(m_checkRevocation=="entityOnly" || m_checkRevocation=="fullChain"),
				(m_checkRevocation=="fullChain"),
				(m_checkRevocation=="entityOnly" || m_checkRevocation=="fullChain") ? inlineCRLs : nullptr
				)) {
            return true;
        }
    }

    log.debug("failed to validate certificate chain using supplied PKIX information");
    return false;
}

bool AbstractPKIXTrustEngine::validate(
    X509* certEE,
    STACK_OF(X509)* certChain,
    const CredentialResolver& credResolver,
    CredentialCriteria* criteria
    ) const
{
    return validateWithCRLs(certEE,certChain,credResolver,criteria);
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
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX").error("X.509 credential was NULL, unable to perform validation");
        return false;
    }
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX").error("only the OpenSSL XSEC provider is supported");
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
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX");

    const KeyInfoResolver* inlineResolver = m_keyInfoResolver;
    if (!inlineResolver)
        inlineResolver = XMLToolingConfig::getConfig().getKeyInfoResolver();
    if (!inlineResolver) {
        log.error("unable to perform PKIX validation, no KeyInfoResolver available");
        return false;
    }

    // Pull the certificate chain out of the signature.
    X509Credential* x509cred;
    auto_ptr<Credential> cred(inlineResolver->resolve(&sig,X509Credential::RESOLVE_CERTS|X509Credential::RESOLVE_CRLS));
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
    XSECCryptoX509* certEE=nullptr;
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
    
    if (!certEE) {
        log.debug("failed to verify signature with embedded certificates");
        return false;
    }
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        log.error("only the OpenSSL XSEC provider is supported");
        return false;
    }

    STACK_OF(X509)* untrusted=sk_X509_new_null();
    for (vector<XSECCryptoX509*>::const_iterator i=certs.begin(); i!=certs.end(); ++i)
        sk_X509_push(untrusted,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
    const vector<XSECCryptoX509CRL*>& crls = x509cred->getCRLs();
    bool ret = validateWithCRLs(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(), untrusted, credResolver, criteria, &crls);
    sk_X509_free(untrusted);
    return ret;
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
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX");

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
    XSECCryptoX509* certEE=nullptr;
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

    if (!certEE) {
        log.debug("failed to verify signature with embedded certificates");
        return false;
    }
    else if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        log.error("only the OpenSSL XSEC provider is supported");
        return false;
    }

    STACK_OF(X509)* untrusted=sk_X509_new_null();
    for (vector<XSECCryptoX509*>::const_iterator i=certs.begin(); i!=certs.end(); ++i)
        sk_X509_push(untrusted,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
    const vector<XSECCryptoX509CRL*>& crls = x509cred->getCRLs();
    bool ret = validateWithCRLs(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(), untrusted, credResolver, criteria, &crls);
    sk_X509_free(untrusted);
    return ret;
}
