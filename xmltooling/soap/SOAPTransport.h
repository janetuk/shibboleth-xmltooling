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
 * @file xmltooling/soap/SOAPTransport.h
 *
 * Encapsulates a transport layer protocol for sending/receiving messages.
 */

#ifndef __xmltooling_soaptrans_h__
#define __xmltooling_soaptrans_h__

#include <xmltooling/base.h>
#include <iostream>

namespace xmltooling {

    class XMLTOOL_API Credential;
    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API CredentialResolver;
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
        SOAPTransport();
    public:
        virtual ~SOAPTransport();

        /**
         * A simple structure to capture SOAP addressing information.
         */
        struct XMLTOOL_API Address {
            /**
             * Constructor.
             *
             * @param from      name of sender
             * @param to        name of recipient
             * @param endpoint  endpoint URL
             */
            Address(const char* from, const char* to, const char* endpoint) : m_from(from), m_to(to), m_endpoint(endpoint) {
            }

            /** Name of sender. */
            const char* m_from;

            /** Name of recipient. */
            const char* m_to;

            /** Endpoint URL. */
            const char* m_endpoint;
        };

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

        /**
         * Determines whether TLS/SSL connections include a check of the server's certificate
         * against the expected hostname or address. Defaults to true, and has no effect for
         * insecure protocols.
         *
         * @param verify    true iff the hostname should be verified against the server's certificate
         * @return  true iff the transport supports hostname verification
         */
        virtual bool setVerifyHost(bool verify)=0;

#ifndef XMLTOOLING_NO_XMLSEC
        /**
         * Supplies transport credentials.
         *
         * <p>The lifetime of the credential must be longer than the lifetime of this object.
         *
         * @param credential  a Credential instance, or NULL
         * @return true iff the transport supports the use of the Credential
         */
        virtual bool setCredential(const Credential* credential=NULL)=0;

        /**
         * Provides an X509TrustEngine to the transport to authenticate the transport peer.
         * The lifetime of the engine must be longer than the lifetime of this object.
         *
         * @param trustEngine   an X509TrustEngine instance, or NULL
         * @param credResolver  a CredentialResolver to supply the peer's trusted credentials, or NULL
         * @param criteria      optional criteria for selecting peer credentials
         * @param mandatory     flag controls whether message is sent at all if the
         *                      transport isn't authenticated using the TrustEngine
         * @return true iff the transport supports the use of a TrustEngine
         */
        virtual bool setTrustEngine(
            const X509TrustEngine* trustEngine=NULL,
            const CredentialResolver* credResolver=NULL,
            CredentialCriteria* criteria=NULL,
            bool mandatory=true
            )=0;
#endif

        /**
         * Sets an implementation-specific transport provider option.
         *
         * <p>Requires knowledge of the underlying SOAPTransport implementation.
         * Without the proper knowledge and inputs, crashes may result.
         *
         * @param provider  name of the SOAPTransport class the caller believes is in use
         * @param option    implementation-specific string containing the option to set
         * @param value     implementation- and option-specific string to use
         * @return  true iff the transport supports the option and value supplied
         */
        virtual bool setProviderOption(const char* provider, const char* option, const char* value);

        /**
         * Sends a stream of data over the transport. The function may return without
         * having received any data, depending on the nature of the transport.
         *
         * <p>If the stream is empty, a request may be issued with no body if the transport
         * supports that feature.
         *
         * @param in    input stream to send
         */
        virtual void send(std::istream& in)=0;

        /**
         * Sends an optional stream of data over the transport. The function may return without
         * having received any data, depending on the nature of the transport.
         *
         * <p>If the parameter is omitted, a request may be issued with no body if the transport
         * supports that feature.
         *
         * @param in    input stream to send
         */
        virtual void send(std::istream* in=NULL);

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
        virtual bool isAuthenticated() const=0;

        /**
         * Returns the MIME type of the response, if any.
         *
         * @return  MIME type of response, or an empty string
         */
        virtual std::string getContentType() const=0;
    };

#ifndef XMLTOOLING_NO_XMLSEC
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
#endif

};

#endif /* __xmltooling_soaptrans_h__ */
