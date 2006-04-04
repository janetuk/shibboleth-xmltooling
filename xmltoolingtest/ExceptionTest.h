/*
 *  Copyright 2001-2005 Internet2
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

#include "XMLObjectBaseTestCase.h"

#include <xmltooling/exceptions.h>


class ExceptionTest : public CxxTest::TestSuite {
public:

    void testException(void) {
        TS_TRACE("testException");

#define TEST(n,b,a) XMLToolingException e##n(b); \
                TS_ASSERT(!strcmp(a,e##n.what()))

#define TESTP(n,b,a,p) MarshallingException e##n(b,p); \
                TS_ASSERT(!strcmp(a,e##n.what()))


        TESTP(1,"This is a test.",          "This is a test.",      params(2,"Foo","bar"));
        TESTP(2,"This is a test.$",         "This is a test.",      params(2,"Foo","bar"));
        TESTP(3,"This is a $ test.",        "This is a  test.",     params(2,"Foo","bar"));
        TESTP(4,"$$This is a test.$",       "$This is a test.",     params(2,"Foo","bar"));
        TESTP(5,"$This is a $test.",        "This is a test.",      params(2,"Foo","bar"));
        TESTP(6,"$1 is a $2",               "Foo is a bar",         params(2,"Foo","bar"));
        TESTP(7,"$This is a $test.",        "Foo is a bar.",        namedparams(2,"This","Foo","test","bar"));
        TESTP(8,"Unable to generate random data: $1",
                "Unable to generate random data: OpenSSLCryptoProvider::getRandom - OpenSSL random not properly initialised",
                params(1,"OpenSSLCryptoProvider::getRandom - OpenSSL random not properly initialised"));

        string buf=e7.toString();
        auto_ptr<XMLToolingException> ptr(XMLToolingException::fromString(buf.c_str()));
        TS_ASSERT(typeid(*ptr)==typeid(MarshallingException));
        TS_ASSERT(!strcmp(ptr->what(),"Foo is a bar."));
    }
};
