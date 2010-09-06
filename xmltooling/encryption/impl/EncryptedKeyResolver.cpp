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
 * EncryptedKeyResolver.cpp
 * 
 * Resolves encrypted keys based on EncryptedData information or other external factors.
 */

#include "internal.h"
#include "encryption/EncryptedKeyResolver.h"
#include "signature/KeyInfo.h"

using namespace xmlencryption;
using namespace xmlsignature;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

EncryptedKeyResolver::EncryptedKeyResolver()
{
}

EncryptedKeyResolver::~EncryptedKeyResolver()
{
}

const EncryptedKey* EncryptedKeyResolver::resolveKey(const EncryptedData& encryptedData, const XMLCh* recipient) const
{
    if (!encryptedData.getKeyInfo())
        return nullptr;

    const vector<XMLObject*>& others = const_cast<const KeyInfo*>(encryptedData.getKeyInfo())->getUnknownXMLObjects();
    for (vector<XMLObject*>::const_iterator i = others.begin(); i != others.end(); i++) {
        EncryptedKey* encKey = dynamic_cast<EncryptedKey*>(*i);
        if (encKey) {
            if (!recipient || !encKey->getRecipient() || XMLString::equals(recipient,encKey->getRecipient()))
                return encKey;
        }
    }

    static const XMLCh rmtype[] = { // http://www.w3.org/2001/04/xmlenc#EncryptedKey
        chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
        chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
        chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash, chDigit_0, chDigit_4, chForwardSlash,
        chLatin_x, chLatin_m, chLatin_l, chLatin_e, chLatin_n, chLatin_c, chPound,
        chLatin_E, chLatin_n, chLatin_c, chLatin_r, chLatin_y, chLatin_p, chLatin_t, chLatin_e, chLatin_d, chLatin_K, chLatin_e, chLatin_y, chNull
    };

    const XMLObject* treeRoot = nullptr;
    const vector<RetrievalMethod*>& methods = const_cast<const KeyInfo*>(encryptedData.getKeyInfo())->getRetrievalMethods();
    for (vector<RetrievalMethod*>::const_iterator m = methods.begin(); m != methods.end(); ++m) {
        if (XMLString::equals((*m)->getType(), rmtype)) {
            const XMLCh* ref = (*m)->getURI();
            if (ref && *ref == chPound) {
                if (!treeRoot) {
                    treeRoot = &encryptedData;
                    while (treeRoot->getParent())
                        treeRoot = treeRoot->getParent();
                }
                const EncryptedKey* encKey = dynamic_cast<const EncryptedKey*>(XMLHelper::getXMLObjectById(*treeRoot, ref+1));
                if (encKey)
                    return encKey;
            }
        }
    }

    return nullptr;
}
