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

#include <xmltooling/security/CredentialResolver.h>
#include <xmltooling/security/X509Credential.h>

#include <fstream>

class FilesystemCredentialResolverTest : public CxxTest::TestSuite {
public:
    void setUp() {
    }
    
    void tearDown() {
    }

    void testFilesystemProvider() {
        string config = data_path + "FilesystemCredentialResolver.xml";
        ifstream in(config.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        XercesJanitor<DOMDocument> janitor(doc);

        auto_ptr<CredentialResolver> credResolver(
            XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(
                CHAINING_CREDENTIAL_RESOLVER,doc->getDocumentElement()
                )
            );

        Locker locker(credResolver.get());
        const X509Credential* cred=dynamic_cast<const X509Credential*>(credResolver->resolve());
        TSM_ASSERT("Retrieved credential was null", cred!=nullptr);
        TSM_ASSERT("Retrieved key was null", cred->getPrivateKey()!=nullptr);
        TSM_ASSERT_EQUALS("Unexpected number of certificates", 1, cred->getEntityCertificateChain().size());
        TSM_ASSERT_EQUALS("Custom key name not found", 1, cred->getKeyNames().count("Sample Key"));
    }
};
