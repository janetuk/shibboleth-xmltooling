/*
 *  Copyright 2001-2009 Internet2
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

using namespace xmltooling;
using namespace xercesc;
using namespace std;

AbstractDOMCachingXMLObject::AbstractDOMCachingXMLObject() : m_dom(NULL), m_document(NULL)
{
}

AbstractDOMCachingXMLObject::AbstractDOMCachingXMLObject(const AbstractDOMCachingXMLObject& src)
    : AbstractXMLObject(src), m_dom(NULL), m_document(NULL)
{
}

AbstractDOMCachingXMLObject::~AbstractDOMCachingXMLObject()
{
    if (m_document)
        m_document->release();
}

DOMElement* AbstractDOMCachingXMLObject::getDOM() const
{
    return m_dom;
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

void AbstractDOMCachingXMLObject::setDocument(DOMDocument* doc) const
{
    if (m_document != doc) {
        if (m_document)
            m_document->release();
        m_document=doc;
    }
}

void AbstractDOMCachingXMLObject::releaseDOM() const
{
    if (m_dom) {
        if (m_log.isDebugEnabled()) {
            string qname=getElementQName().toString();
            m_log.debug("releasing cached DOM representation for (%s)", qname.empty() ? "unknown" : qname.c_str());
        }
        setDOM(NULL);
    }
}

void AbstractDOMCachingXMLObject::releaseParentDOM(bool propagateRelease) const
{
    if (getParent() && getParent()->getDOM()) {
        m_log.debug(
            "releasing cached DOM representation for parent object with propagation set to %s",
            propagateRelease ? "true" : "false"
            );
        getParent()->releaseDOM();
        if (propagateRelease)
            getParent()->releaseParentDOM(propagateRelease);
    }
}

namespace {
    class _release : public binary_function<XMLObject*,bool,void> {
    public:
        void operator()(XMLObject* obj, bool propagate) const {
            if (obj) {
                obj->releaseDOM();
                if (propagate)
                    obj->releaseChildrenDOM(propagate);
            }
        }
    };
};

void AbstractDOMCachingXMLObject::releaseChildrenDOM(bool propagateRelease) const
{
    if (hasChildren()) {
        m_log.debug(
            "releasing cached DOM representation for children with propagation set to %s",
            propagateRelease ? "true" : "false"
            );
        const list<XMLObject*>& children=getOrderedChildren();
        for_each(children.begin(),children.end(),bind2nd(_release(),propagateRelease));
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
            m_log.error(
                "DOM clone failed, unable to locate builder for element (%s)", q->toString().c_str()
                );
            domCopy->getOwnerDocument()->release();
            throw UnmarshallingException("Unable to locate builder for cloned element.");
        }
        XercesJanitor<DOMDocument> janitor(domCopy->getOwnerDocument());
        XMLObject* ret = b->buildFromElement(domCopy,true); // bind document
        janitor.release(); // safely transferred
        return ret;
    }
    return NULL;
}

void AbstractDOMCachingXMLObject::detach()
{
    // This is an override that duplicates some of the checking in the base class but
    // adds document management in preparation for deletion of the parent.

    if (!getParent())
        return;

    if (getParent()->hasParent())
        throw XMLObjectException("Cannot detach an object whose parent is itself a child.");

    AbstractDOMCachingXMLObject* parent = dynamic_cast<AbstractDOMCachingXMLObject*>(getParent());
    if (parent && parent->m_document) {
        // Transfer control of document to me...
        setDocument(parent->m_document);
        parent->m_document = NULL;
    }
    // The rest is done by the base.
    AbstractXMLObject::detach();
}
