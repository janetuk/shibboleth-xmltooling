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
using std::istream;

GenericResponse::GenericResponse()
{
}

GenericResponse::~GenericResponse()
{
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
    std::string cookie(name);
    cookie = cookie + '=' + value;
    setResponseHeader("Set-Cookie", cookie.c_str());
}

long HTTPResponse::sendError(istream& inputStream)
{
    return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_ERROR);
}

long HTTPResponse::sendResponse(istream& inputStream)
{
    return sendResponse(inputStream, XMLTOOLING_HTTP_STATUS_OK);
}
