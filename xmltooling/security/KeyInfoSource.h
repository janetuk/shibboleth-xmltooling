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
 * @file xmltooling/security/KeyInfoSource.h
 * 
 * Interface for objects that can supply KeyInfo objects to a TrustEngine
 * via the KeyInfoIterator interface.
 */

#if !defined(__xmltooling_keysource_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_keysource_h__

#include <xmltooling/base.h>
#include <string>

namespace xmlsignature {
    class XMLTOOL_API KeyInfo;
};

namespace xmltooling {

    /**
     * Callback interface to supply KeyInfo objects to a TrustEngine.
     * Applications can adapt TrustEngines to their environment by supplying
     * implementations of this interface. 
     */
    class XMLTOOL_API KeyInfoIterator {
        MAKE_NONCOPYABLE(KeyInfoIterator);
    protected:
        KeyInfoIterator() {}
    public:
        virtual ~KeyInfoIterator() {}
        
        /**
         * Indicates whether additional KeyInfo objects are available.
         * 
         * @return true iff another KeyInfo object can be fetched
         */
        virtual bool hasNext() const=0;
        
        /**
         * Returns the next KeyInfo object available.
         * 
         * @return the next KeyInfo object, or NULL if none are left
         */
        virtual const xmlsignature::KeyInfo* next()=0;
    };

    /**
     * Interface for objects that can supply KeyInfo objects to a TrustEngine
     * via the KeyInfoIterator interface.
     */
    class XMLTOOL_API KeyInfoSource {
    protected:
        KeyInfoSource() {}
    public:
        virtual ~KeyInfoSource() {}

        /**
         * Returns the name of this source of keys, for example a peer entity name
         * or a principal's name.
         * 
         * @return  name of key source, or empty string
         */
        virtual std::string getName() const=0;
        
        /**
         * Provides access to the KeyInfo information associated with the source.
         * The caller must free the returned interface when finished with it.
         * 
         * @return interface for obtaining KeyInfo data  
         */
        virtual KeyInfoIterator* getKeyInfoIterator() const=0;
    };

};

#endif /* __xmltooling_keysource_h__ */
