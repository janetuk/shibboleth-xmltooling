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
 * @file KeyInfo.h
 * 
 * XMLObjects representing XML Digital Signature, version 20020212, KeyInfo element
 * and related content.
 */

#ifndef __xmltooling_keyinfo_h__
#define __xmltooling_keyinfo_h__

#include <xmltooling/ElementProxy.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/validation/ValidatingXMLObject.h>

namespace xmltooling {

    /**
     * XMLObject representing XML Digital Signature, version 20020212, KeyInfo element.
     */
    BEGIN_XMLOBJECT(KeyInfo,ElementProxy);
        DECL_XMLOBJECT_ATTRIB(Id,ID);
        static const XMLCh TYPE_NAME[];
    END_XMLOBJECT;

    BEGIN_XMLOBJECTBUILDER(KeyInfo);
    END_XMLOBJECTBUILDER;

#ifdef XMLTOOLING_DEFINE_CONSTANTS
    const XMLCh KeyInfo::LOCAL_NAME[] = {
        chLatin_K, chLatin_e, chLatin_y, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chNull
    }; 
    const XMLCh KeyInfo::TYPE_NAME[] = {
        chLatin_K, chLatin_e, chLatin_y, chLatin_I, chLatin_n, chLatin_f, chLatin_o,
        chLatin_T, chLatin_y, chLatin_p, chLatin_e, chNull
    }; 
    const XMLCh KeyInfo::ID_ATTRIB_NAME[] = {
        chLatin_I, chLatin_d, chNull
    };
#endif

    /**
     * XMLObject representing XML Digital Signature, version 20020212, KeyName element.
     */
    BEGIN_XMLOBJECT(KeyName,XMLObject);
        DECL_XMLOBJECT_CONTENT(Name);
    END_XMLOBJECT;

    BEGIN_XMLOBJECTBUILDER(KeyName);
    END_XMLOBJECTBUILDER;

#ifdef XMLTOOLING_DEFINE_CONSTANTS
    const XMLCh KeyName::LOCAL_NAME[] = {
        chLatin_K, chLatin_e, chLatin_y, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chNull
    }; 
#endif

    /**
     * XMLObject representing XML Digital Signature, version 20020212, MgmtData element.
     */
    BEGIN_XMLOBJECT(MgmtData,XMLObject);
        DECL_XMLOBJECT_CONTENT(Data);
    END_XMLOBJECT;

    BEGIN_XMLOBJECTBUILDER(MgmtData);
    END_XMLOBJECTBUILDER;

#ifdef XMLTOOLING_DEFINE_CONSTANTS
    const XMLCh MgmtData::LOCAL_NAME[] = {
        chLatin_M, chLatin_g, chLatin_m, chLatin_t, chLatin_D, chLatin_a, chLatin_t, chLatin_a, chNull
    }; 
#endif
};

#endif /* __xmltooling_keyinfo_h__ */
