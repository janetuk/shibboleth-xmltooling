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
 * Resolves public keys and certificates based on KeyInfo information or
 * external factors. 
 */

#if !defined(__xmltooling_keyres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keyres_h__

#include <xmltooling/signature/KeyInfo.h>

#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

#include <vector>

namespace xmlsignature {

    /**
     * An API for resolving keys. The default/simple implementation
     * allows a hard-wired key to be supplied. This is mostly
     * useful for testing, or to adapt another mechanism for supplying
     * keys to this interface.
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
        virtual XSECCryptoKey* resolveKey(const KeyInfo* keyInfo) const {
            return m_key ? m_key->clone() : NULL;
        }

        /**
         * Returns a key based on the supplied KeyInfo information.
         * The caller must delete the key when done with it.
         * 
         * @param keyInfo   the key information
         * @return  the resolved key
         */
        virtual XSECCryptoKey* resolveKey(DSIGKeyInfoList* keyInfo) const {
            return m_key ? m_key->clone() : NULL;
        }

        /**
         * Returns a set of certificates based on the supplied KeyInfo information.
         * The certificates must be cloned if kept beyond the lifetime of the KeyInfo source.
         * 
         * @param keyInfo   the key information
         * @param certs     reference to vector to store certificates
         * @return  number of certificates returned
         */
        virtual std::vector<XSECCryptoX509*>::size_type resolveCertificates(
            const KeyInfo* keyInfo, std::vector<XSECCryptoX509*>& certs
            ) const;
        
        /**
         * Returns a set of certificates based on the supplied KeyInfo information.
         * The certificates must be cloned if kept beyond the lifetime of the KeyInfo source.
         * 
         * @param keyInfo   the key information
         * @param certs     reference to vector to store certificates
         * @return  number of certificates returned
         */
        virtual std::vector<XSECCryptoX509*>::size_type resolveCertificates(
            DSIGKeyInfoList* keyInfo, std::vector<XSECCryptoX509*>& certs 
            ) const;

    protected:
        XSECCryptoKey* m_key;
    };

    /**
     * Registers KeyResolver classes into the runtime.
     */
    void XMLTOOL_API registerKeyResolvers();

    /** KeyResolver based on hard-wired key */
    #define FILESYSTEM_KEY_RESOLVER  "org.opensaml.xmlooling.FilesystemKeyResolver"

    /** KeyResolver based on extracting information directly out of a KeyInfo */
    #define INLINE_KEY_RESOLVER  "org.opensaml.xmlooling.InlineKeyResolver"
};

#endif /* __xmltooling_keyres_h__ */
