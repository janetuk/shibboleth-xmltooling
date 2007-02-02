/*
 * Copyright 2001-2007 The Apache Software Foundation.
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
 * @file xmltooling/security/XSECCryptoX509CRL.h
 * 
 * Wrapper for X.509 CRL objects, similar to existing XSEC wrappers.
 */

#if !defined(__xmltooling_x509crl_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_x509crl_h__

#include <xmltooling/base.h>

#include <xsec/framework/XSECDefs.hpp>
#include <xsec/utils/XSECSafeBuffer.hpp>

namespace xmltooling {
    /**
     * Interface class for X.509 CRLs
     * The library uses classes derived from this to process X.509 CRLs.
     */
    class XMLTOOL_API XSECCryptoX509CRL {
        MAKE_NONCOPYABLE(XSECCryptoX509CRL);
    protected:
        XSECCryptoX509CRL() {}
    public:
    	virtual ~XSECCryptoX509CRL() {}
    
    	/**
    	 * Returns a string that identifies the crypto owner of this library.
         * 
         * @return  the crypto provider name
    	 */
    	virtual const XMLCh* getProviderName() const=0;
    
    	/**
    	 * Load a CRL into the object.
    	 * Takes a base64 DER-encoded CRL and loads it.
    	 *
    	 * @param buf buffer containing the Base64 encoded CRL
    	 * @param len number of bytes of data in the CRL buffer
    	 */
    
        /**
         * Returns a duplicate of the original object.
         *
         * @return  the duplicate
         */
        virtual XSECCryptoX509CRL* clone() const=0;

        /**
         * Load a Base64-encoded CRL into the object.
         *
         * @param buf buffer containing the base64-encoded CRL
         * @param len number of bytes of data in the CRL buffer (0 if the string is null terminated)
         */
    	virtual void loadX509CRLBase64Bin(const char* buf, unsigned int len)=0;
    
    	/**
    	 * Load a PEM encoded CRL into the object.
    	 *
    	 * @param buf buffer containing the PEM encoded CRL
    	 * @param len number of bytes of data in the CRL buffer (0 if the string is null terminated)
    	 */
    	void loadX509CRLPEM(const char* buf, unsigned int len=0);
    
    	/**
    	 * Get a Base64 DER encoded copy of the CRL
    	 *
    	 * @return A safeBuffer containing the DER encoded certificate
    	 */
    	virtual safeBuffer& getDEREncodingSB(void)=0;
    };
};

#endif /* __xmltooling_x509crl_h__ */


