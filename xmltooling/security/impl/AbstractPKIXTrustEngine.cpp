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
#include "security/SecurityHelper.h"
#include "security/X509Credential.h"
#include "signature/SignatureValidator.h"
#include "util/NDC.h"
#include "util/PathResolver.h"

#include <fstream>
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

    static string XMLTOOL_DLLLOCAL X509_NAME_to_string(X509_NAME* n)
    {
        string s;
        BIO* b = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(b,n,0,XN_FLAG_RFC2253);
        BIO_flush(b);
        BUF_MEM* bptr=nullptr;
        BIO_get_mem_ptr(b, &bptr);
        if (bptr && bptr->length > 0) {
            s.append(bptr->data, bptr->length);
        }
        BIO_free(b);
        return s;
    }

    static void XMLTOOL_DLLLOCAL getRemoteCRLs(vector<XSECCryptoX509CRL*>& crls, const char* cdpuri, Category& log) {
        // This is a temporary CRL cache implementation to avoid breaking binary compatibility
        // for the library. Caching can't rely on any member objects within the TrustEngine,
        // including locks, so we're using the global library lock for the time being.
        // All other state is kept in the file system.

        // The filenames for the CRL cache are based on a hash of the CRL location.
        string cdpfile = SecurityHelper::doHash("SHA1", cdpuri, strlen(cdpuri)) + ".crl";
        XMLToolingConfig::getConfig().getPathResolver()->resolve(cdpfile, PathResolver::XMLTOOLING_RUN_FILE);
        string cdpstaging = cdpfile + ".bak";
        string counterfile = cdpfile + ".cnt";

        // Need to move this to a configurable parameter once we can break binary compatibility.
        // Ideally this would be based on a percentage of the original CRL window, but OpenSSL
        // doesn't provide much in the way of ASN1_TIME parsing functions. It does support adding
        // a fixed time value and comparing against known times.
        #define MIN_SECS_REMAINING 86400;
        long counter = 0;
        try {
            // While holding the lock, check for a cached copy of the CRL, and check its validity.
            Locker glock(&XMLToolingConfig::getConfig());
#ifdef WIN32
            struct _stat stat_buf;
            if (_stat(cdpfile.c_str(), &stat_buf) == 0) {
#else
            struct stat stat_buf;
            if (stat(cdpfile.c_str(), &stat_buf) == 0) {
#endif
                SecurityHelper::loadCRLsFromFile(crls, cdpfile.c_str());
                if (crls.empty() || crls.front()->getProviderName() != DSIGConstants::s_unicodeStrPROVOpenSSL ||
                    X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), nullptr) < 0) {
                    for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                    crls.clear();
                    remove(cdpfile.c_str());    // may as well delete the local copy
                    remove(counterfile.c_str());
                    log.info("cached CRL(s) from (%s) have expired", cdpuri);
                }
                else {
                    // Look for a file containing the allowable time remaining on the CRL.
                    // We store this counter in the file system because of the binary compatibility issue.
                    try {
                        ifstream countersrc(counterfile.c_str());
                        if (countersrc)
                            countersrc >> counter;
                    }
                    catch (exception&) {
                        counter = 0;
                    }
                    if (counter == 0)
                        counter = MIN_SECS_REMAINING;

                    // See if the time left is under the counter threshold.
                    time_t exp = time(nullptr) + counter;
                    if (X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), &exp) < 0) {
                        for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                        crls.clear();
                        log.info("cached CRL(s) from (%s) will expire within %ld seconds, attempting to update them", cdpuri, counter);
                    }
                }
            }
        }
        catch (exception& ex) {
            log.error("exception loading cached copy of CRL at (%s): %s", cdpuri, ex.what());
        }

        if (crls.empty()) {
            try {
                // If we get here, the cached copy didn't exist yet, or it's time to refresh.
                SOAPTransport::Address addr("AbstractPKIXTrustEngine", cdpuri, cdpuri);
                string scheme(addr.m_endpoint, strchr(addr.m_endpoint,':') - addr.m_endpoint);
                auto_ptr<SOAPTransport> soap(XMLToolingConfig::getConfig().SOAPTransportManager.newPlugin(scheme.c_str(), addr));
                soap->send();
                istream& msg = soap->receive();
                Locker glock(&XMLToolingConfig::getConfig());
                ofstream out(cdpstaging.c_str(), fstream::trunc|fstream::binary);
                out << msg.rdbuf();
                out.close();
                SecurityHelper::loadCRLsFromFile(crls, cdpstaging.c_str());
                if (crls.empty() || crls.front()->getProviderName() != DSIGConstants::s_unicodeStrPROVOpenSSL ||
                    X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), nullptr) < 0) {
                    // The "new" CRLs weren't usable, so get rid of them.
                    for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                    crls.clear();
                    remove(cdpstaging.c_str());
                    log.error("updated CRL(s) from (%s) have already expired", cdpuri);

                    // If counter isn't 0, then we were attempting an update of still-valid CRLs, so reload the old ones
                    // and cut the counter in half for next time.
                    if (counter > 0) {
                        SecurityHelper::loadCRLsFromFile(crls, cdpfile.c_str());
                        ofstream countersink(counterfile.c_str(), fstream::trunc);
                        counter /= 2;
                        countersink << counter;
                        log.info("failed CRL update attempt, reducing threshold to %ld seconds", counter);
                    }
                }
                else {
                    // If counter isn't zero, we reloaded; see if the new CRLs are "more" valid than before.
                    if (counter > 0) {
                        time_t exp = time(nullptr) + counter;
                        if (X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), &exp) < 0) {
                            // Still invalid past the acceptable interval, so they're the same as what we had.
                            // Remove the extra copy, and cut the counter in half for next time.
                            remove(cdpstaging.c_str());
                            ofstream countersink(counterfile.c_str(), fstream::trunc);
                            counter /= 2;
                            countersink << counter;
                            log.info("remote CRL(s) unchanged, reducing threshold to %ld seconds", counter);
                        }
                        else {
                            counter = 0;
                            log.info("remote CRL(s) updated");
                        }
                    }

                    if (counter == 0) {
                        // "Commit" the new CRLs.
                        remove(cdpfile.c_str());
                        remove(counterfile.c_str());
                        if (rename(cdpstaging.c_str(), cdpfile.c_str()) != 0)
                            log.error("unable to rename CRL staging file");
                    }
                }
            }
            catch (exception& ex) {
                log.error("exception downloading/caching CRL at (%s): %s", cdpuri, ex.what());
            }
        }
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
    
        // This contains the state of the validate operation.
        int count=0;
        X509_STORE_CTX ctx;

        // AFAICT, EE and untrusted are passed in but not owned by the ctx.
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
        if (X509_STORE_CTX_init(&ctx,store,EE,untrusted)!=1) {
            log_openssl();
            log.error("unable to initialize X509_STORE_CTX");
            X509_STORE_free(store);
            return false;
        }
#else
        X509_STORE_CTX_init(&ctx,store,EE,untrusted);
#endif

        STACK_OF(X509)* CAstack = sk_X509_new_null();
        const vector<XSECCryptoX509*>& CAcerts = pkixInfo->getTrustAnchors();
        for (vector<XSECCryptoX509*>::const_iterator i=CAcerts.begin(); i!=CAcerts.end(); ++i) {
            if ((*i)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
                sk_X509_push(CAstack,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
                ++count;
            }
        }
        log.debug("supplied (%d) CA certificate(s)", count);

        // Seems to be most efficient to just pass in the CA stack.
        X509_STORE_CTX_trusted_stack(&ctx,CAstack);
        X509_STORE_CTX_set_depth(&ctx,100);    // we check the depth down below
        X509_STORE_CTX_set_verify_cb(&ctx,error_callback);

        // Do a first pass verify. If CRLs aren't used, this is the only pass.
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

        if (useCRL) {
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
            // When we add CRLs, we have to be sure the nextUpdate hasn't passed, because OpenSSL won't accept
            // the CRL in that case. If we end up not adding a CRL for a particular link in the chain, the
            // validation will fail (if the fullChain option was set).
            set<string> crlissuers;
            if (inlineCRLs) {
                for (vector<XSECCryptoX509CRL*>::const_iterator j=inlineCRLs->begin(); j!=inlineCRLs->end(); ++j) {
                    if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL &&
                        (X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()), nullptr)==1)) {
                        // owned by store
                        X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                        string crlissuer(X509_NAME_to_string(X509_CRL_get_issuer(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL())));
                        if (!crlissuer.empty()) {
                            log.debug("added CRL issued by (%s)", crlissuer.c_str());
                            crlissuers.insert(crlissuer);
                        }
                    }
                }
            }
            const vector<XSECCryptoX509CRL*>& crls = pkixInfo->getCRLs();
            for (vector<XSECCryptoX509CRL*>::const_iterator j=crls.begin(); j!=crls.end(); ++j) {
                if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL &&
                    (X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()), nullptr)==1)) {
                    // owned by store
                    X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                    string crlissuer(X509_NAME_to_string(X509_CRL_get_issuer(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL())));
                    if (!crlissuer.empty()) {
                        log.debug("added CRL issued by (%s)", crlissuer.c_str());
                        crlissuers.insert(crlissuer);
                    }
                }
            }

            for (int i = 0; i < sk_X509_num(untrusted); ++i) {
                X509 *cert = sk_X509_value(untrusted, i);
                string crlissuer(X509_NAME_to_string(X509_get_issuer_name(cert)));
                if (crlissuers.count(crlissuer)) {
                   // We already have a CRL for this cert, so skip it.
                   continue;
                }

                bool foundUsableCDP = false;
                STACK_OF(DIST_POINT)* dps = (STACK_OF(DIST_POINT)*)X509_get_ext_d2i(cert, NID_crl_distribution_points, nullptr, nullptr);
                for (int ii = 0; !foundUsableCDP && ii < sk_DIST_POINT_num(dps); ++ii) {
                    DIST_POINT* dp = sk_DIST_POINT_value(dps, ii);
                    if (!dp->distpoint || dp->distpoint->type != 0)
                        continue;
                    for (int iii = 0; !foundUsableCDP && iii < sk_GENERAL_NAME_num(dp->distpoint->name.fullname); ++iii) {
                        GENERAL_NAME* gen = sk_GENERAL_NAME_value(dp->distpoint->name.fullname, iii);
                        // Only consider HTTP URIs, and stop after the first one we find.
#ifdef HAVE_STRCASECMP
                        if (gen->type == GEN_URI && (!strncasecmp((const char*)gen->d.ia5->data, "http:", 5) ||
                                !strncasecmp((const char*)gen->d.ia5->data, "https:", 6))) {
#else
                        if (gen->type == GEN_URI && (!strnicmp((const char*)gen->d.ia5->data, "http:", 5) ||
                                !strnicmp((const char*)gen->d.ia5->data, "https:", 6))) {
#endif
                            const char* cdpuri = (const char*)gen->d.ia5->data;
                            vector<XSECCryptoX509CRL*> crls;
                            getRemoteCRLs(crls, cdpuri, log);
                            for (vector<XSECCryptoX509CRL*>::const_iterator j = crls.begin(); j != crls.end(); ++j) {
                                if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL &&
                                    X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()), nullptr) > 0) {
                                    // owned by store
                                    X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                                    log.debug("added CRL issued by (%s)", crlissuer.c_str());
                                    crlissuers.insert(crlissuer);
                                    foundUsableCDP = true;
                                }
                            }
                            for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                        }
                    }
                }
                sk_DIST_POINT_free(dps);
            }

            if (count > 0) {
                X509_STORE_set_flags(store, fullCRLChain ? (X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL) : (X509_V_FLAG_CRL_CHECK));
		    }
		    else {
			    log.warn("CRL checking is enabled, but none were supplied");
                X509_STORE_CTX_cleanup(&ctx);
                X509_STORE_free(store);
                sk_X509_free(CAstack);
                return false;
		    }
#else
			log.warn("CRL checking is enabled, but OpenSSL version is too old");
            X509_STORE_CTX_cleanup(&ctx);
            X509_STORE_free(store);
            sk_X509_free(CAstack);
            return false;
#endif
            // Do a second pass verify with CRLs in place.
            ret=X509_verify_cert(&ctx);
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
