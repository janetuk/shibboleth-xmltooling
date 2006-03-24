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

#if !defined(__xmltooling_validator_h__)
#define __xmltooling_validator_h__

#include <map>
#include <vector>
#include <xmltooling/QName.h>
#include <xmltooling/XMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

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
         * Evaluates the registered validators against the given XMLObject and it's children.
         * 
         * @param xmlObject the XMLObject tree to validate
         * 
         * @throws ValidationException thrown if the element tree is not valid
         */
        static void checkValidity(const XMLObject* xmlObject);

        /**
         * Registers a new validator for the given key.
         * 
         * @param key       the key used to retrieve the validator
         * @param validator the validator
         */
        static void registerValidator(const QName& key, Validator* validator) {
            std::map< QName, std::vector<Validator*> >::iterator i=m_map.find(key);
            if (i==m_map.end())
                m_map.insert(std::make_pair(key,std::vector<Validator*>(1,validator)));
            else
                i->second.push_back(validator);
        }

        /**
         * Deregisters validators.
         * 
         * @param key       the key for the validators to be deregistered
         */
        static void deregisterValidators(const QName& key);

        /**
         * Unregisters and destroys all registered validators. 
         */
        static void destroyValidators();

    protected:
        Validator() {}
    
    private:
        static std::map< QName, std::vector<Validator*> > m_map;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_validator_h__ */
