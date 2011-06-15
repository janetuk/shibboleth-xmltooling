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
 * version.cpp
 * 
 * Library version macros and constants.
 */

#include "internal.h"
#include "version.h"

XMLTOOL_API const char* const    gXMLToolingVersionStr = XMLTOOLING_VERSIONSTR;
XMLTOOL_API const char* const    gXMLToolingFullVersionStr = XMLTOOLING_FULLVERSIONSTR;
XMLTOOL_API const char* const    gXMLToolingDotVersionStr = XMLTOOLING_FULLVERSIONDOT;
XMLTOOL_API const unsigned int   gXMLToolingMajVersion = XMLTOOLING_VERSION_MAJOR;
XMLTOOL_API const unsigned int   gXMLToolingMinVersion = XMLTOOLING_VERSION_MINOR;
XMLTOOL_API const unsigned int   gXMLToolingRevision   = XMLTOOLING_VERSION_REVISION;
