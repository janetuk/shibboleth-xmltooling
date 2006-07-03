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
 * XMLConstants.cpp
 * 
 * Fundamental XML namespace constants 
 */


#include "internal.h"
#include "util/XMLConstants.h"
#include <xercesc/util/XMLUniDefs.hpp>

using namespace xmltooling;
    
const XMLCh XMLConstants::XML_NS[] = // http://www.w3.org/XML/1998/namespace
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chLatin_X, chLatin_M, chLatin_L, chForwardSlash, chDigit_1, chDigit_9, chDigit_9, chDigit_8, chForwardSlash,
  chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh XMLConstants::XMLNS_NS[] = // http://www.w3.org/2000/xmlns/
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chDigit_2, chDigit_0, chDigit_0, chDigit_0, chForwardSlash,
  chLatin_x, chLatin_m, chLatin_l, chLatin_n, chLatin_s, chForwardSlash, chNull
};

const XMLCh XMLConstants::XMLNS_PREFIX[] = { chLatin_x, chLatin_m, chLatin_l, chLatin_n, chLatin_s, chNull };

const XMLCh XMLConstants::XML_PREFIX[] = { chLatin_x, chLatin_m, chLatin_l, chNull };

const XMLCh XMLConstants::XSD_NS[] = // http://www.w3.org/2001/XMLSchema
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash,
  chLatin_X, chLatin_M, chLatin_L, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chNull
};

const XMLCh XMLConstants::XSD_PREFIX[] = { chLatin_x, chLatin_s, chNull };

const XMLCh XMLConstants::XSI_NS[] = // http://www.w3.org/2001/XMLSchema-instance
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash,
  chLatin_X, chLatin_M, chLatin_L, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chDash,
  chLatin_i, chLatin_n, chLatin_s, chLatin_t, chLatin_a, chLatin_n, chLatin_c, chLatin_e, chNull
};

const XMLCh XMLConstants::XSI_PREFIX[] = { chLatin_x, chLatin_s, chLatin_i, chNull };

const XMLCh XMLConstants::XMLSIG_NS[] = // http://www.w3.org/2000/09/xmldsig#
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chDigit_2, chDigit_0, chDigit_0, chDigit_0, chForwardSlash, chDigit_0, chDigit_9, chForwardSlash,
  chLatin_x, chLatin_m, chLatin_l, chLatin_d, chLatin_s, chLatin_i, chLatin_g, chPound, chNull
};

const XMLCh XMLConstants::XMLSIG_PREFIX[] = { chLatin_d, chLatin_s, chNull };

const XMLCh XMLConstants::XMLENC_NS[] = // http://www.w3.org/2001/04/xmlenc#
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash, chDigit_0, chDigit_4, chForwardSlash,
  chLatin_x, chLatin_m, chLatin_l, chLatin_e, chLatin_n, chLatin_c, chPound, chNull
};

const XMLCh XMLConstants::XMLENC_PREFIX[] = { chLatin_x, chLatin_e, chLatin_n, chLatin_c, chNull };

const XMLCh XMLConstants::XMLTOOLING_NS[] = // http://www.opensaml.org/xmltooling
{ chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash,
  chLatin_w, chLatin_w, chLatin_w, chPeriod,
  chLatin_o, chLatin_p, chLatin_e, chLatin_n, chLatin_s, chLatin_a, chLatin_m, chLatin_l, chPeriod,
  chLatin_o, chLatin_r, chLatin_g, chForwardSlash,
  chLatin_x, chLatin_m, chLatin_l, chLatin_t, chLatin_o, chLatin_o, chLatin_l, chLatin_i, chLatin_n, chLatin_g, chNull
};

const XMLCh XMLConstants::XML_TRUE[] = { chLatin_t, chLatin_r, chLatin_u, chLatin_e, chNull };

const XMLCh XMLConstants::XML_FALSE[] = { chLatin_f, chLatin_a, chLatin_l, chLatin_s, chLatin_e, chNull };

const XMLCh XMLConstants::XML_ONE[] = { chDigit_1, chNull };

const XMLCh XMLConstants::XML_ZERO[] = { chDigit_0, chNull };
