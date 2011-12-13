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
 * HTTPRequest.cpp
 * 
 * Interface to HTTP requests.
 */

#include "internal.h"
#include "HTTPRequest.h"

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

using namespace xmltooling;
using namespace boost;
using namespace std;

GenericRequest::GenericRequest()
{
}

GenericRequest::~GenericRequest()
{
}

HTTPRequest::HTTPRequest()
{
}

HTTPRequest::~HTTPRequest()
{
}

bool HTTPRequest::isSecure() const
{
    return strcmp(getScheme(),"https")==0;
}

namespace {
    void handle_cookie_fn(map<string,string>& cookieMap, vector<string>& nvpair, const string& s) {
        nvpair.clear();
        split(nvpair, s, is_any_of("="));
        if (nvpair.size() == 2) {
            trim(nvpair[0]);
            cookieMap[nvpair[0]] = nvpair[1];
        }
    }
}

const char* HTTPRequest::getCookie(const char* name) const
{
    if (m_cookieMap.empty()) {
        string cookies=getHeader("Cookie");
        vector<string> nvpair;
        tokenizer< char_separator<char> > nvpairs(cookies, char_separator<char>(";"));
        for_each(nvpairs.begin(), nvpairs.end(), boost::bind(handle_cookie_fn, boost::ref(m_cookieMap), boost::ref(nvpair), _1));
    }
    map<string,string>::const_iterator lookup=m_cookieMap.find(name);
    return (lookup==m_cookieMap.end()) ? nullptr : lookup->second.c_str();
}
