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
 * @file xmltooling/util/NDC.h
 * 
 * Diagnostic context for logging 
 */

#ifndef __xmltooling_ndc_h__
#define __xmltooling_ndc_h__

#include <string>
#include <xmltooling/base.h>

namespace xmltooling {
    
    /**
     * A portable stack-based context for diagnostic logging 
     */
    class XMLTOOL_API NDC
    {
    public:
        /**
         * Constructor pushes logging context onto diagnostic stack
         * @param context   null-terminated label for context
         */
        NDC(const char* context);

        /**
         * Constructor pushes logging context onto diagnostic stack
         * @param context   string label for context
         */
        NDC(const std::string& context);
        
        /**
         * Destructor pops context off of diagnostic stack
         */
        ~NDC();
        
    MAKE_NONCOPYABLE(NDC);
    };

};

#endif /* __xmltooling_ndc_h__ */
