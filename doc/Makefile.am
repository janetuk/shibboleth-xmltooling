AUTOMAKE_OPTIONS = foreign

pkgdocdir = $(datadir)/doc/@PACKAGE@-@PACKAGE_VERSION@

install-data-hook:
    rm -rf $(DESTDIR)$(pkgdocdir)/api
    cp -r doc/api $(DESTDIR)$(pkgdocdir)
    rm -rf `find $(DESTDIR)$(pkgdocdir)/api -name .svn`

docfiles = \
    README.txt \
	LICENSE.txt \
	NOTICE.txt \
	OPENSSL.LICENSE \
	LOG4CPP.LICENSE \
	CURL.LICENSE

pkgdoc_DATA = $(docfiles)

EXTRA_DIST = $(docfiles) api
