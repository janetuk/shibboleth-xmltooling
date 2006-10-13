/*
 *  Copyright 2001-2005 Internet2
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

#include <fstream>
#include <xmltooling/signature/KeyInfo.h>
#include <xmltooling/signature/KeyResolver.h>

using namespace xmlsignature;

class InlineKeyResolverTest : public CxxTest::TestSuite {
    KeyResolver* m_resolver;
public:
    InlineKeyResolverTest() : m_resolver(NULL) {}

    void setUp() {
        string config = data_path + "InlineKeyResolver.xml";
        ifstream in(config.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        XercesJanitor<DOMDocument> janitor(doc);
        m_resolver=XMLToolingConfig::getConfig().KeyResolverManager.newPlugin(INLINE_KEY_RESOLVER,doc->getDocumentElement());
    }

    void tearDown() {
        delete m_resolver;
        m_resolver=NULL;
    }

    void testResolver() {
        string path=data_path + "KeyInfo1.xml";
        ifstream fs(path.c_str());
        DOMDocument* doc=XMLToolingConfig::getConfig().getValidatingParser().parse(fs);
        TS_ASSERT(doc!=NULL);
        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        TS_ASSERT(b!=NULL);
        auto_ptr<KeyInfo> kiObject(dynamic_cast<KeyInfo*>(b->buildFromDocument(doc)));
        TS_ASSERT(kiObject.get()!=NULL);

        auto_ptr<XSECCryptoKey> key(m_resolver->resolveKey(kiObject.get()));
        TSM_ASSERT("Unable to resolve public key.", key.get()!=NULL);
        TSM_ASSERT_EQUALS("Unexpected key type.", key->getKeyType(), XSECCryptoKey::KEY_RSA_PUBLIC);

        auto_ptr<XSECCryptoX509CRL> crl(m_resolver->resolveCRL(kiObject.get()));
        TSM_ASSERT("Unable to resolve CRL.", crl.get()!=NULL);

        KeyResolver::ResolvedCertificates certs;
        TSM_ASSERT_EQUALS("Wrong certificate count.", m_resolver->resolveCertificates(kiObject.get(), certs), 1);
    }
};
