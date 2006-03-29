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

#ifndef __xmltooling_base_h__
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

#ifndef NULL
#define NULL    0
#endif

/**
 * Blocks copy c'tor and assignment operator for a class.
 */
#define MAKE_NONCOPYABLE(type) \
    private: \
        type(const type&); \
        type& operator=(const type&);

/**
 * Begins the declaration of an XMLObject specialization.
 * Basic boilerplate includes a protected constructor, empty virtual destructor,
 * and Unicode constants for the default associated element's name and prefix.
 * 
 * @param cname the name of the class to declare
 * @param base  the base class to derive from using public virtual inheritance
 */
#define BEGIN_XMLOBJECT(cname,base) \
    class XMLTOOL_API cname : public virtual base, public virtual ValidatingXMLObject { \
    protected: \
        cname() {} \
    public: \
        virtual ~cname() {} \
        /##** Type-specific clone method. */ \
        virtual cname* clone##cname() const=0; \
        /##** Element prefix */ \
        static const XMLCh PREFIX[]; \
        /##** Element local name */ \
        static const XMLCh LOCAL_NAME[]

/**
 * Ends the declaration of an XMLObject specialization.
 */
#define END_XMLOBJECT }

/**
 * Declares abstract get/set methods for a named XML attribute.
 * 
 * @param proper    the proper name of the attribute
 * @param upcased   the upcased name of the attribute
 */
#define DECL_XMLOBJECT_ATTRIB(proper,upcased) \
    /##** proper attribute name */ \
    static const XMLCh upcased##_ATTRIB_NAME[]; \
    /##** Returns the proper attribute. */ \
    virtual const XMLCh* get##proper() const=0; \
    /##** Sets the proper attribute. */ \
    virtual void set##proper(const XMLCh* proper)=0

/**
 * Implements get/set methods and a private member for a named XML attribute.
 * 
 * @param proper    the proper name of the attribute
 */
#define IMPL_XMLOBJECT_ATTRIB(proper) \
    private: \
        XMLCh* m_##proper; \
    public: \
        const XMLCh* get##proper() const { \
            return m_##proper; \
        } \
        void set##proper(const XMLCh* proper) { \
            m_##proper = prepareForAssignment(m_##proper,proper); \
        }

/**
 * Implements cloning methods for an XMLObject specialization implementation class.
 * 
 * @param cname    the name of the XMLObject specialization
 */
#define IMPL_XMLOBJECT_CLONE(cname) \
    cname* clone##cname() const { \
        return clone(); \
    } \
    cname* clone() const { \
        auto_ptr<XMLObject> domClone(AbstractDOMCachingXMLObject::clone()); \
        cname##Impl* ret=dynamic_cast<cname##Impl*>(domClone.get()); \
        if (ret) { \
            domClone.release(); \
            return ret; \
        } \
        return new cname##Impl(*this); \
    }

/**
 * Begins the declaration of an XMLObjectBuilder specialization.
 * Basic boilerplate includes an empty virtual destructor, and
 * a default builder.
 * 
 * @param cname the name of the XMLObject specialization
 */
#define BEGIN_XMLOBJECTBUILDER(cname) \
    /##** Builder for cname objects. */ \
    class XMLTOOL_API cname##Builder : public xmltooling::XMLObjectBuilder { \
    public: \
        virtual ~cname##Builder() {} \
        /##** Default builder. */ \
        virtual cname* buildObject() const=0

/**
 * Ends the declaration of an XMLObjectBuilder specialization.
 */
#define END_XMLOBJECTBUILDER }

/**
 * Begins the declaration of an XMLObjectBuilder specialization implementation class.
 * 
 * @param cname         the name of the XMLObject specialization
 * @param namespaceURI  the XML namespace of the default associated element
 */
#define BEGIN_XMLOBJECTBUILDERIMPL(cname,namespaceURI) \
    class XMLTOOL_DLLLOCAL cname##BuilderImpl : public cname##Builder { \
    public: \
        cname* buildObject( \
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL\
            ) const; \
        cname* buildObject() const { \
            return buildObject(namespaceURI,cname::LOCAL_NAME,cname::PREFIX); \
        }

/**
 * Ends the declaration of an XMLObjectBuilder specialization implementation class.
 */
#define END_XMLOBJECTBUILDERIMPL }

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

    /**
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

    /**
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
