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

#include <xmltooling/security/XSECCryptoX509CRL.h>

#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

#include <algorithm>
#include <vector>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
    class XMLTOOL_API Signature;

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
         * Returns a key based on the supplied KeyInfo information.
         * The caller must delete the key when done with it.
         * 
         * @param sig   signature containing the key information
         * @return  the resolved key
         */
        XSECCryptoKey* resolveKey(const Signature* sig) const;

        /**
         * A wrapper that handles disposal of certificates when required.
         */
        class XMLTOOL_API ResolvedCertificates {
            MAKE_NONCOPYABLE(ResolvedCertificates);
            bool m_owned;
            std::vector<XSECCryptoX509*> m_certs;
        public:
            ResolvedCertificates() : m_owned(false) {}
            
            ~ResolvedCertificates() {
                clear();
            }
            
            /**
             * Empties the container and frees any held resources.
             */
            void clear() {
                if (m_owned) {
                    std::for_each(m_certs.begin(), m_certs.end(), xmltooling::cleanup<XSECCryptoX509>());
                    m_owned = false;
                }
                m_certs.clear();
            }
            
            /**
             * Transfers ownership of certificates outside wrapper.
             * 
             * @param writeTo   a container into which to move the certificates
             * @return  true iff the certificates must be freed by caller
             */
            bool release(std::vector<XSECCryptoX509*>& writeTo) {
                writeTo.assign(m_certs.begin(),m_certs.end());
                m_certs.clear();
                if (m_owned) {
                    m_owned=false;
                    return true;
                }
                return false;
            }
            
            /**
             * Accesses the underlying array of certificates.
             * 
             * @return reference to certificate container
             */
            const std::vector<XSECCryptoX509*>& v() const {
                return m_certs;
            }
            
            friend class XMLTOOL_API KeyResolver;
        };

        /**
         * Returns a set of certificates based on the supplied KeyInfo information.
         * The certificates must be cloned if kept beyond the lifetime of the KeyInfo source.
         * 
         * @param keyInfo   the key information
         * @param certs     reference to object to hold certificates
         * @return  number of certificates returned
         */
        virtual std::vector<XSECCryptoX509*>::size_type resolveCertificates(
            const KeyInfo* keyInfo, ResolvedCertificates& certs
            ) const;
        
        /**
         * Returns a set of certificates based on the supplied KeyInfo information.
         * The certificates must be cloned if kept beyond the lifetime of the KeyInfo source.
         * 
         * @param keyInfo   the key information
         * @param certs     reference to object to hold certificates
         * @return  number of certificates returned
         */
        virtual std::vector<XSECCryptoX509*>::size_type resolveCertificates(
            DSIGKeyInfoList* keyInfo, ResolvedCertificates& certs 
            ) const;

        /**
         * Returns a set of certificates based on the supplied KeyInfo information.
         * The certificates must be cloned if kept beyond the lifetime of the KeyInfo source.
         * 
         * @param sig   signature containing the key information
         * @param certs     reference to object to hold certificates
         * @return  number of certificates returned
         */
        std::vector<XSECCryptoX509*>::size_type resolveCertificates(
            const Signature* sig, ResolvedCertificates& certs
            ) const;

        /**
         * Returns a CRL based on the supplied KeyInfo information.
         * The caller must delete the CRL when done with it.
         * 
         * @param keyInfo   the key information
         * @return  the resolved CRL
         */
        virtual xmltooling::XSECCryptoX509CRL* resolveCRL(const KeyInfo* keyInfo) const;
        
        /**
         * Returns a CRL based on the supplied KeyInfo information.
         * The caller must delete the CRL when done with it.
         * 
         * @param keyInfo   the key information
         * @return  the resolved CRL
         */
        virtual xmltooling::XSECCryptoX509CRL* resolveCRL(DSIGKeyInfoList* keyInfo) const;

        /**
         * Returns a CRL based on the supplied KeyInfo information.
         * The caller must delete the CRL when done with it.
         * 
         * @param sig   signature containing the key information
         * @return  the resolved CRL
         */
        xmltooling::XSECCryptoX509CRL* resolveCRL(const Signature* sig) const;

    protected:
        /** Stores an explicit key. */
        XSECCryptoKey* m_key;

        /**
         * Accessor for certificate vector from derived KeyResolver classes.
         *
         * @param certs certificate wrapper to access
         * @return modifiable reference to vector inside wrapper
         */
        std::vector<XSECCryptoX509*>& accessCertificates(ResolvedCertificates& certs) const {
            return certs.m_certs;
        }

        /**
         * Accessor for certificate ownership flag from derived KeyResolver classes.
         *
         * @param certs certificate wrapper to access
         * @return modifiable reference to ownership flag inside wrapper
         */
        bool& accessOwned(ResolvedCertificates& certs) const {
            return certs.m_owned;
        }
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
