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

namespace xmltooling {

    BEGIN_XMLOBJECTBUILDERIMPL(KeyInfo,XMLConstants::XMLSIG_NS);
    END_XMLOBJECTBUILDERIMPL;

};

#endif /* __xmltooling_keyinfoimpl_h__ */
