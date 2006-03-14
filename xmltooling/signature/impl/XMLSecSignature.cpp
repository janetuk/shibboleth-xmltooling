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
 * XMLSecSignature.cpp
 * 
 * Signature classes for XMLSec-based signature-handling
 */

#include "internal.h"
#include "exceptions.h"
#include "signature/impl/XMLSecSignature.h"
#include "util/NDC.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmltooling;
using namespace log4cpp;
using namespace std;

const XMLCh xmltooling::Signature::LOCAL_NAME[] = {
    chLatin_S, chLatin_i, chLatin_g, chLatin_n, chLatin_a, chLatin_t, chLatin_u, chLatin_r, chLatin_e, chNull
}; 

const XMLCh xmltooling::Signature::PREFIX[] = {
    chLatin_d, chLatin_s, chNull
}; 

XMLSecSignatureImpl::~XMLSecSignatureImpl()
{
    // Release the associated signature.
    if (m_signature)
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(m_signature);

    XMLString::release(&m_c14n);
    XMLString::release(&m_sm);
}

void XMLSecSignatureImpl::releaseDOM()
{
    // This should save off the DOM
    UnknownElementImpl::releaseDOM();
    
    // Release the associated signature.
    if (m_signature) {
        XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(m_signature);
        m_signature=NULL;
    }
}

XMLObject* XMLSecSignatureImpl::clone() const
{
    XMLSecSignatureImpl* ret=new XMLSecSignatureImpl();

    ret->m_c14n=XMLString::replicate(m_c14n);
    ret->m_sm=XMLString::replicate(m_sm);

    // If there's no XML locally, serialize this object into the new one, otherwise just copy it over.
    if (m_xml.empty())
        serialize(ret->m_xml);
    else
        ret->m_xml=m_xml;

    return ret;
}

DSIGKeyInfoList* XMLSecSignatureImpl::getKeyInfo() const
{
    return m_signature ? m_signature->getKeyInfoList() : NULL;
}

class _addcert : public std::binary_function<DSIGKeyInfoX509*,XSECCryptoX509*,void> {
public:
    void operator()(DSIGKeyInfoX509* bag, XSECCryptoX509* cert) const {
        safeBuffer& buf=cert->getDEREncodingSB();
        bag->appendX509Certificate(buf.sbStrToXMLCh());
    }
};

void XMLSecSignatureImpl::sign(const SigningContext* ctx)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Signature");
    log.debug("applying signature");

    if (!m_signature)
        throw SignatureException("Only a marshalled Signature object can be signed.");

    try {
        log.debug("creating signature content");
        ctx->createSignature(m_signature);
        const std::vector<XSECCryptoX509*>& certs=ctx->getX509Certificates();
        if (!certs.empty()) {
            DSIGKeyInfoX509* x509Data=m_signature->appendX509Data();
            for_each(certs.begin(),certs.end(),bind1st(_addcert(),x509Data));
        }
        
        log.debug("computing signature");
        m_signature->setSigningKey(ctx->getSigningKey());
        m_signature->sign();
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw SignatureException(string("Caught an XMLSecurity exception while signing: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw SignatureException(string("Caught an XMLSecurity exception while signing: ") + e.getMsg());
    }
}

DOMElement* XMLSecSignatureMarshaller::marshall(XMLObject* xmlObject, DOMDocument* document, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Marshaller");
    log.debug("marshalling ds:Signature");

    XMLSecSignatureImpl* sig=dynamic_cast<XMLSecSignatureImpl*>(xmlObject);
    if (!sig)
        throw MarshallingException("Only objects of class XMLSecSignatureImpl can be marshalled.");
    
    DOMElement* cachedDOM=sig->getDOM();
    if (cachedDOM) {
        if (!document || document==cachedDOM->getOwnerDocument()) {
            log.debug("Signature has a usable cached DOM, reusing it");
            if (document)
                setDocumentElement(cachedDOM->getOwnerDocument(),cachedDOM);
            sig->releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(document->importNode(cachedDOM, true));

        try {
            XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(sig->m_signature);
            sig->m_signature=NULL;
            sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                document, cachedDOM
                );
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
        }
        catch(XSECCryptoException& e) {
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
        }

        // Recache the DOM.
        setDocumentElement(document, cachedDOM);
        log.debug("caching imported DOM for Signature");
        sig->setDOM(cachedDOM, false);
        sig->releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM.
    bool bindDocument=false;
    if (sig->m_xml.empty()) {
        // Fresh signature, so we just create an empty one.
        log.debug("creating empty Signature element");
        if (!document) {
            document=DOMImplementationRegistry::getDOMImplementation(NULL)->createDocument();
            bindDocument=true;
        }
        sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignature();
        sig->m_signature->setDSIGNSPrefix(Signature::PREFIX);
        cachedDOM=sig->m_signature->createBlankSignature(
            document, sig->getCanonicalizationMethod(), sig->getSignatureAlgorithm()
            );
    }
    else {
        // We need to reparse the XML we saved off into a new DOM.
        MemBufInputSource src(reinterpret_cast<const XMLByte*>(sig->m_xml.c_str()),sig->m_xml.length(),"XMLSecSignatureImpl");
        Wrapper4InputSource dsrc(&src,false);
        log.debug("parsing Signature XML back into DOM tree");
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

        // Now reload the signature from the DOM.
        try {
            sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                document, cachedDOM
                );
        }
        catch(XSECException& e) {
            if (bindDocument)
                document->release();
            auto_ptr_char temp(e.getMsg());
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
        }
        catch(XSECCryptoException& e) {
            if (bindDocument)
                document->release();
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
        }
    }
    
    // Recache the DOM and clear the serialized copy.
    setDocumentElement(document, cachedDOM);
    log.debug("caching DOM for Signature (document is %sbound)", bindDocument ? "" : "not ");
    sig->setDOM(cachedDOM, bindDocument);
    sig->releaseParentDOM(true);
    sig->m_xml.erase();
    return cachedDOM;
}

DOMElement* XMLSecSignatureMarshaller::marshall(XMLObject* xmlObject, DOMElement* parentElement, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Marshaller");
    log.debug("marshalling ds:Signature");

    XMLSecSignatureImpl* sig=dynamic_cast<XMLSecSignatureImpl*>(xmlObject);
    if (!sig)
        throw MarshallingException("Only objects of class XMLSecSignatureImpl can be marshalled.");
    
    DOMElement* cachedDOM=sig->getDOM();
    if (cachedDOM) {
        if (parentElement->getOwnerDocument()==cachedDOM->getOwnerDocument()) {
            log.debug("Signature has a usable cached DOM, reusing it");
            parentElement->appendChild(cachedDOM);
            sig->releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(cachedDOM, true));

        try {
            XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(sig->m_signature);
            sig->m_signature=NULL;
            sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                parentElement->getOwnerDocument(), cachedDOM
                );
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
        }
        catch(XSECCryptoException& e) {
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
        }

        // Recache the DOM.
        parentElement->appendChild(cachedDOM);
        log.debug("caching imported DOM for Signature");
        sig->setDOM(cachedDOM, false);
        sig->releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM.
    if (sig->m_xml.empty()) {
        // Fresh signature, so we just create an empty one.
        log.debug("creating empty Signature element");
        sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignature();
        sig->m_signature->setDSIGNSPrefix(Signature::PREFIX);
        cachedDOM=sig->m_signature->createBlankSignature(
            parentElement->getOwnerDocument(), sig->getCanonicalizationMethod(), sig->getSignatureAlgorithm()
            );
    }
    else {
        MemBufInputSource src(reinterpret_cast<const XMLByte*>(sig->m_xml.c_str()),sig->m_xml.length(),"XMLSecSignatureImpl");
        Wrapper4InputSource dsrc(&src,false);
        log.debug("parsing XML back into DOM tree");
        DOMDocument* internalDoc=XMLToolingInternalConfig::getInternalConfig().m_parserPool->parse(dsrc);
        
        log.debug("reimporting new DOM into caller-supplied document");
        cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(internalDoc->getDocumentElement(), true));
        internalDoc->release();

        // Now reload the signature from the DOM.
        try {
            sig->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                parentElement->getOwnerDocument(), cachedDOM
                );
        }
        catch(XSECException& e) {
            auto_ptr_char temp(e.getMsg());
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
        }
        catch(XSECCryptoException& e) {
            throw MarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
        }
    }

    // Recache the DOM and clear the serialized copy.
    parentElement->appendChild(cachedDOM);
    log.debug("caching DOM for Signature");
    sig->setDOM(cachedDOM, false);
    sig->releaseParentDOM(true);
    sig->m_xml.erase();
    return cachedDOM;
}

XMLObject* XMLSecSignatureUnmarshaller::unmarshall(DOMElement* element, bool bindDocument) const
{
    Category::getInstance(XMLTOOLING_LOGCAT".Unmarshaller").debug("unmarshalling ds:Signature");

    auto_ptr<XMLSecSignatureImpl> ret(new XMLSecSignatureImpl());
    try {
        ret->m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
            element->getOwnerDocument(), element
            );
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw UnmarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw UnmarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
    }

    ret->setDOM(element, bindDocument);
    return ret.release();
}
