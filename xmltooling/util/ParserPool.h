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
 * @file ParserPool.h
 * 
 * XML parsing
 */

#if !defined(__xmltooling_pool_h__)
#define __xmltooling_pool_h__

#include <xmltooling/unicode.h>
#include <xmltooling/util/Threads.h>

#include <map>
#include <stack>
#include <istream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * A thread-safe pool of DOMBuilders that share characteristics
     */
    class XMLTOOL_API ParserPool : public xercesc::DOMEntityResolver, xercesc::DOMErrorHandler
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
         * @param domsrc A DOM source containing the content to be parsed
         * @return The DOM document resulting from the parse
         * @throws XMLParserException thrown if there was a problem reading, parsing, or validating the XML
         */
        xercesc::DOMDocument* parse(xercesc::DOMInputSource& domsrc);

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
        xercesc::DOMInputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI);

        /**
         * Handles parsing errors
         */
        bool handleError(const xercesc::DOMError& e);

    private:
        xercesc::DOMBuilder* createBuilder();
        xercesc::DOMBuilder* checkoutBuilder();
        void checkinBuilder(xercesc::DOMBuilder* builder);

#ifdef HAVE_GOOD_STL
        xstring m_schemaLocations;
        std::map<xstring,xstring> m_schemaLocMap;
#else
        std::string m_schemaLocations;
        std::map<std::string,std::string> m_schemaLocMap;
#endif
        bool m_namespaceAware,m_schemaAware;
        std::stack<xercesc::DOMBuilder*> m_pool;
        Mutex* m_lock;
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
             * @param is        reference to an input stream
             */
            StreamBinInputStream(std::istream& is) : m_is(is), m_pos(0) {}
            /// @cond off
            virtual unsigned int curPos() const { return m_pos; }
            virtual unsigned int readBytes(XMLByte* const toFill, const unsigned int maxToRead);
            /// @endcond
        private:
            std::istream& m_is;
            unsigned int m_pos;
        };

    private:
        std::istream& m_is;
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_pool_h__ */
