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
 * @file xmltooling/signature/OpenSSLCredentialResolver.h
 * 
 * OpenSSL-specific credential resolver
 */

#if !defined(__xmltooling_opensslcredres_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_opensslcredres_h__

#include <xmltooling/signature/CredentialResolver.h>

#include <openssl/ssl.h>

namespace xmlsignature {

    /**
     * An OpenSSL-specific API for resolving local/owned keys and certificates
     */
    class XMLTOOL_API OpenSSLCredentialResolver : public CredentialResolver
    {
    protected:
        OpenSSLCredentialResolver() {}
        
    public:
        virtual ~OpenSSLCredentialResolver() {}
        
        /**
         * Attaches credentials to an OpenSSL SSL context object.
         * The resolver is unlockable after attachment.
         * 
         * @param ctx   an SSL context
         */
        virtual void attach(SSL_CTX* ctx) const=0;
    };

};

#endif /* __xmltooling_opensslcredres_h__ */
