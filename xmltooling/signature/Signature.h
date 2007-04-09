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
 * @file Signature.h
 * 
 * XMLObject representing XML Digital Signature, version 20020212, Signature element. 
 */

#if !defined(__xmltooling_sig_h__) && !defined(XMLTOOLING_NO_XMLSEC)
#define __xmltooling_sig_h__

#include <xmltooling/exceptions.h>
#include <xmltooling/XMLObjectBuilder.h>
#include <xmltooling/signature/ContentReference.h>
#include <xmltooling/util/XMLConstants.h>

#include <xsec/dsig/DSIGSignature.hpp>

/**
 * @namespace xmlsignature
 * Public namespace of XML Signature classes
 */
namespace xmlsignature {

    class XMLTOOL_API KeyInfo;

    /**
     * XMLObject representing XML Digital Signature, version 20020212, Signature element.
     * The default signature settings include Exclusive c14n w/o comments, SHA-1 digests,
     * and RSA-SHA1 signing. 
     */
    class XMLTOOL_API Signature : public virtual xmltooling::XMLObject
    {
    public:
        virtual ~Signature() {}

        /** Element local name */
        static const XMLCh LOCAL_NAME[];

        /**
         * Gets the canonicalization method for the ds:SignedInfo element.
         * 
         * @return the canonicalization method
         */
        virtual const XMLCh* getCanonicalizationMethod() const=0;
        
        /**
         * Gets the signing algorithm for the signature.
         * 
         * @return    the signature algorithm
         */
        virtual const XMLCh* getSignatureAlgorithm() const=0;

        /**
         * Sets the canonicalization method for the ds:SignedInfo element.
         * 
         * @param c14n  the canonicalization method
         */
        virtual void setCanonicalizationMethod(const XMLCh* c14n)=0;
        
        /**
         * Sets the signing algorithm for the signature.
         * 
         * @param sm    the signature algorithm
         */
        virtual void setSignatureAlgorithm(const XMLCh* sm)=0;

        /**
         * Sets the signing key used to create the signature.
         * 
         * @param signingKey the secret/private key used to create the signature
         */
        virtual void setSigningKey(XSECCryptoKey* signingKey)=0;

        /**
         * Sets a KeyInfo object to embed in the Signature.
         * 
         * @param keyInfo   pointer to a KeyInfo object, or NULL
         */
        virtual void setKeyInfo(KeyInfo* keyInfo)=0;

        /**
         * Gets the KeyInfo object associated with the Signature.
         * This is <strong>NOT</strong> provided for access to the
         * data associated with an unmarshalled signature. It is
         * used only in the creation of signatures. Access to data
         * for validation purposes is provided through the native
         * DSIGSignature object.
         * 
         * @return  pointer to a KeyInfo object, or NULL
         */
        virtual KeyInfo* getKeyInfo() const=0;

        /**
         * Sets the ContentReference object to the Signature to be applied
         * when the signature is created.
         * 
         * @param reference the reference to attach, or NULL 
         */
        virtual void setContentReference(ContentReference* reference)=0;

        /**
         * Gets the ContentReference object associated with the Signature.
         * This is <strong>NOT</strong> provided for access to the
         * data associated with an unmarshalled signature. It is
         * used only in the creation of signatures. Access to data
         * for validation purposes is provided through the native
         * DSIGSignature object.
         * 
         * @return  pointer to a ContentReference object, or NULL
         */
        virtual ContentReference* getContentReference() const=0;

        
        /**
         * Gets the native Apache signature object, if present.
         * 
         * @return  the native Apache signature interface
         */
        virtual DSIGSignature* getXMLSignature() const=0;

        /**
         * Compute and append the signature based on the assigned
         * ContentReference, KeyInfo, and signing key.
         *
         * @param credential    optional source of signing key and KeyInfo
         */
        virtual void sign(const xmltooling::Credential* credential=NULL)=0;

        /**
         * Type-safe clone operation.
         * 
         * @return  copy of object
         */
        virtual Signature* cloneSignature() const=0;

        /**
         * Sign the input data and return a base64-encoded signature. The signature value
         * <strong>MUST NOT</strong> contain any embedded linefeeds.
         * 
         * <p>Allows specialized applications to create raw signatures over any input using
         * the same cryptography layer as XML Signatures use. 
         * 
         * @param key               key to sign with, will <strong>NOT</strong> be freed
         * @param sigAlgorithm      XML signature algorithm identifier
         * @param in                input data
         * @param in_len            size of input data in bytes
         * @param out               output buffer
         * @param out_len           size of output buffer in bytes
         * @return  size in bytes of base64-encoded signature
         */
        static unsigned int createRawSignature(
            XSECCryptoKey* key,
            const XMLCh* sigAlgorithm,
            const char* in,
            unsigned int in_len,
            char* out,
            unsigned int out_len
            );
         
        /**
         * Verifies a base-64 encoded signature over the input data.
         * 
         * <p>Allows specialized applications to verify raw signatures over any input using
         * the same cryptography layer as XML Signatures use. 
         * 
         * @param key               key to verify with, will <strong>NOT</strong> be freed
         * @param sigAlgorithm      XML signature algorithm identifier
         * @param signature         base64-encoded signature value
         * @param in                input data
         * @param in_len            size of input data in bytes
         * @return  true iff signature verifies
         */
        static bool verifyRawSignature(
            XSECCryptoKey* key,
            const XMLCh* sigAlgorithm,
            const char* signature,
            const char* in,
            unsigned int in_len
            );

    protected:
        Signature() {}
    };

    /**
     * Builder for Signature objects.
     */
    class XMLTOOL_API SignatureBuilder : public xmltooling::XMLObjectBuilder
    {
    public:
#ifdef HAVE_COVARIANT_RETURNS
        virtual Signature* buildObject(
#else
        virtual xmltooling::XMLObject* buildObject(
#endif
            const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix=NULL, const xmltooling::QName* schemaType=NULL
            ) const;
            
        /**
         * Default builder
         * 
         * @return empty Signature object
         */
#ifdef HAVE_COVARIANT_RETURNS
        virtual Signature* buildObject() const;
#else
        virtual xmltooling::XMLObject* buildObject() const;
#endif
        /** Singleton builder. */
        static Signature* buildSignature() {
            const SignatureBuilder* b = dynamic_cast<const SignatureBuilder*>(
                xmltooling::XMLObjectBuilder::getBuilder(
                    xmltooling::QName(xmlconstants::XMLSIG_NS,Signature::LOCAL_NAME)
                    )
                );
            if (b) {
#ifdef HAVE_COVARIANT_RETURNS
                return b->buildObject();
#else
                return dynamic_cast<Signature*>(b->buildObject());
#endif
            }
            throw xmltooling::XMLObjectException("Unable to obtain typed builder for Signature.");
        }
    };

    DECL_XMLTOOLING_EXCEPTION(SignatureException,XMLTOOL_EXCEPTIONAPI(XMLTOOL_API),xmlsignature,xmltooling::XMLSecurityException,Exceptions in signature processing);

};

#endif /* __xmltooling_sig_h__ */
