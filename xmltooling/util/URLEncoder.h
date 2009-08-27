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
 * @file xmltooling/util/URLEncoder.h
 *
 * Interface to a URL-encoding mechanism along with a
 * default implementation.
 */

#ifndef __xmltool_urlenc_h__
#define __xmltool_urlenc_h__

#include <xmltooling/base.h>

namespace xmltooling {
    /**
     * Interface to a URL-encoding mechanism along with a default implementation.
     *
     * Since URL-encoding is not canonical, it's important that the same
     * encoder is used during some library operations and the calling code.
     * Applications can supply an alternative implementation to the library
     * if required.
     */
    class XMLTOOL_API URLEncoder {
        MAKE_NONCOPYABLE(URLEncoder);
    public:
        URLEncoder() {}
        virtual ~URLEncoder() {}

        /**
         * Produce a URL-safe but equivalent version of the input string.
         *
         * @param s input string to encode
         * @return a string object containing the result of encoding the input
         */
        virtual std::string encode(const char* s) const;

        /**
         * Perform an in-place decoding operation on the input string.
         * The resulting string will be NULL-terminated.
         *
         * @param s input string to decode in a writable buffer
         */
        virtual void decode(char* s) const;

    protected:
        /**
         * Returns true iff the input character requires encoding.
         *
         * @param ch    the character to check
         * @return  true iff the character should be encoded
         */
        virtual bool isBad(char ch) const {
            static char badchars[]="=&/?:\"\\+<>#%{}|^~[],`;@";
            return (ch<=0x20 || ch>=0x7F || strchr(badchars,ch));
        }
    };
};

#endif /* __xmltool_urlenc_h__ */
