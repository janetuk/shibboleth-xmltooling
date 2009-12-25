AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I .

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

dist-hook:
	rm -rf `find $(distdir)/@PACKAGE_NAME@test/data -name .svn`
	rm -rf `find $(distdir)/doc/api -name .svn`


SUBDIRS = doc schemas @PACKAGE_NAME@ @PACKAGE_NAME@test

if DX_COND_doc
all-local: doxygen-doc
else
all-local:
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

GENFILES = @PACKAGE_NAME@.spec @PACKAGE_NAME@.pc pkginfo Portfile

EXTRA_DIST = $(DX_CONFIG) \
        cpp-@PACKAGE_NAME@.sln \
        acx_pthread.m4 \
        acinclude.m4 \
        ax_create_pkgconfig_info.m4 \
        libtool.m4 \
        doxygen.m4 \
        depcomp \
        config_win32.h \
        $(GENFILES) \
        $(GENFILES:%=%.in) \
        depend \
        postinstall
