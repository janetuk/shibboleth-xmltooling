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

#include "XMLObjectBaseTestCase.h"

#include <xmltooling/security/SecurityHelper.h>

class SecurityHelperTest : public CxxTest::TestSuite {
    vector<XSECCryptoX509*> certs;
public:
    void setUp() {
    }

    void tearDown() {
        for_each(certs.begin(), certs.end(), xmltooling::cleanup<XSECCryptoX509>());
    }

    void testKeysFromFiles() {
        string pathname = data_path + "key.pem";
        auto_ptr<XSECCryptoKey> key1(SecurityHelper::loadKeyFromFile(pathname.c_str()));
        pathname = data_path + "key.der";
        auto_ptr<XSECCryptoKey> key2(SecurityHelper::loadKeyFromFile(pathname.c_str()));
        pathname = data_path + "test.pfx";
        auto_ptr<XSECCryptoKey> key3(SecurityHelper::loadKeyFromFile(pathname.c_str(), NULL, "password"));

        TSM_ASSERT("PEM/DER keys did not match", SecurityHelper::matches(key1.get(), key2.get()));
        TSM_ASSERT("DER/PKCS12 keys did not match", SecurityHelper::matches(key2.get(), key3.get()));

        pathname = data_path + "key2.pem";
        auto_ptr<XSECCryptoKey> key4(SecurityHelper::loadKeyFromFile(pathname.c_str()));
        TSM_ASSERT("Different keys matched", !SecurityHelper::matches(key3.get(), key4.get()));
    }

    void testCertificatesFromFiles() {
        string pathname = data_path + "cert.pem";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        pathname = data_path + "cert.der";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str());
        pathname = data_path + "test.pfx";
        SecurityHelper::loadCertificatesFromFile(certs, pathname.c_str(), NULL, "password");

        TSM_ASSERT_EQUALS("Wrong certificate count", certs.size(), 3);

        auto_ptr<XSECCryptoKey> key1(certs[0]->clonePublicKey());
        auto_ptr<XSECCryptoKey> key2(certs[0]->clonePublicKey());
        auto_ptr<XSECCryptoKey> key3(certs[0]->clonePublicKey());

        TSM_ASSERT("PEM/DER keys did not match", SecurityHelper::matches(key1.get(), key2.get()));
        TSM_ASSERT("DER/PKCS12 keys did not match", SecurityHelper::matches(key2.get(), key3.get()));

        for_each(certs.begin(), certs.end(), xmltooling::cleanup<XSECCryptoX509>());
    }
};
