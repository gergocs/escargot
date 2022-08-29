#if defined(ESCARGOT_ENABLE_TEMPORAL)
/*
 * Copyright (c) 2022-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __EscargotTemporalObject__
#define __EscargotTemporalObject__

#include "Escargot.h"
#include "runtime/Object.h"
#include "runtime/GlobalObject.h"
#include "runtime/Temporal.h"
#include "runtime/DateObject.h"

namespace Escargot {

class TemporalDate {
public:
    TemporalDate(short mYear, char mMonth, char mDay);
    short m_year;
    char m_month;
    char m_day;
};

class TemporalTime {
public:
    TemporalTime(char mHour, char mMinute, char mSecond, short mMillisecond, short mMicrosecond, short mNanosecond);
    TemporalTime();
    char m_hour;
    char m_minute;
    char m_second;
    short m_millisecond;
    short m_microsecond;
    short m_nanosecond;
};

static const char* dateTimeUnitStrings[] = { "year", "month", "week", "day", "hour", "minute", "second", "millisecond", "microsecond", "nanosecond" };

class TemporalObject : public Temporal {
public:
    class TimeZone : public gc {
    public:
        TimeZone(bool z, String* offsetString, String* name);
        bool z;
        String* offsetString;
        String* name;
    };

    class DateTime : public gc {
    public:
        DateTime(int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, String* calendar, TimeZone* tz);

        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int millisecond;
        int microsecond;
        int nanosecond;
        String* calendar;
        TimeZone* tz;
    };

    enum DateTimeUnits {
        YEAR_UNIT,
        MONTH_UNIT,
        WEEK_UNIT,
        DAY_UNIT,
        HOUR_UNIT,
        MINUTE_UNIT,
        SECOND_UNIT,
        MILLISECOND_UNIT,
        MICROSECOND_UNIT,
        NANOSECOND_UNIT
    };

    explicit TemporalObject(ExecutionState& state);
    explicit TemporalObject(ExecutionState& state, Object* proto);
    static Value toISODateTime(ExecutionState& state, DateObject& d);
    static Value toISODate(ExecutionState& state, DateObject& d);
    static Value toISOTime(ExecutionState& state, DateObject& d);
    static TemporalObject::DateTime parseValidIso8601String(ExecutionState& state, std::string isoString, bool parseTimeZone);
    static TemporalObject::DateTime parseTemporalInstantString(ExecutionState& state, const std::string& isoString);
    static TemporalObject::DateTime parseTemporalDateString(ExecutionState& state, const std::string& isoString);
    static TemporalObject::DateTime parseTemporalDateTimeString(ExecutionState& state, const std::string& isoString);
    static std::map<TemporalObject::DateTimeUnits, int> parseTemporalDurationString(ExecutionState& state, const std::string& isoString);
    static TemporalObject::DateTime parseTemporalYearMonthString(ExecutionState& state, const std::string& isoString);
    static TemporalObject::DateTime parseTemporalMonthDayString(ExecutionState& state, const std::string& isoString);
    static TemporalObject::TimeZone parseTemporalTimeZoneString(ExecutionState& state, const std::string& isoString);
    static std::string getNNumberFromString(std::string& isoString, int n, unsigned int& index);
    static std::map<TemporalObject::DateTimeUnits, int> getSeconds(ExecutionState& state, std::string& isoString, unsigned int& index);
    static std::string offset(ExecutionState& state, std::string& isoString, unsigned int& index);
    static std::string tzComponent(ExecutionState& state, std::string& isoString, unsigned int& index);
    static TemporalObject::TimeZone parseTimeZoneOffset(ExecutionState& state, std::string& isoString, unsigned int& index);
    static bool isNumber(const std::string& s)
    {
        return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
};

class TemporalCalendarObject : public Temporal {
public:
    explicit TemporalCalendarObject(ExecutionState& state);
    explicit TemporalCalendarObject(ExecutionState& state, Object* proto, String* identifier = nullptr);

    bool isTemporalCalendarObject() const override
    {
        return true;
    }

    static Value getterHelper(ExecutionState& state, const Value& callee, Object* thisValue, Value* argv);

    static TemporalCalendarObject* createTemporalCalendar(ExecutionState& state, String* id, Optional<Object*> newTarget);
    static bool isBuiltinCalendar(String* id);
    static Value getBuiltinCalendar(ExecutionState& state, String* id);
    static Value getISO8601Calendar(ExecutionState& state);
    static ValueVector calendarFields(ExecutionState& state, const Value& calendar, const ValueVector& fieldNames);
    static Value calendarYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarMonth(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarMonthCode(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDay(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDayOfWeek(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDayOfYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarWeekOfYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDaysInWeek(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDaysInMonth(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarDaysInYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarMonthsInYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value calendarInLeapYear(ExecutionState& state, Object* calendar, const Value& dateLike);
    static Value toTemporalCalendar(ExecutionState& state, const Value& calendar);
    static Value toTemporalCalendarWithISODefault(ExecutionState& state, const Value& calendar);
    static Value getTemporalCalendarWithISODefault(ExecutionState& state, const Value& item);
    static Value dateFromFields(ExecutionState& state, const Value& calendar, const Value& fields, const Value& options);
    static Value calendarYearMonthFromFields(ExecutionState& state, const Value& calendar, const Value& fields, const Value& options);
    static Value calendarMonthDayFromFields(ExecutionState& state, const Value& calendar, const Value& fields, const Value& options);
    static int toISOWeekOfYear(ExecutionState& state, int year, int month, int day);
    static Value parseTemporalCalendarString(ExecutionState& state, const Value& isoString);
    static bool calendarEquals(const TemporalCalendarObject& firstCalendar, const TemporalCalendarObject& secondCalendar);
    static Value ISODaysInYear(ExecutionState& state, int year);
    static Value ISODaysInMonth(ExecutionState& state, int year, int month);
    static bool isIsoLeapYear(ExecutionState& state, int year);
    static std::string buildISOMonthCode(ExecutionState& state, int month);
    static int ISOYear(ExecutionState& state, const Value& temporalObject);
    static int ISOMonth(ExecutionState& state, const Value& temporalObject);
    static std::string ISOMonthCode(ExecutionState& state, const Value& temporalObject);
    static int ISODay(ExecutionState& state, const Value& temporalObject);
    static Value defaultMergeFields(ExecutionState& state, const Value& fields, const Value& additionalFields);
    String* getIdentifier() const
    {
        return m_identifier;
    }

    static int dayOfYear(ExecutionState& state, const Value& epochDays) { return DateObject::daysInYear(DateObject::makeDate(state, epochDays, Value(0)).toInt32(state)) + 1; }

    bool operator==(const TemporalCalendarObject& rhs) const;
    bool operator!=(const TemporalCalendarObject& rhs) const;

private:
    String* m_identifier;
};

class TemporalPlainDateObject : public Temporal, private TemporalDate {
public:
    explicit TemporalPlainDateObject(ExecutionState& state, int isoYear, int isoMonth, int isoDay, Optional<Value> calendarLike);
    explicit TemporalPlainDateObject(ExecutionState& state, Object* proto, short isoYear, char isoMonth, char isoDay, Optional<Value> calendarLike);

    bool isTemporalPlainDateObject() const override
    {
        return true;
    }

    static std::map<TemporalObject::DateTimeUnits, int> createISODateRecord(ExecutionState& state, int year, int month, int day);
    static Value createTemporalDate(ExecutionState& state, int isoYear, int isoMonth, int isoDay, const Value& calendar, Optional<Object*> newTarget = nullptr);
    static bool isValidISODate(ExecutionState& state, int year, int month, int day);
    static Value toTemporalDate(ExecutionState& state, const Value& item, Optional<Object*> options = nullptr);
    static std::map<TemporalObject::DateTimeUnits, int> balanceISODate(ExecutionState& state, int year, int month, int day);
    static int compareISODate(int firstYear, int firstMonth, int firstDay, int secondYear, int secondMonth, int secondDay);

    short year() const
    {
        return m_year;
    }
    char month() const
    {
        return m_month;
    }
    char day() const
    {
        return m_day;
    }
    TemporalCalendarObject* calendar() const { return m_calendar; }

private:
    TemporalCalendarObject* m_calendar;
};

class TemporalPlainTimeObject : public Temporal, private TemporalTime {
public:
    explicit TemporalPlainTimeObject(ExecutionState& state);
    explicit TemporalPlainTimeObject(ExecutionState& state, Object* proto);

    bool isTemporalPlainTimeObject() const override
    {
        return true;
    }

    static Value toTemporalTime(ExecutionState& state, const Value& item, Value options);
    static std::map<TemporalObject::DateTimeUnits, Value> toPartialTime(ExecutionState& state, const Value& temporalTimeLike);
    static std::map<TemporalObject::DateTimeUnits, int> regulateTime(ExecutionState& state, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, const Value& overflow);
    static Value createTemporalTime(ExecutionState& state, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, Optional<Object*> newTarget);
    static bool isValidTime(ExecutionState& state, int h, int m, int s, int ms, int us, int ns);
    static std::map<TemporalObject::DateTimeUnits, int> balanceTime(ExecutionState& state, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static std::map<TemporalObject::DateTimeUnits, int> constrainTime(ExecutionState& state, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static std::map<TemporalObject::DateTimeUnits, int> toTemporalTimeRecord(ExecutionState& state, const Value& temporalTimeLike);
    static int compareTemporalTime(ExecutionState& state, const int time1[6], const int time2[6]);

    void setTime(char h, char m, char s, short ms, short qs, short ns);
    void setCalendar(ExecutionState& state, String* calendar);

    char getHour() const
    {
        return m_hour;
    }
    char getMinute() const
    {
        return m_minute;
    }
    char getSecond() const
    {
        return m_second;
    }
    short getMillisecond() const
    {
        return m_millisecond;
    }
    short getMicrosecond() const
    {
        return m_microsecond;
    }
    short getNanosecond() const
    {
        return m_nanosecond;
    }
    TemporalCalendarObject* getCalendar() const
    {
        return m_calendar;
    }

private:
    TemporalCalendarObject* m_calendar;
};

class TemporalPlainDateTimeObject : public Temporal, private TemporalDate, private TemporalTime {
public:
    explicit TemporalPlainDateTimeObject(ExecutionState& state);
    explicit TemporalPlainDateTimeObject(ExecutionState& state, Object* proto, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);

    bool isTemporalPlainDateTimeObject() const override
    {
        return true;
    }

    static uint64_t getEpochFromISOParts(ExecutionState& state, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static bool ISODateTimeWithinLimits(ExecutionState& state, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static std::map<TemporalObject::DateTimeUnits, std::string> interpretTemporalDateTimeFields(ExecutionState& state, const Value& calendar, const Value& fields, const Value& options);
    static Value toTemporalDateTime(ExecutionState& state, const Value& item, const Value& options = Value());
    static std::map<TemporalObject::DateTimeUnits, int> balanceISODateTime(ExecutionState& state, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static Value createTemporalDateTime(ExecutionState& state, int isoYear, int isoMonth, int isoDay, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond, const Value& calendar, Optional<Object*> newTarget);

    short getYear() const
    {
        return m_year;
    }
    char getMonth() const
    {
        return m_month;
    }
    char getDay() const
    {
        return m_day;
    }
    char getHour() const
    {
        return m_hour;
    }
    char getMinute() const
    {
        return m_minute;
    }
    char getSecond() const
    {
        return m_second;
    }
    short getMillisecond() const
    {
        return m_millisecond;
    }
    short getMicrosecond() const
    {
        return m_microsecond;
    }
    short getNanosecond() const
    {
        return m_nanosecond;
    }

    Value getCalendar() const
    {
        return Value(m_calendar);
    }
    void setCalendar(const Value mCalendar)
    {
        m_calendar = mCalendar.asObject()->asTemporalCalendarObject();
    }

private:
    TemporalCalendarObject* m_calendar;
};

class TemporalZonedDateTimeObject : public Temporal {
public:
    explicit TemporalZonedDateTimeObject(ExecutionState& state);
    explicit TemporalZonedDateTimeObject(ExecutionState& state, Object* proto);

    bool isTemporalZonedDateTimeObject() const override
    {
        return true;
    }

    const BigInt* getNanoseconds() const
    {
        return m_nanoseconds;
    }

    void setNanoseconds(BigInt* mNanoseconds)
    {
        m_nanoseconds = mNanoseconds;
    }

    const Value& getTimeZone() const
    {
        return m_timeZone;
    }

    void setTimeZone(const Value& mTimeZone)
    {
        m_timeZone = mTimeZone;
    }

    const TemporalCalendarObject* getCalendar() const
    {
        return m_calendar;
    }

    void setCalendar(const Value mCalendar)
    {
        m_calendar = mCalendar.asObject()->asTemporalCalendarObject();
    }

private:
    BigInt* m_nanoseconds;
    Value m_timeZone;
    TemporalCalendarObject* m_calendar;
};

class TemporalDurationObject : public Temporal, private TemporalDate, private TemporalTime {
public:
    explicit TemporalDurationObject(ExecutionState& state, int years, int months, int weeks, int days, int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanoseconds);
    explicit TemporalDurationObject(ExecutionState& state, Object* proto, int years, int months, int weeks, int days, int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanosecond);
    static bool isValidDuration(const int fields[]);
    static int durationSign(const int fields[]);
    static std::map<TemporalObject::DateTimeUnits, int> createDurationRecord(ExecutionState& state, int years, int months, int weeks, int days, int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanoseconds);
    static Value createTemporalDuration(ExecutionState& state, int years, int months, int weeks, int days, int hours, int minutes, int seconds, int milliseconds, int microseconds, int nanoseconds, Optional<Object*> newTarget);

    static Value toTemporalDuration(ExecutionState& state, const Value& item);
    static std::map<TemporalObject::DateTimeUnits, int> toTemporalDurationRecord(ExecutionState& state, const Value& temporalDurationLike);
    static std::map<TemporalObject::DateTimeUnits, Value> toTemporalPartialDurationRecord(ExecutionState& state, const Value& temporalDurationLike);
    static Value createNegatedTemporalDuration(ExecutionState& state, const Value& duration);

    bool isTemporalDurationObject() const override
    {
        return true;
    }
    int getYear() const
    {
        return m_year;
    }
    int getMonth() const
    {
        return m_month;
    }
    int getWeek() const
    {
        return m_week;
    }
    int getDay() const
    {
        return m_day;
    }
    int getHour() const
    {
        return m_hour;
    }
    int getMinute() const
    {
        return m_minute;
    }
    int getSecond() const
    {
        return m_second;
    }
    int getMillisecond() const
    {
        return m_millisecond;
    }
    int getMicrosecond() const
    {
        return m_microsecond;
    }
    int getNanosecond() const
    {
        return m_nanosecond;
    }
    TemporalCalendarObject* getCalendar() const
    {
        return m_calendar;
    }

private:
    int m_week;
    TemporalCalendarObject* m_calendar;
};

class TemporalInstantObject : public Temporal {
public:
    static const int64_t dayToNanosecond = 86400000000000;
    static const int64_t HourToNanosecond = 3600000000000;
    static const int64_t MinuteToNanosecond = 60000000000;
    static const int64_t SecondToNanosecond = 1000000000;
    static const int64_t MillisecondToNanosecond = 1000000;
    static const int64_t MicrosecondToNanosecond = 1000;

    explicit TemporalInstantObject(ExecutionState& state);
    explicit TemporalInstantObject(ExecutionState& state, Object* proto);

    static bool isValidEpochNanoseconds(const Value& epochNanoseconds);
    static Value createTemporalInstant(ExecutionState& state, const Value& epochNanoseconds, Optional<Object*> newTarget = nullptr);
    static Value toTemporalInstant(ExecutionState& state, const Value& item);
    static Value parseTemporalInstant(ExecutionState& state, const std::string& isoString);
    static int compareEpochNanoseconds(ExecutionState& state, const BigInt& firstNanoSeconds, const BigInt& secondNanoSeconds);

    static int64_t offsetStringToNanoseconds(ExecutionState& state, String* offset);

    bool isTemporalInstantObject() const override
    {
        return true;
    }

    const BigInt* getNanoseconds() const
    {
        return m_nanoseconds;
    }

    void setNanoseconds(BigInt* mNanoseconds)
    {
        m_nanoseconds = mNanoseconds;
    }

private:
    BigInt* m_nanoseconds;
};

class TemporalPlainYearMonthObject : public Temporal {
public:
    explicit TemporalPlainYearMonthObject(ExecutionState& state);
    explicit TemporalPlainYearMonthObject(ExecutionState& state, Object* proto, int isoYear = 0, int isoMonth = 0, Object* calendar = nullptr, int referenceISODay = 0);

    static Value createTemporalYearMonth(ExecutionState& state, int isoYear, int isoMonth, const Value& calendar, int referenceISODay, Optional<Object*> newTarget = nullptr);
    static bool isoYearMonthWithinLimits(int isoYear, int isoMonth);
    static Value toTemporalYearMonth(ExecutionState& state, const Value& item, const Value& options);

    int getIsoYear() const
    {
        return m_isoYear;
    }
    int getIsoMonth() const
    {
        return m_isoMonth;
    }
    Object* getCalendar() const
    {
        return m_calendar;
    }
    int getReferenceIsoDay() const
    {
        return m_referenceISODay;
    }

private:
    int m_isoYear;
    int m_isoMonth;
    Object* m_calendar;
    int m_referenceISODay;
};

class TemporalPlainMonthDayObject : public Temporal {
public:
    explicit TemporalPlainMonthDayObject(ExecutionState& state);
    explicit TemporalPlainMonthDayObject(ExecutionState& state, Object* proto, int isoMonth = 0, int isoDay = 0, TemporalCalendarObject* calendar = nullptr, int referenceISOYear = 0);

    static Value createTemporalMonthDay(ExecutionState& state, int isoMonth, int isoDay, TemporalCalendarObject* calendar, int referenceISOYear, Optional<Object*> newTarget = nullptr);
    static Value toTemporalMonthDay(ExecutionState& state, const Value& item, const Value& options);

    bool isTemporalPlainMonthDayObject() const override
    {
        return true;
    }

    int getIsoMonth() const
    {
        return m_isoMonth;
    }
    int getIsoDay() const
    {
        return m_isoDay;
    }
    TemporalCalendarObject* getCalendar() const
    {
        return m_calendar;
    }
    int getReferenceIsoYear() const
    {
        return m_referenceISOYear;
    }

private:
    int m_isoMonth;
    int m_isoDay;
    TemporalCalendarObject* m_calendar;
    int m_referenceISOYear;
};

class TemporalTimeZoneObject : public Temporal {
public:
    explicit TemporalTimeZoneObject(ExecutionState& state);
    explicit TemporalTimeZoneObject(ExecutionState& state, Object* proto, ASCIIString* identifier = new ASCIIString(""), const Value& offsetNanoseconds = Value());

    static std::map<TemporalObject::DateTimeUnits, int> getISOPartsFromEpoch(ExecutionState& state, const Value& epochNanoseconds);
    static Value createTemporalTimeZone(ExecutionState& state, const std::string& identifier, Optional<Object*> newTarget = nullptr);
    static bool isValidTimeZoneName(const std::string& timeZone);
    static std::string canonicalizeTimeZoneName(const std::string& timeZone);
    static std::string formatTimeZoneOffsetString(long long offsetNanoseconds);
    static std::string defaultTimeZone();
    static Value toTemporalTimeZone(ExecutionState& state, const Value& temporalTimeZoneLike);
    static std::map<TemporalObject::DateTimeUnits, int> getIANATimeZoneDateTimeParts(ExecutionState& state, const Value& epochNanoseconds);
    static int64_t getOffsetNanosecondsFor(ExecutionState& state, const Value& timeZone, const Value& instant);
    static Value builtinTimeZoneGetOffsetStringFor(ExecutionState& state, const Value& timeZone, const Value& instant);
    static Value builtinTimeZoneGetPlainDateTimeFor(ExecutionState& state, const Value& timeZone, const Value& instant, const Value& calendar);
    static Value getIANATimeZoneOffsetNanoseconds(ExecutionState& state, const Value& epochNanoseconds, const std::string& timeZoneIdentifier);
    static ValueVector getIANATimeZoneEpochValue(ExecutionState& state, const std::string& timeZoneIdentifier, int year, int month, int day, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond);
    static Value getIANATimeZoneNextTransition(ExecutionState& state, const Value& epochNanoseconds, std::string timeZoneIdentifier);
    static Value getIANATimeZonePreviousTransition(ExecutionState& state, const Value& epochNanoseconds, std::string timeZoneIdentifier);

    bool isTemporalTimeZoneObject() const override
    {
        return true;
    }

    const ASCIIString* getIdentifier() const
    {
        return m_identifier;
    }
    const Value& getOffsetNanoseconds() const
    {
        return m_offsetNanoseconds;
    }

private:
    static Value getIANATimeZoneTransition(ExecutionState& state, BigInt* rightNanos, BigInt* rightOffsetNs, BigInt* leftNanos, BigInt* leftOffsetNs, BigInt* cap, const std::string& timeZoneIdentifier);
    ASCIIString* m_identifier;
    Value m_offsetNanoseconds;
};

} // namespace Escargot

#endif
#endif
