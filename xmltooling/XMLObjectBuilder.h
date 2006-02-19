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
 * @file XMLObjectBuilder.h
 * 
 * Factory interface for XMLObjects 
 */

#if !defined(__xmltooling_xmlobjbuilder_h__)
#define __xmltooling_xmlobjbuilder_h__

#include <xmltooling/XMLObject.h>

namespace xmltooling {

    /**
     * A factory interface for obtaining XMLObjects.
     * Subclasses MAY supply additional factory methods.
     */
    class XMLTOOL_API XMLObjectBuilder
    {
    MAKE_NON_COPYABLE(XMLObjectBuilder);
    public:
        virtual ~XMLObjectBuilder() {}
        
        /**
         * Creates an empty XMLObject.
         * 
         * @return the empty XMLObject
         */
        XMLObject* buildObject() const;

        /**
         * Resets the state of the builder.
         * 
         * This normally means null'ing out any properties that were
         * needed to build an object.
         */
        void resetState();
    };

};

#endif /* __xmltooling_xmlobjbuilder_h__ */
