/*
 *  Copyright 2001-2010 Internet2
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
 * @file xmltooling/exceptions.h
 * 
 * Exception classes.
 */
 
#ifndef __xmltooling_exceptions_h__
#define __xmltooling_exceptions_h__

#include <xmltooling/base.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

/**
 * Declares a derived exception class
 * 
 * @param name      the exception class
 * @param linkage   linkage specification for class
 * @param ns        the exception class C++ namespace
 * @param base      the base class
 * @param desc      documentation comment for class
 */
#define DECL_XMLTOOLING_EXCEPTION(name,linkage,ns,base,desc) \
    XMLTOOLING_DOXYGEN(desc) \
    class linkage name : public base { \
    public: \
        XMLTOOLING_DOXYGEN(Constructor) \
        name(const char* msg=nullptr, const xmltooling::params& p=xmltooling::params()) : base(msg,p) {} \
        XMLTOOLING_DOXYGEN(Constructor) \
        name(const char* msg, const xmltooling::namedparams& p) : base(msg,p) {} \
        XMLTOOLING_DOXYGEN(Constructor) \
        name(const std::string& msg, const xmltooling::params& p=xmltooling::params()) : base(msg,p) {} \
        XMLTOOLING_DOXYGEN(Constructor) \
        name(const std::string& msg, const xmltooling::namedparams& p) : base(msg,p) {} \
        virtual ~name() throw () {} \
        virtual const char* getClassName() const { return #ns"::"#name; } \
        void raise() const {throw *this;} \
    }

/**
 * Declares a factory function for an exception class.
 * 
 * @param name  the exception class name
 * @param ns    the exception class C++ namespace
 */
#define DECL_XMLTOOLING_EXCEPTION_FACTORY(name,ns) \
    xmltooling::XMLToolingException* name##Factory() \
    { \
        return new ns::name(); \
    }

/**
 * Registers a factory for an exception class.
 * 
 * @param name      the exception class name
 * @param ns        the exception class C++ namespace
 */
#define REGISTER_XMLTOOLING_EXCEPTION_FACTORY(name,ns) XMLToolingException::registerFactory(#ns"::"#name,name##Factory)

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace xmltooling {
    
    /**
     * Wrapper around a variable number of arguments.
     */
    class XMLTOOL_API params
    {
    public:
        /**
         * Initializes with zero parameters.
         */
        params() {}
        
        /**
         * Initializes the parameter set.
         * 
         * @param count     the number of parameters that follow
         */
        params(int count,...);
        
        /**
         * Returns an immutable reference to the set of parameters.
         * 
         * @return the parameter set
         */
        const std::vector<const char*>& get() const {return v;}
        
    protected:
        /** Contains the parameters being passed. */
        std::vector<const char*> v;
    };
    
    /**
     * Wrapper around a variable number of name/value pairs.
     */
    class XMLTOOL_API namedparams : public params
    {
    public:
        /**
         * Initializes with zero parameters.
         */
        namedparams() {}

        /**
         * Initializes the named parameter set.
         * 
         * @param count     the number of name/value pairs that follow (must be even)
         */
        namedparams(int count,...);
    };

    /**
     * Base exception class, supports parametrized messages and XML serialization.
     * Parameters are prefixed with a dollar sign ($) and can be positional ($1)
     * or named ($info).
     */
    class XMLTOOL_EXCEPTIONAPI(XMLTOOL_API) XMLToolingException : public std::exception
    {
    public:
        virtual ~XMLToolingException() throw () {}

        /**
         * Constructs an exception using a message and positional parameters.
         * 
         * @param msg   error message
         * @param p     an ordered set of positional parameter strings
         */
        XMLToolingException(const char* msg=nullptr, const params& p=params());

        /**
         * Constructs an exception using a message and named parameters.
         * 
         * @param msg   error message
         * @param p     a set of named parameter strings
         */
        XMLToolingException(const char* msg, const namedparams& p);

        /**
         * Constructs an exception using a message and positional parameters.
         * 
         * @param msg   error message
         * @param p     an ordered set of positional parameter strings
         */
        XMLToolingException(const std::string& msg, const params& p=params());

        /**
         * Constructs an exception using a message and named parameters.
         * 
         * @param msg   error message
         * @param p     a set of named parameter strings
         */
        XMLToolingException(const std::string& msg, const namedparams& p);

        /**
         * Returns the error message, after processing any parameter references.
         * 
         * @return  the processed message
         */
        const char* getMessage() const;

        /**
         * Returns the error message, after processing any parameter references.
         * 
         * @return  the processed message
         */
        const char* what() const throw () {return getMessage();}

        /**
         * Sets the error message.
         * 
         * @param msg   the error message
         */
        void setMessage(const char* msg);

        /**
         * Sets the error message.
         * 
         * @param msg   the error message
         */
        void setMessage(const std::string& msg) {
            setMessage(msg.c_str());
        }

        /**
         * Attach a set of positional parameters to the exception.
         * 
         * @param p     an ordered set of named parameter strings
         */
        void addProperties(const params& p);
        
        /**
         * Attach a set of named parameters to the exception.
         * 
         * @param p     a set of named parameter strings
         */
        void addProperties(const namedparams& p);

        /**
         * Attach a single positional parameter at the next available position.
         * 
         * @param value the parameter value
         */
        void addProperty(const char* value) {
            addProperties(params(1,value));
        }

        /**
         * Attach a single named parameter.
         * 
         * @param name  the parameter name
         * @param value the parameter value
         */
        void addProperty(const char* name, const char* value) {
            addProperties(namedparams(1,name,value));
        }

        /**
         * Returns the parameter property with the designated position (based from one).
         * 
         * @param index     position to access
         * @return  the parameter property or nullptr
         */
        const char* getProperty(unsigned int index) const;

        /**
         * Returns the parameter property with the designated name.
         * 
         * @param name     named parameter to access
         * @return  the parameter property or nullptr
         */
        const char* getProperty(const char* name) const;

        /**
         * Raises an exception using itself.
         * Used to raise an exception of a derived type.
         */
        virtual void raise() const {
            throw *this;
        }

        /**
         * Returns a unique name for the exception class.
         * 
         * @return class name
         */
        virtual const char* getClassName() const {
            return "xmltooling::XMLToolingException";
        }
        
        /**
         * Returns a string containing a serialized representation of the exception.
         * 
         * @return  the serialization
         */
        std::string toString() const;

        /**
         * Returns a set of query string name/value pairs, URL-encoded, representing the
         * exception's type, message, and parameters.
         *
         * @return  the query string representation
         */
        std::string toQueryString() const;

    private:
        std::string m_msg;
        mutable std::string m_processedmsg;
        std::map<std::string,std::string> m_params;

    public:
        /**
         * Builds an empty exception of the given type.
         * 
         * @param exceptionClass    the name of the exception type to build
         * @return an empty exception object
         */
        static XMLToolingException* getInstance(const char* exceptionClass);

        /**
         * Builds an exception from a serialized input stream.
         * 
         * @param in    input stream
         * @return the exception object found in the stream
         */
        static XMLToolingException* fromStream(std::istream& in);
        
        /**
         * Builds an exception from a serialized input buffer.
         * 
         * @param s   input buffer
         * @return the exception object found in the buffer
         */
        static XMLToolingException* fromString(const char* s);
                
        /** A factory function that returns an empty exception object of a given type. */
        typedef XMLToolingException* ExceptionFactory();
        
        /**
         * Registers a factory to create exceptions of a given class name.
         * 
         * @param exceptionClass    name of exception type
         * @param factory           factory function to build exceptions with
         */
        static void registerFactory(const char* exceptionClass, ExceptionFactory* factory) {
            m_factoryMap[exceptionClass] = factory;
        }
        
        /**
         * Unregisters the factory for a given class name.
         * 
         * @param exceptionClass    name of exception type
         */
        static void deregisterFactory(const char* exceptionClass) {
            m_factoryMap.erase(exceptionClass);
        }

        /**
         * Unregisters all factories.
         */
        static void deregisterFactories() {
            m_factoryMap.clear();
        }

    private:
        typedef std::map<std::string,ExceptionFactory*> ExceptionFactoryMap;
        static ExceptionFactoryMap m_factoryMap;
    };

    DECL_XMLTOOLING_EXCEPTION(XMLParserException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions related to XML parsing);
    DECL_XMLTOOLING_EXCEPTION(XMLObjectException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions in basic object usage);
    DECL_XMLTOOLING_EXCEPTION(MarshallingException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions during object marshalling);
    DECL_XMLTOOLING_EXCEPTION(UnmarshallingException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions during object unmarshalling);
    DECL_XMLTOOLING_EXCEPTION(UnknownElementException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions due to processing of unknown element content);
    DECL_XMLTOOLING_EXCEPTION(UnknownAttributeException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions due to processing of unknown attributes);
    DECL_XMLTOOLING_EXCEPTION(UnknownExtensionException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions from use of an unrecognized extension/plugin);
    DECL_XMLTOOLING_EXCEPTION(ValidationException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions during object validation);
    DECL_XMLTOOLING_EXCEPTION(IOException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions related to physical input/output errors);

#ifndef XMLTOOLING_NO_XMLSEC
    DECL_XMLTOOLING_EXCEPTION(XMLSecurityException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmltooling,XMLToolingException,Exceptions related to the XML security layer);
#endif
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

#endif /* __xmltooling_exceptions_h__ */
