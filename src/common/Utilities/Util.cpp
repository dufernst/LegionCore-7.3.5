/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "Util.h"
#include "Common.h"
#include "CompilerDefs.h"
#include "utf8.h"
#include "Errors.h" // for ASSERT
#include <cstdarg>
#include "StringFormat.h"
#include <boost/asio/ip/address.hpp>

#if COMPILER == COMPILER_GNU
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

bool m_worldCrashChecker;
std::atomic<bool> m_stopEvent;

uint64 GetThreadID()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoll(ss.str());
}

Tokenizer::Tokenizer(const std::string &src, const char sep, uint32 vectorReserve /*= 0*/, bool keepEmptyStrings /*= true*/)
{
    m_str = new char[src.length() + 1];
    memcpy(m_str, src.c_str(), src.length() + 1);

    if (vectorReserve)
        m_storage.reserve(vectorReserve);

    char* posold = m_str;
    char* posnew = m_str;

    for (;;)
    {
        if (*posnew == sep)
        {
            if (keepEmptyStrings || posold != posnew)
                m_storage.push_back(posold);

            posold = posnew + 1;
            *posnew = '\0';
        }
        else if (*posnew == '\0')
        {
            // Hack like, but the old code accepted these kind of broken strings,
            // so changing it would break other things
            if (posold != posnew)
                m_storage.push_back(posold);

            break;
        }

        ++posnew;
    }
}

void stripLineInvisibleChars(std::string &str)
{
    static std::string const invChars(" \t\7\n");

    size_t wpos = 0;

    bool space = false;
    for (size_t pos = 0; pos < str.size(); ++pos)
    {
        if (invChars.find(str[pos]) != std::string::npos)
        {
            if (!space)
            {
                str[wpos++] = ' ';
                space = true;
            }
        }
        else
        {
            if (wpos != pos)
                str[wpos++] = str[pos];
            else
                ++wpos;
            space = false;
        }
    }

    if (wpos < str.size())
        str.erase(wpos, str.size());
    if (str.find("|TInterface") != std::string::npos)
        str.clear();

}

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
struct tm* localtime_r(const time_t* time, struct tm *result)
{
    localtime_s(result, time);
    return result;
}

#endif

std::string secsToTimeString(uint64 timeInSecs, bool shortText, bool hoursOnly)
{
    uint64 secs = timeInSecs % MINUTE;
    uint64 minutes = timeInSecs % HOUR / MINUTE;
    uint64 hours = timeInSecs % DAY / HOUR;
    uint64 days = timeInSecs / DAY;

    std::ostringstream ss;
    if (days)
        ss << days << (shortText ? "d" : " Day(s) ");
    if (hours || hoursOnly)
        ss << hours << (shortText ? "h" : " Hour(s) ");
    if (!hoursOnly)
    {
        if (minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if (secs || (!days && !hours && !minutes))
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

uint32 TimeStringToSecs(const std::string& timestring)
{
    uint32 secs = 0;
    uint32 buffer = 0;
    uint32 multiplier = 0;

    for (auto itr : timestring)
    {
        if (isdigit(itr))
        {
            buffer *= 10;
            buffer += itr - '0';
        }
        else
        {
            switch (itr)
            {
            case 'd': multiplier = DAY;     break;
            case 'h': multiplier = HOUR;    break;
            case 'm': multiplier = MINUTE;  break;
            case 's': multiplier = 1;       break;
            default: return 0;                         //bad format
            }
            buffer *= multiplier;
            secs += buffer;
            buffer = 0;
        }
    }

    return secs;
}

std::string TimeToTimestampStr(time_t t)
{
    tm aTm;
    localtime_r(&t, &aTm);

    try
    {
        return Trinity::StringFormat("%04d-%02d-%02d_%02d-%02d-%02d",aTm.tm_year + 1900, aTm.tm_mon + 1, aTm.tm_mday, aTm.tm_hour, aTm.tm_min, aTm.tm_sec);
    }
    catch (std::exception const& ex)
    {
        fprintf(stderr, "Failed to initialize timestamp part in TimeToTimestampStr %s", ex.what());
        fflush(stderr);
        ABORT();
    }
}

void ApplyPercentModFloatVar(float& var, float val, bool apply)
{
    if (val <= -100.0f) // prevent set var to zero
        val = -99.99f;

    var *= (apply ? (100.0f + val) / 100.0f : 100.0f / (100.0f + val));
}

int32 RoundingFloatValue(float val)
{
    int32 intVal = val;
    float difference = val - intVal;
    if (difference >= 0.44444445f)
        intVal++;

    return intVal;
}

/// Check if the string is a valid ip address representation
bool IsIPAddress(char const* ipaddress)
{
    if (!ipaddress)
        return false;

    boost::system::error_code error;
    boost::asio::ip::address::from_string(ipaddress, error);
    return !error;
}

uint32 CreatePIDFile(std::string const& filename)
{
    FILE* pid_file = fopen(filename.c_str(), "w");
    if (pid_file == nullptr)
        return 0;

    uint32 pid = GetPID();

    fprintf(pid_file, "%u", pid);
    fclose(pid_file);

    return pid;
}

uint32 GetPID()
{
#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif

    return uint32(pid);
}

size_t utf8length(std::string& utf8str)
{
    try
    {
        return utf8::distance(utf8str.c_str(), utf8str.c_str() + utf8str.size());
    }
    catch (std::exception&)
    {
        utf8str = "";
        return 0;
    }
}

void utf8truncate(std::string& utf8str, size_t len)
{
    try
    {
        size_t wlen = utf8::distance(utf8str.c_str(), utf8str.c_str() + utf8str.size());
        if (wlen <= len)
            return;

        std::wstring wstr;
        wstr.resize(wlen);
        utf8::utf8to16(utf8str.c_str(), utf8str.c_str() + utf8str.size(), &wstr[0]);
        wstr.resize(len);
        char* oend = utf8::utf16to8(wstr.c_str(), wstr.c_str() + wstr.size(), &utf8str[0]);
        utf8str.resize(oend - (&utf8str[0]));                 // remove unused tail
    }
    catch (std::exception&)
    {
        utf8str = "";
    }
}

bool isBasicLatinCharacter(wchar_t wchar)
{
    if (wchar >= L'a' && wchar <= L'z') // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
        return true;
    if (wchar >= L'A' && wchar <= L'Z') // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
        return true;
    return false;
}

bool isExtendedLatinCharacter(wchar_t wchar)
{
    if (isBasicLatinCharacter(wchar))
        return true;
    if (wchar >= 0x00C0 && wchar <= 0x00D6) // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
        return true;
    if (wchar >= 0x00D8 && wchar <= 0x00DF) // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
        return true;
    if (wchar == 0x00DF) // LATIN SMALL LETTER SHARP S
        return true;
    if (wchar >= 0x00E0 && wchar <= 0x00F6) // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
        return true;
    if (wchar >= 0x00F8 && wchar <= 0x00FE) // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
        return true;
    if (wchar >= 0x0100 && wchar <= 0x012F) // LATIN CAPITAL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK
        return true;
    if (wchar == 0x1E9E) // LATIN CAPITAL LETTER SHARP S
        return true;
    return false;
}

bool isCyrillicCharacter(wchar_t wchar)
{
    if (wchar >= 0x0410 && wchar <= 0x044F) // CYRILLIC CAPITAL LETTER A - CYRILLIC SMALL LETTER YA
        return true;
    if (wchar == 0x0401 || wchar == 0x0451) // CYRILLIC CAPITAL LETTER IO, CYRILLIC SMALL LETTER IO
        return true;
    return false;
}

bool isEastAsianCharacter(wchar_t wchar)
{
    if (wchar >= 0x1100 && wchar <= 0x11F9) // Hangul Jamo
        return true;
    if (wchar >= 0x3041 && wchar <= 0x30FF) // Hiragana + Katakana
        return true;
    if (wchar >= 0x3131 && wchar <= 0x318E) // Hangul Compatibility Jamo
        return true;
    if (wchar >= 0x31F0 && wchar <= 0x31FF) // Katakana Phonetic Ext.
        return true;
    if (wchar >= 0x3400 && wchar <= 0x4DB5) // CJK Ideographs Ext. A
        return true;
    if (wchar >= 0x4E00 && wchar <= 0x9FC3) // Unified CJK Ideographs
        return true;
    if (wchar >= 0xAC00 && wchar <= 0xD7A3) // Hangul Syllables
        return true;
    if (wchar >= 0xFF01 && wchar <= 0xFFEE) // Halfwidth forms
        return true;
    return false;
}

bool isNumeric(wchar_t wchar)
{
    return wchar >= L'0' && wchar <= L'9';
}

bool isNumeric(char c)
{
    return c >= '0' && c <= '9';
}

bool isNumeric(char const* str)
{
    for (char const* c = str; *c; ++c)
        if (!isNumeric(*c))
            return false;

    return true;
}

bool isNumericOrSpace(wchar_t wchar)
{
    return isNumeric(wchar) || wchar == L' ';
}

bool isBasicLatinString(const std::wstring& wstr, bool numericOrSpace)
{
    for (wchar_t i : wstr)
        if (!isBasicLatinCharacter(i) && (!numericOrSpace || !isNumericOrSpace(i)))
            return false;
    return true;
}

bool isExtendedLatinString(const std::wstring& wstr, bool numericOrSpace)
{
    for (wchar_t i : wstr)
        if (!isExtendedLatinCharacter(i) && (!numericOrSpace || !isNumericOrSpace(i)))
            return false;
    return true;
}

bool isCyrillicString(const std::wstring& wstr, bool numericOrSpace)
{
    for (wchar_t i : wstr)
        if (!isCyrillicCharacter(i) && (!numericOrSpace || !isNumericOrSpace(i)))
            return false;
    return true;
}

bool isEastAsianString(const std::wstring& wstr, bool numericOrSpace)
{
    for (wchar_t i : wstr)
        if (!isEastAsianCharacter(i) && (!numericOrSpace || !isNumericOrSpace(i)))
            return false;
    return true;
}

wchar_t wcharToUpper(wchar_t wchar)
{
    if (wchar >= L'a' && wchar <= L'z') // LATIN SMALL LETTER A - LATIN SMALL LETTER Z
        return wchar_t(uint16(wchar) - 0x0020);
    if (wchar == 0x00DF) // LATIN SMALL LETTER SHARP S
        return wchar_t(0x1E9E);
    if (wchar >= 0x00E0 && wchar <= 0x00F6) // LATIN SMALL LETTER A WITH GRAVE - LATIN SMALL LETTER O WITH DIAERESIS
        return wchar_t(uint16(wchar) - 0x0020);
    if (wchar >= 0x00F8 && wchar <= 0x00FE) // LATIN SMALL LETTER O WITH STROKE - LATIN SMALL LETTER THORN
        return wchar_t(uint16(wchar) - 0x0020);
    if (wchar >= 0x0101 && wchar <= 0x012F) // LATIN SMALL LETTER A WITH MACRON - LATIN SMALL LETTER I WITH OGONEK (only %2=1)
    {
        if (wchar % 2 == 1)
            return wchar_t(uint16(wchar) - 0x0001);
    }
    if (wchar >= 0x0430 && wchar <= 0x044F) // CYRILLIC SMALL LETTER A - CYRILLIC SMALL LETTER YA
        return wchar_t(uint16(wchar) - 0x0020);
    if (wchar == 0x0451) // CYRILLIC SMALL LETTER IO
        return wchar_t(0x0401);

    return wchar;
}

wchar_t wcharToUpperOnlyLatin(wchar_t wchar)
{
    return isBasicLatinCharacter(wchar) ? wcharToUpper(wchar) : wchar;
}

wchar_t wcharToLower(wchar_t wchar)
{
    if (wchar >= L'A' && wchar <= L'Z') // LATIN CAPITAL LETTER A - LATIN CAPITAL LETTER Z
        return wchar_t(uint16(wchar) + 0x0020);
    if (wchar >= 0x00C0 && wchar <= 0x00D6) // LATIN CAPITAL LETTER A WITH GRAVE - LATIN CAPITAL LETTER O WITH DIAERESIS
        return wchar_t(uint16(wchar) + 0x0020);
    if (wchar >= 0x00D8 && wchar <= 0x00DE) // LATIN CAPITAL LETTER O WITH STROKE - LATIN CAPITAL LETTER THORN
        return wchar_t(uint16(wchar) + 0x0020);
    if (wchar >= 0x0100 && wchar <= 0x012E) // LATIN CAPITAL LETTER A WITH MACRON - LATIN CAPITAL LETTER I WITH OGONEK (only %2=0)
    {
        if (wchar % 2 == 0)
            return wchar_t(uint16(wchar) + 0x0001);
    }
    if (wchar == 0x1E9E) // LATIN CAPITAL LETTER SHARP S
        return wchar_t(0x00DF);
    if (wchar == 0x0401) // CYRILLIC CAPITAL LETTER IO
        return wchar_t(0x0451);
    if (wchar >= 0x0410 && wchar <= 0x042F) // CYRILLIC CAPITAL LETTER A - CYRILLIC CAPITAL LETTER YA
        return wchar_t(uint16(wchar) + 0x0020);

    return wchar;
}

void wstrToUpper(std::wstring& str)
{
    std::transform(str.begin(), str.end(), str.begin(), wcharToUpper);
}

void wstrToLower(std::wstring& str)
{
    std::transform(str.begin(), str.end(), str.begin(), wcharToLower);
}

bool Utf8toWStr(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize)
{
    try
    {
        CheckedBufferOutputIterator<wchar_t> out(wstr, wsize);
        out = utf8::utf8to16(utf8str, utf8str+csize, out);
        wsize -= out.remaining(); // remaining unused space
        wstr[wsize] = L'\0';
    }
    catch (std::exception const&)
    {
        // Replace the converted string with an error message if there is enough space
        // Otherwise just return an empty string
        wchar_t const* errorMessage = L"An error occurred converting string from UTF-8 to WStr";
        size_t errorMessageLength = wcslen(errorMessage);
        if (wsize >= errorMessageLength)
        {
            wcscpy(wstr, errorMessage);
            wsize = wcslen(wstr);
        }
        else if (wsize > 0)
        {
            wstr[0] = L'\0';
            wsize = 0;
        }
        else
            wsize = 0;

        return false;
    }

    return true;
}


bool Utf8toWStr(const std::string& utf8str, wchar_t* wstr, size_t& wsize)
{
    return Utf8toWStr(utf8str.c_str(), utf8str.size(), wstr, wsize);
}

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr)
{
    wstr.clear();
    try
    {
        utf8::utf8to16(utf8str.c_str(), utf8str.c_str()+utf8str.size(), std::back_inserter(wstr));
    }
    catch (std::exception const&)
    {
        wstr.clear();
        return false;
    }

    return true;
}

bool WStrToUtf8(wchar_t* wstr, size_t size, std::string& utf8str)
{
    try
    {
        std::string utf8str2;
        utf8str2.resize(size * 4);                            // allocate for most long case

        if (size)
        {
            char* oend = utf8::utf16to8(wstr, wstr + size, &utf8str2[0]);
            utf8str2.resize(oend - (&utf8str2[0]));               // remove unused tail
        }
        utf8str = utf8str2;
    }
    catch (std::exception&)
    {
        utf8str = "";
        return false;
    }

    return true;
}

bool WStrToUtf8(const std::wstring& wstr, std::string& utf8str)
{
    try
    {
        std::string utf8str2;
        utf8str2.resize(wstr.size() * 4);                     // allocate for most long case

        if (!wstr.empty())
        {
            char* oend = utf8::utf16to8(wstr.c_str(), wstr.c_str() + wstr.size(), &utf8str2[0]);
            utf8str2.resize(oend - (&utf8str2[0]));                // remove unused tail
        }
        utf8str = utf8str2;
    }
    catch (std::exception&)
    {
        utf8str = "";
        return false;
    }

    return true;
}

std::wstring GetMainPartOfName(std::wstring wname, uint32 declension)
{
    // supported only Cyrillic cases
    if (wname.empty() || !isCyrillicCharacter(wname[0]) || declension > 5)
        return wname;

    // Important: end length must be <= MAX_INTERNAL_PLAYER_NAME-MAX_PLAYER_NAME (3 currently)

    static wchar_t const a_End[] = { wchar_t(1), wchar_t(0x0430), wchar_t(0x0000) };
    static wchar_t const o_End[] = { wchar_t(1), wchar_t(0x043E), wchar_t(0x0000) };
    static wchar_t const ya_End[] = { wchar_t(1), wchar_t(0x044F), wchar_t(0x0000) };
    static wchar_t const ie_End[] = { wchar_t(1), wchar_t(0x0435), wchar_t(0x0000) };
    static wchar_t const i_End[] = { wchar_t(1), wchar_t(0x0438), wchar_t(0x0000) };
    static wchar_t const yeru_End[] = { wchar_t(1), wchar_t(0x044B), wchar_t(0x0000) };
    static wchar_t const u_End[] = { wchar_t(1), wchar_t(0x0443), wchar_t(0x0000) };
    static wchar_t const yu_End[] = { wchar_t(1), wchar_t(0x044E), wchar_t(0x0000) };
    static wchar_t const oj_End[] = { wchar_t(2), wchar_t(0x043E), wchar_t(0x0439), wchar_t(0x0000) };
    static wchar_t const ie_j_End[] = { wchar_t(2), wchar_t(0x0435), wchar_t(0x0439), wchar_t(0x0000) };
    static wchar_t const io_j_End[] = { wchar_t(2), wchar_t(0x0451), wchar_t(0x0439), wchar_t(0x0000) };
    static wchar_t const o_m_End[] = { wchar_t(2), wchar_t(0x043E), wchar_t(0x043C), wchar_t(0x0000) };
    static wchar_t const io_m_End[] = { wchar_t(2), wchar_t(0x0451), wchar_t(0x043C), wchar_t(0x0000) };
    static wchar_t const ie_m_End[] = { wchar_t(2), wchar_t(0x0435), wchar_t(0x043C), wchar_t(0x0000) };
    static wchar_t const soft_End[] = { wchar_t(1), wchar_t(0x044C), wchar_t(0x0000) };
    static wchar_t const j_End[] = { wchar_t(1), wchar_t(0x0439), wchar_t(0x0000) };

    static wchar_t const* const dropEnds[6][8] =
    {
        { &a_End[1],  &o_End[1],    &ya_End[1],   &ie_End[1],  &soft_End[1], &j_End[1],    nullptr,     nullptr },
        { &a_End[1],  &ya_End[1],   &yeru_End[1], &i_End[1],   nullptr,      nullptr,      nullptr,     nullptr },
        { &ie_End[1], &u_End[1],    &yu_End[1],   &i_End[1],   nullptr,      nullptr,      nullptr,     nullptr },
        { &u_End[1],  &yu_End[1],   &o_End[1],    &ie_End[1],  &soft_End[1], &ya_End[1],   &a_End[1],   nullptr },
        { &oj_End[1], &io_j_End[1], &ie_j_End[1], &o_m_End[1], &io_m_End[1], &ie_m_End[1], &yu_End[1],  nullptr },
        { &ie_End[1], &i_End[1],    nullptr,      nullptr,     nullptr,      nullptr,      nullptr,     nullptr }
    };

    for (wchar_t const* const* itr = &dropEnds[declension][0]; *itr; ++itr)
    {
        size_t len = size_t((*itr)[-1]);                    // get length from string size field

        if (wname.substr(wname.size() - len, len) == *itr)
            return wname.substr(0, wname.size() - len);
    }

    return wname;
}

bool utf8ToConsole(const std::string& utf8str, std::string& conStr)
{
#if PLATFORM == TC_PLATFORM_WINDOWS
    std::wstring wstr;
    if (!Utf8toWStr(utf8str, wstr))
        return false;

    conStr.resize(wstr.size());
    CharToOemBuffW(&wstr[0], &conStr[0], wstr.size());
#else
    // not implemented yet
    conStr = utf8str;
#endif

    return true;
}

bool consoleToUtf8(const std::string& conStr, std::string& utf8str)
{
#if PLATFORM == TC_PLATFORM_WINDOWS
    std::wstring wstr;
    wstr.resize(conStr.size());
    OemToCharBuffW(&conStr[0], &wstr[0], conStr.size());

    return WStrToUtf8(wstr, utf8str);
#else
    // not implemented yet
    utf8str = conStr;
    return true;
#endif
}

bool Utf8FitTo(const std::string& str, const std::wstring& search)
{
    std::wstring temp;

    if (!Utf8toWStr(str, temp))
        return false;

    // converting to lower case
    wstrToLower(temp);

    return temp.find(search) != std::wstring::npos;
}

void utf8printf(FILE* out, const char *str, ...)
{
    va_list ap;
    va_start(ap, str);
    vutf8printf(out, str, &ap);
    va_end(ap);
}

void vutf8printf(FILE* out, const char *str, va_list* ap)
{
#if PLATFORM == TC_PLATFORM_WINDOWS
    char temp_buf[32 * 1024];
    wchar_t wtemp_buf[32 * 1024];

    size_t temp_len = vsnprintf(temp_buf, 32 * 1024, str, *ap);

    size_t wtemp_len = 32 * 1024 - 1;
    Utf8toWStr(temp_buf, temp_len, wtemp_buf, wtemp_len);

    CharToOemBuffW(&wtemp_buf[0], &temp_buf[0], wtemp_len + 1);
    fprintf(out, "%s", temp_buf);
#else
    vfprintf(out, str, *ap);
#endif
}

std::string ByteArrayToHexStr(uint8 const* bytes, uint32 arrayLen, bool reverse /* = false */)
{
    int32 init = 0;
    int32 end = arrayLen;
    int8 op = 1;

    if (reverse)
    {
        init = arrayLen - 1;
        end = -1;
        op = -1;
    }

    std::ostringstream ss;
    for (int32 i = init; i != end; i += op)
    {
        char buffer[4];
        sprintf(buffer, "%02X", bytes[i]);
        ss << buffer;
    }

    return ss.str();
}

bool Utf8ToUpperOnlyLatin(std::string& utf8String)
{
    std::wstring wstr;
    if (!Utf8toWStr(utf8String, wstr))
        return false;

    std::transform(wstr.begin(), wstr.end(), wstr.begin(), wcharToUpperOnlyLatin);

    return WStrToUtf8(wstr, utf8String);
}

void HexStrToByteArray(std::string const& str, uint8* out, bool reverse /*= false*/)
{
    // string must have even number of characters
    if (str.length() & 1)
        return;

    int32 init = 0;
    int32 end = str.length();
    int8 op = 1;

    if (reverse)
    {
        init = str.length() - 2;
        end = -2;
        op = -1;
    }

    uint32 j = 0;
    for (int32 i = init; i != end; i += 2 * op)
    {
        char buffer[3] = { str[i], str[i + 1], '\0' };
        out[j++] = strtoul(buffer, nullptr, 16);
    }
}

flag128::flag128(uint32 p1, uint32 p2, uint32 p3, uint32 p4)
{
    part[0] = p1;
    part[1] = p2;
    part[2] = p3;
    part[3] = p4;
}

flag128::flag128(uint64 p1, uint64 p2)
{
    part[0] = PAIR64_LOPART(p1);
    part[1] = PAIR64_HIPART(p1);
    part[2] = PAIR64_LOPART(p2);
    part[3] = PAIR64_HIPART(p2);
}

flag128::flag128(const flag128& right)
{
    part[0] = right.part[0];
    part[1] = right.part[1];
    part[2] = right.part[2];
    part[3] = right.part[3];
}

bool flag128::IsEqual(uint32 p1, uint32 p2, uint32 p3, uint32 p4) const
{
    return part[0] == p1 && part[1] == p2 && part[2] == p3 && part[3] == p4;
}

bool flag128::HasFlag(uint32 p1, uint32 p2, uint32 p3, uint32 p4) const
{
    return part[0] & p1 || part[1] & p2 || part[2] & p3 || part[3] & p4;
}

void flag128::Set(uint32 p1, uint32 p2, uint32 p3, uint32 p4)
{
    part[0] = p1;
    part[1] = p2;
    part[2] = p3;
    part[3] = p4;
}

bool flag128::operator<(const flag128& right) const
{
    for (uint8 i = 4; i > 0; --i)
    {
        if (part[i - 1] < right.part[i - 1])
            return true;
        if (part[i - 1] > right.part[i - 1])
            return false;
    }
    return false;
}

bool flag128::operator==(const flag128& right) const
{
    return part[0] == right.part[0] && part[1] == right.part[1] && part[2] == right.part[2] && part[3] == right.part[3];
}

bool flag128::operator!=(const flag128& right) const
{
    return !this->operator ==(right);
}

flag128& flag128::operator=(const flag128& right)
{
    part[0] = right.part[0];
    part[1] = right.part[1];
    part[2] = right.part[2];
    part[3] = right.part[3];
    return *this;
}

flag128 flag128::operator&(const flag128& right) const
{
    return flag128(part[0] & right.part[0], part[1] & right.part[1], part[2] & right.part[2], part[3] & right.part[3]);
}

flag128& flag128::operator&=(const flag128& right)
{
    part[0] &= right.part[0];
    part[1] &= right.part[1];
    part[2] &= right.part[2];
    part[3] &= right.part[3];
    return *this;
}

flag128 flag128::operator|(const flag128& right) const
{
    return flag128(part[0] | right.part[0], part[1] | right.part[1], part[2] | right.part[2], part[3] | right.part[3]);
}

flag128& flag128::operator|=(const flag128& right)
{
    part[0] |= right.part[0];
    part[1] |= right.part[1];
    part[2] |= right.part[2];
    part[3] |= right.part[3];
    return *this;
}

flag128 flag128::operator~() const
{
    return flag128(~part[0], ~part[1], ~part[2], ~part[3]);
}

flag128 flag128::operator^(const flag128& right) const
{
    return flag128(part[0] ^ right.part[0], part[1] ^ right.part[1], part[2] ^ right.part[2], part[3] ^ right.part[3]);
}

flag128& flag128::operator^=(const flag128& right)
{
    part[0] ^= right.part[0];
    part[1] ^= right.part[1];
    part[2] ^= right.part[2];
    part[3] ^= right.part[3];
    return *this;
}

flag128::operator bool() const
{
    return part[0] != 0 || part[1] != 0 || part[2] != 0 || part[3] != 0;
}

bool flag128::operator!() const
{
    return !this->operator bool();
}

uint32& flag128::operator[](uint8 el)
{
    return part[el];
}

const uint32& flag128::operator[](uint8 el) const
{
    return part[el];
}
