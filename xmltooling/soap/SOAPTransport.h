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
 * @file xmltooling/soap/SOAPTransport.h
 * 
 * Encapsulates a transport layer protocol for sending/receiving messages.
 */

#ifndef __xmltooling_soaptrans_h__
#define __xmltooling_soaptrans_h__

#include <xmltooling/base.h>
#include <iostream>

namespace xmlsignature {
    class XMLTOOL_API CredentialResolver;
    class XMLTOOL_API KeyResolver;
};

namespace xmltooling {
    
    class XMLTOOL_API X509TrustEngine;
    
    /**
     * Encapsulates a transport layer protocol for sending/receiving messages.
     * 
     * Most of the methods are const, meaning they don't affect the transport
     * layer until the data is sent.
     */
    class XMLTOOL_API SOAPTransport
    {
        MAKE_NONCOPYABLE(SOAPTransport);
    protected:
        SOAPTransport() {}
    public:
        virtual ~SOAPTransport() {}
        
        /**
         * Indicates whether transport provides confidentiality.
         * 
         * @return  true iff transport layer provides confidentiality
         */
        virtual bool isConfidential() const=0;
        
        /**
         * Sets the connection timeout.
         * 
         * @param timeout  time to wait for connection to server in seconds, or -1 for no timeout
         * @return  true iff the transport supports connection timeouts
         */
        virtual bool setConnectTimeout(long timeout)=0;
        
        /**
         * Sets the request timeout.
         * 
         * @param timeout  time to wait for a response in seconds, or -1 for no timeout
         * @return  true iff the transport supports request/response timeouts
         */
        virtual bool setTimeout(long timeout)=0;
        
        /**
         * Common types of transport authentication that may be supported.
         */
        enum transport_auth_t {
            transport_auth_none = 0,
            transport_auth_basic = 1,
            transport_auth_digest = 2,
            transport_auth_ntlm = 3,
            transport_auth_gss = 4
        };
        
        /**
         * Sets a particular form of transport authentication and credentials.
         * 
         * @param authType  type of transport authentication to use
         * @param username  username for transport authentication
         * @param password  simple password/credential for transport authentication
         * @return  true iff the transport supports the indicated form of authentication
         */
        virtual bool setAuth(transport_auth_t authType, const char* username=NULL, const char* password=NULL)=0;

#ifndef XMLTOOLING_NO_XMLSEC
        /**
         * Provides a CredentialResolver to the transport to supply transport credentials.
         * The lifetime of the resolver must be longer than the lifetime of this object.
         * 
         * <p>The CredentialResolver <strong>MUST</strong> be locked by the caller. 
         * 
         * @param credResolver  a locked CredentialResolver instance, or NULL
         * @return true iff the transport supports the use of a CredentialResolver
         */
        virtual bool setCredentialResolver(const xmlsignature::CredentialResolver* credResolver)=0;

        /**
         * Provides a TrustEngine to the transport to authenticate the transport peer.
         * The lifetime of the engine must be longer than the lifetime of this object.
         * 
         * @param trustEngine   a TrustEngine instance, or NULL
         * @param mandatory     flag controls whether message is sent at all if the
         *                      transport isn't authenticated using the TrustEngine
         * @param keyResolver   optional externally supplied KeyResolver, or NULL
         * @return true iff the transport supports the use of a TrustEngine
         */
        virtual bool setTrustEngine(
            const X509TrustEngine* trustEngine,
            bool mandatory=true,
            const xmlsignature::KeyResolver* keyResolver=NULL
            )=0;
#endif

        /**
         * Sends a stream of data over the transport. The function may return without
         * having received any data, depending on the nature of the transport.
         * 
         * @param in    input stream to send
         */        
        virtual void send(std::istream& in)=0;
        
        /**
         * Returns reference to response stream.  The resulting stream must be
         * checked directly to determine whether data is available.
         * 
         * @return  reference to a stream containing the response, if any
         */
        virtual std::istream& receive()=0;
        
        /**
         * Returns result of authenticating transport peer.
         * 
         * @return true iff TrustEngine or other mechanism successfully authenticated the peer
         */
        virtual bool isSecure() const=0;

        /**
         * Returns the MIME type of the response, if any.
         * 
         * @return  MIME type of response, or an empty string
         */
        virtual std::string getContentType() const=0;
    };

    /**
     * Registers SOAPTransport classes into the runtime.
     */
    void XMLTOOL_API registerSOAPTransports();
    
    /**
     * Notifies transport infrastructure to initialize. 
     */
    void XMLTOOL_API initSOAPTransports();
    
    /**
     * Notifies transport infrastructure to shutdown. 
     */
    void XMLTOOL_API termSOAPTransports();

};

#endif /* __xmltooling_soaptrans_h__ */
