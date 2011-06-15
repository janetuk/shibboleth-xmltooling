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
 * @file xmltooling/io/GenericRequest.h
 *
 * Interface to generic protocol requests that transport XML messages.
 */

#ifndef __xmltooling_genreq_h__
#define __xmltooling_genreq_h__

#include <xmltooling/base.h>

#include <string>
#include <vector>

#ifndef XMLTOOLING_NO_XMLSEC
# include <xsec/enc/XSECCryptoX509.hpp>
#endif

namespace xmltooling {

    /**
     * Interface to generic protocol requests that transport XML messages.
     *
     * <p>This interface need not be threadsafe.
     */
    class XMLTOOL_API GenericRequest {
        MAKE_NONCOPYABLE(GenericRequest);
    protected:
        GenericRequest();
    public:
        virtual ~GenericRequest();

        /**
         * Returns the URL scheme of the request (http, https, ftp, ldap, etc.)
         *
         * @return the URL scheme
         */
        virtual const char* getScheme() const=0;

        /**
         * Returns true iff the request is over a confidential channel.
         *
         * @return confidential channel indicator
         */
        virtual bool isSecure() const=0;

        /**
         * Returns hostname of service that received request.
         *
         * @return hostname of service
         */
        virtual const char* getHostname() const=0;

        /**
         * Returns incoming port.
         *
         * @return  incoming port
         */
        virtual int getPort() const=0;

        /**
         * Returns the MIME type of the request, if known.
         *
         * @return the MIME type, or an empty string
         */
        virtual std::string getContentType() const=0;

        /**
         * Returns the length of the request body, if known.
         *
         * @return the content length, or -1 if unknown
         */
        virtual long getContentLength() const=0;

        /**
         * Returns the raw request body.
         *
         * @return the request body, or nullptr
         */
        virtual const char* getRequestBody() const=0;

        /**
         * Returns a decoded named parameter value from the request.
         * If a parameter has multiple values, only one will be returned.
         *
         * @param name  the name of the parameter to return
         * @return a single parameter value or nullptr
         */
        virtual const char* getParameter(const char* name) const=0;

        /**
         * Returns all of the decoded values of a named parameter from the request.
         * All values found will be returned.
         *
         * @param name      the name of the parameter to return
         * @param values    a vector in which to return pointers to the decoded values
         * @return  the number of values returned
         */
        virtual std::vector<const char*>::size_type getParameters(
            const char* name, std::vector<const char*>& values
            ) const=0;

        /**
         * Returns the transport-authenticated identity associated with the request,
         * if authentication is solely handled by the transport.
         *
         * @return the authenticated username or an empty string
         */
        virtual std::string getRemoteUser() const=0;

        /**
         * Gets the authentication type associated with the request.
         *
         * @return  the authentication type or nullptr
         */
        virtual std::string getAuthType() const {
            return "";
        }

        /**
         * Returns the IP address of the client.
         *
         * @return the client's IP address
         */
        virtual std::string getRemoteAddr() const=0;

        /**
         * Returns the chain of certificates sent by the client.
         * They are not guaranteed to be valid according to any particular definition.
         *
         * @return the client's certificate chain
         */
        virtual const
#ifndef XMLTOOLING_NO_XMLSEC
            std::vector<XSECCryptoX509*>&
#else
            std::vector<std::string>&
#endif
            getClientCertificates() const=0;
    };
};

#endif /* __xmltooling_genreq_h__ */
