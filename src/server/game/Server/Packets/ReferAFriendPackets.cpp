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

#include "ReferAFriendPackets.h"

void WorldPackets::ReferAFriend::AcceptLevelGrant::Read()
{
    _worldPacket >> Granter;
}

void WorldPackets::ReferAFriend::GrantLevel::Read()
{
    _worldPacket >> Target;
}

WorldPacket const* WorldPackets::ReferAFriend::ProposeLevelGrant::Write()
{
    _worldPacket << Sender;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::ReferAFriend::ReferAFriendFailure::Write()
{
    _worldPacket << int32(Reason);
    _worldPacket.WriteBits(Str.length(), 6);
    _worldPacket.WriteString(Str);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::ReferAFriend::RecruitAFriendResponse::Write()
{
    _worldPacket.WriteBits(Result, 3);
    _worldPacket.FlushBits();

    return &_worldPacket;
}
