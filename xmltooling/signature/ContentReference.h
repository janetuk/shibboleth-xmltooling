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
 * @file ContentReference.h
 * 
 * Interface for creating signature references 
 */

#if !defined(__xmltooling_sigref_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sigref_h__

#include <vector>
#include <xsec/dsig/DSIGSignature.hpp>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmlsignature {

    /**
     * Interface for creating signature references based on application requirements.
     */
    class XMLTOOL_API ContentReference
    {
        MAKE_NONCOPYABLE(ContentReference);
    public:
        virtual ~ContentReference() {}

        /**
         * Given a native signature, asks the object to create the reference(s).
         * 
         * @param sig   native signature interface
         */
        virtual void createReferences(DSIGSignature* sig)=0;
        
    protected:
        ContentReference() {}
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_sigref_h__ */
