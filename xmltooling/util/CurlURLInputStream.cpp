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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
//#include <sys/types.h>
//#include <sys/time.h>

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
      : fMulti(0)
      , fEasy(0)
      , fMemoryManager(urlSource.getMemoryManager())
      , fURLSource(urlSource)
      , fURL(0)
      , fTotalBytesRead(0)
      , fWritePtr(0)
      , fBytesRead(0)
      , fBytesToRead(0)
      , fDataAvailable(false)
      , fBufferHeadPtr(fBuffer)
      , fBufferTailPtr(fBuffer)
      , m_log(logging::Category::getInstance(XMLTOOLING_LOGCAT".libcurl.NetAccessor"))
{
	// Allocate the curl multi handle
	fMulti = curl_multi_init();
	
	// Allocate the curl easy handle
	fEasy = curl_easy_init();
	
	// Get the text of the URL we're going to use
	fURL.reset(XMLString::transcode(fURLSource.getURLText(), fMemoryManager), fMemoryManager);

	m_log.debug("libcurl trying to fetch %s", fURL.get());

	// Set URL option
	curl_easy_setopt(fEasy, CURLOPT_URL, fURL.get());
	curl_easy_setopt(fEasy, CURLOPT_WRITEDATA, this);						// Pass this pointer to write function
	curl_easy_setopt(fEasy, CURLOPT_WRITEFUNCTION, staticWriteCallback);	// Our static write function
    curl_easy_setopt(fEasy, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(fEasy, CURLOPT_TIMEOUT, 60);
    curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(fEasy, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(fEasy, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(fEasy, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(fEasy, CURLOPT_FAILONERROR, 1);
	
	// Add easy handle to the multi stack
	curl_multi_add_handle(fMulti, fEasy);
}


CurlURLInputStream::~CurlURLInputStream()
{
	// Remove the easy handle from the multi stack
	curl_multi_remove_handle(fMulti, fEasy);
	
	// Cleanup the easy handle
	curl_easy_cleanup(fEasy);
	
	// Cleanup the multi handle
	curl_multi_cleanup(fMulti);
}


size_t
CurlURLInputStream::staticWriteCallback(char *buffer,
                                      size_t size,
                                      size_t nitems,
                                      void *outstream)
{
	return ((CurlURLInputStream*)outstream)->writeCallback(buffer, size, nitems);
}



size_t
CurlURLInputStream::writeCallback(char *buffer,
                                      size_t size,
                                      size_t nitems)
{
	size_t cnt = size * nitems;
	size_t totalConsumed = 0;
		
	// Consume as many bytes as possible immediately into the buffer
	size_t consume = (cnt > fBytesToRead) ? fBytesToRead : cnt;
	memcpy(fWritePtr, buffer, consume);
	fWritePtr		+= consume;
	fBytesRead		+= consume;
	fTotalBytesRead	+= consume;
	fBytesToRead	-= consume;

	//m_log.debug("write callback consuming %d bytes", consume);

	// If bytes remain, rebuffer as many as possible into our holding buffer
	buffer			+= consume;
	totalConsumed	+= consume;
	cnt				-= consume;
	if (cnt > 0)
	{
		size_t bufAvail = sizeof(fBuffer) - (fBufferHeadPtr - fBuffer);
		consume = (cnt > bufAvail) ? bufAvail : cnt;
		memcpy(fBufferHeadPtr, buffer, consume);
		fBufferHeadPtr	+= consume;
		buffer			+= consume;
		totalConsumed	+= consume;
		//m_log.debug("write callback rebuffering %d bytes", consume);
	}
	
	// Return the total amount we've consumed. If we don't consume all the bytes
	// then an error will be generated. Since our buffer size is equal to the
	// maximum size that curl will write, this should never happen unless there
	// is a logic error somewhere here.
	return totalConsumed;
}


unsigned int
CurlURLInputStream::readBytes(XMLByte* const          toFill
                                     , const unsigned int maxToRead)
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
			fWritePtr		+= bufCnt;
			fBytesRead		+= bufCnt;
			fTotalBytesRead	+= bufCnt;
			fBytesToRead	-= bufCnt;
			
			fBufferTailPtr	+= bufCnt;
			if (fBufferTailPtr == fBufferHeadPtr)
				fBufferHeadPtr = fBufferTailPtr = fBuffer;
				
			//m_log.debug("consuming %d buffered bytes", bufCnt);

			tryAgain = true;
			continue;
		}
	
		// Ask the curl to do some work
		int runningHandles = 0;
		CURLMcode curlResult = curl_multi_perform(fMulti, &runningHandles);
        //m_log.debug("curl_multi_perform returned %d", curlResult);
		tryAgain = (curlResult == CURLM_CALL_MULTI_PERFORM);
		
		// Process messages from curl
		int msgsInQueue = 0;
		for (CURLMsg* msg = NULL; (msg = curl_multi_info_read(fMulti, &msgsInQueue)) != NULL; )
		{
			m_log.debug("msg %d, %d from curl", msg->msg, msg->data.result);

			if (msg->msg != CURLMSG_DONE)
				continue;
				
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
		}
		
		// If nothing is running any longer, bail out
        if (runningHandles == 0) {
            //m_log.debug("libcurl indicated no running handles");
			break;
        }
		
		// If there is no further data to read, and we haven't
		// read any yet on this invocation, call select to wait for data
		if (!tryAgain && fBytesRead == 0)
		{
			fd_set readSet;
			fd_set writeSet;
			fd_set exceptSet;
			int fdcnt=0;
			
			// Ask curl for the file descriptors to wait on
            FD_ZERO(&readSet);
            FD_ZERO(&writeSet);
            FD_ZERO(&exceptSet);
			(void) curl_multi_fdset(fMulti, &readSet, &writeSet, &exceptSet, &fdcnt);
			
			// Wait on the file descriptors
			timeval tv;
			tv.tv_sec  = 2;
			tv.tv_usec = 0;
			(void) select(fdcnt+1, &readSet, &writeSet, &exceptSet, &tv);
		}
	}
	
    //m_log.debug("returning with %d bytes to parser", fBytesRead);
	return fBytesRead;
}

