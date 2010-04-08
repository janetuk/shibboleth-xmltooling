/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file xmltooling/util/CurlURLInputStream.h
 *
 * Asynchronous use of curl to fetch data from a URL.
 */

#if !defined(__xmltooling_curlinstr_h__) && !defined(XMLTOOLING_LITE)
#define __xmltooling_curlinstr_h__

#include <xmltooling/logging.h>

#include <string>
#include <vector>
#include <curl/curl.h>
#include <xercesc/util/BinInputStream.hpp>

namespace xmltooling {

    /**
     * Adapted from Xerces-C as a more advanced input stream implementation
     * for subsequent use in parsing remote documents.
     */
    class XMLTOOL_API CurlURLInputStream : public xercesc::BinInputStream
    {
    public :
        /**
         * Constructor.
         *
         * @param url       the URL of the resource to fetch
         * @param cacheTag  optional pointer to string used for cache management
         */
        CurlURLInputStream(const char* url, std::string* cacheTag=NULL);

        /**
         * Constructor.
         *
         * @param url       the URL of the resource to fetch
         * @param cacheTag  optional pointer to string used for cache management
         */
        CurlURLInputStream(const XMLCh* url, std::string* cacheTag=NULL);

        /**
         * Constructor taking a DOM element supporting the following content:
         * 
         * <dl>
         *  <dt>uri | url</dt>
         *  <dd>identifies the remote resource</dd>
         *  <dt>verifyHost</dt>
         *  <dd>true iff name of host should be matched against TLS/SSL certificate</dd>
         *  <dt>TransportOption elements, like so:</dt>
         *  <dd>&lt;TransportOption provider="CURL" option="150"&gt;0&lt;/TransportOption&gt;</dd>
         * </dl>
         * 
         * @param e         DOM to supply configuration
         * @param cacheTag  optional pointer to string used for cache management
         */
        CurlURLInputStream(const xercesc::DOMElement* e, std::string* cacheTag=NULL);

        ~CurlURLInputStream();

#ifdef XMLTOOLING_XERCESC_64BITSAFE
        XMLFilePos
#else
        unsigned int
#endif
        curPos() const {
            return fTotalBytesRead;
        }

#ifdef XMLTOOLING_XERCESC_INPUTSTREAM_HAS_CONTENTTYPE
        const XMLCh* getContentType() const {
            return fContentType;
        }
#endif

        xsecsize_t readBytes(XMLByte* const toFill, const xsecsize_t maxToRead);

        /**
         * Access the OpenSSL context options in place for this object.
         *
         * @return bitmask suitable for use with SSL_CTX_set_options
         */
        int getOpenSSLOps() const {
            return fOpenSSLOps;
        }

    private :
        CurlURLInputStream(const CurlURLInputStream&);
        CurlURLInputStream& operator=(const CurlURLInputStream&);

        // libcurl callbacks for data read/write
        static size_t staticWriteCallback(char *buffer, size_t size, size_t nitems, void *outstream);
        size_t writeCallback(char *buffer, size_t size, size_t nitems);

        void init(const xercesc::DOMElement* e=NULL);
        bool readMore(int *runningHandles);

        logging::Category&  fLog;
        std::string*        fCacheTag;
        std::string         fURL;
        std::vector<std::string>    fSavedOptions;
        int                 fOpenSSLOps;

        CURLM*              fMulti;
        CURL*               fEasy;
        struct curl_slist*  fHeaders;

        unsigned long       fTotalBytesRead;
        XMLByte*            fWritePtr;
        xsecsize_t          fBytesRead;
        xsecsize_t          fBytesToRead;
        bool                fDataAvailable;

        // Overflow buffer for when curl writes more data to us
        // than we've asked for.
        XMLByte*            fBuffer;
        XMLByte*            fBufferHeadPtr;
        XMLByte*            fBufferTailPtr;
        size_t              fBufferSize;

        XMLCh*              fContentType;
        long                fStatusCode;

        char                fError[CURL_ERROR_SIZE];
    };
};

#endif // __xmltooling_curlinstr_h__
