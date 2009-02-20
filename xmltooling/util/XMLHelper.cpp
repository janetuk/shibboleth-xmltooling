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
 * XMLHelper.cpp
 * 
 * A helper class for working with W3C DOM objects. 
 */

#include "internal.h"
#include "exceptions.h"
#include "util/XMLHelper.h"
#include "util/XMLConstants.h"

#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace xercesc;
using namespace std;

static const XMLCh type[]={chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull };
    
bool XMLHelper::hasXSIType(const DOMElement* e)
{
    if (e) {
        if (e->hasAttributeNS(xmlconstants::XSI_NS, type)) {
            return true;
        }
    }

    return false;
}

xmltooling::QName* XMLHelper::getXSIType(const DOMElement* e)
{
    DOMAttr* attribute = e->getAttributeNodeNS(xmlconstants::XSI_NS, type);
    if (attribute) {
        const XMLCh* attributeValue = attribute->getTextContent();
        if (attributeValue && *attributeValue) {
            int i;
            if ((i=XMLString::indexOf(attributeValue,chColon))>0) {
                XMLCh* prefix=new XMLCh[i+1];
                XMLString::subString(prefix,attributeValue,0,i);
                prefix[i]=chNull;
                xmltooling::QName* ret=new xmltooling::QName(e->lookupNamespaceURI(prefix), attributeValue + i + 1, prefix);
                delete[] prefix;
                return ret;
            }
            else {
                return new xmltooling::QName(e->lookupNamespaceURI(NULL), attributeValue);
            }
        }
    }

    return NULL;
}

DOMAttr* XMLHelper::getIdAttribute(const DOMElement* domElement)
{
    if(!domElement->hasAttributes()) {
        return NULL;
    }
    
    DOMNamedNodeMap* attributes = domElement->getAttributes();
    DOMAttr* attribute;
    for(XMLSize_t i = 0; i < attributes->getLength(); i++) {
        attribute = static_cast<DOMAttr*>(attributes->item(i));
        if(attribute->isId()) {
            return attribute;
        }
    }
    
    return NULL;
}

const XMLObject* XMLHelper::getXMLObjectById(const XMLObject& tree, const XMLCh* id)
{
    if (XMLString::equals(id, tree.getXMLID()))
        return &tree;
    
    const XMLObject* ret;
    const list<XMLObject*>& children = tree.getOrderedChildren();
    for (list<XMLObject*>::const_iterator i=children.begin(); i!=children.end(); ++i) {
        if (*i) {
            ret = getXMLObjectById(*(*i), id);
            if (ret)
                return ret;
        }
    }
    
    return NULL;
}

XMLObject* XMLHelper::getXMLObjectById(XMLObject& tree, const XMLCh* id)
{
    if (XMLString::equals(id, tree.getXMLID()))
        return &tree;
    
    XMLObject* ret;
    const list<XMLObject*>& children = tree.getOrderedChildren();
    for (list<XMLObject*>::const_iterator i=children.begin(); i!=children.end(); ++i) {
        if (*i) {
            ret = getXMLObjectById(*(*i), id);
            if (ret)
                return ret;
        }
    }
    
    return NULL;
}

xmltooling::QName* XMLHelper::getNodeQName(const DOMNode* domNode)
{
    if (domNode)
        return new xmltooling::QName(domNode->getNamespaceURI(), domNode->getLocalName(), domNode->getPrefix());
    return NULL; 
}

xmltooling::QName* XMLHelper::getAttributeValueAsQName(const DOMAttr* attribute)
{
    return getNodeValueAsQName(attribute);
}

xmltooling::QName* XMLHelper::getNodeValueAsQName(const DOMNode* domNode)
{
    if (!domNode)
        return NULL;
    
    int i;
    const XMLCh* value=domNode->getTextContent();
    if (value && (i=XMLString::indexOf(value,chColon))>0) {
        XMLCh* prefix=new XMLCh[i+1];
        XMLString::subString(prefix,value,0,i);
        prefix[i]=chNull;
        xmltooling::QName* ret=new xmltooling::QName(domNode->lookupNamespaceURI(prefix), value + i + 1, prefix);
        delete[] prefix;
        return ret;
    }
    
    return new xmltooling::QName(domNode->lookupNamespaceURI(NULL), value);
}

DOMElement* XMLHelper::appendChildElement(DOMElement* parentElement, DOMElement* childElement)
{
    DOMDocument* parentDocument = parentElement->getOwnerDocument();
    if (childElement->getOwnerDocument() != parentDocument) {
        childElement = static_cast<DOMElement*>(parentDocument->importNode(childElement, true));
    }

    parentElement->appendChild(childElement);
    return childElement;
}

const XMLCh* XMLHelper::getTextContent(const DOMElement* e)
{
    DOMNode* child=e->getFirstChild();
    while (child) {
        if (child->getNodeType()==DOMNode::TEXT_NODE)
            return child->getNodeValue();
        child=child->getNextSibling();
    }
    return NULL;
}

DOMElement* XMLHelper::getFirstChildElement(const DOMNode* n, const XMLCh* localName)
{
    DOMNode* child = n->getFirstChild();
    while (child && child->getNodeType() != DOMNode::ELEMENT_NODE)
        child = child->getNextSibling();
    if (child && localName) {
        if (!XMLString::equals(localName,child->getLocalName()))
            return getNextSiblingElement(child, localName);
    }
    return static_cast<DOMElement*>(child);
}    

DOMElement* XMLHelper::getLastChildElement(const DOMNode* n, const XMLCh* localName)
{
    DOMNode* child = n->getLastChild();
    while (child && child->getNodeType() != DOMNode::ELEMENT_NODE)
        child = child->getPreviousSibling();
    if (child && localName) {
        if (!XMLString::equals(localName,child->getLocalName()))
            return getPreviousSiblingElement(child, localName);
    }
    return static_cast<DOMElement*>(child);
}    

DOMElement* XMLHelper::getFirstChildElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName)
{
    DOMElement* e = getFirstChildElement(n, localName);
    while (e && !XMLString::equals(e->getNamespaceURI(),ns))
        e = getNextSiblingElement(e, localName);
    return e;
}

DOMElement* XMLHelper::getLastChildElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName)
{
    DOMElement* e = getLastChildElement(n, localName);
    while (e && !XMLString::equals(e->getNamespaceURI(),ns))
        e = getPreviousSiblingElement(e, localName);
    return e;
}

DOMElement* XMLHelper::getNextSiblingElement(const DOMNode* n, const XMLCh* localName)
{
    DOMNode* sib = n->getNextSibling();
    while (sib && sib->getNodeType() != DOMNode::ELEMENT_NODE)
        sib = sib->getNextSibling();
    if (sib && localName) {
        if (!XMLString::equals(localName,sib->getLocalName()))
            return getNextSiblingElement(sib, localName);
    }   
    return static_cast<DOMElement*>(sib);
}

DOMElement* XMLHelper::getPreviousSiblingElement(const DOMNode* n, const XMLCh* localName)
{
    DOMNode* sib = n->getPreviousSibling();
    while (sib && sib->getNodeType() != DOMNode::ELEMENT_NODE)
        sib = sib->getPreviousSibling();
    if (sib && localName) {
        if (!XMLString::equals(localName,sib->getLocalName()))
            return getPreviousSiblingElement(sib, localName);
    }   
    return static_cast<DOMElement*>(sib);
}

DOMElement* XMLHelper::getNextSiblingElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName)
{
    DOMElement* e = getNextSiblingElement(n, localName);
    while (e && !XMLString::equals(e->getNamespaceURI(),ns))
        e = getNextSiblingElement(e, localName);
    return e;
}

DOMElement* XMLHelper::getPreviousSiblingElement(const DOMNode* n, const XMLCh* ns, const XMLCh* localName)
{
    DOMElement* e = getPreviousSiblingElement(n, localName);
    while (e && !XMLString::equals(e->getNamespaceURI(),ns))
        e = getPreviousSiblingElement(e, localName);
    return e;
}

void XMLHelper::serialize(const DOMNode* n, std::string& buf, bool pretty)
{
    static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
    static const XMLCh UTF8[]={ chLatin_U, chLatin_T, chLatin_F, chDigit_8, chNull };

    MemBufFormatTarget target;
    DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);

#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
    DOMLSSerializer* serializer = static_cast<DOMImplementationLS*>(impl)->createLSSerializer();
    XercesJanitor<DOMLSSerializer> janitor(serializer);
    if (pretty && serializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, pretty))
        serializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, pretty);
    DOMLSOutput *theOutput = static_cast<DOMImplementationLS*>(impl)->createLSOutput();
    XercesJanitor<DOMLSOutput> j_theOutput(theOutput);
    theOutput->setEncoding(UTF8);
    theOutput->setByteStream(&target);
    if (!serializer->write(n, theOutput))
        throw XMLParserException("unable to serialize XML");
#else
    DOMWriter* serializer = static_cast<DOMImplementationLS*>(impl)->createDOMWriter();
    XercesJanitor<DOMWriter> janitor(serializer);
    serializer->setEncoding(UTF8);
    if (pretty && serializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, pretty))
        serializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, pretty);
    if (!serializer->writeNode(&target, *n))
        throw XMLParserException("unable to serialize XML");
#endif

    buf.erase();
    buf.append(reinterpret_cast<const char*>(target.getRawBuffer()),target.getLen());
}

namespace {
    class StreamFormatTarget : public XMLFormatTarget
    {
    public:
        StreamFormatTarget(std::ostream& out) : m_out(out) {}
        ~StreamFormatTarget() {}

        void writeChars(const XMLByte *const toWrite, const xsecsize_t count, XMLFormatter *const formatter) {
            m_out.write(reinterpret_cast<const char*>(toWrite),count);
        }

        void flush() {
            m_out.flush();
        }

    private:
        std::ostream& m_out;
    };
};

ostream& XMLHelper::serialize(const DOMNode* n, ostream& out, bool pretty)
{
    static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
    static const XMLCh UTF8[]={ chLatin_U, chLatin_T, chLatin_F, chDigit_8, chNull };

    StreamFormatTarget target(out);
    DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);

#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
    DOMLSSerializer* serializer = static_cast<DOMImplementationLS*>(impl)->createLSSerializer();
    XercesJanitor<DOMLSSerializer> janitor(serializer);
    if (pretty && serializer->getDomConfig()->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, pretty))
        serializer->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, pretty);
    DOMLSOutput *theOutput = static_cast<DOMImplementationLS*>(impl)->createLSOutput();
    XercesJanitor<DOMLSOutput> j_theOutput(theOutput);
    theOutput->setEncoding(UTF8);
    theOutput->setByteStream(&target);
    if (!serializer->write(n, theOutput))
        throw XMLParserException("unable to serialize XML");
#else
    DOMWriter* serializer=(static_cast<DOMImplementationLS*>(impl))->createDOMWriter();
    XercesJanitor<DOMWriter> janitor(serializer);
    serializer->setEncoding(UTF8);
    if (pretty && serializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, pretty))
        serializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, pretty);
    if (!serializer->writeNode(&target,*n))
        throw XMLParserException("unable to serialize XML");
#endif

    return out;
}

ostream& xmltooling::operator<<(ostream& ostr, const DOMNode& node)
{
    return XMLHelper::serialize(&node, ostr);
}

ostream& xmltooling::operator<<(ostream& ostr, const XMLObject& obj)
{
    return ostr << *(obj.marshall());
}
