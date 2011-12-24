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
 * ParserPool.cpp
 *
 * A thread-safe pool of parsers that share characteristics.
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "util/CurlURLInputStream.h"
#include "util/NDC.h"
#include "util/ParserPool.h"
#include "util/Threads.h"
#include "util/XMLHelper.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

using namespace xmltooling::logging;
using namespace xmltooling;
using namespace xercesc;
using namespace boost;
using namespace std;


namespace {
    class MyErrorHandler : public DOMErrorHandler {
    public:
        unsigned int errors;

        MyErrorHandler() : errors(0) {}

        bool handleError(const DOMError& e)
        {
#ifdef _DEBUG
            xmltooling::NDC ndc("handleError");
#endif
            Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ParserPool");

            DOMLocator* locator=e.getLocation();
            auto_ptr_char temp(e.getMessage());

            switch (e.getSeverity()) {
                case DOMError::DOM_SEVERITY_WARNING:
                    log.warnStream() << "warning on line " << locator->getLineNumber()
                        << ", column " << locator->getColumnNumber()
                        << ", message: " << temp.get() << logging::eol;
                    return true;

                case DOMError::DOM_SEVERITY_ERROR:
                    ++errors;
                    log.errorStream() << "error on line " << locator->getLineNumber()
                        << ", column " << locator->getColumnNumber()
                        << ", message: " << temp.get() << logging::eol;
                    return true;

                case DOMError::DOM_SEVERITY_FATAL_ERROR:
                    ++errors;
                    log.errorStream() << "fatal error on line " << locator->getLineNumber()
                        << ", column " << locator->getColumnNumber()
                        << ", message: " << temp.get() << logging::eol;
                    return true;
            }

            ++errors;
            log.errorStream() << "undefined error type on line " << locator->getLineNumber()
                << ", column " << locator->getColumnNumber()
                << ", message: " << temp.get() << logging::eol;
            return false;
        }
    };
}


ParserPool::ParserPool(bool namespaceAware, bool schemaAware)
    : m_namespaceAware(namespaceAware), m_schemaAware(schemaAware), m_lock(Mutex::create()), m_security(new SecurityManager()) {}

ParserPool::~ParserPool()
{
    while(!m_pool.empty()) {
        m_pool.top()->release();
        m_pool.pop();
    }
}

DOMDocument* ParserPool::newDocument()
{
    return DOMImplementationRegistry::getDOMImplementation(nullptr)->createDocument();
}

#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS

DOMDocument* ParserPool::parse(DOMLSInput& domsrc)
{
    DOMLSParser* parser=checkoutBuilder();
    XercesJanitor<DOMLSParser> janitor(parser);
    try {
        MyErrorHandler deh;
        parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, dynamic_cast<DOMErrorHandler*>(&deh));
        DOMDocument* doc=parser->parse(&domsrc);
        if (deh.errors) {
            if (doc)
                doc->release();
            throw XMLParserException("XML error(s) during parsing, check log for specifics");
        }
        parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, (void*)nullptr);
        parser->getDomConfig()->setParameter(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        return doc;
    }
    catch (XMLException& ex) {
        parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, (void*)nullptr);
        parser->getDomConfig()->setParameter(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        auto_ptr_char temp(ex.getMessage());
        throw XMLParserException(string("Xerces error during parsing: ") + (temp.get() ? temp.get() : "no message"));
    }
    catch (XMLToolingException&) {
        parser->getDomConfig()->setParameter(XMLUni::fgDOMErrorHandler, (void*)nullptr);
        parser->getDomConfig()->setParameter(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        throw;
    }
}

#else

DOMDocument* ParserPool::parse(DOMInputSource& domsrc)
{
    DOMBuilder* parser=checkoutBuilder();
    XercesJanitor<DOMBuilder> janitor(parser);
    try {
        MyErrorHandler deh;
        parser->setErrorHandler(&deh);
        DOMDocument* doc=parser->parse(domsrc);
        if (deh.errors) {
            if (doc)
                doc->release();
            throw XMLParserException("XML error(s) during parsing, check log for specifics");
        }
        parser->setErrorHandler(nullptr);
        parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        return doc;
    }
    catch (XMLException& ex) {
        parser->setErrorHandler(nullptr);
        parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        auto_ptr_char temp(ex.getMessage());
        throw XMLParserException(string("Xerces error during parsing: ") + (temp.get() ? temp.get() : "no message"));
    }
    catch (XMLToolingException&) {
        parser->setErrorHandler(nullptr);
        parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);
        checkinBuilder(janitor.release());
        throw;
    }
}

#endif

DOMDocument* ParserPool::parse(istream& is)
{
    StreamInputSource src(is);
    Wrapper4InputSource domsrc(&src,false);
    return parse(domsrc);
}

// Functor to double its argument separated by a character and append to a buffer
template <class T> class doubleit {
public:
    doubleit(T& t, const typename T::value_type& s) : temp(t), sep(s) {}
    void operator() (const pair<const T,T>& s) { temp += s.first + sep + s.first + sep; }
    T& temp;
    const typename T::value_type& sep;
};

bool ParserPool::loadSchema(const XMLCh* nsURI, const XMLCh* pathname)
{
    // Just check the pathname and then directly register the pair into the map.

    auto_ptr_char p(pathname);
#ifdef WIN32
    struct _stat stat_buf;
    if (_stat(p.get(), &stat_buf) != 0)
#else
    struct stat stat_buf;
    if (stat(p.get(), &stat_buf) != 0)
#endif
    {
#if _DEBUG
        xmltooling::NDC ndc("loadSchema");
#endif
        Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ParserPool");
        auto_ptr_char n(nsURI);
        log.error("failed to load schema for (%s), file not found (%s)",n.get(),p.get());
        return false;
    }

    Lock lock(m_lock);
    m_schemaLocMap[nsURI]=pathname;
    m_schemaLocations.erase();
    for_each(m_schemaLocMap.begin(), m_schemaLocMap.end(), doubleit<xstring>(m_schemaLocations,chSpace));

    return true;
}

bool ParserPool::loadCatalogs(const char* pathnames)
{
    string temp(pathnames);
    boost::tokenizer< char_separator<char> > catpaths(temp, char_separator<char>(PATH_SEPARATOR_STR));
    for_each(
        catpaths.begin(), catpaths.end(),
        // Call loadCatalog with an inner call to s->c_str() on each entry.
        boost::bind(static_cast<bool (ParserPool::*)(const char*)>(&ParserPool::loadCatalog),
            boost::ref(this), boost::bind(&string::c_str, _1))
        );
    return catpaths.begin() != catpaths.end();
}

bool ParserPool::loadCatalog(const char* pathname)
{
    auto_ptr_XMLCh temp(pathname);
    return loadCatalog(temp.get());
}

bool ParserPool::loadCatalog(const XMLCh* pathname)
{
#if _DEBUG
    xmltooling::NDC ndc("loadCatalog");
#endif
    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ParserPool");

    // XML constants
    static const XMLCh catalog[] =  UNICODE_LITERAL_7(c,a,t,a,l,o,g);
    static const XMLCh system[] =   UNICODE_LITERAL_6(s,y,s,t,e,m);
    static const XMLCh systemId[] = UNICODE_LITERAL_8(s,y,s,t,e,m,I,d);
    static const XMLCh uri[] =      UNICODE_LITERAL_3(u,r,i);
    static const XMLCh CATALOG_NS[] = {
        chLatin_u, chLatin_r, chLatin_n, chColon,
        chLatin_o, chLatin_a, chLatin_s, chLatin_i, chLatin_s, chColon,
        chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chColon,
        chLatin_t, chLatin_c, chColon,
        chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chColon,
        chLatin_x, chLatin_m, chLatin_l, chLatin_n, chLatin_s, chColon,
        chLatin_x, chLatin_m, chLatin_l, chColon,
        chLatin_c, chLatin_a, chLatin_t, chLatin_a, chLatin_l, chLatin_o, chLatin_g, chNull
    };

    // Parse the catalog with the internal parser pool.

    if (log.isDebugEnabled()) {
        auto_ptr_char temp(pathname);
        log.debug("loading XML catalog from %s", temp.get());
    }

    LocalFileInputSource fsrc(nullptr,pathname);
    Wrapper4InputSource domsrc(&fsrc,false);
    try {
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(domsrc);
        XercesJanitor<DOMDocument> janitor(doc);

        // Check root element.
        const DOMElement* root=doc->getDocumentElement();
        if (!XMLHelper::isNodeNamed(root,CATALOG_NS,catalog)) {
            auto_ptr_char temp(pathname);
            log.error("unknown root element, failed to load XML catalog from %s", temp.get());
            return false;
        }

        // Fetch all the <system> elements.
        DOMNodeList* mappings=root->getElementsByTagNameNS(CATALOG_NS,system);
        Lock lock(m_lock);
        for (XMLSize_t i=0; i<mappings->getLength(); i++) {
            root=static_cast<DOMElement*>(mappings->item(i));
            const XMLCh* from=root->getAttributeNS(nullptr,systemId);
            const XMLCh* to=root->getAttributeNS(nullptr,uri);
            m_schemaLocMap[from]=to;
        }
        m_schemaLocations.erase();
        for_each(m_schemaLocMap.begin(), m_schemaLocMap.end(), doubleit<xstring>(m_schemaLocations,chSpace));
    }
    catch (std::exception& e) {
        log.error("catalog loader caught exception: %s", e.what());
        return false;
    }

    return true;
}

#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
DOMLSInput* ParserPool::resolveResource(
            const XMLCh *const resourceType,
            const XMLCh *const namespaceUri,
            const XMLCh *const publicId,
            const XMLCh *const systemId,
            const XMLCh *const baseURI
            )
#else
DOMInputSource* ParserPool::resolveEntity(
    const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI
    )
#endif
{
#if _DEBUG
    xmltooling::NDC ndc("resolveEntity");
#endif
    if (!systemId)
        return nullptr;
    xstring sysId(systemId);

    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ParserPool");
    if (log.isDebugEnabled()) {
        auto_ptr_char sysId(systemId);
        auto_ptr_char base(baseURI);
        log.debug("asked to resolve %s with baseURI %s",sysId.get(),base.get() ? base.get() : "(null)");
    }

    // Find well-known schemas in the specified location.
    map<xstring,xstring>::const_iterator i = m_schemaLocMap.find(sysId);
    if (i != m_schemaLocMap.end())
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI, i->second.c_str()));

    // Check for entity as a suffix of a value in the map.
    bool (*p_ends_with)(const xstring&, const xstring&) = ends_with;
    i = find_if(
        m_schemaLocMap.begin(), m_schemaLocMap.end(),
        boost::bind(p_ends_with, boost::bind(&map<xstring,xstring>::value_type::second, _1), boost::ref(sysId))
        );
    if (i != m_schemaLocMap.end())
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI, i->second.c_str()));

    // We'll allow anything without embedded slashes.
    if (XMLString::indexOf(systemId, chForwardSlash) == -1 && XMLString::indexOf(systemId, chBackSlash) == -1)
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI, systemId));

    // Shortcircuit the request.
    auto_ptr_char temp(systemId);
    log.debug("unauthorized entity request (%s), blocking it", temp.get());
    static const XMLByte nullbuf[] = {0};
    return new Wrapper4InputSource(new MemBufInputSource(nullbuf, 0, systemId));
}

#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS

DOMLSParser* ParserPool::createBuilder()
{
    static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);
    DOMLSParser* parser=static_cast<DOMImplementationLS*>(impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS,nullptr);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, m_namespaceAware);
    if (m_schemaAware) {
        parser->getDomConfig()->setParameter(XMLUni::fgDOMNamespaces, true);
        parser->getDomConfig()->setParameter(XMLUni::fgXercesSchema, true);
        parser->getDomConfig()->setParameter(XMLUni::fgDOMValidate, true);
        parser->getDomConfig()->setParameter(XMLUni::fgXercesCacheGrammarFromParse, true);

        // We build a "fake" schema location hint that binds each namespace to itself.
        // This ensures the entity resolver will be given the namespace as a systemId it can check.
        parser->getDomConfig()->setParameter(XMLUni::fgXercesSchemaExternalSchemaLocation, const_cast<XMLCh*>(m_schemaLocations.c_str()));
    }
    parser->getDomConfig()->setParameter(XMLUni::fgXercesUserAdoptsDOMDocument, true);
    parser->getDomConfig()->setParameter(XMLUni::fgXercesDisableDefaultEntityResolution, true);
    parser->getDomConfig()->setParameter(XMLUni::fgDOMResourceResolver, dynamic_cast<DOMLSResourceResolver*>(this));
    parser->getDomConfig()->setParameter(XMLUni::fgXercesSecurityManager, m_security.get());
    return parser;
}

DOMLSParser* ParserPool::checkoutBuilder()
{
    Lock lock(m_lock);
    if (m_pool.empty()) {
        DOMLSParser* builder=createBuilder();
        return builder;
    }
    DOMLSParser* p=m_pool.top();
    m_pool.pop();
    if (m_schemaAware)
        p->getDomConfig()->setParameter(XMLUni::fgXercesSchemaExternalSchemaLocation, const_cast<XMLCh*>(m_schemaLocations.c_str()));
    return p;
}

void ParserPool::checkinBuilder(DOMLSParser* builder)
{
    if (builder) {
        Lock lock(m_lock);
        m_pool.push(builder);
    }
}

#else

DOMBuilder* ParserPool::createBuilder()
{
    static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);
    DOMBuilder* parser=static_cast<DOMImplementationLS*>(impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,0);
    parser->setFeature(XMLUni::fgDOMNamespaces, m_namespaceAware);
    if (m_schemaAware) {
        parser->setFeature(XMLUni::fgDOMNamespaces, true);
        parser->setFeature(XMLUni::fgXercesSchema, true);
        parser->setFeature(XMLUni::fgDOMValidation, true);
        parser->setFeature(XMLUni::fgXercesCacheGrammarFromParse, true);

        // We build a "fake" schema location hint that binds each namespace to itself.
        // This ensures the entity resolver will be given the namespace as a systemId it can check.
        parser->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(m_schemaLocations.c_str()));
    }
    parser->setProperty(XMLUni::fgXercesSecurityManager, m_security.get());
    parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument, true);
    parser->setFeature(XMLUni::fgXercesDisableDefaultEntityResolution, true);
    parser->setEntityResolver(this);
    return parser;
}

DOMBuilder* ParserPool::checkoutBuilder()
{
    Lock lock(m_lock);
    if (m_pool.empty()) {
        DOMBuilder* builder=createBuilder();
        return builder;
    }
    DOMBuilder* p=m_pool.top();
    m_pool.pop();
    if (m_schemaAware)
        p->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(m_schemaLocations.c_str()));
    return p;
}

void ParserPool::checkinBuilder(DOMBuilder* builder)
{
    if (builder) {
        Lock lock(m_lock);
        m_pool.push(builder);
    }
}

#endif

StreamInputSource::StreamInputSource(istream& is, const char* systemId) : InputSource(systemId), m_is(is)
{
}

BinInputStream* StreamInputSource::makeStream() const
{
    return new StreamBinInputStream(m_is);
}

StreamInputSource::StreamBinInputStream::StreamBinInputStream(istream& is) : m_is(is), m_pos(0)
{
}

#ifdef XMLTOOLING_XERCESC_64BITSAFE
XMLFilePos
#else
unsigned int
#endif
StreamInputSource::StreamBinInputStream::curPos() const
{
    return m_pos;
}

#ifdef XMLTOOLING_XERCESC_64BITSAFE
const XMLCh* StreamInputSource::StreamBinInputStream::getContentType() const
{
    return nullptr;
}
#endif

xsecsize_t StreamInputSource::StreamBinInputStream::readBytes(XMLByte* const toFill, const xsecsize_t maxToRead)
{
    XMLByte* target=toFill;
    xsecsize_t bytes_read=0,request=maxToRead;

    // Fulfill the rest by reading from the stream.
    if (request && !m_is.eof() && !m_is.fail()) {
        try {
            m_is.read(reinterpret_cast<char* const>(target),request);
            m_pos+=m_is.gcount();
            bytes_read+=m_is.gcount();
        }
        catch(ios_base::failure& e) {
            Category::getInstance(XMLTOOLING_LOGCAT".StreamInputSource").critStream()
                << "XML::StreamInputSource::StreamBinInputStream::readBytes caught an exception: " << e.what()
                << logging::eol;
            *toFill=0;
            return 0;
        }
    }
    return bytes_read;
}

#ifdef XMLTOOLING_LITE

URLInputSource::URLInputSource(const XMLCh* url, const char* systemId, string* cacheTag) : InputSource(systemId), m_url(url)
{
}

URLInputSource::URLInputSource(const DOMElement* e, const char* systemId, string* cacheTag) : InputSource(systemId)
{
    static const XMLCh uri[] = UNICODE_LITERAL_3(u,r,i);
    static const XMLCh url[] = UNICODE_LITERAL_3(u,r,l);

    const XMLCh* attr = e->getAttributeNS(nullptr, url);
    if (!attr || !*attr) {
        attr = e->getAttributeNS(nullptr, uri);
        if (!attr || !*attr)
            throw IOException("No URL supplied via DOM to URLInputSource constructor.");
    }

    m_url.setURL(attr);
}

BinInputStream* URLInputSource::makeStream() const
{
    // Ask the URL to create us an appropriate input stream
    return m_url.makeNewStream();
}

#else

URLInputSource::URLInputSource(const XMLCh* url, const char* systemId, string* cacheTag)
    : InputSource(systemId), m_cacheTag(cacheTag), m_url(url), m_root(nullptr)
{
}

URLInputSource::URLInputSource(const DOMElement* e, const char* systemId, string* cacheTag)
    : InputSource(systemId), m_cacheTag(cacheTag), m_root(e)
{
}

BinInputStream* URLInputSource::makeStream() const
{
    return m_root ? new CurlURLInputStream(m_root, m_cacheTag) : new CurlURLInputStream(m_url.get(), m_cacheTag);
}

#endif

const char URLInputSource::asciiStatusCodeElementName[] = "URLInputSourceStatus";

const XMLCh URLInputSource::utf16StatusCodeElementName[] = UNICODE_LITERAL_20(U,R,L,I,n,p,u,t,S,o,u,r,c,e,S,t,a,t,u,s);
