AUTOMAKE_OPTIONS = foreign

if BUILD_XMLSEC
lib_LTLIBRARIES = libxmltooling-lite.la libxmltooling.la
else
lib_LTLIBRARIES = libxmltooling-lite.la
endif

libxmltoolingincludedir = $(includedir)/xmltooling

encincludedir = $(includedir)/xmltooling/encryption

implincludedir = $(includedir)/xmltooling/impl

ioincludedir = $(includedir)/xmltooling/io

secincludedir = $(includedir)/xmltooling/security

sigincludedir = $(includedir)/xmltooling/signature

soapincludedir = $(includedir)/xmltooling/soap

utilincludedir = $(includedir)/xmltooling/util

valincludedir = $(includedir)/xmltooling/validation

libxmltoolinginclude_HEADERS = \
	AbstractAttributeExtensibleXMLObject.h \
	AbstractComplexElement.h \
	AbstractDOMCachingXMLObject.h \
	AbstractSimpleElement.h \
	AbstractXMLObject.h \
	AttributeExtensibleXMLObject.h \
	base.h \
	ConcreteXMLObjectBuilder.h \
	config_pub.h \
	ElementExtensibleXMLObject.h \
	ElementProxy.h \
	exceptions.h \
	Lockable.h \
	logging.h \
	Namespace.h \
	PluginManager.h \
	QName.h \
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
	io/AbstractXMLObjectUnmarshaller.h \
	io/GenericRequest.h \
	io/GenericResponse.h \
	io/HTTPRequest.h \
	io/HTTPResponse.h

secinclude_HEADERS = \
	security/AbstractPKIXTrustEngine.h \
	security/BasicX509Credential.h \
	security/ChainingTrustEngine.h \
	security/Credential.h \
	security/CredentialContext.h \
	security/CredentialCriteria.h \
	security/CredentialResolver.h \
	security/KeyInfoCredentialContext.h \
	security/KeyInfoResolver.h \
	security/OpenSSLCredential.h \
	security/SecurityHelper.h \
	security/SignatureTrustEngine.h \
	security/TrustEngine.h \
	security/X509Credential.h \
	security/X509TrustEngine.h \
	security/OpenSSLTrustEngine.h \
	security/XSECCryptoX509CRL.h \
	security/OpenSSLCryptoX509CRL.h

siginclude_HEADERS = \
	signature/ContentReference.h \
	signature/KeyInfo.h \
	signature/Signature.h \
	signature/SignatureValidator.h

soapinclude_HEADERS = \
	soap/SOAP.h \
	soap/SOAPClient.h \
	soap/SOAPTransport.h \
	soap/HTTPSOAPTransport.h \
	soap/OpenSSLSOAPTransport.h

utilinclude_HEADERS = \
    util/CurlNetAccessor.hpp \
    util/CurlURLInputStream.hpp \
	util/DateTime.h \
	util/NDC.h \
	util/ParserPool.h \
	util/PathResolver.h \
	util/Predicates.h \
	util/ReloadableXMLFile.h \
	util/ReplayCache.h \
	util/StorageService.h \
	util/TemplateEngine.h \
	util/Threads.h \
	util/URLEncoder.h \
	util/XMLConstants.h \
	util/XMLHelper.h \
	util/XMLObjectChildrenList.h

valinclude_HEADERS = \
	validation/Validator.h \
	validation/ValidatorSuite.h

noinst_HEADERS = \
	internal.h

xmlsec_sources = \
	encryption/impl/Decrypter.cpp \
	encryption/impl/EncryptedKeyResolver.cpp \
	encryption/impl/Encrypter.cpp \
    encryption/impl/EncryptionImpl.cpp \
    encryption/impl/EncryptionSchemaValidators.cpp \
	impl/MemoryStorageService.cpp \
	security/impl/AbstractPKIXTrustEngine.cpp \
	security/impl/BasicX509Credential.cpp \
	security/impl/ChainingCredentialResolver.cpp \
	security/impl/ChainingTrustEngine.cpp \
	security/impl/CredentialCriteria.cpp \
	security/impl/CredentialResolver.cpp \
	security/impl/ExplicitKeyTrustEngine.cpp \
	security/impl/FilesystemCredentialResolver.cpp \
	security/impl/InlineKeyResolver.cpp \
	security/impl/KeyInfoResolver.cpp \
	security/impl/OpenSSLCryptoX509CRL.cpp \
	security/impl/SecurityHelper.cpp \
	security/impl/StaticPKIXTrustEngine.cpp \
	security/impl/TrustEngine.cpp \
	security/impl/XSECCryptoX509CRL.cpp \
    signature/impl/KeyInfoImpl.cpp \
    signature/impl/KeyInfoSchemaValidators.cpp \
	signature/impl/SignatureValidator.cpp \
	signature/impl/XMLSecSignatureImpl.cpp \
	soap/impl/CURLSOAPTransport.cpp \
	util/CurlNetAccessor.cpp \
	util/CurlURLInputStream.cpp \
	util/ReplayCache.cpp \
	util/StorageService.cpp

if BUILD_PTHREAD
thread_sources = util/PThreads.cpp
else
thread_sources =
endif

common_sources = \
	AbstractAttributeExtensibleXMLObject.cpp \
	AbstractComplexElement.cpp \
	AbstractDOMCachingXMLObject.cpp \
	AbstractSimpleElement.cpp \
	AbstractXMLObject.cpp \
	exceptions.cpp \
	Namespace.cpp \
	QName.cpp \
	unicode.cpp \
	XMLObjectBuilder.cpp \
	XMLToolingConfig.cpp \
	impl/AnyElement.cpp \
	impl/UnknownElement.cpp \
	io/AbstractXMLObjectMarshaller.cpp \
	io/AbstractXMLObjectUnmarshaller.cpp \
	io/HTTPRequest.cpp \
	soap/impl/SOAPClient.cpp \
	soap/impl/SOAPImpl.cpp \
	soap/impl/SOAPSchemaValidators.cpp \
	util/DateTime.cpp \
	util/NDC.cpp \
	util/ParserPool.cpp \
	util/PathResolver.cpp \
	util/ReloadableXMLFile.cpp \
	util/TemplateEngine.cpp \
	util/URLEncoder.cpp \
	util/XMLConstants.cpp \
	util/XMLHelper.cpp \
	validation/ValidatorSuite.cpp \
	$(thread_sources)

libxmltooling_lite_la_SOURCES = \
	${common_sources}
libxmltooling_lite_la_CPPFLAGS = -DXMLTOOLING_LITE
libxmltooling_lite_la_LDFLAGS = -version-info 3:0:0

if BUILD_XMLSEC
libxmltooling_la_LDFLAGS = $(XMLSEC_LIBS) -version-info 3:0:0
libxmltooling_la_SOURCES = \
	${common_sources} \
	${xmlsec_sources}
endif

install-exec-hook:
	for la in $(lib_LTLIBRARIES) ; do rm -f $(DESTDIR)$(libdir)/$$la ; done

EXTRA_DIST = \
    xmltooling.vcproj \
    xmltooling-lite.vcproj \
    config_pub.h.in \
    config_pub_win32.h\
    resource.h\
    xmltooling.rc \
    util/Win32Threads.cpp
