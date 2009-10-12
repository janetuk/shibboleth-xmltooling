/*
 * Copyright 2001-2009 The Apache Software Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * OpenSSLCryptoX509CRL.cpp
 * 
 * OpenSSL-based class for handling X.509 CRLs.
 */

#include "internal.h"
#include "security/OpenSSLCryptoX509CRL.h"

#include <xsec/framework/XSECError.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/enc/XSCrypt/XSCryptCryptoBase64.hpp>

#include <xercesc/util/Janitor.hpp>

XSEC_USING_XERCES(ArrayJanitor);
XSEC_USING_XERCES(Janitor);

using namespace xmltooling;

OpenSSLCryptoX509CRL::OpenSSLCryptoX509CRL() : mp_X509CRL(NULL), m_DERX509CRL("")
{
}

OpenSSLCryptoX509CRL::~OpenSSLCryptoX509CRL()
{
	if (mp_X509CRL)
		X509_CRL_free(mp_X509CRL);
}

OpenSSLCryptoX509CRL::OpenSSLCryptoX509CRL(X509_CRL* x) {

	// Build this from an existing X509_CRL structure

	mp_X509CRL = X509_CRL_dup(x);
	
	// Now need to create the DER encoding

	BIO* b64 = BIO_new(BIO_f_base64());
	BIO* bmem = BIO_new(BIO_s_mem());

	BIO_set_mem_eof_return(bmem, 0);
	b64 = BIO_push(b64, bmem);

	// Translate X509 to Base64

	i2d_X509_CRL_bio(b64, x);

	BIO_flush(b64);

	char buf[1024];
	unsigned int l;
	
	m_DERX509CRL.sbStrcpyIn("");

	while ((l = BIO_read(bmem, buf, 1023)) > 0) {
		buf[l] = '\0';
		m_DERX509CRL.sbStrcatIn(buf);
	}

	BIO_free_all(b64);
}

const XMLCh* OpenSSLCryptoX509CRL::getProviderName() const
{
    return DSIGConstants::s_unicodeStrPROVOpenSSL;
}

void OpenSSLCryptoX509CRL::loadX509CRLBase64Bin(const char* buf, unsigned int len)
{

	// Free anything currently held.
	
	if (mp_X509CRL)
		X509_CRL_free(mp_X509CRL);
	
	int bufLen = len;
	unsigned char* outBuf;
	XSECnew(outBuf, unsigned char[len + 1]);
	ArrayJanitor<unsigned char> j_outBuf(outBuf);

	XSCryptCryptoBase64 *b64;
	XSECnew(b64, XSCryptCryptoBase64);
	Janitor<XSCryptCryptoBase64> j_b64(b64);

	b64->decodeInit();
	bufLen = b64->decode((unsigned char *) buf, len, outBuf, len);
	bufLen += b64->decodeFinish(&outBuf[bufLen], len-bufLen);

	if (bufLen > 0) {
#if defined(XSEC_OPENSSL_D2IX509_CONST_BUFFER)
		mp_X509CRL=  d2i_X509_CRL(NULL, (const unsigned char **) (&outBuf), bufLen);
#else
		mp_X509CRL=  d2i_X509_CRL(NULL, &outBuf, bufLen);
#endif
	}

	// Check to see if we have a CRL....
	if (mp_X509CRL == NULL) {
		throw XSECCryptoException(XSECCryptoException::X509Error,
		"OpenSSL:X509CRL - Error translating Base64 DER encoding into OpenSSL X509 CRL structure");
	}

	m_DERX509CRL.sbStrcpyIn(buf);

}

safeBuffer& OpenSSLCryptoX509CRL::getDEREncodingSB()
{
    return m_DERX509CRL;
}

X509_CRL* OpenSSLCryptoX509CRL::getOpenSSLX509CRL()
{
    return mp_X509CRL;
}

XSECCryptoX509CRL* OpenSSLCryptoX509CRL::clone() const
{
    OpenSSLCryptoX509CRL* copy = new OpenSSLCryptoX509CRL();
    copy->mp_X509CRL = X509_CRL_dup(mp_X509CRL);
    copy->m_DERX509CRL = m_DERX509CRL;
    return copy;
}
