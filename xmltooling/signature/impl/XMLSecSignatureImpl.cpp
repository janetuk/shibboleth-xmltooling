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
 * XMLSecSignatureImpl.cpp
 * 
 * Signature class for XMLSec-based signature-handling
 */

#include "internal.h"
#include "exceptions.h"
#include "impl/UnknownElement.h"
#include "signature/Signature.h"
#include "util/NDC.h"
#include "util/XMLConstants.h"
#include "util/XMLHelper.h"

#include <log4cpp/Category.hh>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/enc/XSECCryptoException.hpp>
#include <xsec/framework/XSECException.hpp>

using namespace xmlsignature;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlsignature {
    
    class XMLTOOL_DLLLOCAL XMLSecSignatureImpl : public UnknownElementImpl, public virtual Signature
    {
    public:
        XMLSecSignatureImpl() : UnknownElementImpl(XMLConstants::XMLSIG_NS, Signature::LOCAL_NAME, XMLConstants::XMLSIG_PREFIX),
            m_signature(NULL), m_c14n(NULL), m_sm(NULL) {}
        virtual ~XMLSecSignatureImpl();
        
        void releaseDOM();
        XMLObject* clone() const;
        Signature* cloneSignature() const;

        DOMElement* marshall(DOMDocument* document=NULL, MarshallingContext* ctx=NULL) const;
        DOMElement* marshall(DOMElement* parentElement, MarshallingContext* ctx=NULL) const;
        XMLObject* unmarshall(DOMElement* element, bool bindDocument=false);
        
        // Getters
        const XMLCh* getCanonicalizationMethod() const { return m_c14n ? m_c14n : DSIGConstants::s_unicodeStrURIEXC_C14N_NOC; }
        const XMLCh* getSignatureAlgorithm() const { return m_sm ? m_sm : DSIGConstants::s_unicodeStrURIRSA_SHA1; }

        // Setters
        void setCanonicalizationMethod(const XMLCh* c14n) { m_c14n = prepareForAssignment(m_c14n,c14n); }
        void setSignatureAlgorithm(const XMLCh* sm) { m_sm = prepareForAssignment(m_sm,sm); }

        void sign(const SigningContext& ctx);
        void verify(const VerifyingContext& ctx) const;

    private:
        mutable DSIGSignature* m_signature;
        XMLCh* m_c14n;
        XMLCh* m_sm;
    };
    
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

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
    return cloneSignature();
}

Signature* XMLSecSignatureImpl::cloneSignature() const
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

class _addcert : public std::binary_function<DSIGKeyInfoX509*,XSECCryptoX509*,void> {
public:
    void operator()(DSIGKeyInfoX509* bag, XSECCryptoX509* cert) const {
        safeBuffer& buf=cert->getDEREncodingSB();
        bag->appendX509Certificate(buf.sbStrToXMLCh());
    }
};

void XMLSecSignatureImpl::sign(const SigningContext& ctx)
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Signature");
    log.debug("applying signature");

    if (!m_signature)
        throw SignatureException("Only a marshalled Signature object can be signed.");

    try {
        log.debug("creating signature content");
        ctx.createSignature(m_signature);
        const std::vector<XSECCryptoX509*>* certs=ctx.getX509Certificates();
        if (certs && !certs->empty()) {
            DSIGKeyInfoX509* x509Data=m_signature->appendX509Data();
            for_each(certs->begin(),certs->end(),bind1st(_addcert(),x509Data));
        }
        else {
            auto_ptr<KeyInfo> keyInfo(ctx.getKeyInfo());
            if (keyInfo.get()) {
                DOMElement* domElement=keyInfo->marshall(m_signature->getParentDocument());
                getDOM()->appendChild(domElement);
            }
        }
        
        log.debug("computing signature");
        m_signature->setSigningKey(ctx.getSigningKey());
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

void XMLSecSignatureImpl::verify(const VerifyingContext& ctx) const
{
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Signature");
    log.debug("verifying signature");

    if (!m_signature)
        throw SignatureException("Only a marshalled Signature object can be verified.");

    try {
        ctx.verifySignature(m_signature);
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw SignatureException(string("Caught an XMLSecurity exception verifying signature: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw SignatureException(string("Caught an XMLSecurity exception verifying signature: ") + e.getMsg());
    }
}

DOMElement* XMLSecSignatureImpl::marshall(DOMDocument* document, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Signature");
    log.debug("marshalling ds:Signature");

    DOMElement* cachedDOM=getDOM();
    if (cachedDOM) {
        if (!document || document==cachedDOM->getOwnerDocument()) {
            log.debug("Signature has a usable cached DOM, reusing it");
            if (document)
                setDocumentElement(cachedDOM->getOwnerDocument(),cachedDOM);
            releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(document->importNode(cachedDOM, true));

        try {
            XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(m_signature);
            m_signature=NULL;
            m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                document, cachedDOM
                );
            m_signature->load();
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
        setDOM(cachedDOM, false);
        releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM.
    bool bindDocument=false;
    if (m_xml.empty()) {
        // Fresh signature, so we just create an empty one.
        log.debug("creating empty Signature element");
        if (!document) {
            document=DOMImplementationRegistry::getDOMImplementation(NULL)->createDocument();
            bindDocument=true;
        }
        m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignature();
        m_signature->setDSIGNSPrefix(XMLConstants::XMLSIG_PREFIX);
        cachedDOM=m_signature->createBlankSignature(document, getCanonicalizationMethod(), getSignatureAlgorithm());
    }
    else {
        // We need to reparse the XML we saved off into a new DOM.
        MemBufInputSource src(reinterpret_cast<const XMLByte*>(m_xml.c_str()),m_xml.length(),"XMLSecSignatureImpl");
        Wrapper4InputSource dsrc(&src,false);
        log.debug("parsing Signature XML back into DOM tree");
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

        // Now reload the signature from the DOM.
        try {
            m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                document, cachedDOM
                );
            m_signature->load();
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
    setDOM(cachedDOM, bindDocument);
    releaseParentDOM(true);
    m_xml.erase();
    return cachedDOM;
}

DOMElement* XMLSecSignatureImpl::marshall(DOMElement* parentElement, MarshallingContext* ctx) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("marshall");
#endif
    
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".Signature");
    log.debug("marshalling ds:Signature");

    DOMElement* cachedDOM=getDOM();
    if (cachedDOM) {
        if (parentElement->getOwnerDocument()==cachedDOM->getOwnerDocument()) {
            log.debug("Signature has a usable cached DOM, reusing it");
            parentElement->appendChild(cachedDOM);
            releaseParentDOM(true);
            return cachedDOM;
        }
        
        // We have a DOM but it doesn't match the document we were given, so we import
        // it into the new document.
        cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(cachedDOM, true));

        try {
            XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->releaseSignature(m_signature);
            m_signature=NULL;
            m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                parentElement->getOwnerDocument(), cachedDOM
                );
            m_signature->load();
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
        setDOM(cachedDOM, false);
        releaseParentDOM(true);
        return cachedDOM;
    }
    
    // If we get here, we didn't have a usable DOM.
    if (m_xml.empty()) {
        // Fresh signature, so we just create an empty one.
        log.debug("creating empty Signature element");
        m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignature();
        m_signature->setDSIGNSPrefix(XMLConstants::XMLSIG_PREFIX);
        cachedDOM=m_signature->createBlankSignature(
            parentElement->getOwnerDocument(), getCanonicalizationMethod(), getSignatureAlgorithm()
            );
    }
    else {
        MemBufInputSource src(reinterpret_cast<const XMLByte*>(m_xml.c_str()),m_xml.length(),"XMLSecSignatureImpl");
        Wrapper4InputSource dsrc(&src,false);
        log.debug("parsing XML back into DOM tree");
        DOMDocument* internalDoc=XMLToolingConfig::getConfig().getParser().parse(dsrc);
        
        log.debug("reimporting new DOM into caller-supplied document");
        cachedDOM=static_cast<DOMElement*>(parentElement->getOwnerDocument()->importNode(internalDoc->getDocumentElement(),true));
        internalDoc->release();

        // Now reload the signature from the DOM.
        try {
            m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
                parentElement->getOwnerDocument(), cachedDOM
                );
            m_signature->load();
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
    setDOM(cachedDOM, false);
    releaseParentDOM(true);
    m_xml.erase();
    return cachedDOM;
}

XMLObject* XMLSecSignatureImpl::unmarshall(DOMElement* element, bool bindDocument)
{
    Category::getInstance(XMLTOOLING_LOGCAT".Signature").debug("unmarshalling ds:Signature");

    try {
        m_signature=XMLToolingInternalConfig::getInternalConfig().m_xsecProvider->newSignatureFromDOM(
            element->getOwnerDocument(), element
            );
        m_signature->load();
    }
    catch(XSECException& e) {
        auto_ptr_char temp(e.getMsg());
        throw UnmarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + temp.get());
    }
    catch(XSECCryptoException& e) {
        throw UnmarshallingException(string("Caught an XMLSecurity exception while loading signature: ") + e.getMsg());
    }

    setDOM(element, bindDocument);
    return this;
}

Signature* SignatureBuilder::buildObject(
    const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType
    ) const
{
    if (!XMLString::equals(nsURI,XMLConstants::XMLSIG_NS) || !XMLString::equals(localName,Signature::LOCAL_NAME))
        throw XMLObjectException("XMLSecSignatureBuilder requires standard Signature element name.");
    return buildObject();
}

Signature* SignatureBuilder::buildObject() const
{
    return new XMLSecSignatureImpl();
}

const XMLCh Signature::LOCAL_NAME[] = UNICODE_LITERAL_9(S,i,g,n,a,t,u,r,e);
