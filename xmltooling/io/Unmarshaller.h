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
 * @file Unmarshaller.h
 * 
 * Transforms DOM trees into XMLObjects
 */

#if !defined(__xmltooling_unmarshaller_h__)
#define __xmltooling_unmarshaller_h__

#include <xercesc/dom/DOM.hpp>
#include <xmltooling/XMLObject.h>

using namespace xercesc;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Unmarshallers are used to unmarshall a W3C DOM element into an XMLObject.
     */
    class XMLTOOL_API Unmarshaller
    {
    MAKE_NONCOPYABLE(Unmarshaller);
    public:
        Unmarshaller() {}
        virtual ~Unmarshaller() {}
        
        /**
         * Unmarshalls the given W3C DOM element into an XMLObject.
         * The root of a given XML construct should be unmarshalled with the bindDocument parameter
         * set to true.
         * 
         * @param element       the DOM element to unmarshall
         * @param bindDocument  true iff the resulting XMLObject should take ownership of the DOM's Document 
         * 
         * @return the unmarshalled XMLObject
         * 
         * @throws UnmarshallingException thrown if an error occurs unmarshalling the DOM element into the XMLObject
         */
        virtual XMLObject* unmarshall(DOMElement* element, bool bindDocument=false) const=0;

        /**
         * Retrieves a unmarshaller using the key it was registered with.
         * 
         * @param key the key used to register the unmarshaller
         * @return the unmarshaller
         */
        static const Unmarshaller* getUnmarshaller(const QName& key) {
            std::map<QName,Unmarshaller*>::const_iterator i=m_map.find(key);
            return (i==m_map.end()) ? NULL : i->second;
        }
    
        /**
         * Gets an immutable list of all the unmarshallers currently registered.
         * 
         * @return list of all the unmarshallers currently registered
         */
        static const std::map<QName,Unmarshaller*>& getUnmarshaller() {
            return m_map;
        }
    
        /**
         * Registers a new unmarshaller for the given key.
         * 
         * @param key the key used to retrieve this unmarshaller later
         * @param unmarshaller the unmarshaller
         */
        static void registerUnmarshaller(const QName& key, Unmarshaller* unmarshaller) {
            m_map[key]=unmarshaller;
        }
    
        /**
         * Deregisters a unmarshaller.
         * 
         * @param key the key for the unmarshaller to be deregistered
         */
        static void deregisterUnmarshaller(const QName& key) {
            delete getUnmarshaller(key);
            m_map.erase(key);
        }
        
        /**
         * Unregisters and destroys all registered unmarshallers. 
         */
        static void destroyUnmarshallers();
    
    private:
        static std::map<QName,Unmarshaller*> m_map;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_unmarshaller_h__ */
