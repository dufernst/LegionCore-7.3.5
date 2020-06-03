////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "WowTime.hpp"
// #include <my_getopt.h>

using namespace MS::Utilities;

WowTime::WowTime()
{
    Minute = -1;
    Hour = -1;
    WeekDay = -1;
    MonthDay = -1;
    Month = -1;
    Year = -1;
    Unk1 = 0;
    YearDay = 0;
}

uint32 WowTime::Encode()
{
    return uint32((Minute & 63) | ((Hour & 31) << 6) | ((WeekDay & 7) << 11) | ((MonthDay & 63) << 14) | ((Month & 15) << 20) | ((Year & 31) << 24) | ((Unk1 & 3) << 29));
}

uint32 WowTime::Encode(time_t time)
{
    WowTime wTime;
    wTime.SetUTCTimeFromPosixTime(time);

    return wTime.Encode();
}

void WowTime::Decode(uint32 encodedTime)
{
    int temp = encodedTime & 0x3F;
    if (temp == 63)
        Minute = -1;
    else
        Minute = temp;

    temp = (encodedTime >> 6) & 0x1F;
    if (temp == 31)
        Hour = -1;
    else
        Hour = temp;

    temp = (encodedTime >> 11) & 7;
    if (temp == 7)
        WeekDay = -1;
    else
        WeekDay = temp;

    temp = (encodedTime >> 14) & 0x3F;
    if (temp == 63)
        MonthDay = -1;
    else
        MonthDay = temp;

    temp = (encodedTime >> 20) & 0xF;
    if (temp == 15)
        Month = -1;
    else
        Month = temp;

    temp = (encodedTime >> 24) & 0x1F;
    if (temp == 31)
        Year = -1;
    else
        Year = temp;

    temp = (encodedTime >> 29) & 3;
    if (temp == 3)
        Unk1 = -1;
    else
        Unk1 = temp;
}

WowTime WowTime::FromEncodedTime(uint32 encodedTime)
{
    WowTime wTime;
    wTime.Decode(encodedTime);

    return wTime;
}

void WowTime::AddDays(int32 count, bool keepHoursAndMinutes)
{
    if (Year >= 0 && Month >= 0 && MonthDay >= 0)
    {
        tm timeInfo{};
        timeInfo.tm_sec = 0;
        timeInfo.tm_year = Year + 100;
        timeInfo.tm_mon = Month;
        timeInfo.tm_mday = MonthDay + 1;
        timeInfo.tm_isdst = -1;

        time_t posixTime;
        if (keepHoursAndMinutes)
        {
            timeInfo.tm_hour = Hour;
            timeInfo.tm_min = Minute;

            posixTime = mktime(&timeInfo) + (Globals::InSeconds::Day * count);
        }
        else
            posixTime = mktime(&timeInfo) + (Globals::InSeconds::Day * count) + Globals::InSeconds::Hour;

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
        localtime_s(&timeInfo, &posixTime);
#else
        localtime_r(&posixTime, &timeInfo); // POSIX  
#endif

        Year = timeInfo.tm_year - 100;
        Month = timeInfo.tm_mon;
        MonthDay = timeInfo.tm_mday - 1;
        Hour = timeInfo.tm_hour;
        Minute = timeInfo.tm_min;
        WeekDay = timeInfo.tm_wday;
    }
}

void WowTime::AddMinutes(int32 count)
{
    auto totalMinutes = 0;
    if (Hour >= 0 && Minute >= 0)
        totalMinutes = (Globals::InMinutes::Hour * Hour) + Minute;

    auto remainMinutes = totalMinutes + count;
    if (totalMinutes + count + 1439 >= 2879)
    {
        auto dayCount = remainMinutes / Globals::InMinutes::Day;
        remainMinutes %= Globals::InMinutes::Day;

        if (Year >= 0 && Month >= 0 && MonthDay >= 0)
        {
            tm timeInfo{};
            timeInfo.tm_sec = 0LL;
            timeInfo.tm_year = Year + 100;
            timeInfo.tm_mon = Month;
            timeInfo.tm_mday = MonthDay + 1;
            timeInfo.tm_isdst = -1;

            auto wTime = (Globals::InSeconds::Day * dayCount) + mktime(&timeInfo) + Globals::InSeconds::Hour;

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            localtime_s(&timeInfo, &wTime);
#else
            localtime_r(&wTime, &timeInfo); // POSIX  
#endif

            Year = timeInfo.tm_year - 100;
            Month = timeInfo.tm_mon;
            MonthDay = timeInfo.tm_mday - 1;
            WeekDay = timeInfo.tm_wday;
        }
    }
    if (remainMinutes < 0)
    {
        if (Year >= 0 && Month >= 0 && MonthDay >= 0)
        {
            tm timeInfo{};
            timeInfo.tm_sec = 0LL;
            timeInfo.tm_year = Year + 100;
            timeInfo.tm_mon = Month;
            timeInfo.tm_mday = MonthDay + 1;
            timeInfo.tm_isdst = -1;

            auto wTime = mktime(&timeInfo) - (23 * Globals::InSeconds::Hour);

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            localtime_s(&timeInfo, &wTime);
#else
            localtime_r(&wTime, &timeInfo); // POSIX  
#endif

            Year = timeInfo.tm_year - 100;
            Month = timeInfo.tm_mon;
            MonthDay = timeInfo.tm_mday - 1;
            WeekDay = timeInfo.tm_wday;
        }

        remainMinutes += Globals::InMinutes::Day;
    }

    Hour = remainMinutes / Globals::InMinutes::Hour;
    Minute = remainMinutes % Globals::InMinutes::Hour;
}

void WowTime::AddHolidayDuration(int32 duration)
{
    if (duration >= Globals::InMinutes::Day)
        AddDays(duration / Globals::InMinutes::Day, true);

    AddMinutes(duration % Globals::InMinutes::Day);

    auto oldTotalMinutes = 0;
    if (Hour >= 0 && Minute >= 0)
        oldTotalMinutes = (Globals::InMinutes::Hour * Hour) + Minute;

    duration %= Globals::InMinutes::Day + Minute + Globals::InMinutes::Hour * Hour;
    int64 newTotalMinute = duration - Globals::InMinutes::Day * (((((-1240768329 * duration) >> 32) + duration) >> 31) + ((((-1240768329 * duration) >> 32) + duration) >> 10));

    if (newTotalMinute != oldTotalMinutes)
    {
        signed int unk = Globals::InMinutes::Day;

        if (Hour >= 0 && Minute >= 0)
            unk = Globals::InMinutes::Hour * Hour + Minute + Globals::InMinutes::Day;

        if ((unk - newTotalMinute) % Globals::InMinutes::Day == Globals::InMinutes::Hour)
        {
            if (Hour < 0 || Minute < 0 || ((Globals::InMinutes::Hour * Hour) + Minute) <= 59)
            {
                if (Year >= 0 && Month >= 0 && MonthDay >= 0)
                {
                    tm timeInfo{};
                    timeInfo.tm_sec = 0LL;
                    timeInfo.tm_year = Year + 100;
                    timeInfo.tm_mon = Month;
                    timeInfo.tm_mday = MonthDay + 1;
                    timeInfo.tm_isdst = -1;

                    auto wTime = mktime(&timeInfo) - 23 * Globals::InSeconds::Hour;

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
                    localtime_s(&timeInfo, &wTime);
#else
                    localtime_r(&wTime, &timeInfo); // POSIX  
#endif
                    Year = timeInfo.tm_year - 100;
                    Month = timeInfo.tm_mon;
                    MonthDay = timeInfo.tm_mday - 1;
                    WeekDay = timeInfo.tm_wday;
                }
            }

            Hour = newTotalMinute / Globals::InMinutes::Hour;
            Minute = newTotalMinute % Globals::InMinutes::Hour;
        }
        else
            AddMinutes(Globals::InMinutes::Hour);
    }
}

uint32 WowTime::GetDaysSinceEpoch()
{
    auto posixTime = GetPosixTimeFromUTC();
    if (!posixTime)
        return 0;

    return posixTime / Globals::InSeconds::Day;
}

time_t WowTime::GetPosixTimeFromUTC()
{
    if (Year >= 0 && Month >= 0 && MonthDay >= 0)
    {
        tm timeInfo{};
        timeInfo.tm_sec = 0;
        timeInfo.tm_year = Year + 100;
        timeInfo.tm_mon = Month;
        timeInfo.tm_mday = MonthDay + 1;

        if (Hour >= 0)
        {
            timeInfo.tm_hour = Hour;

            if (Minute >= 0)
                timeInfo.tm_min = Minute;
        }

        timeInfo.tm_isdst = -1;
        timeInfo.tm_wday = WeekDay;

#ifdef _MSC_VER
        auto wTime = _mkgmtime(&timeInfo);
#else
        auto wTime = timegm(&timeInfo);
#endif
        return wTime;
    }

    return 0;
}

time_t WowTime::GetPosixTime() const
{
    tm timeInfo{};
    timeInfo.tm_sec = 0LL;
    timeInfo.tm_year = Year + 100;
    timeInfo.tm_mon = Month;
    timeInfo.tm_mday = MonthDay + 1;
    timeInfo.tm_hour = Hour;
    timeInfo.tm_min = Minute;
    timeInfo.tm_isdst = -1;

    return mktime(&timeInfo);
}

time_t WowTime::GetHourAndMinutes()
{
    return Hour * Globals::InMinutes::Hour + Minute;
}

void WowTime::SetUTCTimeFromPosixTime(time_t posixTime)
{
    tm timeInfo{};

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
    gmtime_s(&timeInfo, &posixTime);
#else
    gmtime_r(&posixTime, &timeInfo); // POSIX  
#endif

    Year = timeInfo.tm_year - 100;
    Month = timeInfo.tm_mon;
    MonthDay = timeInfo.tm_mday - 1;
    Hour = timeInfo.tm_hour;
    Minute = timeInfo.tm_min;
    WeekDay = timeInfo.tm_wday;
}

void WowTime::SetHourAndMinutes(uint32 minutes)
{
    Hour = minutes / Globals::InMinutes::Hour;
    Minute = minutes % Globals::InMinutes::Hour;
}

void WowTime::SetHourAndMinutes(uint32 hours, uint32 minutes)
{
    if (hours <= 23 && minutes <= 59)
    {
        Hour = hours;
        Minute = minutes;
    }
}

bool WowTime::SetDate(uint32 month, uint32 day, uint32 year)
{
    if (month <= 11 && day <= 31)
    {
        auto l_Year = year - 2000;
        if (year <= 1999)
            l_Year = year;

        if (l_Year <= 31)
        {
            Month = month;
            MonthDay = day;
            Year = year;
        }
    }

    return false;
}

time_t WowTime::DiffTime(WowTime const& other)
{
    return other.GetPosixTime() - GetPosixTime();
}

bool WowTime::IsSameDay(WowTime const& other)
{
    return !(other.Year != Year || other.Month != Month || other.MonthDay != MonthDay);
}

bool WowTime::InRange(WowTime const& left, WowTime const& right)
{
    bool l_Cond1 = left <= right;
    bool l_Cond2 = *this >= left;

    if (!l_Cond1)
    {
        if (l_Cond2)
            return true;

        return *this < right;
    }

    if (l_Cond2)
        return *this < right;

    return false;
}

void WowTime::ComputeRegionTime(WowTime& other)
{
    static auto holidayOffsetSeconds = 0;

    auto diffTimeFromRegionToUTC = []() -> time_t
    {
        auto localCurrentTime = time(nullptr);

        auto utcTimeInfo = gmtime(&localCurrentTime);
        utcTimeInfo->tm_isdst = -1;

        return mktime(utcTimeInfo) - localCurrentTime;
    };

    if (YearDay)
    {
        tm timeInfo{};

        if (holidayOffsetSeconds || ((holidayOffsetSeconds = Globals::InSeconds::Hour * YearDay - diffTimeFromRegionToUTC())))
        {
            timeInfo.tm_sec = 0LL;
            timeInfo.tm_year = other.Year + 100;
            timeInfo.tm_mon = other.Month;
            timeInfo.tm_mday = other.MonthDay + 1;
            timeInfo.tm_isdst = -1;
            timeInfo.tm_hour = other.Hour;
            timeInfo.tm_min = other.Minute;

            auto wTime = mktime(&timeInfo) + holidayOffsetSeconds;

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
            localtime_s(&timeInfo, &wTime);
#else
            localtime_r(&wTime, &timeInfo); // POSIX  
#endif
            other.Year = timeInfo.tm_year - 100;
            other.Month = timeInfo.tm_mon;
            other.MonthDay = timeInfo.tm_mday - 1;
            other.Hour = timeInfo.tm_hour;
            other.Minute = timeInfo.tm_min;
            other.WeekDay = -1;
        }
    }
}

bool WowTime::operator==(WowTime const& other) const
{
    return (other.Year < 0 || Year < 0 || other.Year == Year) && (other.Month < 0 || Month < 0 || other.Month == Month) && (other.MonthDay < 0 || MonthDay < 0 || other.MonthDay == MonthDay) && (other.WeekDay < 0 || WeekDay < 0 || other.WeekDay == WeekDay) && (other.Hour < 0 || Hour < 0 || other.Hour == Hour) && (other.Minute < 0 || Minute < 0 || other.Minute == Minute);
}

bool WowTime::operator!=(WowTime const& other) const
{
    return !(*this == other);
}

bool WowTime::operator<(WowTime const& other) const
{
    if (*this == other)
        return false;

    if (other.Year >= 0 && Year >= 0)
        return Year < other.Year;

    if (other.Month >= 0 && Month >= 0)
        return Month < other.Month;

    if (other.MonthDay >= 0 && MonthDay >= 0)
        return MonthDay < other.MonthDay;

    if (other.WeekDay >= 0 && WeekDay >= 0)
        return WeekDay < other.WeekDay;

    if (other.Hour >= 0 && Hour >= 0)
        return Hour < other.Hour;

    if (other.Minute < 0)
        return false;

    if (Minute >= 0)
        return Minute < other.Minute;

    return false;
}

bool WowTime::operator<=(WowTime const& other) const
{
    if (*this == other)
        return true;

    return *this < other;
}

bool WowTime::operator>(WowTime const& other) const
{
    if (*this == other)
        return false;

    return !(*this <= other);
}

bool WowTime::operator>=(WowTime const& other) const
{
    if (*this == other)
        return true;

    return !(*this < other);
}
