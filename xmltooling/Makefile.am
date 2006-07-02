AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libxmltooling.la

libxmltoolingincludedir = $(includedir)/xmltooling

encincludedir = $(includedir)/xmltooling/encryption

implincludedir = $(includedir)/xmltooling/impl

ioincludedir = $(includedir)/xmltooling/io

sigincludedir = $(includedir)/xmltooling/signature

utilincludedir = $(includedir)/xmltooling/util

valincludedir = $(includedir)/xmltooling/validation

libxmltoolinginclude_HEADERS = \
	AbstractAttributeExtensibleXMLObject.h \
	AbstractChildlessElement.h \
	AbstractComplexElement.h \
	AbstractDOMCachingXMLObject.h \
	AbstractElementProxy.h \
	AbstractSimpleElement.h \
	AbstractXMLObject.h \
	AttributeExtensibleXMLObject.h \
	base.h \
	config_pub.h \
	ElementProxy.h \
	exceptions.h \
	Lockable.h \
	Namespace.h \
	PluginManager.h \
	QName.h \
	SimpleElement.h \
	unicode.h \
	version.h \
	XMLObject.h \
	XMLObjectBuilder.h \
	XMLToolingConfig.h

encinclude_HEADERS = \
	encryption/Decrypter.h \
	encryption/EncryptedKeyResolver.h \
	encryption/Encrypter.h \
	encryption/Encryption.h

implinclude_HEADERS = \
	impl/AnyElement.h \
	impl/UnknownElement.h

ioinclude_HEADERS = \
	io/AbstractXMLObjectMarshaller.h \
	io/AbstractXMLObjectUnmarshaller.h

siginclude_HEADERS = \
	signature/ContentReference.h \
	signature/KeyInfo.h \
	signature/KeyResolver.h \
	signature/Signature.h \
	signature/SignatureValidator.h

utilinclude_HEADERS = \
	util/DateTime.h \
	util/NDC.h \
	util/ParserPool.h \
	util/Threads.h \
	util/XMLConstants.h \
	util/XMLHelper.h \
	util/XMLObjectChildrenList.h

valinclude_HEADERS = \
	validation/AbstractValidatingXMLObject.h \
	validation/ValidatingXMLObject.h \
	validation/Validator.h

noinst_HEADERS = \
	internal.h

if BUILD_XMLSEC
xmlsec_sources = \
	encryption/impl/Decrypter.cpp \
	encryption/impl/Encrypter.cpp \
	signature/impl/SignatureValidator.cpp \
	signature/impl/XMLSecSignatureImpl.cpp
else
xmlsec_sources =
endif

if BUILD_PTHREAD
thread_sources = util/PThreads.cpp
else
thread_sources =
endif

libxmltooling_la_SOURCES = \
	AbstractAttributeExtensibleXMLObject.cpp \
	AbstractChildlessElement.cpp \
	AbstractComplexElement.cpp \
	AbstractDOMCachingXMLObject.cpp \
	AbstractXMLObject.cpp \
	exceptions.cpp \
	Namespace.cpp \
	QName.cpp \
	unicode.cpp \
	XMLObjectBuilder.cpp \
	XMLToolingConfig.cpp \
	encryption/impl/EncryptionImpl.cpp \
	encryption/impl/EncryptionSchemaValidators.cpp \
	impl/AnyElement.cpp \
	impl/UnknownElement.cpp \
	io/AbstractXMLObjectMarshaller.cpp \
	io/AbstractXMLObjectUnmarshaller.cpp \
	signature/impl/KeyInfoImpl.cpp \
	signature/impl/KeyInfoSchemaValidators.cpp \
	util/DateTime.cpp \
	util/NDC.cpp \
	util/ParserPool.cpp \
	util/XMLConstants.cpp \
	util/XMLHelper.cpp \
	validation/AbstractValidatingXMLObject.cpp \
	validation/Validator.cpp \
	${xmlsec_sources} \
	$(thread_sources)

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libxmltooling_la_LDFLAGS = -version-info 1:0:0

install-exec-hook:
	for la in $(lib_LTLIBRARIES) ; do rm -f $(DESTDIR)$(libdir)/$$la ; done

EXTRA_DIST = xmltooling.vcproj config_pub.h.in config_pub_win32.h
