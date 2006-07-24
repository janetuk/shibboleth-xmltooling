/*
 *  Copyright 2001-2006 Internet2
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

#include <xmltooling/signature/CredentialResolver.h>

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
                FILESYSTEM_CREDENTIAL_RESOLVER,doc->getDocumentElement()
                )
            );

        Locker locker(credResolver.get());
        auto_ptr<XSECCryptoKey> key(credResolver->getKey());
        TSM_ASSERT("Retrieved key was null", key.get()!=NULL);
        TSM_ASSERT_EQUALS("Unexpected number of certificates", 1, credResolver->getCertificates().size());
    }
};
