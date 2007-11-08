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
 * @file xmltooling/util/Predicates.h
 * 
 * Useful XMLObject predicates for use with STL algorithms. 
 */

#ifndef __xmltooling_predicates_h__
#define __xmltooling_predicates_h__

#include <xmltooling/XMLObject.h>

#include <functional>

namespace xmltooling {

    /**
     * Predicate that checks the QName of an XMLObject.
     */
    class hasQName
    {
    public:
        /**
         * Constructor.
         * 
         * @param q the QName to check for
         */
        hasQName(const QName& q) : m_q(q) {
        }
        
        /**
         * Returns true iff the provided object's QName matches the constructor argument.
         * 
         * @param xmlObject the object to examine
         */
        bool operator()(const XMLObject* xmlObject) const {
            return xmlObject ? (xmlObject->getElementQName() == m_q) : false;
        }
        
    private:
        const QName& m_q;
    };

    /**
     * Predicate that checks the xsi:type of an XMLObject.
     */
    class hasSchemaType
    {
    public:
        /**
         * Constructor.
         * 
         * @param q the QName to check for
         */
        hasSchemaType(const QName& q) : m_q(q) {
        }
        
        /**
         * Returns true iff the provided object's xsi:type matches the constructor argument.
         * 
         * @param xmlObject the object to examine
         */
        bool operator()(const XMLObject* xmlObject) const {
            const QName* xsitype = xmlObject ? xmlObject->getSchemaType() : NULL;
            return xsitype ? (*xsitype == m_q) : false;
        }
        
    private:
        const QName& m_q;
    };

    /**
     * Template algorithm returns first pointer element from a container that matches a predicate.
     *
     * @param c read-only container of pointer-based objects
     * @param p a predicate function
     * @return  the first object in the container matching the predicate, or NULL
     */
    template<typename Container, typename Predicate>
    typename Container::value_type find_if(const typename Container& c, const typename Predicate& p) {
        Container::const_iterator i = std::find_if(c.begin(), c.end(), p);
        return (i!=c.end()) ? *i : NULL;
    }
    
};

#endif /* __xmltooling_predicates_h__ */
