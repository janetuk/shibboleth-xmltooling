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

/**
 * EncryptedKeyResolver.cpp
 * 
 * Resolves encrypted keys based on EncryptedData information or other external factors.
 */

#include "internal.h"
#include "encryption/EncryptedKeyResolver.h"

using namespace xmlencryption;
using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

const EncryptedKey* EncryptedKeyResolver::resolveKey(const EncryptedData& encryptedData, const XMLCh* recipient) const
{
    if (!encryptedData.getKeyInfo())
        return NULL;

    const vector<XMLObject*>& others=const_cast<const KeyInfo*>(encryptedData.getKeyInfo())->getUnknownXMLObjects();
    for (vector<XMLObject*>::const_iterator i=others.begin(); i!=others.end(); i++) {
        EncryptedKey* encKey=dynamic_cast<EncryptedKey*>(*i);
        if (encKey && (!recipient || !encKey->getRecipient() || XMLString::equals(recipient,encKey->getRecipient())))
            return encKey;
    }

    return NULL;
}
