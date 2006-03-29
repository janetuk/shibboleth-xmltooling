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
 * @file AnyElement.h
 * 
 * Advanced anyType implementation suitable for deep processing of unknown content.
 */

#if !defined(__xmltooling_anyelement_h__)
#define __xmltooling_anyelement_h__

#include <xmltooling/XMLObjectBuilder.h>

namespace xmltooling {

    /**
     * Builder for AnyElementImpl objects.
     * Use as the default builder when you want to wrap each unknown element and
     * process the DOM content through xmltooling interfaces. 
     */
    class XMLTOOL_API AnyElementBuilder : public XMLObjectBuilder
    {
    public:
        XMLObject* buildObject(
            const XMLCh* namespaceURI, const XMLCh* elementLocalName, const XMLCh* namespacePrefix=NULL
            ) const;
    };

};

#endif /* __xmltooling_anyelement_h__ */
