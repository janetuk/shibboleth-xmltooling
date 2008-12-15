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
 * unicode.cpp
 * 
 * Helper classes and types for manipulating Unicode 
 */

#include "internal.h"
#include "unicode.h"

#include <xercesc/util/XMLUTF8Transcoder.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xercesc;

static const XMLCh UTF8[]={ chLatin_U, chLatin_T, chLatin_F, chDigit_8, chNull };

char* xmltooling::toUTF8(const XMLCh* src, bool use_malloc)
{
    xsecsize_t eaten,factor=1,bufsize;
    xsecsize_t srclen=XMLString::stringLen(src);
    XMLUTF8Transcoder t(UTF8, 4096);    // block size isn't used any more anyway
    do {
        bufsize = factor*srclen + 10;
        char* buf = use_malloc ? reinterpret_cast<char*>(malloc(bufsize)) : new char[bufsize];
        memset(buf,0,bufsize);
        try {
            t.transcodeTo(
                src,srclen,
                reinterpret_cast<XMLByte*>(buf),bufsize-1,
                eaten,
                XMLTranscoder::UnRep_Throw);
        }
        catch (XMLException&) {
            if (use_malloc)
                free(buf);
            else
                delete[] buf;
            throw XMLToolingException("Source string contained an unrepresentable character.");
        }
        if (eaten >= srclen)
            return buf;
        if (use_malloc)
            free(buf);
        else
            delete[] buf;
        factor++;
    } while (1);
}

XMLCh* xmltooling::fromUTF8(const char* src, bool use_malloc)
{
    xsecsize_t eaten;
    xsecsize_t srclen=strlen(src);
    XMLUTF8Transcoder t(UTF8, 4096);    // block size isn't used any more anyway
    XMLCh* buf = use_malloc ? reinterpret_cast<XMLCh*>(malloc((srclen+1)*sizeof(XMLCh))) : new XMLCh[srclen + 1];
    unsigned char* sizes=new unsigned char[srclen];
    memset(buf,0,(srclen+1)*sizeof(XMLCh));
    t.transcodeFrom(
        reinterpret_cast<const XMLByte*>(src),srclen,
        buf,srclen,
        eaten,sizes);
    delete[] sizes;
    return buf;
}

std::ostream& xmltooling::operator<<(std::ostream& ostr, const XMLCh* s)
{
    if (s) {
        char* p=xmltooling::toUTF8(s);
        ostr << p;
        delete[] p;
    }
    return ostr;
}
