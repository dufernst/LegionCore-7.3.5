/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#include "ByteBuffer.h"
#include "Errors.h"
#include "MessageBuffer.h"
#include "Log.h"
#include "Util.h"
#include <sstream>
#include <ctime>
#include <utf8.h>

ByteBufferException::~ByteBufferException() noexcept = default;

char const* ByteBufferException::what() const noexcept
{
    return msg_.c_str();
}

std::string& ByteBufferException::message() noexcept
{
    return msg_;
}

ByteBufferInvalidValueException::ByteBufferInvalidValueException(char const* type, size_t pos)
{
    message().assign(Trinity::StringFormat("Invalid %s value found in ByteBuffer at pos " SZFMTD, type, pos));
}

ByteBuffer::ByteBuffer(MessageBuffer&& buffer) : _bitpos(InitialBitPos), _storage(buffer.Move())
{
}

std::vector<uint8>&& ByteBuffer::Move() noexcept
{
    _rpos = 0;
    _wpos = 0;
    _bitpos = InitialBitPos;
    _curbitval = 0;
    return std::move(_storage);
}

ByteBuffer& ByteBuffer::operator=(ByteBuffer const& right)
{
    if (this != &right)
    {
        _rpos = right._rpos;
        _wpos = right._wpos;
        _bitpos = right._bitpos;
        _curbitval = right._curbitval;
        _storage = right._storage;
    }
    return *this;
}

ByteBuffer& ByteBuffer::operator=(ByteBuffer&& right) noexcept
{
    if (this != &right)
    {
        _rpos = right._rpos;
        _wpos = right._wpos;
        _bitpos = right._bitpos;
        _curbitval = right._curbitval;
        _storage = right.Move();
    }
    return *this;
}

ByteBuffer::~ByteBuffer() = default;

void ByteBuffer::clear()
{
    _storage.clear();
    _rpos = 0;
    _wpos = 0;
    _bitpos = InitialBitPos;
    _curbitval = 0;
}

void ByteBuffer::FlushBits()
{
    if (_bitpos == 8)
        return;
    _bitpos = 8;
    append(static_cast<uint8 *>(&_curbitval), sizeof(uint8));
    _curbitval = 0;
}

void ByteBuffer::ResetBitPos()
{
    if (_bitpos > 7)
        return;
    _bitpos = InitialBitPos;
    _curbitval = 0;
}

void ByteBuffer::ResetBitReader()
{
    if (_bitpos == InitialBitPos)
        return;
    _curbitval = 0;
    _bitpos = InitialBitPos;
}

bool ByteBuffer::WriteBit(bool bit)
{
    --_bitpos;
    if (bit)
        _curbitval |= (1 << (_bitpos));
    if (_bitpos == 0)
    {
        _bitpos = InitialBitPos;
        append(reinterpret_cast<uint8 *>(&_curbitval), sizeof(_curbitval));
        _curbitval = 0;
    }
    return bit;
}

bool ByteBuffer::ReadBit()
{
    ++_bitpos;
    if (_bitpos > 7)
    {
        _curbitval = read<uint8>();
        _bitpos = 0;
    }
    return ((_curbitval >> (7 - _bitpos)) & 1) != 0;
}

void ByteBuffer::WriteBits(std::size_t value, int32 bits)
{
    for (int32 i = bits - 1; i >= 0; --i)
        WriteBit((value >> i) & 1);
}

uint32 ByteBuffer::ReadBits(int32 bits)
{
    uint32 value = 0;
    for (int32 i = bits - 1; i >= 0; --i)
        if (ReadBit())
            value |= (1 << (i));
    return value;
}

ByteBuffer& ByteBuffer::operator<<(uint8 value)
{
    append<uint8>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint16 value)
{
    append<uint16>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint32 value)
{
    append<uint32>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(uint64 value)
{
    append<uint64>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(std::chrono::milliseconds value)
{
    append<uint32>(value.count());
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(std::chrono::seconds value)
{
    append<uint32>(value.count());
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(int8 value)
{
    append<int8>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(int16 value)
{
    append<int16>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(int32 value)
{
    append<int32>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(int64 value)
{
    append<int64>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(float value)
{
    append<float>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(double value)
{
    append<double>(value);
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(const std::string& value)
{
    if (size_t len = value.length())
        append(reinterpret_cast<uint8 const*>(value.c_str()), len);
    append(static_cast<uint8>(0));
    return *this;
}

ByteBuffer& ByteBuffer::operator<<(const char* str)
{
    if (size_t len = (str ? strlen(str) : 0))
        append(reinterpret_cast<uint8 const*>(str), len);
    append(static_cast<uint8>(0));
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (uint8& value)
{
    value = read<uint8>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (uint16& value)
{
    value = read<uint16>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (uint32& value)
{
    value = read<uint32>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (uint64& value)
{
    value = read<uint64>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (int8& value)
{
    value = read<int8>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (int16& value)
{
    value = read<int16>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (int32& value)
{
    value = read<int32>();
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (int64& value)
{
    value = read<int64>();
    return *this;
}

ByteBufferPositionException::ByteBufferPositionException(size_t pos, size_t size, size_t valueSize)
{
    std::ostringstream ss;
    ss << "Attempted to get value with size: " << valueSize << " in ByteBuffer (pos: " << pos << " size: " << size << ")";
    message().assign(ss.str());
}

ByteBufferPositionException::~ByteBufferPositionException() noexcept = default;

ByteBuffer::ByteBuffer() : _bitpos(InitialBitPos)
{
    _storage.reserve(DEFAULT_SIZE);
}

ByteBuffer::ByteBuffer(size_t reserve) : _bitpos(InitialBitPos)
{
    _storage.reserve(reserve);
}

ByteBuffer::ByteBuffer(ByteBuffer&& buf) noexcept : _rpos(buf._rpos), _wpos(buf._wpos), _bitpos(buf._bitpos), _curbitval(buf._curbitval), _storage(buf.Move()) { }

ByteBuffer::ByteBuffer(ByteBuffer const& right) = default;

ByteBuffer& ByteBuffer::operator >> (float& value)
{
    value = read<float>();
    if (!std::isfinite(value))
        throw ByteBufferInvalidValueException("float", _rpos - sizeof(float));
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (double& value)
{
    value = read<double>();
    if (!std::isfinite(value))
        throw ByteBufferInvalidValueException("double", _rpos - sizeof(double));
    return *this;
}

ByteBuffer& ByteBuffer::operator >> (std::string& value)
{
    value = ReadCString(true);
    return *this;
}

uint8& ByteBuffer::operator[](size_t const pos)
{
    if (pos >= size())
        throw ByteBufferPositionException(pos, 1, size());
    return _storage[pos];
}

uint8 const& ByteBuffer::operator[](size_t const pos) const
{
    if (pos >= size())
        throw ByteBufferPositionException(pos, 1, size());
    return _storage[pos];
}

size_t ByteBuffer::rpos() const
{
    return _rpos;
}

size_t ByteBuffer::rpos(size_t rpos_)
{
    _rpos = rpos_;
    return _rpos;
}

void ByteBuffer::rfinish()
{
    _rpos = wpos();
}

size_t ByteBuffer::wpos() const
{
    return _wpos;
}

size_t ByteBuffer::wpos(size_t wpos_)
{
    _wpos = wpos_;
    return _wpos;
}

size_t ByteBuffer::bitwpos() const
{
    return _wpos * 8 + 8 - _bitpos;
}

size_t ByteBuffer::bitwpos(size_t newPos)
{
    _wpos = newPos / 8;
    _bitpos = 8 - (newPos % 8);
    return _wpos * 8 + 8 - _bitpos;
}

void ByteBuffer::read(uint8* dest, size_t len)
{
    if (_rpos + len > size())
        throw ByteBufferPositionException(_rpos, len, size());
    ResetBitPos();
    std::memcpy(dest, &_storage[_rpos], len);
    _rpos += len;
}

void ByteBuffer::ReadPackedUInt64(uint64& guid)
{
    guid = 0;
    ReadPackedUInt64(read<uint8>(), guid);
}

void ByteBuffer::ReadPackedUInt64(uint8 mask, uint64& value)
{
    for (uint32 i = 0; i < 8; ++i)
        if (mask & (uint8(1) << i))
            value |= (uint64(read<uint8>()) << (i * 8));
}

std::string ByteBuffer::ReadCString(bool requireValidUtf8 /*= true*/)
{
    std::string value;
    while (rpos() < size())                         // prevent crash at wrong string format in packet
    {
        char c = read<char>();
        if (c == 0)
            break;
        value += c;
    }
    if (requireValidUtf8 && !utf8::is_valid(value.begin(), value.end()))
        throw ByteBufferInvalidValueException("string", _rpos - value.length() - 1);
    return value;
}

std::string ByteBuffer::ReadString(uint32 length, bool requireValidUtf8 /*= true*/)
{
    if (_rpos + length > size())
        throw ByteBufferPositionException(_rpos, length, size());

    ResetBitPos();
    if (!length)
        return std::string();

    std::string value(reinterpret_cast<char const*>(&_storage[_rpos]), length);
    _rpos += length;
    if (requireValidUtf8 && !utf8::is_valid(value.begin(), value.end()))
        throw ByteBufferInvalidValueException("string", _rpos - value.length() - 1);
    return value;
}

void ByteBuffer::WriteString(std::string const& str)
{
    FlushBits();
    if (size_t len = str.length())
        append(str.c_str(), len);
}

void ByteBuffer::WriteString(std::string const& str, uint8 strLen)
{
    WriteBits(str.length(), strLen);
    FlushBits();
    if (size_t len = str.length())
        append(str.c_str(), len);
}

uint32 ByteBuffer::ReadPackedTime()
{
    auto packedDate = read<uint32>();
    tm lt = tm();

    lt.tm_min = packedDate & 0x3F;
    lt.tm_hour = (packedDate >> 6) & 0x1F;
    //lt.tm_wday = (packedDate >> 11) & 7;
    lt.tm_mday = ((packedDate >> 14) & 0x3F) + 1;
    lt.tm_mon = (packedDate >> 20) & 0xF;
    lt.tm_year = ((packedDate >> 24) & 0x1F) + 100;

    return uint32(mktime(&lt));
}

ByteBuffer& ByteBuffer::ReadPackedTime(uint32& time)
{
    time = ReadPackedTime();
    return *this;
}

uint8* ByteBuffer::contents()
{
    if (_storage.empty())
        throw ByteBufferException();
    return _storage.data();
}

uint8 const* ByteBuffer::contents() const
{
    if (_storage.empty())
        throw ByteBufferException();
    return _storage.data();
}

size_t ByteBuffer::size() const
{
    return _storage.size();
}

bool ByteBuffer::empty() const
{
    return _storage.empty();
}

void ByteBuffer::resize(size_t newsize)
{
    _storage.resize(newsize, 0);
    _rpos = 0;
    _wpos = size();
}

void ByteBuffer::reserve(size_t ressize)
{
    if (ressize > size())
        _storage.reserve(ressize);
}

void ByteBuffer::append(const char* src, size_t cnt)
{
    return append(reinterpret_cast<const uint8 *>(src), cnt);
}

void ByteBuffer::append(const uint8 *src, size_t cnt)
{
    ASSERT(src, "Attempted to put a NULL-pointer in ByteBuffer (pos: " SZFMTD " size: " SZFMTD ")", _wpos, size());
    ASSERT(cnt, "Attempted to put a zero-sized value in ByteBuffer (pos: " SZFMTD " size: " SZFMTD ")", _wpos, size());
    ASSERT(size() < 10000000);

    FlushBits();
    _storage.insert(_storage.begin() + _wpos, src, src + cnt);
    _wpos += cnt;
}

void ByteBuffer::append(const ByteBuffer& buffer)
{
    if (!buffer.empty())
        append(buffer.contents(), buffer.size());
}

void ByteBuffer::appendPackXYZ(float x, float y, float z)
{
    uint32 packed = 0;
    packed |= static_cast<int>(x / 0.25f) & 0x7FF;
    packed |= (static_cast<int>(y / 0.25f) & 0x7FF) << 11;
    packed |= (static_cast<int>(z / 0.25f) & 0x3FF) << 22;
    *this << packed;
}

size_t ByteBuffer::PackUInt64(uint64 value, uint8* mask, uint8* result)
{
    size_t resultSize = 0;
    *mask = 0;
    memset(result, 0, 8);
    for (uint8 i = 0; value != 0; ++i)
    {
        if (value & 0xFF)
        {
            *mask |= uint8(1 << i);
            result[resultSize++] = uint8(value & 0xFF);
        }
        value >>= 8;
    }
    return resultSize;
}

void ByteBuffer::put(size_t pos, const uint8 *src, size_t cnt)
{
    ASSERT(pos + cnt <= size(), "Attempted to put value with size: " SZFMTD " in ByteBuffer (pos: " SZFMTD " size: " SZFMTD ")", cnt, pos, size());
    ASSERT(src, "Attempted to put a NULL-pointer in ByteBuffer (pos: " SZFMTD " size: " SZFMTD ")", pos, size());
    ASSERT(cnt, "Attempted to put a zero-sized value in ByteBuffer (pos: " SZFMTD " size: " SZFMTD ")", pos, size());

    std::memcpy(&_storage[pos], src, cnt);
}

void ByteBuffer::print_storage() const
{
    if (!sLog->ShouldLog(LOG_FILTER_NETWORKIO, LOG_LEVEL_TRACE)) // optimize disabled debug output
        return;

    std::ostringstream o;
    o << "STORAGE_SIZE: " << size();
    for (uint32 i = 0; i < size(); ++i)
        o << read<uint8>(i) << " - ";
    o << " ";

    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "%s", o.str().c_str());
}

void ByteBuffer::textlike() const
{
    if (!sLog->ShouldLog(LOG_FILTER_NETWORKIO, LOG_LEVEL_TRACE)) // optimize disabled debug output
        return;

    std::ostringstream o;
    o << "STORAGE_SIZE: " << size();
    for (uint32 i = 0; i < size(); ++i)
    {
        char buf[2];
        snprintf(buf, 2, "%c", read<uint8>(i));
        o << buf;
    }
    o << " ";
    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "%s", o.str().c_str());
}

void ByteBuffer::hexlike() const
{
    if (!sLog->ShouldLog(LOG_FILTER_NETWORKIO, LOG_LEVEL_WARN)) // optimize disabled debug output
        return;

    uint32 j = 1, k = 1;

    std::ostringstream o;
    o << "STORAGE_SIZE: " << size();

    for (uint32 i = 0; i < size(); ++i)
    {
        char buf[3];
        snprintf(buf, 3, "%2X ", read<uint8>(i));
        if ((i == (j * 8)) && ((i != (k * 16))))
        {
            o << "| ";
            ++j;
        }
        else if (i == (k * 16))
        {
            o << "\n";
            ++k;
            ++j;
        }

        o << buf;
    }
    o << " ";
    TC_LOG_TRACE(LOG_FILTER_NETWORKIO, "%s", o.str().c_str());
}
