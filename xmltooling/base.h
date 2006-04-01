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

#ifdef _MSC_VER
    #define XMLTOOLING_DOXYGEN(desc) /##** desc */
#else
    #define XMLTOOLING_DOXYGEN(desc)
#endif

/**
 * Blocks copy c'tor and assignment operator for a class.
 */
#define MAKE_NONCOPYABLE(type) \
    private: \
        type(const type&); \
        type& operator=(const type&);

#define UNICODE_LITERAL_1(a) {chLatin_##a, chNull}
#define UNICODE_LITERAL_2(a,b) {chLatin_##a, chLatin_##b, chNull}
#define UNICODE_LITERAL_3(a,b,c) {chLatin_##a, chLatin_##b, chLatin_##c, chNull}
#define UNICODE_LITERAL_4(a,b,c,d) {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chNull}
#define UNICODE_LITERAL_5(a,b,c,d,e) {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chNull}
#define UNICODE_LITERAL_6(a,b,c,d,e,f) {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chNull}
#define UNICODE_LITERAL_7(a,b,c,d,e,f,g) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chNull}
#define UNICODE_LITERAL_8(a,b,c,d,e,f,g,h) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chNull}
#define UNICODE_LITERAL_9(a,b,c,d,e,f,g,h,i) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, chNull}
#define UNICODE_LITERAL_10(a,b,c,d,e,f,g,h,i,j) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chNull}
#define UNICODE_LITERAL_11(a,b,c,d,e,f,g,h,i,j,k) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chNull}
#define UNICODE_LITERAL_12(a,b,c,d,e,f,g,h,i,j,k,l) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chNull}
#define UNICODE_LITERAL_13(a,b,c,d,e,f,g,h,i,j,k,l,m) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chNull}
#define UNICODE_LITERAL_14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chNull}
#define UNICODE_LITERAL_15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chNull}
#define UNICODE_LITERAL_16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chNull}
#define UNICODE_LITERAL_17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chNull}
#define UNICODE_LITERAL_18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, chNull}
#define UNICODE_LITERAL_19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chNull}
#define UNICODE_LITERAL_20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chNull}
#define UNICODE_LITERAL_21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chNull}
#define UNICODE_LITERAL_22(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chLatin_##v, chNull}
#define UNICODE_LITERAL_23(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chLatin_##v, chLatin_##w, chNull}
#define UNICODE_LITERAL_24(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chLatin_##v, chLatin_##w, chLatin_##x, chNull}
#define UNICODE_LITERAL_25(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chLatin_##v, chLatin_##w, chLatin_##x, chLatin_##y, chNull}
#define UNICODE_LITERAL_26(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) \
    {chLatin_##a, chLatin_##b, chLatin_##c, chLatin_##d, chLatin_##e, chLatin_##f, chLatin_##g, chLatin_##h, chLatin_##i, \
        chLatin_##j, chLatin_##k, chLatin_##l, chLatin_##m, chLatin_##n, chLatin_##o, chLatin_##p, chLatin_##q, chLatin_##r, \
        chLatin_##s, chLatin_##t, chLatin_##u, chLatin_##v, chLatin_##w, chLatin_##x, chLatin_##y, chLatin_##z, chNull}

/**
 * Begins the declaration of an XMLObject specialization.
 * Basic boilerplate includes a protected constructor, empty virtual destructor,
 * and Unicode constants for the default associated element's name and prefix.
 * 
 * @param linkage   linkage specifier for the class
 * @param cname     the name of the class to declare
 * @param base      the base class to derive from using public virtual inheritance
 */
#define BEGIN_XMLOBJECT(linkage,cname,base) \
    class linkage cname : public virtual base, public virtual ValidatingXMLObject { \
    protected: \
        cname() {} \
    public: \
        virtual ~cname() {} \
        XMLTOOLING_DOXYGEN(Type-specific clone method.) \
        virtual cname* clone##cname() const=0; \
        XMLTOOLING_DOXYGEN(Element local name) \
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
    public: \
        XMLTOOLING_DOXYGEN(proper attribute name) \
        static const XMLCh upcased##_ATTRIB_NAME[]; \
        XMLTOOLING_DOXYGEN(Returns the proper attribute.) \
        virtual const XMLCh* get##proper() const=0; \
        XMLTOOLING_DOXYGEN(Sets the proper attribute.) \
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
 * Declares abstract get/set methods for a typed XML child object.
 * 
 * @param proper    the proper name of the child type
 */
#define DECL_XMLOBJECT_CHILD(proper) \
    public: \
        XMLTOOLING_DOXYGEN(Returns the proper child.) \
        virtual proper* get##proper() const=0; \
        XMLTOOLING_DOXYGEN(Sets the proper child.) \
        virtual void set##proper(proper* child)=0

/**
 * Implements get/set methods and a private list iterator member for a typed XML child object.
 * 
 * @param proper    the proper name of the child type
 */
#define IMPL_XMLOBJECT_CHILD(proper) \
    private: \
        proper* m_##proper; \
        std::list<XMLObject*>::iterator m_pos_##proper; \
    public: \
        proper* get##proper() const { \
            return m_##proper; \
        } \
        void set##proper(proper* child) { \
            prepareForAssignment(m_##proper,child); \
            *m_pos_##proper = m_##proper = child; \
        }

/**
 * Declares abstract get/set methods for a typed XML child collection.
 * 
 * @param proper    the proper name of the child type
 */
#define DECL_XMLOBJECT_CHILDREN(proper) \
    public: \
        XMLTOOLING_DOXYGEN(Returns modifiable proper collection.) \
        virtual VectorOf(proper) get##proper##s()=0; \
        XMLTOOLING_DOXYGEN(Returns reference to immutable proper collection.) \
        virtual const std::vector<proper*>& get##proper##s() const=0

/**
 * Implements get method and a private vector member for a typed XML child collection.
 * 
 * @param proper    the proper name of the child type
 * @param fence     insertion fence for new objects of the child collection in backing list
 */
#define IMPL_XMLOBJECT_CHILDREN(proper,fence) \
    private: \
        std::vector<proper*> m_##proper##s; \
    public: \
        VectorOf(proper) get##proper##s() { \
            return VectorOf(proper)(this, m_##proper##s, &m_children, fence); \
        } \
        const std::vector<proper*>& get##proper##s() const { \
            return m_##proper##s; \
        } 

/**
 * Implements unmarshalling process branch for typed child collection element
 * 
 * @param proper        the proper name of the child type
 * @param namespaceURI  the XML namespace of the child element
 */
#define PROC_XMLOBJECT_CHILDREN(proper,namespaceURI) \
    if (XMLHelper::isNodeNamed(root,namespaceURI,proper::LOCAL_NAME)) { \
        proper* typesafe=dynamic_cast<proper*>(childXMLObject); \
        if (typesafe) { \
            get##proper##s().push_back(typesafe); \
            return; \
        } \
    }

/**
 * Implements unmarshalling process branch for typed child singleton element
 * 
 * @param proper        the proper name of the child type
 * @param namespaceURI  the XML namespace of the child element
 */
#define PROC_XMLOBJECT_CHILD(proper,namespaceURI) \
    if (XMLHelper::isNodeNamed(root,namespaceURI,proper::LOCAL_NAME)) { \
        proper* typesafe=dynamic_cast<proper*>(childXMLObject); \
        if (typesafe) { \
            set##proper(typesafe); \
            return; \
        } \
    }

/**
 * Declares abstract get/set methods for named XML element content.
 * 
 * @param proper    the proper name to label the element's content
 */
#define DECL_XMLOBJECT_CONTENT(proper) \
    XMLTOOLING_DOXYGEN(Returns proper.) \
    virtual const XMLCh* get##proper() const=0; \
    XMLTOOLING_DOXYGEN(Sets proper.) \
    virtual void set##proper(const XMLCh* proper)=0

/**
 * Implements get/set methods and a private member for named XML element content.
 * 
 * @param proper    the proper name to label the element's content
 */
#define IMPL_XMLOBJECT_CONTENT(proper) \
    private: \
        XMLCh* m_##proper; \
    public: \
        const XMLCh* get##proper() const { \
            return m_##proper; \
        } \
        void set##proper(const XMLCh* proper) { \
            m_##proper = prepareForAssignment(m_##proper,proper); \
        } \
    protected: \
        void marshallElementContent(DOMElement* domElement) const { \
            if(get##proper()) { \
                domElement->appendChild(domElement->getOwnerDocument()->createTextNode(get##proper())); \
            } \
        } \
        void processElementContent(const XMLCh* elementContent) { \
            set##proper(elementContent); \
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
 * Declares and defines an implementation class for an XMLObject with
 * a simple content model and type, handling it as string data.
 * 
 * @param linkage   linkage specifier for the class
 * @param cname     the name of the XMLObject specialization
 * @param proper    the proper name to label the element's content
 */
#define DECL_XMLOBJECTIMPL_SIMPLE(linkage,cname,proper) \
    class linkage cname##Impl \
        : public cname, \
            public AbstractDOMCachingXMLObject, \
            public AbstractValidatingXMLObject, \
            public AbstractXMLObjectMarshaller, \
            public AbstractXMLObjectUnmarshaller \
    { \
    public: \
        virtual ~cname##Impl() {} \
        cname##Impl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType) \
            : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_##proper(NULL) { \
        } \
        cname##Impl(const cname##Impl& src) \
            : AbstractXMLObject(src), \
                AbstractDOMCachingXMLObject(src), \
                AbstractValidatingXMLObject(src), \
                m_##proper(XMLString::replicate(src.m_##proper)) { \
        } \
        IMPL_XMLOBJECT_CLONE(cname) \
        IMPL_XMLOBJECT_CONTENT(proper) \
    }
    
/**
 * Begins the declaration of an XMLObjectBuilder specialization.
 * Basic boilerplate includes an empty virtual destructor, and
 * a default builder that defaults the element name.
 * 
 * @param linkage           linkage specifier for the class
 * @param cname             the name of the XMLObject specialization
 * @param namespaceURI      the XML namespace of the default associated element
 * @param namespacePrefix   the XML namespace prefix of the default associated element
 */
#define BEGIN_XMLOBJECTBUILDER(linkage,cname,namespaceURI,namespacePrefix) \
    XMLTOOLING_DOXYGEN(Builder for cname objects.) \
    class linkage cname##Builder : public xmltooling::XMLObjectBuilder { \
    public: \
        virtual ~cname##Builder() {} \
        XMLTOOLING_DOXYGEN(Default builder.) \
        virtual cname* buildObject() const { \
            return buildObject(namespaceURI,cname::LOCAL_NAME,namespacePrefix); \
        } \
        virtual cname* buildObject( \
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const QName* schemaType=NULL \
            ) const

/**
 * Ends the declaration of an XMLObjectBuilder specialization.
 */
#define END_XMLOBJECTBUILDER }

/**
 * Declares a generic XMLObjectBuilder specialization.
 * 
 * @param linkage           linkage specifier for the class
 * @param cname             the name of the XMLObject specialization
 * @param namespaceURI      the XML namespace of the default associated element
 * @param namespacePrefix   the XML namespace prefix of the default associated element
 */
 #define DECL_XMLOBJECTBUILDER(linkage,cname,namespaceURI,namespacePrefix) \
    BEGIN_XMLOBJECTBUILDER(linkage,cname,namespaceURI,namespacePrefix); \
    END_XMLOBJECTBUILDER

/**
 * Implements the standard XMLObjectBuilder specialization function. 
 * 
 * @param cname the name of the XMLObject specialization
 */
#define IMPL_XMLOBJECTBUILDER(cname) \
    cname* cname##Builder::buildObject( \
        const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType \
        ) const \
    { \
        return new cname##Impl(nsURI,localName,prefix,schemaType); \
    }

/**
 * Begins the declaration of a Schema Validator specialization.
 * 
 * @param linkage           linkage specifier for the class
 * @param cname the base name of the Validator specialization
 */
 #define BEGIN_XMLOBJECTVALIDATOR(linkage,cname) \
    class linkage cname##SchemaValidator : public Validator \
    { \
    public: \
        virtual ~cname##SchemaValidator() {} \
        virtual cname##SchemaValidator* clone() const { \
            return new cname##SchemaValidator(); \
        } \
        virtual void validate(const XMLObject* xmlObject) const { \
            const cname* ptr=dynamic_cast<const cname*>(xmlObject); \
            if (!ptr) \
                throw ValidationException(#cname"SchemaValidator: unsupported object type ($1).",xmltooling::params(1,typeid(xmlObject).name()))

/**
 * Ends the declaration of a Validator specialization.
 */
#define END_XMLOBJECTVALIDATOR } }

/**
 * Validator code that checks the object type.
 * 
 * @param cname     the name of the XMLObject specialization
 */
#define XMLOBJECTVALIDATOR_CHECKTYPE(cname) \
    const cname* ptr=dynamic_cast<const cname*>(xmlObject); \
    if (!ptr) \
        throw ValidationException(#cname"SchemaValidator: unsupported object type ($1).",xmltooling::params(1,typeid(xmlObject).name()))

/**
 * Validator code that checks for a required attribute, content, or singleton.
 * 
 * @param cname     the name of the XMLObject specialization
 * @param proper    the proper name of the attribute, content, or singleton member 
 */
#define XMLOBJECTVALIDATOR_REQUIRE(cname,proper) \
    if (!ptr->get##proper()) \
        throw ValidationException(#cname" must have "#proper".")

/**
 * Validator code that checks for a non-empty collection.
 * 
 * @param cname     the name of the XMLObject specialization
 * @param proper    the proper name of the collection item 
 */
#define XMLOBJECTVALIDATOR_CHECKEMPTY(cname,proper) \
    if (ptr->get##proper##s().empty()) \
        throw ValidationException(#cname" must have at least one "#proper".")

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
