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
#include "security/OpenSSLPathValidator.h"
#include "security/PKIXPathValidatorParams.h"
#include "security/SecurityHelper.h"
#include "security/X509Credential.h"
#include "signature/SignatureValidator.h"
#include "util/NDC.h"
#include "util/PathResolver.h"

#include <fstream>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>

using namespace xmlsignature;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;


namespace xmltooling {
    // Adapter between TrustEngine and PathValidator
    class XMLTOOL_DLLLOCAL PKIXParams : public PKIXPathValidatorParams
    {
        const AbstractPKIXTrustEngine& m_trust;
        const AbstractPKIXTrustEngine::PKIXValidationInfoIterator& m_pkixInfo;
        vector<XSECCryptoX509CRL*> m_crls;
    public:
        PKIXParams(
            const AbstractPKIXTrustEngine& t,
            const AbstractPKIXTrustEngine::PKIXValidationInfoIterator& pkixInfo,
            const vector<XSECCryptoX509CRL*>* inlineCRLs
            ) : m_trust(t), m_pkixInfo(pkixInfo) {
            if (inlineCRLs && !inlineCRLs->empty()) {
                m_crls = *inlineCRLs;
                m_crls.insert(m_crls.end(), pkixInfo.getCRLs().begin(), pkixInfo.getCRLs().end());
            }
        }

        virtual ~PKIXParams() {}

        int getVerificationDepth() const {
            return m_pkixInfo.getVerificationDepth();
        }
        bool isAnyPolicyInhibited() const {
            return m_trust.m_anyPolicyInhibit;
        }
        bool isPolicyMappingInhibited() const {
            return m_trust.m_policyMappingInhibit;
        }
        const set<string>& getPolicies() const {
            return m_trust.m_policyOIDs;
        }
        const vector<XSECCryptoX509*>& getTrustAnchors() const {
            return m_pkixInfo.getTrustAnchors();
        }
        PKIXPathValidatorParams::revocation_t getRevocationChecking() const {
            if (m_trust.m_checkRevocation.empty() || m_trust.m_checkRevocation == "off")
                return PKIXPathValidatorParams::REVOCATION_OFF;
            else if (m_trust.m_checkRevocation == "entityOnly")
                return PKIXPathValidatorParams::REVOCATION_ENTITYONLY;
            else if (m_trust.m_checkRevocation == "fullChain")
                return PKIXPathValidatorParams::REVOCATION_FULLCHAIN;
            return PKIXPathValidatorParams::REVOCATION_OFF;
        }
        const vector<XSECCryptoX509CRL*>& getCRLs() const {
            return m_crls.empty() ? m_pkixInfo.getCRLs() : m_crls;
        }
    };


    static XMLCh fullCRLChain[] =		    UNICODE_LITERAL_12(f,u,l,l,C,R,L,C,h,a,i,n);
    static XMLCh checkRevocation[] =	    UNICODE_LITERAL_15(c,h,e,c,k,R,e,v,o,c,a,t,i,o,n);
    static XMLCh policyMappingInhibit[] =   UNICODE_LITERAL_20(p,o,l,i,c,y,M,a,p,p,i,n,g,I,n,h,i,b,i,t);
    static XMLCh anyPolicyInhibit[] =	    UNICODE_LITERAL_16(a,n,y,P,o,l,i,c,y,I,n,h,i,b,i,t);
    static XMLCh _PathValidator[] =         UNICODE_LITERAL_13(P,a,t,h,V,a,l,i,d,a,t,o,r);
    static XMLCh PolicyOID[] =			    UNICODE_LITERAL_9(P,o,l,i,c,y,O,I,D);
    static XMLCh TrustedName[] =		    UNICODE_LITERAL_11(T,r,u,s,t,e,d,N,a,m,e);
    static XMLCh type[] =                   UNICODE_LITERAL_4(t,y,p,e);
};

AbstractPKIXTrustEngine::PKIXValidationInfoIterator::PKIXValidationInfoIterator()
{
}

AbstractPKIXTrustEngine::PKIXValidationInfoIterator::~PKIXValidationInfoIterator()
{
}

AbstractPKIXTrustEngine::AbstractPKIXTrustEngine(const xercesc::DOMElement* e)
	: TrustEngine(e),
		m_checkRevocation(XMLHelper::getAttrString(e, nullptr, checkRevocation)),
		m_fullCRLChain(XMLHelper::getAttrBool(e, false, fullCRLChain)),
		m_policyMappingInhibit(XMLHelper::getAttrBool(e, false, policyMappingInhibit)),
		m_anyPolicyInhibit(XMLHelper::getAttrBool(e, false, anyPolicyInhibit))
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

    xercesc::DOMElement* c = XMLHelper::getFirstChildElement(e);
    while (c) {
        if (c->hasChildNodes()) {
            auto_ptr_char v(c->getTextContent());
            if (v.get() && *v.get()) {
                if (XMLString::equals(c->getLocalName(), PolicyOID))
                    m_policyOIDs.insert(v.get());
                else if (XMLString::equals(c->getLocalName(), TrustedName))
                    m_trustedNames.insert(v.get());
            }
        }
        else if (XMLString::equals(c->getLocalName(), _PathValidator)) {
            try {
                string t = XMLHelper::getAttrString(c, nullptr, type);
                if (!t.empty()) {
                    Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX").info(
                        "building PathValidator of type %s", t.c_str()
                        );
                    PathValidator* pv = XMLToolingConfig::getConfig().PathValidatorManager.newPlugin(t.c_str(), c);
                    OpenSSLPathValidator* ospv = dynamic_cast<OpenSSLPathValidator*>(pv);
                    if (!ospv) {
                        delete pv;
                        throw XMLSecurityException("PathValidator doesn't support OpenSSL interface.");
                    }
                    m_pathValidators.push_back(ospv);
                }
            }
            catch (exception& ex) {
                Category::getInstance(XMLTOOLING_LOGCAT".TrustEngine.PKIX").error(
                    "error building PathValidator: %s", ex.what()
                    );
            }
        }
        c = XMLHelper::getNextSiblingElement(c);
    }

    if (m_pathValidators.empty()) {
        m_pathValidators.push_back(
            dynamic_cast<OpenSSLPathValidator*>(
                XMLToolingConfig::getConfig().PathValidatorManager.newPlugin(PKIX_PATHVALIDATOR, e)
                )
            );
    }
}

AbstractPKIXTrustEngine::~AbstractPKIXTrustEngine()
{
    for_each(m_pathValidators.begin(), m_pathValidators.end(), xmltooling::cleanup<PathValidator>());
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
    set<string> trustednames = m_trustedNames;
    if (log.isDebugEnabled()) {
        for (set<string>::const_iterator n=m_trustedNames.begin(); n!=m_trustedNames.end(); n++) {
            log.debug("adding to list of trusted names (%s)", n->c_str());
        }
    }
    if (criteria.getPeerName()) {
        trustednames.insert(criteria.getPeerName());
        log.debug("adding to list of trusted names (%s)", criteria.getPeerName());
    }
    for (vector<const Credential*>::const_iterator cred = creds.begin(); cred!=creds.end(); ++cred) {
        trustednames.insert((*cred)->getKeyNames().begin(), (*cred)->getKeyNames().end());
        if (log.isDebugEnabled()) {
            for (set<string>::const_iterator n=(*cred)->getKeyNames().begin(); n!=(*cred)->getKeyNames().end(); n++) {
                log.debug("adding to list of trusted names (%s)", n->c_str());
            }
        }
    }

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
    const vector<XSECCryptoX509CRL*>* inlineCRLs
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
        if (criteria && criteria->getUsage()==Credential::UNSPECIFIED_CREDENTIAL)
            criteria->setUsage(Credential::SIGNING_CREDENTIAL);
        if (!checkEntityNames(certEE,credResolver,*criteria)) {
            log.error("certificate name was not acceptable");
            return false;
        }
    }
    else if (!m_trustedNames.empty()) {
        log.debug("checking that the certificate name is acceptable");
        CredentialCriteria cc;
        cc.setUsage(Credential::SIGNING_CREDENTIAL);
        if (!checkEntityNames(certEE,credResolver,cc)) {
            log.error("certificate name was not acceptable");
            return false;
        }
    }
    
    log.debug("performing certificate path validation...");

    auto_ptr<PKIXValidationInfoIterator> pkix(getPKIXValidationInfoIterator(credResolver, criteria));
    while (pkix->next()) {
        PKIXParams params(*this, *pkix.get(), inlineCRLs);
        for (vector<OpenSSLPathValidator*>::const_iterator v = m_pathValidators.begin(); v != m_pathValidators.end(); ++v) {
            if ((*v)->validate(certEE, certChain, params)) {
                return true;
            }
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
