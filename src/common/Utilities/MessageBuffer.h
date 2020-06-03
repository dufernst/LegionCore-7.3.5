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

#ifndef __MESSAGEBUFFER_H_
#define __MESSAGEBUFFER_H_

#include "Define.h"
#include <vector>

class MessageBuffer
{
    typedef std::vector<uint8>::size_type size_type;

public:
    MessageBuffer();
    explicit MessageBuffer(std::size_t initialSize);
    MessageBuffer(MessageBuffer const& right);
    MessageBuffer(MessageBuffer&& right) noexcept;

    void Reset();
    void Resize(size_type bytes);

    uint8* GetBasePointer();
    uint8* GetReadPointer();
    uint8* GetWritePointer();

    void ReadCompleted(size_type bytes);
    void WriteCompleted(size_type bytes);

    size_type GetActiveSize() const;
    size_type GetRemainingSpace() const;
    size_type GetBufferSize() const;

    // Discards inactive data
    void Normalize();

    // Ensures there's "some" free space, make sure to call Normalize() before this
    void EnsureFreeSpace();

    void Write(void const* data, std::size_t size);

    std::vector<uint8>&& Move();

    MessageBuffer& operator=(MessageBuffer& right);
    MessageBuffer& operator=(MessageBuffer&& right) noexcept;

private:
    size_type _wpos;
    size_type _rpos;
    std::vector<uint8> _storage;
};

#endif /* __MESSAGEBUFFER_H_ */
