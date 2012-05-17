# Process this file with autoreconf
AC_PREREQ([2.50])
AC_INIT([xmltooling],[1.5.0],[https://issues.shibboleth.net/],[xmltooling])
AC_CONFIG_SRCDIR(xmltooling)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Docygen features
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(xmltooling, doxygen.cfg, doc/api)

AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],[Have GCC compile with symbols (Default = no)]),
    enable_debug=$enableval, enable_debug=no)

if test "$enable_debug" = "yes" ; then
    GCC_CFLAGS="$CFLAGS -g -D_DEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -g -D_DEBUG"
else
    GCC_CFLAGS="$CFLAGS -O2 -DNDEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -O2 -DNDEBUG"
fi

AC_CONFIG_HEADERS([config.h xmltooling/config_pub.h])
AC_CONFIG_FILES([xmltooling.spec])
AC_CONFIG_FILES([Makefile xmltooling/Makefile xmltoolingtest/Makefile schemas/Makefile doc/Makefile])

AC_PROG_CC([gcc gcc3 cc])
AC_PROG_CXX([g++ g++3 c++ CC])
AC_CANONICAL_HOST

if test "$GCC" = "yes" ; then
#    AC_HAVE_GCC_VERSION(4,0,0,0,
#        [
#        AC_DEFINE(GCC_HASCLASSVISIBILITY,1,
#            [Define to enable class visibility control in gcc.])
#        GCC_CFLAGS="$GCC_CFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
#        GCC_CXXFLAGS="$GCC_CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
#        ])
    CFLAGS="-Wall $GCC_CFLAGS"
    CXXFLAGS="-Wall $GCC_CXXFLAGS"
fi

# Fix for Sun Workshop compiler in debug mode, may be Sun case #6360993
# Also enables POSIX semantics for some functions.
case "${host_cpu}-${host_os}" in
    *solaris*)
	CFLAGS="$CFLAGS -D_POSIX_PTHREAD_SEMANTICS"
	CXXFLAGS="$CXXFLAGS -D_POSIX_PTHREAD_SEMANTICS"
	if test "$CXX" = "CC" ; then
		CXXFLAGS="$CXXFLAGS -Qoption ccfe -stabs=no%dfltlit+no%dflthlp"
	fi
    ;;
esac

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF([long])

# Checks for library functions.
AC_CHECK_FUNCS([strchr strdup strstr timegm strcasecmp])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_FUNC(dlclose, , [ AC_CHECK_LIB(dl, dlopen) ])

# checks for pthreads
ACX_PTHREAD([enable_threads="pthread"],[enable_threads="no"])
if test $enable_threads != "pthread"; then
    AC_MSG_ERROR([unable to find pthreads, currently this is required])
else
    AC_DEFINE([HAVE_PTHREAD],[1],[Define if you have POSIX threads libraries and header files.])
	AM_CONDITIONAL(BUILD_PTHREAD,test "$enable_threads" = "pthread")
    LIBS="$PTHREAD_LIBS $LIBS"
    CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
    CXXFLAGS="$PTHREAD_CFLAGS $CXXFLAGS"
fi

AC_CHECK_FUNCS([pthread_rwlock_init])

AC_LANG([C++])

# C++ requirements
AC_CXX_NAMESPACES
AC_CXX_REQUIRE_STL

# Boost
BOOST_REQUIRE
BOOST_BIND
BOOST_CONVERSION
BOOST_LAMBDA
BOOST_PTR_CONTAINER
BOOST_SMART_PTR
BOOST_STRING_ALGO
BOOST_TOKENIZER
CPPFLAGS="$BOOST_CPPFLAGS $CPPFLAGS"

# are covariant methods allowed?
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[]],
        [[
        class base { public: virtual base *GetPtr( void ) { return this; } };
        class derived: virtual public base { public: virtual derived *GetPtr( void ) { return this; } };
        ]])],
    [AC_DEFINE([HAVE_COVARIANT_RETURNS],[1],[Define to 1 if C++ compiler supports covariant virtual methods.])])

# is nullptr supported?
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[]],[[const char* ptr = nullptr;]])],
    [AC_DEFINE([HAVE_NULLPTR],[1],[Define to 1 if C++ compiler supports nullptr keyword.])])

# log4shib settings (favor this version over the log4cpp code)
AC_PATH_PROG(LOG4SHIB_CONFIG,log4shib-config)
AC_ARG_WITH(log4shib,
    AS_HELP_STRING([--with-log4shib=PATH],[where log4shib-config is installed]),
    [
    LOG4SHIB_CONFIG="${with_log4shib}"
    if ! test -f "${LOG4SHIB_CONFIG}" ; then
    	LOG4SHIB_CONFIG="${with_log4shib}/bin/log4shib-config"
    fi
    ])
if test -f "${LOG4SHIB_CONFIG}" ; then
    LIBS="`${LOG4SHIB_CONFIG} --libs` $LIBS"
    CPPFLAGS="`${LOG4SHIB_CONFIG} --cflags` $CPPFLAGS"
    AC_CHECK_HEADER([log4shib/CategoryStream.hh],,AC_MSG_ERROR([unable to find log4shib header files]))
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <log4shib/Category.hh>
#include <log4shib/CategoryStream.hh>]],
            [[log4shib::Category::getInstance("foo").errorStream() << log4shib::eol]])],
        [AC_DEFINE([XMLTOOLING_LOG4SHIB],[1],[Define to 1 if log4shib library is used.])],
        [AC_MSG_ERROR([unable to link with log4shib])])
else
    AC_MSG_WARN([log4shib-config not found, may need to use --with-log4shib option])
    AC_MSG_WARN([will look for original log4cpp library])
    
    # log4cpp settings
    AC_PATH_PROG(LOG4CPP_CONFIG,log4cpp-config)
    AC_ARG_WITH(log4cpp,
        AS_HELP_STRING([--with-log4cpp=PATH],[where log4cpp-config is installed]),
        [
        LOG4CPP_CONFIG="${with_log4cpp}"
        if ! test -f "${LOG4CPP_CONFIG}" ; then
        	LOG4CPP_CONFIG="${with_log4cpp}/bin/log4cpp-config"
        fi
        ])
    if test -f "${LOG4CPP_CONFIG}"; then
        AC_MSG_WARN([will try to use log4cpp, note that most non-Internet2 supplied versions are not thread-safe])
        LIBS="`${LOG4CPP_CONFIG} --libs` $LIBS"
        CPPFLAGS="`${LOG4CPP_CONFIG} --cflags` $CPPFLAGS"
        AC_CHECK_HEADER([log4cpp/CategoryStream.hh],,AC_MSG_ERROR([unable to find log4cpp header files]))
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>]],
                [[log4cpp::Category::getInstance("foo").errorStream() << log4cpp::eol]])],
            [AC_DEFINE([XMLTOOLING_LOG4CPP],[1],[Define to 1 if log4cpp library is used.])],
            [AC_MSG_ERROR([unable to link with log4cpp, need version 1.0 or later])])
	else
	    AC_MSG_ERROR([log4cpp-config not found, may need to use --with-log4cpp option])
	fi
fi

# Xerces settings
AC_ARG_WITH(xerces,
    AS_HELP_STRING([--with-xerces=PATH],[where xerces-c is installed]),,
    [with_xerces=/usr])
if test x_$with_xerces != x_/usr; then
    CPPFLAGS="-I${with_xerces}/include $CPPFLAGS"
    LIBS="-L${with_xerces}/lib -lxerces-c $LIBS"
else
    LIBS="-lxerces-c $LIBS"
fi

AC_CHECK_HEADER([xercesc/dom/DOM.hpp],,AC_MSG_ERROR([unable to find xerces header files]))
AC_MSG_CHECKING([Xerces version])
AC_PREPROC_IFELSE(
    [AC_LANG_PROGRAM([#include <xercesc/util/XercesVersion.hpp>],
[#if  _XERCES_VERSION != 20600
int i = 0;
#else
#error cannot use version 2.6.0
#endif])],
    [AC_MSG_RESULT(OK)],
    [AC_MSG_FAILURE([Xerces-C v2.6.0 has bugs that inhibit use with signed XML, please use a newer version])])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/util/PlatformUtils.hpp>]],
        [[xercesc::XMLPlatformUtils::Initialize()]])],
    ,[AC_MSG_ERROR([unable to link with Xerces])])

AC_MSG_CHECKING([whether Xerces is 64-bit clean])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/framework/MemBufInputSource.hpp>]],
        [[using namespace XERCES_CPP_NAMESPACE; XMLFilePos testvar;]])],
    [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XERCESC_64BITSAFE],[1],[Define to 1 if Xerces has a 64-bit-safe API.])],
    [AC_MSG_RESULT([no])])

AC_MSG_CHECKING([whether Xerces BinInputStream requires getContentType])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/util/BinMemInputStream.hpp>]],
        [[using namespace XERCES_CPP_NAMESPACE; XMLByte buf[1024]; BinMemInputStream in(buf,1024); in.getContentType();]])],
    [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XERCESC_INPUTSTREAM_HAS_CONTENTTYPE],[1],[Define to 1 if Xerces InputStream class requires getContentType.])],
    [AC_MSG_RESULT([no])])

AC_MSG_CHECKING([whether Xerces DOMLS API is compliant])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/dom/DOM.hpp>]],
        [[using namespace XERCES_CPP_NAMESPACE;
        DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(NULL);
        DOMLSSerializer *ls = ((DOMImplementationLS*)impl)->createLSSerializer();]])],
    [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XERCESC_COMPLIANT_DOMLS],[1],[Define to 1 if Xerces supports a compliant DOMLS API.])],
    [AC_MSG_RESULT([no])])

AC_MSG_CHECKING([whether Xerces has setIdAttribute(XMLCh*, bool)])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/dom/DOM.hpp>]],
        [[using namespace XERCES_CPP_NAMESPACE; DOMElement * elt; elt->setIdAttribute(NULL, false);]])],
    [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE],[1],[Define to 1 if Xerces DOM ID methods take extra parameter.])],
    [AC_MSG_RESULT([no])])

AC_MSG_CHECKING([whether Xerces XMLString::release(XMLByte**) exists])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[#include <xercesc/util/XMLString.hpp>]],
    [[using namespace XERCES_CPP_NAMESPACE; XMLByte* buf=NULL; XMLString::release(&buf);]])],
    [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XERCESC_HAS_XMLBYTE_RELEASE],[1],[Define to 1 if Xerces XMLString includes XMLByte release.])],
    [AC_MSG_RESULT([no])])

# XML-Security settings
AC_ARG_WITH(xmlsec,
    AS_HELP_STRING([--with-xmlsec=PATH], [where xmlsec is installed]),,
    [with_xmlsec=/usr])

if test x_$with_xmlsec != x_no; then

    # OpenSSL settings (need to do this first in case it's in a different spot from xmlsec)
    AC_ARG_WITH(openssl,
        AS_HELP_STRING([--with-openssl=PATH], [where openssl is installed]),
        [if test x_$with_openssl != x_/usr; then
            SSLFLAGS="-I${with_openssl}/include"
            SSLLIBS="-L${with_openssl}/lib -lcrypto -lssl"
        else
            SSLLIBS="-lcrypto -lssl"
        fi])
    
    if test "x$SSLLIBS" = "x" ; then
        AC_PATH_PROG(PKG_CONFIG, pkg-config)
        if test "x$PKG_CONFIG" != x && test "x$PKG_CONFIG" != "xno" ; then
            if pkg-config openssl ; then
                SSLLIBS="`$PKG_CONFIG --libs openssl`"
                SSLFLAGS="`$PKG_CONFIG --cflags openssl`"
            else
                AC_MSG_WARN([OpenSSL not supported by pkg-config, try --with-openssl instead])
            fi
        fi
    fi
    
    if test "x$SSLLIBS" = "x" ; then
        SSLLIBS="-lcrypto -lssl"
    fi
    
    AC_MSG_CHECKING(for OpenSSL cflags)
    AC_MSG_RESULT($SSLFLAGS)
    CPPFLAGS="$CPPFLAGS $SSLFLAGS"
    AC_MSG_CHECKING(for OpenSSL libraries)
    AC_MSG_RESULT($SSLLIBS)
    XMLSEC_LIBS="$XMLSEC_LIBS $SSLLIBS"

    # save and append master libs
    save_LIBS="$LIBS"
    LIBS="$XMLSEC_LIBS $LIBS"
    
    AC_CHECK_HEADER([openssl/pem.h],,AC_MSG_ERROR([unable to find openssl header files]))
    AC_MSG_CHECKING(for ERR_load_CRYPTO_string)
    AC_TRY_LINK_FUNC([ERR_load_CRYPTO_strings],,AC_MSG_ERROR([unable to link with openssl libraries]))
    AC_MSG_RESULT(yes)

    AC_CHECK_DECL(EVP_sha512,
        [AC_DEFINE([XMLTOOLING_OPENSSL_HAVE_SHA2],[1],[Define to 1 if OpenSSL supports the SHA-2 hash family.])],
		,[#include <openssl/evp.h>])
    AC_CHECK_DECL(EVP_PKEY_set1_EC_KEY,
        [AC_DEFINE([XMLTOOLING_OPENSSL_HAVE_EC],[1],[Define to 1 if OpenSSL has EC support.])],
        ,[#include <openssl/evp.h>])

    # restore master libs
    LIBS="$save_LIBS"

    if test x_$with_xmlsec != x_/usr; then
        CPPFLAGS="-I${with_xmlsec}/include $CPPFLAGS"
        XMLSEC_LIBS="-L${with_xmlsec}/lib -lxml-security-c $XMLSEC_LIBS"
    else
        XMLSEC_LIBS="-lxml-security-c $XMLSEC_LIBS"
    fi
    
    # save and append master libs
    save_LIBS="$LIBS"
    LIBS="$XMLSEC_LIBS $LIBS"
    
    AC_CHECK_HEADER([xsec/utils/XSECPlatformUtils.hpp],,AC_MSG_ERROR([unable to find XML-Security header files]))
    AC_MSG_CHECKING([XML-Security version])
    AC_PREPROC_IFELSE(
	    [AC_LANG_PROGRAM([#include <xsec/utils/XSECPlatformUtils.hpp>],
        [#if XSEC_VERSION_MAJOR > 1 || (XSEC_VERSION_MAJOR == 1 && XSEC_VERSION_MEDIUM > 3)
int i = 0;
#else
#error need version 1.4.0 or later
#endif])],
        [AC_MSG_RESULT(OK)],
        [AC_MSG_FAILURE([XML-Security version 1.4.0 or greater is required.])])
        
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/utils/XSECPlatformUtils.hpp>]],
            [[XSECPlatformUtils::Initialise()]])],,
        [AC_MSG_ERROR([unable to link with XML-Security])])
    AC_CHECK_TYPE([xsecsize_t],[AC_DEFINE([HAVE_XSECSIZE_T],[1],[Define to 1 if you have the 'xsecsize_t' type.])])

    AC_MSG_CHECKING([whether XML-Security-C has multiple CRL support])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/dsig/DSIGKeyInfoX509.hpp>]],
            [[DSIGKeyInfoX509* klist; klist->getX509CRLListSize();]])],
        [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XMLSEC_MULTIPLECRL],[1],[Define to 1 if XML-Security-C handles multiple CRLs.])],
        [AC_MSG_RESULT([no])])

    AC_MSG_CHECKING([whether XML-Security-C exposes the signature algorithm URI])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/dsig/DSIGSignature.hpp>]],
            [[DSIGSignature* sig; sig->getAlgorithmURI();]])],
        [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XMLSEC_SIGALGORITHM],[1],[Define to 1 if XML-Security-C exposes the signature algorithm URI.])],
        [AC_MSG_RESULT([no])])

    AC_MSG_CHECKING([whether XML-Security-C includes ECC support])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/dsig/DSIGKeyInfoValue.hpp>]],
            [[DSIGKeyInfoValue* info; info->getECNamedCurve();]])],
        [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XMLSEC_ECC],[1],[Define to 1 if XML-Security-C includes ECC support.])],
        [AC_MSG_RESULT([no])])

    AC_MSG_CHECKING([whether XML-Security-C includes C14N 1.1 support])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/dsig/DSIGConstants.hpp>]],
            [[transformType t = TRANSFORM_C14N11;]])],
        [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XMLSEC_C14N11],[1],[Define to 1 if XML-Security-C includes C14N 1.1 support.])],
        [AC_MSG_RESULT([no])])

    AC_MSG_CHECKING([whether XML-Security-C includes debug logging support])
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[#include <xsec/utils/XSECPlatformUtils.hpp>]],
            [[XSECPlatformUtils::SetReferenceLoggingSink(NULL)]])],
        [AC_MSG_RESULT([yes])AC_DEFINE([XMLTOOLING_XMLSEC_DEBUGLOGGING],[1],[Define to 1 if XML-Security-C includes debug logging support.])],
        [AC_MSG_RESULT([no])])

    # restore master libs
    LIBS="$save_LIBS"

    # libcurl settings
    AC_PATH_PROG(CURL_CONFIG,curl-config)
    AC_ARG_WITH(curl,
        AS_HELP_STRING([--with-curl=PATH], [where curl-config is installed]),
        [
        CURL_CONFIG="${with_curl}"
        if ! test -f "${CURL_CONFIG}" ; then
            CURL_CONFIG="${with_curl}/bin/curl-config"
        fi
        ])
    if test -f "${CURL_CONFIG}" ; then
        CPPFLAGS="`${CURL_CONFIG} --cflags` $CPPFLAGS"
        XMLSEC_LIBS="`${CURL_CONFIG} --libs` $XMLSEC_LIBS"
    else
        AC_MSG_ERROR([curl-config not found, may need to use --with-curl option])
    fi

    # save and append master libs
    save_LIBS="$LIBS"
    LIBS="$XMLSEC_LIBS $LIBS"
	
    AC_CHECK_HEADER([curl/curl.h],,AC_MSG_ERROR([unable to find libcurl header files]))
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <curl/curl.h>]],
            [[curl_global_init(CURL_GLOBAL_ALL)]])],,
        [AC_MSG_ERROR([unable to link with libcurl])])
    AC_MSG_CHECKING([for CURLOPT_SSL_CTX_FUNCTION in curl.h])
    AC_EGREP_HEADER([CURLOPT_SSL_CTX_FUNCTION], [curl/curl.h],
        [AC_MSG_RESULT(yes)],
        [AC_MSG_ERROR([need libcurl that supports CURLOPT_SSL_CTX_FUNCTION])])
    AC_CHECK_TYPE([curl_off_t],[AC_DEFINE([HAVE_CURL_OFF_T],[1],[Define to 1 if you have the 'curl_off_t' type.])],,[[#include <curl/curl.h>]])
    
    # restore master libs
    LIBS="$save_LIBS"
    
    AC_SUBST(XMLSEC_LIBS)
else
    AC_MSG_WARN([xmlsec disabled, building without signature/encryption support])
    AC_DEFINE([XMLTOOLING_NO_XMLSEC],[1],[Define to 1 if you wish to disable XML-Security-dependent features.])
fi
AM_CONDITIONAL(BUILD_XMLSEC,test x_$with_xmlsec != x_no)

# Does the STL in use help or screw us?
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <string>]],[[std::basic_string<unsigned short> foo; foo=foo+(unsigned short)65]])],
    [AC_DEFINE([HAVE_GOOD_STL],[1],[Define to 1 if you have an STL implementation that supports useful string specialization.])],[])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <vector>]],[[std::iterator_traits<std::vector<int>::iterator>::value_type foo=0]])],
    [AC_DEFINE([HAVE_ITERATOR_TRAITS],[1],[Define to 1 if you have an STL implementation that supports std::iterator_traits.])],[])

# Check for unit test support
CXXTEST="/usr/bin/cxxtestgen.pl"
CXXTESTFLAGS=""
AC_ARG_WITH(cxxtest,
    AS_HELP_STRING([--with-cxxtest=PATH], [where cxxtest is installed]),
    [if test x_$with_cxxtest != x_/usr; then
        CXXTEST="${with_cxxtest}/cxxtestgen.pl"
        CXXTESTFLAGS="-I${with_cxxtest}"
    fi])
if ! test -f "${CXXTEST}"; then
    AC_MSG_WARN([cxxtestgen not found, won't build unit tests])
fi

AC_SUBST(CXXTEST)
AC_SUBST(CXXTESTFLAGS)
AM_CONDITIONAL(BUILD_UNITTEST,test -f ${CXXTEST})

AX_CREATE_PKGCONFIG_INFO(,,[$XMLSEC_LIBS -lxmltooling],[OpenSAML XMLTooling-C library])

LIBTOOL="$LIBTOOL --silent"

# output packaging and makefiles
AC_OUTPUT
