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
 * xmltooling/version.h
 * 
 * Library version macros and constants 
 */

#if !defined(__xmltooling_version_h__)
#define __xmltooling_version_h__

// This is all based on Xerces, on the theory it might be useful to
// support this kind of stuff in the future. If they ever yank some
// of this stuff, it can be copied into here.

#include <xercesc/util/XercesVersion.hpp>

// ---------------------------------------------------------------------------
// V E R S I O N   S P E C I F I C A T I O N

/**
 * MODIFY THESE NUMERIC VALUES TO COINCIDE WITH XMLTOOLING VERSION
 * AND DO NOT MODIFY ANYTHING ELSE IN THIS VERSION HEADER FILE
 */

#define XMLTOOLING_VERSION_MAJOR 1
#define XMLTOOLING_VERSION_MINOR 0
#define XMLTOOLING_VERSION_REVISION 0

/** DO NOT MODIFY BELOW THIS LINE */

/**
 * MAGIC THAT AUTOMATICALLY GENERATES THE FOLLOWING:
 *
 *	gXMLToolingVersionStr, gXMLToolingFullVersionStr, gXMLToolingMajVersion, gXMLToolingMinVersion, gXMLToolingRevision
 */

// ---------------------------------------------------------------------------
// V E R S I O N   I N F O R M A T I O N

// XMLTooling version strings; these particular macros cannot be used for
// conditional compilation as they are not numeric constants

#define XMLTOOLING_FULLVERSIONSTR INVK_CAT3_SEP_UNDERSCORE(XMLTOOLING_VERSION_MAJOR,XMLTOOLING_VERSION_MINOR,XMLTOOLING_VERSION_REVISION)
#define XMLTOOLING_FULLVERSIONDOT INVK_CAT3_SEP_PERIOD(XMLTOOLING_VERSION_MAJOR,XMLTOOLING_VERSION_MINOR,XMLTOOLING_VERSION_REVISION)
#define XMLTOOLING_FULLVERSIONNUM INVK_CAT3_SEP_NIL(XMLTOOLING_VERSION_MAJOR,XMLTOOLING_VERSION_MINOR,XMLTOOLING_VERSION_REVISION)
#define XMLTOOLING_VERSIONSTR     INVK_CAT2_SEP_UNDERSCORE(XMLTOOLING_VERSION_MAJOR,XMLTOOLING_VERSION_MINOR)

const char* const    gXMLToolingVersionStr = XMLTOOLING_VERSIONSTR;
const char* const    gXMLToolingFullVersionStr = XMLTOOLING_FULLVERSIONSTR;
const unsigned int   gXMLToolingMajVersion = XMLTOOLING_VERSION_MAJOR;
const unsigned int   gXMLToolingMinVersion = XMLTOOLING_VERSION_MINOR;
const unsigned int   gXMLToolingRevision   = XMLTOOLING_VERSION_REVISION;

// XMLTooling version numeric constants that can be used for conditional
// compilation purposes.

#define _XMLTOOLING_VERSION CALC_EXPANDED_FORM (XMLTOOLING_VERSION_MAJOR,XMLTOOLING_VERSION_MINOR,XMLTOOLING_VERSION_REVISION)

#endif // __xmltooling_version_h__
