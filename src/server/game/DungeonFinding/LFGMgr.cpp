/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "Common.h"
#include "SharedDefines.h"
#include "DisableMgr.h"
#include "ObjectMgr.h"
#include "SocialMgr.h"
#include "LFGMgr.h"
#include <utility>
#include "LFGScripts.h"
#include "LFGGroupData.h"
#include "LFGPlayerData.h"
#include "InstanceSaveMgr.h"
#include "Chat.h"
#include "LFGQueue.h"
#include "Group.h"
#include "Player.h"
#include "GroupMgr.h"
#include "GameEventMgr.h"
#include "LFGPackets.h"
#include "AreaTriggerData.h"
#include "QuestData.h"
#include "BattlegroundMgr.h"

namespace lfg
{
LFGMgr::LFGMgr() : m_QueueTimer(0), m_ShortageCheckTimer(0), m_lfgProposalId(1), m_options(sWorld->getIntConfig(CONFIG_LFG_OPTIONSMASK))
{
    new LFGPlayerScript();
    new LFGGroupScript();
}

LFGMgr::~LFGMgr()
{
    for (auto& itr : RewardMapStore)
        delete itr.second;
}

LFGMgr* LFGMgr::instance()
{
    static LFGMgr instance;
    return &instance;
}

void LFGMgr::_LoadFromDB(Field* fields, ObjectGuid guid)
{
    if (!fields)
        return;

    if (!guid.IsParty())
        return;

    SetLeader(guid, ObjectGuid::Create<HighGuid::Player>(fields[0].GetUInt64()));

    uint32 dungeon = fields[16].GetUInt32();
    uint8 state = fields[17].GetUInt8();
    if (!dungeon || !state)
        return;

    SetDungeon(guid, dungeon);
    SetQueueId(guid, GetQueueId(dungeon));

    switch (state)
    {
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
            SetState(guid, static_cast<LfgState>(state), GetQueueId(dungeon));
            break;
        default:
            break;
    }
}

void LFGMgr::_SaveToDB(ObjectGuid guid, uint32 db_guid)
{
    if (!guid.IsParty())
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 queueId = GetQueueId(guid);
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_LFG_DATA);
    stmt->setUInt32(0, db_guid);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_LFG_DATA);
    stmt->setUInt32(0, db_guid);
    stmt->setUInt32(1, GetDungeon(guid));
    stmt->setUInt32(2, GetState(guid, queueId));
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void LFGMgr::LoadRewards()
{
    uint32 oldMSTime = getMSTime();

    for (auto& itr : RewardMapStore)
        delete itr.second;
    RewardMapStore.clear();

    // ORDER BY is very important for GetDungeonReward!
    QueryResult result = WorldDatabase.Query("SELECT dungeonId, maxLevel, firstQuestId, otherQuestId, bonusQuestId, EncounterMask FROM lfg_dungeon_rewards ORDER BY dungeonId, maxLevel ASC");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 lfg dungeon rewards. DB table `lfg_dungeon_rewards` is empty!");
        return;
    }

    uint32 count = 0;

    Field* fields = nullptr;
    do
    {
        fields = result->Fetch();
        uint32 dungeonId = fields[0].GetUInt32();
        if (!GetLFGDungeonEntry(dungeonId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Dungeon %u specified in table `lfg_dungeon_rewards` does not exist!", dungeonId);
            continue;
        }

        uint32 maxLevel = fields[1].GetUInt8();
        uint32 firstQuestId = fields[2].GetUInt32();
        uint32 otherQuestId = fields[3].GetUInt32();
        uint32 bonusQuestId = fields[4].GetUInt32();
        uint32 encounterMask = fields[5].GetUInt32();

        if (!maxLevel || maxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Level %u specified for dungeon %u in table `lfg_dungeon_rewards` can never be reached!", maxLevel, dungeonId);
            maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
        }

        if (!firstQuestId || !sQuestDataStore->GetQuestTemplate(firstQuestId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "First quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", firstQuestId, dungeonId);
            continue;
        }

        if (otherQuestId && !sQuestDataStore->GetQuestTemplate(otherQuestId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Other quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", otherQuestId, dungeonId);
            otherQuestId = 0;
        }

        if (bonusQuestId && !sQuestDataStore->GetQuestTemplate(bonusQuestId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Bonus quest %u specified for dungeon %u in table `lfg_dungeon_rewards` does not exist!", bonusQuestId, dungeonId);
            bonusQuestId = 0;
        }

        RewardMapStore.insert(std::make_pair(dungeonId, new LfgReward(maxLevel, firstQuestId, otherQuestId, bonusQuestId, encounterMask)));
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u lfg dungeon rewards in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

LFGDungeonData const* LFGMgr::GetLFGDungeon(uint32 id)
{
    if (id >= 16777216)
        id = id & 0xFFFFF;

    if (id >= sLfgDungeonsStore.GetNumRows())
        return nullptr;

    return LfgDungeonVStore[id];
}

LFGDungeonData const* LFGMgr::GetLFGDungeon(uint16 scenarioId, uint16 mapId)
{
    for (uint32 i = 0; i < sLfgDungeonsStore.GetNumRows(); i++)
    {
        LFGDungeonData* dungeon = LfgDungeonVStore[i];
        if (!dungeon)
            continue;

        if (LFGDungeonsEntry const* dungeonEntry = dungeon->dbc)
            if (dungeonEntry->ScenarioID == scenarioId && dungeonEntry->MapID == mapId)
                return dungeon;
    }

    return nullptr;
}

LFGDungeonData const* LFGMgr::GetLFGDungeon(uint32 id, uint32 team)
{
    if (LFGDungeonData* dungeon = LfgDungeonVStore[id])
        if (LFGDungeonsEntry const* dungeonEntry = dungeon->dbc)
            if (dungeonEntry->FitsTeam(team))
                return dungeon;

    return nullptr;
}

LFGDungeonData const* LFGMgr::GetLFGDungeon(uint32 mapId, Difficulty diff, uint32 team)
{
    static auto const skipDifficultyCheck = diff == DIFFICULTY_MYTHIC_DUNGEON || diff == DIFFICULTY_MYTHIC_KEYSTONE;

    for (uint32 i = 0; i < sLfgDungeonsStore.GetNumRows(); i++)
    {
        LFGDungeonData* dungeon = LfgDungeonVStore[i];
        if (!dungeon)
            continue;

        LFGDungeonsEntry const* dungeonEntry = dungeon->dbc;
        if (dungeonEntry->MapID == mapId && dungeonEntry->FitsTeam(team))
        {
            if (!skipDifficultyCheck && dungeonEntry->DifficultyID == diff)
                return dungeon;
            return dungeon;
        }

        ///< WTF?
    }

    return nullptr;
}

void LFGMgr::LoadLFGDungeons(bool reload /* = false */)
{
    uint32 oldMSTime = getMSTime();

    LfgDungeonStore.clear();
    LfgDungeonVStore.assign(sLfgDungeonsStore.GetNumRows() + 1, nullptr);

    for (LFGDungeonsEntry const* dungeon : sLfgDungeonsStore)
    {
        if (dungeon->TypeID == LFG_TYPE_ZONE)
            continue;

        LfgDungeonStore[dungeon->ID] = LFGDungeonData(dungeon);
        LfgDungeonVStore[dungeon->ID] = &LfgDungeonStore[dungeon->ID];
    }

    QueryResult result = WorldDatabase.Query("SELECT dungeonId, position_x, position_y, position_z, orientation FROM lfg_entrances");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 lfg entrance positions. DB table `lfg_entrances` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 dungeonId = fields[0].GetUInt32();
        LFGDungeonData* dungeon = LfgDungeonVStore[dungeonId];
        if (!dungeon)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "table `lfg_entrances` contains coordinates for wrong dungeon %u", dungeonId);
            continue;
        }

        dungeon->x = fields[1].GetFloat();
        dungeon->y = fields[2].GetFloat();
        dungeon->z = fields[3].GetFloat();
        dungeon->o = fields[4].GetFloat();

        sObjectMgr->AddInstanceGraveYard(dungeon->map, dungeon->x, dungeon->y, dungeon->z, dungeon->o);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u lfg entrance positions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

    if (reload)
        CachedDungeonMapStore.clear();

    for (auto& itr : LfgDungeonStore)
    {
        LFGDungeonData& dungeon = itr.second;
        if (dungeon.type != LFG_TYPE_RANDOM && dungeon.x == 0.0f && dungeon.y == 0.0f && dungeon.z == 0.0f)
        {
            if (AreaTriggerStruct const* at = sAreaTriggerDataStore->GetMapEntranceTrigger(dungeon.map))
            {
                dungeon.map = at->target_mapId;
                dungeon.x = at->target_X;
                dungeon.y = at->target_Y;
                dungeon.z = at->target_Z;
                dungeon.o = at->target_Orientation;
            }
            else
                TC_LOG_ERROR(LOG_FILTER_LFG, "LFGMgr::LoadLFGDungeons: Failed to load teleport positions for dungeon %s, cant find areatrigger for map %u. dungeonId %u", dungeon.name.c_str(), dungeon.map, dungeon.id);
        }

        if (dungeon.type != LFG_TYPE_RANDOM)
            CachedDungeonMapStore[dungeon.random_id].insert(dungeon.id);
        CachedDungeonMapStore[0].insert(dungeon.id);
    }
}

void LFGMgr::Update(uint32 diff)
{
    time_t currTime = time(nullptr);

    if (!isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    for (auto it = RoleChecksStore.begin(); it != RoleChecksStore.end();)
    {
        auto itRoleCheck = it++;
        LfgRoleCheck& roleCheck = itRoleCheck->second;
        if (currTime < roleCheck.cancelTime)
            continue;

        roleCheck.state = LFG_ROLECHECK_FAILED_TIMEOUT;

        for (LfgRolesMap::const_iterator itRoles = roleCheck.roles.begin(); itRoles != roleCheck.roles.end(); ++itRoles)
        {
            ObjectGuid guid = itRoles->first;
            RestoreState(guid, "Remove Obsolete RoleCheck", roleCheck.queueId);
            SendLfgRoleCheckUpdate(guid, roleCheck, 127);
            if (guid == roleCheck.leader)
                SendLfgJoinResult(guid, LfgJoinResultData(LFG_JOIN_FAILED, LFG_ROLECHECK_FAILED_TIMEOUT));
        }

        RestoreState(itRoleCheck->first, "Remove Obsolete RoleCheck", roleCheck.queueId);
        RoleChecksStore.erase(itRoleCheck);
    }

    for (auto it = ProposalsStore.begin(); it != ProposalsStore.end();)
    {
        auto itRemove = it++;
        if (itRemove->second.cancelTime < currTime)
            RemoveProposal(itRemove, LFG_UPDATETYPE_PROPOSAL_FAILED);
    }

    for (auto it = BootsStore.begin(); it != BootsStore.end();)
    {
        auto itBoot = it++;
        LfgPlayerBoot& boot = itBoot->second;
        if (boot.cancelTime < currTime)
        {
            boot.inProgress = false;
            for (LfgAnswerContainer::const_iterator itVotes = boot.votes.begin(); itVotes != boot.votes.end(); ++itVotes)
            {
                ObjectGuid pguid = itVotes->first;
                if (pguid != boot.victim)
                    SendLfgBootProposalUpdate(pguid, boot);
                SetState(pguid, LFG_STATE_DUNGEON, boot.queueId);
            }
            SetState(itBoot->first, LFG_STATE_DUNGEON, boot.queueId);
            BootsStore.erase(itBoot);
        }
    }

    uint32 lastProposalId = m_lfgProposalId;
    for (uint8 i = 0; i < ALL_TEAMS; i++)
        for (auto& itr : QueuesStore[i])
            if (uint8 newProposals = itr.second.FindGroups())
            {
                TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::Update: Found %u new groups in queue %u team %u", newProposals, itr.first, i);
            }

    if (lastProposalId != m_lfgProposalId)
    {
        // FIXME lastProposalId ? lastProposalId +1 ?
        for (LfgProposalContainer::const_iterator itProposal = ProposalsStore.find(m_lfgProposalId), next; itProposal != ProposalsStore.end(); itProposal = next)
        {
            next = itProposal;
            ++next;

            uint32 proposalId = itProposal->first;
            LfgProposal& proposal = ProposalsStore[proposalId];

            ObjectGuid guid = ObjectGuid::Empty;
            for (LfgProposalPlayerContainer::const_iterator itPlayers = proposal.players.begin(); itPlayers != proposal.players.end(); ++itPlayers)
            {
                guid = itPlayers->first;
                SetState(guid, LFG_STATE_PROPOSAL, proposal.queueId);
                ObjectGuid gguid = GetGroup(guid, proposal.queueId);
                if (!gguid.IsEmpty())
                {
                    SetState(gguid, LFG_STATE_PROPOSAL, proposal.queueId);
                    LfgDungeonSet const& dungeons = GetSelectedDungeons(guid, proposal.queueId);
                    //SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_ABORTED, dungeons));
                    SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, dungeons));
                }
                else
                    SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid, proposal.queueId)));
                SendLfgUpdateProposal(guid, proposal);
                StopAllOtherQueue(guid, proposal.queueId);
            }

            if (proposal.state == LFG_PROPOSAL_SUCCESS)
            {
                WorldPackets::LFG::ProposalResponse response;
                response.Accepted = true;
                response.ProposalID = proposalId;
                response.Ticket.RequesterGuid = guid;
                UpdateProposal(response, guid);
            }
        }
    }

    if (m_QueueTimer > LFG_QUEUEUPDATE_INTERVAL)
    {
        m_QueueTimer = 0;
        time_t currTime_ = time(nullptr);
        for (auto& i : QueuesStore)
            for (auto& itr : i)
                itr.second.UpdateQueueTimers(currTime_);
    }
    else
        m_QueueTimer += diff;

    // Update all queues shortage data
    if (m_ShortageCheckTimer > sWorld->getIntConfig(CONFIG_LFG_SHORTAGE_CHECK_INTERVAL) * IN_MILLISECONDS)
    {
        m_ShortageCheckTimer = 0;
        for (auto& i : QueuesStore)
            for (auto& itr : i)
                itr.second.UpdateShortageData();
    }
    else
        m_ShortageCheckTimer += diff;
}

void LFGMgr::JoinLfg(Player* player, uint8 roles, LfgDungeonSet& dungeons)
{
    if (!player || !player->GetSession() || dungeons.empty())
        return;

    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    Group* group = player->GetGroup();
    ObjectGuid playerGuid = player->GetGUID();
    ObjectGuid groupGuid = group ? group->GetGUID() : playerGuid;
    LfgJoinResultData joinData;
    GuidSet players;
    uint32 rDungeonId = 0;
    uint32 queueId = 0;
    if (group)
        queueId = GetQueueId(groupGuid);

    bool isContinueDungeonRequest = group && group->isLFGGroup() && GetState(groupGuid, queueId) != LFG_STATE_FINISHED_DUNGEON;

    if (HasQueue(playerGuid) && PlayerDungeons[playerGuid].size() >= sWorld->getIntConfig(CONFIG_LFG_MAX_QUEUES))
    {
        joinData.result = LFG_JOIN_FAILED_REASON_TOO_MANY_LFG;
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::Join: [%u] joining with %u members. result: %u", playerGuid.GetGUIDLow(), group ? group->GetMembersCount() : 1, joinData.result);
        SendLfgJoinResult(playerGuid, joinData);
        return;
    }

    if (isContinueDungeonRequest)
    {
        dungeons.clear();

        uint32 oldrDungeonId = 0;
        LfgDungeonSet const& selectedDungeons = GetSelectedDungeons(playerGuid, queueId);
        if (!selectedDungeons.empty())
        {
            LFGDungeonData const* rDungeonData = GetLFGDungeon(*selectedDungeons.begin() & 0xFFFFF, player->GetTeam());
            if (rDungeonData && rDungeonData->type == LFG_TYPE_RANDOM)
                oldrDungeonId = rDungeonData->id;
        }

        if (oldrDungeonId)
            dungeons.insert(oldrDungeonId);
        else
            dungeons.insert(GetDungeon(groupGuid));
    }

    if (!dungeons.empty())
        queueId = GetQueueId(*dungeons.begin() & 0xFFFFF);

    LfgState state = GetState(groupGuid, queueId);
    if (state == LFG_STATE_QUEUED)
    {
        LFGQueue& queue = GetQueue(groupGuid, queueId);
        if (!dungeons.empty())
        {
            LFGDungeonData const* entry = GetLFGDungeon(queueId, player->GetTeam());
            if (entry && queue.GetQueueType(groupGuid) != entry->internalType)
            {
                ChatHandler(player).PSendSysMessage("You cannot queue in different type queues at the same time.");
                joinData.result = LFG_JOIN_INTERNAL_ERROR;
            }
        }

        if (joinData.result == LFG_JOIN_OK) // ??
            queue.RemoveFromQueue(groupGuid);
    }

    // Check player or group member restrictions
    if (player->InBattleground() || player->InArena() || player->InBattlegroundQueue())
        joinData.result = LFG_JOIN_USING_BG_SYSTEM;
    else if (player->HasAura(LFG_SPELL_DUNGEON_DESERTER))
        joinData.result = LFG_JOIN_DESERTER;
    else if (player->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
        joinData.result = LFG_JOIN_RANDOM_COOLDOWN;
    else if (dungeons.empty())
        joinData.result = LFG_JOIN_NOT_MEET_REQS;
    else if (group)
    {
        uint8 memberCount = 0;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr && joinData.result == LFG_JOIN_OK; itr = itr->next())
        {
            if (Player* groupPlayer = itr->getSource())
            {
                if (groupPlayer->HasAura(LFG_SPELL_DUNGEON_DESERTER))
                    joinData.result = LFG_JOIN_PARTY_DESERTER;
                else if (groupPlayer->HasAura(LFG_SPELL_DUNGEON_COOLDOWN))
                    joinData.result = LFG_JOIN_PARTY_RANDOM_COOLDOWN;
                else if (groupPlayer->InBattleground() || groupPlayer->InArena() || groupPlayer->InBattlegroundQueue())
                    joinData.result = LFG_JOIN_USING_BG_SYSTEM;
                ++memberCount;
                players.insert(groupPlayer->GetGUID());
            }
        }
        if (memberCount != group->GetMembersCount() && joinData.result == LFG_JOIN_OK)
            joinData.result = LFG_JOIN_DISCONNECTED;
    }
    else if (joinData.result == LFG_JOIN_OK)
        players.insert(playerGuid);

    bool isRaid = false;
    if (joinData.result == LFG_JOIN_OK)
    {
        bool isDungeon = false;
        bool isScenario = false;
        for (auto it = dungeons.begin(); it != dungeons.end() && joinData.result == LFG_JOIN_OK; ++it)
        {
            LFGDungeonData const* entry = GetLFGDungeon(*it & 0xFFFFF, player->GetTeam());
            if (!entry)
            {
                joinData.result = LFG_JOIN_DUNGEON_INVALID;
                break;
            }

            switch (entry->dbc->Substruct)
            {
                case LFG_QUEUE_DUNGEON:
                    isDungeon = true;
                    break;
                case LFG_QUEUE_SCENARIO:
                    isScenario = true;
                    break;
                case LFG_QUEUE_LFR:
                case LFG_QUEUE_TIMEWALK_RAID:
                    isRaid = true;
                    break;
                default:
                    break;
            }

            if (isDungeon && isRaid || isDungeon && isScenario || isRaid && isScenario)
                joinData.result = LFG_JOIN_INTERNAL_ERROR;

            switch (entry->dbc->TypeID)
            {
                // FIXME: can join to random dungeon and random scenario at the same time
                case LFG_TYPE_RANDOM:
                    if (dungeons.size() > 1)
                        joinData.result = LFG_JOIN_INTERNAL_ERROR;
                    else
                        rDungeonId = *it;
                    break;
                case LFG_TYPE_DUNGEON:
                case LFG_TYPE_RAID:
                    break;
                default:
                    TC_LOG_ERROR(LOG_FILTER_LFG, "Wrong dungeon type %u for dungeon %u", entry->dbc->TypeID, *it);
                    joinData.result = LFG_JOIN_DUNGEON_INVALID;
                    break;
            }

            switch (entry->dbc->Substruct)
            {
                case LFG_QUEUE_SCENARIO:
                {
                    if (entry->dbc->DifficultyID == DIFFICULTY_HC_SCENARIO && !isContinueDungeonRequest)
                    {
                        if (sWorld->getBoolConfig(CONFIG_LFG_DEBUG_JOIN))
                            break;

                        // heroic scenarios can be queued only in full group
                        if (!group)
                            joinData.result = LFG_JOIN_PARTY_INFO_FAILED;
                        else if (group->GetMembersCount() < entry->dbc->GetMinGroupSize())
                            joinData.result = LFG_JOIN_TOO_FEW_MEMBERS;
                    }
                    break;
                }
                default:
                    break;
            }

            if (group && entry->dbc->GetMaxGroupSize() < group->GetMembersCount())
                joinData.result = LFG_JOIN_TOO_MUCH_MEMBERS;
        }

        if (rDungeonId)
            queueId = GetQueueId(rDungeonId & 0xFFFFF);

        if (joinData.result == LFG_JOIN_OK)
        {
            if (rDungeonId)
                dungeons = GetDungeonsByRandom(queueId);

            GetCompatibleDungeons(dungeons, players, joinData.lockmap);
            if (dungeons.empty())
                joinData.result = /*group ? LFG_JOIN_PARTY_NOT_MEET_REQS : */LFG_JOIN_NOT_MEET_REQS;
        }

        if (isScenario)
            roles = roles & (PLAYER_ROLE_LEADER | PLAYER_ROLE_DAMAGE);
    }

    if (joinData.result != LFG_JOIN_OK)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::Join: [%u] joining with %u members. result: %u", playerGuid.GetGUIDLow(), group ? group->GetMembersCount() : 1, joinData.result);
        if (!dungeons.empty())                             // Only should show lockmap when have no dungeons available
            joinData.lockmap.clear();
        SendLfgJoinResult(playerGuid, joinData);
        return;
    }

    SetTeam(playerGuid, player->GetTeamId(), queueId);
    PlayerDungeons[playerGuid].insert(queueId);

    WorldPackets::LFG::RideTicket ticket;
    ticket.RequesterGuid = playerGuid;
    ticket.Id = queueId;
    ticket.Type = WorldPackets::LFG::RideType::Lfg;
    ticket.Time = int32(time(nullptr));

    std::string debugNames;
    if (group)
    {
        GroupDungeons[groupGuid].insert(queueId); // Need for work check HasQueue

        // Create new rolecheck
        LfgRoleCheck& roleCheck = RoleChecksStore[groupGuid];
        roleCheck.roles.clear();
        roleCheck.cancelTime = time_t(time(nullptr)) + LFG_TIME_ROLECHECK;
        roleCheck.state = LFG_ROLECHECK_INITIALITING;
        roleCheck.leader = playerGuid;
        roleCheck.dungeons = dungeons;
        roleCheck.rDungeonId = rDungeonId;
        roleCheck.queueId = queueId;

        if (rDungeonId)
        {
            dungeons.clear();
            dungeons.insert(rDungeonId);
        }

        SetState(groupGuid, LFG_STATE_ROLECHECK, queueId);

        //LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons);
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (Player* plrg = itr->getSource())
            {
                ObjectGuid pguid = plrg->GetGUID();
                SetTicket(pguid, ticket, queueId);
                SetRoles(pguid, roles, queueId);
                SetTeam(pguid, plrg->GetTeamId(), queueId);
                //plrg->SendLfgUpdateParty(updateData);
                SendLfgJoinResult(pguid, joinData);
                SetState(pguid, LFG_STATE_ROLECHECK, queueId);
                if (!isContinueDungeonRequest)
                    SetSelectedDungeons(pguid, dungeons, queueId);
                roleCheck.roles[pguid] = 0;

                if (!debugNames.empty())
                    debugNames.append(", ");
                debugNames.append(plrg->GetName());
            }
        }

        UpdateRoleCheck(groupGuid, playerGuid, roles);
    }
    else
    {
        LfgRolesMap rolesMap;
        rolesMap[playerGuid] = roles;
        LFGQueue& queue = GetQueue(playerGuid, queueId);
        queue.AddQueueData(playerGuid, time(nullptr), dungeons, rolesMap);
        queue.queueId = queueId;

        if (!isContinueDungeonRequest)
        {
            if (rDungeonId)
            {
                dungeons.clear();
                dungeons.insert(rDungeonId);
            }
            SetSelectedDungeons(playerGuid, dungeons, queueId);
        }

        SetTicket(playerGuid, ticket, queueId);
        SendLfgJoinResult(playerGuid, joinData);
        SetState(groupGuid, LFG_STATE_QUEUED, queueId);
        SetRoles(playerGuid, roles, queueId);
        SendLfgUpdatePlayer(playerGuid, LfgUpdateData(LFG_UPDATETYPE_JOIN_QUEUE, dungeons));
        SendLfgUpdatePlayer(playerGuid, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons));

        if (!isContinueDungeonRequest)
        {
            if (rDungeonId != 0 || isRaid)
                SetEligibleForCTAReward(playerGuid, queue.IsEligibleForCTAReward(roles & ~PLAYER_ROLE_LEADER));
            else
                SetEligibleForCTAReward(playerGuid, 0);
        }

        debugNames.append(player->GetName());
    }

    if (sLog->ShouldLog(LOG_FILTER_LFG, LOG_LEVEL_DEBUG))
    {
        std::ostringstream o;
        o << "LFGMgr::Join: [" << playerGuid << "] joined (" << (group ? "group" : "player") << ") Members: " << debugNames.c_str() << ". Dungeons (" << uint32(dungeons.size()) << "): " << ConcatenateDungeons(dungeons);
        TC_LOG_DEBUG(LOG_FILTER_LFG, "%s", o.str().c_str());
    }
}

void LFGMgr::LeaveLfg(ObjectGuid guid, uint32 queueId)
{
    if (!queueId)
    {
        if (HasQueue(guid))
        {
            std::set<uint32> dungeonList = PlayerDungeons[guid];
            for (auto& dungeon : dungeonList)
                LeaveLfg(guid, dungeon);
        }
        return;
    }
    ObjectGuid gguid = GetGroup(guid, queueId);

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::LeaveLfg: (%s) queueId %u", guid.ToString().c_str(), queueId);

    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    LfgState state = GetState(guid, queueId);
    switch (state)
    {
        case LFG_STATE_QUEUED:
        case LFG_STATE_WAITE:
            if (!gguid.IsEmpty())
            {
                SetState(gguid, LFG_STATE_NONE, queueId);
                GuidSet const& players = GetPlayers(gguid);
                for (auto player : players)
                {
                    SetState(player, LFG_STATE_NONE, queueId);
                    SendLfgUpdateParty(player, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, GetSelectedDungeons(player, queueId)));
                }

                LFGQueue& queue = GetQueue(gguid, queueId);
                queue.RemoveFromQueue(gguid);
            }
            else
            {
                SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, GetSelectedDungeons(guid, queueId)));
                SetState(guid, LFG_STATE_NONE, queueId);
                LFGQueue& queue = GetQueue(guid, queueId);
                queue.RemoveFromQueue(guid);
            }
            RemoveFromQueue(guid, queueId);
            RemoveFromGroupQueue(gguid, queueId);
            break;
        case LFG_STATE_ROLECHECK:
            if (!gguid.IsEmpty())
                UpdateRoleCheck(gguid);                    // No player to update role = LFG_ROLECHECK_ABORTED
            break;
        case LFG_STATE_PROPOSAL:
        {
            auto it = ProposalsStore.begin();
            ObjectGuid pguid = gguid == guid ? GetLeader(gguid) : guid;
            while (it != ProposalsStore.end())
            {
                auto itPlayer = it->second.players.find(pguid);
                if (itPlayer != it->second.players.end())
                {
                    itPlayer->second.accept = LFG_ANSWER_DENY;
                    break;
                }
                ++it;
            }

            if (it != ProposalsStore.end())
                RemoveProposal(it, LFG_UPDATETYPE_PROPOSAL_DECLINED);
            break;
        }
        case LFG_STATE_NONE:
        case LFG_STATE_RAIDBROWSER:
            break;
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
        case LFG_STATE_BOOT:
        {
            if (guid != gguid) // Player
                SetState(guid, LFG_STATE_NONE, queueId);

            SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_PROPOSAL_BEGIN, GetSelectedDungeons(guid, queueId)));
            SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, GetSelectedDungeons(guid, queueId)));
            LFGQueue& queue = GetQueue(guid, queueId);
            queue.RemoveFromQueue(guid);
            RemoveFromQueue(guid, queueId);
            RemoveFromGroupQueue(gguid, queueId);
            if (auto player = ObjectAccessor::FindPlayer(guid))
                player->EnterInTimeWalk(nullptr);
            break;
        }
        default:
            break;
    }
}

void LFGMgr::UpdateRoleCheck(ObjectGuid gguid, ObjectGuid guid /* = 0 */, uint8 roles /* = PLAYER_ROLE_NONE */, uint8 partyIndex /*= 0*/)
{
    if (gguid.IsEmpty())
        return;

    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    auto itRoleCheck = RoleChecksStore.find(gguid);
    if (itRoleCheck == RoleChecksStore.end())
        return;

    LfgRoleCheck& roleCheck = itRoleCheck->second;
    bool sendRoleChosen = roleCheck.state != LFG_ROLECHECK_DEFAULT && !guid.IsEmpty();

    if (guid.IsEmpty())
        roleCheck.state = LFG_ROLECHECK_ABORTED;
    else if (roles < PLAYER_ROLE_TANK)
        roleCheck.state = LFG_ROLECHECK_NO_ROLE;
    else
    {
        roleCheck.roles[guid] = roles;

        LfgRolesMap::const_iterator itRoles = roleCheck.roles.begin();
        while (itRoles != roleCheck.roles.end() && itRoles->second != PLAYER_ROLE_NONE)
            ++itRoles;

        if (itRoles == roleCheck.roles.end())
        {
            LfgRolesMap check_roles = roleCheck.roles;
            uint32 n = 0;
            if (roleCheck.bgQueueId)
                roleCheck.state = LFG_ROLECHECK_FINISHED;
            else
                roleCheck.state = CheckGroupRoles(check_roles, LfgRoleData(*roleCheck.dungeons.begin() & 0xFFFFF), n) ? LFG_ROLECHECK_FINISHED : LFG_ROLECHECK_WRONG_ROLES;
        }
    }

    uint32 queueId = roleCheck.bgQueueId;
    LfgDungeonSet dungeons;
    if (roleCheck.rDungeonId)
        dungeons.insert(roleCheck.rDungeonId);
    else
        dungeons = roleCheck.dungeons;

    if (!roleCheck.bgQueueId)
    {

        queueId = GetQueueId(*dungeons.begin() & 0xFFFFF);

        if (roles)
            if (LFGDungeonData const* dungeonData = GetLFGDungeon(*dungeons.begin()))
                if (dungeonData->dbc->IsScenario() && !dungeonData->dbc->IsChallenge())
                    roles = roles & (PLAYER_ROLE_LEADER | PLAYER_ROLE_DAMAGE);
    }

    Battleground* bg = nullptr;
    GroupQueueInfo* ginfo = nullptr;
    uint32 avgTime = 0;
    PVPDifficultyEntry const* bracketEntry = nullptr;
    if (roleCheck.state == LFG_ROLECHECK_FINISHED && roleCheck.bgQueueId)
    {
        bg = sBattlegroundMgr->GetBattlegroundTemplate(roleCheck.bgQueueId);

        if (Group* group = sGroupMgr->GetGroupByGUID(gguid))
        {
            Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
            if (leader && bg)
            {
                BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(roleCheck.bgQueueTypeId);

                bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), leader->getLevel());
                if (bracketEntry)
                {
                    if (roleCheck.isSkirmish)
                        ginfo = bgQueue.AddGroup(leader, group, roleCheck.bgQueueId, bracketEntry, MS::Battlegrounds::JoinType::Arena2v2, false, false, roleCheck.ignormap);
                    else
                        ginfo = bgQueue.AddGroup(leader, group, roleCheck.bgQueueId, bracketEntry, 0, false, false, roleCheck.ignormap);
                    avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex);
                }
            }
        }
    }

    LfgJoinResultData joinData = LfgJoinResultData(LFG_JOIN_FAILED, roleCheck.state, queueId);
    for (LfgRolesMap::const_iterator it = roleCheck.roles.begin(); it != roleCheck.roles.end(); ++it)
    {
        ObjectGuid pguid = it->first;
        auto player = ObjectAccessor::FindPlayer(pguid);
        if (!player)
        {
            if (roleCheck.state == LFG_ROLECHECK_FINISHED)
                SetState(pguid, LFG_STATE_QUEUED, queueId);
            else if (roleCheck.state != LFG_ROLECHECK_INITIALITING)
                ClearState(pguid, queueId);
            continue;
        }

        if (sendRoleChosen)
            SendLfgRoleChosen(pguid, guid, roles);

        SendLfgRoleCheckUpdate(pguid, roleCheck, partyIndex);
        switch (roleCheck.state)
        {
            case LFG_ROLECHECK_INITIALITING:
                continue;
            case LFG_ROLECHECK_FINISHED:
                SetState(pguid, LFG_STATE_QUEUED, queueId);
                SetRoles(pguid, it->second, queueId);
                //SendLfgUpdateParty(pguid, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, dungeons));
                SendLfgUpdateParty(pguid, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, dungeons));
                if (roleCheck.bgQueueId)
                {
                    if (bracketEntry)
                        player->SetQueueRoleMask(bracketEntry->RangeIndex, roleCheck.roles[player->GetGUID()]);

                    if (ginfo && bg)
                    {
                        WorldPackets::Battleground::BattlefieldStatusQueued queued;
                        sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, player, player->AddBattlegroundQueueId(roleCheck.bgQueueTypeId), ginfo->JoinTime, avgTime, ginfo->JoinType, true);
                        player->SendDirectMessage(queued.Write());
                    }
                }
                break;
            case LFG_ROLECHECK_FAILED_TIMEOUT:
            case LFG_ROLECHECK_WRONG_ROLES:
                //SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_FAILED, dungeons));
                break;
            case LFG_ROLECHECK_ABORTED:
                //SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_ABORTED, dungeons));
                break;
            default:
                if (roleCheck.leader == pguid)
                    SendLfgJoinResult(pguid, joinData);
                //SendLfgUpdateParty(pguid, LfgUpdateData(LFG_UPDATETYPE_ROLECHECK_FAILED, dungeons));
                SendLfgUpdateParty(pguid, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, dungeons));
                RestoreState(pguid, "Rolecheck Failed", queueId);
                break;
        }
    }

    if (roleCheck.state == LFG_ROLECHECK_FINISHED)
    {
        SetState(gguid, LFG_STATE_QUEUED, queueId);
        if (roleCheck.bgQueueId)
        {
            if (bracketEntry)
            {
                if (roleCheck.isSkirmish)
                    sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, MS::Battlegrounds::JoinType::Arena2v2, roleCheck.bgQueueTypeId, roleCheck.bgQueueId, bracketEntry->RangeIndex, ROLES_DEFAULT, bracketEntry->MinLevel));
                else
                    sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, 0, roleCheck.bgQueueTypeId, roleCheck.bgQueueId, bracketEntry->RangeIndex, ROLES_DEFAULT, bracketEntry->MinLevel));
            }
        }
        else
        {
            LFGQueue& queue = GetQueue(gguid, queueId);
            queue.AddQueueData(gguid, time(nullptr), roleCheck.dungeons, roleCheck.roles);
            queue.queueId = queueId;
        }
        RoleChecksStore.erase(itRoleCheck);
    }
    else if (roleCheck.state != LFG_ROLECHECK_INITIALITING)
    {
        RestoreState(gguid, "Rolecheck Failed", queueId);
        RoleChecksStore.erase(itRoleCheck);
    }
}

void LFGMgr::GetCompatibleDungeons(LfgDungeonSet& dungeons, GuidSet const& players, LfgLockPartyMap& lockMap)
{
    lockMap.clear();
    for (auto it = players.begin(); it != players.end() && !dungeons.empty(); ++it)
    {
        ObjectGuid guid = *it;
        LfgLockMap const& cachedLockMap = GetLockedDungeons(guid);
        for (auto it2 = cachedLockMap.begin(); it2 != cachedLockMap.end() && !dungeons.empty(); ++it2)
        {
            uint32 dungeonId = it2->first & 0xFFFFF;
            auto itDungeon = dungeons.find(dungeonId);
            if (itDungeon != dungeons.end())
            {
                dungeons.erase(itDungeon);
                lockMap[guid][dungeonId] = it2->second;
            }
        }
    }
    if (!dungeons.empty())
        lockMap.clear();
}

bool LFGMgr::CheckGroupRoles(LfgRolesMap& groles, LfgRoleData const& roleData, uint32 &n, bool removeLeaderFlag /*= true*/)
{
    if (groles.empty())
        return false;

    uint8 damage = 0;
    uint8 tank = 0;
    uint8 healer = 0;

    if (removeLeaderFlag)
        for (auto& grole : groles)
            grole.second &= ~PLAYER_ROLE_LEADER;

    for (auto it = groles.begin(); it != groles.end(); ++it, ++n)
    {
        if (n > 1000)
            return false;

        if (it->second == PLAYER_ROLE_NONE || (it->second & ROLE_FULL_MASK) == 0)
            return false;

        if (it->second & PLAYER_ROLE_TANK)
        {
            if (it->second != PLAYER_ROLE_TANK)                 // if not one role taken - check enother
            {
                it->second -= PLAYER_ROLE_TANK;                 // exclude role for recurse check
                if (CheckGroupRoles(groles, roleData, n, false))   // check role with it
                    return true;                                // if plr not tank group can be completed
                it->second += PLAYER_ROLE_TANK;                 // return back excluded role.
            }
            else if (tank >= roleData.tanksNeeded)              // if set one role check needed count for tank
                return false;
            ++tank;                                         // set role.
        }

        if (it->second & PLAYER_ROLE_DAMAGE)
        {
            if (it->second != PLAYER_ROLE_DAMAGE)
            {
                it->second -= PLAYER_ROLE_DAMAGE;
                if (CheckGroupRoles(groles, roleData, n, false))
                    return true;
                it->second += PLAYER_ROLE_DAMAGE;
            }
            else if (damage >= roleData.dpsNeeded)
                return false;
            //else
            ++damage;
        }

        if (it->second & PLAYER_ROLE_HEALER)
        {
            if (it->second != PLAYER_ROLE_HEALER)
            {
                it->second -= PLAYER_ROLE_HEALER;
                if (CheckGroupRoles(groles, roleData, n, false))
                    return true;
                it->second += PLAYER_ROLE_HEALER;
            }
            else if (healer >= roleData.healerNeeded)
                return false;
            ++healer;
        }
    }
    return tank + healer + damage == uint8(groles.size());
}

void LFGMgr::MakeNewGroup(LfgProposal const& proposal)
{
    GuidList players, tankPlayers, healPlayers, dpsPlayers;
    GuidList playersToTeleport;

    for (auto it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        auto guid = it->first;
        if (guid == proposal.leader)
            players.push_back(guid);
        else
        {
            switch (it->second.role & ~PLAYER_ROLE_LEADER)
            {
            case PLAYER_ROLE_TANK:
                tankPlayers.push_back(guid);
                break;
            case PLAYER_ROLE_HEALER:
                healPlayers.push_back(guid);
                break;
            case PLAYER_ROLE_DAMAGE:
                dpsPlayers.push_back(guid);
                break;
            default:
                ASSERT(false, "Invalid LFG role %u", it->second.role);
                break;
            }
        }

        if (proposal.isNew || GetGroup(guid, proposal.queueId) != proposal.group)
            playersToTeleport.push_back(guid);
    }

    players.splice(players.end(), tankPlayers);
    players.splice(players.end(), healPlayers);
    players.splice(players.end(), dpsPlayers);

    auto dungeon = GetLFGDungeon(proposal.dungeonId);
    ASSERT(dungeon);

    auto grp = proposal.group ? sGroupMgr->GetGroupByGUID(proposal.group) : nullptr;
    for (auto pguid : players)
    {
        auto player = ObjectAccessor::FindPlayer(pguid);
        if (!player)
            continue;

        Group* group = player->GetGroup();
        if (group && group != grp)
            group->RemoveMember(player->GetGUID());

        if (!grp)
        {
            grp = new Group();
            grp->Create(player, 0, true);
            grp->ConvertToLFG(dungeon, false);
            auto gguid = grp->GetGUID();
            SetDungeon(gguid, dungeon->dbc->Entry());
            SetQueueId(gguid, proposal.queueId);
            SetState(gguid, LFG_STATE_DUNGEON, proposal.queueId);
            SetLfgGroup(pguid, gguid, proposal.queueId);

            if (dungeon->dbc->GetInternalType() == LFG_TYPE_RAID)
                grp->SetRaidDifficultyID(Difficulty(dungeon->difficulty));
            else
                grp->SetDungeonDifficultyID(Difficulty(dungeon->difficulty));

            sGroupMgr->AddGroup(grp);
            if (players.size() > 5)
                grp->ConvertToRaid();
        }
        else if (group != grp)
            grp->AddMember(player);

        int8 role = proposal.players.find(pguid)->second.role;
        grp->SetLfgRoles(pguid, role);

        if (dungeon->type == LFG_TYPE_RANDOM)
            player->AddDelayedEvent(10, [player]() -> void
        {
            if (player)
                player->CastSpell(player, LFG_SPELL_DUNGEON_COOLDOWN, false);
        });
    }

    ASSERT(grp);
    _SaveToDB(grp->GetGUID(), grp->GetDbStoreId());

    grp->SendUpdate();

    for (GuidList::const_iterator it = playersToTeleport.begin(); it != playersToTeleport.end(); ++it)
        if (auto player = ObjectAccessor::FindPlayer(*it))
            TeleportPlayer(player, false);
}

uint32 LFGMgr::AddProposal(LfgProposal& proposal)
{
    proposal.id = ++m_lfgProposalId;
    ProposalsStore[m_lfgProposalId] = proposal;
    return m_lfgProposalId;
}

void LFGMgr::UpdateProposal(WorldPackets::LFG::ProposalResponse response, ObjectGuid RequesterGuid)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    auto itProposal = ProposalsStore.find(response.ProposalID);
    if (itProposal == ProposalsStore.end())
        return;

    LfgProposal& proposal = itProposal->second;

    auto itProposalPlayer = proposal.players.find(RequesterGuid);
    if (itProposalPlayer == proposal.players.end())
        return;

    LfgProposalPlayer& player = itProposalPlayer->second;
    player.accept = LfgAnswer(response.Accepted);

    if (!response.Accepted)
    {
        RemoveProposal(itProposal, LFG_UPDATETYPE_PROPOSAL_DECLINED);
        return;
    }

    bool allAnswered = true;
    for (LfgProposalPlayerContainer::const_iterator itPlayers = proposal.players.begin(); itPlayers != proposal.players.end(); ++itPlayers)
        if (itPlayers->second.accept != LFG_ANSWER_AGREE)   // No answer (-1) or not accepted (0)
            allAnswered = false;

    if (!allAnswered && !onTest())
    {
        for (LfgProposalPlayerContainer::const_iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
            SendLfgUpdateProposal(it->first, proposal);
        return;
    }

    bool sendUpdate = proposal.state != LFG_PROPOSAL_SUCCESS;
    proposal.state = LFG_PROPOSAL_SUCCESS;

    LFGQueue& queue = GetQueue(response.Ticket.RequesterGuid, response.Ticket.Id);
    for (LfgProposalPlayerContainer::const_iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        ObjectGuid pguid = it->first;
        ObjectGuid gguid = it->second.group;

        uint32 dungeonId = proposal.dungeonId;
        int32 waitTime = -1;

        LfgDungeonSet const& selectedDungeons = GetSelectedDungeons(pguid, proposal.queueId);
        if (!selectedDungeons.empty())
            dungeonId = *selectedDungeons.begin();

        LfgUpdateData updateData = LfgUpdateData(LFG_UPDATETYPE_GROUP_FOUND, selectedDungeons);

        if (sendUpdate)
            SendLfgUpdateProposal(pguid, proposal);

        if (!gguid.IsEmpty())
        {
            waitTime = int32((response.Ticket.Time - queue.GetJoinTime(gguid)) / IN_MILLISECONDS);
            SendLfgUpdateParty(pguid, updateData);
        }
        else
        {
            waitTime = int32((response.Ticket.Time - queue.GetJoinTime(pguid)) / IN_MILLISECONDS);
            SendLfgUpdatePlayer(pguid, updateData);
        }
        updateData.updateType = LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
        SendLfgUpdatePlayer(pguid, updateData);
        SendLfgUpdateParty(pguid, updateData);

        uint8 role = GetRoles(pguid, proposal.queueId);
        role &= ~PLAYER_ROLE_LEADER;
        switch (role)
        {
            case PLAYER_ROLE_DAMAGE:
                queue.UpdateWaitTimeDps(waitTime, dungeonId);
                break;
            case PLAYER_ROLE_HEALER:
                queue.UpdateWaitTimeHealer(waitTime, dungeonId);
                break;
            case PLAYER_ROLE_TANK:
                queue.UpdateWaitTimeTank(waitTime, dungeonId);
                break;
            default:
                queue.UpdateWaitTimeAvg(waitTime, dungeonId);
                break;
        }

        SetState(pguid, LFG_STATE_DUNGEON, proposal.queueId);
    }

    for (GuidList::const_iterator it = proposal.queues.begin(); it != proposal.queues.end(); ++it)
        queue.RemoveFromQueue(*it);

    MakeNewGroup(proposal);
    ProposalsStore.erase(itProposal);
}

void LFGMgr::RemoveProposal(LfgProposalContainer::iterator itProposal, LfgUpdateType type)
{
    LfgProposal& proposal = itProposal->second;
    proposal.state = LFG_PROPOSAL_FAILED;

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: Proposal %u, state FAILED, UpdateType %u", itProposal->first, type);

    if (type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        for (auto& player : proposal.players)
            if (player.second.accept == LFG_ANSWER_PENDING)
                player.second.accept = LFG_ANSWER_DENY;

    GuidSet toRemove;
    for (auto& player : proposal.players)
    {
        StartAllOtherQueue(player.first, proposal.queueId);

        if (player.second.accept == LFG_ANSWER_AGREE)
            continue;

        ObjectGuid guid = player.second.group ? player.second.group : player.first;
        if (player.second.accept == LFG_ANSWER_DENY || type == LFG_UPDATETYPE_PROPOSAL_FAILED)
        {
            player.second.accept = LFG_ANSWER_DENY;
            toRemove.insert(guid);
        }
    }

    uint32 dungeonId = 0;
    for (LfgProposalPlayerContainer::const_iterator it = proposal.players.begin(); it != proposal.players.end(); ++it)
    {
        ObjectGuid guid = it->first;
        ObjectGuid gguid = it->second.group ? it->second.group : guid;

        SendLfgUpdateProposal(guid, proposal);

        if (toRemove.find(gguid) != toRemove.end())
        {
            LfgUpdateData updateData;
            updateData.dungeons = GetSelectedDungeons(guid, proposal.queueId);

            if (it->second.accept == LFG_ANSWER_DENY)
            {
                updateData.updateType = type;
                TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: %s didn't accept. Removing from queue and compatible cache", guid.ToString().c_str());
            }
            else
            {
                updateData.updateType = LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
                TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: %s in same group that someone that didn't accept. Removing from queue and compatible cache", guid.ToString().c_str());
            }

            RestoreState(guid, "Proposal Fail (didn't accepted or in group with someone that didn't accept", proposal.queueId);
            if (gguid != guid)
            {
                RestoreState(it->second.group, "Proposal Fail (someone in group didn't accepted)", proposal.queueId);
                SendLfgUpdateParty(guid, updateData);
            }
            else
                SendLfgUpdatePlayer(guid, updateData);
        }
        else
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemoveProposal: Readding %s to queue.", guid.ToString().c_str());
            SetState(guid, LFG_STATE_QUEUED, proposal.queueId);
            if (gguid != guid)
            {
                SetState(gguid, LFG_STATE_QUEUED, proposal.queueId);
                SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid, proposal.queueId)));
            }
            else
                SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid, proposal.queueId)));
        }
    }

    LFGQueue& queue = GetQueue(proposal.players.begin()->first, proposal.queueId);
    for (auto guid : toRemove)
    {
        queue.RemoveFromQueue(guid);
        proposal.queues.remove(guid);
        RemoveFromQueue(guid, proposal.queueId);
    }
    RemoveFromGroupQueue(proposal.group, proposal.queueId);

    for (GuidList::const_iterator it = proposal.queues.begin(); it != proposal.queues.end(); ++it)
        queue.AddToQueue(*it);

    ProposalsStore.erase(itProposal);
}

void LFGMgr::InitBoot(ObjectGuid gguid, ObjectGuid kicker, ObjectGuid victim, std::string const& reason)
{
    uint32 queueId = GetQueueId(gguid);
    SetState(gguid, LFG_STATE_BOOT, queueId);

    LfgPlayerBoot& boot = BootsStore[gguid];
    boot.inProgress = true;
    boot.cancelTime = time_t(time(nullptr)) + LFG_TIME_BOOT;
    boot.reason = reason;
    boot.victim = victim;
    boot.votesNeeded = GetVotesNeededForKick(gguid);
    boot.queueId = queueId;

    GuidSet const& players = GetPlayers(gguid);

    for (auto guid : players)
    {
        SetState(guid, LFG_STATE_BOOT, boot.queueId);
        boot.votes[guid] = LFG_ANSWER_PENDING;
    }

    boot.votes[victim] = LFG_ANSWER_DENY;                  // Victim auto vote NO
    boot.votes[kicker] = LFG_ANSWER_AGREE;                 // Kicker auto vote YES

    for (auto player : players)
        SendLfgBootProposalUpdate(player, boot);
}

void LFGMgr::UpdateBoot(ObjectGuid gguid, ObjectGuid guid, bool accept)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);

    if (gguid.IsEmpty())
        return;

    auto itBoot = BootsStore.find(gguid);
    if (itBoot == BootsStore.end())
        return;

    LfgPlayerBoot& boot = itBoot->second;

    if (boot.votes[guid] != LFG_ANSWER_PENDING)
        return;

    boot.votes[guid] = LfgAnswer(accept);

    uint8 votesNum = 0;
    uint8 agreeNum = 0;
    for (LfgAnswerContainer::const_iterator itVotes = boot.votes.begin(); itVotes != boot.votes.end(); ++itVotes)
    {
        if (itVotes->second != LFG_ANSWER_PENDING)
        {
            ++votesNum;
            if (itVotes->second == LFG_ANSWER_AGREE)
                ++agreeNum;
        }
    }

    if (agreeNum < boot.votesNeeded && (votesNum - agreeNum) < boot.votesNeeded)
        return;

    boot.inProgress = false;
    for (LfgAnswerContainer::const_iterator itVotes = boot.votes.begin(); itVotes != boot.votes.end(); ++itVotes)
    {
        ObjectGuid pguid = itVotes->first;
        if (pguid != boot.victim)
        {
            SetState(pguid, LFG_STATE_DUNGEON, boot.queueId);
            SendLfgBootProposalUpdate(pguid, boot);
        }
    }

    SetState(gguid, LFG_STATE_DUNGEON, boot.queueId);
    if (agreeNum >= boot.votesNeeded)
    {
        if (Group* group = sGroupMgr->GetGroupByGUID(gguid))
            Player::RemoveFromGroup(group, boot.victim, GROUP_REMOVEMETHOD_KICK_LFG);
        DecreaseKicksLeft(gguid);
    }
    BootsStore.erase(itBoot);
}

void LFGMgr::TeleportPlayer(Player* player, bool out, bool fromOpcode /*= false*/)
{
    LFGDungeonData const* dungeon = nullptr;
    Group* group = player->GetGroup();

    if (group && group->isLFGGroup())
        dungeon = GetLFGDungeon(GetDungeon(group->GetGUID()), player->GetTeam());

    if (!dungeon)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "TeleportPlayer: Player %s not in group/lfggroup or dungeon not found!", player->GetName());
        player->SendLfgTeleportError(uint8(LFG_TELEPORTERROR_INVALID_LOCATION));
        return;
    }

    if (out)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "TeleportPlayer: Player %s is being teleported out. Current Map %u - Expected Map %u", player->GetName(), player->GetMapId(), uint32(dungeon->map));
        if (player->GetMapId() == uint32(dungeon->map))
        {
            player->AddDelayedEvent(100, [player]() -> void
            {
                if (!player || !player->IsInWorld())
                    return;

                if (player->IsBeingTeleported())
                    return;

                if (!player->isAlive())
                {
                    player->ResurrectPlayer(1.0f);
                    player->SpawnCorpseBones();
                }

                player->ForceChangeTalentGroup(player->GetLastActiveSpec(true));

                player->ScheduleDelayedOperation(DELAYED_BG_MOUNT_RESTORE);
                player->ScheduleDelayedOperation(DELAYED_BG_TAXI_RESTORE);
                player->ScheduleDelayedOperation(DELAYED_BG_GROUP_RESTORE);

                player->SafeTeleport(player->GetBattlegroundEntryPoint());
            });
        }

        return;
    }

    LfgTeleportError error = LFG_TELEPORTERROR_OK;

    if (!player->isAlive())
        error = LFG_TELEPORTERROR_PLAYER_DEAD;
    else if (player->IsMirrorTimerActive(FATIGUE_TIMER))
        error = LFG_TELEPORTERROR_FATIGUE;
    else if (player->GetMapId() != uint32(dungeon->map))
    {
        uint32 mapid = dungeon->map;
        float x = dungeon->x;
        float y = dungeon->y;
        float z = dungeon->z;
        float orientation = dungeon->o;

        if (!fromOpcode)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr && !mapid; itr = itr->next())
            {
                Player* plrg = itr->getSource();
                if (plrg && plrg != player && plrg->GetMapId() == uint32(dungeon->map))
                {
                    mapid = plrg->GetMapId();
                    x = plrg->GetPositionX();
                    y = plrg->GetPositionY();
                    z = plrg->GetPositionZ();
                    orientation = plrg->GetOrientation();
                    break;
                }
            }
        }

        player->AddDelayedEvent(100, [player, mapid, x, y, z, orientation]() -> void
        {
            if (!player || !player->IsInWorld())
                return;

            if (player->IsBeingTeleported())
                return;

            if (!player->GetMap()->IsDungeon())
                player->SetBattlegroundEntryPoint();

            if (player->isInFlight())
            {
                player->GetMotionMaster()->MovementExpired();
                player->CleanupAfterTaxiFlight();
            }

            bool ok = player->SafeTeleport(mapid, x, y, z, orientation);

            player->SendLfgTeleportError(ok ? LFG_TELEPORTERROR_OK : LFG_TELEPORTERROR_INVALID_LOCATION);

            if (ok)
               player->CastSpell(player, 228128); // Dungeon Preparation - Able to adjust talents.
        });

        return;
    }
    else
        error = LFG_TELEPORTERROR_INVALID_LOCATION;

    //if (error != LFG_TELEPORTERROR_OK)
    player->SendLfgTeleportError(uint8(error));

    TC_LOG_DEBUG(LOG_FILTER_LFG, "TeleportPlayer: Player %s is being teleported in to map %u (x: %f, y: %f, z: %f) Result: %u", player->GetName(), dungeon->map, dungeon->x, dungeon->y, dungeon->z, error);
}

void LFGMgr::SendUpdateStatus(ObjectGuid guid, LfgUpdateData const& updateData, bool Suspended)
{
    if (!HasQueue(guid))
        return;

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
        return;

    ObjectGuid gguid = player->GetGroup() ? player->GetGroup()->GetGUID() : player->GetGUID();

    bool queued = false;
    bool join = false;
    bool NotifyUI = false;
    bool canLeave = player->GetGroup() ? player->GetGroup()->GetLeaderGUID() == player->GetGUID() : true;

    switch (updateData.updateType)
    {
        case LFG_UPDATETYPE_ADDED_TO_QUEUE:
            join = true;
            queued = true;
            break;
        case LFG_UPDATETYPE_ROLECHECK_ABORTED:
            NotifyUI = true;
            join = true;
            queued = true;
            break;
        case LFG_UPDATETYPE_PROPOSAL_DECLINED:
            NotifyUI = true;
            break;
        case LFG_UPDATETYPE_JOIN_QUEUE:
        case LFG_UPDATETYPE_PROPOSAL_BEGIN:
            join = true;
            break;
        case LFG_UPDATETYPE_LEADER_UNK1:
        case LFG_UPDATETYPE_DUNGEON_FINISHED:
            NotifyUI = true;
            break;
        case LFG_UPDATETYPE_UPDATE_STATUS:
            join = updateData.state == LFG_STATE_ROLECHECK || updateData.state == LFG_STATE_NONE;
            queued = updateData.state == LFG_STATE_QUEUED;
            NotifyUI = updateData.state != LFG_STATE_QUEUED;
            break;
        case LFG_UPDATETYPE_GROUP_FOUND:
        case LFG_UPDATETYPE_GROUP_DISBAND_UNK16:
            break;
        default:
            break;
    }

    uint32 queueId = 0;
    LfgQueueData const* queueData = nullptr;
    if (!updateData.dungeons.empty())
        queueId = GetQueueId(*updateData.dungeons.begin()&0xFFFFF);

    WorldPackets::LFG::QueueStatusUpdate update;
    if (auto ticket = GetTicket(player->GetGUID(), queueId))
        update.Ticket = *ticket;

    update.SubType = queueData ? queueData->subType : LFG_QUEUE_DUNGEON;
    update.Reason = updateData.updateType;
    update.RequestedRoles = GetRoles(guid, queueId);
    if (Suspended)
        update.SuspendedPlayers.push_back(player->GetGUID());
    update.IsParty = canLeave; // This is not party
    update.NotifyUI = NotifyUI;
    update.Joined = join;
    update.LfgJoined = updateData.updateType != LFG_UPDATETYPE_REMOVED_FROM_QUEUE;
    update.Queued = queued;

    std::transform(updateData.dungeons.begin(), updateData.dungeons.end(), std::back_inserter(update.Slots), [=](uint32 dungeonId)
    {
        return GetLFGDungeonEntry(dungeonId);
    });

    player->SendDirectMessage(update.Write());
}

void LFGMgr::FinishDungeon(ObjectGuid gguid, const uint32 dungeonId)
{
    uint32 gDungeonId = GetDungeon(gguid);
    uint32 queueId = GetQueueId(gguid, dungeonId&0x00FFFFFF);
    if (gDungeonId != dungeonId)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] Finished dungeon %u but group queued for %u. Ignoring", gguid.GetCounter(), dungeonId, gDungeonId);
        return;
    }

    if (GetState(gguid, queueId) == LFG_STATE_FINISHED_DUNGEON) // Shouldn't happen. Do not reward multiple times
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] Already rewarded group. Ignoring", gguid.GetCounter());
        return;
    }

    SetState(gguid, LFG_STATE_FINISHED_DUNGEON, queueId);

    const GuidSet& players = GetPlayers(gguid);
    for (auto guid : players)
    {
        if (GetState(guid, queueId) == LFG_STATE_FINISHED_DUNGEON)
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: %s Already rewarded player. Ignoring", guid.ToString().c_str());
            continue;
        }

        uint32 rDungeonId = 0;
        const LfgDungeonSet& dungeons = GetSelectedDungeons(guid, queueId);
        if (!dungeons.empty())
            rDungeonId = *dungeons.begin();

        SetState(guid, LFG_STATE_FINISHED_DUNGEON, queueId);
        SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_DUNGEON_FINISHED, GetSelectedDungeons(guid, queueId)));
        StartAllOtherQueue(guid, queueId);

        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player || !player->IsInWorld())
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] not found in world", guid.GetCounter());
            continue;
        }

        LFGDungeonData const* rDungeon = GetLFGDungeon(rDungeonId, player->GetTeam());
        if (!rDungeon)
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: %s dungeon %u does not exist", guid.ToString().c_str(), rDungeonId);
            continue;
        }
        // if 'random' dungeon is not random nor seasonal, check actual dungeon (it can be raid finder)
        // if (rDungeon->type != LFG_TYPE_RANDOM && !rDungeon->seasonal)
        // {
            // TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] dungeon %u type %i is not random nor seasonal %i and can't be rewarded, rDungeon->id %i", guid, rDungeonId, rDungeon->type, rDungeon->seasonal, rDungeon->id);
            // continue;
        // }

        LFGDungeonData const* dungeonDone = GetLFGDungeon(dungeonId, player->GetTeam());
        if (!dungeonDone)
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: dungeonDone %i not found", dungeonId);
            continue;
        }
        /*if(!dungeonDone->dbc->CanBeRewarded())
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: dungeonDone %i not CanBeRewarded %i", dungeonId, dungeonDone->dbc->CanBeRewarded());
            continue;
        }*/

        // there can be more that 1 non-random dungeon selected, so fall back to current dungeon id
        //rDungeonId = dungeonDone->random_id;
        rDungeon = dungeonDone;

        uint32 mapId = dungeonDone ? uint32(dungeonDone->map) : 0;

        if (player->GetMapId() != mapId)
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: %s is in map %u and should be in %u to get reward", guid.ToString().c_str(), player->GetMapId(), mapId);
            continue;
        }

        // Update achievements
        if (rDungeon->difficulty == DIFFICULTY_HEROIC)
            player->UpdateAchievementCriteria(CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS, 1);

        player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_INSTANCE, dungeonDone->dbc->GetMaxGroupSize());

        LfgReward const* reward = GetDungeonReward(rDungeonId, player->getLevel());
        if (!reward)
        {
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] Don`t find reward for DungeonId %i player %u level", guid.GetCounter(), rDungeonId, player->getLevel());
            continue;
        }

        bool done = reward->RewardPlayer(player, rDungeon, rDungeonId);
        player->AddLfgCooldown(rDungeonId);

        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::FinishDungeon: [" UI64FMTD "] done dungeon %u, %s previously done.", player->GetGUID().GetCounter(), GetDungeon(gguid), done ? " " : " not");
        LfgPlayerRewardData data = LfgPlayerRewardData(rDungeon->dbc->Entry(), dungeonDone->dbc->Entry(), done, reward);

        if (data.rdungeonEntry && data.sdungeonEntry)
        {
            Quest const* quest = nullptr;
            if (data.done)
                quest = sQuestDataStore->GetQuestTemplate(data.reward->otherQuest);
            else
                quest = sQuestDataStore->GetQuestTemplate(data.reward->firstQuest);

            WorldPackets::LFG::PlayerReward playerData;
            playerData.ActualSlot = data.rdungeonEntry;
            playerData.QueuedSlot = data.sdungeonEntry;

            if (quest)
            {
                playerData.RewardMoney = player->GetQuestMoneyReward(quest);
                playerData.AddedXP = quest->XPValue(player);

                for (auto const& i : {0, 1, 2, 3})
                {
                    WorldPackets::LFG::PlayerReward::PlayerRewards rewards;

                    if (uint32 itemId = quest->RewardItemId[i])
                    {
                        rewards.IsCurrency = false;
                        rewards.RewardItem = itemId;
                        rewards.RewardItemQuantity = quest->RewardItemCount[i];
                    }

                    if (uint32 currency = quest->RewardCurrencyId[i])
                    {
                        rewards.IsCurrency = true;
                        rewards.RewardItemQuantity = quest->RewardCurrencyCount[i] * sDB2Manager.GetCurrencyPrecision(currency);
                        rewards.BonusCurrency = currency;
                    }

                    playerData.Players.push_back(rewards);
                }
            }
            if (data.reward->bonusQuestId)
            {
                if (CTARewardStore[guid] && player->GetGroup() && (player->GetGroup()->GetLfgRoles(guid) & ~PLAYER_ROLE_LEADER) == CTARewardStore[guid])
                    if (quest = sQuestDataStore->GetQuestTemplate(data.reward->bonusQuestId))
                        player->RewardQuest(quest, 0, nullptr, false);

                SetEligibleForCTAReward(guid, 0);
            }

            player->AddUpdatePacket(playerData.Write());
        }
    }

    if (Group* group = sGroupMgr->GetGroupByGUID(gguid))
        group->SendUpdate();
}

LfgDungeonSet const& LFGMgr::GetDungeonsByRandom(uint32 randomdungeon)
{
    return CachedDungeonMapStore[randomdungeon];
}

LfgReward const* LFGMgr::GetDungeonReward(uint32 dungeon, uint8 level)
{
    LfgReward const* rew = nullptr;
    for (auto const& pair : Trinity::Containers::MapEqualRange(RewardMapStore, dungeon & 0x00FFFFFF))
    {
        rew = pair.second;
        if (pair.second->maxLevel >= level)
            break;
    }

    return rew;
}

LfgType LFGMgr::GetDungeonType(uint32 dungeonId)
{
    LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId);
    if (!dungeon)
        return LFG_TYPE_NONE;

    return LfgType(dungeon->type);
}

LfgState LFGMgr::GetState(ObjectGuid guid, uint32 queueId)
{
    LfgState state;
    if (guid.IsParty())
        state = GroupsStore[guid].GetState();
    else
        state = PlayersStore[guid][queueId].GetState();

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetState: %s = %u", guid.ToString().c_str(), state);
    return state;
}

LfgState LFGMgr::GetOldState(ObjectGuid guid, uint32 queueId)
{
    LfgState state;
    if (guid.IsParty())
        state = GroupsStore[guid].GetOldState();
    else
        state = PlayersStore[guid][queueId].GetOldState();

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetOldState: %s = %u", guid.ToString().c_str(), state);
    return state;
}

uint32 LFGMgr::GetCompletedMask(ObjectGuid guid)
{
    return CompletedMaskStore[guid];
}

void LFGMgr::SetCompletedMask(ObjectGuid guid, uint32 mask)
{
    CompletedMaskStore[guid] |= mask;
}

uint32 LFGMgr::GetDungeon(ObjectGuid guid, bool asId /*= true */)
{
    uint32 dungeon = GroupsStore[guid].GetDungeon(asId);
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetDungeon: %s asId: %u = %u", guid.ToString().c_str(), asId, dungeon);
    return dungeon;
}

uint32 LFGMgr::GetDungeonMapId(ObjectGuid guid)
{
    uint32 dungeonId = GroupsStore[guid].GetDungeon(true);
    uint32 mapId = 0;
    if (dungeonId)
        if (LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId))
            mapId = dungeon->map;

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetDungeonMapId: %s = %u (DungeonId = %u)", guid.ToString().c_str(), mapId, dungeonId);
    return mapId;
}

uint8 LFGMgr::GetRoles(ObjectGuid guid, uint32 queueId)
{
    uint8 roles = PlayersStore[guid][queueId].GetRoles();
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetRoles: %s = %u", guid.ToString().c_str(), roles);
    return roles;
}

LfgDungeonSet const& LFGMgr::GetSelectedDungeons(ObjectGuid guid, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetSelectedDungeons: %s", guid.ToString().c_str());
    return PlayersStore[guid][queueId].GetSelectedDungeons();
}

LfgLockMap LFGMgr::GetLockedDungeons(ObjectGuid guid)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetLockedDungeons: %s", guid.ToString().c_str());
    LfgLockMap lock;
    Player* player = ObjectAccessor::FindPlayer(guid);
    if (!player)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "Player: %s not ingame while retrieving his LockedDungeons.", guid.ToString().c_str());
        return lock;
    }

    uint8 level = player->getLevel();
    uint8 expansion = player->GetSession()->Expansion();
    LfgDungeonSet const& dungeons = GetDungeonsByRandom(0);

    for (auto itr : dungeons)
    {
        LFGDungeonData const* dungeon = GetLFGDungeon(itr);
        if (!dungeon)
            continue;

        InstancePlayerBind const* pBind = player->GetBoundInstance(dungeon->map, Difficulty(dungeon->difficulty));
        InstanceSave* pSave = pBind ? pBind->save : nullptr;
        uint32 avgItemLevel = player->GetAverageItemLevelTotal(false);

        LockData lockData;
        if (dungeon->expansion > expansion)
            lockData.status = LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION;
        else if (dungeon->difficulty > DIFFICULTY_NORMAL && pBind && pBind->perm && pSave && pSave->GetResetTime() > time(nullptr) && !dungeon->dbc->IsScenario() && !dungeon->dbc->IsRaidFinder())
            lockData.status = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (DisableMgr::IsDisabledFor(DISABLE_TYPE_LFG, dungeon->id, player))
            lockData.status = LFG_LOCKSTATUS_RAID_LOCKED;
        else if (dungeon->minlevel > level)
            lockData.status = LFG_LOCKSTATUS_TOO_LOW_LEVEL;
        else if (dungeon->maxlevel != 0 && dungeon->maxlevel < level)
            lockData.status = LFG_LOCKSTATUS_TOO_HIGH_LEVEL;
        else if (dungeon->seasonal && !IsSeasonActive(dungeon->id))
            lockData.status = LFG_LOCKSTATUS_NOT_IN_SEASON;
        else if (!sConditionMgr->IsPlayerMeetingCondition(player, sDB2Manager.LFGRoleRequirementCondition(dungeon->dbc->ID, player->GetSpecializationRole())))
            lockData.status = LFG_LOCKSTATUS_NOT_COMLETE_CHALANGE; // atm data only for challenges check, same in beta BFA
        // else if (dungeon->dbc->GroupID == LFG_GROUP_NORMAL_LEGION && !player->HasAchieved(ar->achievement)) // Check artifact in Legion
            // lockData.status = LFG_LOCKSTATUS_NOT_HAVE_ARTIFACT;
        // merge faction check with check on invalid TP pos and check on test invalid maps (BUT we still have to send it! LOL, in WoD blizz deleted invalid maps from client DBC)
        else if (!dungeon->dbc->FitsTeam(player->GetTeam()) || DisableMgr::IsDisabledFor(DISABLE_TYPE_MAP, dungeon->map, player) || (dungeon->type != LFG_TYPE_RANDOM && dungeon->x == 0.0f && dungeon->y == 0.0f && dungeon->z == 0.0f) || !dungeon->dbc->IsValid())
            // TODO: for non-faction check find correct reason
            lockData.status = LFG_LOCKSTATUS_WRONG_FACTION;
        else if (dungeon->dbc->MinGear && avgItemLevel < dungeon->dbc->MinGear)
        {
            lockData.currItemLevel = avgItemLevel;
            lockData.reqItemLevel = dungeon->dbc->MinGear;
            lockData.status = LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE;
        }
        else
        {
            AccessRequirement const* ar = sObjectMgr->GetAccessRequirement(dungeon->map, Difficulty(dungeon->difficulty), dungeon->id);
            if (!ar)
                ar = sObjectMgr->GetAccessRequirement(dungeon->map, Difficulty(dungeon->difficulty));
            if (ar)
            {
                if (ar->achievement && !player->HasAchieved(ar->achievement))
                    lockData.status = LFG_LOCKSTATUS_MISSING_ACHIEVEMENT;
                else if (player->GetTeam() == ALLIANCE && ar->quest_A && !player->HasAccountQuest(ar->quest_A))
                    lockData.status = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
                else if (player->GetTeam() == HORDE && ar->quest_H && !player->HasAccountQuest(ar->quest_H))
                    lockData.status = LFG_LOCKSTATUS_QUEST_NOT_COMPLETED;
                else
                {
                    if (ar->item)
                    {
                        if (!player->HasItemCount(ar->item) && (!ar->item2 || !player->HasItemCount(ar->item2)))
                            lockData.status = LFG_LOCKSTATUS_MISSING_ITEM;
                    }
                    else if (ar->item2 && !player->HasItemCount(ar->item2))
                        lockData.status = LFG_LOCKSTATUS_MISSING_ITEM;
                }
            }
        }

        /* check of dbc->RequiredPlayerConditionId ?
         @todo VoA closed if WG is not under team control (LFG_LOCKSTATUS_RAID_LOCKED)
        lockData = LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE;
        lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_LOW_LEVEL;
        lockData = LFG_LOCKSTATUS_ATTUNEMENT_TOO_HIGH_LEVEL;
        */

        if (lockData.status != LFG_LOCKSTATUS_OK)
            lock[dungeon->dbc->Entry()] = lockData;
    }

    return lock;
}

void LFGMgr::SendLfgPlayerLockInfo(Player* player)
{
    uint8 level = player->getLevel();
    LfgDungeonSet const& rewardableDungeons = sLFGMgr->GetRewardableDungeons(level, player->GetSession()->Expansion());
    LfgLockMap const& lock = sLFGMgr->GetLockedDungeons(player->GetGUID());

    WorldPackets::LFG::PlayerInfo info;
    info.BlackListMap.Initialize(lock, player->GetGUID());
    for (auto const& dungeon : rewardableDungeons)
    {
        LfgReward const* reward = sLFGMgr->GetDungeonReward(dungeon, level);
        if (!reward)
            continue;

        Quest const* rewardQuest = sQuestDataStore->GetQuestTemplate(reward->firstQuest);
        Quest const* bonusQuest = sQuestDataStore->GetQuestTemplate(reward->bonusQuestId);

        LFGDungeonData const* dungeonData = sLFGMgr->GetLFGDungeon(dungeon & 0xFFFFF);
        if (!dungeonData)
            continue;

        if (dungeonData->expansion != CURRENT_EXPANSION)
            bonusQuest = nullptr;

        uint32 completedMask = reward->encounterMask ? player->GetEncounterMask(dungeonData, reward) : 0;
        if (player->IsLfgCooldown(dungeon & 0xFFFFF))
            rewardQuest = sQuestDataStore->GetQuestTemplate(reward->otherQuest);

        WorldPackets::LFG::PlayerDungeonInfo dungeonInfo;
        dungeonInfo.Slot = dungeon;
        dungeonInfo.FirstReward = !player->IsLfgCooldown(dungeon & 0xFFFFF);
        dungeonInfo.ShortageEligible = !player->GetGroup();
        dungeonInfo.CompletionQuantity = 1;
        dungeonInfo.CompletionLimit = 1;
        dungeonInfo.CompletionCurrencyID = 0;
        dungeonInfo.SpecificQuantity = 0;
        dungeonInfo.SpecificLimit = 1;
        dungeonInfo.OverallQuantity = 0;
        dungeonInfo.OverallLimit = 1;
        dungeonInfo.PurseWeeklyQuantity = 0;
        dungeonInfo.PurseWeeklyLimit = 0;
        dungeonInfo.PurseQuantity = 0;
        dungeonInfo.PurseLimit = 0;
        dungeonInfo.Quantity = 1;
        dungeonInfo.CompletedMask = completedMask;
        dungeonInfo.EncounterMask = reward->encounterMask;
        dungeonInfo.Reward.Initialize(rewardQuest, player);

        if (bonusQuest)
        {
            for (uint32 i = LFG_ROLE_SHORTAGE_RARE; i < LFG_ROLE_SHORTAGE_MAX; ++i)
            {
                switch (i)
                {
                case LFG_ROLE_SHORTAGE_RARE:
                {
                    WorldPackets::LFG::ShortageReward Reward;
                    Reward.Initialize(bonusQuest, player);
                    Reward.Mask = sLFGMgr->GetShortageRolesForQueue(player->GetGUID(), dungeon & 0xFFFFF);
                    dungeonInfo.ShortageRewards.push_back(Reward);
                    break;
                }
                case LFG_ROLE_SHORTAGE_UNCOMMON:
                case LFG_ROLE_SHORTAGE_PLENTIFUL:
                    dungeonInfo.ShortageRewards.emplace_back();
                    break;
                default:
                    break;
                }
            }
        }
        info.Dungeon.push_back(dungeonInfo);
    }

    player->SendDirectMessage(info.Write());
}

void LFGMgr::SendLfgPartyLockInfo(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    Group* group = player->GetGroup();
    if (!group)
        return;

    WorldPackets::LFG::PartyInfo partyInfo;

    // Get the locked dungeons of the other party members
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* plrg = itr->getSource();
        if (!plrg)
            continue;

        ObjectGuid pguid = plrg->GetGUID();
        if (pguid == guid)
            continue;

        partyInfo.Player.emplace_back();
        WorldPackets::LFG::BlackList& lfgBlackList = partyInfo.Player.back();
        lfgBlackList.PlayerGuid = pguid;
        for (auto const& lock : sLFGMgr->GetLockedDungeons(pguid))
            lfgBlackList.Slots.emplace_back(lock.first, lock.second.status, lock.second.reqItemLevel, lock.second.currItemLevel);
    }

    player->SendDirectMessage(partyInfo.Write());
}

uint8 LFGMgr::GetKicksLeft(ObjectGuid guid)
{
    uint8 kicks = GroupsStore[guid].GetKicksLeft();
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::GetKicksLeft: %s = %u", guid.ToString().c_str(), kicks);
    return kicks;
}

void LFGMgr::RestoreState(ObjectGuid guid, char const* debugMsg, uint32 queueId)
{
    if (guid.IsParty())
    {
        LfgGroupData& data = GroupsStore[guid];
        if (sLog->ShouldLog(LOG_FILTER_LFG, LOG_LEVEL_DEBUG))
        {
            std::string const& ps = GetStateString(data.GetState());
            std::string const& os = GetStateString(data.GetOldState());
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RestoreState: Group: %s (%s) State: %s, oldState: %s", guid.ToString().c_str(), debugMsg, ps.c_str(), os.c_str());
        }
        data.RestoreState();
    }
    else
    {
        LfgPlayerData& data = PlayersStore[guid][queueId];
        if (sLog->ShouldLog(LOG_FILTER_LFG, LOG_LEVEL_DEBUG))
        {
            std::string const& ps = GetStateString(data.GetState());
            std::string const& os = GetStateString(data.GetOldState());
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RestoreState: Player: [" UI64FMTD "] (%s) State: %s, oldState: %s", guid.GetCounter(), debugMsg, ps.c_str(), os.c_str());
        }
        data.RestoreState();
    }
}

void LFGMgr::SetState(ObjectGuid guid, LfgState state, uint32 queueId)
{
    if (guid.IsParty())
    {
        LfgGroupData& data = GroupsStore[guid];
        if (sLog->ShouldLog(LOG_FILTER_LFG, LOG_LEVEL_TRACE))
        {
            std::string const& ns = GetStateString(state);
            std::string const& ps = GetStateString(data.GetState());
            std::string const& os = GetStateString(data.GetOldState());
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetState: Group: [" UI64FMTD "] newState: %s, previous: %s, oldState: %s", guid.GetCounter(), ns.c_str(), ps.c_str(), os.c_str());
        }
        data.SetState(state);
    }
    else
    {
        LfgPlayerData& data = PlayersStore[guid][queueId];
        if (sLog->ShouldLog(LOG_FILTER_LFG, LOG_LEVEL_TRACE))
        {
            std::string const& ns = GetStateString(state);
            std::string const& ps = GetStateString(data.GetState());
            std::string const& os = GetStateString(data.GetOldState());
            TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetState: Player: [" UI64FMTD "] newState: %s, previous: %s, oldState: %s", guid.GetCounter(), ns.c_str(), ps.c_str(), os.c_str());
        }
        data.SetState(state);
    }
}

void LFGMgr::SetDungeon(ObjectGuid guid, uint32 dungeon)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetDungeon: %s dungeon %u", guid.ToString().c_str(), dungeon);
    GroupsStore[guid].SetDungeon(dungeon);
}

void LFGMgr::SetRoles(ObjectGuid guid, uint8 roles, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetRoles: %s roles: %u", guid.ToString().c_str(), roles);
    PlayersStore[guid][queueId].SetRoles(roles);
}

void LFGMgr::SetSelectedDungeons(ObjectGuid guid, LfgDungeonSet const& dungeons, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetSelectedDungeons: %s Dungeons: %s", guid.ToString().c_str(), ConcatenateDungeons(dungeons).c_str());
    PlayersStore[guid][queueId].SetSelectedDungeons(dungeons);
}

void LFGMgr::DecreaseKicksLeft(ObjectGuid guid)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::DecreaseKicksLeft: %s", guid.ToString().c_str());
    GroupsStore[guid].DecreaseKicksLeft();
}

void LFGMgr::RemovePlayerData(ObjectGuid guid, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemovePlayerData: %s", guid.ToString().c_str());
    auto it = PlayersStore.find(guid);
    if (it != PlayersStore.end())
        PlayersStore.erase(it);
}

bool LFGMgr::HasPlayerData(ObjectGuid guid, uint32 queueId)
{
    auto itr = PlayersStore.find(guid);
    if (itr == PlayersStore.end())
        return false;

    auto iter = itr->second.find(queueId);
    return iter != itr->second.end();
}

WorldPackets::LFG::RideTicket const* LFGMgr::GetTicket(ObjectGuid guid, uint32 queueId) const
{
    auto itr = PlayersStore.find(guid);
    if (itr == PlayersStore.end())
        return nullptr;

    auto iter = itr->second.find(queueId);
    if (iter == itr->second.end())
        return nullptr;

    return &iter->second.GetTicket();
}

void LFGMgr::SetTicket(ObjectGuid guid, WorldPackets::LFG::RideTicket const& ticket, uint32 queueId)
{
    PlayersStore[guid][queueId].SetTicket(ticket);
}

uint8 LFGMgr::GetTeam(ObjectGuid guid, uint32 queueId)
{
    return PlayersStore[guid][queueId].GetTeam();
}

void LFGMgr::SetTeam(ObjectGuid guid, uint8 team, uint32 queueId)
{
    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) || sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFG))
        team = 0;

    if (LFGDungeonData const* dungeon = GetLFGDungeon(queueId))
        if (dungeon->type != LFG_TYPE_RANDOM && sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFR))
            team = 0;

    PlayersStore[guid][queueId].SetTeam(team);
}

ObjectGuid LFGMgr::GetLfgGroup(ObjectGuid guid, uint32 queueId)
{
    return PlayersStore[guid][queueId].GetLfgGroup();
}

ObjectGuid LFGMgr::GetGroup(ObjectGuid guid, uint32 queueId)
{
    return PlayersStore[guid][queueId].GetGroup();
}

void LFGMgr::SetGroup(ObjectGuid guid, ObjectGuid group, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetGroup: %s queueId %u", guid.ToString().c_str(), queueId);
    PlayersStore[guid][queueId].SetGroup(group);
}

void LFGMgr::SetLfgGroup(ObjectGuid guid, ObjectGuid group, uint32 queueId)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::SetLfgGroup: %s queueId %u", guid.ToString().c_str(), queueId);
    PlayersStore[guid][queueId].SetLfgGroup(group);
}

void LFGMgr::ClearState(ObjectGuid guid, uint32 queueId)
{
    PlayersStore[guid][queueId].ClearState();
}

void LFGMgr::RemoveGroupData(ObjectGuid guid)
{
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGMgr::RemoveGroupData: %s", guid.ToString().c_str());
    auto it = GroupsStore.find(guid);
    if (it == GroupsStore.end())
        return;

    uint32 queueId = it->second.GetQueueId();
    LfgState state = GetState(guid, queueId);
    GuidSet const& players = it->second.GetPlayers();
    for (auto player : players)
    {
        ObjectGuid guid2 = player;
        SetGroup(player, ObjectGuid::Empty, queueId);
        SetLfgGroup(player, ObjectGuid::Empty, queueId);
        if (state != LFG_STATE_PROPOSAL)
        {
            SetState(player, LFG_STATE_NONE, queueId);
            SendLfgUpdateParty(guid2, LfgUpdateData(LFG_UPDATETYPE_REMOVED_FROM_QUEUE, GetSelectedDungeons(guid2, queueId)));
        }
    }

    {
        std::lock_guard<std::recursive_mutex> _lock(m_lock);
        GroupsStore.erase(it);
    }
}

uint8 LFGMgr::RemovePlayerFromGroup(ObjectGuid gguid, ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    return GroupsStore[gguid].RemovePlayer(guid);
}

void LFGMgr::AddPlayerToGroup(ObjectGuid gguid, ObjectGuid guid)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    GroupsStore[gguid].AddPlayer(guid);
}

void LFGMgr::SetLeader(ObjectGuid gguid, ObjectGuid leader)
{
    GroupsStore[gguid].SetLeader(leader);
}

GuidSet const& LFGMgr::GetPlayers(ObjectGuid guid)
{
    return GroupsStore[guid].GetPlayers();
}

void LFGMgr::InitiBattlgroundCheckRoles(Group* group, ObjectGuid playerGuid, uint32 queueId, uint8 roles, uint8 bgQueueTypeId, WorldPackets::Battleground::IgnorMapInfo ignormap, bool isSkirmish)
{
    std::lock_guard<std::recursive_mutex> _lock(m_lock);
    auto& roleCheck = RoleChecksStore[group->GetGUID()];
    roleCheck.roles.clear();
    roleCheck.cancelTime = time_t(time(nullptr)) + LFG_TIME_ROLECHECK;
    roleCheck.state = LFG_ROLECHECK_INITIALITING;
    roleCheck.leader = playerGuid;
    roleCheck.dungeons = {};
    roleCheck.rDungeonId = 0;
    roleCheck.queueId = 0;
    roleCheck.bgQueueId = queueId;
    roleCheck.bgQueueTypeId = bgQueueTypeId;
    roleCheck.ignormap = ignormap;
    roleCheck.isSkirmish = isSkirmish;

    SetState(group->GetGUID(), LFG_STATE_ROLECHECK, 0);

    WorldPackets::LFG::RideTicket ticket;
    ticket.RequesterGuid = playerGuid;
    ticket.Id = queueId;
    ticket.Type = WorldPackets::LFG::RideType::Battlegrounds;
    ticket.Time = int32(time(nullptr));

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        if (Player* plrg = itr->getSource())
        {
            ObjectGuid pguid = plrg->GetGUID();
            SetTicket(pguid, ticket, queueId);
            SetRoles(pguid, roles, queueId);
            SetTeam(pguid, plrg->GetTeamId(), queueId);

            SetState(pguid, LFG_STATE_ROLECHECK, queueId);
            roleCheck.roles[pguid] = 0;
        }
    }

    UpdateRoleCheck(group->GetGUID(), playerGuid, roles);
}

uint8 LFGMgr::GetPlayerCount(ObjectGuid guid)
{
    return GroupsStore[guid].GetPlayerCount();
}

ObjectGuid LFGMgr::GetLeader(ObjectGuid guid)
{
    return GroupsStore[guid].GetLeader();
}

bool LFGMgr::HasIgnore(ObjectGuid guid1, ObjectGuid guid2)
{
    Player* plr1 = ObjectAccessor::FindPlayer(guid1);
    Player* plr2 = ObjectAccessor::FindPlayer(guid2);
    if (!plr1 || !plr1->CanContact())
        return true;
    if (!plr2 || !plr2->CanContact())
        return true;
    return plr1->GetSocial()->HasIgnore(guid2) || plr2->GetSocial()->HasIgnore(guid1);
}

void LFGMgr::SendLfgRoleChosen(ObjectGuid guid, ObjectGuid pguid, uint8 roles)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        WorldPackets::LFG::RoleChosen chosen;
        chosen.Player = pguid;
        chosen.RoleMask = roles;
        chosen.Accepted = roles != 0;
        player->AddUpdatePacket(chosen.Write());
    }
}

void LFGMgr::SendLfgRoleCheckUpdate(ObjectGuid guid, LfgRoleCheck const& roleCheck, uint8 partyIndex)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        if (roleCheck.roles.empty())
            return;

        LfgDungeonSet dungeons;
        if (roleCheck.rDungeonId)
            dungeons.insert(roleCheck.rDungeonId);
        else
            dungeons = roleCheck.dungeons;

        WorldPackets::LFG::RoleCheckUpdate update;
        update.RoleCheckStatus = roleCheck.state;
        update.IsBeginning = roleCheck.state == LFG_ROLECHECK_INITIALITING;
        update.IsRequeue = roleCheck.state == LFG_ROLECHECK_FINISHED || roleCheck.state == LFG_ROLECHECK_ABORTED;
        update.PartyIndex = partyIndex;
        update.BgQueueID = roleCheck.bgQueueId;
        update.GroupFinderActivityID = 0;

        if (!roleCheck.bgQueueId)
        {
            std::transform(dungeons.begin(), dungeons.end(), std::back_inserter(update.JoinSlots), [this](uint32 dungeonId)
            {
                return GetLFGDungeonEntry(dungeonId);
            });
        }

        auto roles = roleCheck.roles.find(player->GetGUID())->second;

        WorldPackets::LFG::RoleCheckUpdate::CheckUpdateMember updateMember;
        updateMember.Guid = player->GetGUID();
        updateMember.RolesDesired = roles;
        updateMember.Level = player->getLevel();
        updateMember.RoleCheckComplete = roles != 0;
        update.Members.push_back(updateMember);

        for (auto const& i : roleCheck.roles)
        {
            if (i.first == player->GetGUID())
                continue;

            updateMember.Guid = i.first;
            updateMember.RolesDesired = i.second;
            if (CharacterInfo const* characterInfo = sWorld->GetCharacterInfo(i.first))
                updateMember.Level = characterInfo->Level;
            else
                updateMember.Level = player->getLevel();
            updateMember.RoleCheckComplete = i.second != 0;
            update.Members.push_back(updateMember);
        }

        player->AddUpdatePacket(update.Write());
    }
}

void LFGMgr::SendLfgUpdatePlayer(ObjectGuid guid, LfgUpdateData const& data, bool Suspended)
{
    SendUpdateStatus(guid, data, Suspended);
}

void LFGMgr::SendLfgUpdateParty(ObjectGuid guid, LfgUpdateData const& data)
{
    SendUpdateStatus(guid, data, false);
}

void LFGMgr::SendLfgJoinResult(ObjectGuid guid, LfgJoinResultData const& data)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        WorldPackets::LFG::JoinResult result;
        if (auto ticket = GetTicket(player->GetGUID(), data.queueId))
            result.Ticket = *ticket;

        result.Result = data.result;
        if (data.result == LFG_JOIN_ROLE_CHECK_FAILED_2)
            result.ResultDetail = data.state;
        for (auto const& map : data.lockmap)
        {
            WorldPackets::LFG::BlackList list;
            list.Initialize(map.second, map.first);
            result.Slots.push_back(list);
        }

        player->AddUpdatePacket(result.Write());
    }
}

void LFGMgr::SendLfgBootProposalUpdate(ObjectGuid guid, LfgPlayerBoot const& boot)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        auto playerVote = boot.votes.find(guid)->second;
        uint8 votesNum = 0;
        uint8 agreeNum = 0;
        for (auto const& v : boot.votes)
            if (v.second != LFG_ANSWER_PENDING)
            {
                ++votesNum;
                if (v.second == LFG_ANSWER_AGREE)
                    ++agreeNum;
            }

        WorldPackets::LFG::BootPlayer bootPlayer;
        bootPlayer.Info.VoteInProgress = boot.inProgress;
        bootPlayer.Info.VotePassed = playerVote == LFG_ANSWER_DENY;
        bootPlayer.Info.MyVoteCompleted = playerVote != LFG_ANSWER_PENDING;
        bootPlayer.Info.MyVote = playerVote == LFG_ANSWER_AGREE;
        bootPlayer.Info.Target = boot.victim;
        bootPlayer.Info.TotalVotes = votesNum;
        bootPlayer.Info.BootVotes = agreeNum;
        bootPlayer.Info.TimeLeft = (boot.cancelTime - time(nullptr)) / 1000;
        bootPlayer.Info.VotesNeeded = boot.votesNeeded;
        bootPlayer.Info.Reason = boot.reason;
        player->AddUpdatePacket(bootPlayer.Write());
    }
}

void LFGMgr::SendLfgUpdateProposal(ObjectGuid guid, LfgProposal const& proposal)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        ObjectGuid gguid = proposal.players.find(guid)->second.group;
        bool silent = !proposal.isNew && gguid == proposal.group;
        uint32 dungeonEntry = proposal.dungeonId;
        if (!silent)
        {
            auto const& selectedDungeons = GetSelectedDungeons(guid, proposal.queueId);
            if (!selectedDungeons.empty() && selectedDungeons.find(proposal.dungeonId) == selectedDungeons.end())
                dungeonEntry = *selectedDungeons.begin();
        }

        dungeonEntry = GetLFGDungeonEntry(dungeonEntry);

        WorldPackets::LFG::ProposalUpdate update;
        if (auto ticket = GetTicket(player->GetGUID(), proposal.queueId))
            update.Ticket = *ticket;
        update.InstanceID = ObjectGuid::Create<HighGuid::RaidGroup>(dungeonEntry).GetGUIDLow();
        update.ProposalID = proposal.id;
        update.Slot = dungeonEntry;
        update.State = proposal.state;
        update.CompletedMask = proposal.encounters;
        update.ValidCompletedMask = true;
        update.ProposalSilent = silent;
        update.IsRequeue = !proposal.isNew;

        if(LfgReward const* reward = GetDungeonReward(dungeonEntry, player->getLevel()))
            update.EncounterMask = reward->encounterMask;

        for (auto const& i : proposal.players)
        {
            update.Players.emplace_back();
            auto& proposalPlayer = update.Players.back();
            proposalPlayer.Roles = i.second.role;
            proposalPlayer.Me = i.first == guid;
            proposalPlayer.SameParty = i.second.group ? i.second.group == gguid : false;
            proposalPlayer.MyParty = i.second.group ? i.second.group == proposal.group : false;
            proposalPlayer.Responded = i.second.accept != LFG_ANSWER_PENDING;
            proposalPlayer.Accepted = i.second.accept == LFG_ANSWER_AGREE;
        }

        player->AddUpdatePacket(update.Write());
    }
}

void LFGMgr::SendLfgQueueStatus(ObjectGuid guid, LfgQueueStatusData const& data)
{
    if (auto player = ObjectAccessor::FindPlayer(guid))
    {
        WorldPackets::LFG::QueueStatus status;
        if (auto ticket = GetTicket(player->GetGUID(), GetQueueId(data.dungeonId & 0xFFFFF)))
            status.Ticket = *ticket;

        status.AvgWaitTimeMe = data.waitTimeAvg;
        status.AvgWaitTime = data.waitTime;
        status.QueuedTime = data.queuedTime;
        status.Slot = GetLFGDungeonEntry(data.dungeonId);
        status.AvgWaitTimeByRole[0] = data.waitTimeTank;
        status.AvgWaitTimeByRole[1] = data.waitTimeHealer;
        status.AvgWaitTimeByRole[2] = data.waitTimeDps;
        status.LastNeeded[0] = data.queueInfo->tanks;
        status.LastNeeded[1] = data.queueInfo->healers;
        status.LastNeeded[2] = data.queueInfo->dps;
        player->AddUpdatePacket(status.Write());
    }
}

bool LFGMgr::IsLfgGroup(ObjectGuid guid)
{
    return !guid.IsEmpty() && guid.IsParty() && GroupsStore[guid].IsLfgGroup();
}

uint8 LFGMgr::GetQueueTeam(ObjectGuid guid, uint32 queueId)
{
    if (guid.IsParty())
    {
        auto const& players = GetPlayers(guid);
        if (auto pguid = players.empty() ? ObjectGuid::Empty : *players.begin())
            return GetTeam(pguid, queueId);
    }

    return GetTeam(guid, queueId);
}

uint32 LFGMgr::GetQueueId(uint32 dungeonId)
{
    if (LFGDungeonData const* dungeon = GetLFGDungeon(dungeonId))
        return dungeon->random_id ? dungeon->random_id : dungeon->id;

    return 0; //Not found? Oo
}

uint32 LFGMgr::GetQueueId(ObjectGuid guid, uint32 dungeonId)
{
    auto it = GroupsStore.find(guid);
    if (it == GroupsStore.end())
        return GetQueueId(dungeonId);

    return it->second.GetQueueId();
}

void LFGMgr::SetQueueId(ObjectGuid guid, uint32 queueId)
{
    GroupsStore[guid].SetQueueId(queueId);
}

LFGQueue& LFGMgr::GetQueue(ObjectGuid guid, uint32 queueId)
{
    queueId = GetQueueId(queueId);
    return QueuesStore[GetQueueTeam(guid, queueId)][queueId];
}

bool LFGMgr::AllQueued(GuidList const& check, uint32 queueId)
{
    if (check.empty())
        return false;

    for (auto itr : check)
        if (GetState(itr, queueId) != LFG_STATE_QUEUED)
            return false;

    return true;
}

void LFGMgr::Clean()
{
    for (auto& i : QueuesStore)
        i.clear();
}

bool LFGMgr::isOptionEnabled(uint32 option)
{
    return (m_options & option) != 0;
}

uint32 LFGMgr::GetOptions()
{
    return m_options;
}

void LFGMgr::SetOptions(uint32 options)
{
    m_options = options;
}

LfgUpdateData LFGMgr::GetLfgStatus(ObjectGuid guid, uint32 queueId)
{
    LfgUpdateType updateType = LFG_UPDATETYPE_ADDED_TO_QUEUE;

    switch (GetState(guid, queueId))
    {
        case LFG_STATE_NONE:
        case LFG_STATE_ROLECHECK:
        case LFG_STATE_PROPOSAL:
        case LFG_STATE_RAIDBROWSER:
        case LFG_STATE_DUNGEON:
        case LFG_STATE_FINISHED_DUNGEON:
        case LFG_STATE_BOOT:
            updateType = LFG_UPDATETYPE_UPDATE_STATUS;
            break;
        case LFG_STATE_WAITE:
            updateType = LFG_UPDATETYPE_PAUSE;
            break;
        case LFG_STATE_QUEUED:
        default:
            break;
    }
    LfgPlayerData& playerData = PlayersStore[guid][queueId];
    return LfgUpdateData(updateType, playerData.GetState(), playerData.GetSelectedDungeons());
}

bool LFGMgr::IsSeasonActive(uint32 dungeonId)
{
	switch (dungeonId)
    {
        case 285: // The Headless Horseman
            return IsHolidayActive(HOLIDAY_HALLOWS_END);
        case 286: // The Frost Lord Ahune
            return IsHolidayActive(HOLIDAY_FIRE_FESTIVAL);
        case 287: // Coren Direbrew
            return IsHolidayActive(HOLIDAY_BREWFEST);
        case 288: // The Crown Chemical Co.
            return IsHolidayActive(HOLIDAY_LOVE_IS_IN_THE_AIR);
            // pre-cata event dungeons
        case 296:   // Grand Ambassador Flamelash
        case 297:   // Crown Princess Theradras
        case 298:   // Kai'ju Gahz'rilla
        case 299:   // Prince Sarsarun
        case 306:   // Kai'ju Gahz'rilla
        case 308:   // Grand Ambassador Flamelash
        case 309:   // Crown Princess Theradras
        case 310:   // Prince Sarsarun
            return false;
        case 744: // Random Timewalking Dungeon (Burning Crusade)
            return IsHolidayActive(HOLIDAY_TIMEWALKING_BC);
        case 995: // Random Timewalking Dungeon (Wrath of the Lich King)
            return IsHolidayActive(HOLIDAY_TIMEWALKING_WOTLK);
        case 1146: // Random Timewalking Dungeon (Cataclysm)
            return IsHolidayActive(HOLIDAY_TIMEWALKING_CATACLYSM);
        case 1453: // Random Timewalking Dungeon (Mists of Pandaria)
            return IsHolidayActive(HOLIDAY_TIMEWALKING_PANDARIA);
        default:
            break;
    }
    return false;
}

std::string LFGMgr::DumpQueueInfo(bool full)
{
    std::ostringstream o;

    o << "Number of Queues: A " << QueuesStore[0].size() << " H " << QueuesStore[1].size() << " N " << QueuesStore[2].size() << "\n";
    for (auto& i : QueuesStore)
        for (auto& itr : i)
        {
            auto const& queued = itr.second.DumpQueueInfo();
            auto const& compatibles = itr.second.DumpCompatibleInfo(full);
            o << queued << compatibles;
        }

    return o.str();
}

void LFGMgr::SetupGroupMember(ObjectGuid guid, ObjectGuid gguid)
{
    LfgDungeonSet dungeons;
    dungeons.insert(GetDungeon(gguid));
    uint32 queueId = GetQueueId(gguid, *dungeons.begin()&0xFFFFF);
    SetSelectedDungeons(guid, dungeons, queueId);
    SetState(guid, GetState(gguid, queueId), queueId);
    SetGroup(guid, gguid, queueId);
    AddPlayerToGroup(gguid, guid);
}

bool LFGMgr::selectedRandomLfgDungeon(ObjectGuid guid, uint32 queueId)
{
    if (GetState(guid, queueId) != LFG_STATE_NONE)
    {
        LfgDungeonSet const& dungeons = GetSelectedDungeons(guid, queueId);
        if (!dungeons.empty())
        {
            LFGDungeonData const* dungeon = GetLFGDungeon(*dungeons.begin());
            if (dungeon && (dungeon->type == LFG_TYPE_RANDOM || dungeon->seasonal))
                return true;
        }
    }

    return false;
}

bool LFGMgr::inLfgDungeonMap(ObjectGuid guid, uint32 map, Difficulty difficulty, uint32& queueId)
{
    if (uint32 dungeonId = GetDungeon(guid, true))
        if (auto dungeon = GetLFGDungeon(dungeonId))
            if (uint32(dungeon->map) == map && dungeon->difficulty == difficulty)
                return true;

    return false;
}

uint32 LFGMgr::GetLFGDungeonEntry(uint32 id)
{
    if (id)
        if (LFGDungeonData const* dungeon = GetLFGDungeon(id))
            if (LFGDungeonsEntry const* _dbc = dungeon->dbc)
                return _dbc->Entry();

    return 0;
}

LfgDungeonSet LFGMgr::GetRewardableDungeons(uint8 level, uint8 expansion)
{
    LfgDungeonSet randomDungeons;
    for (uint32 i = 0; i < sLfgDungeonsStore.GetNumRows(); i++)
    {
        LFGDungeonData* dungeon = LfgDungeonVStore[i];
        if (!dungeon)
            continue;

        if (dungeon->dbc->CanBeRewarded() && (!dungeon->seasonal || IsSeasonActive(dungeon->id)) && dungeon->expansion <= expansion && dungeon->minlevel <= level && level <= dungeon->maxlevel)
            if (GetDungeonReward(dungeon->dbc->Entry(), level))
                randomDungeons.insert(dungeon->dbc->Entry());
    }
    return randomDungeons;
}

LfgRoleData::LfgRoleData(uint32 dungeonId)
{
    Init(sLFGMgr->GetLFGDungeon(dungeonId));
}

LfgRoleData::LfgRoleData(LFGDungeonData const* data)
{
    Init(data);
}

void LfgRoleData::Init(LFGDungeonData const* data)
{
    tanksNeeded = data->dbc->CountTank;
    healerNeeded = data->dbc->CountHealer;
    dpsNeeded = data->dbc->CountDamage;

    minTanksNeeded = data->dbc->MinCountTank;
    minHealerNeeded = data->dbc->MinCountHealer;
    minDpsNeeded = data->dbc->MinCountDamage;
}

LFGDungeonData::LFGDungeonData() : dbc(nullptr), id(0), random_id(0), x(0.0f), y(0.0f), z(0.0f), o(0.0f), name(""), map(0), type(0), expansion(0), minlevel(0), maxlevel(0), difficulty(DIFFICULTY_NORMAL), internalType(LFG_TYPE_DUNGEON), seasonal(false)
{
}

LFGDungeonData::LFGDungeonData(LFGDungeonsEntry const* _dbc) : dbc(_dbc), id(_dbc->ID), random_id(_dbc->RandomID), x(0.0f), y(0.0f), z(0.0f), o(0.0f), name(_dbc->Name->Str[LOCALE_enUS]), map(_dbc->MapID), type(_dbc->TypeID), expansion(_dbc->ExpansionLevel),
minlevel(_dbc->MinLevel), maxlevel(_dbc->MaxLevel), difficulty(Difficulty(_dbc->DifficultyID)), internalType(_dbc->GetInternalType()), seasonal((_dbc->Flags & LFG_FLAG_SEASONAL) != 0)
{
}

LfgJoinResultData::LfgJoinResultData(LfgJoinResult _result, LfgRoleCheckState _state, uint32 _queueId) : result(_result), state(_state), queueId(_queueId)
{
}

LfgUpdateData::LfgUpdateData(LfgUpdateType _type) : updateType(_type), state(LFG_STATE_NONE)
{
}

LfgUpdateData::LfgUpdateData(LfgUpdateType _type, LfgDungeonSet _dungeons) : updateType(_type), state(LFG_STATE_NONE), dungeons( std::move(_dungeons))
{
}

LfgUpdateData::LfgUpdateData(LfgUpdateType _type, LfgState _state, LfgDungeonSet _dungeons) : updateType(_type), state(_state), dungeons(std::move(_dungeons))
{
}

LfgQueueStatusData::LfgQueueStatusData(uint32 _dungeonId, int32 _waitTime, int32 _waitTimeAvg, int32 _waitTimeTank, int32 _waitTimeHealer, int32 _waitTimeDps, uint32 _queuedTime, LfgQueueData* _queueInfo) : queueInfo(_queueInfo), dungeonId(_dungeonId), waitTime(_waitTime), waitTimeAvg(_waitTimeAvg), waitTimeTank(_waitTimeTank), waitTimeHealer(_waitTimeHealer), waitTimeDps(_waitTimeDps), queuedTime(_queuedTime)
{
}

LfgPlayerRewardData::LfgPlayerRewardData(uint32 random, uint32 current, bool _done, LfgReward const* _reward) : reward(_reward), rdungeonEntry(random), sdungeonEntry(current), done(_done)
{
}

LfgReward::LfgReward(uint32 _maxLevel, uint32 _firstQuest, uint32 _otherQuest, uint32 _bonusQuestId, uint32 _encounterMask) : maxLevel(_maxLevel), firstQuest(_firstQuest), otherQuest(_otherQuest), bonusQuestId(_bonusQuestId), encounterMask(_encounterMask)
{
}

bool LfgReward::RewardPlayer(Player* player, LFGDungeonData const* randomDungeon, uint32 dungeonId) const
{
    bool done = false;
    Quest const* quest = sQuestDataStore->GetQuestTemplate(firstQuest);
    if (!quest)
        return false;

    if (!player->IsLfgCooldown(dungeonId))
    {
        player->AddDelayedEvent(100, [player, quest, randomDungeon] () -> void {
            if (!player)
                return;

            player->RewardQuest(quest, 0, nullptr, false);

            if (uint32 bonusRep = randomDungeon ? randomDungeon->dbc->BonusReputationAmount : 0)
                if (uint32 faction = player->GetLfgBonusFaction())
                    player->GetReputationMgr().ModifyReputation(sFactionStore.LookupEntry(faction), bonusRep);
            });
    }
    else
    {
        done = true;

        if (quest = sQuestDataStore->GetQuestTemplate(otherQuest))
            player->AddDelayedEvent(100, [player, quest] () -> void { if (player) player->RewardQuest(quest, 0, nullptr, false); });
    }

    return done;
}

LfgProposalPlayer::LfgProposalPlayer() : role(0), accept(LFG_ANSWER_PENDING) { }

LfgProposal::LfgProposal(uint32 dungeon) : id(0), dungeonId(dungeon), state(LFG_PROPOSAL_INITIATING), cancelTime(0), encounters(0), isNew(true) { }

uint8 LFGMgr::GetVotesNeededForKick(ObjectGuid gguid)
{
    LFGDungeonData const* dungeonData = GetLFGDungeon(GetDungeon(gguid, true));
    if (!dungeonData)
        return LFG_DUNGEON_KICK_VOTES_NEEDED;

    switch (dungeonData->dbc->GetInternalType())
    {
        case LFG_TYPE_SCENARIO:
            return LFG_SCENARIO_KICK_VOTES_NEEDED;
        case LFG_TYPE_RAID:
            return LFG_RAID_KICK_VOTES_NEEDED;
        default:
            break;
    }

    return LFG_DUNGEON_KICK_VOTES_NEEDED;
}

void LFGMgr::ToggleTesting()
{
    m_Testing = !m_Testing;
}

bool LFGMgr::onTest() const
{
    return m_Testing;
}

void LFGMgr::SetEligibleForCTAReward(ObjectGuid guid, uint8 roles)
{
    CTARewardStore[guid] = roles;
}

uint8 LFGMgr::GetShortageRolesForQueue(ObjectGuid guid, uint32 dungeonId)
{
    return GetQueue(guid, dungeonId).GetShortageRoles();
}

uint8 LFGMgr::GetEligibleRolesForCTA(ObjectGuid guid)
{
    return CTARewardStore[guid];
}

void LFGMgr::RemoveFromQueue(ObjectGuid guid, uint32 queueId)
{
    auto it = PlayerDungeons.find(guid);
    if (it == PlayerDungeons.end())
        return;

    it->second.erase(queueId);
    if (it->second.empty())
        PlayerDungeons.erase(guid);
}

void LFGMgr::RemoveFromGroupQueue(ObjectGuid guid, uint32 queueId)
{
    TC_LOG_ERROR(LOG_FILTER_LFG, "RemoveFromGroupQueue guid %s queueId %u", guid.ToString().c_str(), queueId);

    auto it = GroupDungeons.find(guid);
    if (it == GroupDungeons.end())
        return;

    it->second.erase(queueId);
    if (it->second.empty())
        GroupDungeons.erase(guid);
}

bool LFGMgr::HasQueue(ObjectGuid guid)
{
    auto it = PlayerDungeons.find(guid);
    if (it == PlayerDungeons.end())
        return false;

    return !it->second.empty();
}

bool LFGMgr::HasGroupQueue(ObjectGuid guid)
{
    auto it = GroupDungeons.find(guid);
    if (it == GroupDungeons.end())
        return false;

    return !it->second.empty();
}

void LFGMgr::StartAllOtherQueue(ObjectGuid guid, uint32 queueId)
{
    auto it = PlayerDungeons.find(guid);
    if (it == PlayerDungeons.end())
        return;

    for (auto& queue : it->second)
    {
        if (queueId == queue)
            continue;

        SetState(guid, LFG_STATE_QUEUED, queue);
        SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_ADDED_TO_QUEUE, GetSelectedDungeons(guid, queue)));
    }
}

void LFGMgr::StopAllOtherQueue(ObjectGuid guid, uint32 queueId)
{
    auto it = PlayerDungeons.find(guid);
    if (it == PlayerDungeons.end())
        return;

    for (auto& queue : it->second)
    {
        if (queueId == queue)
            continue;

        SetState(guid, LFG_STATE_WAITE, queue);
        SendLfgUpdatePlayer(guid, LfgUpdateData(LFG_UPDATETYPE_PAUSE, GetSelectedDungeons(guid, queue)));
    }
}

void LFGMgr::SendLfgUpdateQueue(ObjectGuid guid)
{
    auto it = PlayerDungeons.find(guid);
    if (it == PlayerDungeons.end())
        return;

    for (auto& queue : it->second)
        sLFGMgr->SendLfgUpdatePlayer(guid, sLFGMgr->GetLfgStatus(guid, queue), true);
}


} // namespace lfg
