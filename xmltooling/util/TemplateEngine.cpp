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
 * TemplateEngine.cpp
 *
 * Simple template replacement engine.
 */

#include "internal.h"
#include "io/GenericRequest.h"
#include "util/TemplateEngine.h"

using namespace xmltooling;
using namespace std;

namespace {
    static const pair<const string,string> emptyPair;
}

TemplateEngine::TemplateEngine()
{
    setTagPrefix("mlp");
}

TemplateEngine::~TemplateEngine()
{
}

TemplateEngine::TemplateParameters::TemplateParameters() : m_request(nullptr)
{
}

TemplateEngine::TemplateParameters::~TemplateParameters()
{
}

const char* TemplateEngine::TemplateParameters::getParameter(const char* name) const
{
    map<string,string>::const_iterator i=m_map.find(name);
    return (i!=m_map.end() ? i->second.c_str() : (m_request ? m_request->getParameter(name) : nullptr));
}

const multimap<string,string>* TemplateEngine::TemplateParameters::getLoopCollection(const char* name) const
{
    map< string,multimap<string,string> >::const_iterator i=m_collectionMap.find(name);
    return (i!=m_collectionMap.end() ? &(i->second) : nullptr);
}

void TemplateEngine::setTagPrefix(const char* tagPrefix)
{
    keytag = string("<") + tagPrefix + " ";
    iftag = string("<") + tagPrefix + "if ";
    ifnottag = string("<") + tagPrefix + "ifnot ";
    ifendtag = string("</") + tagPrefix + "if>";
    ifnotendtag = string("</") + tagPrefix + "ifnot>";
    fortag = string("<") + tagPrefix + "for ";
    forendtag = string("</") + tagPrefix + "for>";
}

string TemplateEngine::unsafe_chars = "#%&():[]\\`{}";

void TemplateEngine::html_encode(ostream& os, const char* start) const
{
    while (start && *start) {
        switch (*start) {
            case '<':   os << "&lt;";       break;
            case '>':   os << "&gt;";       break;
            case '"':   os << "&quot;";     break;
            case '&':   os << "&#38;";      break;
            case '\'':  os << "&#39;";      break;

            default:
                if (unsafe_chars.find_first_of(*start) != string::npos)
                    os << "&#" << static_cast<short>(*start) << ';';
                else
                    os << *start;

            /*
            case '#':   os << "&#35;";      break;
            case '%':   os << "&#37;";      break;
            case '(':   os << "&#40;";      break;
            case ')':   os << "&#41;";      break;
            case ':':   os << "&#58;";      break;
            case '[':   os << "&#91;";      break;
            case '\\':  os << "&#92;";      break;
            case ']':   os << "&#93;";      break;
            case '`':   os << "&#96;";      break;
            case '{':   os << "&#123;";     break;
            case '}':   os << "&#125;";     break;
            default:    os << *start;
            */
        }
        start++;
    }
}

void TemplateEngine::trimspace(string& s) const
{
  string::size_type end = s.size() - 1, start = 0;

  // Trim stuff on right.
  while (end > 0 && !isgraph(s[end])) end--;

  // Trim stuff on left.
  while (start < end && !isgraph(s[start])) start++;

  // Modify the string.
  s = s.substr(start, end - start + 1);
}

void TemplateEngine::process(
    bool visible,
    const string& buf,
    const char*& lastpos,
    ostream& os,
    const TemplateParameters& parameters,
    const std::pair<const std::string,std::string>& loopentry,
    const XMLToolingException* e
    ) const
{
    const char* line = buf.c_str();
    const char* thispos;

    while ((thispos = strchr(lastpos, '<')) != nullptr) {
        // Output the string up to this token.
        if (visible)
            os << buf.substr(lastpos-line, thispos-lastpos);

        // Make sure this token matches our tokens.
#ifdef HAVE_STRCASECMP
        if (visible && !strncasecmp(thispos, keytag.c_str(), keytag.length()))
#else
        if (visible && !_strnicmp(thispos, keytag.c_str(), keytag.length()))
#endif
        {
            // Save this position off.
            lastpos = thispos + keytag.length();

            // search for the end-tag
            if ((thispos = strstr(lastpos, "/>")) != nullptr) {
                string key = buf.substr(lastpos-line, thispos-lastpos);
                trimspace(key);

                if (key == "$name" && !loopentry.first.empty())
                    html_encode(os,loopentry.first.c_str());
                else if (key == "$value" && !loopentry.second.empty())
                    html_encode(os,loopentry.second.c_str());
                else {
                    const char* p = parameters.getParameter(key.c_str());
                    if (!p && e)
                        p = e->getProperty(key.c_str());
                    if (p)
                        html_encode(os,p);
                }
                lastpos = thispos + 2; // strlen("/>")
            }
        }
#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, iftag.c_str(), iftag.length()))
#else
        else if (!_strnicmp(thispos, iftag.c_str(), iftag.length()))
#endif
        {
            // Save this position off.
            lastpos = thispos + iftag.length();

            // search for the end of this tag
            if ((thispos = strchr(lastpos, '>')) != nullptr) {
                string key = buf.substr(lastpos-line, thispos-lastpos);
                trimspace(key);
                bool cond=false;
                if (visible)
                    cond = parameters.getParameter(key.c_str()) || (e && e->getProperty(key.c_str()));
                lastpos = thispos + 1; // strlen(">")
                process(cond, buf, lastpos, os, parameters, loopentry, e);
            }
        }
#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, ifendtag.c_str(), ifendtag.length()))
#else
        else if (!_strnicmp(thispos, ifendtag.c_str(), ifendtag.length()))
#endif
        {
            // Save this position off and pop the stack.
            lastpos = thispos + ifendtag.length();
            return;
        }
#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, ifnottag.c_str(), ifnottag.length()))
#else
        else if (!_strnicmp(thispos, ifnottag.c_str(), ifnottag.length()))
#endif
        {
            // Save this position off.
            lastpos = thispos + ifnottag.length();

            // search for the end of this tag
            if ((thispos = strchr(lastpos, '>')) != nullptr) {
                string key = buf.substr(lastpos-line, thispos-lastpos);
                trimspace(key);
                bool cond=visible;
                if (visible)
                    cond = !(parameters.getParameter(key.c_str()) || (e && e->getProperty(key.c_str())));
                lastpos = thispos + 1; // strlen(">")
                process(cond, buf, lastpos, os, parameters, loopentry, e);
            }
        }
#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, ifnotendtag.c_str(), ifnotendtag.length()))
#else
        else if (!_strnicmp(thispos, ifnotendtag.c_str(), ifnotendtag.length()))
#endif
        {
            // Save this position off and pop the stack.
            lastpos = thispos + ifnotendtag.length();
            return;
        }

#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, fortag.c_str(), fortag.length()))
#else
        else if (!_strnicmp(thispos, fortag.c_str(), fortag.length()))
#endif
        {
            // Save this position off.
            lastpos = thispos + iftag.length();
            string key;
            bool cond = visible;

            // search for the end of this tag
            if ((thispos = strchr(lastpos, '>')) != nullptr) {
                key = buf.substr(lastpos-line, thispos-lastpos);
                trimspace(key);
                lastpos = thispos + 1; // strlen(">")
            }

            const multimap<string,string>* forParams = parameters.getLoopCollection(key.c_str());
            if (!forParams || forParams->size() == 0) {
                process(false, buf, lastpos, os, parameters, emptyPair, e);
            }
            else {
                const char* savlastpos = lastpos;
                for (multimap<string,string>::const_iterator i=forParams->begin(); i!=forParams->end(); ++i) {
                    lastpos = savlastpos;
                    process(cond, buf, lastpos, os, parameters, *i, e);
                }
            }
        }

#ifdef HAVE_STRCASECMP
        else if (!strncasecmp(thispos, forendtag.c_str(), forendtag.length()))
#else
        else if (!_strnicmp(thispos, forendtag.c_str(), forendtag.length()))
#endif
        {
            // Save this position off and pop the stack.
            lastpos = thispos + forendtag.length();
            return;
        }

        else {
            // Skip it.
            if (visible)
                os << '<';
            lastpos = thispos + 1;
        }
    }
    if (visible)
        os << buf.substr(lastpos-line);
}

void TemplateEngine::run(istream& is, ostream& os, const TemplateParameters& parameters, const XMLToolingException* e) const
{
    string buf,line;
    while (getline(is, line))
        buf += line + '\n';

    const char* pos=buf.c_str();
    process(true, buf, pos, os, parameters, emptyPair, e);
}
