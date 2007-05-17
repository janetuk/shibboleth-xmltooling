AUTOMAKE_OPTIONS = foreign

if BUILD_UNITTEST

bin_PROGRAMS = xmltoolingtest
xmltoolingtest_CXXFLAGS = $(CXXTESTFLAGS)
if BUILD_XMLSEC
xmltoolingtest_LDFLAGS = $(XMLSEC_LIBS)
xmltoolingtest_LDADD = $(top_builddir)/xmltooling/libxmltooling.la
else
xmltoolingtest_LDADD = $(top_builddir)/xmltooling/libxmltooling-lite.la
endif

else

bin_PROGRAMS = 

endif

if BUILD_XMLSEC
xmlsec_sources = \
    EncryptionTest.h \
    FilesystemCredentialResolverTest.h \
    InlineKeyResolverTest.h \
    SignatureTest.h
else
xmlsec_sources =
endif

xmltoolingtest_h = \
    ComplexXMLObjectTest.h \
    ExceptionTest.h \
    KeyInfoTest.h \
    MarshallingTest.h \
    MemoryStorageServiceTest.h \
    UnmarshallingTest.h \
    TemplateEngineTest.h \
    xmltoolingtest.h \
    ${xmlsec_sources}

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

EXTRA_DIST = xmltoolingtest.vcproj $(xmltoolingtest_h) data
