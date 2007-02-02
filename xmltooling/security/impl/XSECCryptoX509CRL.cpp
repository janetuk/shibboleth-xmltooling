/*
 * Copyright 2001-2007 The Apache Software Foundation.
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
 * XSECCryptoX509CRL.cpp
 * 
 * Wrapper for X.509 CRL objects, similar to existing XSEC wrappers.
 */

#include "internal.h"
#include "security/XSECCryptoX509CRL.h"

#include <xsec/framework/XSECError.hpp>
#include <xsec/enc/XSECCryptoException.hpp>

using namespace xmltooling;

void XSECCryptoX509CRL::loadX509CRLPEM(const char* buf, unsigned int len)
{
	const char * b;
	char * b1 = NULL;
	if (len == 0)
		b = buf;
	else {
		XSECnew(b1, char[len+1]);
		memcpy(b1, buf, len);
		b1[len] = '\0';
		b = b1;
	}

	const char *p = strstr(buf, "-----BEGIN X509 CRL-----");

	if (p == NULL) {

		if (b1 != NULL)
			delete[] b1;

		throw XSECCryptoException(XSECCryptoException::X509Error,
		"X509CRL::loadX509CRLPEM - Cannot find start of PEM CRL");

	}

	p += strlen("-----BEGIN X509 CRL-----");

	while (*p == '\n' || *p == '\r' || *p == '-')
		p++;

	safeBuffer output;
	int i = 0;
	while (*p != '\0' && *p != '-') {
		output[i++] = *p;
		++p;
	}

	if (strstr(p, "-----END X509 CRL-----") != p) {

		if (b1 != NULL)
			delete[] b1;

		throw XSECCryptoException(XSECCryptoException::X509Error,
		"X509CRL::loadX509PEMCRL - Cannot find end of PEM certificate");

	}
	
	if (b1 != NULL)
		delete[] b1;

	output[i] = '\0';

	this->loadX509CRLBase64Bin(output.rawCharBuffer(), i);

}

