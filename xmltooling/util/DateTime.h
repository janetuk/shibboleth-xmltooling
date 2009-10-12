/*
 *  Copyright 2001-2009 Internet2
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
 * @file xmltooling/util/DateTime.h
 *
 * Manipulation of XML date/time data.
 */

#ifndef __xmltool_datetime_h__
#define __xmltool_datetime_h__

#include <xmltooling/base.h>

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4244 )
#endif

#include <xercesc/util/XMLDateTime.hpp>

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

namespace xmltooling
{
    /**
     * Class for manipulating XML date/time information.
     *
     * This is mostly copied from Xerces-C, but they haven't produced a usable date/time
     * class, so we had to incorporate a version of it for now. It can't be inherited
     * since the fields needed are private.
     */
    class XMLTOOL_API DateTime
    {
    public:
        /// @cond OFF
        DateTime();
        DateTime(const XMLCh* const);
        DateTime(time_t epoch, bool duration=false);
        DateTime(const DateTime&);
        DateTime& operator=(const DateTime&);
        ~DateTime();

        void setBuffer(const XMLCh* const);

        const XMLCh* getRawData() const;
        const XMLCh* getFormattedString() const;
        int getSign() const;

        XMLCh* getDateTimeCanonicalRepresentation() const;
        XMLCh* getTimeCanonicalRepresentation() const;

        void parseDateTime();
        void parseDate();
        void parseTime();
        void parseDay();
        void parseMonth();
        void parseYear();
        void parseMonthDay();
        void parseYearMonth();
        void parseDuration();

        static int compare(const DateTime* const, const DateTime* const);
        static int compare(const DateTime* const, const DateTime* const, bool);
        static int compareOrder(const DateTime* const, const DateTime* const);

        int getYear() const {return fValue[CentYear];}
        int getMonth() const {return fValue[Month];}
        int getDay() const {return fValue[Day];}
        int getHour() const {return fValue[Hour];}
        int getMinute() const {return fValue[Minute];}
        int getSecond() const {return fValue[Second];}
        time_t getEpoch(bool duration=false) const;

        /// @endcond
    private:
        enum valueIndex {
            CentYear   = 0,
            Month      ,
            Day        ,
            Hour       ,
            Minute     ,
            Second     ,
            MiliSecond ,  //not to be used directly
            utc        ,
            TOTAL_SIZE
        };

        enum utcType {
            UTC_UNKNOWN = 0,
            UTC_STD        ,          // set in parse() or normalize()
            UTC_POS        ,          // set in parse()
            UTC_NEG                   // set in parse()
        };

        enum timezoneIndex {
            hh = 0,
            mm ,
            TIMEZONE_ARRAYSIZE
        };

        static int compareResult(int, int, bool);
        static void addDuration(DateTime* pDuration, const DateTime* const pBaseDate, int index);
        static int compareResult(const DateTime* const, const DateTime* const, bool, int);
        static int getRetVal(int, int);

        void reset();
        //inline void assertBuffer() const;
        void copy(const DateTime&);

        void initParser();
        bool isNormalized() const;

        void getDate();
        void getTime();
        void getYearMonth();
        void getTimeZone(const int);
        void parseTimeZone();

        int findUTCSign(const int start);
        int indexOf(const int start, const int end, const XMLCh ch) const;
        int parseInt(const int start, const int end) const;
        int parseIntYear(const int end) const;
        double parseMiliSecond(const int start, const int end) const;

        void validateDateTime() const;
        void normalize();
        void fillString(XMLCh*& ptr, valueIndex ind, int expLen) const;
        int  fillYearString(XMLCh*& ptr, valueIndex ind) const;
        void searchMiliSeconds(XMLCh*& miliStartPtr, XMLCh*& miliEndPtr) const;

    	bool operator==(const DateTime& toCompare) const;

        static const int DATETIMES[][TOTAL_SIZE];
        int fValue[TOTAL_SIZE];
        int fTimeZone[TIMEZONE_ARRAYSIZE];
        int fStart;
        int fEnd;
        int fBufferMaxLen;
        XMLCh* fBuffer;

        double fMiliSecond;
        bool fHasTime;
    };
}

#endif /* __xmltool_datetime_h__ */
