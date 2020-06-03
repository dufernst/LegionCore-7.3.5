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

#include "BattlegroundPackets.h"

WorldPacket const* WorldPackets::Battleground::PVPSeason::Write()
{
    _worldPacket << uint32(CurrentSeason);
    _worldPacket << uint32(PreviousSeason);

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::BattlefieldStatusHeader const& header)
{
    data << header.Ticket;

    data << header.QueueID;
    data << header.RangeMin;
    data << header.RangeMax;
    data << header.TeamSize;
    data << header.InstanceID;

    data.WriteBit(header.RegisteredMatch);
    data.WriteBit(header.TournamentRules);
    data.FlushBits();

    return data;
}

WorldPacket const* WorldPackets::Battleground::RatedInfo::Write()
{
    for (auto & info : Info)
    {
        _worldPacket << info.PersonalRating;
        _worldPacket << info.Ranking;
        _worldPacket << info.SeasonPlayed;
        _worldPacket << info.SeasonWon;
        _worldPacket << info.SeasonPlayed;
        _worldPacket << info.SeasonWon;
        _worldPacket << info.WeeklyPlayed;
        _worldPacket << info.WeeklyWon;
        _worldPacket << info.BestWeeklyRating;
        _worldPacket << info.BestWeeklyLastRating;
        _worldPacket << info.BestSeasonRating;
    }

    return &_worldPacket;
}

void WorldPackets::Battleground::ListClient::Read()
{
    _worldPacket >> ListID;
}

void WorldPackets::Battleground::Port::Read()
{
    _worldPacket >> Ticket;
    AcceptedInvite = _worldPacket.ReadBit();
}

void WorldPackets::Battleground::Join::Read()
{
    _worldPacket >> QueueID;
    _worldPacket >> RolesMask;

    for (auto& blackList : BlacklistMap.map)
        _worldPacket >> blackList;

    JoinAsGroup = _worldPacket.ReadBit();
}

void WorldPackets::Battleground::JoinArena::Read()
{
    _worldPacket >> TeamSizeIndex;
    _worldPacket >> Roles;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::PVPLogData::RatingData const& ratingData)
{
    data.append(ratingData.Prematch, 2);
    data.append(ratingData.Postmatch, 2);
    data.append(ratingData.PrematchMMR, 2);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::PVPLogData::HonorData const& honorData)
{
    data << uint32(honorData.HonorKills);
    data << uint32(honorData.Deaths);
    data << uint32(honorData.ContributionPoints);

    return data;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::PVPLogData::PlayerData const& playerData)
{
    data << playerData.PlayerGUID;
    data << uint32(playerData.Kills);
    data << uint32(playerData.DamageDone);
    data << uint32(playerData.HealingDone);
    data << static_cast<uint32>(playerData.Stats.size());
    data << int32(playerData.PrimaryTalentTree);
    data << uint32(playerData.PrimaryTalentTreeNameIndex);
    data << uint32(playerData.Race);
    data << uint32(playerData.PrestigeRank);
    if (!playerData.Stats.empty())
        data.append(playerData.Stats.data(), playerData.Stats.size());

    data.WriteBit(playerData.Faction);
    data.WriteBit(playerData.IsInWorld);
    data.WriteBit(playerData.Honor.is_initialized());
    data.WriteBit(playerData.PreMatchRating.is_initialized());
    data.WriteBit(playerData.RatingChange.is_initialized());
    data.WriteBit(playerData.PreMatchMMR.is_initialized());
    data.WriteBit(playerData.MmrChange.is_initialized());
    data.FlushBits();

    if (playerData.Honor)
        data << *playerData.Honor;

    if (playerData.PreMatchRating)
        data << uint32(*playerData.PreMatchRating);

    if (playerData.RatingChange)
        data << uint32(*playerData.RatingChange);

    if (playerData.PreMatchMMR)
        data << uint32(*playerData.PreMatchMMR);

    if (playerData.MmrChange)
        data << uint32(*playerData.MmrChange);

    return data;
}

WorldPacket const* WorldPackets::Battleground::PVPLogData::Write()
{
    _worldPacket.reserve(Players.size() * sizeof(PlayerData) + sizeof(PVPLogData));

    _worldPacket.WriteBit(Ratings.is_initialized());
    _worldPacket.WriteBit(Winner.is_initialized());
    _worldPacket << static_cast<uint32>(Players.size());
    _worldPacket.append(PlayerCount, 2);

    if (Ratings)
        _worldPacket << *Ratings;

    if (Winner)
        _worldPacket << *Winner;

    for (PlayerData const& player : Players)
        _worldPacket << player;

    return &_worldPacket;
}

void WorldPackets::Battleground::AreaSpiritHealerQueue::Read()
{
    _worldPacket >> HealerGuid;
}

void WorldPackets::Battleground::AreaSpiritHealerQuery::Read()
{
    _worldPacket >> HealerGuid;
}

WorldPacket const* WorldPackets::Battleground::AreaSpiritHealerTime::Write()
{
    _worldPacket << HealerGuid;
    _worldPacket << TimeLeft;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::ReportPvPPlayerAFKResult::Write()
{
    _worldPacket << Offender;
    _worldPacket << Result;
    _worldPacket << NumBlackMarksOnOffender;
    _worldPacket << NumPlayersIHaveReported;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldList::Write()
{
    _worldPacket << BattlemasterGuid;
    _worldPacket << BattlemasterListID;
    _worldPacket << MaxLevel;
    _worldPacket << MinLevel;

    _worldPacket << static_cast<uint32>(Battlefields.size());
    for (uint32 const& map : Battlefields)
        _worldPacket << map;

    _worldPacket.WriteBit(PvpAnywhere);
    _worldPacket.WriteBit(HasWinToday);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::PVPOptionsEnabled::Write()
{
    _worldPacket.WriteBit(RatedArenas);
    _worldPacket.WriteBit(ArenaSkirmish);
    _worldPacket.WriteBit(PugBattlegrounds);
    _worldPacket.WriteBit(WargameBattlegrounds);
    _worldPacket.WriteBit(WargameArenas);
    _worldPacket.WriteBit(RatedBattlegrounds);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::RequestPVPRewardsResponse::Write()
{
    _worldPacket << RandomBGRewards;

    _worldPacket.WriteBit(HasRatedBGWinToday);
    _worldPacket.WriteBit(HasArenaSkirmishWinToday);
    _worldPacket.WriteBit(HasArena2v2WinToday);
    _worldPacket.WriteBit(HasArena3v3WinToday);
    _worldPacket.WriteBit(HasBrawlBattlegroundWinToday);
    _worldPacket.WriteBit(HasBrawlArenaWinToday);
    _worldPacket.FlushBits();

    _worldPacket << RatedBGRewards;
    _worldPacket << ArenaSkirmishRewards;
    _worldPacket << ArenaRewards2v2;
    _worldPacket << ArenaRewards3v3;
    _worldPacket << BrawlRewardsBattleground;
    _worldPacket << BrawlRewardsArena;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::PlayerPositions::Write()
{
    _worldPacket << static_cast<uint32>(FlagCarriers.size());
    for (auto& map : FlagCarriers)
    {
        _worldPacket << map.Guid;
        _worldPacket << map.Pos;
        _worldPacket << map.IconID;
        _worldPacket << map.ArenaSlot;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldStatusNone::Write()
{
    _worldPacket << Ticket;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldStatusNeedConfirmation::Write()
{
    _worldPacket << Header;
    _worldPacket << Mapid;
    _worldPacket << Timeout;
    _worldPacket << Role;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldStatusFailed::Write()
{
    _worldPacket << Ticket;
    _worldPacket << QueueID;
    _worldPacket << Reason;
    _worldPacket << ClientID;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::PlayerJoined::Write()
{
    _worldPacket << Guid;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::PlayerLeft::Write()
{
    _worldPacket << Guid;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::Points::Write()
{
    _worldPacket << BgPoints;
    _worldPacket.WriteBit(Team);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::Init::Write()
{
    _worldPacket << ServerTime; // C_PvP.GetArenaCrowdControlInfo
    _worldPacket << MaxPoints;

    return &_worldPacket;
}

ByteBuffer& operator<<(ByteBuffer& data, WorldPackets::Battleground::BattlegroundCapturePointInfoData& info)
{
    data << info.Guid;
    data << info.Pos;
    data << int8(info.NodeState);
    if (info.NodeState == NODE_STATE_HORDE_ASSAULT || info.NodeState == NODE_STATE_ALLIANCE_ASSAULT)
    {
        data << info.CaptureTime;
        data << info.CaptureTotalDuration;
    }

    return data;
}

WorldPacket const* WorldPackets::Battleground::MapObjectivesInit::Write()
{
    _worldPacket << static_cast<uint32>(CapturePointInfo.size());
    for (auto& v : CapturePointInfo)
        _worldPacket << v;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlegroundCapturePointInfo::Write()
{
    _worldPacket << Info;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::StatusWaitForGroups::Write()
{
    _worldPacket << Header;

    _worldPacket << Mapid;
    _worldPacket << Timeout;

    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        _worldPacket << TotalPlayers[i];
        _worldPacket << AwaitingPlayers[i];
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldStatusQueued::Write()
{
    _worldPacket << Header;

    _worldPacket << AverageWaitTime;
    _worldPacket << WaitTime;

    _worldPacket.WriteBit(AsGroup);
    _worldPacket.WriteBit(SuspendedQueue);
    _worldPacket.WriteBit(EligibleForMatchmaking);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::BattlefieldStatusActive::Write()
{
    _worldPacket << Header;

    _worldPacket << Mapid;
    _worldPacket << StartTimer;
    _worldPacket << ShutdownTimer;

    _worldPacket.WriteBit(ArenaFaction);
    _worldPacket.WriteBit(LeftEarly);
    _worldPacket.FlushBits();

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::ConquestFormulaConstants::Write()
{
    _worldPacket << PvpMinCPPerWeek;
    _worldPacket << PvpMaxCPPerWeek;
    _worldPacket << PvpCPBaseCoefficient;
    _worldPacket << PvpCPExpCoefficient;
    _worldPacket << PvpCPNumerator;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::ArenaPrepOpponentSpecializations::Write()
{
    _worldPacket << static_cast<uint32>(Data.size());
    for (auto const& itr : Data)
    {
        _worldPacket << itr.SpecializationID;
        _worldPacket << itr.Unk;
        _worldPacket << itr.Guid;
    }

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::ArenaCrowdControlSpells::Write()
{
    _worldPacket << PlayerGuid;
    _worldPacket << CrowdControlSpellID;

    return &_worldPacket;
}

void WorldPackets::Battleground::JoinSkirmish::Read()
{
    JoinAsGroup = _worldPacket.ReadBit();
    UnkBool = _worldPacket.ReadBit();
    _worldPacket >> RolesMask;
    _worldPacket >> Bracket;
}

void WorldPackets::Battleground::ReportPvPPlayerAFK::Read()
{
    _worldPacket >> Offender;
}

void WorldPackets::Battleground::BattlemasterJoinBrawl::Read()
{
    _worldPacket >> RolesMask;
}

void WorldPackets::Battleground::SetCemetryPreferrence::Read()
{
    _worldPacket >> GraveyardID;
}

void WorldPackets::Battleground::AcceptWargameInvite::Read()
{
    _worldPacket >> OpposingPartyMember;
    _worldPacket >> QueueID;
    Accept = _worldPacket.ReadBit();
}

void WorldPackets::Battleground::StartWargame::Read()
{
    _worldPacket >> OpposingPartyMember;
    _worldPacket >> OpposingPartyMemberVirtualRealmAddress;
    _worldPacket >> UnkShort;
    _worldPacket >> QueueID;
    TournamentRules = _worldPacket.ReadBit();
}

void WorldPackets::Battleground::RequstCrowdControlSpell::Read()
{
    _worldPacket >> PlayerGuid;
}

WorldPacket const* WorldPackets::Battleground::RequestPvpBrawlInfoResponse::Write()
{
    _worldPacket << TimeToEnd;
    _worldPacket << BattlegroundTypeId;
    _worldPacket << BrawlType;

    _worldPacket.WriteBit(IsActive);
    _worldPacket.WriteBits(Name.length(), 9);
    _worldPacket.WriteBits(Description.length(), 10);
    _worldPacket.WriteBits(Description2.length(), 14);
    _worldPacket.FlushBits();

    _worldPacket.WriteString(Name);
    _worldPacket.WriteString(Description);
    _worldPacket.WriteString(Description2);

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::CheckWargameEntry::Write()
{
    _worldPacket << OpposingPartyMember;
    _worldPacket << RealmID;
    _worldPacket << UnkShort;
    _worldPacket << OpposingPartyUserServer;
    _worldPacket << OpposingPartyBnetAccountID;
    _worldPacket << QueueID;
    _worldPacket << TimeoutSeconds;
    _worldPacket << TournamentRules;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Battleground::WargameRequestSuccessfullySentToOpponent::Write()
{
    _worldPacket << UnkInt;
    _worldPacket.WriteBit(UnkInt2.is_initialized());
    _worldPacket.WriteBit(UnkInt3.is_initialized());
    _worldPacket.FlushBits();

    if (UnkInt2.is_initialized())
        _worldPacket << *UnkInt2;

    if (UnkInt3.is_initialized())
        _worldPacket << *UnkInt3;

    return &_worldPacket;
}
