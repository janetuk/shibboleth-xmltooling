## $Id:$ 

AUTOMAKE_OPTIONS = foreign

schemafiles = \
	soap-envelope.xsd \
	xenc-schema.xsd \
        xmldsig-core-schema.xsd \
        xml.xsd \
        xmltooling.xsd

pkgxmldir = $(datadir)/xml/@PACKAGE_NAME@-@PACKAGE_VERSION@

pkgxml_DATA = \
	catalog.xml \
	$(schemafiles)

#
# Some of these need building
#
do-build-file:
	rm -f ${FILE}.tmp
	sed < ${srcdir}/${FILE}.in > ${FILE}.tmp \
	    -e 's:@-PKGXMLDIR-@:${pkgxmldir}:g'
	cmp -s ${FILE} ${FILE}.tmp || mv ${FILE}.tmp ${FILE}
	rm -f ${FILE}.tmp

catalog.xml: ${srcdir}/catalog.xml.in Makefile ${top_builddir}/config.status
	$(MAKE) do-build-file FILE=$@

CLEANFILES = catalog.xml

EXTRA_DIST = catalog.xml.in \
	$(schemafiles)
