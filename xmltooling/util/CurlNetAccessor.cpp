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

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xmltooling/util/CurlURLInputStream.hpp>
#include <xmltooling/util/CurlNetAccessor.hpp>

using namespace xmltooling;
using namespace xercesc;

const XMLCh xmltooling::CurlNetAccessor::fgMyName[] =
{
    chLatin_C, chLatin_u, chLatin_r, chLatin_l, chLatin_N, chLatin_e,
    chLatin_t, chLatin_A, chLatin_c, chLatin_c, chLatin_e, chLatin_s,
    chLatin_s, chLatin_o, chLatin_r, chNull
};


CurlNetAccessor::CurlNetAccessor()
{
}


CurlNetAccessor::~CurlNetAccessor()
{
}

BinInputStream*
CurlNetAccessor::makeNew(const XMLURL&  urlSource, const XMLNetHTTPInfo* httpInfo/*=0*/)
{
	// Just create a CurlURLInputStream
	// We defer any checking of the url type for curl in CurlURLInputStream
	CurlURLInputStream* retStrm =
		new (urlSource.getMemoryManager()) CurlURLInputStream(urlSource, httpInfo);
	return retStrm;            
}
