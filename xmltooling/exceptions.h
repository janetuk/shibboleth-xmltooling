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
 * @file exception.h
 * 
 * Exception classes
 */
 
#if !defined(__xmltooling_exceptions_h__)
#define __xmltooling_exceptions_h__

#include <string>
#include <xmltooling/base.h>

#define DECL_XMLTOOLING_EXCEPTION(type) \
    class XMLTOOL_EXCEPTIONAPI(XMLTOOL_API) type : public XMLToolingException { \
    public: \
        type(const char* msg) : XMLToolingException(msg) {} \
        type(std::string& msg) : XMLToolingException(msg) {} \
        virtual ~type() {} \
    }

namespace xmltooling {
    
    /**
     * Base exception class.
     * std::exception seems to be inconsistently defined, so this is just
     * a substitute base class.
     */
    class XMLTOOL_EXCEPTIONAPI(XMLTOOL_API) XMLToolingException
    {
    public:
        XMLToolingException() {}
        virtual ~XMLToolingException() {}
        XMLToolingException(const char* const msg) : m_msg(msg) {}
        XMLToolingException(const std::string& msg) : m_msg(msg) {}
        virtual const char* what() const { return m_msg.c_str(); }
    private:
        std::string m_msg;
    };

    DECL_XMLTOOLING_EXCEPTION(XMLParserException);

};

#endif /* __xmltooling_exceptions_h__ */
