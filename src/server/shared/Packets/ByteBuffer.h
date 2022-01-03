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

#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include "Define.h"
#include "ByteConverter.h"
#include <string>
#include <vector>
#include <cstring>

class MessageBuffer;

class ByteBufferException : public std::exception
{
public:
    ~ByteBufferException() noexcept override;

    char const* what() const noexcept override;

protected:
    std::string& message() noexcept;

private:
    std::string msg_;
};

class ByteBufferPositionException : public ByteBufferException
{
public:
    ByteBufferPositionException(size_t pos, size_t size, size_t valueSize);

    ~ByteBufferPositionException() noexcept override;
};

class ByteBufferInvalidValueException : public ByteBufferException
{
public:
    ByteBufferInvalidValueException(char const* type, size_t pos);

    ~ByteBufferInvalidValueException() noexcept = default;
};

class ByteBuffer
{
public:
    static size_t const DEFAULT_SIZE = 0x1000;
    static uint8 const InitialBitPos = 8;

    ByteBuffer();

    explicit ByteBuffer(size_t reserve);
    ByteBuffer(ByteBuffer&& buf) noexcept;
    ByteBuffer(ByteBuffer const& right);
    explicit ByteBuffer(MessageBuffer&& buffer);

    std::vector<uint8>&& Move() noexcept;

    ByteBuffer& operator=(ByteBuffer const& right);
    ByteBuffer& operator=(ByteBuffer&& right) noexcept;

    virtual ~ByteBuffer();

    void clear();

    void FlushBits();
    void ResetBitPos();
    void ResetBitReader();

    bool WriteBit(bool bit);
    bool ReadBit();
    void WriteBits(std::size_t value, int32 bits);
    uint32 ReadBits(int32 bits);

    ByteBuffer& operator<<(uint8 value);
    ByteBuffer& operator<<(uint16 value);
    ByteBuffer& operator<<(uint32 value);
    ByteBuffer& operator<<(uint64 value);
    ByteBuffer& operator<<(std::chrono::milliseconds value);
    ByteBuffer& operator<<(std::chrono::seconds value);
    ByteBuffer& operator<<(int8 value);
    ByteBuffer& operator<<(int16 value);
    ByteBuffer& operator<<(int32 value);
    ByteBuffer& operator<<(int64 value);
    ByteBuffer& operator<<(float value);
    ByteBuffer& operator<<(double value);
    ByteBuffer& operator<<(const std::string& value);
    ByteBuffer& operator<<(const char* str);

    ByteBuffer& operator>>(uint8& value);
    ByteBuffer& operator>>(uint16& value);
    ByteBuffer& operator>>(uint32& value);
    ByteBuffer& operator>>(uint64& value);
    ByteBuffer& operator>>(int8& value);
    ByteBuffer& operator>>(int16& value);
    ByteBuffer& operator>>(int32& value);
    ByteBuffer& operator>>(int64& value);
    ByteBuffer& operator>>(float& value);
    ByteBuffer& operator>>(double& value);
    ByteBuffer& operator>>(std::string& value);

    uint8& operator[](size_t pos);
    uint8 const& operator[](size_t pos) const;

    size_t rpos() const;
    size_t rpos(size_t rpos_);

    void rfinish();

    size_t wpos() const;
    size_t wpos(size_t wpos_);

    size_t bitwpos() const;
    size_t bitwpos(size_t newPos);

    void read(uint8* dest, size_t len);

    void ReadPackedUInt64(uint64& guid);
    void ReadPackedUInt64(uint8 mask, uint64& value);

    std::string ReadCString(bool requireValidUtf8 = true);
    std::string ReadString(uint32 length, bool requireValidUtf8 = true);

    void WriteString(std::string const& str);
    void WriteString(std::string const& str, uint8 strLen);

    uint32 ReadPackedTime();
    ByteBuffer& ReadPackedTime(uint32& time);

    uint8* contents();
    uint8 const* contents() const;

    size_t size() const;
    bool empty() const;
    void resize(size_t newsize);
    void reserve(size_t ressize);

    void append(const char* src, size_t cnt);
    void append(const uint8 *src, size_t cnt);
    void append(const ByteBuffer& buffer);

    void appendPackXYZ(float x, float y, float z);

    static size_t PackUInt64(uint64 value, uint8* mask, uint8* result);

    void put(size_t pos, const uint8 *src, size_t cnt);

    void print_storage() const;
    void textlike() const;
    void hexlike() const;

    template<class T> void append(const T *src, size_t cnt)
    {
        return append(reinterpret_cast<const uint8 *>(src), cnt * sizeof(T));
    }

    template <typename T> T read()
    {
        ResetBitPos();
        auto r = read<T>(_rpos);
        _rpos += sizeof(T);
        return r;
    }

    template <typename T> T read(size_t pos) const
    {
        if (pos + sizeof(T) > size())
            throw ByteBufferPositionException(pos, sizeof(T), size());
        T val = *reinterpret_cast<T const*>(&_storage[pos]);
        EndianConvert(val);
        return val;
    }

    template <typename T>
    void put(std::size_t pos, T value)
    {
        static_assert(std::is_fundamental<T>::value, "append(compound)");
        EndianConvert(value);
        put(pos, reinterpret_cast<uint8 *>(&value), sizeof(value));
    }

    template <typename T> void append(T value)
    {
        static_assert(std::is_fundamental<T>::value, "append(compound)");
        EndianConvert(value);
        append(reinterpret_cast<uint8 *>(&value), sizeof(value));
    }

protected:
    size_t _rpos{}, _wpos{}, _bitpos{};
    uint8 _curbitval{};
    std::vector<uint8> _storage;
};

template<> inline std::string ByteBuffer::read<std::string>()
{
    std::string tmp;
    *this >> tmp;
    return tmp;
}

#endif
