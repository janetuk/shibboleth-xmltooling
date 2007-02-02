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
 * @file XMLObjectBuilder.h
 * 
 * Factory interface for XMLObjects 
 */

#if !defined(__xmltooling_xmlobjbuilder_h__)
#define __xmltooling_xmlobjbuilder_h__

#include <map>
#include <xmltooling/QName.h>
#include <xmltooling/XMLObject.h>
#include <xmltooling/util/XMLHelper.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A factory interface for obtaining XMLObjects.
     * Subclasses MAY supply additional factory methods.
     */
    class XMLTOOL_API XMLObjectBuilder
    {
    MAKE_NONCOPYABLE(XMLObjectBuilder);
    public:
        virtual ~XMLObjectBuilder() {}
        
        /**
         * Creates an empty XMLObject with a particular element name.
         * The results are undefined if localName is NULL or empty.
         * 
         * @param nsURI         namespace URI for element
         * @param localName     local name of element
         * @param prefix        prefix of element name
         * @param schemaType    xsi:type of the object
         * @return the empty XMLObject
         */
        virtual XMLObject* buildObject(
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL
            ) const=0;

        /**
         * Creates an empty XMLObject with a particular element name.
         * 
         * @param q     QName of element for object
         * @return the empty XMLObject
         */
        XMLObject* buildFromQName(const QName& q) const {
            return buildObject(q.getNamespaceURI(),q.getLocalPart(),q.getPrefix());
        }

        /**
         * Creates an unmarshalled XMLObject from a DOM Element.
         * 
         * @param element       the unmarshalling source
         * @param bindDocument  true iff the XMLObject should take ownership of the DOM Document
         * @return the unmarshalled XMLObject
         */
        XMLObject* buildFromElement(DOMElement* element, bool bindDocument=false) const {
            std::auto_ptr<XMLObject> ret(
                buildObject(element->getNamespaceURI(),element->getLocalName(),element->getPrefix(),XMLHelper::getXSIType(element))
                );
            ret->unmarshall(element,bindDocument);
            return ret.release();
        }

        /**
         * Creates an unmarshalled XMLObject from the root of a DOM Document.
         * 
         * @param doc           the unmarshalling source
         * @param bindDocument  true iff the XMLObject should take ownership of the DOM Document
         * @return the unmarshalled XMLObject
         */
        XMLObject* buildFromDocument(DOMDocument* doc, bool bindDocument=true) const {
            return buildFromElement(doc->getDocumentElement(),bindDocument);
        }

        /**
         * Creates an unmarshalled XMLObject using the default build method, if a builder can be found.
         * 
         * @param element       the unmarshalling source
         * @param bindDocument  true iff the new XMLObject should take ownership of the DOM Document
         * @return  the unmarshalled object or NULL if no builder is available 
         */
        static XMLObject* buildOneFromElement(DOMElement* element, bool bindDocument=false) {
            const XMLObjectBuilder* b=getBuilder(element);
            return b ? b->buildFromElement(element,bindDocument) : NULL;
        }

        /**
         * Retrieves an XMLObjectBuilder using the key it was registered with.
         * 
         * @param key the key used to register the builder
         * @return the builder or NULL
         */
        static const XMLObjectBuilder* getBuilder(const QName& key) {
            std::map<QName,XMLObjectBuilder*>::const_iterator i=m_map.find(key);
            return (i==m_map.end()) ? NULL : i->second;
        }

        /**
         * Retrieves an XMLObjectBuilder for a given DOM element.
         * If no match is found, the default builder is returned, if any.
         * 
         * @param element the element for which to locate a builder
         * @return the builder or NULL
         */
        static const XMLObjectBuilder* getBuilder(const DOMElement* element);

        /**
         * Retrieves the default XMLObjectBuilder for DOM elements
         * 
         * @return the default builder or NULL
         */
        static const XMLObjectBuilder* getDefaultBuilder() {
            return m_default;
        }

        /**
         * Gets an immutable list of all the builders currently registered.
         * 
         * @return list of all the builders currently registered
         */
        static const std::map<QName,XMLObjectBuilder*>& getBuilders() {
            return m_map;
        }
    
        /**
         * Registers a new builder for the given key.
         * 
         * @param builderKey the key used to retrieve this builder later
         * @param builder the builder
         */
        static void registerBuilder(const QName& builderKey, XMLObjectBuilder* builder) {
            deregisterBuilder(builderKey);
            m_map[builderKey]=builder;
        }

        /**
         * Registers a default builder
         * 
         * @param builder the default builder
         */
        static void registerDefaultBuilder(XMLObjectBuilder* builder) {
            deregisterDefaultBuilder();
            m_default=builder;
        }

        /**
         * Deregisters a builder.
         * 
         * @param builderKey the key for the builder to be deregistered
         */
        static void deregisterBuilder(const QName& builderKey) {
            delete getBuilder(builderKey);
            m_map.erase(builderKey);
        }

        /**
         * Deregisters default builder.
         */
        static void deregisterDefaultBuilder() {
            delete m_default;
            m_default=NULL;
        }

        /**
         * Unregisters and destroys all registered builders. 
         */
        static void destroyBuilders();

    protected:
        XMLObjectBuilder() {}
    
    private:
        static std::map<QName,XMLObjectBuilder*> m_map;
        static XMLObjectBuilder* m_default;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_xmlobjbuilder_h__ */
