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
 * KeyInfoImpl.h
 * 
 * Builder class for KeyInfo 
 */

#ifndef __xmltooling_keyinfoimpl_h__
#define __xmltooling_keyinfoimpl_h__

#include <xmltooling/signature/KeyInfo.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/validation/Validator.h>

#define BEGIN_XMLSIGOBJECTBUILDERIMPL(cname) \
    BEGIN_XMLOBJECTBUILDERIMPL(cname,XMLConstants::XMLSIG_NS,XMLConstants::XMLSIG_PREFIX)

namespace xmltooling {
    
    BEGIN_XMLSIGOBJECTBUILDERIMPL(KeyInfo);
    END_XMLOBJECTBUILDERIMPL;

    BEGIN_XMLSIGOBJECTBUILDERIMPL(KeyName);
    END_XMLOBJECTBUILDERIMPL;

    BEGIN_XMLSIGOBJECTBUILDERIMPL(MgmtData);
    END_XMLOBJECTBUILDERIMPL;
    
    BEGIN_XMLOBJECTVALIDATOR(KeyInfoSchema);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(KeyNameSchema);
    END_XMLOBJECTVALIDATOR;

    BEGIN_XMLOBJECTVALIDATOR(MgmtDataSchema);
    END_XMLOBJECTVALIDATOR;
};

#endif /* __xmltooling_keyinfoimpl_h__ */
