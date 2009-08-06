AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I .

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

dist-hook:
	rm -rf `find $(distdir)/xmltoolingtest/data -name .svn`
	rm -rf `find $(distdir)/doc/api -name .svn`


SUBDIRS = doc schemas xmltooling xmltoolingtest

GENFILES = xmltooling.spec pkginfo Portfile

if DX_COND_doc
all-local: doxygen-doc $(GENFILES)
else
all-local: $(GENFILES)
endif

install-data-hook:
	rm -rf $(DESTDIR)$(datadir)/doc/@PACKAGE@/api
	cp -r doc/api $(DESTDIR)$(datadir)/doc/@PACKAGE@

$(GENFILES): %: %.in Makefile
	rm -f $@.tmp
	sed < $@.in > $@.tmp \
	    -e 's:@-VERSION-@:${VERSION}:'
	mv $@.tmp $@

EXTRA_DIST = $(DX_CONFIG) \
        cpp-xmltooling.sln \
        acx_pthread.m4 \
        acinclude.m4 \
        libtool.m4 \
        doxygen.m4 \
        depcomp \
        config_win32.h \
        $(GENFILES) \
        $(GENFILES:%=%.in) \
        depend \
        postinstall

DISTCLEANFILES = $(GENFILES)
