## $Id:$ 

AUTOMAKE_OPTIONS = foreign

pkgxmldir = $(datadir)/xml/@PACKAGE@

pkgxml_DATA = \
	catalog.xml \
	xenc-schema.xsd \
	xmldsig-core-schema.xsd \
	xml.xsd \
	xmltooling.xsd

#
# Some of these need building
#
do-build-file:
	rm -f ${FILE}.tmp
	sed < ${srcdir}/${FILE}.in > ${FILE}.tmp \
	    -e 's:@-PKGXMLDIR-@:${pkgxmldir}:'
	cmp -s ${FILE} ${FILE}.tmp || mv ${FILE}.tmp ${FILE}
	rm -f ${FILE}.tmp

catalog.xml: ${srcdir}/catalog.xml.in Makefile ${top_builddir}/config.status
	$(MAKE) do-build-file FILE=$@

CLEANFILES = catalog.xml

EXTRA_DIST = \
    catalog.xml.in \
    soap-envelope.xsd \
    xenc-schema.xsd \
    xmldsig-core-schema.xsd \
    xml.xsd \
    xmltooling.xsd
    