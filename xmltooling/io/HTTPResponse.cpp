/*
 *  Copyright 2009 Internet2
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
 * HTTPResponse.cpp
 * 
 * Interface to HTTP responses.
 */

#include "internal.h"
#include "HTTPResponse.h"

using namespace xmltooling;
using namespace std;

GenericResponse::GenericResponse()
{
}

GenericResponse::~GenericResponse()
{
}

vector<string> HTTPResponse::m_allowedSchemes;

vector<string>& HTTPResponse::getAllowedSchemes()
{
    return m_allowedSchemes;
}

void HTTPResponse::sanitizeURL(const char* url)
{
    const char* ch;
    for (ch=url; *ch; ++ch) {
        if (iscntrl(*ch))
            throw IOException("URL contained a control character.");
    }

    ch = strchr(url, ':');
    if (!ch)
        throw IOException("URL is malformed.");
    string s(url, ch - url);
    for (vector<string>::const_iterator i = m_allowedSchemes.begin(); i != m_allowedSchemes.end(); ++i) {
#ifdef HAVE_STRCASECMP
        if (!strcasecmp(s.c_str(), i->c_str()))
#else
        if (!stricmp(s.c_str(), i->c_str()))
#endif
            return;
    }

    throw IOException("URL contains invalid scheme ($1).", params(1, s.c_str()));
}

HTTPResponse::HTTPResponse()
{
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::setContentType(const char* type)
{
    setResponseHeader("Content-Type", type);
}

void HTTPResponse::setCookie(const char* name, const char* value)
{
    string cookie(name);
    cookie = cookie + '=' + value;
    setResponseHeader("Set-Cookie", cookie.c_str());
}

void HTTPResponse::setResponseHeader(const char* name, const char* value)
{
    for (const char* ch=name; *ch; ++ch) {
        if (iscntrl(*ch))
            throw IOException("Response header name contained a control character.");
    }

    for (const char* ch=value; *ch; ++ch) {
        if (iscntrl(*ch))
            throw IOException("Value for response header ($1) contained a control character.", params(1,name));
    }
}

long HTTPResponse::sendRedirect(const char* url)
{
    sanitizeURL(url);
    return XMLTOOLING_HTTP_STATUS_MOVED;
}

long HTTPResponse::sendError(istream& inputStream)
{
    return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_ERROR);
}

long HTTPResponse::sendResponse(istream& inputStream)
{
    return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_OK);
}
