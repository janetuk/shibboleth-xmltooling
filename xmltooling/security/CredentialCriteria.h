/*
 *  Copyright 2001-2009 Internet2
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
 * @file xmltooling/security/CredentialCriteria.h
 * 
 * Class for specifying criteria by which a CredentialResolver should resolve credentials.
 */

#if !defined(__xmltooling_credcrit_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_credcrit_h__

#include <xmltooling/base.h>

#include <set>

class DSIGKeyInfoList;
class XSECCryptoKey;

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
    class XMLTOOL_API Signature;
};

namespace xmltooling {

    /**
     * Class for specifying criteria by which a CredentialResolver should resolve credentials.
     */
    class XMLTOOL_API CredentialCriteria
    {
        MAKE_NONCOPYABLE(CredentialCriteria);
    public:
        /** Default constructor. */
        CredentialCriteria();

        virtual ~CredentialCriteria();

        /**
         * Determines whether the supplied Credential matches this CredentialCriteria.
         *
         * @param credential    the Credential to evaluate
         * @return true iff the Credential is consistent with this criteria
         */
        virtual bool matches(const Credential& credential) const;
       
        /**
         * Get key usage criteria.
         * 
         * @return the usage mask
         */
        unsigned int getUsage() const {
            return m_keyUsage;
        }
    
        /**
         * Set key usage criteria.
         * 
         * @param usage the usage mask to set
         */
        void setUsage(unsigned int usage) {
            m_keyUsage = usage;
        }

        /**
         * Get the peer name criteria.
         * 
         * @return the peer name
         */
        const char* getPeerName() const {
            return m_peerName.c_str();
        }
    
        /**
         * Set the peer name criteria.
         * 
         * @param peerName peer name to set
         */
        void setPeerName(const char* peerName) {
            m_peerName.erase();
            if (peerName)
                m_peerName = peerName;
        }
    
        /**
         * Get the key algorithm criteria.
         * 
         * @return the key algorithm
         */
        const char* getKeyAlgorithm() const {
            return m_keyAlgorithm.c_str();
        }
    
        /**
         * Set the key algorithm criteria.
         * 
         * @param keyAlgorithm The key algorithm to set
         */
        void setKeyAlgorithm(const char* keyAlgorithm) {
            m_keyAlgorithm.erase();
            if (keyAlgorithm)
                m_keyAlgorithm = keyAlgorithm;
        }

        /**
         * Get the key size criteria.
         *
         * @return  the key size, or 0
         */
        unsigned int getKeySize() const {
            return m_keySize;
        }

        /**
         * Set the key size criteria.
         *
         * @param keySize Key size to set
         */
        void setKeySize(unsigned int keySize) {
            m_keySize = keySize;
        }
    
        /**
         * Set the key algorithm and size criteria based on an XML algorithm specifier.
         *
         * @param algorithm XML algorithm specifier
         */
        void setXMLAlgorithm(const XMLCh* algorithm);

        /**
         * Gets key name criteria.
         * 
         * @return an immutable set of key names
         */
        const std::set<std::string>& getKeyNames() const {
            return m_keyNames;
        }

        /**
         * Gets key name criteria.
         * 
         * @return a mutable set of key names
         */
        std::set<std::string>& getKeyNames() {
            return m_keyNames;
        }

        /**
         * Returns the public key criteria.
         * 
         * @return  a public key
         */
        virtual XSECCryptoKey* getPublicKey() const {
            return m_key;
        }

        /**
         * Sets the public key criteria.
         *
         * <p>The lifetime of the key <strong>MUST</strong> extend
         * for the lifetime of this object.
         * 
         * @param key a public key
         */
        void setPublicKey(XSECCryptoKey* key) {
            m_key = key;
        }

        /**
         * Bitmask constants controlling the kinds of criteria set automatically
         * based on a KeyInfo object.
         */
        enum keyinfo_extraction_t {
            KEYINFO_EXTRACTION_KEY = 1,
            KEYINFO_EXTRACTION_KEYNAMES = 2
        };

        /**
         * Gets the KeyInfo criteria.
         * 
         * @return the KeyInfo criteria
         */
        const xmlsignature::KeyInfo* getKeyInfo() const {
            return m_keyInfo;
        }
    
        /**
         * Sets the KeyInfo criteria.
         * 
         * @param keyInfo       the KeyInfo criteria
         * @param extraction    bitmask of criteria to auto-extract from KeyInfo
         */
        virtual void setKeyInfo(const xmlsignature::KeyInfo* keyInfo, int extraction=0);

        /**
         * Gets the native KeyInfo criteria.
         * 
         * @return the native KeyInfo criteria
         */
        DSIGKeyInfoList* getNativeKeyInfo() const {
            return m_nativeKeyInfo;
        }

        /**
         * Sets the KeyInfo criteria.
         * 
         * @param keyInfo       the KeyInfo criteria
         * @param extraction    bitmask of criteria to auto-extract from KeyInfo
         */
        virtual void setNativeKeyInfo(DSIGKeyInfoList* keyInfo, int extraction=0);

        /**
         * Sets the KeyInfo criteria from an XML Signature.
         * 
         * @param sig           the Signature containing KeyInfo criteria
         * @param extraction    bitmask of criteria to auto-extract from KeyInfo
         */
        void setSignature(const xmlsignature::Signature& sig, int extraction=0);

    private:
        unsigned int m_keyUsage;
        unsigned int m_keySize;
        std::string m_peerName,m_keyAlgorithm;
        std::set<std::string> m_keyNames;
        XSECCryptoKey* m_key;
        const xmlsignature::KeyInfo* m_keyInfo;
        DSIGKeyInfoList* m_nativeKeyInfo;
        Credential* m_credential;
    };
};

#endif /* __xmltooling_credcrit_h__ */
