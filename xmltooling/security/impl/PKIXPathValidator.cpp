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
 * PKIXPathValidator.cpp
 * 
 * A path validator based on PKIX support in OpenSSL.
 */

#include "internal.h"
#include "logging.h"
#include "security/OpenSSLPathValidator.h"
#include "security/OpenSSLCryptoX509CRL.h"
#include "security/PKIXPathValidatorParams.h"
#include "security/SecurityHelper.h"
#include "util/NDC.h"
#include "util/PathResolver.h"
#include "util/Threads.h"
#include "util/XMLHelper.h"

#include <memory>
#include <algorithm>
#include <fstream>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;


namespace {
    static int XMLTOOL_DLLLOCAL error_callback(int ok, X509_STORE_CTX* ctx)
    {
        if (!ok) {
            Category::getInstance("OpenSSL").error(
                "path validation failure at depth(%d): %s", ctx->error_depth, X509_verify_cert_error_string(ctx->error)
                );
        }
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

    static time_t XMLTOOL_DLLLOCAL getCRLTime(const ASN1_TIME *a)
    {
        struct tm t;
        memset(&t, 0, sizeof(t));
        // RFC 5280, sections 5.1.2.4 and 5.1.2.5 require thisUpdate and nextUpdate
        // to be encoded as UTCTime until 2049, and RFC 5280 section 4.1.2.5.1
        // further restricts the format to "YYMMDDHHMMSSZ" ("even where the number
        // of seconds is zero").
        // As long as OpenSSL doesn't provide any API to convert ASN1_TIME values
        // time_t, we therefore have to parse it ourselves, unfortunately.
        if (sscanf((const char*)a->data, "%2d%2d%2d%2d%2d%2dZ",
            &t.tm_year, &t.tm_mon, &t.tm_mday,
            &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
            if (t.tm_year <= 50) {
                // RFC 5280, section 4.1.2.5.1
                t.tm_year += 100;
            }
            t.tm_mon--;
#if defined(HAVE_TIMEGM)
            return timegm(&t);
#else
            // Windows, and hopefully most others...?
            return mktime(&t) - timezone;
#endif
        }
        return (time_t)-1;
    }

    static const XMLCh minRefreshDelay[] =      UNICODE_LITERAL_15(m,i,n,R,e,f,r,e,s,h,D,e,l,a,y);
    static const XMLCh minSecondsRemaining[] =  UNICODE_LITERAL_19(m,i,n,S,e,c,o,n,d,s,R,e,m,a,i,n,i,n,g);
    static const XMLCh minPercentRemaining[] =  UNICODE_LITERAL_19(m,i,n,P,e,r,c,e,n,t,R,e,m,a,i,n,i,n,g);
};

namespace xmltooling {

    class XMLTOOL_DLLLOCAL PKIXPathValidator : public OpenSSLPathValidator
    {
    public:
        PKIXPathValidator(const xercesc::DOMElement* e)
            : m_log(Category::getInstance(XMLTOOLING_LOGCAT".PathValidator.PKIX")),
              m_lock(XMLToolingConfig::getConfig().getNamedMutex(XMLTOOLING_LOGCAT".PathValidator.PKIX")),
              m_minRefreshDelay(XMLHelper::getAttrInt(e, 60, minRefreshDelay)),
              m_minSecondsRemaining(XMLHelper::getAttrInt(e, 86400, minSecondsRemaining)),
              m_minPercentRemaining(XMLHelper::getAttrInt(e, 10, minPercentRemaining)) {
        }

        virtual ~PKIXPathValidator() {}

        bool validate(
            XSECCryptoX509* certEE, const vector<XSECCryptoX509*>& certChain, const PathValidatorParams& params
            ) const;
        bool validate(
            X509* certEE, STACK_OF(X509)* certChain, const PathValidatorParams& params
            ) const;

    private:
        XSECCryptoX509CRL* getRemoteCRLs(const char* cdpuri) const;
        bool isFreshCRL(XSECCryptoX509CRL *c, Category* log=nullptr) const;

        Category& m_log;
        Mutex& m_lock;
        time_t m_minRefreshDelay,m_minSecondsRemaining;
        unsigned short m_minPercentRemaining;

        static map<string,time_t> m_crlUpdateMap;
    };

    PathValidator* XMLTOOL_DLLLOCAL PKIXPathValidatorFactory(const xercesc::DOMElement* const & e)
    {
        return new PKIXPathValidator(e);
    }

};

map<string,time_t> PKIXPathValidator::m_crlUpdateMap;

void XMLTOOL_API xmltooling::registerPathValidators()
{
    XMLToolingConfig& conf=XMLToolingConfig::getConfig();
    conf.PathValidatorManager.registerFactory(PKIX_PATHVALIDATOR, PKIXPathValidatorFactory);
}

PathValidator::PathValidator()
{
}

PathValidator::~PathValidator()
{
}

PathValidator::PathValidatorParams::PathValidatorParams()
{
}

PathValidator::PathValidatorParams::~PathValidatorParams()
{
}

PKIXPathValidatorParams::PKIXPathValidatorParams()
{
}

PKIXPathValidatorParams::~PKIXPathValidatorParams()
{
}

OpenSSLPathValidator::OpenSSLPathValidator()
{
}

OpenSSLPathValidator::~OpenSSLPathValidator()
{
}

bool PKIXPathValidator::validate(
    XSECCryptoX509* certEE, const vector<XSECCryptoX509*>& certChain, const PathValidatorParams& params
    ) const
{
    if (certEE->getProviderName()!=DSIGConstants::s_unicodeStrPROVOpenSSL) {
        m_log.error("only the OpenSSL XSEC provider is supported");
        return false;
    }

    STACK_OF(X509)* untrusted=sk_X509_new_null();
    for (vector<XSECCryptoX509*>::const_iterator i=certChain.begin(); i!=certChain.end(); ++i)
        sk_X509_push(untrusted,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());

    bool ret = validate(static_cast<OpenSSLCryptoX509*>(certEE)->getOpenSSLX509(), untrusted, params);
    sk_X509_free(untrusted);
    return ret;
}

bool PKIXPathValidator::validate(X509* EE, STACK_OF(X509)* untrusted, const PathValidatorParams& params) const
{
#ifdef _DEBUG
    NDC ndc("validate");
#endif

    const PKIXPathValidatorParams* pkixParams = dynamic_cast<const PKIXPathValidatorParams*>(&params);
    if (!pkixParams) {
        m_log.error("input parameters were of incorrect type");
        return false;
    }

    // First we build a stack of CA certs. These objects are all referenced in place.
    m_log.debug("supplying PKIX Validation information");

    // We need this for CRL support.
    X509_STORE* store=X509_STORE_new();
    if (!store) {
        log_openssl();
        return false;
    }

    // PKIX policy checking (cf. RFCs 3280/5280 section 6)
    if (pkixParams->isPolicyMappingInhibited() || pkixParams->isAnyPolicyInhibited() || (!pkixParams->getPolicies().empty())) {
#if (OPENSSL_VERSION_NUMBER < 0x00908000L)
        m_log.error("PKIX policy checking option is configured, but OpenSSL version is less than 0.9.8");
        X509_STORE_free(store);
        return false;
#else
        unsigned long pflags = 0;
        X509_VERIFY_PARAM *vpm = X509_VERIFY_PARAM_new();
        if (!vpm) {
            log_openssl();
            X509_STORE_free(store);
            return false;
        }

        // populate the "user-initial-policy-set" input variable
        const set<string>& policies = pkixParams->getPolicies();
        if (!policies.empty()) {
            for (set<string>::const_iterator o=policies.begin(); o!=policies.end(); o++) {
                ASN1_OBJECT *oid = OBJ_txt2obj(o->c_str(), 1);
                if (oid && X509_VERIFY_PARAM_add0_policy(vpm, oid)) {
                    m_log.debug("OID (%s) added to set of acceptable policies", o->c_str());
                }
                else {
                    log_openssl();
                    m_log.error("unable to parse/configure policy OID value (%s)", o->c_str());
                    if (oid)
                        ASN1_OBJECT_free(oid);
                    X509_VERIFY_PARAM_free(vpm);
                    X509_STORE_free(store);
                    return false;
                }
            }
            // when the user has supplied at least one policy OID, he obviously wants to check
            // for an explicit policy ("initial-explicit-policy")
            pflags |= X509_V_FLAG_EXPLICIT_POLICY;
        }

        // "initial-policy-mapping-inhibit" input variable
        if (pkixParams->isPolicyMappingInhibited())
            pflags |= X509_V_FLAG_INHIBIT_MAP;
        // "initial-any-policy-inhibit" input variable
        if (pkixParams->isAnyPolicyInhibited())
            pflags |= X509_V_FLAG_INHIBIT_ANY;

        if (!X509_VERIFY_PARAM_set_flags(vpm, pflags) || !X509_STORE_set1_param(store, vpm)) {
            log_openssl();
            m_log.error("unable to set PKIX policy checking parameters");
            X509_VERIFY_PARAM_free(vpm);
            X509_STORE_free(store);
            return false;
        }

        X509_VERIFY_PARAM_free(vpm);
#endif
    }

    // This contains the state of the validate operation.
    int count=0;
    X509_STORE_CTX ctx;

    // AFAICT, EE and untrusted are passed in but not owned by the ctx.
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
    if (X509_STORE_CTX_init(&ctx,store,EE,untrusted)!=1) {
        log_openssl();
        m_log.error("unable to initialize X509_STORE_CTX");
        X509_STORE_free(store);
        return false;
    }
#else
    X509_STORE_CTX_init(&ctx,store,EE,untrusted);
#endif

    STACK_OF(X509)* CAstack = sk_X509_new_null();
    const vector<XSECCryptoX509*>& CAcerts = pkixParams->getTrustAnchors();
    for (vector<XSECCryptoX509*>::const_iterator i=CAcerts.begin(); i!=CAcerts.end(); ++i) {
        if ((*i)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL) {
            sk_X509_push(CAstack,static_cast<OpenSSLCryptoX509*>(*i)->getOpenSSLX509());
            ++count;
        }
    }
    m_log.debug("supplied (%d) CA certificate(s)", count);

    // Seems to be most efficient to just pass in the CA stack.
    X509_STORE_CTX_trusted_stack(&ctx,CAstack);
    X509_STORE_CTX_set_depth(&ctx,100);    // we check the depth down below
    X509_STORE_CTX_set_verify_cb(&ctx,error_callback);

    // Do a first pass verify. If CRLs aren't used, this is the only pass.
    int ret=X509_verify_cert(&ctx);
    if (ret==1) {
        // Now see if the depth was acceptable by counting the number of intermediates.
        int depth=sk_X509_num(ctx.chain)-2;
        if (pkixParams->getVerificationDepth() < depth) {
            m_log.error(
                "certificate chain was too long (%d intermediates, only %d allowed)",
                (depth==-1) ? 0 : depth,
                pkixParams->getVerificationDepth()
                );
            ret=0;
        }
    }

    if (pkixParams->getRevocationChecking() != PKIXPathValidatorParams::REVOCATION_OFF) {
#if (OPENSSL_VERSION_NUMBER >= 0x00907000L)
        // When we add CRLs, we have to be sure the nextUpdate hasn't passed, because OpenSSL won't accept
        // the CRL in that case. If we end up not adding a CRL for a particular link in the chain, the
        // validation will fail (if the fullChain option was set).
        set<string> crlissuers;
        time_t now = time(nullptr);

        const vector<XSECCryptoX509CRL*>& crls = pkixParams->getCRLs();
        for (vector<XSECCryptoX509CRL*>::const_iterator j=crls.begin(); j!=crls.end(); ++j) {
            if ((*j)->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL &&
                (X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()), &now) > 0)) {
                // owned by store
                X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL()));
                string crlissuer(X509_NAME_to_string(X509_CRL_get_issuer(static_cast<OpenSSLCryptoX509CRL*>(*j)->getOpenSSLX509CRL())));
                if (!crlissuer.empty()) {
                    m_log.debug("added CRL issued by (%s)", crlissuer.c_str());
                    crlissuers.insert(crlissuer);
                }
            }
        }

        for (int i = 0; i < sk_X509_num(untrusted); ++i) {
            X509 *cert = sk_X509_value(untrusted, i);
            string crlissuer(X509_NAME_to_string(X509_get_issuer_name(cert)));
            if (crlissuers.count(crlissuer)) {
               // We already have a CRL for this cert, so skip CRLDP processing for this one.
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
                    if (gen->type == GEN_URI && (!strncasecmp((const char*)gen->d.ia5->data, "http:", 5))) {
#else
                    if (gen->type == GEN_URI && (!strnicmp((const char*)gen->d.ia5->data, "http:", 5))) {
#endif
                        const char* cdpuri = (const char*)gen->d.ia5->data;
                        auto_ptr<XSECCryptoX509CRL> crl(getRemoteCRLs(cdpuri));
                        if (crl.get() && crl->getProviderName()==DSIGConstants::s_unicodeStrPROVOpenSSL &&
                            (isFreshCRL(crl.get()) || (ii == sk_DIST_POINT_num(dps)-1 && iii == sk_GENERAL_NAME_num(dp->distpoint->name.fullname)-1))) {
                            // owned by store
                            X509_STORE_add_crl(store, X509_CRL_dup(static_cast<OpenSSLCryptoX509CRL*>(crl.get())->getOpenSSLX509CRL()));
                            m_log.debug("added CRL issued by (%s)", crlissuer.c_str());
                            crlissuers.insert(crlissuer);
                            foundUsableCDP = true;
                        }
                    }
                }
            }
            sk_DIST_POINT_free(dps);
        }

        // Do a second pass verify with CRLs in place.
        if (pkixParams->getRevocationChecking() == PKIXPathValidatorParams::REVOCATION_FULLCHAIN)
            X509_STORE_CTX_set_flags(&ctx, X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);
        else
            X509_STORE_CTX_set_flags(&ctx, X509_V_FLAG_CRL_CHECK);
        ret=X509_verify_cert(&ctx);
#else
        m_log.warn("CRL checking is enabled, but OpenSSL version is too old");
        ret = 0;
#endif
    }

    if (ret == 1) {
        m_log.debug("successfully validated certificate chain");
    }
#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
    else if (X509_STORE_CTX_get_error(&ctx) == X509_V_ERR_NO_EXPLICIT_POLICY && !pkixParams->isPolicyMappingInhibited()) {
        m_log.warn("policy mapping requires OpenSSL 1.0.0 or later");
    }
#endif

    // Clean up...
    X509_STORE_CTX_cleanup(&ctx);
    X509_STORE_free(store);
    sk_X509_free(CAstack);

    return (ret == 1);
}

XSECCryptoX509CRL* PKIXPathValidator::getRemoteCRLs(const char* cdpuri) const
{
    // This is a filesystem-based CRL cache using a shared lock across all instances
    // of this class.

    // The filenames for the CRL cache are based on a hash of the CRL location.
    string cdpfile = SecurityHelper::doHash("SHA1", cdpuri, strlen(cdpuri)) + ".crl";
    XMLToolingConfig::getConfig().getPathResolver()->resolve(cdpfile, PathResolver::XMLTOOLING_RUN_FILE);
    string cdpstaging = cdpfile + ".tmp";

    time_t now = time(nullptr);
    vector<XSECCryptoX509CRL*> crls;

    try {
        // While holding the lock, check for a cached copy of the CRL, and remove "expired" ones.
        Lock glock(m_lock);
#ifdef WIN32
        struct _stat stat_buf;
        if (_stat(cdpfile.c_str(), &stat_buf) == 0) {
#else
        struct stat stat_buf;
        if (stat(cdpfile.c_str(), &stat_buf) == 0) {
#endif
            SecurityHelper::loadCRLsFromFile(crls, cdpfile.c_str());
            if (crls.empty() || crls.front()->getProviderName() != DSIGConstants::s_unicodeStrPROVOpenSSL ||
                X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), &now) < 0) {
                for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                crls.clear();
                remove(cdpfile.c_str());    // may as well delete the local copy
                m_crlUpdateMap.erase(cdpuri);
                m_log.info("deleting cached CRL from %s with nextUpdate field in the past", cdpuri);
            }
        }
    }
    catch (exception& ex) {
        m_log.error("exception loading cached copy of CRL from %s: %s", cdpuri, ex.what());
    }

    if (crls.empty() || !isFreshCRL(crls.front(), &m_log)) {
        bool updateTimestamp = true;
        try {
            // If we get here, the cached copy didn't exist yet, or it's time to refresh.
            // To limit the rate of unsuccessful attempts when a CRLDP is unreachable,
            // we remember the timestamp of the last attempt (both successful/unsuccessful).
            time_t ts = 0;
            m_lock.lock();
            map<string,time_t>::const_iterator tsit = m_crlUpdateMap.find(cdpuri);
            if (tsit != m_crlUpdateMap.end())
                ts = tsit->second;
            m_lock.unlock();

            if (difftime(now, ts) > m_minRefreshDelay) {
                SOAPTransport::Address addr("AbstractPKIXTrustEngine", cdpuri, cdpuri);
                string scheme(addr.m_endpoint, strchr(addr.m_endpoint,':') - addr.m_endpoint);
                auto_ptr<SOAPTransport> soap(XMLToolingConfig::getConfig().SOAPTransportManager.newPlugin(scheme.c_str(), addr));
                soap->send();
                istream& msg = soap->receive();
                Lock glock(m_lock);
                ofstream out(cdpstaging.c_str(), fstream::trunc|fstream::binary);
                out << msg.rdbuf();
                out.close();
                SecurityHelper::loadCRLsFromFile(crls, cdpstaging.c_str());
                if (crls.empty() || crls.front()->getProviderName() != DSIGConstants::s_unicodeStrPROVOpenSSL ||
                    X509_cmp_time(X509_CRL_get_nextUpdate(static_cast<OpenSSLCryptoX509CRL*>(crls.front())->getOpenSSLX509CRL()), &now) < 0) {
                    // The "new" CRL wasn't usable, so get rid of it.
                    for_each(crls.begin(), crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
                    crls.clear();
                    remove(cdpstaging.c_str());
                    m_log.error("ignoring CRL retrieved from %s with nextUpdate field in the past", cdpuri);
                }
                else {
                    // "Commit" the new CRL. Note that we might add a CRL which doesn't pass
                    // isFreshCRL, but that's preferrable over adding none at all.
                    m_log.info("CRL refreshed from %s", cdpuri);
                    remove(cdpfile.c_str());
                    if (rename(cdpstaging.c_str(), cdpfile.c_str()) != 0)
                        m_log.error("unable to rename CRL staging file");
                }
            }
            else {
                updateTimestamp = false;    // don't update if we're within the backoff window
            }
        }
        catch (exception& ex) {
            m_log.error("exception downloading/caching CRL from %s: %s", cdpuri, ex.what());
        }

        if (updateTimestamp) {
            Lock glock(m_lock);
            m_crlUpdateMap[cdpuri] = now;
        }
    }

    if (crls.empty())
        return nullptr;
    for_each(crls.begin() + 1, crls.end(), xmltooling::cleanup<XSECCryptoX509CRL>());
    return crls.front();
}

bool PKIXPathValidator::isFreshCRL(XSECCryptoX509CRL *c, Category* log) const
{
    if (c) {
        const X509_CRL* crl = static_cast<OpenSSLCryptoX509CRL*>(c)->getOpenSSLX509CRL();
        time_t thisUpdate = getCRLTime(X509_CRL_get_lastUpdate(crl));
        time_t nextUpdate = getCRLTime(X509_CRL_get_nextUpdate(crl));
        time_t now = time(nullptr);

        if (thisUpdate < 0 || nextUpdate < 0) {
            // we failed to parse at least one of the fields (they were not encoded
            // as required by RFC 5280, actually)
            time_t exp = now + m_minSecondsRemaining;
            if (log) {
                log->warn("isFreshCRL (issuer '%s'): improperly encoded thisUpdate or nextUpdate field - falling back to simple time comparison",
                          (X509_NAME_to_string(X509_CRL_get_issuer(crl))).c_str());
            }
            return (X509_cmp_time(X509_CRL_get_nextUpdate(crl), &exp) > 0) ? true : false;
        }
        else {
            if (log && log->isDebugEnabled()) {
                log->debug("isFreshCRL (issuer '%s'): %.0f seconds until nextUpdate (%3.2f%% elapsed since thisUpdate)",
                          (X509_NAME_to_string(X509_CRL_get_issuer(crl))).c_str(),
                          difftime(nextUpdate, now), (difftime(now, thisUpdate) * 100) / difftime(nextUpdate, thisUpdate));
            }

            // consider it recent enough if there are at least MIN_SECS_REMAINING
            // to the nextUpdate, and at least MIN_PERCENT_REMAINING of its
            // overall "validity" are remaining to the nextUpdate
            return (now + m_minSecondsRemaining < nextUpdate) &&
                    ((difftime(nextUpdate, now) * 100) / difftime(nextUpdate, thisUpdate) > m_minPercentRemaining);
        }
    }
    return false;
}
