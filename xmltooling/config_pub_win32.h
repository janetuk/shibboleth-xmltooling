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

/* Define if C++ compiler supports covariant virtual methods. */
#define HAVE_COVARIANT_RETURNS 1

/* Define to 1 if C++ compiler supports nullptr keyword. */
#if _MSC_VER >= 1600
# define HAVE_NULLPTR 1
#endif

/* Define to 1 if you have an STL implementation that supports useful string
   specialization. */
#define HAVE_GOOD_STL 1

/* Define to 1 if you have an STL implementation that supports
   std::iterator_traits. */
#define HAVE_ITERATOR_TRAITS 1

/* Define if log4shib library is used. */
#define XMLTOOLING_LOG4SHIB 1

/* Define if log4cpp library is used. */
/* #undef XMLTOOLING_LOG4CPP */

/* Define to 1 to disable XML-Security-dependent features. */
/* #undef XMLTOOLING_NO_XMLSEC */

/* Define if you wish to disable Xalan-dependent features. */
#define XSEC_NO_XALAN

#include <xercesc/util/XercesVersion.hpp>

#if (XERCES_VERSION_MAJOR >= 3)
# define XMLTOOLING_XERCESC_COMPLIANT_DOMLS     1
# define XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE  1
# define XMLTOOLING_XERCESC_64BITSAFE           1
#endif

/* Define to 1 if you have the `xsecsize_t' type. */
#define HAVE_XSECSIZE_T 1
