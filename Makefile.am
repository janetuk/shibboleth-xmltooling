AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I .

EXTRA_DIST =
	schemas \
	cpp-xmltooling.sln \
	libtool.m4 \
	acx_pthread.m4 \
	depcomp \
	config_win32.h

dist-hook:
	rm -rf `find $(distdir)/xmltoolingtest/data -name .svn`
	rm -rf `find $(distdir)/schemas -name .svn`
	rm -rf `find $(distdir)/schemas -name Makefile`


SUBDIRS = schemas xmltooling xmltoolingtest

#all-local: opensaml.spec pkginfo
#
#opensaml.spec: opensaml.spec.in Makefile
#	rm -f $@.tmp
#	sed < $< > $@.tmp \
#	    -e 's:@-VERSION-@:${VERSION}:'
#	mv $@.tmp $@
#
#pkginfo: pkginfo.in Makefile
#	rm -f $@.tmp
#	sed < $< > $@.tmp \
#	    -e 's:@-VERSION-@:${VERSION}:'
#	mv $@.tmp $@
#
#DISTCLEANFILES = opensaml.spec pkginfo
