AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libxmltooling.la

libxmltoolingincludedir = \
	$(includedir)/xmltooling \
	$(includedir)/xmltooling/util
	
libxmltoolinginclude_HEADERS = \
	base.h \
	config_pub.h \
	exceptions.h \
	ILockable.h \
	Namespace.h \
	QName.h \
	unicode.h \
	version.h \
	XMLObject.h \
	XMLObjectBuilder.h \
	XMLToolingConfig.h \
	util/NDC.h \
	util/ParserPool.h \
	util/XMLConstants.h \
	util/XMLHelper.h

noinst_HEADERS = \
	internal.h

libxmltooling_la_SOURCES = \
	Namespace.cpp \
	QName.cpp \
	unicode.cpp \
	XMLToolingConfig.cpp \
	util/NDC.cpp \
	util/ParserPool.cpp \
	util/XMLConstants.cpp \
	util/XMLHelper.cpp

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libxmltooling_la_LDFLAGS = -version-info 1:0:0

install-exec-hook:
	for la in $(lib_LTLIBRARIES) ; do rm -f $(DESTDIR)$(libdir)/$$la ; done

EXTRA_DIST = xmltooling.vcproj config_pub.h.in config_pub_win32.h
