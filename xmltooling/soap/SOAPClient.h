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

#include <xmltooling/soap/SOAP.h>
#include <xmltooling/soap/SOAPTransport.h>

namespace soap11 {
    
    /**
     * Implements SOAP 1.1 messaging over a transport.
     * 
     * In the abstract, this can be a one-way exchange, or use asynchronous
     * transports, but this is mostly theoretical.
     */
    class XMLTOOL_API SOAPClient
    {
        MAKE_NONCOPYABLE(SOAPClient);
    public:
        SOAPClient() : m_response(NULL) {}
        virtual ~SOAPClient();
        
        /**
         * Sends the supplied envelope to the identified recipient/endpoint.
         * 
         * <p>The caller is responsible for freeing the outgoing envelope.
         * 
         * <p>The client object will instantiate a transport layer object
         * appropriate for the endpoint URL provided and supply it to the
         * prepareTransport() method below.
         * 
         * @param env       SOAP envelope to send
         * @param to        identifier/name of party to send message to
         * @param endpoint  URL of endpoint to recieve message
         */
        virtual void send(const Envelope* env, const char* to, const char* endpoint);
        
        /**
         * Returns the response message, if any. As long as a response is
         * "expected" but not available, NULL will be returned. If no response
         * will be forthcoming, an exception is raised.
         * 
         * <p>The caller is responsible for freeing the incoming envelope.
         */
        virtual Envelope* receive();

    protected:
        /**
         * Allows client to supply transport-layer settings prior to sending message.
         * 
         * @param transport reference to transport layer
         * @return true iff transport preparation was successful 
         */
        virtual bool prepareTransport(const xmltooling::SOAPTransport& transport) {}
        
        /** Holds response until retrieved by caller. */
        Envelope* m_response;
    };

};

#endif /* __xmltooling_soap11client_h__ */
