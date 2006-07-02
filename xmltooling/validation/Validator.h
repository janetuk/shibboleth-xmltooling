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
 * @file Validator.h
 * 
 * Rules checking of XMLObjects 
 */

#ifndef __xmltooling_validator_h__
#define __xmltooling_validator_h__

#include <xmltooling/XMLObject.h>

namespace xmltooling {

    /**
     * An interface for classes that implement rules for checking the 
     * validity of XMLObjects.
     */
    class XMLTOOL_API Validator
    {
    MAKE_NONCOPYABLE(Validator);
    public:
        virtual ~Validator() {}
        
        /**
         * Checks to see if an XMLObject is valid.
         * 
         * @param xmlObject the XMLObject to validate

         * @throws ValidationException thrown if the element is not valid
         */
        virtual void validate(const XMLObject* xmlObject) const=0;

        /**
         * Returns a copy of the validator.
         *
         * @return the new validator
         */
        virtual Validator* clone() const=0;

    protected:
        Validator() {}
    };

};

#endif /* __xmltooling_validator_h__ */
