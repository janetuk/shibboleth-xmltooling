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
 * @file xmltooling/security/CredentialCriteria.h
 * 
 * Class for specifying criteria by which a CredentialResolver should resolve credentials. 
 */

#if !defined(__xmltooling_credcrit_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_credcrit_h__

#include <xmltooling/unicode.h>
#include <xmltooling/signature/KeyInfo.h>
#include <xmltooling/signature/Signature.h>

#include <string>
#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/dsig/DSIGKeyInfoName.hpp>

namespace xmltooling {

    /**
     * Class for specifying criteria by which a CredentialResolver should resolve credentials.
     */
    class XMLTOOL_API CredentialCriteria
    {
        MAKE_NONCOPYABLE(CredentialCriteria);
    public:
        CredentialCriteria() : m_keyUsage(UNSPECIFIED_CREDENTIAL), m_keyInfo(NULL), m_nativeKeyInfo(NULL) {}
        virtual ~CredentialCriteria() {}

        enum UsageType {
            UNSPECIFIED_CREDENTIAL,
            SIGNING_CREDENTIAL,
            TLS_CREDENTIAL,
            ENCRYPTION_CREDENTIAL
        };
        
        /**
         * Get the key usage criteria.
         * 
         * @return the usage.
         */
        UsageType getUsage() const {
            return m_keyUsage;
        }
    
        /**
         * Set the key usage criteria.
         * 
         * @param usage the usage to set
         */
        void setUsage(UsageType usage) {
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
         * @return returns the keyAlgorithm.
         */
        const char* getKeyAlgorithm() const {
            return m_keyAlgorithm.c_str();
        }
    
        /**
         * Set the key algorithm criteria.
         * 
         * @param keyAlgorithm The keyAlgorithm to set.
         */
        void setKeyAlgorithm(const char* keyAlgorithm) {
            m_keyAlgorithm.erase();
            if (keyAlgorithm)
                m_keyAlgorithm = keyAlgorithm;
        }
    
        /**
         * Get the key name criteria.
         * 
         * @return the key name
         */
        const char* getKeyName() const {
            return m_keyName.c_str();
        }
    
        /**
         * Set the key name criteria.
         * 
         * @param keyName key name to set
         */
        void setKeyName(const char* keyName) {
            m_keyName.erase();
            if (keyName)
                m_keyName = keyName;
        }
        
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
         * @param keyInfo   the KeyInfo criteria
         */
        void setKeyInfo(const xmlsignature::KeyInfo* keyInfo) {
            m_keyInfo = keyInfo;
        } 

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
         * @param keyInfo   the KeyInfo criteria
         */
        void setNativeKeyInfo(DSIGKeyInfoList* keyInfo) {
            m_nativeKeyInfo = keyInfo;
        }

        void setSignature(const xmlsignature::Signature& sig) {
            xmlsignature::KeyInfo* k = sig.getKeyInfo();
            if (k)
                return setKeyInfo(k);
            DSIGSignature* dsig = sig.getXMLSignature();
            if (dsig)
                setNativeKeyInfo(dsig->getKeyInfoList());
        }

    private:
        UsageType m_keyUsage;
        std::string m_peerName,m_keyAlgorithm,m_keyName;
        const xmlsignature::KeyInfo* m_keyInfo;
        DSIGKeyInfoList* m_nativeKeyInfo;
    };
};

#endif /* __xmltooling_credcrit_h__ */
