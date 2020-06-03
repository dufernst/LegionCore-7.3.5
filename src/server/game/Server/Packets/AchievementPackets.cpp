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

#include "AchievementPackets.h"
#include "WowTime.hpp"

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Achievement::EarnedAchievement const& earned)
{
    data << uint32(earned.Id);
    data << MS::Utilities::WowTime::Encode(earned.Date);
    data << earned.Owner;
    data << uint32(earned.VirtualRealmAddress);
    data << uint32(earned.NativeRealmAddress);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& _worldPacket, WorldPackets::Achievement::CriteriaTreeProgress const& progress)
{
    _worldPacket << uint32(progress.Id);
    _worldPacket << uint64(progress.Quantity);
    _worldPacket << progress.Player;
    _worldPacket << MS::Utilities::WowTime::Encode(progress.Date);
    _worldPacket << uint32(progress.TimeFromStart);
    _worldPacket << uint32(progress.TimeFromCreate);
    _worldPacket.WriteBits(progress.Flags, 4);
    _worldPacket.FlushBits();

    return _worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& _worldPacket, WorldPackets::Achievement::AllAchievements const& achieve)
{
    _worldPacket << static_cast<uint32>(achieve.Earned.size());
    _worldPacket << static_cast<uint32>(achieve.Progress.size());

    for (WorldPackets::Achievement::EarnedAchievement const& earned : achieve.Earned)
        _worldPacket << earned;

    for (WorldPackets::Achievement::CriteriaTreeProgress const& progress : achieve.Progress)
        _worldPacket << progress;

    return _worldPacket;
}

WorldPacket const* WorldPackets::Achievement::AllAchievements::Write()
{
    _worldPacket << *this;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::CriteriaUpdate::Write()
{
    _worldPacket << uint32(CriteriaID);
    _worldPacket << uint64(Quantity);
    _worldPacket << PlayerGUID;
    _worldPacket << uint32(Flags);
    _worldPacket << MS::Utilities::WowTime::Encode(CurrentTime);
    _worldPacket << uint32(ElapsedTime);
    _worldPacket << uint32(CreationTime);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::CriteriaDeleted::Write()
{
    _worldPacket << uint32(CriteriaID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::AchievementDeleted::Write()
{
    _worldPacket << uint32(AchievementID);
    _worldPacket << uint32(Immunities);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::AchievementEarned::Write()
{
    _worldPacket << Sender;
    _worldPacket << Earner;
    _worldPacket << uint32(AchievementID);
    _worldPacket << MS::Utilities::WowTime::Encode(Time);
    _worldPacket << uint32(EarnerNativeRealm);
    _worldPacket << uint32(EarnerVirtualRealm);
    _worldPacket.WriteBit(Initial);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::ServerFirstAchievement::Write()
{
    _worldPacket.WriteBits(Name.length(), 7);
    _worldPacket.WriteBit(GuildAchievement);
    _worldPacket << PlayerGUID;
    _worldPacket << AchievementID;
    _worldPacket.WriteString(Name);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::GuildCriteriaUpdate::Write()
{
    _worldPacket << static_cast<uint32>(Progress.size());

    for (GuildCriteriaProgress const& progress : Progress)
    {
        _worldPacket << int32(progress.CriteriaID);
        _worldPacket << uint32(progress.DateCreated);
        _worldPacket << uint32(progress.DateStarted);
        _worldPacket << MS::Utilities::WowTime::Encode(progress.DateUpdated);
        _worldPacket << uint64(progress.Quantity);
        _worldPacket << progress.PlayerGUID;
        _worldPacket << int32(progress.Flags);
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::GuildCriteriaDeleted::Write()
{
    _worldPacket << GuildGUID;
    _worldPacket << int32(CriteriaID);

    return &_worldPacket;
}

void WorldPackets::Achievement::GuildSetFocusedAchievement::Read()
{
    _worldPacket >> AchievementID;
}

WorldPacket const* WorldPackets::Achievement::GuildAchievementDeleted::Write()
{
    _worldPacket << GuildGUID;
    _worldPacket << uint32(AchievementID);
    _worldPacket << MS::Utilities::WowTime::Encode(TimeDeleted);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::GuildAchievementEarned::Write()
{
    _worldPacket << GuildGUID;
    _worldPacket << uint32(AchievementID);
    _worldPacket << MS::Utilities::WowTime::Encode(TimeEarned);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::AllGuildAchievements::Write()
{
    _worldPacket << static_cast<uint32>(Earned.size());
    for (EarnedAchievement const& earned : Earned)
        _worldPacket << earned;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::CriteriaUpdateAccount::Write()
{
    _worldPacket << Data;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Achievement::AllAchievementCriteriaDataAccount::Write()
{
    _worldPacket << static_cast<uint32>(Data.size());
    for (auto const& itr : Data)
        _worldPacket << itr;

    return &_worldPacket;
}

void WorldPackets::Achievement::GuildGetAchievementMembers::Read()
{
    _worldPacket >> AchievementID;
}

void WorldPackets::Achievement::SetAchievementsHidden::Read()
{
    Hidden = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Achievement::RespondInspectAchievements::Write()
{
    _worldPacket << Player;
    _worldPacket << Data;

    return &_worldPacket;
}
