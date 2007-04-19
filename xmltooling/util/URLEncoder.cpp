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
 * URLEncoder.cpp
 * 
 * Interface to a URL-encoding mechanism along with a
 * default implementation. 
 */

#include "internal.h"
#include "util/URLEncoder.h"

using namespace xmltooling;
using namespace std;

static char x2c(char *what)
{
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void URLEncoder::decode(char* s) const
{
    register int x,y;

    for(x=0,y=0;s[y];++x,++y)
    {
        if((s[x] = s[y]) == '%')
        {
            s[x] = x2c(&s[y+1]);
            y+=2;
        }
        else if (s[x] == '+')
        {
            s[x] = ' ';
        }
    }
    s[x] = '\0';
}

static inline char hexchar(unsigned short s)
{
    return (s<=9) ? ('0' + s) : ('A' + s - 10);
}

string URLEncoder::encode(const char* s) const
{
    string ret;
    for (; *s; s++) {
        if (isBad(*s)) {
            ret+='%';
            ret+=hexchar((unsigned char)*s >> 4);
            ret+=hexchar((unsigned char)*s & 0x0F);
        }
        else
            ret+=*s;
    }
    return ret;
}