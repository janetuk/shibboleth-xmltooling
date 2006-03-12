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
 * AbstractXMLObjectMarshaller.cpp
 * 
 * A thread-safe abstract marshaller.
 */

#include "internal.h"
#include "DOMCachingXMLObject.h"
#include "exceptions.h"
#include "io/AbstractXMLObjectMarshaller.h"
#include "util/NDC.h"
#include "util/XMLConstants.h"
#include "util/XMLHelper.h"

#include <algorithm>
#include <functional>
#include <xercesc/util/XMLUniDefs.hpp>
#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

#define XT_log (*static_cast<Category*>(m_log))

AbstractXMLObjectMarshaller::AbstractXMLObjectMarshaller()
    : m_log(&Category::getInstance(XMLTOOLING_LOGCAT".Marshaller")) {}

DOMElement* AbstractXMLObjectMarshaller::marshall(XMLObject* xmlObject, DOMDocument* document, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif

    if (XT_log.isDebugEnabled()) {
        XT_log.debug("starting to marshalling %s", xmlObject->getElementQName().toString().c_str());
    }

    DOMCachingXMLObject* dc=dynamic_cast<DOMCachingXMLObject*>(xmlObject);
    if (dc) {
        DOMElement* cachedDOM=dc->getDOM();
        if (cachedDOM) {
            if (!document || document==cachedDOM->getOwnerDocument()) {
                XT_log.debug("XMLObject has a usable cached DOM, reusing it");
                if (document)
                    setDocumentElement(cachedDOM->getOwnerDocument(),cachedDOM);
                dc->releaseParentDOM(true);
                return cachedDOM;
            }
            
            // We have a DOM but it doesn't match the document we were given. This both sucks and blows.
            // Without an adoptNode option to maintain the child pointers, we have to either import the
            // DOM while somehow reassigning all the nested references (which amounts to a complete
            // *unmarshall* operation), or we just release the existing DOM and hope that we can get
            // it back. This depends on all objects being able to preserve their DOM at all costs.
            dc->releaseChildrenDOM(true);
            dc->releaseDOM();
        }
    }
    
    // If we get here, we didn't have a usable DOM (and/or we released the one we had).
    // We may need to create our own document.
    bool bindDocument=false;
    if (!document) {
        document=DOMImplementationRegistry::getDOMImplementation(NULL)->createDocument();
        bindDocument=true;
    }

    try {
        XT_log.debug("creating root element to marshall");
        DOMElement* domElement = document->createElementNS(
            xmlObject->getElementQName().getNamespaceURI(), xmlObject->getElementQName().getLocalPart()
            );
        setDocumentElement(document, domElement);
        marshallInto(*xmlObject, domElement, ctx);

        //Recache the DOM.
        if (dc) {
            XT_log.debug("caching DOM for XMLObject (document is %sbound)", bindDocument ? "" : "not ");
            dc->setDOM(domElement, bindDocument);
            dc->releaseParentDOM(true);
        }

        return domElement;
    }
    catch (...) {
        // Delete the document if need be, and rethrow.
        if (bindDocument) {
            document->release();
        }
        throw;
    }
}

DOMElement* AbstractXMLObjectMarshaller::marshall(XMLObject* xmlObject, DOMElement* parentElement, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif

    if (XT_log.isDebugEnabled()) {
        XT_log.debug("starting to marshalling %s", xmlObject->getElementQName().toString().c_str());
    }

    DOMCachingXMLObject* dc=dynamic_cast<DOMCachingXMLObject*>(xmlObject);
    if (dc) {
        DOMElement* cachedDOM=dc->getDOM();
        if (cachedDOM) {
            if (parentElement->getOwnerDocument()==cachedDOM->getOwnerDocument()) {
                XT_log.debug("XMLObject has a usable cached DOM, reusing it");
                if (parentElement!=cachedDOM->getParentNode()) {
                    parentElement->appendChild(cachedDOM);
                    dc->releaseParentDOM(true);
                }
                return cachedDOM;
            }
            
            // We have a DOM but it doesn't match the document we were given. This both sucks and blows.
            // Without an adoptNode option to maintain the child pointers, we have to either import the
            // DOM while somehow reassigning all the nested references (which amounts to a complete
            // *unmarshall* operation), or we just release the existing DOM and hope that we can get
            // it back. This depends on all objects being able to preserve their DOM at all costs.
            dc->releaseChildrenDOM(true);
            dc->releaseDOM();
        }
    }
    
    // If we get here, we didn't have a usable DOM (and/or we released the one we had).
    XT_log.debug("creating root element to marshall");
    DOMElement* domElement = parentElement->getOwnerDocument()->createElementNS(
        xmlObject->getElementQName().getNamespaceURI(), xmlObject->getElementQName().getLocalPart()
        );
    parentElement->appendChild(domElement);
    marshallInto(*xmlObject, domElement, ctx);

    //Recache the DOM.
    if (dc) {
        XT_log.debug("caching DOM for XMLObject");
        dc->setDOM(domElement, false);
        dc->releaseParentDOM(true);
    }

    return domElement;
}

#ifndef XMLTOOLING_NO_XMLSEC
    class _signit : public unary_function<const pair<Signature*,const SigningContext*>&, void> {
    public:
        void operator()(const pair<Signature*,const SigningContext*>& p) const {
            p.first->sign(p.second);
        }
    };
#endif

void AbstractXMLObjectMarshaller::marshallInto(
    XMLObject& xmlObject, DOMElement* targetElement, MarshallingContext* ctx
    ) const
{
    if (xmlObject.getElementQName().hasPrefix())
        targetElement->setPrefix(xmlObject.getElementQName().getPrefix());
    marshallElementType(xmlObject, targetElement);
    marshallNamespaces(xmlObject, targetElement);
    marshallAttributes(xmlObject, targetElement);
    marshallChildElements(xmlObject, targetElement);
    marshallElementContent(xmlObject, targetElement);

#ifndef XMLTOOLING_NO_XMLSEC
    if (ctx) {
        for_each(ctx->m_signingContexts.begin(),ctx->m_signingContexts.end(),_signit());
    }
#endif
}

void AbstractXMLObjectMarshaller::marshallElementType(XMLObject& xmlObject, DOMElement* domElement) const
{
    const QName* type = xmlObject.getSchemaType();
    if (type) {
        XT_log.debug("setting xsi:type attribute for XMLObject");
        
        const XMLCh* typeLocalName = type->getLocalPart();
        if (!typeLocalName || !*typeLocalName) {
            throw MarshallingException("Schema type of XMLObject may not have an empty local name.");
        }

        static const XMLCh xsitype[] = {
            chLatin_x, chLatin_s, chLatin_i, chColon, chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull
            };
        
        XMLCh* xsivalue=const_cast<XMLCh*>(typeLocalName);
        const XMLCh* prefix=type->getPrefix();
        if (prefix && *prefix) {
            xsivalue=new XMLCh[XMLString::stringLen(typeLocalName) + XMLString::stringLen(prefix) + 2*sizeof(XMLCh)];
            *xsivalue=chNull;
            XMLString::catString(xsivalue,prefix);
            static const XMLCh colon[] = {chColon, chNull};
            XMLString::catString(xsivalue,colon);
            XMLString::catString(xsivalue,typeLocalName);
        }   
        domElement->setAttributeNS(XMLConstants::XSI_NS, xsitype, xsivalue);
        if (xsivalue != typeLocalName)
            XMLString::release(&xsivalue);

        XT_log.debug("Adding XSI namespace to list of namespaces used by XMLObject");
        xmlObject.addNamespace(Namespace(XMLConstants::XSI_NS, XMLConstants::XSI_PREFIX));
    }
}

class _addns : public binary_function<DOMElement*,Namespace,void> {
public:
    void operator()(DOMElement* domElement, const Namespace& ns) const {
        const XMLCh* prefix=ns.getNamespacePrefix();
        const XMLCh* uri=ns.getNamespaceURI();
        
        // Check to see if the prefix is already declared properly above this node.
        if (!ns.alwaysDeclare()) {
            const XMLCh* declared=lookupNamespaceURI(domElement->getParentNode(),prefix);
            if (declared && XMLString::equals(declared,uri))
                return;
        }
            
        if (prefix && *prefix) {
            XMLCh* xmlns=new XMLCh[XMLString::stringLen(XMLConstants::XMLNS_PREFIX) + XMLString::stringLen(prefix) + 2*sizeof(XMLCh)];
            *xmlns=chNull;
            XMLString::catString(xmlns,XMLConstants::XMLNS_PREFIX);
            static const XMLCh colon[] = {chColon, chNull};
            XMLString::catString(xmlns,colon);
            XMLString::catString(xmlns,prefix);
            domElement->setAttributeNS(XMLConstants::XMLNS_NS, xmlns, uri);
        }
        else {
            domElement->setAttributeNS(XMLConstants::XMLNS_NS, XMLConstants::XMLNS_PREFIX, uri);
        }
    }

    const XMLCh* lookupNamespaceURI(const DOMNode* n, const XMLCh* prefix) const {
        // Return NULL if no declaration in effect. The empty string signifies the null namespace.
        if (!n || n->getNodeType()!=DOMNode::ELEMENT_NODE) {
            // At the root, the default namespace is set to the null namespace.
            if (!prefix || !*prefix)
                return &chNull;
            return NULL;    // we're done
        }
        DOMNamedNodeMap* attributes = static_cast<const DOMElement*>(n)->getAttributes();
        if (!attributes)
            return lookupNamespaceURI(n->getParentNode(),prefix);   // defer to parent
        DOMNode* childNode;
        DOMAttr* attribute;
        for (XMLSize_t i=0; i<attributes->getLength(); i++) {
            childNode = attributes->item(i);
            if (childNode->getNodeType() != DOMNode::ATTRIBUTE_NODE)    // not an attribute?
                continue;
            attribute = static_cast<DOMAttr*>(childNode);
            if (!XMLString::equals(attribute->getNamespaceURI(),XMLConstants::XMLNS_NS))
                continue;   // not a namespace declaration
            // Local name should be the prefix and the value would be the URI, except for the default namespace.
            if ((!prefix || !*prefix) && XMLString::equals(attribute->getLocalName(),XMLConstants::XMLNS_PREFIX))
                return attribute->getNodeValue();
            else if (XMLString::equals(prefix,attribute->getLocalName()))
                return attribute->getNodeValue();
        }
        // Defer to parent.
        return lookupNamespaceURI(n->getParentNode(),prefix);
    }
};

void AbstractXMLObjectMarshaller::marshallNamespaces(const XMLObject& xmlObject, DOMElement* domElement) const
{
    XT_log.debug("marshalling namespace attributes for XMLObject");
    const set<Namespace>& namespaces = xmlObject.getNamespaces();
    for_each(namespaces.begin(),namespaces.end(),bind1st(_addns(),domElement));
}

class _marshallchild : public binary_function<XMLObject*,DOMElement*,void> {
    void* m_log;
public:
    _marshallchild(void* log) : m_log(log) {}
    void operator()(XMLObject* obj, DOMElement* element) const {
        if (!obj)
            return;
        if (XT_log.isDebugEnabled()) {
            XT_log.debug("getting marshaller for child XMLObject: %s", obj->getElementQName().toString().c_str());
        }

        const Marshaller* marshaller = Marshaller::getMarshaller(obj);
        if (!marshaller) {
            XT_log.error(
                "no default unmarshaller installed, unknown child object: %s",
                obj->getElementQName().toString().c_str()
                );
            throw MarshallingException("Marshaller found unknown child element, but no default marshaller was found.");
        }
        marshaller->marshall(obj, element);
    }
};

void AbstractXMLObjectMarshaller::marshallChildElements(const XMLObject& xmlObject, DOMElement* domElement) const
{
    XT_log.debug("marshalling child elements for XMLObject");

    const list<XMLObject*>& children=xmlObject.getOrderedChildren();
    for_each(children.begin(),children.end(),bind2nd(_marshallchild(m_log),domElement));
}
