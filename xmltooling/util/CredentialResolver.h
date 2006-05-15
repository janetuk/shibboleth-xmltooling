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
 * @file CredentialResolver.h
 * 
 * Provides access to keys and certificates.
 */

#if !defined(__xmltooling_credres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_credres_h__

#include <xmltooling/Lockable.h>

#include <vector>
#include <xsec/enc/XSECCryptoKey.hpp>
#include <xsec/enc/XSECCryptoX509.hpp>

namespace xmltooling {

    /**
     * An abstract interface to credential formats like files, keystores, hardware tokens, etc.
     * All non-const methods require that the interface be locked.
     */
    class XMLTOOL_API CredentialResolver : public virtual Lockable
    {
    MAKE_NONCOPYABLE(CredentialResolver);
    public:
        virtual ~CredentialResolver() {}

        /**
         * Returns an identifier for the credential.
         * 
         * @return      the identifier
         */
        virtual const char* getId() const=0;

        /**
         * Gets the public key associated with the credential.
         * The caller <strong>MUST NOT</strong> modify the object.
         * 
         * @return      the public key, or NULL
         */
        virtual XSECCryptoKey* getPublicKey()=0;

        /**
         * Gets the private key associated with the credential.
         * The caller <strong>MUST NOT</strong> modify the object.
         * 
         * @return      the private key, or NULL
         */
        virtual XSECCryptoKey* getPrivateKey()=0;

        /**
         * Gets the certificate chain associated with the credential.
         * The caller <strong>MUST NOT</strong> modify the objects.
         * The EE certificate <strong>MUST</strong> be first.
         * 
         * @return      a chain of certificates, or NULL
         */
        virtual const std::vector<XSECCryptoX509*>* getX509Certificates()=0;
    
    protected:
        CredentialResolver() {}
    };

};

#endif /* __xmltooling_credres_h__ */
