/* Define if C++ compiler supports covariant virtual methods. */
#define HAVE_COVARIANT_RETURNS 1

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
