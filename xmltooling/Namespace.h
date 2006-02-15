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
 * @file Namespace.h
 * 
 * Representing XML namespace attributes 
 */

#if !defined(__xmltooling_namespace_h__)
#define __xmltooling_namespace_h__

#include <string>
#include <xmltooling/Namespace.h>

namespace xmltooling {
    
    /**
     * A data structure for encapsulating XML Namespace attributes
     */
    class XMLTOOL_API Namespace
    {
    public:
        /**
         * Default constructor
         */
        Namespace();
        
        /**
         * Constructor that takes an existing declaration
         * @param uri       namespace URI
         * @param prefix    namespace prefix (without the colon)
         */
        Namespace(const XMLCh* uri, const XMLCh* prefix);
        
        ~Namespace();
#ifndef HAVE_GOOD_STL
        /**
         * Deep copy constructor
         */
        Namespace(const Namespace&);

        /**
         * Deep assignment operator
         */
        Namespace& operator=(const Namespace&);
#endif
        
#ifdef HAVE_GOOD_STL
        /**
         * Returns the namespace prefix
         * @return  Null-terminated Unicode string containing the prefix, without the colon
         */
        const XMLCh* getNamespacePrefix() const { return m_prefix.c_str(); }

        /**
         * Returns the namespace URI
         * @return  Null-terminated Unicode string containing the URI
         */
        const XMLCh* getNamespaceURI() const { return m_uri.c_str(); }
#else
        /**
         * Returns the namespace prefix
         * @return  Null-terminated Unicode string containing the prefix, without the colon
         */
        const XMLCh* getNamespacePrefix() const { return m_prefix; }

        /**
         * Returns the namespace URI
         * @return  Null-terminated Unicode string containing the URI
         */
        const XMLCh* getNamespaceURI() const { return m_uri; }
#endif

        /**
         * Sets the namespace prefix
         * @param prefix    Null-terminated Unicode string containing the prefix, without the colon
         */
        void setNamespacePrefix(const XMLCh* prefix);

        /**
         * Sets the namespace URI
         * @param uri  Null-terminated Unicode string containing the URI
         */
        void setNamespaceURI(const XMLCh* uri);
        
    private:
#ifdef HAVE_GOOD_STL
        xstring m_uri;
        xstring m_prefix;
#else
        XMLCh* m_uri;
        XMLCh* m_prefix;
#endif
    };

    /**
     * Returns true iff op1's namespace lexically compares less than op2's namespace,
     * or if equal, iff op1's prefix lexically compares less than op2's prefix.
     * 
     * Needed for use with sorted STL containers.
     * 
     * @param op1   First namspace to compare
     * @param op2   Second namespace to compare
     */
    extern XMLTOOL_API bool operator<(const Namespace& op1, const Namespace& op2);

#ifndef HAVE_GOOD_STL
    /**
     * Returns true iff op1's namespace and prefix are equal to op2's namespace and prefix.
     * @param op1   First namspace to compare
     * @param op2   Second namespace to compare
     */
    extern XMLTOOL_API bool operator==(const Namespace& op1, const Namespace& op2);
#endif

};

#endif /* __xmltooling_namespace_h__ */
