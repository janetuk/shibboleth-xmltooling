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
 * @file AbstractValidatingXMLObject.h
 * 
 * Extension of AbstractXMLObject that implements a ValidatingXMLObject. 
 */

#if !defined(__xmltooling_abstractvalxmlobj_h__)
#define __xmltooling_abstractvalxmlobj_h__

#include <xmltooling/AbstractXMLObject.h>
#include <xmltooling/validation/ValidatingXMLObject.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Extension of AbstractXMLObject that implements a ValidatingXMLObject.
     */
    class XMLTOOL_API AbstractValidatingXMLObject : public virtual AbstractXMLObject, public virtual ValidatingXMLObject
    {
    public:
        virtual ~AbstractValidatingXMLObject();
        
        /**
         * @see ValidatingXMLObject::registerValidator()
         */
        void registerValidator(Validator* validator);
        
        /**
         * @see ValidatingXMLObject::deregisterValidator()
         */
        void deregisterValidator(Validator* validator);

        /**
         * @see ValidatingXMLObject::deregisterAll()
         */
        void deregisterAll();
        
        /**
         * @see ValidatingXMLObject::validate()
         */
        void validate(bool validateDescendants) const;

    protected:
        AbstractValidatingXMLObject() : m_validators(NULL) {}

        /** Copy constructor. */
        AbstractValidatingXMLObject(const AbstractValidatingXMLObject& src);

    private:
        struct XMLTOOL_DLLLOCAL ValidatorWrapper {
            ~ValidatorWrapper(); 
            std::vector<Validator*> v;
        };
    
        mutable ValidatorWrapper* m_validators;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_abstractvalxmlobj_h__ */
