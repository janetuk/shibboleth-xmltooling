AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libxmltooling.la

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
	config_pub.h \
	ElementExtensibleXMLObject.h \
	ElementProxy.h \
	exceptions.h \
	Lockable.h \
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
	io/AbstractXMLObjectUnmarshaller.h

secinclude_HEADERS = \
	security/AbstractPKIXTrustEngine.h \
	security/CachingKeyResolver.h \
	security/ChainingTrustEngine.h \
	security/CredentialResolver.h \
	security/KeyInfoSource.h \
	security/KeyResolver.h \
	security/OpenSSLCredentialResolver.h \
	security/TrustEngine.h \
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
	util/DateTime.h \
	util/NDC.h \
	util/ParserPool.h \
	util/ReloadableXMLFile.h \
	util/ReplayCache.h \
	util/StorageService.h \
	util/TemplateEngine.h \
	util/Threads.h \
	util/XMLConstants.h \
	util/XMLHelper.h \
	util/XMLObjectChildrenList.h

valinclude_HEADERS = \
	validation/Validator.h \
	validation/ValidatorSuite.h

noinst_HEADERS = \
	internal.h

if BUILD_XMLSEC
xmlsec_sources = \
	encryption/impl/Decrypter.cpp \
	encryption/impl/Encrypter.cpp \
	security/impl/TrustEngine.cpp \
	security/impl/AbstractPKIXTrustEngine.cpp \
	security/impl/ChainingTrustEngine.cpp \
	security/impl/CredentialResolver.cpp \
	security/impl/FilesystemCredentialResolver.cpp \
	security/impl/InlineKeyResolver.cpp \
	security/impl/ExplicitKeyTrustEngine.cpp \
	security/impl/KeyResolver.cpp \
	security/impl/XSECCryptoX509CRL.cpp \
	security/impl/OpenSSLCryptoX509CRL.cpp \
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
	encryption/impl/EncryptionImpl.cpp \
	encryption/impl/EncryptionSchemaValidators.cpp \
	impl/AnyElement.cpp \
	impl/MemoryStorageService.cpp \
	impl/UnknownElement.cpp \
	io/AbstractXMLObjectMarshaller.cpp \
	io/AbstractXMLObjectUnmarshaller.cpp \
	signature/impl/KeyInfoImpl.cpp \
	signature/impl/KeyInfoSchemaValidators.cpp \
	soap/impl/SOAPClient.cpp \
	soap/impl/SOAPImpl.cpp \
	soap/impl/SOAPSchemaValidators.cpp \
	soap/impl/CURLSOAPTransport.cpp \
	util/DateTime.cpp \
	util/NDC.cpp \
	util/ParserPool.cpp \
	util/ReloadableXMLFile.cpp \
	util/ReplayCache.cpp \
	util/StorageService.cpp \
	util/TemplateEngine.cpp \
	util/XMLConstants.cpp \
	util/XMLHelper.cpp \
	validation/ValidatorSuite.cpp \
	${xmlsec_sources} \
	$(thread_sources)

# this is different from the project version
# http://sources.redhat.com/autobook/autobook/autobook_91.html
libxmltooling_la_LDFLAGS = -version-info 1:0:0

install-exec-hook:
	for la in $(lib_LTLIBRARIES) ; do rm -f $(DESTDIR)$(libdir)/$$la ; done

EXTRA_DIST = xmltooling.vcproj config_pub.h.in config_pub_win32.h
