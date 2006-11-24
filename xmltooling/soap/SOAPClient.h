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
 * @file xmltooling/soap/SOAPClient.h
 * 
 * Implements SOAP 1.1 messaging over a transport.
 */

#ifndef __xmltooling_soap11client_h__
#define __xmltooling_soap11client_h__

#include <xmltooling/security/KeyInfoSource.h>
#include <xmltooling/soap/SOAPTransport.h>

namespace soap11 {

    class XMLTOOL_API Envelope;
    class XMLTOOL_API Fault;

    /**
     * Implements SOAP 1.1 messaging over a transport.
     * 
     * In the abstract, this can be a one-way exchange, or use asynchronous
     * transports, but this is mostly theoretical at this point.
     */
    class XMLTOOL_API SOAPClient
    {
        MAKE_NONCOPYABLE(SOAPClient);
    public:
        SOAPClient(bool validate=false) : m_validate(validate), m_transport(NULL) {}
        virtual ~SOAPClient();
        
        /**
         * Controls schema validation of incoming XML messages.
         * This is separate from other forms of programmatic validation of objects,
         * but can detect a much wider range of syntax errors. 
         * 
         * @param validate  true iff the client should use a validating XML parser
         */
        void setValidating(bool validate=true) {
            m_validate = validate;
        }
        
        /**
         * Sends the supplied envelope to the identified recipient/endpoint.
         * 
         * <p>The client object will instantiate a transport layer object
         * appropriate for the endpoint URL provided and supply it to the
         * prepareTransport() method below.
         * 
         * @param env       SOAP envelope to send
         * @param peer      peer to send message to, expressed in TrustEngine terms
         * @param endpoint  URL of endpoint to recieve message
         */
        virtual void send(const Envelope* env, const xmltooling::KeyInfoSource& peer, const char* endpoint);
        
        /**
         * Returns the response message, if any. As long as a response is
         * "expected" but not available, NULL will be returned. If no response
         * will be forthcoming, an exception is raised.
         * 
         * <p>The caller is responsible for freeing the returned envelope.
         */
        virtual Envelope* receive();
        
        /**
         * Resets the object for another call.
         */
        virtual void reset();

    protected:
        /**
         * Allows client to supply transport-layer settings prior to sending message.
         * 
         * @param transport reference to transport layer
         */
        virtual void prepareTransport(const xmltooling::SOAPTransport& transport) {}

        /**
         * Handling of SOAP faults.
         * 
         * @param fault SOAP Fault received by client
         * @return true iff the Fault should be treated as a fatal error
         */
        virtual bool handleFault(const soap11::Fault& fault);
            
        /** Flag controlling schema validation. */
        bool m_validate;

        /** Holds response until retrieved by caller. */
        xmltooling::SOAPTransport* m_transport;
    };

};

#endif /* __xmltooling_soap11client_h__ */
