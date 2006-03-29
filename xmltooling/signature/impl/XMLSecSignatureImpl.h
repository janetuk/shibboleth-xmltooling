/*
*  Copyright 2001-2006 Internet2
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
 * XMLSecSignatureImpl.h
 * 
 * Builder class for XMLSec-based signature-handling
 */

#if !defined(__xmltooling_xmlsecsig_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_xmlsecsig_h__

#include <xmltooling/signature/Signature.h>

namespace xmltooling {

    class XMLTOOL_DLLLOCAL XMLSecSignatureBuilder : public SignatureBuilder
    {
    public:
        Signature* buildObject(
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL
            ) const;
        Signature* buildObject() const;
    };

};

#endif /* __xmltooling_xmlsecsig_h__ */
