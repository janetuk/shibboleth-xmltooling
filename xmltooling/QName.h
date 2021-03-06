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
         * 
         * @param uri       namespace URI
         * @param localPart local name
         * @param prefix    namespace prefix (without the colon)
         */
        QName(const XMLCh* uri=nullptr, const XMLCh* localPart=nullptr, const XMLCh* prefix=nullptr);

        /**
         * Constructor
         * 
         * @param uri       namespace URI
         * @param localPart local name
         * @param prefix    namespace prefix (without the colon)
         */
        QName(const char* uri, const char* localPart, const char* prefix=nullptr);
        
        ~QName();
        
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
         * Sets the namespace prefix
         * @param prefix    Null-terminated ASCII string containing the prefix, without the colon
         */
        void setPrefix(const char* prefix);

        /**
         * Sets the namespace URI
         * @param uri  Null-terminated ASCII string containing the URI
         */
        void setNamespaceURI(const char* uri);
        
        /**
         * Sets the local part of the name
         * @param localPart  Null-terminated ASCII string containing the local name
         */
        void setLocalPart(const char* localPart);
        
        /**
         * Gets a string representation of the QName for logging, etc.
         * Format is prefix:localPart or {namespaceURI}localPart if no prefix.
         * 
         * @return the string representation
         */
        std::string toString() const;
        
    private:
        xstring m_uri;
        xstring m_local;
        xstring m_prefix;
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
