/*
 *  Copyright 2001-2007 Internet2
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
 * SignatureValidator.cpp
 * 
 * Validator for signatures based on an externally-supplied key 
 */
 
#include "internal.h"
#include "signature/SignatureValidator.h"

#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

void SignatureValidator::validate(const XMLObject* xmlObject) const
{
    const Signature* sigObj=dynamic_cast<const Signature*>(xmlObject);
    if (!sigObj)
        throw ValidationException("Validator only applies to Signature objects.");
    validate(sigObj);
}

void SignatureValidator::validate(const Signature* sigObj) const
{
    DSIGSignature* sig=sigObj->getXMLSignature();
    if (!sig)
        throw ValidationException("Signature does not exist yet.");
    else if (!m_key && !m_credential)
        throw ValidationException("No Credential or key set on Validator.");

    XSECCryptoKey* key = m_key ? m_key : (m_credential ? m_credential->getPublicKey() : NULL);
    if (!key)
        throw ValidationException("Credential did not contain a verification key.");

    try {
        sig->setSigningKey(key->clone());
        if (!sig->verify())
            throw ValidationException("Digital signature does not validate with the supplied key.");
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw ValidationException(string("Caught an XMLSecurity exception verifying signature: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw ValidationException(string("Caught an XMLSecurity exception verifying signature: ") + e.getMsg());
    }
}
