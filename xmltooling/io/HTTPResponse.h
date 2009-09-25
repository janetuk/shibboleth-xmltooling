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
 * @file xmltooling/io/HTTPResponse.h
 * 
 * Interface to HTTP responses.
 */

#ifndef __xmltooling_httpres_h__
#define __xmltooling_httpres_h__

#include <xmltooling/io/GenericResponse.h>

namespace xmltooling {
    
    /**
     * Interface to HTTP response.
     * 
     * <p>To supply information to the surrounding web server environment,
     * a shim must be supplied in the form of this interface to adapt the
     * library to different proprietary server APIs.
     * 
     * <p>This interface need not be threadsafe.
     */
    class XMLTOOL_API HTTPResponse : public GenericResponse {
    protected:
        HTTPResponse() {}
    public:
        virtual ~HTTPResponse() {}
        
        void setContentType(const char* type) {
            setResponseHeader("Content-Type", type);
        }
        
        /**
         * Sets or clears a response header.
         * 
         * @param name  header name
         * @param value value to set, or NULL to clear
         */
        virtual void setResponseHeader(const char* name, const char* value)=0;

        /**
         * Sets a client cookie.
         * 
         * @param name  cookie name
         * @param value value to set, or NULL to clear
         */
        virtual void setCookie(const char* name, const char* value);
        
        /**
         * Redirect the client to the specified URL and complete the response.
         * Any headers previously set will be sent ahead of the redirect.
         * 
         * @param url   location to redirect client
         * @return a result code to return from the calling MessageEncoder
         */
        virtual long sendRedirect(const char* url)=0;
        
        /** Some common HTTP status codes. */
        enum status_t {
            XMLTOOLING_HTTP_STATUS_OK = 200,
            XMLTOOLING_HTTP_STATUS_MOVED = 302,
            XMLTOOLING_HTTP_STATUS_UNAUTHORIZED = 401,
            XMLTOOLING_HTTP_STATUS_FORBIDDEN = 403,
            XMLTOOLING_HTTP_STATUS_NOTFOUND = 404,
            XMLTOOLING_HTTP_STATUS_ERROR = 500
        };
        
        using GenericResponse::sendResponse;

        long sendError(std::istream& inputStream) {
            return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_ERROR);
        }

        long sendResponse(std::istream& inputStream) {
            return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_OK);
        }
    };
};

#endif /* __xmltooling_httpres_h__ */
