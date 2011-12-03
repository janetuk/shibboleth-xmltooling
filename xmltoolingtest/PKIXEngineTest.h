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

#include "XMLObjectBaseTestCase.h"

#include <xmltooling/security/ChainingTrustEngine.h>
#include <xmltooling/security/CredentialResolver.h>
#include <xmltooling/security/SecurityHelper.h>

#include <fstream>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

class PKIXEngineTest : public CxxTest::TestSuite {

    X509TrustEngine* buildTrustEngine(const char* filename) {
        string config = data_path + "x509/" + filename + ".xml";
        ifstream in(config.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        XercesJanitor<DOMDocument> janitor(doc);
        return dynamic_cast<X509TrustEngine*>(
            XMLToolingConfig::getConfig().TrustEngineManager.newPlugin(
                STATIC_PKIX_TRUSTENGINE, doc->getDocumentElement()
                )
            );
    }

    CredentialResolver* m_dummy;
    ChainingTrustEngine* m_chain;
    XSECCryptoX509* m_ee;   // end entity
    XSECCryptoX509* m_int1; // any policy
    XSECCryptoX509* m_int2; // explicit policy
    XSECCryptoX509* m_int3; // policy mapping

public:
    void setUp() {
        m_dummy = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(DUMMY_CREDENTIAL_RESOLVER, nullptr);
        m_chain = dynamic_cast<ChainingTrustEngine*>(XMLToolingConfig::getConfig().TrustEngineManager.newPlugin(CHAINING_TRUSTENGINE, nullptr));

        m_ee = m_int1 = m_int2 = m_int3 = nullptr;
        vector<XSECCryptoX509*> certs;
        string pathname = data_path + "x509/mdt-signer.crt.pem";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        pathname = data_path + "x509/mdt-ica.1.crt.pem";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        pathname = data_path + "x509/mdt-ica.2.crt.pem";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        pathname = data_path + "x509/mdt-ica.3.crt.pem";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        m_ee = certs[0];
        m_int1 = certs[1];
        m_int2 = certs[2];
        m_int3 = certs[3];
    }

    void tearDown() {
        delete m_chain;
        delete m_dummy;
        delete m_ee;
        delete m_int1;
        delete m_int2;
        delete m_int3;
    }


    void testAnyPolicy() {
        TrustEngine* trust = buildTrustEngine("AnyPolicy");
        m_chain->addTrustEngine(trust);

        vector<XSECCryptoX509*> untrusted(1, m_int1);
        TSM_ASSERT("PKIX validation failed", m_chain->validate(m_ee, untrusted, *m_dummy));

        TSM_ASSERT_EQUALS("Trust engine removal failed", m_chain->removeTrustEngine(trust), trust);
        delete trust;
    }

    void testExplicitPolicy() {
        TrustEngine* trust = buildTrustEngine("ExplicitPolicy");
        m_chain->addTrustEngine(trust);

        vector<XSECCryptoX509*> untrusted(1, m_int1);
        TSM_ASSERT("PKIX validation succeeded despite anyPolicyInhibit", !m_chain->validate(m_ee, untrusted, *m_dummy));

        untrusted[0] = m_int2;
        TSM_ASSERT("PKIX validation failed", m_chain->validate(m_ee, untrusted, *m_dummy));

        untrusted[0] = m_int3;
        TSM_ASSERT("PKIX validation failed", m_chain->validate(m_ee, untrusted, *m_dummy));

        TSM_ASSERT_EQUALS("Trust engine removal failed", m_chain->removeTrustEngine(trust), trust);
        delete trust;
    }

    void testExplicitPolicyMap() {
        TrustEngine* trust = buildTrustEngine("ExplicitPolicyMap");
        m_chain->addTrustEngine(trust);

        vector<XSECCryptoX509*> untrusted(1, m_int3);
        TSM_ASSERT("PKIX validation failed", m_chain->validate(m_ee, untrusted, *m_dummy));

        TSM_ASSERT_EQUALS("Trust engine removal failed", m_chain->removeTrustEngine(trust), trust);
        delete trust;
    }

    void testExplicitPolicyNoMap() {
        TrustEngine* trust = buildTrustEngine("ExplicitPolicyNoMap");
        m_chain->addTrustEngine(trust);

        vector<XSECCryptoX509*> untrusted(1, m_int3);
        TSM_ASSERT("PKIX validation succeeded despite policyMappingInhibit", !m_chain->validate(m_ee, untrusted, *m_dummy));

        TSM_ASSERT_EQUALS("Trust engine removal failed", m_chain->removeTrustEngine(trust), trust);
        delete trust;
    }

};
