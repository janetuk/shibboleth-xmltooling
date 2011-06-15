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
 * @file xmltooling/security/BasicX509Credential.h
 * 
 * Wraps an X.509-based Credential by storing key/cert objects inside. 
 */

#if !defined(__xmltooling_basicx509cred_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_basicx509cred_h__

#include <xmltooling/security/X509Credential.h>

#include <set>
#include <vector>
#include <string>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
};

namespace xmltooling {

    /**
     * Wraps an X.509-based Credential by storing key/cert objects inside.
     */
    class XMLTOOL_API BasicX509Credential : public virtual X509Credential
    {
    protected:
        /**
         * Constructor.
         * 
         * @param ownCerts  true iff any certificates subsequently stored should be freed by destructor
         */
        BasicX509Credential(bool ownCerts);

        /**
         * Constructor.
         * 
         * @param key   key pair or secret key
         * @param certs array of X.509 certificates, the first entry being the entity certificate
         * @param crl   optional CRL
         */
        BasicX509Credential(XSECCryptoKey* key, const std::vector<XSECCryptoX509*>& certs, XSECCryptoX509CRL* crl=nullptr);

        /**
         * Constructor.
         * 
         * @param key   key pair or secret key
         * @param certs array of X.509 certificates, the first entry being the entity certificate
         * @param crls  array of X.509 CRLs
         */
        BasicX509Credential(XSECCryptoKey* key, const std::vector<XSECCryptoX509*>& certs, const std::vector<XSECCryptoX509CRL*>& crls);

        /** The private/secret key/keypair. */
        XSECCryptoKey* m_key;

        /** Key names (derived from credential, KeyInfo, or both). */
        std::set<std::string> m_keyNames;

        /** Subject DN. */
        std::string m_subjectName;

        /** Issuer DN. */
        std::string m_issuerName;

        /** Serial number. */
        std::string m_serial;

        /** The X.509 certificate chain. */
        std::vector<XSECCryptoX509*> m_xseccerts;

        /** Indicates whether to destroy certificates. */
        bool m_ownCerts;

        /** The X.509 CRLs. */
        std::vector<XSECCryptoX509CRL*> m_crls;

        /** The KeyInfo object representing the information. */
        xmlsignature::KeyInfo* m_keyInfo;

        /** The KeyInfo object representing the information in compact form. */
        xmlsignature::KeyInfo* m_compactKeyInfo;

        /**
         * Initializes (or reinitializes) a ds:KeyInfo to represent the Credential.
         *
         * @param types the kinds of KeyInfo content to include 
         */
        void initKeyInfo(unsigned int types=0);

    public:
        virtual ~BasicX509Credential();
        
        // Virtual function overrides.
        unsigned int getUsage() const;
        const char* getAlgorithm() const;
        unsigned int getKeySize() const;
        XSECCryptoKey* getPrivateKey() const;
        XSECCryptoKey* getPublicKey() const;
        const std::set<std::string>& getKeyNames() const;
        xmlsignature::KeyInfo* getKeyInfo(bool compact=false) const;
        const std::vector<XSECCryptoX509*>& getEntityCertificateChain() const;
        XSECCryptoX509CRL* getCRL() const;
        const std::vector<XSECCryptoX509CRL*>& getCRLs() const;
        const char* getSubjectName() const;
        const char* getIssuerName() const;
        const char* getSerialNumber() const;
        void extract();
    };
};

#endif /* __xmltooling_basicx509cred_h__ */
