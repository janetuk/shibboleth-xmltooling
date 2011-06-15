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
 * @file xmltooling/security/OpenSSLCredential.h
 * 
 * OpenSSL-specific credential
 */

#if !defined(__xmltooling_opensslcred_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_opensslcred_h__

#include <xmltooling/security/X509Credential.h>

#include <openssl/ssl.h>

namespace xmltooling {

    /**
     * An OpenSSL-specific credential
     */
    class XMLTOOL_API OpenSSLCredential : public virtual X509Credential
    {
    protected:
        OpenSSLCredential();
        
    public:
        virtual ~OpenSSLCredential();
        
        /**
         * Attaches credential to an OpenSSL SSL context object.
         * The credential <strong>MUST</strong> be disposable after attachment.
         * 
         * @param ctx   an SSL context
         */
        virtual void attach(SSL_CTX* ctx) const=0;
    };

};

#endif /* __xmltooling_opensslcred_h__ */
