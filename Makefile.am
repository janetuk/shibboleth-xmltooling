AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

dist-hook:
	rm -rf `find $(distdir)/xmltoolingtest/data -name .svn`
	rm -rf `find $(distdir)/doc/api -name .svn`


SUBDIRS = doc schemas @PACKAGE_NAME@ xmltoolingtest

if DX_COND_doc
all-local: doxygen-doc
else
all-local:
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

GENFILES = @PACKAGE_NAME@.spec pkginfo Portfile

EXTRA_DIST = $(DX_CONFIG) \
        cpp-@PACKAGE_NAME@.sln \
        config_win32.h \
        $(GENFILES) \
        $(GENFILES:%=%.in) \
        postinstall
