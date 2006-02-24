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
 * @file Marshaller.h
 * 
 * Transforms XMLObjects into DOM trees
 */

#if !defined(__xmltooling_marshaller_h__)
#define __xmltooling_marshaller_h__

#include <map>
#include <xercesc/dom/DOM.hpp>
#include <xmltooling/XMLObject.h>

using namespace xercesc;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Marshallers are used to marshall an XMLObject into a W3C DOM element.
     */
    class XMLTOOL_API Marshaller
    {
    MAKE_NONCOPYABLE(Marshaller);
    public:
        Marshaller() {}
        virtual ~Marshaller() {}
        
        /**
         * Marshall this element, and its children, into a W3C DOM element rooted in a given document.
         * If no document is provided, then the marshaller will create one, and bind it to the
         * object being marshalled.
         * 
         * @param xmlObject the object to marshall
         * @param document the DOM document the marshalled element will be rooted in
         * 
         * @return the W3C DOM element representing this XML element
         * 
         * @throws MarshallingException thrown if there is a problem marshalling the given object
         */
        virtual DOMElement* marshall(XMLObject* xmlObject, DOMDocument* document=NULL) const=0;

        /**
         * Retrieves a Marshaller using the key it was registered with.
         * 
         * @param key the key used to register the marshaller
         * @return the marshaller or NULL
         */
        static const Marshaller* getMarshaller(const QName& key) {
            std::map<QName,Marshaller*>::const_iterator i=m_map.find(key);
            return (i==m_map.end()) ? NULL : i->second;
        }

        /**
         * Retrieves a Marshaller for an XML object
         * If no match is found, the default marshaller is returned, if any.
         * 
         * @param xmlObject the object for which to return a marshaller
         * @return the marshaller or NULL
         */
        static const Marshaller* getMarshaller(const XMLObject* xmlObject);

        /**
         * Retrieves default Marshaller for DOM elements
         * 
         * @return the default marshaller or NULL
         */
        static const Marshaller* getDefaultMarshaller() {
            return m_default;
        }
    
        /**
         * Gets an immutable list of all the marshallers currently registered.
         * 
         * @return list of all the marshallers currently registered
         */
        static const std::map<QName,Marshaller*>& getMarshallers() {
            return m_map;
        }
    
        /**
         * Registers a new marshaller for the given key.
         * 
         * @param key the key used to retrieve this marshaller later
         * @param marshaller the marshaller
         */
        static void registerMarshaller(const QName& key, Marshaller* marshaller) {
            deregisterMarshaller(key);
            m_map[key]=marshaller;
        }

        /**
         * Registers default marshaller
         * 
         * @param marshaller the default marshaller
         */
        static void registerDefaultMarshaller(Marshaller* marshaller) {
            deregisterDefaultMarshaller();
            m_default=marshaller;
        }

        /**
         * Deregisters a marshaller.
         * 
         * @param key the key for the marshaller to be deregistered
         */
        static void deregisterMarshaller(const QName& key) {
            delete getMarshaller(key);
            m_map.erase(key);
        }

        /**
         * Deregisters default marshaller.
         */
        static void deregisterDefaultMarshaller() {
            delete m_default;
            m_default=NULL;
        }

        /**
         * Unregisters and destroys all registered marshallers. 
         */
        static void destroyMarshallers();
    
    private:
        static std::map<QName,Marshaller*> m_map;
        static Marshaller* m_default;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_marshaller_h__ */
