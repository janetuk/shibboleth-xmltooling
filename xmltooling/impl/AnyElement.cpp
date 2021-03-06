/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * AnyElement.cpp
 * 
 * Advanced anyType implementation suitable for deep processing of unknown content.
 */

#include "internal.h"
#include "exceptions.h"
#include "impl/AnyElement.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
using namespace xercesc;
using namespace std;

AnyElementImpl::AnyElementImpl()
{
}

AnyElementImpl::AnyElementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
    : AbstractXMLObject(nsURI, localName, prefix, schemaType)
{
}

AnyElementImpl::AnyElementImpl(const AnyElementImpl& src)
        : AbstractXMLObject(src),
          AbstractDOMCachingXMLObject(src),
          AbstractComplexElement(src),
          AbstractAttributeExtensibleXMLObject(src)
{
}

AnyElementImpl::~AnyElementImpl()
{
}

void AnyElementImpl::_clone(const AnyElementImpl& src)
{
    const vector<XMLObject*>& children = src.getUnknownXMLObjects();
    for (vector<XMLObject*>::const_iterator i=children.begin(); i!=children.end(); ++i)
        getUnknownXMLObjects().push_back((*i)->clone());
}

XMLObject* AnyElementImpl::clone() const {
    auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone());
    AnyElementImpl* ret=dynamic_cast<AnyElementImpl*>(domClone.get());
    if (ret) {
        domClone.release();
        return ret;
    }

    auto_ptr<AnyElementImpl> ret2(new AnyElementImpl(*this));
    ret2->_clone(*ret2.get());
    return ret2.release();
}

void AnyElementImpl::marshallAttributes(DOMElement* domElement) const
{
    marshallExtensionAttributes(domElement);
}

void AnyElementImpl::processChildElement(XMLObject* childXMLObject, const DOMElement* root)
{
    getUnknownXMLObjects().push_back(childXMLObject);
}

void AnyElementImpl::processAttribute(const DOMAttr* attribute)
{
    unmarshallExtensionAttribute(attribute);
}

XMLObject* AnyElementBuilder::buildObject(
    const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType
    ) const
{
    return new AnyElementImpl(nsURI, localName, prefix, schemaType);
}
