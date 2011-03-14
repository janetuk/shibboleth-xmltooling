/*
 *  Copyright 2001-2010 Internet2
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
 * @file xmltooling/Namespace.h
 * 
 * Representing XML namespace attributes.
 */

#if !defined(__xmltooling_namespace_h__)
#define __xmltooling_namespace_h__

#include <xmltooling/unicode.h>

namespace xmltooling {

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

    /**
     * A data structure for encapsulating XML Namespace attributes.
     */
    class XMLTOOL_API Namespace
    {
    public:
        /**
         * Tri-state indicator of namespace usage.
         */
        enum namespace_usage_t {
            Indeterminate,
            NonVisiblyUsed,
            VisiblyUsed
        };

        /**
         * Constructor
         * @param uri               namespace URI
         * @param prefix            namespace prefix (without the colon)
         * @param alwaysDeclare     true iff the namespace should always be declared regardless of in-scope declarations
         * @param usage             indicates usage of namespace in the context of an XMLObject
         */
        Namespace(const XMLCh* uri=nullptr, const XMLCh* prefix=nullptr, bool alwaysDeclare=false, namespace_usage_t usage=Indeterminate);
        
        ~Namespace();
        
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

        /**
         * Returns true iff the namespace should always be declared regardless of in-scope declarations
         * @return the alwaysDeclared setting
         */
        const bool alwaysDeclare() const { return m_pinned; }

        /**
         * Returns the usage of the namespace by an XMLObject
         * @return the usage setting
         */
        const namespace_usage_t usage() const { return m_usage; }

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

        /**
         * Sets the alwaysDeclared property
         * @param alwaysDeclare     true iff the namespace should always be declared regardless of in-scope declarations
         */
        void setAlwaysDeclare(bool alwaysDeclare) { m_pinned = alwaysDeclare; } 
        
        /**
         * Sets the usage property
         * @param usage usage of the namespace by an XMLObject
         */
        void setUsage(namespace_usage_t usage) { m_usage = usage; }

    private:
        bool m_pinned;
        namespace_usage_t m_usage;
        xstring m_uri;
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
     * @param op1   First namspace to compare
     * @param op2   Second namespace to compare
     */
    extern XMLTOOL_API bool operator<(const Namespace& op1, const Namespace& op2);

    /**
     * Returns true iff op1's namespace and prefix are equal to op2's namespace and prefix.
     * @param op1   First namspace to compare
     * @param op2   Second namespace to compare
     */
    extern XMLTOOL_API bool operator==(const Namespace& op1, const Namespace& op2);

};

#endif /* __xmltooling_namespace_h__ */
