AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I .

include doxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

EXTRA_DIST = $(DX_CONFIG) \
	doc \
	schemas \
	cpp-xmltooling.sln \
	libtool.m4 \
	acx_pthread.m4 \
	doxygen.m4 \
	depcomp \
	config_win32.h

dist-hook:
	rm -rf `find $(distdir)/xmltoolingtest/data -name .svn`
	rm -rf `find $(distdir)/schemas -name .svn`
	rm -rf `find $(distdir)/schemas -name Makefile`
	rm -rf `find $(distdir)/doc -name .svn`
	rm -rf `find $(distdir)/doc -name Makefile`


SUBDIRS = schemas xmltooling xmltoolingtest

all-local: xmltooling.spec pkginfo doxygen-doc

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

DISTCLEANFILES = xmltooling.spec pkginfo
