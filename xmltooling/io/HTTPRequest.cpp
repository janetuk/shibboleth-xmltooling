/*
 *  Copyright 2001-2007 Internet2
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
 * HTTPRequest.cpp
 * 
 * Interface to HTTP requests  
 */

#include "internal.h"
#include "HTTPRequest.h"

using namespace xmltooling;
using namespace std;

const char* HTTPRequest::getCookie(const char* name) const
{
    if (m_cookieMap.empty()) {
        string cookies=getHeader("Cookie");

        string::size_type pos=0,cname,namelen,val,vallen;
        while (pos !=string::npos && pos < cookies.length()) {
            while (isspace(cookies[pos])) pos++;
            cname=pos;
            pos=cookies.find_first_of("=",pos);
            if (pos == string::npos)
                break;
            namelen=pos-cname;
            pos++;
            if (pos==cookies.length())
                break;
            val=pos;
            pos=cookies.find_first_of(";",pos);
            if (pos != string::npos) {
                vallen=pos-val;
                pos++;
                m_cookieMap.insert(make_pair(cookies.substr(cname,namelen),cookies.substr(val,vallen)));
            }
            else
                m_cookieMap.insert(make_pair(cookies.substr(cname,namelen),cookies.substr(val)));
        }
    }
    map<string,string>::const_iterator lookup=m_cookieMap.find(name);
    return (lookup==m_cookieMap.end()) ? NULL : lookup->second.c_str();
}
