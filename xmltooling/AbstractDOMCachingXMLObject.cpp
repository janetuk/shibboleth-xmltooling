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
#include "exceptions.h"
#include "AbstractDOMCachingXMLObject.h"

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

void AbstractDOMCachingXMLObject::setDOM(DOMElement* dom, bool bindDocument)
{
    m_dom=dom;
    if (dom) {
        if (bindDocument) {
            DOMDocument* tmp=setDocument(dom->getOwnerDocument());
            if (tmp)
                tmp->release();
        }
    }
    else if (m_document) {
        m_document->release();
        m_document=NULL;
    }
}

void AbstractDOMCachingXMLObject::releaseDOM()
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".DOM");
    if (log.isDebugEnabled())
        log.debug("releasing cached DOM reprsentation for %s", getElementQName().toString().c_str());
    setDOM(NULL);
}

void AbstractDOMCachingXMLObject::releaseParentDOM(bool propagateRelease)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".DOM");
    if (log.isDebugEnabled()) {
        log.debug(
            "releasing cached DOM representation for parent of %s with propagation set to %s",
            getElementQName().toString().c_str(), propagateRelease ? "true" : "false"
            );
    }

    DOMCachingXMLObject* domCachingParent = dynamic_cast<DOMCachingXMLObject*>(getParent());
    if (domCachingParent) {
        domCachingParent->releaseDOM();
        if (propagateRelease)
            domCachingParent->releaseParentDOM(propagateRelease);
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

void AbstractDOMCachingXMLObject::releaseChildrenDOM(bool propagateRelease)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".DOM");
    if (log.isDebugEnabled()) {
        log.debug(
            "releasing cached DOM representation for children of %s with propagation set to %s",
            getElementQName().toString().c_str(), propagateRelease ? "true" : "false"
            );
    }
    
    vector<XMLObject*> children;
    if (getOrderedChildren(children))
        for_each(children.begin(),children.end(),bind2nd(_release(),propagateRelease));
}

XMLObject* AbstractDOMCachingXMLObject::prepareForAssignment(const XMLObject* oldValue, XMLObject* newValue) {

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
