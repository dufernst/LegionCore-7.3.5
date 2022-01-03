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

#ifndef _UTIL_H
#define _UTIL_H

#include <algorithm>
#include <atomic>
#include <exception>
#include <iterator>
#include <vector>
#include <list>
#include <boost/container/static_vector.hpp>

#include "Define.h"
#include "Common.h"
#include "Log.h"
#include "Random.h"

template <class T>
class CheckedBufferOutputIterator
{
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using pointer = T*;
        using reference = T&;
        using difference_type = std::ptrdiff_t;

        CheckedBufferOutputIterator(T* buf, size_t n) : _buf(buf), _end(buf+n) {}

        T& operator*() const { check(); return *_buf; }
        CheckedBufferOutputIterator& operator++() { check(); ++_buf; return *this; }
        CheckedBufferOutputIterator operator++(int) { CheckedBufferOutputIterator v = *this; operator++(); return v; }

        size_t remaining() const { return (_end - _buf); }

    private:
        T* _buf;
        T* _end;
        void check() const
        {
            if (!(_buf < _end))
                throw std::out_of_range("index");
        }
};

template <class tValues, class tFlags, class tFlagType, uint8 tArraySize>
class FlaggedValuesArray
{
public:
    FlaggedValuesArray()
    {
        for (uint32 i = 0; i < tArraySize; ++i)
            _values[i] = tValues(0);
        _flags = 0;
    }

    tFlags GetFlags() const { return _flags; }
    bool HasFlag(tFlagType flag) const { return _flags & (SI64LIT(1) << flag); }
    void AddFlag(tFlagType flag) { _flags |= (SI64LIT(1) << flag); }
    void DelFlag(tFlagType flag) { _flags &= ~(SI64LIT(1) << flag); }

    tValues GetValue(tFlagType flag) const { return _values[flag]; }
    void SetValue(tFlagType flag, tValues value) { _values[flag] = value; }
    void AddValue(tFlagType flag, tValues value) { _values[flag] += value; }

private:
    tValues _values[tArraySize];
    tFlags _flags;
};

class Tokenizer
{
public:
    typedef std::vector<char const*> StorageType;
    typedef StorageType::size_type size_type;
    typedef StorageType::const_iterator const_iterator;
    typedef StorageType::reference reference;
    typedef StorageType::const_reference const_reference;

    Tokenizer(const std::string &src, char sep, uint32 vectorReserve = 0, bool keepEmptyStrings = true);
    ~Tokenizer() { delete[] m_str; }

    const_iterator begin() const { return m_storage.begin(); }
    const_iterator end() const { return m_storage.end(); }

    size_type size() const { return m_storage.size(); }
    bool empty() const { return m_storage.empty(); }

    reference operator [] (size_type i) { return m_storage[i]; }
    const_reference operator [] (size_type i) const { return m_storage[i]; }

    private:
    char* m_str;
    StorageType m_storage;
};

void stripLineInvisibleChars(std::string &str);

struct tm* localtime_r(const time_t* time, struct tm *result);

std::string secsToTimeString(uint64 timeInSecs, bool shortText = false, bool hoursOnly = false);
uint32 TimeStringToSecs(const std::string& timestring);
std::string TimeToTimestampStr(time_t t);

void ApplyPercentModFloatVar(float& var, float val, bool apply);
int32 RoundingFloatValue(float val);

// Percentage calculation
template <class T, class U>
T CalculatePct(T base, U pct)
{
    return T(base * static_cast<float>(pct) / 100.0f);
}

template <class T, class U>
T AddPct(T &base, U pct)
{
    return base += CalculatePct(base, pct);
}

template <class T, class U>
T ApplyPct(T &base, U pct)
{
    return base = CalculatePct(base, pct);
}

template <class T>
T RoundToInterval(T& num, T floor, T ceil)
{
    return num = std::min(std::max(num, floor), ceil);
}

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr);
bool Utf8toWStr(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize);
bool Utf8toWStr(const std::string& utf8str, wchar_t* wstr, size_t& wsize);
bool WStrToUtf8(const std::wstring& wstr, std::string& utf8str);
bool WStrToUtf8(wchar_t* wstr, size_t size, std::string& utf8str);
size_t utf8length(std::string& utf8str);
void utf8truncate(std::string& utf8str, size_t len);

bool isBasicLatinCharacter(wchar_t wchar);
bool isExtendedLatinCharacter(wchar_t wchar);
bool isCyrillicCharacter(wchar_t wchar);
bool isEastAsianCharacter(wchar_t wchar);
bool isNumeric(wchar_t wchar);
bool isNumeric(char c);
bool isNumeric(char const* str);
bool isNumericOrSpace(wchar_t wchar);
bool isBasicLatinString(const std::wstring& wstr, bool numericOrSpace);
bool isExtendedLatinString(const std::wstring& wstr, bool numericOrSpace);
bool isCyrillicString(const std::wstring& wstr, bool numericOrSpace);
bool isEastAsianString(const std::wstring& wstr, bool numericOrSpace);
wchar_t wcharToUpper(wchar_t wchar);
wchar_t wcharToUpperOnlyLatin(wchar_t wchar);
wchar_t wcharToLower(wchar_t wchar);
void wstrToUpper(std::wstring& str);
void wstrToLower(std::wstring& str);

std::wstring GetMainPartOfName(std::wstring wname, uint32 declension);

bool utf8ToConsole(const std::string& utf8str, std::string& conStr);
bool consoleToUtf8(const std::string& conStr, std::string& utf8str);
bool Utf8FitTo(const std::string& str, const std::wstring& search);
void utf8printf(FILE* out, const char *str, ...);
void vutf8printf(FILE* out, const char *str, va_list* ap);
bool Utf8ToUpperOnlyLatin(std::string& utf8String);

bool IsIPAddress(char const* ipaddress);

uint32 CreatePIDFile(std::string const& filename);
uint32 GetPID();

std::string ByteArrayToHexStr(uint8 const* bytes, uint32 length, bool reverse = false);
void HexStrToByteArray(std::string const& str, uint8* out, bool reverse = false);

extern std::atomic<bool> m_stopEvent;
extern bool m_worldCrashChecker;
uint64 GetThreadID();

#endif

//handler for operations on large flags
#ifndef _FLAG128
#define _FLAG128

#ifndef PAIR64_HIPART
#define PAIR64_HIPART(x)   (uint32)((uint64(x) >> 32) & UI64LIT(0x00000000FFFFFFFF))
#define PAIR64_LOPART(x)   (uint32)(uint64(x)         & UI64LIT(0x00000000FFFFFFFF))
#endif

// simple class for not-modifiable list
template <typename T>
class HookList final
{
    typedef std::vector<T> ContainerType;
    ContainerType _container;

public:
    typedef typename ContainerType::const_iterator const_iterator;
    typedef typename ContainerType::iterator iterator;

    HookList<T>& operator+=(T&& t)
    {
        _container.push_back(std::move(t));
        return *this;
    }

    size_t size() const
    {
        return _container.size();
    }

    iterator begin()
    {
        return _container.begin();
    }

    iterator end()
    {
        return _container.end();
    }

    const_iterator begin() const
    {
        return _container.begin();
    }

    const_iterator end() const
    {
        return _container.end();
    }
};

class flag128
{
    uint32 part[4];
public:
    flag128(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0, uint32 p4 = 0);
    flag128(uint64 p1, uint64 p2);
    flag128(const flag128& right);

    bool IsEqual(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0, uint32 p4 = 0) const;
    bool HasFlag(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0, uint32 p4 = 0) const;;
    void Set(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0, uint32 p4 = 0);

    bool operator <(const flag128& right) const;
    bool operator ==(const flag128& right) const;
    bool operator !=(const flag128& right) const;
    flag128& operator =(const flag128& right);
    flag128 operator &(const flag128& right) const;
    flag128& operator &=(const flag128& right);
    flag128 operator |(const flag128& right) const;
    flag128& operator |=(const flag128& right);
    flag128 operator ~() const;
    flag128 operator ^(const flag128& right) const;
    flag128& operator ^=(const flag128& right);
    operator bool() const;
    bool operator !() const;
    uint32& operator [](uint8 el);
    const uint32& operator [](uint8 el) const;
};

template <class X>
class cyber_ptr
{

public:
    struct coun
    {
    coun() : counter(0) {}
        std::atomic<int> counter;
        bool ready = false;
    };

    //! Init from main class
    cyber_ptr(X* p)
    {
        ASSERT(p && "Trying create class with null object. Bad initialization.");
        InitParent(p);
    }

    //! Child creation
    cyber_ptr(coun* c, X* p) : numerator(c), ptr(p)
    {
    }

    //! Copy
    cyber_ptr(const cyber_ptr<X> &right)
    {
        numerator = right.numerator;
        ptr = right.ptr;
        incrase();
    }

    //! null init
    cyber_ptr()
    {}

    virtual ~cyber_ptr()
    {
        // only for initiated objects
        if (numerator)
        {
            // unlink ptr object from childs.
            if (parent)
                numerator->ready = false;

            --numerator->counter;
            // if all links already deleted - clean numerator from memory.
            if (!numerator->counter.load())
                delete numerator;
        }
    }

    //! Get ptr object
    X* get()
    {
        if (!numerator || !numerator->ready)
            return NULL;
        return ptr;
    }

    //! Init new parent ptr
    void InitParent(X* object)
    {
        //This shouldn't happend.
        if (ptr)
        {
            if (numerator)
            {
                if (numerator->ready)
                    return;
            }

        }
        //ASSERT(!ptr && "Already initiated");

        ptr = object;

        numerator = new coun();
        numerator->counter = 1;
        numerator->ready = true;

        parent = true;
    }

    //! create child and link with main class
    cyber_ptr<X> shared_from_this()
    {
        incrase();
        return cyber_ptr<X>(numerator, ptr);
    }

    //! increase number copy  of our ptr
    void incrase()
    {
        if (numerator)
            ++numerator->counter;
    }
    bool isParent() const { return parent; }

    //- operators
    cyber_ptr<X>& operator=(const cyber_ptr<X>& right) // copy assignment
    {
        //if (this != &right)
        {
            numerator = right.numerator;
            ptr = right.ptr;
            incrase();
        }
        return *this;
    }

    cyber_ptr<X>& operator=(cyber_ptr<X>&& right) noexcept
    // move assignment
    {
        //if (this != &right)
        {
            numerator = right.numerator;
            ptr = right.ptr;
            incrase();
        }
        return *this;
    }


    coun *numerator = NULL;
    X* ptr = NULL;
private:
    bool parent = false;
};

template<typename T, std::size_t N>
class Array
{
    typedef boost::container::static_vector<T, N> storage_type;
    typedef std::integral_constant<std::size_t, N> max_capacity;

    typedef typename storage_type::value_type value_type;
    typedef typename storage_type::size_type size_type;
    typedef typename storage_type::reference reference;
    typedef typename storage_type::const_reference const_reference;
    typedef typename storage_type::iterator iterator;
    typedef typename storage_type::const_iterator const_iterator;

public:
    Array() { }

    iterator begin() { return _storage.begin(); }
    const_iterator begin() const { return _storage.begin(); }

    iterator end() { return _storage.end(); }
    const_iterator end() const { return _storage.end(); }

    size_type size() const { return _storage.size(); }
    bool empty() const { return _storage.empty(); }

    reference operator[](size_type i) { return _storage[i]; }
    const_reference operator[](size_type i) const { return _storage[i]; }

    void resize(size_type newSize)
    {
        if (newSize < max_capacity::value)
            _storage.resize(newSize);
    }

    void push_back(value_type const& value)
    {
        if (_storage.size() < max_capacity::value)
            _storage.push_back(value);
    }

    void push_back(value_type&& value)
    {
        if (_storage.size() < max_capacity::value)
            _storage.push_back(std::forward<value_type>(value));
    }

private:
    storage_type _storage;
};

template<typename E>
typename std::underlying_type<E>::type AsUnderlyingType(E enumValue)
{
    static_assert(std::is_enum<E>::value, "AsUnderlyingType can only be used with enums");
    return static_cast<typename std::underlying_type<E>::type>(enumValue);
}

#endif
