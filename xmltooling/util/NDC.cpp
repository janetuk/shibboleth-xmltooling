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
 * NDC.cpp
 * 
 * Diagnostic context for logging 
 */

#include "internal.h"
#include "logging.h"
#include "util/NDC.h"

#if defined(XMLTOOLING_LOG4SHIB)
# include <log4shib/NDC.hh>
#elif defined(XMLTOOLING_LOG4CPP)
# include <log4cpp/NDC.hh>
#endif

using namespace xmltooling;

NDC::NDC(const char* context)
{
    logging::NDC::push(context);
}

NDC::NDC(const std::string& context)
{
    logging::NDC::push(context);
}

NDC::~NDC()
{
    logging::NDC::pop();
}
