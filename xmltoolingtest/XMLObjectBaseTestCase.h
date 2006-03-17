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

class SimpleXMLObject : public AbstractXMLObjectMarshaller, public AbstractXMLObjectUnmarshaller
{
public:
    static const XMLCh NAMESPACE[];
    static const XMLCh NAMESPACE_PREFIX[];
    static const XMLCh LOCAL_NAME[];
    static const XMLCh ID_ATTRIB_NAME[];

    SimpleXMLObject() : AbstractDOMCachingXMLObject(NAMESPACE, LOCAL_NAME, NAMESPACE_PREFIX), m_id(NULL), m_value(NULL) {
#ifndef XMLTOOLING_NO_XMLSEC
        m_children.push_back(NULL);
        m_signature=m_children.begin();
#endif
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

    void marshallAttributes(DOMElement* domElement) const {
        if(getId()) {
            domElement->setAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME, getId());
            domElement->setIdAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME);
        }
    }

    void marshallElementContent(DOMElement* domElement) const {
        if(getValue()) {
            domElement->setTextContent(getValue());
        }
    }

    void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
        if (XMLHelper::isNodeNamed(root, SimpleXMLObject::NAMESPACE, SimpleXMLObject::LOCAL_NAME))
            getSimpleXMLObjects().push_back(dynamic_cast<SimpleXMLObject*>(childXMLObject));
#ifndef XMLTOOLING_NO_XMLSEC
        else if (XMLHelper::isNodeNamed(root, XMLConstants::XMLSIG_NS, Signature::LOCAL_NAME))
            setSignature(dynamic_cast<Signature*>(childXMLObject));
#endif
        else
            throw UnmarshallingException("Unknown child element cannot be added to parent object.");
    }

    void processAttribute(const DOMAttr* attribute) {
        if (XMLHelper::isNodeNamed(attribute, NULL, SimpleXMLObject::ID_ATTRIB_NAME))
            setId(attribute->getValue());
        else
            throw UnmarshallingException("Unknown attribute cannot be processed by parent object.");
    }

    void processElementContent(const XMLCh* elementContent) {
        setValue(elementContent);
    }

private:
    XMLCh* m_id;
    XMLCh* m_value;
    vector<SimpleXMLObject*> m_simples;
#ifndef XMLTOOLING_NO_XMLSEC
    list<XMLObject*>::iterator m_signature;
#endif
};

class SimpleXMLObjectBuilder : public XMLObjectBuilder
{
public:
    SimpleXMLObject* buildObject(const DOMElement* e=NULL) const {
        return new SimpleXMLObject();
    }
};

class WildcardXMLObject : public AbstractElementProxy, public AbstractAttributeExtensibleXMLObject,
    public AbstractXMLObjectMarshaller, public AbstractXMLObjectUnmarshaller
{
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

    void marshallAttributes(DOMElement* domElement) const {
        for (map<QName,XMLCh*>::const_iterator i=m_attributeMap.begin(); i!=m_attributeMap.end(); i++) {
            DOMAttr* attr=domElement->getOwnerDocument()->createAttributeNS(i->first.getNamespaceURI(),i->first.getLocalPart());
            if (i->first.hasPrefix())
                attr->setPrefix(i->first.getPrefix());
            attr->setNodeValue(i->second);
            domElement->setAttributeNode(attr);
        }
    }

    void marshallElementContent(DOMElement* domElement) const {
        if(getTextContent()) {
            domElement->appendChild(domElement->getOwnerDocument()->createTextNode(getTextContent()));
        }
    }

    void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
        getXMLObjects().push_back(childXMLObject);
    }

    void processAttribute(const DOMAttr* attribute) {
        QName q(attribute->getNamespaceURI(),attribute->getLocalName(),attribute->getPrefix()); 
        setAttribute(q,attribute->getNodeValue());
    }

    void processElementContent(const XMLCh* elementContent) {
        setTextContent(elementContent);
    }
};

class WildcardXMLObjectBuilder : public XMLObjectBuilder
{
public:
    WildcardXMLObject* buildObject(const DOMElement* e=NULL) const {
        return new WildcardXMLObject(e->getNamespaceURI(),e->getLocalName(),e->getPrefix());
    }
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif
