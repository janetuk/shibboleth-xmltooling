AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I .

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

dist-hook:
	rm -rf `find $(distdir)/xmltoolingtest/data -name .svn`
	rm -rf `find $(distdir)/doc/api -name .svn`


SUBDIRS = doc schemas xmltooling xmltoolingtest

if DX_COND_doc
all-local: doxygen-doc xmltooling.spec pkginfo
APIDOCS = doc/api
else
all-local: xmltooling.spec pkginfo
endif

install-data-hook:
	rm -rf $(DESTDIR)/$(datadir)/doc/@PACKAGE@/api
	cp -r doc/api $(DESTDIR)/$(datadir)/doc/@PACKAGE@

xmltooling.spec: xmltooling.spec.in Makefile
	rm -f $@.tmp
	sed < $< > $@.tmp \
	    -e 's:@-VERSION-@:${VERSION}:'
	mv $@.tmp $@

pkginfo: pkginfo.in Makefile
	rm -f $@.tmp
	sed < $< > $@.tmp \
	    -e 's:@-VERSION-@:${VERSION}:'
	mv $@.tmp $@

EXTRA_DIST = $(DX_CONFIG) \
        $(APIDOCS) \
        cpp-xmltooling.sln \
        acx_pthread.m4 \
        acinclude.m4 \
        libtool.m4 \
        doxygen.m4 \
        depcomp \
        config_win32.h \
        xmltooling.spec.in \
        xmltooling.spec \
        pkginfo.in \
        pkginfo \
        checkinstall

DISTCLEANFILES = xmltooling.spec pkginfo
