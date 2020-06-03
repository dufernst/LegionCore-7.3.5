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

#include "LFGPackets.h"
#include "LFG.h"

void WorldPackets::LFG::BlackList::Initialize(std::map<uint32, lfg::LockData> const& lock, ObjectGuid const& guid /*= ObjectGuid::Empty*/)
{
    if (guid)
    {
        PlayerGuid = boost::in_place();
        PlayerGuid = guid;
    }

    for (auto const& map : lock)
    {
        BlackListInfo info;
        info.Slot = map.first;
        info.Reason = map.second.status;
        info.SubReason1 = map.second.reqItemLevel;
        info.SubReason2 = map.second.currItemLevel;
        Slots.push_back(info);
    }

    std::sort(std::begin(Slots), std::end(Slots), [](BlackListInfo const& a, BlackListInfo const& b) -> bool
    {
        return a.Slot < b.Slot;
    });
}

WorldPackets::LFG::BlackList::BlackListInfo::BlackListInfo(uint32 slot, uint32 reason, int32 subReason1, int32 subReason2): Slot(slot), Reason(reason), SubReason1(subReason1), SubReason2(subReason2)
{
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LFG::BlackList const& blackList)
{
    data.WriteBit(blackList.PlayerGuid.is_initialized());
    data << static_cast<uint32>(blackList.Slots.size());

    if (blackList.PlayerGuid)
        data << *blackList.PlayerGuid;

    for (auto const& map : blackList.Slots)
    {
        data << map.Slot;
        data << map.Reason;
        data << map.SubReason1;
        data << map.SubReason2;
    }

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LFG::BootInfo const& boot)
{
    data.WriteBit(boot.VoteInProgress);
    data.WriteBit(boot.VotePassed);
    data.WriteBit(boot.MyVoteCompleted);
    data.WriteBit(boot.MyVote);
    data.WriteBits(boot.Reason.length(), 8);
    data << boot.Target;
    data << boot.TotalVotes;
    data << boot.BootVotes;
    data << boot.TimeLeft;
    data << boot.VotesNeeded;
    data << boot.Reason;

    return data;
}

void WorldPackets::LFG::ShortageReward::Initialize(::Quest const* quest /*= nullptr*/, Player* player /*= nullptr*/)
{
    if (!quest)
        return;

    Mask = 0;
    if (player)
    {
        if (player->getLevel() < sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
            RewardXP = player->GetQuestXPReward(quest);
        else
            RewardMoney = player->GetQuestMoneyReward(quest);
    }

    RewardSpellID = quest->RewardSpell;
    RewardHonor = quest->RewardHonor;

    for (uint8 i = 0; i < std::extent<decltype(quest->RewardItemId)>::value; ++i)
        if (uint32 itemId = quest->RewardItemId[i])
            Item.emplace_back(itemId, quest->RewardItemCount[i]);

    for (uint8 i = 0; i < std::extent<decltype(quest->RewardCurrencyId)>::value; ++i)
    {
        uint32 currencyID = quest->RewardCurrencyId[i];
        if (!currencyID)
            continue;

        Currency.emplace_back(currencyID, quest->RewardCurrencyCount[i] * sDB2Manager.GetCurrencyPrecision(currencyID));
    }
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::LFG::ShortageReward const& reward)
{
    data << reward.Mask;
    data << reward.RewardMoney;
    data << reward.RewardXP;

    data << static_cast<uint32>(reward.Item.size());
    data << static_cast<uint32>(reward.Currency.size());
    data << static_cast<uint32>(reward.BonusCurrency.size());

    for (auto const& itemreward : reward.Item)
    {
        data << itemreward.ItemID;
        data << itemreward.Quantity;
    }

    for (auto const& currencyreward : reward.Currency)
    {
        data << currencyreward.CurrencyID;
        data << currencyreward.Quantity;
    }

    for (auto const& bonusCurrencyreward : reward.BonusCurrency)
    {
        data << bonusCurrencyreward.CurrencyID;
        data << bonusCurrencyreward.Quantity;
    }

    data.WriteBit(reward.RewardSpellID.is_initialized());
    data.WriteBit(reward.UnkInt2.is_initialized());
    data.WriteBit(reward.UnkInt3.is_initialized());
    data.WriteBit(reward.RewardHonor.is_initialized());
    data.FlushBits();

    if (reward.RewardSpellID)
        data << *reward.RewardSpellID;

    if (reward.UnkInt2)
        data << *reward.UnkInt2;

    if (reward.UnkInt3)
        data << *reward.UnkInt3;

    if (reward.RewardHonor)
        data << *reward.RewardHonor;

    return data;
}

WorldPacket const* WorldPackets::LFG::PlayerInfo::Write()
{
    _worldPacket << static_cast<uint32>(Dungeon.size());
    _worldPacket << BlackListMap;

    for (auto const& map : Dungeon)
    {
        _worldPacket << map.Slot;
        _worldPacket << map.CompletionQuantity;
        _worldPacket << map.CompletionLimit;
        _worldPacket << map.CompletionCurrencyID;
        _worldPacket << map.SpecificQuantity;
        _worldPacket << map.SpecificLimit;
        _worldPacket << map.OverallQuantity;
        _worldPacket << map.OverallLimit;
        _worldPacket << map.PurseWeeklyQuantity;
        _worldPacket << map.PurseWeeklyLimit;
        _worldPacket << map.PurseQuantity;
        _worldPacket << map.PurseLimit;
        _worldPacket << map.Quantity;
        _worldPacket << map.CompletedMask;
        _worldPacket << map.EncounterMask;
        _worldPacket << static_cast<uint32>(map.ShortageRewards.size());
        _worldPacket.WriteBit(map.FirstReward);
        _worldPacket.WriteBit(map.ShortageEligible);
        _worldPacket << map.Reward;

        for (auto const& shortageRewardMap : map.ShortageRewards)
            _worldPacket << shortageRewardMap;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::JoinResult::Write()
{
    _worldPacket << Ticket;
    _worldPacket << Result;
    _worldPacket << ResultDetail;
    _worldPacket << static_cast<uint32>(Slots.size());

    for (auto const& blackList : Slots)
        _worldPacket << blackList;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::QueueStatusUpdate::Write()
{
    _worldPacket << Ticket;

    _worldPacket << SubType;
    _worldPacket << Reason;

    _worldPacket << static_cast<uint32>(Slots.size());
    _worldPacket << RequestedRoles;
    _worldPacket << static_cast<uint32>(SuspendedPlayers.size());

    for (uint32 const& map : Slots)
        _worldPacket << map;

    for (ObjectGuid const& map2 : SuspendedPlayers)
        _worldPacket << map2;

    _worldPacket.WriteBit(IsParty);
    _worldPacket.WriteBit(NotifyUI);
    _worldPacket.WriteBit(Joined);
    _worldPacket.WriteBit(LfgJoined);
    _worldPacket.WriteBit(Queued);
    _worldPacket.WriteBit(Unused);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::LFG::LockInfoRequest::Read()
{
    Player = _worldPacket.ReadBit();
    _worldPacket >> PartyIndex;
}

WorldPacket const* WorldPackets::LFG::QueueStatus::Write()
{
    _worldPacket << Ticket;

    _worldPacket << Slot;
    _worldPacket << AvgWaitTime;
    _worldPacket << QueuedTime;

    for (uint8 i = 0; i < 3; i++)
    {
        _worldPacket << AvgWaitTimeByRole[i];
        _worldPacket << LastNeeded[i];
    }

    _worldPacket << AvgWaitTimeMe;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::ProposalUpdate::Write()
{
    _worldPacket << Ticket;

    _worldPacket << InstanceID;

    _worldPacket << ProposalID;
    _worldPacket << Slot;

    _worldPacket << State;

    _worldPacket << CompletedMask;
    _worldPacket << EncounterMask;
    _worldPacket << static_cast<uint32>(Players.size());
    _worldPacket << uint8(0); // unused by client

    _worldPacket.WriteBit(ValidCompletedMask);
    _worldPacket.WriteBit(ProposalSilent);
    _worldPacket.WriteBit(IsRequeue);
    _worldPacket.FlushBits();

    for (auto const& map : Players)
    {
        _worldPacket << map.Roles;
        _worldPacket.WriteBit(map.Me);
        _worldPacket.WriteBit(map.SameParty);
        _worldPacket.WriteBit(map.MyParty);
        _worldPacket.WriteBit(map.Responded);
        _worldPacket.WriteBit(map.Accepted);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::PlayerReward::Write()
{
    _worldPacket << ActualSlot;
    _worldPacket << QueuedSlot;
    _worldPacket << RewardMoney;
    _worldPacket << AddedXP;

    _worldPacket << static_cast<uint32>(Players.size());
    for (auto const& map : Players)
    {
        _worldPacket << map.RewardItem;
        _worldPacket << map.RewardItemQuantity;
        _worldPacket << map.BonusCurrency;
        _worldPacket.WriteBit(map.IsCurrency);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

void WorldPackets::LFG::LfgJoin::Read()
{
    QueueAsGroup = _worldPacket.ReadBit();
    UnkBit = _worldPacket.ReadBit();
    _worldPacket.ResetBitReader();

    _worldPacket >> PartyIndex;
    _worldPacket >> Roles;

    auto numDungeons = _worldPacket.read<uint32>();
    for (uint8 i = 0; i < numDungeons; i++)
        Slot.insert(_worldPacket.read<uint32>() & 0xFFFFF);
}

WorldPacket const* WorldPackets::LFG::RoleCheckUpdate::Write()
{
    _worldPacket << PartyIndex;
    _worldPacket << RoleCheckStatus;
    _worldPacket << static_cast<uint32>(JoinSlots.size());
    _worldPacket << BgQueueID;
    _worldPacket << GroupFinderActivityID;
    _worldPacket << static_cast<uint32>(Members.size());

    for (auto const& map : JoinSlots)
        _worldPacket << map;

    _worldPacket.WriteBit(IsBeginning);
    _worldPacket.WriteBit(IsRequeue);
    _worldPacket.FlushBits();

    for (auto const& map : Members)
    {
        _worldPacket << map.Guid;
        _worldPacket << map.RolesDesired;
        _worldPacket << map.Level;
        _worldPacket.WriteBit(map.RoleCheckComplete);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::RoleChosen::Write()
{
    _worldPacket << Player;
    _worldPacket << RoleMask;
    _worldPacket.WriteBit(Accepted);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::PartyInfo::Write()
{
    _worldPacket << static_cast<uint32>(Player.size());
    for (auto const& map : Player)
        _worldPacket << map;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::BootPlayer::Write()
{
    _worldPacket << Info;

    return &_worldPacket;
}

void WorldPackets::LFG::LfgBootPlayerVote::Read()
{
    Vote = _worldPacket.ReadBit();
}

void WorldPackets::LFG::LfgProposalResponse::Read()
{
    _worldPacket >> Data.Ticket;
    _worldPacket >> Data.InstanceID;
    _worldPacket >> Data.ProposalID;
    Data.Accepted = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::LFG::TeleportDenied::Write()
{
    _worldPacket.WriteBits(Reason, 4);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

void WorldPackets::LFG::LfgTeleport::Read()
{
    TeleportOut = _worldPacket.ReadBit();
}

void WorldPackets::LFG::LfgCompleteRoleCheck::Read()
{
    _worldPacket >> RolesDesired;
    _worldPacket >> PartyIndex;
}

void WorldPackets::LFG::LfgLeave::Read()
{
    _worldPacket >> Ticket;
}

void WorldPackets::LFG::BonusFactionID::Read()
{
    _worldPacket >> FactionID;
}

WorldPacket const* WorldPackets::LFG::LFGOfferContinue::Write()
{
    _worldPacket << Slot;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::SlotInvalid::Write()
{
    _worldPacket << Reason;
    _worldPacket << SubReason1;
    _worldPacket << SubReason2;

    return &_worldPacket;
}

void WorldPackets::LFG::CompleteReadyCheck::Read()
{
    _worldPacket >> PartyIndex;
    IsReady = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::LFG::ReadyCheckResult::Write()
{
    _worldPacket << PlayerGuid;
    _worldPacket.WriteBit(IsReady);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::ReadyCheckUpdate::Write()
{
    _worldPacket << UnkByte;
    _worldPacket << UnkByte2;
    _worldPacket << UnkLong;
    _worldPacket << static_cast<uint32>(Data.size());
    _worldPacket.WriteBit(IsCompleted);
    _worldPacket.FlushBits();

    for (auto const& v : Data)
    {
        _worldPacket << v.PlayerGuid;
        _worldPacket.WriteBit(v.IsReady);
        _worldPacket.WriteBit(v.UnkBit);
        _worldPacket.FlushBits();
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::LFG::SetFastLaunchResult::Write()
{
    _worldPacket.WriteBit(Set);
    _worldPacket.FlushBits();

    return &_worldPacket;
}
