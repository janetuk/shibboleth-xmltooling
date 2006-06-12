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

/**
 * @file EncryptedKeyResolver.h
 * 
 * Resolves encrypted keys based on EncryptedData information or other external factors.
 */

#if !defined(__xmltooling_enckeyres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_enckeyres_h__

#include <xmltooling/encryption/Encryption.h>
#include <xmltooling/signature/KeyResolver.h>

namespace xmlencryption {

    /**
     * An API for resolving encrypted decryption keys.
     */
    class XMLTOOL_API EncryptedKeyResolver : public xmlsignature::KeyResolver {
    public:
        virtual ~EncryptedKeyResolver() {}
        
        /**
         * Returns an encrypted key based on the supplied KeyInfo information.
         * 
         * @param encryptedData an encrypted object
         * @return  the resolved EncryptedKey object
         */
        virtual EncryptedKey* resolveKey(EncryptedData* encryptedData)=0;
    };

};

#endif /* __xmltooling_enckeyres_h__ */
