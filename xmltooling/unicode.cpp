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
 * unicode.cpp
 * 
 * Helper classes and types for manipulating Unicode 
 */

#include "internal.h"
#include "unicode.h"

#include <xercesc/util/XMLUTF8Transcoder.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

static XMLCh UTF8[]={ chLatin_U, chLatin_T, chLatin_F, chDigit_8, chNull };

char* xmltooling::toUTF8(const XMLCh* src)
{
    unsigned int eaten;
    unsigned int srclen=XMLString::stringLen(src);
    XMLUTF8Transcoder t(UTF8, srclen*4 + 1);
    char* buf=new char[srclen*4 + 1];
    memset(buf,0,srclen*4 + 1);
    t.transcodeTo(
        src,srclen,
        reinterpret_cast<XMLByte*>(buf),srclen*4,
        eaten,XMLTranscoder::UnRep_RepChar);
    return buf;
}

XMLCh* xmltooling::fromUTF8(const char* src)
{
    unsigned int eaten;
    unsigned int srclen=strlen(src);
    XMLUTF8Transcoder t(UTF8, srclen + 1);
    XMLCh* buf=new XMLCh[srclen + 1];
    unsigned char* sizes=new unsigned char[srclen];
    memset(buf,0,(srclen+1)*sizeof(XMLCh));
    t.transcodeFrom(
        reinterpret_cast<const XMLByte*>(src),srclen,
        buf,srclen,
        eaten,sizes);
    delete[] sizes;
    return buf;
}
