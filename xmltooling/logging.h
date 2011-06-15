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
 * @file xmltooling/logging.h
 * 
 * Assist with logging portability.
 */
 
#ifndef __xmltooling_logging_h__
#define __xmltooling_logging_h__

#include <xmltooling/base.h>

#if defined(XMLTOOLING_LOG4SHIB)
# include <log4shib/Category.hh>
# include <log4shib/CategoryStream.hh>
namespace xmltooling {
    namespace logging = log4shib;
};
#elif defined(XMLTOOLING_LOG4CPP)
# include <log4cpp/Category.hh>
# include <log4cpp/CategoryStream.hh>
namespace xmltooling {
    namespace logging = log4cpp;
};
#else
# error "Supported logging library not available."
#endif

#endif /* __xmltooling_logging_h__ */
