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
 * SOAPClient.cpp
 * 
 * Implements SOAP 1.1 messaging over a transport.
 */

#include "internal.h"
#include "exceptions.h"
#include "logging.h"
#include "soap/SOAP.h"
#include "soap/SOAPClient.h"
#include "util/XMLHelper.h"
#include "validation/ValidatorSuite.h"

#include <sstream>

using namespace soap11;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace xercesc;
using namespace std;

void SOAPTransport::send(istream* in)
{
    if (!in)
        throw IOException("SOAP transport does not support an empty request body.");
    return send(*in);
}

SOAPClient::~SOAPClient()
{
    delete m_transport;
}

void SOAPClient::reset()
{
    delete m_transport;
    m_transport=NULL;
}

void SOAPClient::send(const Envelope& env, const SOAPTransport::Address& addr)
{
    // Prepare a transport object.
    const char* pch = addr.m_endpoint ? strchr(addr.m_endpoint,':') : NULL;
    if (!pch)
        throw IOException("SOAP endpoint was not a URL.");
    string scheme(addr.m_endpoint, pch-addr.m_endpoint);
    m_transport = XMLToolingConfig::getConfig().SOAPTransportManager.newPlugin(scheme.c_str(), addr);
    prepareTransport(*m_transport);
    
    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".SOAPClient");
    if (log.isDebugEnabled())
        log.debugStream() << "marshalled envelope:\n" << env << logging::eol;
    
    // Serialize envelope.
    stringstream s;
    s << env;
    
    // Send to peer.
    m_transport->send(s);
}

Envelope* SOAPClient::receive()
{
    if (!m_transport)
        throw IOException("No call is active.");
    
    // If we can get the stream, then the call is still active.
    istream& out = m_transport->receive();
    if (!out)
        return NULL;    // nothing yet
    
    // Check content type.
    string s = m_transport->getContentType();
    if (s.find("text/xml") == string::npos)
        throw IOException("Incorrect content type ($1) for SOAP response.", params(1,s.c_str() ? s.c_str() : "none"));
    
    // Parse and bind the document into an XMLObject.
    DOMDocument* doc = (m_validate ? XMLToolingConfig::getConfig().getValidatingParser()
        : XMLToolingConfig::getConfig().getParser()).parse(out); 
    XercesJanitor<DOMDocument> janitor(doc);

    Category& log = Category::getInstance(XMLTOOLING_LOGCAT".SOAPClient");
    if (log.isDebugEnabled()) {
#ifdef XMLTOOLING_LOG4SHIB
        log.debugStream() << "received XML:\n" << *(doc->getDocumentElement()) << logging::eol;
#else
        string buf;
        XMLHelper::serialize(doc->getDocumentElement(), buf);
        log.debugStream() << "received XML:\n" << buf << logging::eol;
#endif
    }
    
    auto_ptr<XMLObject> xmlObject(XMLObjectBuilder::buildOneFromElement(doc->getDocumentElement(), true));
    janitor.release();
    if (!m_validate)
        SchemaValidators.validate(xmlObject.get());

    Envelope* env = dynamic_cast<Envelope*>(xmlObject.get());
    if (!env)
        throw IOException("Response was not a SOAP 1.1 Envelope.");

    Body* body = env->getBody();
    if (body && body->hasChildren()) {
        //Check for a Fault.
        const Fault* fault = dynamic_cast<Fault*>(body->getUnknownXMLObjects().front());
        if (fault && handleFault(*fault))
            throw IOException("SOAP client detected a Fault.");
    }

    xmlObject.release();
    return env;
}

bool SOAPClient::handleFault(const Fault& fault)
{
    const xmltooling::QName* code = (fault.getFaultcode() ? fault.getFaultcode()->getCode() : NULL);
    auto_ptr_char str((fault.getFaultstring() ? fault.getFaultstring()->getString() : NULL));
    Category::getInstance(XMLTOOLING_LOGCAT".SOAPClient").error(
        "SOAP client detected a Fault: (%s) (%s)",
        (code ? code->toString().c_str() : "no code"),
        (str.get() ? str.get() : "no message")
        );
    return true;
}
