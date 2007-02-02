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
 * @file ValidatorSuite.h
 * 
 * Groups of rule checkers of XMLObjects based on type or element name. 
 */

#ifndef __xmltooling_valsuite_h__
#define __xmltooling_valsuite_h__

#include <xmltooling/QName.h>
#include <xmltooling/validation/Validator.h>

#include <map>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A collection of validators that can be applied to an XMLObject and its children. These collections can represent
     * usage specific checks, such as those outlined in schemas or profiles of specific XML specifications.
     * 
     * Registered Validators must be stateless. Validators are fetched based on schema type and
     * element name, in that order.
     */
    class XMLTOOL_API ValidatorSuite
    {
    MAKE_NONCOPYABLE(ValidatorSuite);
    public:
        /**
         * Creates a new suite.
         * 
         * @param id    an identifier for the suite
         */
        ValidatorSuite(const char* id) : m_id(id) {}
        
        ~ValidatorSuite() {
            destroyValidators();
        }

        /**
         * Gets a unique ID for this suite.
         * 
         * @return a unique ID for this suite
         */
        const char* getId() {
            return m_id.c_str();
        }

        /**
         * Evaluates the registered validators against the given XMLObject and it's children.
         * 
         * @param xmlObject the XMLObject tree to validate
         * 
         * @throws ValidationException thrown if the element tree is not valid
         */
        void validate(const XMLObject* xmlObject) const;

        /**
         * Registers a new validator for the given key.
         * 
         * @param key       the key used to retrieve the validator
         * @param validator the validator
         */
        void registerValidator(const QName& key, Validator* validator) {
            m_map.insert(std::make_pair(key,validator));
        }

        /**
         * Deregisters validators.
         * 
         * @param key       the key for the validators to be deregistered
         */
        void deregisterValidators(const QName& key);

        /**
         * Unregisters and destroys all registered validators. 
         */
        void destroyValidators();

    private:
        std::string m_id;
        std::multimap<QName,Validator*> m_map;
    };

    /**
     * Validator suite for schema-style structural validation.
     * 
     * This is <strong>NOT</strong> a comprehensive replacement for real
     * schema validation, but it does basic structural checking of overall
     * element relationships and some basic attribute presence checking.
     */
    extern XMLTOOL_API xmltooling::ValidatorSuite SchemaValidators;
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_valsuite_h__ */
