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
 * ParserPool.cpp
 * 
 * XML parsing
 */

#include "internal.h"
#include "exceptions.h"
#include "util/NDC.h"
#include "util/ParserPool.h"
#include "util/XMLHelper.h"

#include <algorithm>
#include <functional>
#include <sys/types.h>
#include <sys/stat.h>
#include <log4cpp/Category.hh>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

using namespace xmltooling;
using namespace std;
using namespace log4cpp;

ParserPool::ParserPool(bool namespaceAware, bool schemaAware)
    : m_namespaceAware(namespaceAware), m_schemaAware(schemaAware), m_lock(Mutex::create()) {}

ParserPool::~ParserPool()
{
    while(!m_pool.empty()) {
        m_pool.top()->release();
        m_pool.pop();
    }
    delete m_lock;
}

DOMDocument* ParserPool::newDocument()
{
    return DOMImplementationRegistry::getDOMImplementation(NULL)->createDocument();
}

DOMDocument* ParserPool::parse(DOMInputSource& domsrc)
{
    DOMBuilder* parser=checkoutBuilder();
    XercesJanitor<DOMBuilder> janitor(parser);
    try {
        DOMDocument* doc=parser->parse(domsrc);
        parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument,true);
        checkinBuilder(janitor.release());
        return doc;
    }
    catch (XMLException&) {
        checkinBuilder(janitor.release());
        throw;
    }
    catch (XMLToolingException&) {
        checkinBuilder(janitor.release());
        throw;
    }
}

DOMDocument* ParserPool::parse(istream& is)
{
    StreamInputSource src(is);
    Wrapper4InputSource domsrc(&src,false);
    return parse(domsrc);
}

// Functor to double its argument separated by a character and append to a buffer
template <class T> class doubleit
{
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
#ifdef HAVE_GOOD_STL
    m_schemaLocMap[nsURI]=pathname;
    m_schemaLocations.erase();
    for_each(m_schemaLocMap.begin(),m_schemaLocMap.end(),doubleit<xstring>(m_schemaLocations,chSpace));
#else
    auto_ptr_char n(nsURI);
    m_schemaLocMap[n.get()]=p.get();
    m_schemaLocations.erase();
    for_each(m_schemaLocMap.begin(),m_schemaLocMap.end(),doubleit<string>(m_schemaLocations,' '));
#endif

    return true;
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

    LocalFileInputSource fsrc(NULL,pathname);
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
            const XMLCh* from=root->getAttributeNS(NULL,systemId);
            const XMLCh* to=root->getAttributeNS(NULL,uri);
#ifdef HAVE_GOOD_STL
            m_schemaLocMap[from]=to;
#else
            auto_ptr_char f(from);
            auto_ptr_char t(to);
            m_schemaLocMap[f.get()]=t.get();
#endif
        }
        m_schemaLocations.erase();
#ifdef HAVE_GOOD_STL
        for_each(m_schemaLocMap.begin(),m_schemaLocMap.end(),doubleit<xstring>(m_schemaLocations,chSpace));
#else
        for_each(m_schemaLocMap.begin(),m_schemaLocMap.end(),doubleit<string>(m_schemaLocations,' '));
#endif
    }
    catch (exception& e) {
        log.error("catalog loader caught exception: %s", e.what());
        return false;
    }

    return true;
}

DOMInputSource* ParserPool::resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI)
{
#if _DEBUG
    xmltooling::NDC ndc("resolveEntity");
#endif
    if (!systemId)
        return NULL;

    Category& log=Category::getInstance(XMLTOOLING_LOGCAT".ParserPool");
    if (log.isDebugEnabled()) {
        auto_ptr_char sysId(systemId);
        auto_ptr_char base(baseURI);
        log.debug("asked to resolve %s with baseURI %s",sysId.get(),base.get() ? base.get() : "(null)");
    }

#ifdef HAVE_GOOD_STL
    // Find well-known schemas in the specified location.
    map<xstring,xstring>::const_iterator i=m_schemaLocMap.find(systemId);
    if (i!=m_schemaLocMap.end())
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI,i->second.c_str()));

    // Check for entity as a value in the map.
    for (i=m_schemaLocMap.begin(); i!=m_schemaLocMap.end(); ++i) {
        if (XMLString::endsWith(i->second.c_str(), systemId))
            return new Wrapper4InputSource(new LocalFileInputSource(baseURI,i->second.c_str()));
    }

    // We'll allow anything without embedded slashes.
    if (XMLString::indexOf(systemId, chForwardSlash)==-1)
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI,systemId));
#else
    // Find well-known schemas in the specified location.
    auto_ptr_char temp(systemId);
    map<string,string>::const_iterator i=m_schemaLocMap.find(temp.get());
    if (i!=m_schemaLocMap.end()) {
        auto_ptr_XMLCh temp2(i->second.c_str());
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI,temp2.get()));
    }

    // Check for entity as a value in the map.
    for (i=m_schemaLocMap.begin(); i!=m_schemaLocMap.end(); ++i) {
        auto_ptr_XMLCh temp2(i->second.c_str());
        if (XMLString::endsWith(temp2.get(), systemId))
            return new Wrapper4InputSource(new LocalFileInputSource(baseURI,temp2.get()));
    }

    // We'll allow anything without embedded slashes.
    if (XMLString::indexOf(systemId, chForwardSlash)==-1)
        return new Wrapper4InputSource(new LocalFileInputSource(baseURI,systemId));
#endif    

    // Shortcircuit the request.
#ifdef HAVE_GOOD_STL
    auto_ptr_char temp(systemId);
#endif
    log.debug("unauthorized entity request (%s), blocking it", temp.get());
    static const XMLByte nullbuf[] = {0};
    return new Wrapper4InputSource(new MemBufInputSource(nullbuf,0,systemId));
}

bool ParserPool::handleError(const DOMError& e)
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
                << ", message: " << temp.get() << CategoryStream::ENDLINE;
            return true;

        case DOMError::DOM_SEVERITY_ERROR:
            log.errorStream() << "error on line " << locator->getLineNumber()
                << ", column " << locator->getColumnNumber()
                << ", message: " << temp.get() << CategoryStream::ENDLINE;
            throw XMLParserException(string("error during XML parsing: ") + (temp.get() ? temp.get() : "no message"));

        case DOMError::DOM_SEVERITY_FATAL_ERROR:
            log.critStream() << "fatal error on line " << locator->getLineNumber()
                << ", column " << locator->getColumnNumber()
                << ", message: " << temp.get() << CategoryStream::ENDLINE;
            throw XMLParserException(string("fatal error during XML parsing: ") + (temp.get() ? temp.get() : "no message"));
    }
    throw XMLParserException(string("unclassified error during XML parsing: ") + (temp.get() ? temp.get() : "no message"));
}

DOMBuilder* ParserPool::createBuilder()
{
    static const XMLCh impltype[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation* impl=DOMImplementationRegistry::getDOMImplementation(impltype);
    DOMBuilder* parser=static_cast<DOMImplementationLS*>(impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS,0);
    if (m_namespaceAware)
        parser->setFeature(XMLUni::fgDOMNamespaces,true);
    if (m_schemaAware) {
        parser->setFeature(XMLUni::fgXercesSchema,true);
        parser->setFeature(XMLUni::fgDOMValidation,true);
        parser->setFeature(XMLUni::fgXercesCacheGrammarFromParse,true);
        parser->setFeature(XMLUni::fgXercesValidationErrorAsFatal,true);
        
        // We build a "fake" schema location hint that binds each namespace to itself.
        // This ensures the entity resolver will be given the namespace as a systemId it can check. 
#ifdef HAVE_GOOD_STL
        parser->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(m_schemaLocations.c_str()));
#else
        auto_ptr_XMLCh temp(m_schemaLocations.c_str());
        parser->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(temp.get()));
#endif
    }
    parser->setFeature(XMLUni::fgXercesUserAdoptsDOMDocument,true);
    parser->setEntityResolver(this);
    parser->setErrorHandler(this);
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
    if (m_schemaAware) {
#ifdef HAVE_GOOD_STL
        p->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(m_schemaLocations.c_str()));
#else
        auto_ptr_XMLCh temp2(m_schemaLocations.c_str());
        p->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation,const_cast<XMLCh*>(temp2.get()));
#endif
    }
    return p;
}

void ParserPool::checkinBuilder(DOMBuilder* builder)
{
    if (builder) {
        Lock lock(m_lock);
        m_pool.push(builder);
    }
}

unsigned int StreamInputSource::StreamBinInputStream::readBytes(XMLByte* const toFill, const unsigned int maxToRead)
{
    XMLByte* target=toFill;
    unsigned int bytes_read=0,request=maxToRead;

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
                << CategoryStream::ENDLINE;
            *toFill=0;
            return 0;
        }
    }
    return bytes_read;
}
