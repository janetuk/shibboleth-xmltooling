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
 * SOAPClient.cpp
 * 
 * Implements SOAP 1.1 messaging over a transport.
 */

#include "internal.h"
#include "exceptions.h"
#include "soap/SOAP.h"
#include "soap/SOAPClient.h"
#include "util/XMLHelper.h"
#include "validation/ValidatorSuite.h"

#include <sstream>
#include <log4cpp/Category.hh>

using namespace soap11;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

SOAPClient::~SOAPClient()
{
    reset();
}

void SOAPClient::reset()
{
    delete m_transport;
    m_transport=NULL;
}

void SOAPClient::send(const Envelope* env, const KeyInfoSource& peer, const char* endpoint)
{
    // Prepare a transport object.
    const char* pch = strchr(endpoint,':');
    if (!pch)
        throw IOException("SOAP endpoint was not a URL.");
    string scheme(endpoint, pch-endpoint);
    m_transport = XMLToolingConfig::getConfig().SOAPTransportManager.newPlugin(scheme.c_str(), make_pair(&peer,endpoint));
    prepareTransport(*m_transport);
    
    // Serialize envelope.
    stringstream s;
    s << *env;
    
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
    
    // Parse and bind the document into an XMLObject.
    DOMDocument* doc = (m_validate ? XMLToolingConfig::getConfig().getValidatingParser()
        : XMLToolingConfig::getConfig().getParser()).parse(out); 
    XercesJanitor<DOMDocument> janitor(doc);
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
        const Fault* fault = dynamic_cast<Fault*>(body->getXMLObjects().front());
        if (fault && handleFault(*fault))
            throw IOException("SOAP client detected a Fault.");
    }

    xmlObject.release();
    return env;
}

bool SOAPClient::handleFault(const Fault& fault)
{
    const QName* code = (fault.getFaultcode() ? fault.getFaultcode()->getCode() : NULL);
    auto_ptr_char str((fault.getFaultstring() ? fault.getFaultstring()->getString() : NULL));
    Category::getInstance(XMLTOOLING_LOGCAT".SOAPClient").error(
        "SOAP client detected a Fault: (%s) (%s)",
        (code ? code->toString().c_str() : "no code"),
        (str.get() ? str.get() : "no message")
        );
    return true;
}
