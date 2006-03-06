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
 * @file base.h
 * 
 * Base header file definitions
 * Must be included prior to including any other header
 */

#if !defined(__xmltooling_base_h__)
#define __xmltooling_base_h__

#if defined (_MSC_VER) || defined(__BORLANDC__)
  #include <xmltooling/config_pub_win32.h>
#else
  #include <xmltooling/config_pub.h>
#endif

/**
 * @namespace xmltooling
 * Public namespace of XML Tooling library
 */

// Windows and GCC4 Symbol Visibility Macros

#ifdef WIN32
  #define XMLTOOL_IMPORT __declspec(dllimport)
  #define XMLTOOL_EXPORT __declspec(dllexport)
  #define XMLTOOL_DLLLOCAL
  #define XMLTOOL_DLLPUBLIC
#else
  #define XMLTOOL_IMPORT
  #ifdef GCC_HASCLASSVISIBILITY
    #define XMLTOOL_EXPORT __attribute__ ((visibility("default")))
    #define XMLTOOL_DLLLOCAL __attribute__ ((visibility("hidden")))
    #define XMLTOOL_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define XMLTOOL_EXPORT
    #define XMLTOOL_DLLLOCAL
    #define XMLTOOL_DLLPUBLIC
  #endif
#endif

// Define XMLTOOL_API for DLL builds
#ifdef XMLTOOLING_EXPORTS
  #define XMLTOOL_API XMLTOOL_EXPORT
#else
  #define XMLTOOL_API XMLTOOL_IMPORT
#endif

// Throwable classes must always be visible on GCC in all binaries
#ifdef WIN32
  #define XMLTOOL_EXCEPTIONAPI(api) api
#elif defined(GCC_HASCLASSVISIBILITY)
  #define XMLTOOL_EXCEPTIONAPI(api) XMLTOOL_EXPORT
#else
  #define XMLTOOL_EXCEPTIONAPI(api)
#endif

// Macro to block copy c'tor and assignment operator for a class
#define MAKE_NONCOPYABLE(type) \
    private: \
        type(const type&); \
        type& operator=(const type&);

#ifndef NULL
#define NULL    0
#endif

#include <utility>

namespace xmltooling {

    /**
     * Template function for cloning a sequence of XMLObjects.
     * Invokes the clone() member on each element of the input sequence and adds the copy to
     * the output sequence. Order is preserved.
     * 
     * @param in    input sequence to clone
     * @param out   output sequence to copy cloned pointers into
     */
    template<class InputSequence,class OutputSequence> void clone(const InputSequence& in, OutputSequence& out) {
        for (typename InputSequence::const_iterator i=in.begin(); i!=in.end(); i++) {
            if (*i)
                out.push_back((*i)->clone());
            else
                out.push_back(*i);
        }
    }

    /*
     * Functor for cleaning up heap objects in containers.
     */
    template<class T> struct cleanup
    {
        /**
         * Function operator to delete an object.
         * 
         * @param ptr   object to delete
         */
        void operator()(T* ptr) {delete ptr;}
        
        /**
         * Function operator to delete an object stored as const.
         * 
         * @param ptr   object to delete after casting away const
         */
        void operator()(const T* ptr) {delete const_cast<T*>(ptr);}
    };

    /*
     * Functor for cleaning up heap objects in key/value containers.
     */
    template<class A,class B> struct cleanup_pair
    {
        /**
         * Function operator to delete an object.
         * 
         * @param p   a pair in which the second component is the object to delete
         */
        void operator()(const std::pair<A,B*>& p) {delete p.second;}
    };
};

#endif /* __xmltooling_base_h__ */
