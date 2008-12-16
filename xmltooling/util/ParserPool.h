/*
 *  Copyright 2001-2008 Internet2
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
 * @file xmltooling/util/ParserPool.h
 * 
 * A thread-safe pool of parsers that share characteristics.
 */

#ifndef __xmltooling_pool_h__
#define __xmltooling_pool_h__

#include <xmltooling/unicode.h>
#include <xmltooling/util/Threads.h>

#include <map>
#include <stack>
#include <istream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/SecurityManager.hpp>

#ifndef XMLTOOLING_NO_XMLSEC
# include <xsec/framework/XSECDefs.hpp>
#endif

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A thread-safe pool of DOMBuilders that share characteristics.
     */
    class XMLTOOL_API ParserPool :
#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
        public xercesc::DOMLSResourceResolver
#else
        public xercesc::DOMEntityResolver
#endif
    {
        MAKE_NONCOPYABLE(ParserPool);
    public:
        /**
         * Constructs a new pool
         * 
         * @param namespaceAware    indicates whether parsers should be namespace-aware or not
         * @param schemaAware       indicates whether parsers should be schema-validating or not
         */
        ParserPool(bool namespaceAware=true, bool schemaAware=false);
        ~ParserPool();

        /**
         * Creates a new document using a parser from this pool.
         * 
         * @return new XML document
         * 
         */
        xercesc::DOMDocument* newDocument();

        /**
         * Parses a document using a pooled parser with the proper settings
         * 
         * @param domsrc An input source containing the content to be parsed
         * @return The DOM document resulting from the parse
         * @throws XMLParserException thrown if there was a problem reading, parsing, or validating the XML
         */
        xercesc::DOMDocument* parse(
#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
            xercesc::DOMLSInput& domsrc
#else
            xercesc::DOMInputSource& domsrc
#endif
            );

        /**
         * Parses a document using a pooled parser with the proper settings
         * 
         * @param is An input stream containing the content to be parsed
         * @return The DOM document resulting from the parse
         * @throws XMLParserException thrown if there was a problem reading, parsing, or validating the XML
         */
        xercesc::DOMDocument* parse(std::istream& is);

        /**
         * Load an OASIS catalog file to map schema namespace URIs to filenames.
         * 
         * This does not provide real catalog support; only the &lt;uri&gt; element
         * is supported to map from a namespace URI to a relative path or file:// URI.
         * 
         * @param pathname  path to a catalog file
         * @return true iff the catalog was successfully processed
         */
        bool loadCatalog(const XMLCh* pathname);
        
        /**
         * Load a schema explicitly from a local file.
         * 
         * Note that "successful processing" does not imply that the schema is valid,
         * only that a reference to it was successfully registered with the pool.
         * 
         * @param nsURI     XML namespace to load
         * @param pathname  path to schema file
         * @return true iff the schema was successfully processed
         */
        bool loadSchema(const XMLCh* nsURI, const XMLCh* pathname);

        /**
         * Supplies all external entities (primarily schemas) to the parser
         */
#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
        xercesc::DOMLSInput* resolveResource(
            const XMLCh *const resourceType,
            const XMLCh *const namespaceUri,
            const XMLCh *const publicId,
            const XMLCh *const systemId,
            const XMLCh *const baseURI
            );
#else
        xercesc::DOMInputSource* resolveEntity(
            const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI
            );
#endif

    private:
#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
        xercesc::DOMLSParser* createBuilder();
        xercesc::DOMLSParser* checkoutBuilder();
        void checkinBuilder(xercesc::DOMLSParser* builder);
#else
        xercesc::DOMBuilder* createBuilder();
        xercesc::DOMBuilder* checkoutBuilder();
        void checkinBuilder(xercesc::DOMBuilder* builder);
#endif

#ifdef HAVE_GOOD_STL
        xstring m_schemaLocations;
        std::map<xstring,xstring> m_schemaLocMap;
#else
        std::string m_schemaLocations;
        std::map<std::string,std::string> m_schemaLocMap;
#endif
        bool m_namespaceAware,m_schemaAware;
#ifdef XMLTOOLING_XERCESC_COMPLIANT_DOMLS
        std::stack<xercesc::DOMLSParser*> m_pool;
#else
        std::stack<xercesc::DOMBuilder*> m_pool;
#endif
        Mutex* m_lock;
        xercesc::SecurityManager* m_security;
    };

    /**
     * A parser source that wraps a C++ input stream
     */
    class XMLTOOL_API StreamInputSource : public xercesc::InputSource
    {
    MAKE_NONCOPYABLE(StreamInputSource);
    public:
        /**
         * Constructs an input source around an input stream reference.
         * 
         * @param is        reference to an input stream
         * @param systemId  optional system identifier to attach to the stream
         */
        StreamInputSource(std::istream& is, const char* systemId=NULL) : xercesc::InputSource(systemId), m_is(is) {}
        /// @cond off
        virtual xercesc::BinInputStream* makeStream() const { return new StreamBinInputStream(m_is); }
        /// @endcond

        /**
         * A Xerces input stream that wraps a C++ input stream
         */
        class XMLTOOL_API StreamBinInputStream : public xercesc::BinInputStream
        {
        public:
            /**
             * Constructs a Xerces input stream around a C++ input stream reference.
             * 
             * @param is            reference to an input stream
             */
            StreamBinInputStream(std::istream& is) : m_is(is), m_pos(0) {}
            /// @cond off
#ifdef XMLTOOLING_XERCESC_64BITSAFE
            XMLFilePos
#else
            unsigned int
#endif
                curPos() const { return m_pos; }
            xsecsize_t readBytes(XMLByte* const toFill, const xsecsize_t maxToRead);
#ifdef XMLTOOLING_XERCESC_64BITSAFE
            const XMLCh* getContentType() const { return NULL; }
#endif
            /// @endcond
        private:
            std::istream& m_is;
            xsecsize_t m_pos;
        };

    private:
        std::istream& m_is;
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_pool_h__ */
