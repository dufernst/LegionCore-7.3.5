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

#include "WorldStatePackets.h"

WorldPacket const* WorldPackets::WorldState::InitWorldStates::Write()
{
    std::sort(Worldstates.begin(), Worldstates.end(), [](WorldStateInfo const& a, WorldStateInfo const& b) -> bool
    {
        return a.VariableID < b.VariableID;
    });

    _worldPacket.reserve(16 + Worldstates.size() * 8);

    _worldPacket << uint32(MapID);
    _worldPacket << uint32(AreaID);
    _worldPacket << uint32(SubareaID);

    _worldPacket << uint32(Worldstates.size());
    for (auto const& wsi : Worldstates)
    {
        _worldPacket << uint32(wsi.VariableID);
        _worldPacket << int32(wsi.Value);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::WorldState::UpdateWorldState::Write()
{
    _worldPacket << uint32(VariableID);
    _worldPacket << int32(Value);
    _worldPacket.WriteBit(Hidden);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::WorldState::StartElapsedTimer::Write()
{
    _worldPacket << Timer.TimerID;
    _worldPacket << uint32(Timer.CurrentDuration);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::WorldState::StartElapsedTimers::Write()
{
    _worldPacket << static_cast<uint32>(Timers.size());
    for (auto const& v : Timers)
    {
        _worldPacket << v.TimerID;
        _worldPacket << uint32(v.CurrentDuration);
    }

    return &_worldPacket;
}
