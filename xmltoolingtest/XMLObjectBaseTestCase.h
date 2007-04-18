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

#include <cxxtest/TestSuite.h>
#include <xmltooling/AbstractComplexElement.h>
#include <xmltooling/ElementProxy.h>
#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>
#include <xmltooling/impl/AnyElement.h>
#include <xmltooling/impl/UnknownElement.h>
#include <xmltooling/util/ParserPool.h>
#include <xmltooling/util/XMLConstants.h>
#include <xmltooling/util/XMLHelper.h>
#include <xmltooling/util/XMLObjectChildrenList.h>

#ifndef XMLTOOLING_NO_XMLSEC
    #include <xmltooling/signature/Signature.h>
    using namespace xmlsignature;
#endif

using namespace xmltooling;
using namespace xercesc;
using namespace std;

extern string data_path;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

class SimpleXMLObject
    : public AbstractComplexElement,
        public AbstractDOMCachingXMLObject,
        public AbstractXMLObjectMarshaller,
        public AbstractXMLObjectUnmarshaller
{
protected:
    SimpleXMLObject(const SimpleXMLObject& src)
            : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src),
                m_id(XMLString::replicate(src.m_id)) {
#ifndef XMLTOOLING_NO_XMLSEC
        m_children.push_back(NULL);
        m_signature=m_children.begin();
#endif
        VectorOf(SimpleXMLObject) mine=getSimpleXMLObjects();
        for (vector<SimpleXMLObject*>::const_iterator i=src.m_simples.begin(); i!=src.m_simples.end(); i++) {
            mine.push_back(dynamic_cast<SimpleXMLObject*>((*i)->clone()));
        }
    }

public:
    static const XMLCh NAMESPACE[];
    static const XMLCh NAMESPACE_PREFIX[];
    static const XMLCh LOCAL_NAME[];
    static const XMLCh DERIVED_NAME[];
    static const XMLCh TYPE_NAME[];
    static const XMLCh ID_ATTRIB_NAME[];

    SimpleXMLObject(
        const XMLCh* nsURI=NULL, const XMLCh* localName=NULL, const XMLCh* prefix=NULL, const QName* schemaType=NULL
        ) : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_id(NULL) {
#ifndef XMLTOOLING_NO_XMLSEC
        m_children.push_back(NULL);
        m_signature=m_children.begin();
#endif
    }

    virtual ~SimpleXMLObject() {
        XMLString::release(&m_id);
    }

    XMLObject* clone() const {
        auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone());
        SimpleXMLObject* ret=dynamic_cast<SimpleXMLObject*>(domClone.get());
        if (ret) {
            domClone.release();
            return ret;
        }

        return new SimpleXMLObject(*this);
    }

    const XMLCh* getXMLID() const { return getId(); }
    const XMLCh* getId() const { return m_id; }
    void setId(const XMLCh* id) { m_id=prepareForAssignment(m_id,id); }

    const XMLCh* getValue() const { return getTextContent(); }
    void setValue(const XMLCh* value) { setTextContent(value); }

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
    
    const std::vector<SimpleXMLObject*>& getSimpleXMLObjects() const {
        return m_simples;
    }

protected:
    void marshallAttributes(xercesc::DOMElement* domElement) const {
        if(getId()) {
            domElement->setAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME, getId());
            domElement->setIdAttributeNS(NULL, SimpleXMLObject::ID_ATTRIB_NAME);
        }
    }

    void processChildElement(XMLObject* childXMLObject, const xercesc::DOMElement* root) {
        SimpleXMLObject* simple=dynamic_cast<SimpleXMLObject*>(childXMLObject);
        if (simple) {
            getSimpleXMLObjects().push_back(simple);
            return;
        }
        
#ifndef XMLTOOLING_NO_XMLSEC
        Signature* sig=dynamic_cast<Signature*>(childXMLObject);
        if (sig) {
            setSignature(sig);
            return;
        }
#endif

        throw UnmarshallingException("Unknown child element cannot be added to parent object.");
    }

    void processAttribute(const xercesc::DOMAttr* attribute) {
        if (XMLHelper::isNodeNamed(attribute, NULL, SimpleXMLObject::ID_ATTRIB_NAME))
            setId(attribute->getValue());
        else
            throw UnmarshallingException("Unknown attribute cannot be processed by parent object.");
    }

private:
    XMLCh* m_id;
    vector<SimpleXMLObject*> m_simples;
#ifndef XMLTOOLING_NO_XMLSEC
    list<XMLObject*>::iterator m_signature;
#endif
};

class SimpleXMLObjectBuilder : public XMLObjectBuilder
{
public:
    XMLObject* buildObject() const {
        return buildObject(SimpleXMLObject::NAMESPACE, SimpleXMLObject::LOCAL_NAME, SimpleXMLObject::NAMESPACE_PREFIX);
    }

    XMLObject* buildObject(
        const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL
        ) const {
        return new SimpleXMLObject(nsURI, localName, prefix, schemaType);
    }

    static SimpleXMLObject* buildSimpleXMLObject() {
        const SimpleXMLObjectBuilder* b = dynamic_cast<const SimpleXMLObjectBuilder*>(
            XMLObjectBuilder::getBuilder(QName(SimpleXMLObject::NAMESPACE,SimpleXMLObject::LOCAL_NAME))
            );
        if (b)
            return dynamic_cast<SimpleXMLObject*>(b->buildObject());
        throw XMLObjectException("Unable to obtain typed builder for SimpleXMLObject.");
    }
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif
