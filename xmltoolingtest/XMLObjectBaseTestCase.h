/*
 *  Copyright 2001-2005 Internet2
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

#include <cxxtest/TestSuite.h>
#include <xmltooling/AbstractAttributeExtensibleXMLObject.h>
#include <xmltooling/AbstractDOMCachingXMLObject.h>
#include <xmltooling/AbstractElementProxy.h>
#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>
#ifndef XMLTOOLING_NO_XMLSEC
    #include <xmltooling/signature/Signature.h>
#endif
#include <xmltooling/util/ParserPool.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/util/XMLHelper.h>
#include <xmltooling/util/XMLObjectChildrenList.h>

using namespace xmltooling;
using namespace std;

extern ParserPool* validatingPool;
extern ParserPool* nonvalidatingPool;
extern string data_path;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

class SimpleXMLObject : public AbstractDOMCachingXMLObject
{
public:
    static const XMLCh NAMESPACE[];
    static const XMLCh NAMESPACE_PREFIX[];
    static const XMLCh LOCAL_NAME[];
    static const XMLCh ID_ATTRIB_NAME[];

    SimpleXMLObject() : AbstractDOMCachingXMLObject(NAMESPACE, LOCAL_NAME, NAMESPACE_PREFIX), m_id(NULL), m_value(NULL) {
        m_children.push_back(NULL);
        m_signature=m_children.begin();
    }

    virtual ~SimpleXMLObject() {
        XMLString::release(&m_id);
        XMLString::release(&m_value);
    }
    
    const XMLCh* getId() const { return m_id; }
    void setId(const XMLCh* id) { m_id=prepareForAssignment(m_id,id); }

    const XMLCh* getValue() const { return m_value; }
    void setValue(const XMLCh* value) { m_value=prepareForAssignment(m_value,value); }

#ifndef XMLTOOLING_NO_XMLSEC    
    Signature* getSignature() const {
        return dynamic_cast<Signature*>(*m_signature);
    }

    void setSignature(Signature* sig) {
        *m_signature=prepareForAssignment(*m_signature,sig);
    }
#endif

    VectorOf(SimpleXMLObject) getSimpleXMLObjects() {
        return VectorOf(SimpleXMLObject)(this, m_simples, &m_children, m_children.end());
    }
    
    SimpleXMLObject* clone() const {
        auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone());
        SimpleXMLObject* ret=dynamic_cast<SimpleXMLObject*>(domClone.get());
        if (ret) {
            domClone.release();
            return ret;
        }

        ret=new SimpleXMLObject();
        ret->m_namespaces=m_namespaces;
        ret->setId(m_id);
        ret->setValue(m_value);
        xmltooling::clone(m_children, ret->m_children);
        return ret;
    }

private:
    XMLCh* m_id;
    XMLCh* m_value;
    vector<SimpleXMLObject*> m_simples;
    list<XMLObject*>::iterator m_signature;
};

class SimpleXMLObjectBuilder : public XMLObjectBuilder
{
public:
    SimpleXMLObject* buildObject() const {
        return new SimpleXMLObject();
    }
};

class SimpleXMLObjectMarshaller : public AbstractXMLObjectMarshaller
{
public:
    SimpleXMLObjectMarshaller() {}

private:
    void marshallAttributes(const XMLObject& xmlObject, DOMElement* domElement) const {
        const SimpleXMLObject& simpleXMLObject = dynamic_cast<const SimpleXMLObject&>(xmlObject);
        
        if(simpleXMLObject.getId()) {
            domElement->setAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME, simpleXMLObject.getId());
            domElement->setIdAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME);
        }
    }

    void marshallElementContent(const XMLObject& xmlObject, DOMElement* domElement) const {
        const SimpleXMLObject& simpleXMLObject = dynamic_cast<const SimpleXMLObject&>(xmlObject);

        if(simpleXMLObject.getValue()) {
            domElement->setTextContent(simpleXMLObject.getValue());
        }
    }
};

class SimpleXMLObjectUnmarshaller : public AbstractXMLObjectUnmarshaller
{
public:
    SimpleXMLObjectUnmarshaller() {}

private:
    void processChildElement(XMLObject& parentXMLObject, XMLObject* childXMLObject, const DOMElement* root) const {
        SimpleXMLObject& simpleXMLObject = dynamic_cast<SimpleXMLObject&>(parentXMLObject);

        if (XMLHelper::isNodeNamed(root, SimpleXMLObject::NAMESPACE, SimpleXMLObject::LOCAL_NAME))
            simpleXMLObject.getSimpleXMLObjects().push_back(dynamic_cast<SimpleXMLObject*>(childXMLObject));
        else if (XMLHelper::isNodeNamed(root, XMLConstants::XMLSIG_NS, Signature::LOCAL_NAME))
            simpleXMLObject.setSignature(dynamic_cast<Signature*>(childXMLObject));
        else
            throw UnmarshallingException("Unknown child element cannot be added to parent object.");
    }

    void processAttribute(XMLObject& xmlObject, const DOMAttr* attribute) const {
        SimpleXMLObject& simpleXMLObject = dynamic_cast<SimpleXMLObject&>(xmlObject);

        if (XMLHelper::isNodeNamed(attribute, NULL, SimpleXMLObject::ID_ATTRIB_NAME))
            simpleXMLObject.setId(attribute->getValue());
        else
            throw UnmarshallingException("Unknown attribute cannot be processed by parent object.");
    }

    void processElementContent(XMLObject& xmlObject, const XMLCh* elementContent) const {
        SimpleXMLObject& simpleXMLObject = dynamic_cast<SimpleXMLObject&>(xmlObject);
        
        simpleXMLObject.setValue(elementContent);
    }

};

class WildcardXMLObjectMarshaller;

class WildcardXMLObject : public AbstractElementProxy, public AbstractAttributeExtensibleXMLObject
{
    friend class WildcardXMLObjectMarshaller;
public:
    WildcardXMLObject(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix)
        : AbstractDOMCachingXMLObject(nsURI, localName, prefix),
        AbstractElementProxy(nsURI, localName, prefix),
        AbstractAttributeExtensibleXMLObject(nsURI, localName, prefix) {}
    virtual ~WildcardXMLObject() {}
    
    WildcardXMLObject* clone() const {
        auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone());
        WildcardXMLObject* ret=dynamic_cast<WildcardXMLObject*>(domClone.get());
        if (ret) {
            domClone.release();
            return ret;
        }

        ret=new WildcardXMLObject(
            getElementQName().getNamespaceURI(),getElementQName().getLocalPart(),getElementQName().getPrefix()
            );
        ret->m_namespaces=m_namespaces;
        for (map<QName,XMLCh*>::const_iterator i=m_attributeMap.begin(); i!=m_attributeMap.end(); i++) {
            ret->m_attributeMap[i->first]=XMLString::replicate(i->second);
        }
        ret->setTextContent(getTextContent());
        xmltooling::clone(m_children, ret->m_children);
        return ret;
    }
};

class WildcardXMLObjectBuilder : public XMLObjectBuilder
{
public:
    XMLObject* buildObject() const {
        throw XMLObjectException("No default builder available.");
    }

    WildcardXMLObject* buildObject(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix) const {
        return new WildcardXMLObject(nsURI,localName,prefix);
    }
};

class WildcardXMLObjectMarshaller : public AbstractXMLObjectMarshaller
{
public:
    WildcardXMLObjectMarshaller() : AbstractXMLObjectMarshaller() {}

private:
    void marshallAttributes(const XMLObject& xmlObject, DOMElement* domElement) const {
        const WildcardXMLObject& wcXMLObject = dynamic_cast<const WildcardXMLObject&>(xmlObject);

        for (map<QName,XMLCh*>::const_iterator i=wcXMLObject.m_attributeMap.begin(); i!=wcXMLObject.m_attributeMap.end(); i++) {
            DOMAttr* attr=domElement->getOwnerDocument()->createAttributeNS(i->first.getNamespaceURI(),i->first.getLocalPart());
            if (i->first.hasPrefix())
                attr->setPrefix(i->first.getPrefix());
            attr->setNodeValue(i->second);
            domElement->setAttributeNode(attr);
        }
    }

    void marshallElementContent(const XMLObject& xmlObject, DOMElement* domElement) const {
        const WildcardXMLObject& wcXMLObject = dynamic_cast<const WildcardXMLObject&>(xmlObject);

        if(wcXMLObject.getTextContent()) {
            domElement->appendChild(domElement->getOwnerDocument()->createTextNode(wcXMLObject.getTextContent()));
        }
    }
};

class WildcardXMLObjectUnmarshaller : public AbstractXMLObjectUnmarshaller
{
public:
    WildcardXMLObjectUnmarshaller() {}

private:
    XMLObject* buildXMLObject(const DOMElement* domElement) const {
        const WildcardXMLObjectBuilder* builder =
            dynamic_cast<const WildcardXMLObjectBuilder*>(XMLObjectBuilder::getBuilder(domElement));
        if (builder)
            return builder->buildObject(domElement->getNamespaceURI(),domElement->getLocalName(),domElement->getPrefix());
        throw UnmarshallingException("Failed to locate WildcardObjectBuilder for element.");
    }

    void processChildElement(XMLObject& parentXMLObject, XMLObject* childXMLObject, const DOMElement* root) const {
        WildcardXMLObject& wcXMLObject = dynamic_cast<WildcardXMLObject&>(parentXMLObject);

        wcXMLObject.getXMLObjects().push_back(childXMLObject);
    }

    void processAttribute(XMLObject& xmlObject, const DOMAttr* attribute) const {
        WildcardXMLObject& wcXMLObject = dynamic_cast<WildcardXMLObject&>(xmlObject);
       
        QName q(attribute->getNamespaceURI(),attribute->getLocalName(),attribute->getPrefix()); 
        wcXMLObject.setAttribute(q,attribute->getNodeValue());
    }

    void processElementContent(XMLObject& xmlObject, const XMLCh* elementContent) const {
        WildcardXMLObject& wcXMLObject = dynamic_cast<WildcardXMLObject&>(xmlObject);
        
        wcXMLObject.setTextContent(elementContent);
    }

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif
