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
 * @file xmltooling/soap/OpenSSLSOAPTransport.h
 * 
 * Encapsulates OpenSSL-capable SOAP transport layer.
 */

#ifndef __xmltooling_opensslsoaptrans_h__
#define __xmltooling_opensslsoaptrans_h__

#include <xmltooling/soap/SOAPTransport.h>

#include <openssl/ssl.h>

namespace xmltooling {
    
    /**
     * Encapsulates OpenSSL-capable SOAP transport layer.
     */
    class XMLTOOL_API OpenSSLSOAPTransport : public virtual SOAPTransport 
    {
    protected:
        OpenSSLSOAPTransport();
    public:
        virtual ~OpenSSLSOAPTransport();
        
        /** OpenSSL context callback for manipulating credentials and validation behavior. */
        typedef bool (*ssl_ctx_callback_fn)(OpenSSLSOAPTransport* transport, SSL_CTX* ssl_ctx, void* userptr);

        /**
         * Sets a callback function to invoke against the SSL_CTX before the handshake.
         * 
         * @param fn        callback function
         * @param userptr   a caller-supplied value to pass to the callback function
         * @return true iff the callback was set
         */
        virtual bool setSSLCallback(ssl_ctx_callback_fn fn, void* userptr=NULL)=0;
        
        /**
         * Sets indicator that the transport peer has been authenticated.
         * 
         * @param auth    flag to set
         */
        virtual void setAuthenticated(bool auth)=0;
    };

};

#endif /* __xmltooling_opensslsoaptrans_h__ */
