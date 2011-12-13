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

    class XMLTOOL_API Credential;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

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
        unsigned int getUsage() const;
    
        /**
         * Set key usage criteria.
         * 
         * @param usage the usage mask to set
         */
        void setUsage(unsigned int usage);

        /**
         * Get the peer name criteria.
         * 
         * @return the peer name
         */
        const char* getPeerName() const;
    
        /**
         * Set the peer name criteria.
         * 
         * @param peerName peer name to set
         */
        void setPeerName(const char* peerName);
    
        /**
         * Get the key algorithm criteria.
         * 
         * @return the key algorithm
         */
        const char* getKeyAlgorithm() const;
    
        /**
         * Set the key algorithm criteria.
         * 
         * @param keyAlgorithm the key algorithm to set
         */
        void setKeyAlgorithm(const char* keyAlgorithm);

        /**
         * Get the key size criteria.
         * <p>If a a maximum size is also set, this is treated as a minimum.
         *
         * @return  the key size, or 0
         */
        unsigned int getKeySize() const;

        /**
         * Set the key size criteria.
         * <p>If a a maximum size is also set, this is treated as a minimum.
         *
         * @param keySize key size to set
         */
        void setKeySize(unsigned int keySize);

        /**
         * Get the maximum key size criteria.
         *
         * @return  the maximum key size, or 0
         */
        unsigned int getMaxKeySize() const;

        /**
         * Set the maximum key size criteria.
         *
         * @param keySize maximum key size to set
         */
        void setMaxKeySize(unsigned int keySize);

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
        const std::set<std::string>& getKeyNames() const;

        /**
         * Gets key name criteria.
         * 
         * @return a mutable set of key names
         */
        std::set<std::string>& getKeyNames();

        /**
         * Returns the public key criteria.
         * 
         * @return  a public key
         */
        virtual XSECCryptoKey* getPublicKey() const;

        /**
         * Sets the public key criteria.
         *
         * <p>The lifetime of the key <strong>MUST</strong> extend
         * for the lifetime of this object.
         * 
         * @param key a public key
         */
        void setPublicKey(XSECCryptoKey* key);

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
        const xmlsignature::KeyInfo* getKeyInfo() const;
    
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
        DSIGKeyInfoList* getNativeKeyInfo() const;

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

        /**
         * Resets object to a default state.
         */
        virtual void reset();

    private:
        unsigned int m_keyUsage;
        unsigned int m_keySize,m_maxKeySize;
        std::string m_peerName,m_keyAlgorithm;
        std::set<std::string> m_keyNames;
        XSECCryptoKey* m_key;
        const xmlsignature::KeyInfo* m_keyInfo;
        DSIGKeyInfoList* m_nativeKeyInfo;
        Credential* m_credential;
    };

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif
};

#endif /* __xmltooling_credcrit_h__ */
