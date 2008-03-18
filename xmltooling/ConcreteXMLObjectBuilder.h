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
 * @file xmltooling/ConcreteXMLObjectBuilder.h
 * 
 * Factory interface for concrete XMLObjects, supporting default construction. 
 */

#ifndef __xmltooling_concxmlobjbuilder_h__
#define __xmltooling_concxmlobjbuilder_h__

#include <xmltooling/XMLObjectBuilder.h>

namespace xmltooling {

    /**
     * A factory interface for obtaining XMLObjects.
     * Subclasses MAY supply additional factory methods.
     */
    class XMLTOOL_API ConcreteXMLObjectBuilder : public XMLObjectBuilder
    {
    public:
        virtual ~ConcreteXMLObjectBuilder() {}

        using XMLObjectBuilder::buildObject;
        
        /**
         * Creates an empty XMLObject with a defaulted element name and/or type.
         * 
         * @return the empty XMLObject
         */
        virtual XMLObject* buildObject() const=0;

    protected:
        ConcreteXMLObjectBuilder() {}
    };

};

#endif /* __xmltooling_concxmlobjbuilder_h__ */
