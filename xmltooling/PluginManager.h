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
 * @file xmltooling/PluginManager.h
 * 
 * Plugin management template
 */

#ifndef __xmltooling_plugin_h__
#define __xmltooling_plugin_h__

#include <xmltooling/base.h>
#include <xmltooling/exceptions.h>

#include <map>
#include <string>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {

    /**
     * Template for management/access to plugins constructed based on a Key type
     * and arbitrary parameters.
     * 
     * @param T         class of plugin to manage
     * @param Key       the key for type lookup
     * @param Params    parameters for plugin construction
     */
    template <class T, class Key, typename Params> class PluginManager
    {
    public:
        PluginManager() {}
        ~PluginManager() {}

        /** Factory function for plugin. */
        typedef T* Factory(const Params&);

        /**
         * Registers the factory for a given type.
         * 
         * @param type      the key to the plugin type
         * @param factory   the factory function for the plugin type
         */
        void registerFactory(const Key& type, typename PluginManager::Factory* factory) {
            if (factory)
                m_map[type]=factory;
        }

        /**
         * Unregisters the factory for a given type.
         * 
         * @param type  the key to the plugin type
         */
        void deregisterFactory(const Key& type) {
            m_map.erase(type);
        }

        /**
         * Unregisters all registered factories.
         */
        void deregisterFactories() {
            m_map.clear();
        }

        /**
         * Builds a new instance of a plugin of a given type, configuring it
         * with the supplied parameters.
         * 
         * @param type  the key to the plugin type
         * @param p     parameters to configure plugin
         * @return      the constructed plugin  
         */
        T* newPlugin(const Key& type, const Params& p) {
            typename std::map<Key, typename PluginManager::Factory*>::const_iterator i=m_map.find(type);
            if (i==m_map.end())
                throw UnknownExtensionException("Unknown plugin type.");
            return i->second(p);
        }
        
    private:
        std::map<Key, typename PluginManager::Factory*> m_map;
    };

};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_plugin_h__ */
