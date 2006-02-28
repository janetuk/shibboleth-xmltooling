AC_PREREQ([2.50])
AC_INIT([xmltooling], [1.0], [mace-opensaml-users@internet2.edu], [xmltooling])
AM_CONFIG_HEADER(config.h)
AM_CONFIG_HEADER(xmltooling/config_pub.h)
AM_INIT_AUTOMAKE([xmltooling], [1.0])

sinclude(acx_pthread.m4)

AC_ARG_ENABLE(debug,
    AC_HELP_STRING(--enable-debug, [Have GCC compile with symbols (Default = no)]),
    enable_debug=$enableval, enable_debug=no)

if test "$enable_debug" = "yes" ; then
    GCC_CFLAGS="$CFLAGS -g -D_DEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -g -D_DEBUG"
else
    GCC_CFLAGS="$CFLAGS -O2 -DNDEBUG"
    GCC_CXXFLAGS="$CXXFLAGS -O2 -DNDEBUG"
fi

AC_PROG_CC([gcc gcc3 cc])
AC_PROG_CXX([g++ g++3 c++ CC])

if test "$GCC" = "yes" ; then
    CFLAGS="$GCC_CFLAGS"
    CXXFLAGS="$GCC_CXXFLAGS"
fi

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_LANG(C++)

# C++ requirements
AC_CXX_REQUIRE_STL
AC_CXX_NAMESPACES

AC_LANG(C)

# checks for pthreads
ACX_PTHREAD([enable_threads="pthread"],[enable_threads="no"])
if test $enable_threads != "pthread"; then
    AC_MSG_ERROR([unable to find pthreads, currently this is required])
else
    AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
    LIBS="$PTHREAD_LIBS $LIBS"
    CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
    CXXFLAGS="$PTHREAD_CFLAGS $CXXFLAGS"
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_STRUCT_TIMEZONE

# Checks for library functions.
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([strchr strdup strstr gmtime_r timegm])
AC_CHECK_HEADERS([dlfcn.h])

# OpenSSL settings
AC_PATH_PROG(PKG_CONFIG, pkg-config)
if test "x$PKG_CONFIG" = x || test "x$PKG_CONFIG" = "xno" ; then
    AC_ARG_WITH(openssl, 
            AC_HELP_STRING([--with-openssl=PATH], [where openssl is installed]),
            [if test x_$with_openssl != x_/usr; then
	    SSLLIBS="-L${with_openssl}/lib"
	    SSLFLAGS="-I${with_openssl}/include"
            fi])
    SSLLIBS="$SSLLIBS -lssl -lcrypto"
else
    SSLLIBS="`$PKG_CONFIG --libs openssl`"
    SSLFLAGS="`$PKG_CONFIG --cflags openssl`"
fi
AC_MSG_CHECKING(for OpenSSL libraries)
AC_MSG_RESULT($SSLLIBS)
LIBS="$LIBS $SSLLIBS"
AC_MSG_CHECKING(for OpenSSL cflags)
AC_MSG_RESULT($SSLFLAGS)
CPPFLAGS="$SSLFLAGS $CPPFLAGS"

AC_CHECK_HEADER([openssl/ssl.h],,
                AC_MSG_ERROR([unable to find openssl header files]))
AC_MSG_CHECKING(for ERR_load_CRYPTO_string)
AC_TRY_LINK_FUNC([ERR_load_CRYPTO_strings],,
             AC_MSG_ERROR([unable to link with openssl libraries]))
AC_MSG_RESULT(yes)
AC_MSG_CHECKING(for SSL_library_init)
AC_TRY_LINK_FUNC([SSL_library_init],, 
             AC_MSG_ERROR([unable to link with openssl libraries]))
AC_MSG_RESULT(yes)

# libcurl settings
AC_PATH_PROG(CURL_CONFIG,curl-config)
AC_ARG_WITH(curl,
            AC_HELP_STRING([--with-curl=PATH], [where libcurl is installed]),
            [CURL_CONFIG="${with_curl}/bin/curl-config"])
if test -f "${CURL_CONFIG}"; then
    LDFLAGS="`${CURL_CONFIG} --libs` $LDFLAGS"
    CPPFLAGS="`${CURL_CONFIG} --cflags` $CPPFLAGS"
else
    AC_MSG_WARN([curl-config not found, guessing at libcurl build settings])
fi
AC_CHECK_HEADER([curl/curl.h],,AC_MSG_ERROR([unable to find libcurl header files]))
AC_CHECK_LIB([curl],[curl_global_init],,AC_MSG_ERROR([unable to link with libcurl]))
AC_MSG_CHECKING([for CURLOPT_SSL_CTX_FUNCTION in curl.h])
AC_EGREP_HEADER([CURLOPT_SSL_CTX_FUNCTION], [curl/curl.h],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_ERROR([need libcurl that supports CURLOPT_SSL_CTX_FUNCTION])])

AC_LANG(C++)

# log4cpp settings
AC_PATH_PROG(LOG4CPP_CONFIG,log4cpp-config)
AC_ARG_WITH(log4cpp,
            AC_HELP_STRING([--with-log4cpp=PATH], [where log4cpp is installed]),
            [LOG4CPP_CONFIG="${with_log4cpp}/bin/log4cpp-config"])
if test -f "${LOG4CPP_CONFIG}"; then
    LDFLAGS="`${LOG4CPP_CONFIG} --libs` $LDFLAGS"
    CPPFLAGS="`${LOG4CPP_CONFIG} --cflags` $CPPFLAGS"
else
    AC_MSG_WARN([log4cpp-config not found, guessing at log4cpp build settings])
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
AC_CHECK_HEADER([xercesc/dom/DOM.hpp],,
                AC_MSG_ERROR([unable to find xerces header files]))
AC_MSG_CHECKING([Xerces version])
AC_PREPROC_IFELSE(
	[AC_LANG_PROGRAM([#include <xercesc/util/XercesVersion.hpp>],
[#if  _XERCES_VERSION != 20600
int i = 0;
#else
#error cannot use version 2.6.0
#endif])],
	[AC_MSG_RESULT(OK)],
	[AC_MSG_FAILURE([Xerces-C v2.6.0 has bugs that inhibit use with signed XML, please use special 2.6.1 tarball provided by Shibboleth Project])])
AC_TRY_LINK(
        [#include <xercesc/util/PlatformUtils.hpp>],
        [xercesc::XMLPlatformUtils::Initialize()],
        [AC_DEFINE(HAVE_LIBXERCESC,1,[Define if Xerces-C library was found])],
        [AC_MSG_ERROR([unable to link with Xerces])])


# XML-Security settings
AC_ARG_WITH(xmlsec,
            AC_HELP_STRING([--with-xmlsec=PATH], [where xmlsec is installed]),
            [if test x_$with_xmlsec != x_/usr; then
                LDFLAGS="-L${with_xmlsec}/lib $LDFLAGS"
                CPPFLAGS="-I${with_xmlsec}/include $CPPFLAGS"
            fi])
LIBS="-lxml-security-c $LIBS"
AC_CHECK_HEADER([xsec/utils/XSECPlatformUtils.hpp],,AC_MSG_ERROR([unable to find xsec header files]))
AC_TRY_LINK(
        [#include <xsec/utils/XSECPlatformUtils.hpp>],
        [XSECPlatformUtils::Initialise()],
        [AC_DEFINE(HAVE_LIBXSEC,1,[Define if XML-Sec library was found])],
        [AC_MSG_ERROR([unable to link with XML-Sec])])

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
            [if test x_$with_xerces != x_/usr; then
                CXXTEST="${with_cxxtest}/cxxtestgen.pl"
                CXXTESTFLAGS="-I${with_cxxtest}"
            fi])
if ! test -f "${CXXTEST}"; then
    AC_MSG_WARN([cxxtestgen not found, won't build unit tests])
fi

AC_SUBST(CXXTEST)
AC_SUBST(CXXTESTFLAGS)
AM_CONDITIONAL(BUILD_UNITTEST,test -f ${CXXTEST})

# output makefiles
AC_OUTPUT(Makefile xmltooling/Makefile schemas/Makefile)