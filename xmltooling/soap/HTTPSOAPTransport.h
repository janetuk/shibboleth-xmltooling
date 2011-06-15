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
 * @file xmltooling/soap/HTTPSOAPTransport.h
 * 
 * Encapsulates HTTP SOAP transport layer.
 */

#ifndef __xmltooling_httpsoaptrans_h__
#define __xmltooling_httpsoaptrans_h__

#include <xmltooling/soap/SOAPTransport.h>
#include <string>
#include <vector>

namespace xmltooling {
    
    /**
     * Encapsulates HTTP SOAP transport layer.
     */
    class XMLTOOL_API HTTPSOAPTransport : public virtual SOAPTransport 
    {
    protected:
        HTTPSOAPTransport();
    public:
        virtual ~HTTPSOAPTransport();
        
        /**
         * Indicate whether content should be sent using HTTP 1.1 and
         * Chunked Transport-Encoding, or buffered and sent with a Content-Length.
         *
         * @param chunked true iff chunked encoding should be used
         * @return  true iff the property is successfully set
         */
        virtual bool useChunkedEncoding(bool chunked=true)=0;

        /**
         * Sets an outgoing HTTP request header.
         * 
         * @param name   name of header, without the colon separator
         * @param value  header value to send
         * @return  true iff the header is successfully set
         */
        virtual bool setRequestHeader(const char* name, const char* value)=0;

        /**
         * Controls redirect behavior.
         *
         * @param follow    true iff Location-based redirects should be honored
         * @param maxRedirs maximum number of redirects to permit
         */
        virtual bool followRedirects(bool follow, unsigned int maxRedirs);
        
        /**
         * Returns the values of an HTTP response header.
         * 
         * @param name  name of header, without the colon separator
         * @return  reference to array of header values
         */
        virtual const std::vector<std::string>& getResponseHeader(const char* name) const=0;
    };

};

#endif /* __xmltooling_httpsoaptrans_h__ */
