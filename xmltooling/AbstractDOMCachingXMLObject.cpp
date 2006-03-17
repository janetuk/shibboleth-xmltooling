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
 * AbstractDOMCachingXMLObject.cpp
 * 
 * Extension of AbstractXMLObject that implements a DOMCachingXMLObject. 
 */

#include "internal.h"
#include "AbstractDOMCachingXMLObject.h"
#include "exceptions.h"
#include "XMLObjectBuilder.h"
#include "util/XMLHelper.h"

#include <algorithm>
#include <functional>
#include <log4cpp/Category.hh>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

AbstractDOMCachingXMLObject::~AbstractDOMCachingXMLObject()
{
    if (m_document)
        m_document->release();
}

void AbstractDOMCachingXMLObject::setDOM(DOMElement* dom, bool bindDocument) const
{
    m_dom=dom;
    if (dom) {
        if (bindDocument) {
            setDocument(dom->getOwnerDocument());
        }
    }
}

void AbstractDOMCachingXMLObject::releaseDOM() const
{
    if (m_dom) {
        Category& log=Category::getInstance(XMLTOOLING_LOGCAT".DOM");
        if (log.isDebugEnabled()) {
            string qname=getElementQName().toString();
            log.debug("releasing cached DOM representation for (%s)", qname.empty() ? "unknown" : qname.c_str());
        }
        setDOM(NULL);
    }
}

void AbstractDOMCachingXMLObject::releaseParentDOM(bool propagateRelease) const
{
    DOMCachingXMLObject* domCachingParent = dynamic_cast<DOMCachingXMLObject*>(getParent());
    if (domCachingParent) {
        if (domCachingParent->getDOM()) {
            Category::getInstance(XMLTOOLING_LOGCAT".DOM").debug(
                "releasing cached DOM representation for parent object with propagation set to %s",
                propagateRelease ? "true" : "false"
                );
            domCachingParent->releaseDOM();
            if (propagateRelease)
                domCachingParent->releaseParentDOM(propagateRelease);
        }
    }
}

class _release : public binary_function<XMLObject*,bool,void> {
public:
    void operator()(XMLObject* obj, bool propagate) const {
        DOMCachingXMLObject* domCaching = dynamic_cast<DOMCachingXMLObject*>(obj);
        if (domCaching) {
            domCaching->releaseDOM();
            if (propagate)
                domCaching->releaseChildrenDOM(propagate);
        }
    }
};

void AbstractDOMCachingXMLObject::releaseChildrenDOM(bool propagateRelease) const
{
    if (hasChildren()) {
        Category::getInstance(XMLTOOLING_LOGCAT".DOM").debug(
            "releasing cached DOM representation for children with propagation set to %s",
            propagateRelease ? "true" : "false"
            );
        for_each(m_children.begin(),m_children.end(),bind2nd(_release(),propagateRelease));
    }
}

DOMElement* AbstractDOMCachingXMLObject::cloneDOM(DOMDocument* doc) const
{
    if (getDOM()) {
        if (!doc)
            doc=DOMImplementationRegistry::getDOMImplementation(NULL)->createDocument();
        return static_cast<DOMElement*>(doc->importNode(getDOM(),true));
    }
    return NULL;
}

XMLObject* AbstractDOMCachingXMLObject::clone() const
{
    // See if we can clone via the DOM.
    DOMElement* domCopy=cloneDOM();
    if (domCopy) {
        // Seemed to work, so now we unmarshall the DOM to produce the clone.
        const XMLObjectBuilder* b=XMLObjectBuilder::getBuilder(domCopy);
        if (!b) {
            auto_ptr<QName> q(XMLHelper::getNodeQName(domCopy));
            Category::getInstance(XMLTOOLING_LOGCAT".DOM").error(
                "DOM clone failed, unable to locate builder for element (%s)", q->toString().c_str()
                );
            domCopy->getOwnerDocument()->release();
            throw UnmarshallingException("Unable to locate builder for cloned element.");
        }
        try {
            auto_ptr<XMLObject> objCopy(b->buildObject(domCopy));
            objCopy->unmarshall(domCopy, true);    // bind document
            return objCopy.release();
        }
        catch (...) {
            domCopy->getOwnerDocument()->release();
            throw;
        }
    }
    return NULL;
}

XMLObject* AbstractDOMCachingXMLObject::prepareForAssignment(XMLObject* oldValue, XMLObject* newValue) {

    if (newValue && newValue->hasParent())
        throw XMLObjectException("child XMLObject cannot be added - it is already the child of another XMLObject");

    if (!oldValue) {
        if (newValue) {
            releaseThisandParentDOM();
            newValue->setParent(this);
            return newValue;
        }
        else {
            return NULL;
        }
    }

    if (oldValue != newValue) {
        delete oldValue;
        releaseThisandParentDOM();
        newValue->setParent(this);
    }

    return newValue;
}
