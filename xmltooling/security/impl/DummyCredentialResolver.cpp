/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * DummyCredentialResolver.cpp
 * 
 * CredentialResolver that returns nothing.
 */

#include "internal.h"
#include "XMLToolingConfig.h"
#include "security/CredentialResolver.h"
#include "util/NDC.h"

using namespace xmltooling;
using namespace std;

using xercesc::DOMElement;

namespace xmltooling {

    class XMLTOOL_DLLLOCAL DummyCredentialResolver : public CredentialResolver
    {
    public:
        DummyCredentialResolver(const DOMElement*) {}
        virtual ~DummyCredentialResolver() {}

        Lockable* lock() {return this;}
        void unlock() {}

        const Credential* resolve(const CredentialCriteria* criteria=nullptr) const {
            return nullptr;
        }
        vector<const Credential*>::size_type resolve(
            vector<const Credential*>& results, const CredentialCriteria* criteria=nullptr
            ) const {
            return 0;
        }
    };

    CredentialResolver* XMLTOOL_DLLLOCAL DummyCredentialResolverFactory(const DOMElement* const & e)
    {
        return new DummyCredentialResolver(e);
    }

};
