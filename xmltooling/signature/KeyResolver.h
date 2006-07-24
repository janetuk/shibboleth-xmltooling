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
 * @file KeyResolver.h
 * 
 * Resolves keys based on KeyInfo information or other external factors. 
 */

#if !defined(__xmltooling_keyres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keyres_h__

#include <xmltooling/signature/KeyInfo.h>

#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/enc/XSECCryptoKey.hpp>

namespace xmlsignature {

    /**
     * An API for resolving keys.
     */
    class XMLTOOL_API KeyResolver {
        MAKE_NONCOPYABLE(KeyResolver);
    public:
        /**
         * Constructor based on a single externally supplied key.
         * The key will be destroyed when the resolver is. 
         * 
         * @param key   external key
         */
        KeyResolver(XSECCryptoKey* key=NULL) : m_key(key) {}
        
        virtual ~KeyResolver() {
            delete m_key;
        }
        
        /**
         * Returns a key based on the supplied KeyInfo information.
         * The caller must delete the key when done with it.
         * 
         * @param keyInfo   the key information
         * @return  the resolved key
         */
        virtual XSECCryptoKey* resolveKey(KeyInfo* keyInfo) {
            return m_key ? m_key->clone() : NULL;
        }

        /**
         * Returns a key based on the supplied KeyInfo information.
         * The caller must delete the key when done with it.
         * 
         * @param keyInfo   the key information
         * @return  the resolved key
         */
        virtual XSECCryptoKey* resolveKey(DSIGKeyInfoList* keyInfo=NULL) {
            return m_key ? m_key->clone() : NULL;
        }
        
    protected:
        XSECCryptoKey* m_key;
    };

};

#endif /* __xmltooling_keyres_h__ */
