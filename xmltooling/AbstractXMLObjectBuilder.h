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
 * @file AbstractXMLObjectBuilder.h
 * 
 * Factory interface for XMLObjects 
 */

#if !defined(__xmltooling_abstractxmlobjbuilder_h__)
#define __xmltooling_abstractxmlobjbuilder_h__

#include <xmltooling/XMLObjectBuilder.h>

namespace xmltooling {

    /**
     * Base implementation of XMLObjectBuilder that automatically 
     * invokes resetState() after the XMLObject is built.
     */
    class XMLTOOL_API AbstractXMLObjectBuilder : public virtual XMLObjectBuilder
    {
    public:
        virtual ~AbstractXMLObjectBuilder() {}
        
        /**
         * @see XMLObjectBuilder::buildObject()
         */
        XMLObject* buildObject() {
            XMLObject* builtObject = doBuildObject();
            resetState();
            return builtObject;
        }

        /**
         * Delegated call that builds the XMLObject prior to a state reset.
         * 
         * @return the built XMLObject
         */
        virtual XMLObject* doBuildObject()=0;
    };

};

#endif /* __xmltooling_abstractxmlobjbuilder_h__ */
