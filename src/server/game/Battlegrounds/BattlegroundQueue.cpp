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

#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "Bracket.h"
#include "Chat.h"
#include "Group.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "LFG.h"
#include "GameEventMgr.h"

BattlegroundQueue::BattlegroundQueue()
{
    for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        for (uint8 j = 0; j < MS::Battlegrounds::MaxBrackets; ++j)
        {
            _sumOfWaitTimes[i][j] = 0;
            _waitTimeLastPlayer[i][j] = 0;
            for (uint32 k = 0; k < MS::Battlegrounds::CountOfPlayersToAverageWaitTime; ++k)
                _waitTimes[i][j][k] = 0;
        }
    }

    for (auto& queuedGroup : _queuedGroups)
        for (auto& j : queuedGroup)
            j.clear();
}

BattlegroundQueue::~BattlegroundQueue()
{
    m_events.KillAllEvents(false);
    KillAllDelayedEvents();

    _queuedPlayers.clear();
    for (auto& queuedGroup : _queuedGroups)
    {
        for (auto& j : queuedGroup)
        {
            for (auto& itr : j)
                delete itr;

            j.clear();
        }
    }
}

BattlegroundQueue::SelectionPool::SelectionPool() : PlayerCount(0)
{
}

void BattlegroundQueue::SelectionPool::Init()
{
    SelectedGroups.clear();
    PlayerCount = 0;
}

bool BattlegroundQueue::SelectionPool::KickGroup(uint32 size)
{
    if (SelectedGroups.empty())
        return true;

    std::map<uint32, std::vector<GroupQueueInfo*>> sortedMap{};

    GroupQueueInfo* selected = SelectedGroups.front();

    for (const auto& info : SelectedGroups)
    {
        sortedMap[info->Players.size()].push_back(info);
        if (info->Players.size() >= size)
            if ((selected->Players.size() >= size && info->Players.size() < selected->Players.size()) || selected->Players.size() < size)
                selected = info;
    }

    if (sortedMap.find(size) != sortedMap.end())
    {
        GroupQueueInfo* info = sortedMap[size].front();
        SelectedGroups.remove(info);
        PlayerCount -= info->Players.size();
        return true;
    }

    for (int32 i = size - 1; i > 0; --i)
    {
        std::vector<GroupQueueInfo*> tempGroups{};
        for (int32 j = i; j > 0; --j)
        {
            if (sortedMap.find(i) != sortedMap.end())
            {
                for (const auto& info : sortedMap[j])
                {
                    if (tempGroups.size() + j <= size + 1)
                        tempGroups.push_back(info);

                    if (abs(static_cast<int32>(tempGroups.size() - size)) <= 1)
                    {
                        for (auto & info2 : tempGroups)
                        {
                            SelectedGroups.remove(info2);
                            PlayerCount -= info2->Players.size();
                        }

                        return true;
                    }
                }
            }
        }
    }

    SelectedGroups.remove(selected);
    PlayerCount -= selected->Players.size();
    return  selected->Players.size() + 1 >= size;
}

uint32 BattlegroundQueue::SelectionPool::GetPlayerCount() const
{
    return PlayerCount;
}

BattlegroundQueue::SelectionPool& BattlegroundQueue::SelectionPool::operator=(const SelectionPool & second)
{
    PlayerCount = second.PlayerCount;
    SelectedGroups = second.SelectedGroups;

    return *this;
}

BattlegroundQueue::CheckMMR::CheckMMR(uint32 mmr, uint32 diff): _mmr(mmr), _diff(diff)
{
}

bool BattlegroundQueue::CheckMMR::operator()(GroupQueueInfo* group)
{
    if (group->IsInvitedToBGInstanceGUID)
        return true;

    return group->MatchmakerRating <= _mmr - _diff || group->MatchmakerRating >= _mmr + _diff;
}

bool BattlegroundQueue::SelectionPool::AddGroup(GroupQueueInfo* ginfo, uint32 desiredCount)
{
    if (!ginfo->IsInvitedToBGInstanceGUID && desiredCount >= PlayerCount + ginfo->Players.size())
    {
        SelectedGroups.push_back(ginfo);
        PlayerCount += ginfo->Players.size();
        return true;
    }

    return PlayerCount < desiredCount;
}

GroupQueueInfo* BattlegroundQueue::AddGroup(Player* leader, Group* grp, uint16 BgTypeId, PVPDifficultyEntry const* bracketEntry, uint8 JoinType, bool isRated, bool isPremade, WorldPackets::Battleground::IgnorMapInfo ignore, uint32 mmr, uint32 _team /*= 0*/)
{
    uint8 bracket = MS::Battlegrounds::GetBracketByJoinType(JoinType);

    auto ginfo = new GroupQueueInfo;
    ginfo->GroupId = grp ? grp->GetGUID().GetCounter() : 0;

    if (JoinType == MS::Battlegrounds::JoinType::Arena1v1 || JoinType == MS::Battlegrounds::JoinType::ArenaSoloQ3v3) 
        ginfo->GroupId = leader->GetGUIDLow();

    ginfo->BgTypeId = BgTypeId;
    ginfo->NativeBgTypeId = BgTypeId;
    ginfo->JoinType = JoinType;
    ginfo->IsRated = isRated;
    ginfo->IsInvitedToBGInstanceGUID = 0;
    ginfo->JoinTime                  = time(nullptr);
    ginfo->RemoveInviteTime          = 0;

    if (_team)
    {
        ginfo->Team = _team; // Wargame
    }
    else if ((sWorld->getBoolConfig(CONFIG_CROSSFACTIONBG) && JoinType == MS::Battlegrounds::JoinType::None) || BgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch)   
    {
        if (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() == m_SelectionPools[TEAM_HORDE].GetPlayerCount())
            ginfo->Team = leader->GetBGTeam();
        else if (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() > m_SelectionPools[TEAM_HORDE].GetPlayerCount())
            ginfo->Team = HORDE;
        else
            ginfo->Team = ALLIANCE;
    }
    else
        ginfo->Team = leader->GetBGTeam();

    ginfo->MatchmakerRating = mmr;
    ginfo->OpponentsMatchmakerRating = 0;
    ginfo->ignore = ignore;
    ginfo->Players.clear();
    ginfo->RoleSoloQ = leader->GetRoleForSoloQ();

    uint32 index = 0;
    if ((!isPremade && (!isRated || BgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch)) || JoinType == MS::Battlegrounds::JoinType::ArenaSoloQ3v3)
        index += MAX_TEAMS;
    if (ginfo->Team == HORDE)
        index++;

    TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "Adding Group to BattlegroundQueue bgTypeId : %u, bracketID : %u, bracket_type: %u, mmr: %i, index : %u", BgTypeId, bracketEntry->RangeIndex, bracket, ginfo->MatchmakerRating, index);

    uint32 lastOnlineTime = time(nullptr);

    if (isRated && sWorld->getBoolConfig(CONFIG_ARENA_QUEUE_ANNOUNCER_ENABLE))
        sWorld->SendWorldText(LANG_ARENA_QUEUE_ANNOUNCE_WORLD_JOIN, ginfo->JoinType, ginfo->JoinType, ginfo->MatchmakerRating);

    if (grp)
    {
        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member)
                continue;

            PlayerQueueInfo& info = _queuedPlayers[member->GetGUID()];
            info.LastOnlineTime = lastOnlineTime;
            info.GroupInfo = ginfo;
            ginfo->Players[member->GetGUID()] = &info;
        }
    }
    else
    {
        PlayerQueueInfo& info = _queuedPlayers[leader->GetGUID()];
        info.LastOnlineTime = lastOnlineTime;
        info.GroupInfo = ginfo;
        ginfo->Players[leader->GetGUID()] = &info;
    }

    _queuedGroups[bracketEntry->RangeIndex][index].push_back(ginfo);

    if (!isRated && !isPremade && sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE))
    {
        if (Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(ginfo->BgTypeId))
        {
            uint32 MinPlayers = bg->GetMinPlayersPerTeam();
            uint32 qHorde = 0;
            uint32 qAlliance = 0;

            for (std::list<GroupQueueInfo*>::const_iterator itr = _queuedGroups[bracketEntry->RangeIndex][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].begin(); itr != _queuedGroups[bracketEntry->RangeIndex][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].end(); ++itr)
                if (!(*itr)->IsInvitedToBGInstanceGUID)
                    qAlliance += (*itr)->Players.size();

            for (std::list<GroupQueueInfo*>::const_iterator itr = _queuedGroups[bracketEntry->RangeIndex][MS::Battlegrounds::QueueGroupTypes::NormalHorde].begin(); itr != _queuedGroups[bracketEntry->RangeIndex][MS::Battlegrounds::QueueGroupTypes::NormalHorde].end(); ++itr)
                if (!(*itr)->IsInvitedToBGInstanceGUID)
                    qHorde += (*itr)->Players.size();

            if (sWorld->getBoolConfig(CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY)) // Show queue status to player only (when joining queue)
                ChatHandler(leader).PSendSysMessage(LANG_BG_QUEUE_ANNOUNCE_SELF, bg->GetName(), bracketEntry->MinLevel, bracketEntry->MaxLevel, qAlliance, MinPlayers > qAlliance ? MinPlayers - qAlliance : 0, qHorde, MinPlayers > qHorde ? MinPlayers - qHorde : 0u);
            else
                sWorld->SendWorldText(LANG_BG_QUEUE_ANNOUNCE_WORLD, bg->GetName(), bracketEntry->MinLevel, bracketEntry->MaxLevel, qAlliance, MinPlayers > qAlliance ? MinPlayers - qAlliance : 0, qHorde, MinPlayers > qHorde ? MinPlayers - qHorde : 0u);
        }
    }

    return ginfo;
}

void BattlegroundQueue::PlayerInvitedToBGUpdateAverageWaitTime(GroupQueueInfo* ginfo, uint8 bracketID)
{
    uint32 timeInQueue = getMSTimeDiff(ginfo->JoinTime, time(nullptr)) * IN_MILLISECONDS;
    uint8 team_index = TEAM_ALLIANCE;
    if (!ginfo->JoinType)
    {
        if (ginfo->Team == HORDE)
            team_index = TEAM_HORDE;
    }
    else
    {
        if (ginfo->IsRated)
            team_index = TEAM_HORDE;
    }

    uint32* lastPlayerAddedPointer = &_waitTimeLastPlayer[team_index][bracketID];
    _sumOfWaitTimes[team_index][bracketID] -= _waitTimes[team_index][bracketID][(*lastPlayerAddedPointer)];
    _waitTimes[team_index][bracketID][(*lastPlayerAddedPointer)] = timeInQueue;
    _sumOfWaitTimes[team_index][bracketID] += timeInQueue;
    (*lastPlayerAddedPointer)++;
    *lastPlayerAddedPointer %= MS::Battlegrounds::CountOfPlayersToAverageWaitTime;
}

uint32 BattlegroundQueue::GetAverageQueueWaitTime(GroupQueueInfo* ginfo, uint8 bracketID) const
{
    if (!ginfo)
        return 0;

    uint8 team_index = TEAM_ALLIANCE;
    if (!ginfo->JoinType)
    {
        if (ginfo->Team == HORDE)
            team_index = TEAM_HORDE;
    }
    else
    {
        if (ginfo->IsRated)
            team_index = TEAM_HORDE;
    }
    if (_waitTimes[team_index][bracketID][MS::Battlegrounds::CountOfPlayersToAverageWaitTime - 1])
        return _sumOfWaitTimes[team_index][bracketID] / MS::Battlegrounds::CountOfPlayersToAverageWaitTime;
    return 0;
}

void BattlegroundQueue::RemovePlayer(ObjectGuid guid, bool decreaseInvitedCount)
{
    AddDelayedEvent(10, [=]() -> void
    {
        if (this)
            RemovePlayerQueue(guid, decreaseInvitedCount);
    });
}

void BattlegroundQueue::RemovePlayerQueue(ObjectGuid guid, bool decreaseInvitedCount)
{
    int32 bracketID = -1;

    auto itr = _queuedPlayers.find(guid);
    if (itr == _queuedPlayers.end())
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue: couldn't find player to remove GUID: %u", guid.GetCounter());
        return;
    }

    GroupQueueInfo* group = itr->second.GroupInfo;
    std::list<GroupQueueInfo*>::iterator group_itr;
    uint32 index = MS::Battlegrounds::GetTeamIdByTeam(group->Team);

    for (uint8 bracket_id_tmp = MS::Battlegrounds::MaxBrackets - 1; bracket_id_tmp >= 0 && bracketID == -1; --bracket_id_tmp)
    {
        for (uint32 j = index; j < MS::Battlegrounds::QueueGroupTypes::Max; j += MAX_TEAMS)
        {
            for (auto group_itr_tmp = _queuedGroups[bracket_id_tmp][j].begin(); group_itr_tmp != _queuedGroups[bracket_id_tmp][j].end(); ++group_itr_tmp)
            {
                if (*group_itr_tmp && *group_itr_tmp == group)
                {
                    bracketID = bracket_id_tmp;
                    group_itr = group_itr_tmp;
                    index = j;
                    break;
                }
            }
        }
    }

    if (bracketID == -1)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue: ERROR Cannot find groupinfo for player GUID: %u", guid.GetCounter());
        return;
    }

    auto pitr = group->Players.find(guid);
    if (pitr != group->Players.end())
        group->Players.erase(pitr);

    if (decreaseInvitedCount && group->IsInvitedToBGInstanceGUID)
    {
        auto isRandomGenerated = false;
        
        switch (group->BgTypeId)
        {
            case MS::Battlegrounds::BattlegroundTypeId::ArenaAll:
            case MS::Battlegrounds::BattlegroundTypeId::RatedBattleground:
            case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll:
            case MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix:
            // random battlegrounds?
                isRandomGenerated = true;
            default:
                break;
        }

        if (Battleground* bg = sBattlegroundMgr->GetBattleground(group->IsInvitedToBGInstanceGUID, isRandomGenerated ? MS::Battlegrounds::BattlegroundTypeId::None : group->BgTypeId))
            bg->DecreaseInvitedCount(group->Team);
    }

    _queuedPlayers.erase(itr);

    if (group->JoinType && group->IsRated && group->Players.empty() && sWorld->getBoolConfig(CONFIG_ARENA_QUEUE_ANNOUNCER_ENABLE))
        sWorld->SendWorldText(LANG_ARENA_QUEUE_ANNOUNCE_WORLD_EXIT, group->JoinType, group->JoinType, group->MatchmakerRating);

    if (group->Players.empty())
    {
        _queuedGroups[bracketID][index].erase(group_itr);
        delete group;
    }
    else if (!group->IsInvitedToBGInstanceGUID && group->IsRated)
    {
        if (Player* plr2 = ObjectAccessor::FindPlayer(group->Players.begin()->first))
        {
            Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(group->BgTypeId);
            uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(group->BgTypeId, group->JoinType);
            uint32 queueSlot = plr2->GetBattlegroundQueueIndex(bgQueueTypeId);

            WorldPackets::Battleground::BattlefieldStatusNone battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusNone(&battlefieldStatus, plr2, queueSlot, plr2->GetBattlegroundQueueJoinTime(bgQueueTypeId));
            plr2->SendDirectMessage(battlefieldStatus.Write());

            WorldPackets::Battleground::BattlefieldStatusFailed failed;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&failed, bg, plr2, queueSlot, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LEAVE_QUEUE);
            plr2->SendDirectMessage(failed.Write());

            plr2->RemoveBattlegroundQueueId(bgQueueTypeId);
        }

        RemovePlayer(group->Players.begin()->first, decreaseInvitedCount);
    }
}

bool BattlegroundQueue::IsPlayerInvited(ObjectGuid pl_guid, uint32 const bgInstanceGuid, uint32 const removeTime)
{
    std::map<ObjectGuid, PlayerQueueInfo>::const_iterator qItr = _queuedPlayers.find(pl_guid);
    return qItr != _queuedPlayers.end() && qItr->second.GroupInfo->IsInvitedToBGInstanceGUID == bgInstanceGuid && qItr->second.GroupInfo->RemoveInviteTime == removeTime;
}

bool BattlegroundQueue::GetPlayerGroupInfoData(ObjectGuid guid, GroupQueueInfo* ginfo)
{
    std::map<ObjectGuid, PlayerQueueInfo>::const_iterator qItr = _queuedPlayers.find(guid);
    if (qItr == _queuedPlayers.end())
        return false;

    *ginfo = *qItr->second.GroupInfo;

    return true;
}

BattlegroundQueue::SortMMR::SortMMR() = default;

bool BattlegroundQueue::SortMMR::operator()(GroupQueueInfo* groupA, GroupQueueInfo* groupB)
{
    return groupA->MatchmakerRating < groupB->MatchmakerRating;
}

bool BattlegroundQueue::InviteGroupToBG(GroupQueueInfo* ginfo, Battleground* bg, uint32 side)
{
    if (side)
        ginfo->Team = side;

    if (!ginfo->IsInvitedToBGInstanceGUID)
    {
        ginfo->IsInvitedToBGInstanceGUID = bg->GetInstanceID();
        ginfo->NativeBgTypeId = bg->GetTypeID();
        uint16 bgTypeId = bg->GetTypeID();
        uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(ginfo->BgTypeId, bg->GetJoinType());
        uint8 bracketID = bg->GetBracketId();

        if (bg->IsArena() && bg->IsRated())
            bg->SetGroupForTeam(ginfo->Team, ginfo->GroupId);

        ginfo->RemoveInviteTime = time(nullptr) + (bg->IsArena() ? ARENA_INVITE_ACCEPT_WAIT_TIME : BG_INVITE_ACCEPT_WAIT_TIME);

        for (auto itr = ginfo->Players.begin(); itr != ginfo->Players.end(); ++itr)
        {
            Player* player = ObjectAccessor::FindPlayer(itr->first);
            if (!player || player->InBattleground())
                continue;

            PlayerInvitedToBGUpdateAverageWaitTime(ginfo, bracketID);
            bg->IncreaseInvitedCount(ginfo->Team);

            player->SetInviteForBattlegroundQueueType(bgQueueTypeId, ginfo->IsInvitedToBGInstanceGUID);

            m_events.AddEvent(new BGQueueInviteEvent(player->GetGUID(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, ginfo->RemoveInviteTime), m_events.CalculateTime(INVITATION_REMIND_TIME));
            m_events.AddEvent(new BGQueueRemoveEvent(player->GetGUID(), ginfo->IsInvitedToBGInstanceGUID, bgTypeId, bgQueueTypeId, ginfo->RemoveInviteTime), m_events.CalculateTime(bg->IsArena() ? ARENA_INVITE_ACCEPT_WAIT_TIME : BG_INVITE_ACCEPT_WAIT_TIME));

            uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);

            TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "Battleground: invited player %s (%u) to BG instance %u queueindex %u bgtype %u, I can't help it if they don't press the enter battle button.", player->GetName(), player->GetGUID().GetCounter(), bg->GetInstanceID(), queueSlot, bg->GetTypeID());

            WorldPackets::Battleground::BattlefieldStatusNeedConfirmation battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusNeedConfirmation(&battlefieldStatus, bg, player, queueSlot, player->GetBattlegroundQueueJoinTime(bgQueueTypeId), bg->IsArena() ? ARENA_INVITE_ACCEPT_WAIT_TIME : BG_INVITE_ACCEPT_WAIT_TIME, ginfo->JoinType, bracketID);
            player->SendDirectMessage(battlefieldStatus.Write());
        }
        return true;
    }

    return false;
}

void BattlegroundQueue::AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
{
    m_Functions.AddDelayedEvent(timeOffset, std::move(function));
}

void BattlegroundQueue::KillAllDelayedEvents()
{
    m_Functions.KillAllFunctions();
}

void BattlegroundQueue::FillPlayersToBG(Battleground* bg, uint8 bracketID)
{
    int32 hordeFree = bg->GetFreeSlotsForTeam(HORDE);
    int32 aliFree = bg->GetFreeSlotsForTeam(ALLIANCE);

    std::list<GroupQueueInfo*>::const_iterator Ali_itr = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].begin();
    uint32 aliCount = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].size();
    uint32 aliIndex = 0;
    for (; aliIndex < aliCount && m_SelectionPools[TEAM_ALLIANCE].AddGroup(*Ali_itr, aliFree); aliIndex++)
        ++Ali_itr;

    std::list<GroupQueueInfo*>::const_iterator Horde_itr = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde].begin();
    uint32 hordeCount = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde].size();
    uint32 hordeIndex = 0;
    for (; hordeIndex < hordeCount && m_SelectionPools[TEAM_HORDE].AddGroup(*Horde_itr, hordeFree); hordeIndex++)
        ++Horde_itr;

    if (sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) == 0)
        return;

    int32 diffAli = aliFree - int32(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount());
    int32 diffHorde = hordeFree - int32(m_SelectionPools[TEAM_HORDE].GetPlayerCount());
    while (abs(diffAli - diffHorde) > 1 && (m_SelectionPools[TEAM_HORDE].GetPlayerCount() > 0 || m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() > 0))
    {
        if (diffAli < diffHorde)
        {
            if (m_SelectionPools[TEAM_ALLIANCE].KickGroup(diffHorde - diffAli))
            {
                for (; aliIndex < aliCount && m_SelectionPools[TEAM_ALLIANCE].AddGroup(*Ali_itr, aliFree >= diffHorde ? aliFree - diffHorde : 0); aliIndex++)
                    ++Ali_itr;
            }

            if (!m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())
            {
                if (aliFree <= diffHorde + 1)
                    break;
                m_SelectionPools[TEAM_HORDE].KickGroup(diffHorde - diffAli);
            }
        }
        else
        {
            if (m_SelectionPools[TEAM_HORDE].KickGroup(diffAli - diffHorde))
            {
                for (; hordeIndex < hordeCount && m_SelectionPools[TEAM_HORDE].AddGroup(*Horde_itr, hordeFree >= diffAli ? hordeFree - diffAli : 0); hordeIndex++)
                    ++Horde_itr;
            }

            if (!m_SelectionPools[TEAM_HORDE].GetPlayerCount())
            {
                if (hordeFree <= diffAli + 1)
                    break;
                m_SelectionPools[TEAM_ALLIANCE].KickGroup(diffAli - diffHorde);
            }
        }

        diffAli = aliFree - int32(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount());
        diffHorde = hordeFree - int32(m_SelectionPools[TEAM_HORDE].GetPlayerCount());
    }
}

bool BattlegroundQueue::CheckPremadeMatch(uint8 bracketID, uint32 MinPlayersPerTeam, uint32 MaxPlayersPerTeam)
{
    if (!_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].empty() && !_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].empty())
    {
        std::list<GroupQueueInfo*>::const_iterator ali_group, horde_group;
        for (ali_group = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].begin(); ali_group != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].end(); ++ali_group)
            if (!(*ali_group)->IsInvitedToBGInstanceGUID)
                break;

        for (horde_group = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].begin(); horde_group != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].end(); ++horde_group)
            if (!(*horde_group)->IsInvitedToBGInstanceGUID)
                break;

        if (ali_group != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].end() && horde_group != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].end())
        {
            m_SelectionPools[TEAM_ALLIANCE].AddGroup(*ali_group, MaxPlayersPerTeam);
            m_SelectionPools[TEAM_HORDE].AddGroup(*horde_group, MaxPlayersPerTeam);
            uint32 maxPlayers = std::min(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount(), m_SelectionPools[TEAM_HORDE].GetPlayerCount());
            for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
                for (std::list<GroupQueueInfo*>::const_iterator itr = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].begin(); itr != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].end(); ++itr)
                    if (!(*itr)->IsInvitedToBGInstanceGUID && !m_SelectionPools[i].AddGroup(*itr, maxPlayers))
                        break;

            return true;
        }
    }

    uint32 time_before = time(nullptr) - sWorld->getIntConfig(CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH);
    for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
    {
        if (!_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance + i].empty())
        {
            auto itr = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance + i].begin();
            if (!(*itr)->IsInvitedToBGInstanceGUID && ((*itr)->JoinTime < time_before || (*itr)->Players.size() < MinPlayersPerTeam))
            {
                _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].push_front(*itr);
                _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance + i].erase(itr);
            }
        }
    }

    return false;
}

bool BattlegroundQueue::CheckNormalMatchDeathMatch(uint8 bracketID, uint32 MinPlayers, uint32 MaxPlayers)
{
    if (_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].size() + _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde].size() >= MinPlayers)
    {
        for (auto & ali_group : _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance])
            if (!ali_group->IsInvitedToBGInstanceGUID && !m_SelectionPools[TEAM_ALLIANCE].AddGroup(ali_group, MaxPlayers))
                break;

        for (auto & horde_group : _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde])
            if (!horde_group->IsInvitedToBGInstanceGUID && !m_SelectionPools[TEAM_HORDE].AddGroup(horde_group, MaxPlayers))
                break;

        return true;
    }
    return false;
}

void ChooseRoleForTeam(std::vector<Player*>& players, uint8 neededRole, uint8 count, uint8 bracketId)
{
    for (Player* player : players)
    {
        if (player->GetQueueRoleMask(bracketId) & neededRole)
        {
            player->SetQueueRoleMask(bracketId, neededRole, true);
            if (--count == 0)
                return;
        }
    }
}

bool BattlegroundQueue::CheckNormalMatch(Battleground* bg_template, uint8 bracketID, uint32 minPlayers, uint32 maxPlayers)
{
    SelectionPool tempSelectionPools[MAX_TEAMS];
    std::list<GroupQueueInfo*>::const_iterator itr_team[MAX_TEAMS];

    for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
    {
        tempSelectionPools[i].Init();
        itr_team[i] = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].begin();
        for (; itr_team[i] != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].end(); ++itr_team[i])
        {
            if (!(*itr_team[i])->IsInvitedToBGInstanceGUID)
            {
                if (m_SelectionPools[i].GetPlayerCount() < minPlayers)
                    m_SelectionPools[i].AddGroup(*itr_team[i], maxPlayers);
                tempSelectionPools[i].AddGroup(*itr_team[i], maxPlayers);
            }
        }
    }

    if (sBattlegroundMgr->isTesting() && bg_template->IsBattleground() && (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() || m_SelectionPools[TEAM_HORDE].GetPlayerCount()))
        return true;

    if (m_SelectionPools[TEAM_HORDE].GetPlayerCount() < minPlayers && m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() < minPlayers)
        return false;    

    uint32 j = TEAM_ALLIANCE;
    if (m_SelectionPools[TEAM_HORDE].GetPlayerCount() < m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())
        j = TEAM_HORDE;

    if (sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) == 1) // old system
    {
        ++itr_team[j];
        for (; itr_team[j] != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + j].end(); ++itr_team[j])
        {
            if (!(*itr_team[j])->IsInvitedToBGInstanceGUID)
                if (!m_SelectionPools[j].AddGroup(*itr_team[j], m_SelectionPools[(j + 1) % MAX_TEAMS].GetPlayerCount()))
                    break;
        }

        if (abs(static_cast<int32>(m_SelectionPools[TEAM_HORDE].GetPlayerCount() - m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount())) > 2)
            return false;
    }
    else if (sWorld->getIntConfig(CONFIG_BATTLEGROUND_INVITATION_TYPE) == 2) // new experemental system
    {
        int16 diff = abs(static_cast<int32>(tempSelectionPools[TEAM_HORDE].GetPlayerCount() - tempSelectionPools[TEAM_ALLIANCE].GetPlayerCount()));

        while (tempSelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && tempSelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers && diff > 2)
        {
            if (diff > 2)
            {
                if (tempSelectionPools[TEAM_HORDE].GetPlayerCount() > tempSelectionPools[TEAM_ALLIANCE].GetPlayerCount())
                    tempSelectionPools[TEAM_HORDE].KickGroup(diff);
                else
                    tempSelectionPools[TEAM_ALLIANCE].KickGroup(diff);
            }

            diff = abs(static_cast<int32>(tempSelectionPools[TEAM_HORDE].GetPlayerCount() - tempSelectionPools[TEAM_ALLIANCE].GetPlayerCount()));
        }

        if (tempSelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && tempSelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers)
        {
            m_SelectionPools[TEAM_ALLIANCE] = tempSelectionPools[TEAM_ALLIANCE];
            m_SelectionPools[TEAM_HORDE] = tempSelectionPools[TEAM_HORDE];
        }
    }

    if (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && m_SelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers)
    {
        std::vector<Player*> playersWithMultiplyRoles[MAX_TEAMS]; // tank-healer-dd
        std::map<uint8, uint8> rolesChooseSimply[MAX_TEAMS]; // tank-healer-dd

        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        {
            playersWithMultiplyRoles[team].clear();
            rolesChooseSimply[team].clear();

            for (auto& groups : m_SelectionPools[team].SelectedGroups)
            {
                for (auto& pair : groups->Players)
                {
                    if (Player * player = ObjectAccessor::FindPlayer(pair.first))
                    {
                        int8 role = player->GetSingleQueueRole(bracketID);
                        if (role < 0) // not single role
                            playersWithMultiplyRoles[team].push_back(player);
                        else
                            ++rolesChooseSimply[team][role];
                    }
                }
            }
        }

        uint8 rolesArray[3]{ lfg::LfgRoles::PLAYER_ROLE_TANK , lfg::LfgRoles::PLAYER_ROLE_HEALER, lfg::LfgRoles::PLAYER_ROLE_DAMAGE };

        for (uint8 role : {ROLES_HEALER, ROLES_TANK})
        {
            uint8 diff = abs(rolesChooseSimply[TEAM_ALLIANCE][role] - rolesChooseSimply[TEAM_HORDE][role]);
            if (diff > 0)
                ChooseRoleForTeam(playersWithMultiplyRoles[rolesChooseSimply[TEAM_HORDE][role] < rolesChooseSimply[TEAM_ALLIANCE][role]], rolesArray[role], diff, bracketID);
        }

        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i) // other players move to dd
            ChooseRoleForTeam(playersWithMultiplyRoles[i], lfg::LfgRoles::PLAYER_ROLE_DAMAGE, playersWithMultiplyRoles[i].size(), bracketID);

    }

    return m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() >= minPlayers && m_SelectionPools[TEAM_HORDE].GetPlayerCount() >= minPlayers;
}

bool BattlegroundQueue::TryChooseCommandWithRoles(uint8 bracketID, uint8 teamid, uint8 healers, uint8 tanks, uint8 dd, uint8 teamIdPool)
{
    uint8 minPlayers = healers + tanks + dd;

    m_SelectionPools[teamIdPool].Init();

    auto itr_team = _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + teamid].begin();
    for (; itr_team != _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + teamid].end(); ++itr_team)
    {
        if (!(*itr_team)->IsInvitedToBGInstanceGUID)
        {
            if (std::find(m_SelectionPools[teamIdPool^1].SelectedGroups.begin(), m_SelectionPools[teamIdPool^1].SelectedGroups.end(), (*itr_team)) != m_SelectionPools[teamIdPool^1].SelectedGroups.end()) // for same factions
                continue;

            uint8 tempHealers = healers;
            uint8 tempTanks = tanks;
            uint8 tempDd = dd;
            bool canUseTeam = true;

            std::list<Player*> players{};
            for (auto playerGuid : (*itr_team)->Players)
                if (Player* player = ObjectAccessor::FindPlayer(playerGuid.first))
                    players.push_back(player);

            players.sort([bracketID](Player* left, Player* right) // try to set at start players with single roles
            {
                return left->GetSingleQueueRole(bracketID) > right->GetSingleQueueRole(bracketID);
            });

            for (auto player : players)
            {
                uint8 roleMask = player->GetQueueRoleMask(bracketID);
                if (tempHealers > 0 && roleMask & lfg::PLAYER_ROLE_HEALER)
                {
                    --tempHealers;
                    player->SetQueueRoleMask(bracketID, lfg::PLAYER_ROLE_HEALER, true);
                }
                else if (tempTanks > 0 && roleMask & lfg::PLAYER_ROLE_TANK)
                {
                    --tempTanks;
                    player->SetQueueRoleMask(bracketID, lfg::PLAYER_ROLE_TANK, true);
                }
                else if (tempDd > 0 && roleMask & lfg::PLAYER_ROLE_DAMAGE)
                {
                    --tempDd;
                    player->SetQueueRoleMask(bracketID, lfg::PLAYER_ROLE_DAMAGE, true);
                }
                else
                {
                    canUseTeam = false;
                    break;
                }
            }

            if (!canUseTeam)
                continue;

            if (m_SelectionPools[teamIdPool].GetPlayerCount() < minPlayers)
            {
                if (m_SelectionPools[teamIdPool].AddGroup(*itr_team, minPlayers))
                {
                    healers = tempHealers;
                    tanks = tempTanks;
                    dd = tempDd;
                }
            }
            else
                break;
        }
    }

    return m_SelectionPools[teamIdPool].GetPlayerCount() == minPlayers;
}

bool BattlegroundQueue::CheckSkirmishOrLFGBrawl(uint8 bracketID, bool isSkirmish /*=true*/)
{
    uint32 combinations[6][3] =// healers-tank-dd
    {
        // skirmish
        {1, 1, 3},

        // 3v3
        {1, 1, 1},
        {1, 0, 2},
        {0, 1, 2},

        // 2v2
        {0, 1, 1},
        { 0,0,2 },
    };

    for (uint8 j = isSkirmish; j < (isSkirmish ? 6 : 1); ++j)
    {
        bool findIt = true;
        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        {
            if (!TryChooseCommandWithRoles(bracketID, i, combinations[j][0], combinations[j][1], combinations[j][2], i))
            {
                findIt = false;
                break;
            }
        }
        if (findIt || (sBattlegroundMgr->isTesting() && (m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount() || m_SelectionPools[TEAM_HORDE].GetPlayerCount())))
            return true;
    }

    // if not find, start again for getting same faction teams
    for (uint8 j = isSkirmish; j < (isSkirmish ? 6 : 1); ++j)
    {
        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i) // select real team
        {
            bool findIt = true;
            m_SelectionPools[TEAM_ALLIANCE].Init();
            m_SelectionPools[TEAM_HORDE].Init();
            for (uint8 teamidPool = TEAM_ALLIANCE; teamidPool < MAX_TEAMS; ++teamidPool) // select team for pool
            {
                if (!TryChooseCommandWithRoles(bracketID, i, combinations[j][0], combinations[j][1], combinations[j][2], teamidPool))
                {
                    findIt = false;
                    break;
                }
            }

            if (findIt)
            {
                uint8 otherTeam = i ^ 1;
                for (auto itr = m_SelectionPools[otherTeam].SelectedGroups.begin(); itr != m_SelectionPools[otherTeam].SelectedGroups.end(); ++itr)
                {
                    (*itr)->Team = otherTeam == TEAM_ALLIANCE ? ALLIANCE : HORDE;
                    _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + otherTeam].push_front(*itr);

                    _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i].remove(*itr);
                }
                return true;
            }
        }
    }

    return false;
}

bool BattlegroundQueue::CalculateSoloQTeams(uint8 bracketID)
{
    std::map<uint8, std::list<GroupQueueInfo*>> specTeamsMap;

    if (_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].empty() && _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde].empty())
        return false;

    for (uint8 i = TEAM_ALLIANCE; i <= TEAM_HORDE; ++i)
    {
        for (auto & itr : _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance + i])
        {
            if (!itr->IsInvitedToBGInstanceGUID)
                specTeamsMap[itr->RoleSoloQ].push_back(itr);
        }
    }

    for (uint8 i = 0; i < 3; ++i)
        specTeamsMap[i + 1].sort(SortMMR());

    for (std::list<GroupQueueInfo*>::const_iterator itr = specTeamsMap[1].begin(); itr != specTeamsMap[1].end();)
    {
        if ((*itr)->IsInvitedToBGInstanceGUID)
        {
            ++itr;
            continue;
        }

        // Melee[0] -> Range[1] -> Heal[2]
        std::list<GroupQueueInfo*> SpecList[3];
        for (uint8 i = 0; i < 3; ++i)
        {
            SpecList[i] = specTeamsMap[i + 1];
            SpecList[i].remove_if(CheckMMR((*itr)->MatchmakerRating, 150));
        }

        if (SpecList[0].size() < 2 || SpecList[1].size() < 2 || SpecList[2].size() < 2)
        {
            ++itr;
            continue;
        }

        std::list<GroupQueueInfo*> groupA;
        std::list<GroupQueueInfo*> groupH;

        for (uint8 i = 0; i < 3; ++i)
        {
            groupA.push_back(SpecList[i].front());
            specTeamsMap[i + 1].remove(SpecList[i].front());
            SpecList[i].pop_front();

            groupH.push_back(SpecList[i].front());
            specTeamsMap[i + 1].remove(SpecList[i].front());
        }

        GroupQueueInfo* group1 = nullptr;
        GroupQueueInfo* group2 = nullptr;

        for (std::list<GroupQueueInfo*>::const_iterator iter = groupA.begin(); iter != groupA.end(); ++iter)
        {
            uint8 QGroupTypes = (*iter)->Team == ALLIANCE ? MS::Battlegrounds::QueueGroupTypes::NormalAlliance : MS::Battlegrounds::QueueGroupTypes::NormalHorde;
            _queuedGroups[bracketID][QGroupTypes].remove(*iter);

            if (!group1)
            {
                group1 = *iter;
                continue;
            }
            PlayerQueueInfo& info = _queuedPlayers[(*iter)->Players.begin()->first];
            info.LastOnlineTime = time(nullptr);
            info.GroupInfo = group1;
            group1->Players[(*iter)->Players.begin()->first] = &info;
        }
        
        for (std::list<GroupQueueInfo*>::const_iterator iter = groupH.begin(); iter != groupH.end(); ++iter)
        {
            uint8 QGroupTypes = (*iter)->Team == ALLIANCE ? MS::Battlegrounds::QueueGroupTypes::NormalAlliance : MS::Battlegrounds::QueueGroupTypes::NormalHorde;
            _queuedGroups[bracketID][QGroupTypes].remove(*iter);

            if (!group2)
            {
                group2 = *iter;
                continue;
            }
            PlayerQueueInfo& info = _queuedPlayers[(*iter)->Players.begin()->first];
            info.LastOnlineTime = time(nullptr);
            info.GroupInfo = group2;
            group2->Players[(*iter)->Players.begin()->first] = &info;
        }

        if (group1 && group2)
        {
            group1->Team = ALLIANCE;
            group2->Team = HORDE;
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].push_back(group1);
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].push_back(group2);
        }

        itr = specTeamsMap[1].begin();
    }

    return !_queuedGroups[bracketID][TEAM_ALLIANCE].empty() && !_queuedGroups[bracketID][TEAM_HORDE].empty();
}

bool BattlegroundQueue::SelectRatedTeams(uint32 bracket_id, GroupQueueInfo* & team1, GroupQueueInfo* & team2)
{
    uint32 discardTime = time(nullptr) >= sBattlegroundMgr->GetRatingDiscardTimer() ? time(nullptr) - sBattlegroundMgr->GetRatingDiscardTimer() : 0;

    GroupQueueInfo* selectedTeam1 = nullptr;
    GroupQueueInfo* selectedTeam2 = nullptr;

    std::vector<GroupQueueInfo*> lowRatingTeams;

    for (uint8 i = MS::Battlegrounds::QueueGroupTypes::PremadeAlliance; i < MS::Battlegrounds::QueueGroupTypes::NormalAlliance; i++)
    {
        for (std::list<GroupQueueInfo*>::const_iterator itr = _queuedGroups[bracket_id][i].begin(); itr != _queuedGroups[bracket_id][i].end(); ++itr)
        {
            lowRatingTeams.push_back(*itr);
        }
    }

    for (std::vector<GroupQueueInfo*>::const_iterator itr = lowRatingTeams.begin(); itr != lowRatingTeams.end(); ++itr)
    {
        GroupQueueInfo* selectedTeam = *itr;
        if (selectedTeam->IsInvitedToBGInstanceGUID)
            continue;

        uint32 selectedRating = selectedTeam->MatchmakerRating;

        float periodic = sGameEventMgr->IsActiveEvent(194) ? 300 : 60; // if it is night time -> period longer for avoid sleavers
        float mmrSteps = floor(float((time(nullptr) - selectedTeam->JoinTime) / periodic)); 
        uint32 mmrMaxDiff = mmrSteps * 100;

        uint32 MinRating = (selectedRating <= sBattlegroundMgr->GetMaxRatingDifference()) ? 0 : selectedRating - sBattlegroundMgr->GetMaxRatingDifference();
        uint32 MaxRating = selectedRating + sBattlegroundMgr->GetMaxRatingDifference();
        if (mmrMaxDiff > 0)
        {
            MinRating = (mmrMaxDiff < MinRating) ? MinRating - mmrMaxDiff : 0;
            MaxRating = mmrMaxDiff + MaxRating;
        }

        std::list<GroupQueueInfo*> randomTeams;

        for (std::vector<GroupQueueInfo*>::const_iterator itr2 = lowRatingTeams.begin(); itr2 != lowRatingTeams.end(); ++itr2)
        {
            GroupQueueInfo* checkTeam = *itr2;

            if (checkTeam->IsInvitedToBGInstanceGUID)
                continue;

            if (selectedTeam == checkTeam)
                continue;

            // the second cycle is just to check for new available teams and schedule update
            // don't fill selectedTeams in this case
            if ((selectedTeam1 && selectedTeam1 == checkTeam) || (selectedTeam2 && selectedTeam2 == checkTeam))
                continue;

            if ((checkTeam->MatchmakerRating >= MinRating && checkTeam->MatchmakerRating <= MaxRating) || checkTeam->JoinTime < discardTime)
            {
                randomTeams.push_back(checkTeam);
                break;
            }
        }

        // we have found opponents for selectedTeam at first time
        // so select random opponent
        if (!randomTeams.empty() && !selectedTeam1 && !selectedTeam2)
        {
            selectedTeam1 = selectedTeam;
            selectedTeam2 = Trinity::Containers::SelectRandomContainerElement(randomTeams);
            continue;
        }
        // now we have found opponents for another selectedTeam, but this team and opponents cannot be selected now
        // because selectedTeam1 and selectedTeam2 has been already filled
        // so we need scheduleupdate for queue
        if (!randomTeams.empty() && selectedTeam1 && selectedTeam2)
        {
            uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(selectedTeam1->BgTypeId, selectedTeam1->JoinType);
            sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(selectedTeam1->MatchmakerRating, selectedTeam1->JoinType, bgQueueTypeId, selectedTeam1->BgTypeId, bracket_id));
            break;
        }
    }

    if (!selectedTeam1 || !selectedTeam2)
        return false;

    team1 = selectedTeam1;
    team2 = selectedTeam2;

    return true;
}

uint16 BattlegroundQueue::GenerateRandomMap(uint16 bgTypeId)
{
    BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(bgTypeId);
    if (!bl)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue:GenerateRandomMap - BattlemasterListEntry not found for %u", bgTypeId);
        return MS::Battlegrounds::BattlegroundTypeId::None;
    }

    std::map<uint16, uint8>* selectionWeights = nullptr;

    if (bl->InstanceType == MS::Battlegrounds::PvpInstanceType::Arena)
        selectionWeights = sBattlegroundMgr->GetSelectionWeight(MS::Battlegrounds::IternalPvpTypes::Arena);
    else if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom || bgTypeId == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground)
        selectionWeights = sBattlegroundMgr->GetSelectionWeight(MS::Battlegrounds::IternalPvpTypes::Battleground);
    else if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll)
        selectionWeights = sBattlegroundMgr->GetSelectionWeight(MS::Battlegrounds::IternalPvpTypes::Brawl);
    else if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
        selectionWeights = sBattlegroundMgr->GetSelectionWeight(MS::Battlegrounds::IternalPvpTypes::BrawlSix);

    if (!selectionWeights || selectionWeights->empty())
        return MS::Battlegrounds::BattlegroundTypeId::None;

    uint32 weight = 0;

    std::map<uint32, uint32> plrIgnoreWeights;
    for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
        for (std::list<GroupQueueInfo*>::const_iterator group = m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.begin(); group != m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.end(); ++group)
            for (uint32 j : (*group)->ignore.map)
                if (j)
                    plrIgnoreWeights[j] += 1;

    uint8 players = std::min(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount(), m_SelectionPools[TEAM_HORDE].GetPlayerCount());
    std::map<uint16, uint8> weights;
    uint32 loVote = 0;
    uint16 loVoteType = MS::Battlegrounds::BattlegroundTypeId::None;
    for (std::map<uint16, uint8>::const_iterator it = selectionWeights->begin(); it != selectionWeights->end(); ++it)
    {
        BattlemasterListEntry const* bl2 = sBattlemasterListStore.LookupEntry(it->first);
        if (!bl2)
            continue;

        if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground && bl2->RatedPlayers != 10)
            continue;

        if (bl2->MinPlayers > players && !sBattlegroundMgr->isTesting())
            continue;

        if (plrIgnoreWeights[bl2->MapID[0]] > 0)
        {
            if (!loVote || loVote > plrIgnoreWeights[bl2->MapID[0]])
            {
                loVoteType = it->first;
                loVote = plrIgnoreWeights[bl2->MapID[0]];
            }
            continue;
        }

        weight += it->second;
        weights[it->first] = it->second;
    }

    if (!weight)
        return loVoteType;

    uint32 selectedWeight = urand(0, weight - 1);
    weight = 0;
    for (std::map<uint16, uint8>::const_iterator it = weights.begin(); it != weights.end(); ++it)
    {
        weight += it->second;
        if (selectedWeight < weight)
            return it->first;
    }

    return MS::Battlegrounds::BattlegroundTypeId::None;
}

void BattlegroundQueue::UpdateEvents(uint32 diff)
{
    m_events.Update(diff);
    m_Functions.Update(diff);
}

void BattlegroundQueue::BattlegroundQueueUpdate(uint32 /*diff*/, uint16 bgTypeId, uint8 bracketID, uint8 joinType /*= 0*/, bool isRated /*= false*/, Roles role /*= ROLES_DEFAULT*/, uint8 bracket_MinLevel /*= 0*/)
{
    if (bracketID >= MS::Battlegrounds::MaxBrackets)
        return;

    if (_queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].empty() && _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].empty() &&
        _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalAlliance].empty() && _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::NormalHorde].empty())
        return;

    std::list<Battleground*>::iterator next;
    for (auto itr = sBattlegroundMgr->BGFreeSlotQueue.begin(); itr != sBattlegroundMgr->BGFreeSlotQueue.end(); itr = next)
    {
        next = itr;
        ++next;

        if ((*itr)->IsBattleground() && ((*itr)->GetTypeID(true) == bgTypeId || (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom && !(*itr)->IsBrawl()) && (*itr)->GetTypeID(true) != MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch)
        && (*itr)->GetMinLevel() == bracket_MinLevel && (*itr)->GetStatus() > STATUS_WAIT_QUEUE && (*itr)->GetStatus() < STATUS_WAIT_LEAVE)
        {
            auto bg = *itr;

            m_SelectionPools[TEAM_ALLIANCE].Init();
            m_SelectionPools[TEAM_HORDE].Init();

            FillPlayersToBG(bg, bracketID);

            for (std::list<GroupQueueInfo*>::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE].SelectedGroups.end(); ++citr)
                InviteGroupToBG(*citr, bg, (*citr)->Team);

            for (std::list<GroupQueueInfo*>::const_iterator citr = m_SelectionPools[TEAM_HORDE].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_HORDE].SelectedGroups.end(); ++citr)
                InviteGroupToBG(*citr, bg, (*citr)->Team);

            if (!bg->HasFreeSlots())
                bg->RemoveFromBGFreeSlotQueue();

            break;
        }
    }

    Battleground* bg_template = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg_template)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground: Update: bg template not found for %u", bgTypeId);
        return;
    }

    PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketById(bg_template->GetMapId(), bracketID);
    if (!bracketEntry)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground: Update: bg bracket entry not found for map %u bracket id %u", bg_template->GetMapId(), bracketID);
        return;
    }

    uint32 MinPlayersPerTeam = bg_template->GetMinPlayersPerTeam();
    uint32 MaxPlayersPerTeam = bg_template->GetMaxPlayersPerTeam();
    if (sBattlegroundMgr->isTesting())
        MinPlayersPerTeam = 1;
    else if ((!bg_template->IsBrawl() && bg_template->IsArena()) || bg_template->IsRBG())
    {
        MinPlayersPerTeam = joinType;
        MaxPlayersPerTeam = joinType;
    }

    for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        m_SelectionPools[i].Init();

    if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch)
    {
        if (CheckNormalMatchDeathMatch(bracketID, MinPlayersPerTeam, MaxPlayersPerTeam))
        {            
            Battleground* bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, 0, true, GenerateRandomMap(bgTypeId));
            if (!bg)
            {
                TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
                for (std::list<GroupQueueInfo*>::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.end(); ++citr)
                    InviteGroupToBG(*citr, bg, (*citr)->Team);

            bg->StartBattleground();
        }
    }
    else if (bg_template->IsBattleground() && !bg_template->IsRBG())
    {
        if (CheckPremadeMatch(bracketID, MinPlayersPerTeam, MaxPlayersPerTeam))
        {
            TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue::Update - create battleground: %u", bgTypeId);
            Battleground* bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, 0, isRated, GenerateRandomMap(bgTypeId));
            if (!bg)
            {
                TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
                for (std::list<GroupQueueInfo*>::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.end(); ++citr)
                    InviteGroupToBG(*citr, bg, (*citr)->Team);

            bg->StartBattleground();

            for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
                m_SelectionPools[i].Init();
        }
    }

    if (!isRated)
    {
        bool isLfgBrawl = bg_template->IsBrawl() && sBattlegroundMgr->GetBrawlData().Type == MS::Battlegrounds::BrawlTypes::Lfg;
        if ((isLfgBrawl && CheckSkirmishOrLFGBrawl(bracketID, false))
            || (!isLfgBrawl && !bg_template->IsArena() && CheckNormalMatch(bg_template, bracketID, MinPlayersPerTeam, MaxPlayersPerTeam))
            || (!isLfgBrawl && bg_template->IsArena() && CheckSkirmishOrLFGBrawl(bracketID)))
        {
            Battleground* bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, joinType, false, GenerateRandomMap(bgTypeId));
            if (!bg)
            {
                TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue::Update - Cannot create battleground: %u", bgTypeId);
                return;
            }

            for (uint32 i = TEAM_ALLIANCE; i < MAX_TEAMS; i++)
                for (std::list<GroupQueueInfo*>::const_iterator citr = m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.begin(); citr != m_SelectionPools[TEAM_ALLIANCE + i].SelectedGroups.end(); ++citr)
                    InviteGroupToBG(*citr, bg, (*citr)->Team);

            bg->StartBattleground();

            if (bg_template->IsArena()) // if 3v3, then need set it
                bg->SetBrawlJoinType(m_SelectionPools[TEAM_ALLIANCE].GetPlayerCount());
        }
    }
    else if (bg_template->IsArena() || bg_template->IsRBG())
    {
        GroupQueueInfo* aTeam = nullptr;
        GroupQueueInfo* hTeam = nullptr;

        if (joinType == MS::Battlegrounds::JoinType::ArenaSoloQ3v3 && !CalculateSoloQTeams(bracketID))
            return;

        if (!SelectRatedTeams(bracketID, aTeam, hTeam))
            return;

        if (!aTeam || !hTeam)
            return;

        Battleground* bg = sBattlegroundMgr->CreateNewBattleground(bgTypeId, bracketEntry, joinType, true, GenerateRandomMap(bgTypeId));
        if (!bg)
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundQueue::Update couldn't create bg instance for rated arena or bg match!");
            return;
        }

        aTeam->OpponentsMatchmakerRating = hTeam->MatchmakerRating;
        hTeam->OpponentsMatchmakerRating = aTeam->MatchmakerRating;

        if (aTeam->Team != ALLIANCE)
        {
            aTeam->Team = ALLIANCE;
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].push_front(aTeam);
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].remove(aTeam);
        }

        if (hTeam->Team != HORDE)
        {
            hTeam->Team = HORDE;
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeHorde].push_front(hTeam);
            _queuedGroups[bracketID][MS::Battlegrounds::QueueGroupTypes::PremadeAlliance].remove(hTeam);
        }

        bg->SetMatchmakerRating(ALLIANCE, aTeam->MatchmakerRating);
        bg->SetMatchmakerRating(HORDE, hTeam->MatchmakerRating);
        InviteGroupToBG(aTeam, bg, ALLIANCE);
        InviteGroupToBG(hTeam, bg, HORDE);

        TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "Starting rated bg match!");
        bg->StartBattleground();

        if (bg->IsArena() && bg->GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1)
        {
            uint8 curPlayerId = 0;

            for (auto itr = aTeam->Players.begin(); itr != aTeam->Players.end(); ++itr)
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    bg->AddMember(itr->first, aTeam->Team);

                    ArenaPlayerInfo pInfo;

                    pInfo.PlayerName = player->GetName();

                    if (WorldSession* session = player->GetSession())
                    {
                        pInfo.PlayerIP = session->GetRemoteAddress();
                        pInfo.HWId = session->_hwid;
                    }

                    pInfo.IPMark = 0;
                    pInfo.HWIdMark = !pInfo.HWId ? 4 : 0;
                    pInfo.PlayerID = curPlayerId;
                    pInfo.PlayersCheckedMask = (1 << curPlayerId);

                    bg->AddNameInNameList(aTeam->Team, &pInfo);
                    curPlayerId++;
                }
            }

            for (auto itr = hTeam->Players.begin(); itr != hTeam->Players.end(); ++itr)
            {
                if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                {
                    bg->AddMember(itr->first, hTeam->Team);

                    ArenaPlayerInfo pInfo;

                    pInfo.PlayerName = player->GetName();

                    if (WorldSession* session = player->GetSession())
                    {
                        pInfo.PlayerIP = session->GetRemoteAddress();
                        pInfo.HWId = session->_hwid;
                    }
                    
                    pInfo.IPMark = 0;
                    pInfo.HWIdMark = !pInfo.HWId ? 4 : 0;
                    pInfo.PlayerID = curPlayerId;
                    pInfo.PlayersCheckedMask = (1 << curPlayerId);

                    bg->AddNameInNameList(hTeam->Team, &pInfo);
                    curPlayerId++;
                }
            }

            std::string team1;
            std::string team2;
            bg->HandleArenaLogPlayerNames(team1, team2, "#@&%");

            switch (bg->GetJoinType())
            {
                case 1:
                {
                    // sLog->outArena("START:  Arena match type: 1v1 for (%s)(%s) vs (%s)(%s)", team1.c_str(), hwid1.c_str(), team2.c_str(), hwid2.c_str());
                    break;
                }
                case 2:
                {
                    sLog->outArena(MS::Battlegrounds::JoinType::Arena2v2, "START:  Arena match type: 2v2 for (%s) vs (%s)", team1.c_str(), team2.c_str());
                    break;
                }
                case 3:
                {
                    sLog->outArena(MS::Battlegrounds::JoinType::Arena3v3, "START:  Arena match type: 3v3 for (%s) vs (%s)", team1.c_str(), team2.c_str());
                    break;
                }
                default:
                    sLog->outArena(bg->GetJoinType(), "Arena match type: %u for Team1Id: %u - Team2Id: %u started for (%s) vs (%s)", bg->GetJoinType(), bg->GetGroupIdByIndex(TEAM_ALLIANCE), bg->GetGroupIdByIndex(TEAM_HORDE), team1.c_str(), team2.c_str());;
                    break;
            }
        }
    }
}

BGQueueInviteEvent::BGQueueInviteEvent(ObjectGuid pl_guid, uint32 BgInstanceGUID, uint16 BgTypeId, uint32 removeTime) : m_PlayerGuid(pl_guid), m_BgInstanceGUID(BgInstanceGUID), m_BgTypeId(BgTypeId), m_RemoveTime(removeTime)
{
}

bool BGQueueInviteEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* player = ObjectAccessor::FindPlayer(m_PlayerGuid);
    if (!player)
        return true;

    Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID, m_BgTypeId);
    if (!bg)
        return true;

    uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bg->GetTypeID(), bg->GetJoinType());
    uint32 queueSlot = player->GetBattlegroundQueueIndex(bgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)
    {
        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        if (bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
        {
            WorldPackets::Battleground::BattlefieldStatusNeedConfirmation battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusNeedConfirmation(&battlefieldStatus, bg, player, queueSlot, player->GetBattlegroundQueueJoinTime(bgQueueTypeId), (bg->IsArena() ? ARENA_INVITE_ACCEPT_WAIT_TIME : BG_INVITE_ACCEPT_WAIT_TIME) - INVITATION_REMIND_TIME, 0);
            player->SendDirectMessage(battlefieldStatus.Write());
        }
    }
    return true;
}

BGQueueRemoveEvent::BGQueueRemoveEvent(ObjectGuid pl_guid, uint32 bgInstanceGUID, uint16 BgTypeId, uint8 bgQueueTypeId, uint32 removeTime) : m_PlayerGuid(pl_guid), m_BgInstanceGUID(bgInstanceGUID), m_RemoveTime(removeTime), m_BgTypeId(BgTypeId), m_BgQueueTypeId(bgQueueTypeId)
{
}

bool BGQueueRemoveEvent::Execute(uint64 /*e_time*/, uint32 /*p_time*/)
{
    Player* player = ObjectAccessor::FindPlayer(m_PlayerGuid);
    if (!player)
        return true;

    Battleground* bg = sBattlegroundMgr->GetBattleground(m_BgInstanceGUID, m_BgTypeId);
    uint32 queueSlot = player->GetBattlegroundQueueIndex(m_BgQueueTypeId);
    if (queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES)
    {
        BattlegroundQueue &bgQueue = sBattlegroundMgr->GetBattlegroundQueue(m_BgQueueTypeId);
        if (bgQueue.IsPlayerInvited(m_PlayerGuid, m_BgInstanceGUID, m_RemoveTime))
        {
            auto joinTime = player->GetBattlegroundQueueJoinTime(m_BgQueueTypeId);
            player->RemoveBattlegroundQueueId(m_BgQueueTypeId);
            bgQueue.RemovePlayer(m_PlayerGuid, true);

            if (bg && bg->IsBattleground() && bg->GetStatus() != STATUS_WAIT_LEAVE)
                sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, 0, m_BgQueueTypeId, m_BgTypeId, bg->GetBracketId()));

            WorldPackets::Battleground::BattlefieldStatusNone battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusNone(&battlefieldStatus, player, queueSlot, joinTime);
            player->SendDirectMessage(battlefieldStatus.Write());

            if (bg && bg->GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1 && !bg->IsWargame())
                player->SendOperationsAfterDelay(OAD_ARENA_DESERTER);
        }
    }

    return true;
}