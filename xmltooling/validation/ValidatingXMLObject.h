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
 * @file ValidatingXMLObject.h
 * 
 * An XMLObject that can evaluate per-object validation rules.  
 */

#if !defined(__xmltooling_valxmlobj_h__)
#define __xmltooling_valxmlobj_h__

#include <vector>
#include <xmltooling/XMLObject.h>
#include <xmltooling/validation/Validator.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A functional interface for XMLObjects that offer the ability
     * to evaluate validation rules defined per-object.
     */
    class XMLTOOL_API ValidatingXMLObject : public virtual XMLObject
    {
    protected:
        ValidatingXMLObject() {}
    
    public:
        virtual ~ValidatingXMLObject() {}
        
        /**
         * Registers a validator for this XMLObject.
         * 
         * @param validator the validator
         */
        virtual void registerValidator(Validator* validator)=0;
        
        /**
         * Deregisters a validator for this XMLObject.
         * 
         * @param validator the validator
         */
        virtual void deregisterValidator(Validator* validator)=0;

        /**
         * Deregisters all validators for this XMLObject.
         */
        virtual void deregisterAll()=0;
        
        /**
         * Validates this XMLObject against all registered validators.
         * 
         * @param validateDescendants true if all the descendants of this object should 
         * be validated as well, false if not
         * 
         * @throws ValidationException thrown if the object is not valid
         */
        virtual void validate(bool validateDescendants) const=0;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_valxmlobj_h__ */
