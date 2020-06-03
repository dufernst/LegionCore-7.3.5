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

#include "InstancePackets.h"

WorldPacket const* WorldPackets::Instance::UpdateLastInstance::Write()
{
    _worldPacket << uint32(MapID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::UpdateInstanceOwnership::Write()
{
    _worldPacket << int32(IOwnInstance);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceInfo::Write()
{
    _worldPacket << static_cast<int32>(LockList.size());

    for (InstanceLockInfos const& lockInfos : LockList)
        _worldPacket << lockInfos;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Instance::InstanceLockInfos const& lockInfos)
{
    data << lockInfos.MapID;
    data << lockInfos.DifficultyID;
    data << lockInfos.InstanceID;
    data << lockInfos.TimeRemaining;
    data << lockInfos.CompletedMask;

    data.WriteBit(lockInfos.Locked);
    data.WriteBit(lockInfos.Extended);

    data.FlushBits();

    return data;
}

WorldPacket const* WorldPackets::Instance::InstanceReset::Write()
{
    _worldPacket << uint32(MapID);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceResetFailed::Write()
{
    _worldPacket << uint32(MapID);
    _worldPacket.WriteBits(ResetFailedReason, 2);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceSaveCreated::Write()
{
    _worldPacket.WriteBit(Gm);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::Instance::InstanceLockResponse::Read()
{
    AcceptLock = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Instance::RaidGroupOnly::Write()
{
    _worldPacket << Delay;
    _worldPacket << Reason;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::BossKillCredit::Write()
{
    _worldPacket << encounterID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::RaidInstanceMessage::Write()
{
    _worldPacket << Type;
    _worldPacket << MapID;
    _worldPacket << DifficultyID;
    _worldPacket.WriteBit(Locked);
    _worldPacket.WriteBit(Extended);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::ChangePlayerDifficultyResult::Write()
{
    _worldPacket.FlushBits();

    _worldPacket.WriteBits(Result, 4);
    switch (Result)
    {
        case 5:
        case 8:
            _worldPacket.WriteBit(Cooldown);
            _worldPacket.FlushBits();
            _worldPacket << CooldownReason;
            break;
        case 11:
            _worldPacket << InstanceMapID;
            _worldPacket << DifficultyRecID;
            break;
        case 2:
            _worldPacket << MapID;
            break;
        case 4:
            _worldPacket << Guid;
            break;
        default:
            break;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceGroupSizeChanged::Write()
{
    _worldPacket << GroupSize;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterStart::Write()
{
    _worldPacket << InCombatResCount;
    _worldPacket << MaxInCombatResCount;
    _worldPacket << CombatResChargeRecovery;
    _worldPacket << NextCombatResChargeTime;
    _worldPacket.WriteBit(InProgress);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterEngageUnit::Write()
{
    _worldPacket << Unit;
    _worldPacket << TargetFramePriority;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterChangePriority::Write()
{
    _worldPacket << Unit;
    _worldPacket << TargetFramePriority;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterDisengageUnit::Write()
{
    _worldPacket << Unit;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterGainCombatResurrectionCharge::Write()
{
    _worldPacket << InCombatResCount;
    _worldPacket << CombatResChargeRecovery;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::EncounterStart::Write()
{
    _worldPacket << uint32(EncounterID);
    _worldPacket << uint32(DifficultyID);
    _worldPacket << uint32(GroupSize);
    _worldPacket << uint32(0);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::EncounterEnd::Write()
{
    _worldPacket << EncounterID;
    _worldPacket << DifficultyID;
    _worldPacket << GroupSize;
    _worldPacket.WriteBit(Success);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::PendingRaidLock::Write()
{
    _worldPacket << CompletedMask;
    _worldPacket << TimeUntilLock;
    _worldPacket.WriteBit(Extending);
    _worldPacket.WriteBit(WarningOnly);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::StartTimer::Write()
{
    _worldPacket << TimeRemaining;
    _worldPacket << TotalTime;
    _worldPacket << int32(Type);

    return &_worldPacket;
}

void WorldPackets::Instance::QueryWorldCountwodnTimer::Read()
{
    uint32(Type) = _worldPacket.read<uint32>();
}

WorldPacket const* WorldPackets::Instance::SummonRaidMemberValidateFailed::Write()
{
    _worldPacket << static_cast<int32>(Members.size());
    for (auto const& v : Members)
    {
        _worldPacket << v.Member;
        _worldPacket << v.ReasonCode;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterSetAllowingRelease::Write()
{
    _worldPacket.WriteBit(ReleaseAllowed);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Instance::InstanceEncounterSetSuppressingRelease::Write()
{
    _worldPacket.WriteBit(SupressinDisabled);
    _worldPacket.FlushBits();

    return &_worldPacket;
}
