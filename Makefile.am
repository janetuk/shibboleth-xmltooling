AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

DISTCLEANFILES = \
	@PACKAGE_NAME@.pc \
	@PACKAGE_NAME@.pc.in \
	@PACKAGE_NAME@-uninstalled.pc \
	@PACKAGE_NAME@-uninstalled.sh

SUBDIRS = doc schemas @PACKAGE_NAME@ xmltoolingtest

if DX_COND_doc
all-local: doxygen-doc
else
all-local:
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PACKAGE_NAME@.pc

GENFILES = @PACKAGE_NAME@.spec

EXTRA_DIST = $(DX_CONFIG) \
	cpp-@PACKAGE_NAME@.sln \
	config_win32.h \
	$(GENFILES) \
	$(GENFILES:%=%.in)
