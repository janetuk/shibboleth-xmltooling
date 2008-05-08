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
 * UnknownElement.cpp
 * 
 * Basic implementation suitable for use as default for unrecognized content
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "impl/UnknownElement.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;
#ifndef XMLTOOLING_NO_XMLSEC
using xmlsignature::Signature;
#endif

void UnknownElementImpl::releaseDOM() const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("releaseDOM");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLObject");
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
    if (getDOM())
        XMLHelper::serialize(getDOM(),s);
}

DOMElement* UnknownElementImpl::marshall(
    DOMDocument* document
#ifndef XMLTOOLING_NO_XMLSEC
    ,const vector<Signature*>* sigs
    ,const Credential* credential
#endif
    ) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLObject");
    log.debug("marshalling unknown content");

    DOMElement* cachedDOM=getDOM();
    if (cachedDOM) {
        if (!document || document==cachedDOM->getOwnerDocument()) {
            log.debug("XMLObject has a usable cached DOM, reusing it");
            if (document)
                setDocumentElement(cachedDOM->getOwnerDocument(),cachedDOM);
            releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(document->importNode(cachedDOM, true));

        // Recache the DOM.
        setDocumentElement(document, cachedDOM);
        log.debug("caching imported DOM for XMLObject");
        setDOM(cachedDOM, false);
        releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM.
    // We need to reparse the XML we saved off into a new DOM.
    bool bindDocument=false;
    MemBufInputSource src(reinterpret_cast<const XMLByte*>(m_xml.c_str()),m_xml.length(),"UnknownElementImpl");
    Wrapper4InputSource dsrc(&src,false);
    log.debug("parsing XML back into DOM tree");
    DOMDocument* internalDoc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
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
    setDOM(cachedDOM, bindDocument);
    releaseParentDOM(true);
    m_xml.erase();
    return cachedDOM;
}


DOMElement* UnknownElementImpl::marshall(
    DOMElement* parentElement
#ifndef XMLTOOLING_NO_XMLSEC
    ,const vector<Signature*>* sigs
    ,const Credential* credential
#endif
    ) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".XMLObject");
    log.debug("marshalling unknown content");

    DOMElement* cachedDOM=getDOM();
    if (cachedDOM) {
        if (parentElement->getOwnerDocument()==cachedDOM->getOwnerDocument()) {
            log.debug("XMLObject has a usable cached DOM, reusing it");
            parentElement->appendChild(cachedDOM);
            releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(cachedDOM, true));

        // Recache the DOM.
        parentElement->appendChild(cachedDOM);
        log.debug("caching imported DOM for XMLObject");
        setDOM(cachedDOM, false);
        releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM (and/or we flushed the one we had).
    // We need to reparse the XML we saved off into a new DOM.
    MemBufInputSource src(reinterpret_cast<const XMLByte*>(m_xml.c_str()),m_xml.length(),"UnknownElementImpl");
    Wrapper4InputSource dsrc(&src,false);
    log.debug("parsing XML back into DOM tree");
    DOMDocument* internalDoc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
    
    log.debug("reimporting new DOM into caller-supplied document");
    cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(internalDoc->getDocumentElement(), true));
    internalDoc->release();

    // Recache the DOM and clear the serialized copy.
    parentElement->appendChild(cachedDOM);
    log.debug("caching DOM for XMLObject");
    setDOM(cachedDOM, false);
    releaseParentDOM(true);
    m_xml.erase();
    return cachedDOM;
}

XMLObject* UnknownElementImpl::unmarshall(DOMElement* element, bool bindDocument)
{
    setDOM(element, bindDocument);
    return this;
}

XMLObject* UnknownElementBuilder::buildObject(
    const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType
    ) const {
    return new UnknownElementImpl(nsURI,localName,prefix);
}

