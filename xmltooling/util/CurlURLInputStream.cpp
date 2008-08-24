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

/*
 * $Id$
 */

#include "internal.h"

#include <curl/curl.h>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <xmltooling/util/CurlURLInputStream.hpp>

using namespace xmltooling;


CurlURLInputStream::CurlURLInputStream(const XMLURL& urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
      : fMemoryManager(urlSource.getMemoryManager())
      , fURLSource(urlSource)
      , fURL(0)
      , fInputStream(NULL)
      , m_log(logging::Category::getInstance(XMLTOOLING_LOGCAT".libcurl.NetAccessor"))
{
	// Get the text of the URL we're going to use
	fURL.reset(XMLString::transcode(fURLSource.getURLText(), fMemoryManager), fMemoryManager);
}


CurlURLInputStream::~CurlURLInputStream()
{
    delete fInputStream;
}


size_t CurlURLInputStream::staticWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t len = size*nmemb;
    reinterpret_cast<std::stringstream*>(stream)->write(reinterpret_cast<const char*>(ptr),len);
    return len;
}


unsigned int CurlURLInputStream::readBytes(XMLByte* const toFill, const unsigned int maxToRead)
{
    if (!fInputStream) {
        // Allocate the curl easy handle.
        CURL* fEasy = curl_easy_init();
        if (!fEasy)
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_InternalError, "unable to allocate libcurl handle", fMemoryManager);

        m_log.debug("libcurl trying to fetch %s", fURL.get());

        // Set URL option
        curl_easy_setopt(fEasy, CURLOPT_URL, fURL.get());
        curl_easy_setopt(fEasy, CURLOPT_WRITEDATA, &fUnderlyingStream);
        curl_easy_setopt(fEasy, CURLOPT_WRITEFUNCTION, staticWriteCallback);
        curl_easy_setopt(fEasy, CURLOPT_CONNECTTIMEOUT, 30);
        curl_easy_setopt(fEasy, CURLOPT_TIMEOUT, 60);
        curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(fEasy, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt(fEasy, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(fEasy, CURLOPT_FAILONERROR, 1);

        char curl_errorbuf[CURL_ERROR_SIZE];
        curl_errorbuf[0]=0;
        curl_easy_setopt(fEasy,CURLOPT_ERRORBUFFER,curl_errorbuf);

        // Fetch the data.
        if (curl_easy_perform(fEasy) != CURLE_OK) {
            curl_easy_cleanup(fEasy);
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_InternalError, curl_errorbuf, fMemoryManager);
        }

        curl_easy_cleanup(fEasy);

        /*
        switch (msg->data.result)
        {
        case CURLE_OK:
            // We completed successfully. runningHandles should have dropped to zero, so we'll bail out below...
            break;

        case CURLE_UNSUPPORTED_PROTOCOL:
            ThrowXMLwithMemMgr(MalformedURLException, XMLExcepts::URL_UnsupportedProto, fMemoryManager);
            break;

        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_RESOLVE_PROXY:
            ThrowXMLwithMemMgr1(NetAccessorException,  XMLExcepts::NetAcc_TargetResolution, fURLSource.getHost(), fMemoryManager);
            break;

        case CURLE_COULDNT_CONNECT:
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ConnSocket, fURLSource.getURLText(), fMemoryManager);

        case CURLE_RECV_ERROR:
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, fURLSource.getURLText(), fMemoryManager);
            break;

        default:
            m_log.error("curl NetAccessor encountered error from libcurl (%d)", msg->data.result);
            ThrowXMLwithMemMgr1(NetAccessorException, XMLExcepts::NetAcc_InternalError, fURLSource.getURLText(), fMemoryManager);
            break;
        }
        */

        fInputStream = new (fMemoryManager) StreamInputSource::StreamBinInputStream(fUnderlyingStream);
    }

    // Defer to the stream wrapper.
    return fInputStream->readBytes(toFill, maxToRead);
}
