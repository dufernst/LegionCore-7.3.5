/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "MessageBuffer.h"

MessageBuffer::MessageBuffer(): _wpos(0), _rpos(0), _storage()
{
    _storage.resize(2048);
}

MessageBuffer::MessageBuffer(std::size_t initialSize): _wpos(0), _rpos(0), _storage()
{
    _storage.resize(initialSize);
}

MessageBuffer::MessageBuffer(MessageBuffer const& right): _wpos(right._wpos), _rpos(right._rpos), _storage(right._storage)
{
}

MessageBuffer::MessageBuffer(MessageBuffer&& right) noexcept: _wpos(right._wpos), _rpos(right._rpos), _storage(right.Move())
{
}

void MessageBuffer::Reset()
{
    _wpos = 0;
    _rpos = 0;
}

void MessageBuffer::Resize(size_type bytes)
{
    _storage.resize(bytes);
}

uint8* MessageBuffer::GetBasePointer()
{
    return _storage.data();
}

uint8* MessageBuffer::GetReadPointer()
{
    return GetBasePointer() + _rpos;
}

uint8* MessageBuffer::GetWritePointer()
{
    return GetBasePointer() + _wpos;
}

void MessageBuffer::ReadCompleted(size_type bytes)
{
    _rpos += bytes;
}

void MessageBuffer::WriteCompleted(size_type bytes)
{
    _wpos += bytes;
}

MessageBuffer::size_type MessageBuffer::GetActiveSize() const
{
    return _wpos - _rpos;
}

MessageBuffer::size_type MessageBuffer::GetRemainingSpace() const
{
    return _storage.size() - _wpos;
}

MessageBuffer::size_type MessageBuffer::GetBufferSize() const
{
    return _storage.size();
}

void MessageBuffer::Normalize()
{
    if (_rpos)
    {
        if (_rpos != _wpos)
            memmove(GetBasePointer(), GetReadPointer(), GetActiveSize());
        _wpos -= _rpos;
        _rpos = 0;
    }
}

void MessageBuffer::EnsureFreeSpace()
{
    // resize buffer if it's already full
    if (GetRemainingSpace() == 0)
        _storage.resize(_storage.size() * 3 / 2);
}

void MessageBuffer::Write(void const* data, std::size_t size)
{
    if (size)
    {
        memcpy(GetWritePointer(), data, size);
        WriteCompleted(size);
    }
}

std::vector<uint8>&& MessageBuffer::Move()
{
    _wpos = 0;
    _rpos = 0;
    return std::move(_storage);
}

MessageBuffer& MessageBuffer::operator=(MessageBuffer& right)
{
    if (this != &right)
    {
        _wpos = right._wpos;
        _rpos = right._rpos;
        _storage = right._storage;
    }

    return *this;
}

MessageBuffer& MessageBuffer::operator=(MessageBuffer&& right) noexcept
{
    if (this != &right)
    {
        _wpos = right._wpos;
        _rpos = right._rpos;
        _storage = right.Move();
    }

    return *this;
}
