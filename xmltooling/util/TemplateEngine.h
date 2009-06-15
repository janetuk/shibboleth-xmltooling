/*
 *  Copyright 2001-2009 Internet2
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
 * @file xmltooling/util/TemplateEngine.h
 *
 * Simple template replacement engine.
 */

#ifndef __xmltooling_template_h__
#define __xmltooling_template_h__

#include <xmltooling/io/GenericRequest.h>

#include <map>
#include <string>
#include <iostream>
#include <vector>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

namespace xmltooling {

    /**
     * Simple template replacement engine. Supports the following:
     * <ul>
     *  <li> &lt;mlp key/&gt; </li>
     *  <li> &lt;mlpif key&gt; stuff &lt;/mlpif&gt;</li>
     *  <li> &lt;mlpifnot key&gt; stuff &lt;/mlpifnot&gt;</li>
     *  <li> &lt;mlpfor key&gt; stuff &lt;/mlpfor&gt;</li>
     *  <li> &lt;mlp $name/&gt; (in for loop only) </li>
     *  <li> &lt;mlp $value/&gt; (in for loop only) </li>
     * </ul>
     *
     * The default tag prefix is "mlp". This can be overridden for
     * compatibility.
     */
    class XMLTOOL_API TemplateEngine
    {
        MAKE_NONCOPYABLE(TemplateEngine);
    public:
        TemplateEngine() {
            setTagPrefix("mlp");
        }

        virtual ~TemplateEngine() {}

        /**
         * Sets the tag name to use when locating template replacement tags.
         *
         * @param tagPrefix base prefix for tags
         */
        void setTagPrefix(const char* tagPrefix);

        /**
         * Interface to parameters to plug into templates.
         * Allows callers to supply a more dynamic lookup mechanism to supplement a basic map.
         */
        class XMLTOOL_API TemplateParameters {
            // MAKE_NONCOPYABLE(TemplateParameters);
        public:
            TemplateParameters() : m_request(NULL) {}
            virtual ~TemplateParameters() {}

            /** Map of known parameters to supply to template. */
            std::map<std::string,std::string> m_map;

            /** Map of sub-collections used in for loops. */
            std::map< std::string,std::multimap<std::string,std::string> > m_collectionMap;

            /** Request from client that resulted in template being processed. */
            const GenericRequest* m_request;

            /**
             * Returns the value of a parameter to plug into the template.
             *
             * @param name  name of parameter
             * @return value of parameter, or NULL
             */
            virtual const char* getParameter(const char* name) const {
                std::map<std::string,std::string>::const_iterator i=m_map.find(name);
                return (i!=m_map.end() ? i->second.c_str() : (m_request ? m_request->getParameter(name) : NULL));
            }

            /**
             * Returns a named collection of sub-parameters to pass into a loop.
             *
             * @param name  name of sub-collection
             * @return pointer to a multimap of sub-parameters, or NULL
             */
            virtual const std::multimap<std::string,std::string>* getLoopCollection(const char* name) const {
                std::map< std::string,std::multimap<std::string,std::string> >::const_iterator i=m_collectionMap.find(name);
                return (i!=m_collectionMap.end() ? &(i->second) : NULL);
            }
        };

        /**
         * Processes template from an input stream and executes replacements and
         * conditional logic based on parameters.
         *
         * @param is            input stream providing template
         * @param os            output stream to send results of executing template
         * @param parameters    parameters to plug into template
         * @param e             optional exception to extract parameters from
         */
        virtual void run(
            std::istream& is,
            std::ostream& os,
            const TemplateParameters& parameters,
            const XMLToolingException* e=NULL
            ) const;

        /**
         * List of non-built-in characters considered "unsafe" and requiring HTML encoding.
         * The default set is #%&():[]\\`{}
         */
        static std::string unsafe_chars;

    private:
        void trimspace(std::string& s) const;
        void html_encode(std::ostream& os, const char* start) const;
        void process(
            bool visible,
            const std::string& buf,
            const char*& lastpos,
            std::ostream& os,
            const TemplateParameters& parameters,
            const std::pair<const std::string,std::string>& loopentry,
            const XMLToolingException* e
            ) const;

        std::string keytag,iftag,ifendtag,ifnottag,ifnotendtag,fortag,forendtag;
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_template_h__ */
