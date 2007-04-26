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
 * @file xmltooling/QName.h
 * 
 * Representing XML QNames 
 */

#ifndef __xmltooling_qname_h__
#define __xmltooling_qname_h__

#include <xmltooling/unicode.h>
#include <algorithm>

namespace xmltooling {

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

    /**
     * A data structure for encapsulating XML QNames.
     * The Xerces class is too limited to use at the moment.
     */
    class XMLTOOL_API QName
    {
    public:
        /**
         * Constructor
         * @param uri       namespace URI
         * @param localPart local name
         * @param prefix    namespace prefix (without the colon)
         */
        QName(const XMLCh* uri=NULL, const XMLCh* localPart=NULL, const XMLCh* prefix=NULL);
        
        ~QName();
#ifndef HAVE_GOOD_STL
        /**
         * Deep copy constructor
         */
        QName(const QName& src);

        /**
         * Deep assignment operator
         */
        QName& operator=(const QName& src);
#endif
        
#ifdef HAVE_GOOD_STL
        /**
         * Indicates whether the QName has a prefix.
         * @return  true iff the prefix is non-empty
         */
        bool hasPrefix() const { return !m_prefix.empty(); }

        /**
         * Indicates whether the QName has a non-empty namespace.
         * @return  true iff the namespace is non-empty
         */
        bool hasNamespaceURI() const { return !m_uri.empty(); }

        /**
         * Indicates whether the QName has a non-empty local name.
         * @return  true iff the local name is non-empty
         */
        bool hasLocalPart() const { return !m_local.empty(); }

        /**
         * Returns the namespace prefix
         * @return  Null-terminated Unicode string containing the prefix, without the colon
         */
        const XMLCh* getPrefix() const { return m_prefix.c_str(); }

        /**
         * Returns the namespace URI
         * @return  Null-terminated Unicode string containing the URI
         */
        const XMLCh* getNamespaceURI() const { return m_uri.c_str(); }

        /**
         * Returns the local part of the name
         * @return  Null-terminated Unicode string containing the local name
         */
        const XMLCh* getLocalPart() const { return m_local.c_str(); }
#else
        /**
         * Indicates whether the QName has a prefix.
         * @return  true iff the prefix is non-empty
         */
        bool hasPrefix() const { return m_prefix && *m_prefix; }

        /**
         * Indicates whether the QName has a non-empty namespace.
         * @return  true iff the namespace is non-empty
         */
        bool hasNamespaceURI() const { return m_uri && *m_uri; }

        /**
         * Indicates whether the QName has a non-empty local name.
         * @return  true iff the local name is non-empty
         */
        bool hasLocalPart() const { return m_local && *m_local; }

        /**
         * Returns the namespace prefix
         * @return  Null-terminated Unicode string containing the prefix, without the colon
         */
        const XMLCh* getPrefix() const { return m_prefix; }

        /**
         * Returns the namespace URI
         * @return  Null-terminated Unicode string containing the URI
         */
        const XMLCh* getNamespaceURI() const { return m_uri; }

        /**
         * Returns the local part of the name
         * @return  Null-terminated Unicode string containing the local name
         */
        const XMLCh* getLocalPart() const { return m_local; }
#endif

        /**
         * Sets the namespace prefix
         * @param prefix    Null-terminated Unicode string containing the prefix, without the colon
         */
        void setPrefix(const XMLCh* prefix);

        /**
         * Sets the namespace URI
         * @param uri  Null-terminated Unicode string containing the URI
         */
        void setNamespaceURI(const XMLCh* uri);
        
        /**
         * Sets the local part of the name
         * @param localPart  Null-terminated Unicode string containing the local name
         */
        void setLocalPart(const XMLCh* localPart);
        
        /**
         * Gets a string representation of the QName for logging, etc.
         * Format is prefix:localPart or {namespaceURI}localPart if no prefix.
         * 
         * @return the string representation
         */
        std::string toString() const;
        
    private:
#ifdef HAVE_GOOD_STL
        xstring m_uri;
        xstring m_local;
        xstring m_prefix;
#else
        XMLCh* m_uri;
        XMLCh* m_local;
        XMLCh* m_prefix;
#endif
    };

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

    /**
     * Returns true iff op1's namespace lexically compares less than op2's namespace,
     * or if equal, iff op1's prefix lexically compares less than op2's prefix.
     * 
     * Needed for use with sorted STL containers.
     * 
     * @param op1   First qname to compare
     * @param op2   Second qname to compare
     */
    extern XMLTOOL_API bool operator<(const QName& op1, const QName& op2);

    /**
     * Returns true iff op1's components are equal to op2's components, excluding prefix.
     * @param op1   First qname to compare
     * @param op2   Second qname to compare
     */
    extern XMLTOOL_API bool operator==(const QName& op1, const QName& op2);

    /**
     * Returns true iff op1's components are not equal to op2's components, excluding prefix.
     * @param op1   First qname to compare
     * @param op2   Second qname to compare
     */
    extern XMLTOOL_API bool operator!=(const QName& op1, const QName& op2);

};

#endif /* __xmltooling_qname_h__ */
