AC_PREREQ([2.50])
AC_INIT([xmltooling], [1.0], [mace-opensaml-users@internet2.edu], [xmltooling])
AM_CONFIG_HEADER(config.h)
AM_CONFIG_HEADER(xmltooling/config_pub.h)
AM_INIT_AUTOMAKE([xmltooling], [1.0])

sinclude(doxygen.m4)
sinclude(acx_pthread.m4)

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
    AC_HELP_STRING(--enable-debug, [Have GCC compile with symbols (Default = no)]),
    enable_debug=$enableval, enable_debug=no)

if test "$enable_debug" = "yes" ; then
    GCC_CFLAGS="$CFLAGS -Wall -g -D_DEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -Wall -g -D_DEBUG"
else
    GCC_CFLAGS="$CFLAGS -Wall -O2 -DNDEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -Wall -O2 -DNDEBUG"
fi

AC_PROG_CC([gcc gcc3 cc])
AC_PROG_CXX([g++ g++3 c++ CC])

if test "$GCC" = "yes" ; then
#    AC_HAVE_GCC_VERSION(4,0,0,0,
#        [
#        AC_DEFINE(GCC_HASCLASSVISIBILITY,1,
#            [Define to enable class visibility control in gcc.])
#        GCC_CFLAGS="$GCC_CFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
#        GCC_CXXFLAGS="$GCC_CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
#        ])
    CFLAGS="$GCC_CFLAGS"
    CXXFLAGS="$GCC_CXXFLAGS"
fi

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strchr strdup strstr timegm strcasecmp])
AC_CHECK_HEADERS([dlfcn.h])
AC_CHECK_FUNC(dlclose, , [ AC_CHECK_LIB(dl, dlopen) ])

# checks for pthreads
ACX_PTHREAD([enable_threads="pthread"],[enable_threads="no"])
if test $enable_threads != "pthread"; then
    AC_MSG_ERROR([unable to find pthreads, currently this is required])
else
    AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
	AM_CONDITIONAL(BUILD_PTHREAD,test "$enable_threads" = "pthread")
    LIBS="$PTHREAD_LIBS $LIBS"
    CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
    CXXFLAGS="$PTHREAD_CFLAGS $CXXFLAGS"
fi

# OpenSSL settings
AC_ARG_WITH(openssl,
    AC_HELP_STRING([--with-openssl=PATH], [where openssl is installed]),
    [if test x_$with_openssl != x_/usr; then
        SSLLIBS="-L${with_openssl}/lib -lcrypto -lssl"
        SSLFLAGS="-I${with_openssl}/include"
    fi])

if test "x$SSLLIBS" = "x" ; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config)
    if test "x$PKG_CONFIG" != x && test "x$PKG_CONFIG" != "xno" ; then
        if pkg-config openssl ; then
            SSLLIBS="`$PKG_CONFIG --libs openssl`"
            SSLFLAGS="`$PKG_CONFIG --cflags openssl`"
        else
            AC_MSG_ERROR([OpenSSL not supported by pkg-config, try --with-openssl instead])
        fi
    fi
fi

if test "x$SSLLIBS" = "x" ; then
    SSLLIBS="-lcrypto -lssl"
fi

AC_MSG_CHECKING(for OpenSSL libraries)
AC_MSG_RESULT($SSLLIBS)
LIBS="$LIBS $SSLLIBS"
AC_MSG_CHECKING(for OpenSSL cflags)
AC_MSG_RESULT($SSLFLAGS)
CPPFLAGS="$SSLFLAGS $CPPFLAGS"

AC_CHECK_HEADER([openssl/pem.h],,
                AC_MSG_ERROR([unable to find openssl header files]))
AC_MSG_CHECKING(for ERR_load_CRYPTO_string)
AC_TRY_LINK_FUNC([ERR_load_CRYPTO_strings],,
             AC_MSG_ERROR([unable to link with openssl libraries]))
AC_MSG_RESULT(yes)

# libcurl settings
AC_PATH_PROG(CURL_CONFIG,curl-config)
AC_ARG_WITH(curl,
    AC_HELP_STRING([--with-curl=PATH], [where curl-config is installed]),
    [
    CURL_CONFIG="${with_curl}"
    if ! test -f "${CURL_CONFIG}" ; then
    	CURL_CONFIG="${with_curl}/bin/curl-config"
    fi
    ])
if test -f "${CURL_CONFIG}" ; then
    LDFLAGS="`${CURL_CONFIG} --libs` $LDFLAGS"
    CPPFLAGS="`${CURL_CONFIG} --cflags` $CPPFLAGS"
else
    AC_MSG_ERROR([curl-config not found, may need to use --with-curl option])
fi
AC_CHECK_HEADER([curl/curl.h],,AC_MSG_ERROR([unable to find libcurl header files]))
AC_CHECK_LIB([curl],[curl_global_init],,AC_MSG_ERROR([unable to link with libcurl]))
AC_MSG_CHECKING([for CURLOPT_SSL_CTX_FUNCTION in curl.h])
AC_EGREP_HEADER([CURLOPT_SSL_CTX_FUNCTION], [curl/curl.h],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_ERROR([need libcurl that supports CURLOPT_SSL_CTX_FUNCTION])])


AC_LANG(C++)

# C++ requirements
AC_CXX_REQUIRE_STL
AC_CXX_NAMESPACES

# are covariant methods allowed?
AC_TRY_LINK(
     [ class base { public: virtual base *GetPtr( void ) { return this; } }; ],
     [ class derived: virtual public base { public: virtual derived *GetPtr( void ) { return this; } }; ],
     [AC_DEFINE(HAVE_COVARIANT_RETURNS,1,[Define if C++ compiler supports covariant virtual methods.])])

# log4cpp settings
AC_PATH_PROG(LOG4CPP_CONFIG,log4cpp-config)
AC_ARG_WITH(log4cpp,
    AC_HELP_STRING([--with-log4cpp=PATH], [where log4cpp-config is installed]),
    [
    LOG4CPP_CONFIG="${with_log4cpp}"
    if ! test -f "${LOG4CPP_CONFIG}" ; then
    	LOG4CPP_CONFIG="${with_log4cpp}/bin/log4cpp-config"
    fi
    ])
if test -f "${LOG4CPP_CONFIG}"; then
    LDFLAGS="`${LOG4CPP_CONFIG} --libs` $LDFLAGS"
    CPPFLAGS="`${LOG4CPP_CONFIG} --cflags` $CPPFLAGS"
else
    AC_MSG_ERROR([log4cpp-config not found, may need to use --with-log4cpp option])
    LIBS="-llog4cpp $LIBS"
fi
AC_CHECK_HEADER([log4cpp/Category.hh],,AC_MSG_ERROR([unable to find log4cpp header files]))
AC_CHECK_HEADER([log4cpp/PropertyConfigurator.hh],,AC_MSG_ERROR([you need at least log4cpp 0.3.x]))
AC_TRY_LINK(
	[#include <log4cpp/Category.hh>],
	[log4cpp::Category::getInstance("foo")],
	[AC_DEFINE(HAVE_LIBLOG4CPP,1,[Define if log4cpp library was found])],
	[AC_MSG_ERROR([unable to link with log4cpp])])

# Xerces settings
AC_ARG_WITH(xerces, 
            AC_HELP_STRING([--with-xerces=PATH], [where xerces-c is installed]),
            [if test x_$with_xerces != x_/usr; then
                LDFLAGS="-L${with_xerces}/lib $LDFLAGS"
                CPPFLAGS="-I${with_xerces}/include $CPPFLAGS"
            fi])
LIBS="-lxerces-c $LIBS"
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
AC_TRY_LINK(
        [#include <xercesc/util/PlatformUtils.hpp>],
        [xercesc::XMLPlatformUtils::Initialize()],
        [AC_DEFINE(HAVE_LIBXERCESC,1,[Define if Xerces-C library was found])],
        [AC_MSG_ERROR([unable to link with Xerces])])


# XML-Security settings
AC_ARG_WITH(xmlsec,
            AC_HELP_STRING([--with-xmlsec=PATH], [where xmlsec is installed]),,
            [with_xmlsec=/usr])

if test x_$with_xmlsec != x_no; then
    if test x_$with_xmlsec != x_/usr; then
        LDFLAGS="-L${with_xmlsec}/lib $LDFLAGS"
        CPPFLAGS="-I${with_xmlsec}/include $CPPFLAGS"
    fi        
    LIBS="-lxml-security-c $LIBS"
    AC_CHECK_HEADER([xsec/utils/XSECPlatformUtils.hpp],,AC_MSG_ERROR([unable to find XML-Security header files]))
    AC_MSG_CHECKING([XML-Security version])
    AC_PREPROC_IFELSE(
	    [AC_LANG_PROGRAM([#include <xsec/utils/XSECPlatformUtils.hpp>],
        [#if XSEC_VERSION_MAJOR > 1 || XSEC_VERSION_MEDIUM >2 
int i = 0;
#else
#error need version 1.3 or later
#endif])],
        [AC_MSG_RESULT(OK)],
        [AC_MSG_FAILURE([XML-Security version 1.3 or greater is required.])])
    AC_TRY_LINK(
            [#include <xsec/utils/XSECPlatformUtils.hpp>],
            [XSECPlatformUtils::Initialise()],,
            [AC_MSG_ERROR([unable to link with XML-Sec])])
else
    AC_MSG_WARN([xmlsec disabled, building without signature/encryption support])
    AC_DEFINE(XMLTOOLING_NO_XMLSEC,1,
        [Define if you wish to disable XML-Security-dependent features.])
fi
AM_CONDITIONAL(BUILD_XMLSEC,test x_$with_xmlsec != x_no)

# Does the STL in use help or screw us?
AC_TRY_LINK(
        [#include <string>],
        [std::basic_string<unsigned short> foo; foo=foo+(unsigned short)65],
        [AC_DEFINE(HAVE_GOOD_STL,1,
            [Define if you have an STL implementation that supports useful string specialization.])],
        )

# Check for unit test support
CXXTEST="/usr/bin/cxxtestgen.pl"
CXXTESTFLAGS=""
AC_ARG_WITH(cxxtest,
            AC_HELP_STRING([--with-cxxtest=PATH], [where cxxtest is installed]),
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

LIBTOOL="$LIBTOOL --silent"

# output makefiles
AC_OUTPUT(Makefile xmltooling/Makefile xmltoolingtest/Makefile schemas/Makefile)
