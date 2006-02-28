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
 * UnknownElement.cpp
 * 
 * Basic implementations suitable for use as defaults for unrecognized content
 */

#include "internal.h"
#include "exceptions.h"
#include "impl/UnknownElement.h"
#include "util/NDC.h"

#include <log4cpp/Category.hh>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

void UnknownElementImpl::releaseDOM()
{
#ifdef _DEBUG
    xmltooling::NDC ndc("releaseDOM");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".UnknownElementImpl");
    log.debug("releasing DOM for unknown content, preserving current DOM in XML form");

    // We're losing our DOM, so assuming we have one, we preserve it.
    serialize(m_xml);

    // This takes care of the generic housekeeping now that we've preserved things.
    AbstractDOMCachingXMLObject::releaseDOM();
}

XMLObject* UnknownElementImpl::clone() const
{
    UnknownElementImpl* ret=new UnknownElementImpl();

    // If there's no XML locally, serialize this object into the new one.
    // Otherwise just copy it over.
    if (m_xml.empty())
        serialize(ret->m_xml);
    else
        ret->m_xml=m_xml;

    return ret;
}

void UnknownElementImpl::serialize(string& s) const
{
    if (getDOM()) {
        static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
        static const XMLCh UTF8[]={ chLatin_U, chLatin_T, chLatin_F, chDigit_8, chNull };
        DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);
        DOMWriter* serializer=(static_cast<DOMImplementationLS*>(impl))->createDOMWriter();
        serializer->setEncoding(UTF8);
        try {
            MemBufFormatTarget target;
            if (!serializer->writeNode(&target,*(getDOM())))
                throw XMLObjectException("unable to serialize XML to preserve DOM");
            s.erase();
            s.append(reinterpret_cast<const char*>(target.getRawBuffer()),target.getLen());
            serializer->release();
        }
        catch (...) {
            serializer->release();
            throw;
        }
    }
}

DOMElement* UnknownElementMarshaller::marshall(XMLObject* xmlObject, DOMDocument* document) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Marshaller");
    log.debug("marshalling unknown content");

    UnknownElementImpl* unk=dynamic_cast<UnknownElementImpl*>(xmlObject);
    if (!unk)
        throw MarshallingException("Only objects of class UnknownElementImpl can be marshalled.");
    
    DOMElement* cachedDOM=unk->getDOM();
    if (cachedDOM) {
        if (!document || document==cachedDOM->getOwnerDocument()) {
            log.debug("XMLObject has a usable cached DOM, reusing it");
            setDocumentElement(cachedDOM->getOwnerDocument(),cachedDOM);
            unk->releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given. This both sucks and blows.
        // Without an adoptNode option to maintain the child pointers, we rely on our custom
        // implementation class to preserve the XML when we release the existing DOM.
        unk->releaseDOM();
    }
    
    // If we get here, we didn't have a usable DOM (and/or we flushed the one we had).
    // We need to reparse the XML we saved off into a new DOM.
    bool bindDocument=false;
    MemBufInputSource src(reinterpret_cast<const XMLByte*>(unk->m_xml.c_str()),unk->m_xml.length(),"UnknownElementImpl");
    Wrapper4InputSource dsrc(&src,false);
    log.debug("parsing XML back into DOM tree");
    DOMDocument* internalDoc=XMLToolingInternalConfig::getInternalConfig().m_parserPool->parse(dsrc);
    if (document) {
        // The caller insists on using his own document, so we now have to import the thing
        // into it. Then we're just dumping the one we built.
        log.debug("reimporting new DOM into caller-supplied document");
        cachedDOM=static_cast<DOMElement*>(document->importNode(internalDoc->getDocumentElement(), true));
        internalDoc->release();
    }
    else {
        // We just bind the document we built to the object as the result.
        cachedDOM=static_cast<DOMElement*>(internalDoc->getDocumentElement());
        document=internalDoc;
        bindDocument=true;
    }

    // Recache the DOM and clear the serialized copy.
    setDocumentElement(document, cachedDOM);
    log.debug("caching DOM for XMLObject (document is %sbound)", bindDocument ? "" : "not ");
    unk->setDOM(cachedDOM, bindDocument);
    unk->releaseParentDOM(true);
    unk->m_xml.erase();
    return cachedDOM;
}

DOMElement* UnknownElementMarshaller::marshall(XMLObject* xmlObject, DOMElement* parentElement) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Marshaller");
    log.debug("marshalling unknown content");

    UnknownElementImpl* unk=dynamic_cast<UnknownElementImpl*>(xmlObject);
    if (!unk)
        throw MarshallingException("Only objects of class UnknownElementImpl can be marshalled.");
    
    DOMElement* cachedDOM=unk->getDOM();
    if (cachedDOM) {
        if (parentElement->getOwnerDocument()==cachedDOM->getOwnerDocument()) {
            log.debug("XMLObject has a usable cached DOM, reusing it");
            parentElement->appendChild(cachedDOM);
            unk->releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given. This both sucks and blows.
        // Without an adoptNode option to maintain the child pointers, we rely on our custom
        // implementation class to preserve the XML when we release the existing DOM.
        unk->releaseDOM();
    }
    
    // If we get here, we didn't have a usable DOM (and/or we flushed the one we had).
    // We need to reparse the XML we saved off into a new DOM.
    MemBufInputSource src(reinterpret_cast<const XMLByte*>(unk->m_xml.c_str()),unk->m_xml.length(),"UnknownElementImpl");
    Wrapper4InputSource dsrc(&src,false);
    log.debug("parsing XML back into DOM tree");
    DOMDocument* internalDoc=XMLToolingInternalConfig::getInternalConfig().m_parserPool->parse(dsrc);
    
    log.debug("reimporting new DOM into caller-supplied document");
    cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(internalDoc->getDocumentElement(), true));
    internalDoc->release();

    // Recache the DOM and clear the serialized copy.
    parentElement->appendChild(cachedDOM);
    log.debug("caching DOM for XMLObject");
    unk->setDOM(cachedDOM, false);
    unk->releaseParentDOM(true);
    unk->m_xml.erase();
    return cachedDOM;
}

XMLObject* UnknownElementUnmarshaller::unmarshall(DOMElement* element, bool bindDocument) const
{
    UnknownElementImpl* ret=new UnknownElementImpl();
    ret->setDOM(element, bindDocument);
    return ret;
}
