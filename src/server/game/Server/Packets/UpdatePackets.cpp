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

#include "UpdatePackets.h"

WorldPacket const* WorldPackets::Update::DestroyArenaUnit::Write()
{
    _worldPacket << Guid;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Update::MapObjEvents::Write()
{
    _worldPacket << UniqueID;
    _worldPacket << DataSize;
    _worldPacket << static_cast<uint32>(Unk2.size());
    for (auto const& itr : Unk2)
        _worldPacket << itr;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Update::SetAnimTimer::Write()
{
    _worldPacket << Unit;
    _worldPacket.WriteBits(Tier, 3);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Update::VignetteUpdate::Write()
{
    _worldPacket.WriteBit(ForceUpdate);
    _worldPacket.FlushBits();

    _worldPacket << static_cast<uint32>(Removed.IDs.size());
    for (ObjectGuid const& ID : Removed.IDs)
        _worldPacket << ID;

    _worldPacket << static_cast<uint32>(Added.IdList.IDs.size());
    for (ObjectGuid const& ID : Added.IdList.IDs)
        _worldPacket << ID;

    _worldPacket << static_cast<uint32>(Added.Data.size());
    for (auto const& x : Added.Data)
    {
        _worldPacket << x.Pos;
        _worldPacket << x.ObjGUID;
        _worldPacket << x.VignetteID;
        _worldPacket << x.ZoneID;
    }

    _worldPacket << static_cast<uint32>(Updated.IdList.IDs.size());
    for (ObjectGuid const& ID : Updated.IdList.IDs)
        _worldPacket << ID;

    _worldPacket << static_cast<uint32>(Updated.Data.size());
    for (auto const& x : Updated.Data)
    {
        _worldPacket << x.Pos;
        _worldPacket << x.ObjGUID;
        _worldPacket << x.VignetteID;
        _worldPacket << x.ZoneID;
    }

    return &_worldPacket;
}
