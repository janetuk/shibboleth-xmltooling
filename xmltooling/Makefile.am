AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libxmltooling.la

libxmltoolingincludedir = $(includedir)/xmltooling

ioincludedir = $(includedir)/xmltooling/io

sigincludedir = $(includedir)/xmltooling/signature

utilincludedir = $(includedir)/xmltooling/util

valincludedir = $(includedir)/xmltooling/validation

libxmltoolinginclude_HEADERS = \
    AbstractAttributeExtensibleXMLObject.h \
    AbstractDOMCachingXMLObject.h \
    AbstractElementProxy.h \
    AbstractXMLObject.h \
    AttributeExtensibleXMLObject.h \
    base.h \
    config_pub.h \
    DOMCachingXMLObject.h \
    ElementProxy.h \
    exceptions.h \
    ILockable.h \
    Namespace.h \
    QName.h \
    unicode.h \
    version.h \
    XMLObject.h \
    XMLObjectBuilder.h \
    XMLToolingConfig.h

ioinclude_HEADERS = \
    io/AbstractXMLObjectUnmarshaller.h \
    io/Marshaller.h \
    io/Unmarshaller.h

siginclude_HEADERS = \
    signature/KeyInfo.h \
    signature/Signature.h \
    signature/SigningContext.h

utilinclude_HEADERS = \
    util/NDC.h \
    util/ParserPool.h \
    util/XMLConstants.h \
    util/XMLHelper.h \
    util/XMLObjectChildrenList.h

valinclude_HEADERS = \
    validation/Validator.h

noinst_HEADERS = \
    internal.h \
    impl/UnknownElement.h \
    signature/impl/XMLSecSignature.h

libxmltooling_la_SOURCES = \
    AbstractAttributeExtensibleXMLObject.cpp \
    AbstractDOMCachingXMLObject.cpp \
    AbstractElementProxy.cpp \
    Namespace.cpp \
    QName.cpp \
    unicode.cpp \
    XMLObjectBuilder.cpp \
    XMLToolingConfig.cpp \
    impl/UnknownElement.cpp \
    io/AbstractXMLObjectMarshaller.cpp \
    io/AbstractXMLObjectUnmarshaller.cpp \
    io/Marshaller.cpp \
    io/Unmarshaller.cpp \
    signature/impl/XMLSecSignature.cpp \
    util/NDC.cpp \
    util/ParserPool.cpp \
    util/XMLConstants.cpp \
    util/XMLHelper.cpp \
    validation/Validator.cpp

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libxmltooling_la_LDFLAGS = -version-info 1:0:0

install-exec-hook:
	for la in $(lib_LTLIBRARIES) ; do rm -f $(DESTDIR)$(libdir)/$$la ; done

EXTRA_DIST = xmltooling.vcproj config_pub.h.in config_pub_win32.h
