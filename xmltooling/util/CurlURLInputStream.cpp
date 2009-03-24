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
 * xmltooling/util/CurlURLInputStream.cpp
 *
 * Asynchronous use of curl to fetch data from a URL.
 */

#include "internal.h"

#include <xmltooling/util/CurlURLInputStream.h>
#include <xmltooling/util/XMLHelper.h>

#include <openssl/ssl.h>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLNetAccessor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace xmltooling;
using namespace xercesc;

namespace {
    static const XMLCh  _CURL[] =           UNICODE_LITERAL_4(C,U,R,L);
    static const XMLCh _option[] =          UNICODE_LITERAL_6(o,p,t,i,o,n);
    static const XMLCh _provider[] =        UNICODE_LITERAL_8(p,r,o,v,i,d,e,r);
    static const XMLCh TransportOption[] =  UNICODE_LITERAL_15(T,r,a,n,s,p,o,r,t,O,p,t,i,o,n);
    static const XMLCh uri[] =              UNICODE_LITERAL_3(u,r,i);
    static const XMLCh url[] =              UNICODE_LITERAL_3(u,r,l);
    static const XMLCh verifyHost[] =       UNICODE_LITERAL_10(v,e,r,i,f,y,H,o,s,t);

    // callback to invoke a caller-defined SSL callback
    CURLcode ssl_ctx_callback(CURL* curl, SSL_CTX* ssl_ctx, void* userptr)
    {
        // Manually disable SSLv2 so we're not dependent on libcurl to do it.
        // Also disable the ticket option where implemented, since this breaks a variety
        // of servers. Newer libcurl also does this for us.
#ifdef SSL_OP_NO_TICKET
        SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2|SSL_OP_NO_TICKET);
#else
        SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL|SSL_OP_NO_SSLv2);
#endif

        return CURLE_OK;
    }
}

CurlURLInputStream::CurlURLInputStream(const char* url)
    : fLog(logging::Category::getInstance(XMLTOOLING_LOGCAT".libcurl.InputStream"))
    , fURL(url)
    , fMulti(0)
    , fEasy(0)
    , fTotalBytesRead(0)
    , fWritePtr(0)
    , fBytesRead(0)
    , fBytesToRead(0)
    , fDataAvailable(false)
    , fBufferHeadPtr(fBuffer)
    , fBufferTailPtr(fBuffer)
    , fContentType(0)
{
    init();
}

CurlURLInputStream::CurlURLInputStream(const XMLCh* url)
    : fLog(logging::Category::getInstance(XMLTOOLING_LOGCAT".libcurl.InputStream"))
    , fMulti(0)
    , fEasy(0)
    , fTotalBytesRead(0)
    , fWritePtr(0)
    , fBytesRead(0)
    , fBytesToRead(0)
    , fDataAvailable(false)
    , fBufferHeadPtr(fBuffer)
    , fBufferTailPtr(fBuffer)
    , fContentType(0)
{
    auto_ptr_char temp(url);
    fURL = temp.get();
    init();
}

CurlURLInputStream::CurlURLInputStream(const DOMElement* e)
    : fLog(logging::Category::getInstance(XMLTOOLING_LOGCAT".libcurl.InputStream"))
    , fMulti(0)
    , fEasy(0)
    , fTotalBytesRead(0)
    , fWritePtr(0)
    , fBytesRead(0)
    , fBytesToRead(0)
    , fDataAvailable(false)
    , fBufferHeadPtr(fBuffer)
    , fBufferTailPtr(fBuffer)
    , fContentType(0)
{
    const XMLCh* attr = e->getAttributeNS(NULL, url);
    if (!attr || !*attr) {
        attr = e->getAttributeNS(NULL, uri);
        if (!attr || !*attr)
            throw IOException("No URL supplied via DOM to CurlURLInputStream constructor.");
    }

    auto_ptr_char temp(attr);
    fURL = temp.get();
    init(e);
}

CurlURLInputStream::~CurlURLInputStream()
{
    if (fEasy) {
        // Remove the easy handle from the multi stack
        curl_multi_remove_handle(fMulti, fEasy);

        // Cleanup the easy handle
        curl_easy_cleanup(fEasy);
    }

    if (fMulti) {
        // Cleanup the multi handle
        curl_multi_cleanup(fMulti);
    }

    XMLString::release(&fContentType);
}

void CurlURLInputStream::init(const DOMElement* e)
{
    // Allocate the curl multi handle
    fMulti = curl_multi_init();

    // Allocate the curl easy handle
    fEasy = curl_easy_init();

    if (!fMulti || !fEasy)
        throw IOException("Failed to allocate libcurl handles.");

    curl_easy_setopt(fEasy, CURLOPT_URL, fURL.c_str());

    // Set up a way to recieve the data
    curl_easy_setopt(fEasy, CURLOPT_WRITEDATA, this);                       // Pass this pointer to write function
    curl_easy_setopt(fEasy, CURLOPT_WRITEFUNCTION, staticWriteCallback);    // Our static write function

    // Do redirects
    curl_easy_setopt(fEasy, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(fEasy, CURLOPT_MAXREDIRS, 6);

    // Default settings.
    curl_easy_setopt(fEasy, CURLOPT_CONNECTTIMEOUT,30);
    curl_easy_setopt(fEasy, CURLOPT_TIMEOUT,60);
    curl_easy_setopt(fEasy, CURLOPT_HTTPAUTH,0);
    curl_easy_setopt(fEasy, CURLOPT_USERPWD,NULL);
    curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(fEasy, CURLOPT_SSL_CIPHER_LIST, "ALL:!aNULL:!LOW:!EXPORT:!SSLv2");
    curl_easy_setopt(fEasy, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(fEasy, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(fEasy, CURLOPT_FAILONERROR, 1);

    // Install SSL callback.
    curl_easy_setopt(fEasy, CURLOPT_SSL_CTX_FUNCTION, ssl_ctx_callback);

    fError[0] = 0;
    curl_easy_setopt(fEasy, CURLOPT_ERRORBUFFER, fError);

    if (e) {
        const XMLCh* flag = e->getAttributeNS(NULL, verifyHost);
        if (flag && (*flag == chLatin_f || *flag == chDigit_0))
            curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYHOST, 0);

        // Process TransportOption elements.
        bool success;
        DOMElement* child = XMLHelper::getLastChildElement(e, TransportOption);
        while (child) {
            if (child->hasChildNodes() && XMLString::equals(child->getAttributeNS(NULL,_provider), _CURL)) {
                auto_ptr_char option(child->getAttributeNS(NULL,_option));
                auto_ptr_char value(child->getFirstChild()->getNodeValue());
                if (option.get() && *option.get() && value.get() && *value.get()) {
                    // For libcurl, the option is an enum and the value type depends on the option.
                    CURLoption opt = static_cast<CURLoption>(strtol(option.get(), NULL, 10));
                    if (opt < CURLOPTTYPE_OBJECTPOINT)
                        success = (curl_easy_setopt(fEasy, opt, strtol(value.get(), NULL, 10)) == CURLE_OK);
#ifdef CURLOPTTYPE_OFF_T
                    else if (opt < CURLOPTTYPE_OFF_T)
                        success = (curl_easy_setopt(fEasy, opt, value.get()) == CURLE_OK);
                    else if (sizeof(curl_off_t) == sizeof(long))
                        success = (curl_easy_setopt(fEasy, opt, strtol(value.get(), NULL, 10)) == CURLE_OK);
                    else
                        success = false;
#else
                    else
                        success = (curl_easy_setopt(fEasy, opt, value.get()) == CURLE_OK);
#endif
                    if (!success)
                        fLog.error("failed to set transport option (%s)", option.get());
                }
            }
            child = XMLHelper::getPreviousSiblingElement(child, TransportOption);
        }
    }

    // Add easy handle to the multi stack
    curl_multi_add_handle(fMulti, fEasy);

    fLog.debug("libcurl trying to fetch %s", fURL.c_str());

    // Start reading, to get the content type
    while(fBufferHeadPtr == fBuffer) {
        int runningHandles = 0;
        try {
            readMore(&runningHandles);
        }
        catch (XMLException& ex) {
            curl_multi_remove_handle(fMulti, fEasy);
            curl_easy_cleanup(fEasy);
            fEasy = NULL;
            curl_multi_cleanup(fMulti);
            fMulti = NULL;
            auto_ptr_char msg(ex.getMessage());
            throw IOException(msg.get());
        }
        if(runningHandles == 0) break;
    }

    // Find the content type
    char* contentType8 = NULL;
    curl_easy_getinfo(fEasy, CURLINFO_CONTENT_TYPE, &contentType8);
    if(contentType8)
        fContentType = XMLString::transcode(contentType8);
}


size_t CurlURLInputStream::staticWriteCallback(char* buffer, size_t size, size_t nitems, void* outstream)
{
    return ((CurlURLInputStream*)outstream)->writeCallback(buffer, size, nitems);
}

size_t CurlURLInputStream::writeCallback(char* buffer, size_t size, size_t nitems)
{
    size_t cnt = size * nitems;
    size_t totalConsumed = 0;

    // Consume as many bytes as possible immediately into the buffer
    size_t consume = (cnt > fBytesToRead) ? fBytesToRead : cnt;
    memcpy(fWritePtr, buffer, consume);
    fWritePtr       += consume;
    fBytesRead      += consume;
    fTotalBytesRead += consume;
    fBytesToRead    -= consume;

    //fLog.debug("write callback consuming %d bytes", consume);

    // If bytes remain, rebuffer as many as possible into our holding buffer
    buffer          += consume;
    totalConsumed   += consume;
    cnt             -= consume;
    if (cnt > 0)
    {
        size_t bufAvail = sizeof(fBuffer) - (fBufferHeadPtr - fBuffer);
        consume = (cnt > bufAvail) ? bufAvail : cnt;
        memcpy(fBufferHeadPtr, buffer, consume);
        fBufferHeadPtr  += consume;
        buffer          += consume;
        totalConsumed   += consume;
        //fLog.debug("write callback rebuffering %d bytes", consume);
    }

    // Return the total amount we've consumed. If we don't consume all the bytes
    // then an error will be generated. Since our buffer size is equal to the
    // maximum size that curl will write, this should never happen unless there
    // is a logic error somewhere here.
    return totalConsumed;
}

bool CurlURLInputStream::readMore(int* runningHandles)
{
    // Ask the curl to do some work
    CURLMcode curlResult = curl_multi_perform(fMulti, runningHandles);

    // Process messages from curl
    int msgsInQueue = 0;
    for (CURLMsg* msg = NULL; (msg = curl_multi_info_read(fMulti, &msgsInQueue)) != NULL; )
    {
        //fLog.debug("msg %d, %d from curl", msg->msg, msg->data.result);

        if (msg->msg != CURLMSG_DONE)
            return true;

        switch (msg->data.result)
        {
        case CURLE_OK:
            // We completed successfully. runningHandles should have dropped to zero, so we'll bail out below...
            break;

        case CURLE_UNSUPPORTED_PROTOCOL:
            ThrowXML(MalformedURLException, XMLExcepts::URL_UnsupportedProto);
            break;

        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_RESOLVE_PROXY:
            ThrowXML1(NetAccessorException,  XMLExcepts::NetAcc_TargetResolution, fURL.c_str());
            break;

        case CURLE_COULDNT_CONNECT:
            ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_ConnSocket, fURL.c_str());
            break;

        case CURLE_RECV_ERROR:
            ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_ReadSocket, fURL.c_str());
            break;

        default:
            fLog.error("error while fetching %s: (%d) %s", fURL.c_str(), msg->data.result, fError);
            ThrowXML1(NetAccessorException, XMLExcepts::NetAcc_InternalError, fURL.c_str());
            break;
        }
    }

    // If nothing is running any longer, bail out
    if(*runningHandles == 0)
        return false;

    // If there is no further data to read, and we haven't
    // read any yet on this invocation, call select to wait for data
    if (curlResult != CURLM_CALL_MULTI_PERFORM && fBytesRead == 0)
    {
        fd_set readSet;
        fd_set writeSet;
        fd_set exceptSet;
        int fdcnt=0;

        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&exceptSet);

        // Ask curl for the file descriptors to wait on
        curl_multi_fdset(fMulti, &readSet, &writeSet, &exceptSet, &fdcnt);

        // Wait on the file descriptors
        timeval tv;
        tv.tv_sec  = 2;
        tv.tv_usec = 0;
        select(fdcnt+1, &readSet, &writeSet, &exceptSet, &tv);
    }

    return curlResult == CURLM_CALL_MULTI_PERFORM;
}

xsecsize_t CurlURLInputStream::readBytes(XMLByte* const toFill, const xsecsize_t maxToRead)
{
    fBytesRead = 0;
    fBytesToRead = maxToRead;
    fWritePtr = toFill;

    for (bool tryAgain = true; fBytesToRead > 0 && (tryAgain || fBytesRead == 0); )
    {
        // First, any buffered data we have available
        size_t bufCnt = fBufferHeadPtr - fBufferTailPtr;
        bufCnt = (bufCnt > fBytesToRead) ? fBytesToRead : bufCnt;
        if (bufCnt > 0)
        {
            memcpy(fWritePtr, fBufferTailPtr, bufCnt);
            fWritePtr       += bufCnt;
            fBytesRead      += bufCnt;
            fTotalBytesRead += bufCnt;
            fBytesToRead    -= bufCnt;

            fBufferTailPtr  += bufCnt;
            if (fBufferTailPtr == fBufferHeadPtr)
                fBufferHeadPtr = fBufferTailPtr = fBuffer;

            //fLog.debug("consuming %d buffered bytes", bufCnt);

            tryAgain = true;
            continue;
        }

        // Ask the curl to do some work
        int runningHandles = 0;
        tryAgain = readMore(&runningHandles);

        // If nothing is running any longer, bail out
        if (runningHandles == 0)
            break;
    }

    return fBytesRead;
}
