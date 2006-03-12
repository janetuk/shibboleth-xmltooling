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
 * XMLObject representing XML Digital Signature, version 20020212, KeyInfo element. 
 */

#if !defined(__xmltooling_keyinfo_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keyinfo_h__

#include <xmltooling/XMLObject.h>
#include <xsec/dsig/DSIGKeyInfoList.hpp>

namespace xmltooling {

    /**
     * XMLObject representing XML Digital Signature, version 20020212, KeyInfo element.
     */
    class XMLTOOL_API KeyInfo : public virtual XMLObject
    {
    public:
        virtual ~KeyInfo() {}

        /** Element local name */
        static const XMLCh LOCAL_NAME[];

        /**
         * Returns immutable ds:KeyInfo information.
         * 
         * @return the ds:KeyInfo information
         */
        virtual const DSIGKeyInfoList* getKeyInfo() const=0; 

        /**
         * Returns mutable ds:KeyInfo information.
         * 
         * @return the ds:KeyInfo information
         */
        virtual DSIGKeyInfoList* getKeyInfo()=0; 
        
    protected:
        KeyInfo() {}
    };

};

#endif /* __xmltooling_keyinfo_h__ */
