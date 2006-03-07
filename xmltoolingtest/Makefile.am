AUTOMAKE_OPTIONS = foreign

if BUILD_UNITTEST
bin_PROGRAMS = xmltoolingtest
xmltoolingtest_CXXFLAGS = $(CXXFLAGS) $(CXXTESTFLAGS)
else
bin_PROGRAMS = 
endif

xmltoolingtest_h = \
    ComplexXMLObjectTest.h \
    MarshallingTest.h \
    UnknownTest.h \
    UnmarshallingTest.h \
    xmltoolingtest.h

noinst_HEADERS = \
    XMLObjectBaseTestCase.h

nodist_xmltoolingtest_SOURCES = $(xmltoolingtest_h:.h=.cpp)

CLEANFILES = $(nodist_xmltoolingtest_SOURCES)

do-cxxtestgen:
	if test $(HFILE) = "xmltoolingtest.h"; then \
		$(CXXTEST) --error-printer --have-eh --have-std --abort-on-fail -o $(CPPFILE) $(HFILE); \
	else \
		$(CXXTEST) --part --have-eh --have-std --abort-on-fail -o $(CPPFILE) $(HFILE); \
	fi;

$(nodist_xmltoolingtest_SOURCES): %.cpp: %.h
	$(MAKE) do-cxxtestgen HFILE=$< CPPFILE=$@

xmltoolingtest_LDADD = $(top_builddir)/xmltooling/libxmltooling.la

EXTRA_DIST = xmltoolingtest.vcproj $(xmltoolingtest_h) data
