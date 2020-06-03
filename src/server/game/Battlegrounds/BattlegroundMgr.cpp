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

#include "ArenaAll.h"
#include "ArenaAshamanesFall.h"
#include "ArenaBlackrookHold.h"
#include "ArenaBladesEdge.h"
#include "ArenaDalaranSewers.h"
#include "ArenaNagrand.h"
#include "ArenaRuinsOfLordaeron.h"
#include "ArenaTheTigersPeak.h"
#include "ArenaTolvir.h"
#include "BattlegroundAlteracValley.h"
#include "BattlegroundArathiBasin.h"
#include "BattlegroundBattleForGilneas.h"
#include "BattlegroundDeepwindGorge.h"
#include "BattlegroundDM.h"
#include "BattlegroundEyeOfTheStorm.h"
#include "BattlegroundIsleOfConquest.h"
#include "BattlegroundKotmoguTemplate.h"
#include "BattlegroundMgr.h"
#include <utility>
#include "BattlegroundRandom.h"
#include "BattlegroundSilvershardMines.h"
#include "BattlegroundStrandOfTheAncients.h"
#include "BattlegroundTwinPeaks.h"
#include "BattlegroundSeethingShore.h"
#include "BattlegroundWarsongGulch.h"
#include "DatabaseEnv.h"
#include "DisableMgr.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "BrawlBattlegroundSouthshoreVsTarrenMill.h"
#include "GameEventMgr.h"
#include "BrawlBattlegroundShadoPan.h"


QueueSchedulerItem::QueueSchedulerItem(uint32 MMRating, uint8 joinType, uint8 bgQueueTypeId, uint16 bgTypeId, uint8 bracketid, Roles role, uint8 bracket_MinLevel) : MatchMakingRating(MMRating), BgTypeID(bgTypeId), _role(role), JoinType(joinType), BgQueueTypeID(bgQueueTypeId), BracketMinLevel(bracket_MinLevel), BracketID(bracketid)
{
}

BrawlData::BrawlData(uint16 bgTypeId, uint32 holidayId, uint32 journalId, std::string name, std::string description, std::string description2, uint8 type)
{
    BgTypeId = bgTypeId;
    HolidayNameId = holidayId;
    Name = std::move(name);
    Description = description;
    Description2 = description2;
    Type = type;
    JournalId = journalId;

    for (auto holiday : sHolidaysStore)
        if (holiday->HolidayNameID == HolidayNameId && holiday->Region == 2)
            HolidayIDs.push_back(holiday->ID);
}

BattlegroundMgr::BattlegroundMgr()
{
    for (uint16 i = MS::Battlegrounds::BattlegroundTypeId::None; i < MS::Battlegrounds::BattlegroundTypeId::Max; i++)
        _battlegrounds[i].clear();

    _nextRatedArenaUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER);
    _testing = false;
    _spectatorData.clear();
}

BattlegroundMgr::~BattlegroundMgr()
{
    DeleteAllBattlegrounds();
}

BattlegroundMgr* BattlegroundMgr::instance()
{
    static BattlegroundMgr instance;
    return &instance;
}

void BattlegroundMgr::DeleteAllBattlegrounds()
{
    for (uint16 i = MS::Battlegrounds::BattlegroundTypeId::None; i < MS::Battlegrounds::BattlegroundTypeId::Max; ++i)
    {
        for (auto itr = _battlegrounds[i].begin(); itr != _battlegrounds[i].end();)
        {
            Battleground* bg = itr->second;
            _battlegrounds[i].erase(itr++);
            if (!_clientBattlegroundIDs[i][bg->GetBracketId()].empty())
                _clientBattlegroundIDs[i][bg->GetBracketId()].erase(bg->GetClientInstanceID());
            delete bg;
        }
    }

    while (!BGFreeSlotQueue.empty())
        delete BGFreeSlotQueue.front();
}

void BattlegroundMgr::Update(uint32 diff)
{
    m_Functions.Update(diff);

    std::map<uint32, Battleground*>::iterator next;
    for (uint16 i = MS::Battlegrounds::BattlegroundTypeId::None; i < MS::Battlegrounds::BattlegroundTypeId::Max; ++i)
    {
        auto itr = _battlegrounds[i].begin();
        if (itr != _battlegrounds[i].end())
            ++itr;

        for (; itr != _battlegrounds[i].end(); itr = next)
        {
            next = itr;
            ++next;

            Battleground* bg = itr->second;
            if (bg && bg->ToBeDeleted())
            {
                std::lock_guard<std::recursive_mutex> _bg_lock(bg->m_bg_lock);
                _battlegrounds[i].erase(itr);
                if (!_clientBattlegroundIDs[i][bg->GetBracketId()].empty())
                    _clientBattlegroundIDs[i][bg->GetBracketId()].erase(bg->GetClientInstanceID());

                delete bg;
            }
        }
    }

    for (uint16 qtype = MS::Battlegrounds::BattlegroundQueueTypeId::None; qtype < MS::Battlegrounds::BattlegroundQueueTypeId::Max; ++qtype)
        _battlegroundQueues[qtype].UpdateEvents(diff);

    if (!_queueUpdateScheduler.empty())
    {
        auto scheduled = std::vector<QueueSchedulerItem*>();
        std::swap(scheduled, _queueUpdateScheduler);

        for (auto const& v : scheduled)
            _battlegroundQueues[v->BgQueueTypeID].BattlegroundQueueUpdate(diff, v->BgTypeID, v->BracketID, v->JoinType, v->MatchMakingRating > 0, v->_role, v->BracketMinLevel);
    }

    if (sWorld->getIntConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE) && sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER))
    {
        if (_nextRatedArenaUpdate < diff)
        {
            TC_LOG_DEBUG(LOG_FILTER_BATTLEGROUND, "BattlegroundMgr: UPDATING ARENA QUEUES");
            for (uint8 qtype = MS::Battlegrounds::BattlegroundQueueTypeId::Arena2v2; qtype <= MS::Battlegrounds::BattlegroundQueueTypeId::Arena3v3; ++qtype)
                for (uint8 bracket = 0; bracket < MS::Battlegrounds::MaxBrackets; ++bracket)
                    _battlegroundQueues[qtype].BattlegroundQueueUpdate(diff, MS::Battlegrounds::BattlegroundTypeId::ArenaAll, bracket, MS::Battlegrounds::GetBgJoinTypeByQueueTypeID(qtype), true);

            for (uint8 bracket = 0; bracket < MS::Battlegrounds::MaxBrackets; ++bracket)
            {
                _battlegroundQueues[MS::Battlegrounds::BattlegroundQueueTypeId::RatedBattleground].BattlegroundQueueUpdate(diff, MS::Battlegrounds::BattlegroundTypeId::RatedBattleground, bracket, MS::Battlegrounds::GetBgJoinTypeByQueueTypeID(MS::Battlegrounds::BattlegroundQueueTypeId::RatedBattleground), true);
                _battlegroundQueues[MS::Battlegrounds::BattlegroundQueueTypeId::Arena1v1].BattlegroundQueueUpdate(diff, MS::Battlegrounds::BattlegroundTypeId::ArenaAll, bracket, MS::Battlegrounds::GetBgJoinTypeByQueueTypeID(MS::Battlegrounds::BattlegroundQueueTypeId::Arena1v1), true);
                _battlegroundQueues[MS::Battlegrounds::BattlegroundQueueTypeId::ArenaSoloQ3v3].BattlegroundQueueUpdate(diff, MS::Battlegrounds::BattlegroundTypeId::ArenaAll, bracket, MS::Battlegrounds::GetBgJoinTypeByQueueTypeID(MS::Battlegrounds::BattlegroundQueueTypeId::ArenaSoloQ3v3), true);
            }

            _nextRatedArenaUpdate = sWorld->getIntConfig(CONFIG_ARENA_RATED_UPDATE_TIMER);
        }
        else
            _nextRatedArenaUpdate -= diff;
    }
}

void BattlegroundMgr::SendBattlegroundList(Player* player, ObjectGuid const& guid, uint16 bgTypeId)
{
    Battleground* bgTemplate = GetBattlegroundTemplate(bgTypeId);
    if (!bgTemplate)
        return;

    PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    WorldPackets::Battleground::BattlefieldList battlefieldList;
    battlefieldList.BattlemasterGuid = guid;
    battlefieldList.BattlemasterListID = bgTypeId;
    battlefieldList.MinLevel = bracketEntry->MinLevel;
    battlefieldList.MaxLevel = bracketEntry->MaxLevel;
    battlefieldList.PvpAnywhere = guid.IsEmpty();
    battlefieldList.HasWinToday = player->HasWinToday(GetPvpRewardType(bgTemplate));

    if (bgTypeId != MS::Battlegrounds::BattlegroundTypeId::ArenaAll && bgTypeId != MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll && bgTypeId != MS::Battlegrounds::BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm && bgTypeId != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
        if (Battleground* bgtemplate_ = GetBattlegroundTemplate(bgTypeId))
            if (PVPDifficultyEntry const* v = sDB2Manager.GetBattlegroundBracketByLevel(bgtemplate_->GetMapId(), player->getLevel()))
                for (auto const& x : _clientBattlegroundIDs[bgTypeId][v->RangeIndex])
                    battlefieldList.Battlefields.push_back(x);

    player->SendDirectMessage(battlefieldList.Write());
}

void BattlegroundMgr::BuildBattlegroundStatusHeader(WorldPackets::Battleground::BattlefieldStatusHeader* header, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 joinType)
{
    header->Ticket.RequesterGuid = player->GetGUID();
    header->Ticket.Id = ticketId;
    header->Ticket.Type = WorldPackets::LFG::RideType::Battlegrounds;
    header->Ticket.Time = joinTime;
    header->QueueID = bg->GetQueueID();
    header->RangeMin = bg->GetMinLevel();
    header->RangeMax = bg->GetMaxLevel();
    header->TeamSize = bg->IsArena() ? joinType : 0;
    header->InstanceID = bg->GetClientInstanceID();
    header->RegisteredMatch = bg->IsRated();
    header->TournamentRules = bg->UseTournamentRules();
}

void BattlegroundMgr::BuildBattlegroundStatusNone(WorldPackets::Battleground::BattlefieldStatusNone* battlefieldStatus, Player* player, uint32 queueSlot, uint32 joinTime)
{
    battlefieldStatus->Ticket.RequesterGuid = player->GetGUID();
    battlefieldStatus->Ticket.Id = queueSlot;
    battlefieldStatus->Ticket.Time = joinTime;
    battlefieldStatus->Ticket.Type = WorldPackets::LFG::RideType::Battlegrounds;
}

void BattlegroundMgr::BuildBattlegroundStatusNeedConfirmation(WorldPackets::Battleground::BattlefieldStatusNeedConfirmation* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 timeout, uint32 joinType, uint8 bracketId /* = 0*/)
{
    BuildBattlegroundStatusHeader(&battlefieldStatus->Header, bg, player, ticketId, joinTime, joinType);
    battlefieldStatus->Mapid = bg->GetMapId();
    battlefieldStatus->Timeout = timeout;
    battlefieldStatus->Role = player->ConvertLFGRoleToRole(player->GetQueueRoleMask(bracketId, true));
}

void BattlegroundMgr::BuildBattlegroundStatusActive(WorldPackets::Battleground::BattlefieldStatusActive* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 joinType)
{
    BuildBattlegroundStatusHeader(&battlefieldStatus->Header, bg, player, ticketId, joinTime, joinType);
    battlefieldStatus->ShutdownTimer = std::chrono::duration_cast<Seconds>(bg->GetRemainingTime());
    battlefieldStatus->ArenaFaction = player->GetBGTeam() == HORDE ? TEAM_ALLIANCE : TEAM_HORDE;
    battlefieldStatus->LeftEarly = false/*bg->UseTournamentRules() || bg->IsRBG()*/;
    battlefieldStatus->StartTimer = std::chrono::duration_cast<Seconds>(bg->GetElapsedTime());
    battlefieldStatus->Mapid = bg->GetMapId();
}

void BattlegroundMgr::BuildBattlegroundStatusQueued(WorldPackets::Battleground::BattlefieldStatusQueued* battlefieldStatus, Battleground* bg, Player* player, uint32 ticketId, uint32 joinTime, uint32 avgWaitTime, uint32 joinType, bool asGroup)
{
    BuildBattlegroundStatusHeader(&battlefieldStatus->Header, bg, player, ticketId, joinTime, joinType);
    battlefieldStatus->AverageWaitTime = avgWaitTime;
    battlefieldStatus->AsGroup = asGroup;
    battlefieldStatus->SuspendedQueue = true;
    battlefieldStatus->EligibleForMatchmaking = false;
    battlefieldStatus->WaitTime = getMSTimeDiff(joinTime, time(nullptr)) * IN_MILLISECONDS;
}

void BattlegroundMgr::BuildBattlegroundStatusFailed(WorldPackets::Battleground::BattlefieldStatusFailed* battlefieldStatus, Battleground* bg, Player* player, uint32 queueSlot, uint8 result, ObjectGuid const* errorGuid /*= nullptr*/)
{
    battlefieldStatus->Ticket.RequesterGuid = player->GetGUID();
    battlefieldStatus->Ticket.Id = queueSlot;
    battlefieldStatus->Ticket.Type = WorldPackets::LFG::RideType::Battlegrounds;
    battlefieldStatus->Ticket.Time = player->GetBattlegroundQueueJoinTime(MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bg->GetTypeID(), bg->GetJoinType()));
    battlefieldStatus->QueueID = bg->GetQueueID();
    battlefieldStatus->Reason = result;
    if (errorGuid && (result == MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_NOT_IN_BATTLEGROUND || result == MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_JOIN_TIMED_OUT))
        battlefieldStatus->ClientID = *errorGuid;
}

Battleground* BattlegroundMgr::GetBattlegroundThroughClientInstance(uint32 instanceId, uint16 bgTypeId)
{
    Battleground* bg = GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return nullptr;

    if (bg->IsArena())
        return GetBattleground(instanceId, bgTypeId);

    for (auto itr : _battlegrounds[bgTypeId])
        if (itr.second->GetClientInstanceID() == instanceId)
            return itr.second;

    return nullptr;
}

Battleground* BattlegroundMgr::GetBattleground(uint32 InstanceID, uint16 bgTypeId)
{
    if (!InstanceID)
        return nullptr;

    if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::None)
    {
        for (uint16 i = MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley; i < MS::Battlegrounds::BattlegroundTypeId::Max; i++)
            if (auto data = Trinity::Containers::MapGetValuePtr(_battlegrounds[i], InstanceID))
                return data;
        return nullptr;
    }

    return Trinity::Containers::MapGetValuePtr(_battlegrounds[bgTypeId], InstanceID);
}

Battleground* BattlegroundMgr::GetBattlegroundTemplate(uint16 bgTypeId)
{
    return _battlegrounds[bgTypeId].empty() ? nullptr : _battlegrounds[bgTypeId].begin()->second;
}

uint32 BattlegroundMgr::CreateClientVisibleInstanceId(uint16 bgTypeId, uint8 bracketID)
{
    if (MS::Battlegrounds::CheckIsArenaTypeByBgTypeID(bgTypeId))
        return 0;

    uint32 lastId = 0;
    for (auto itr = _clientBattlegroundIDs[bgTypeId][bracketID].begin(); itr != _clientBattlegroundIDs[bgTypeId][bracketID].end();)
    {
        if (++lastId != *itr)
            break;

        lastId = *itr;
    }

    _clientBattlegroundIDs[bgTypeId][bracketID].insert(lastId + 1);
    return lastId + 1;
}

Battleground* BattlegroundMgr::CreateNewBattleground(uint16 bgTypeId, PVPDifficultyEntry const* bracketEntry, uint8 joinType, bool isRated, uint16 generatedType/*=bgTypeId*/, bool useTournamentRules /*= false*/, bool isWarGame /*= false*/)
{
    CreateBattlegroundData const* bgData = GetBattlegroundData(bgTypeId);
    if (!bgData)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
        return nullptr;
    }

    uint16 oldbgTypeId = bgTypeId;
    bool isRandom = (bgData->IsArena && !bgData->IsBrawl) || bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom || bgTypeId == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground || bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll || bgTypeId == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix;
    if (isRandom)
    {
        if (generatedType == MS::Battlegrounds::BattlegroundTypeId::None || generatedType == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom || generatedType == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground)
            return nullptr;

        bgTypeId = generatedType;
    }

    if (oldbgTypeId == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground)
        bgTypeId = Trinity::Containers::SelectRandomContainerElement(MS::Battlegrounds::RatedBattlegroundsContainer);

    Battleground* bg;
    switch (bgTypeId)
    {
        case MS::Battlegrounds::BattlegroundTypeId::ArenaTheTigersPeak:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaTheTigersPeak:
            bg = new ArenaTheTigersPeak;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaBlackrookHold:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaBlackRookHold:
            bg = new ArenaBlackrookHold;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaAshamanesFall:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAshamanesFall:
            bg = new ArenaAshamanesFall;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaTolvir:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaTolViron:
            bg = new ArenaTolvir;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaBladesEdge:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaBladesEdge:
            bg = new ArenaBladesEdge;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaRuinsOfLordaeron:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaRuinsofLordaeron:
            bg = new ArenaRuinsOfLordaeron;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaDalaranSewers:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaDalaranSewers:
            bg = new ArenaDalaranSewers;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaNagrandArena:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaNagrandArena:
            bg = new ArenaNagrandArena;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaAll:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll:
            bg = new ArenaAll;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley:
            bg = new BattlegroundAlteracValley;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongSix:
            bg = new BattlegroundWarsongGulch;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundArathiBasin:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter:
            bg = new BattlegroundArathiBasin;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundEyeOfTheStorm:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
            bg = new BattlegroundEyeOfTheStorm;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundStrandOfTheAncients:
            bg = new BattlegroundStrandOfTheAncients;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundIsleOfConquest:
            bg = new BattlegroundIsleOfConquest;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeepwindGorge:
            bg = new BattlegroundDeepwindGorge;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundTwinPeaks:
            bg = new BattlegroundTwinPeaks;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundBattleForGilneas:
            bg = new BattlegroundBattleForGilneas;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundKotmoguTemplate:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
            bg = new BattlegroundKotmoguTemplate;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundSilvershardMines:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundSilvershardSix:
            bg = new BattlegroundSilvershardMines;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch:
            bg = new BattlegroundDeathMatch;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundSouthshoreVsTarrenMill:
            bg = new BrawlBattlegroundSouthshoreVsTarrenMill;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundSeethingShore:
            bg = new BattlegroundSeethingShore;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundShadoPan:
            bg = new BrawlBattlegroundShadoPan;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::RatedBattleground:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix:
            return nullptr;
        default:
            return nullptr;
    }

    bgData = GetBattlegroundData(bgTypeId);
    if (!bgData)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground: CreateNewBattleground - bg template not found for %u", bgTypeId);
        return nullptr;
    }

    bg->Initialize(bgData);
    bg->SetBracket(bracketEntry);
    bg->SetInstanceID(sMapMgr->GenerateInstanceId());
    bg->SetClientInstanceID(CreateClientVisibleInstanceId(isRandom ? MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom : bgTypeId, bracketEntry->RangeIndex));
    bg->Reset();
    bg->SetStatus(STATUS_WAIT_JOIN);
    bg->SetJoinType(joinType);
    bg->SetRated(isRated);
    bg->SetTypeID(isRandom ? oldbgTypeId : bgTypeId);
    bg->SetRBG(oldbgTypeId == MS::Battlegrounds::BattlegroundTypeId::RatedBattleground);
    bg->SetRandomTypeID(bgTypeId);
    bg->SetTournamentRules(useTournamentRules);
    bg->SetWargame(isWarGame);
    bg->SetIsBrawl(bgData->IsBrawl);

    if (oldbgTypeId == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
        bg->SetArena(true);

    uint8 memberCount = 0;
    auto offset = MS::Battlegrounds::QueueOffsets::Battleground;
    if (isWarGame)
        offset = MS::Battlegrounds::QueueOffsets::Wargame;

    bg->SetQueueID(bgData->bgTypeId | static_cast<uint64>(memberCount & 0x3F) << 24 | offset);

    return bg;
}

void BattlegroundMgr::CreateBattleground(CreateBattlegroundData& data)
{
    Battleground* bg;
    switch (data.bgTypeId)
    {
        case MS::Battlegrounds::BattlegroundTypeId::ArenaTheTigersPeak:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaTheTigersPeak:
            bg = new ArenaTheTigersPeak;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaBlackrookHold:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaBlackRookHold:
            bg = new ArenaBlackrookHold;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaAshamanesFall:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAshamanesFall:
            bg = new ArenaAshamanesFall;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaTolvir:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaTolViron:
            bg = new ArenaTolvir;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaBladesEdge:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaBladesEdge:
            bg = new ArenaBladesEdge;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaRuinsOfLordaeron:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaRuinsofLordaeron:
            bg = new ArenaRuinsOfLordaeron;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaDalaranSewers:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaDalaranSewers:
            bg = new ArenaDalaranSewers;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaNagrandArena:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaNagrandArena:
            bg = new ArenaNagrandArena;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::ArenaAll:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll:
            bg = new ArenaAll;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley:
            bg = new BattlegroundAlteracValley;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongSix:
            bg = new BattlegroundWarsongGulch;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundArathiBasin:
            bg = new BattlegroundArathiBasin;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundEyeOfTheStorm:
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
            bg = new BattlegroundEyeOfTheStorm;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundStrandOfTheAncients:
            bg = new BattlegroundStrandOfTheAncients;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundIsleOfConquest:
            bg = new BattlegroundIsleOfConquest;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundTwinPeaks:
            bg = new BattlegroundTwinPeaks;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundSeethingShore:
            bg = new BattlegroundSeethingShore;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundBattleForGilneas:
            bg = new BattlegroundBattleForGilneas;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix:
            bg = new BattlegroundRandom;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundKotmoguTemplate:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
            bg = new BattlegroundKotmoguTemplate;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundSilvershardMines:
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundSilvershardSix:
            bg = new BattlegroundSilvershardMines;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeepwindGorge:
            bg = new BattlegroundDeepwindGorge;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch:
            bg = new BattlegroundDeathMatch;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundShadoPan:
            bg = new BrawlBattlegroundShadoPan;
            break;
        case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundSouthshoreVsTarrenMill:
            bg = new BrawlBattlegroundSouthshoreVsTarrenMill;
            break;
        default:
            bg = new Battleground;
            break;
    }

    bg->Initialize(&data);
    AddBattleground(bg->GetInstanceID(), bg->GetTypeID(), bg);
}

void BattlegroundMgr::AddBattleground(uint32 InstanceID, uint16 bgTypeId, Battleground* BG)
{
    _battlegrounds[bgTypeId][InstanceID] = BG;
}

void BattlegroundMgr::RemoveBattleground(uint32 instanceID, uint16 bgTypeId)
{
    _battlegrounds[bgTypeId].erase(instanceID);
}

void BattlegroundMgr::CreateInitialBattlegrounds()
{
    uint32 oldMSTime = getMSTime();

    //                                               0   1                 2              3       4           5
    QueryResult result = WorldDatabase.Query("SELECT id, AllianceStartLoc, HordeStartLoc, Weight, ScriptName, MinPlayersPerTeam FROM battleground_template");
    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 battlegrounds. DB table `battleground_template` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 ID = fields[0].GetUInt32();
        if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, ID))
            continue;

        BattlemasterListEntry const* bl = sBattlemasterListStore.LookupEntry(ID);
        if (!bl)
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Battleground ID %u not found in BattlemasterList.dbc. Battleground not created.", ID);
            continue;
        }

        CreateBattlegroundData& data = _battlegroundData[ID];
        data.bgTypeId = uint16(ID);
        data.MinPlayersPerTeam = bl->MinPlayers;
        if (uint32 dbVal = fields[5].GetUInt32())
            data.MinPlayersPerTeam = dbVal;
        data.MaxPlayersPerTeam = bl->MaxPlayers;
        data.LevelMin = bl->MinLevel;
        data.LevelMax = bl->MaxLevel;
        data.IsArena = bl->InstanceType == MS::Battlegrounds::PvpInstanceType::Arena;
        data.IsRbg = bl->Flags == 2;
        data.BattlegroundName = bl->Name->Str[sObjectMgr->GetDBCLocaleIndex()];
        data.MapID = bl->MapID[0];
        data.IsBrawl = bl->Flags & 32;
        data.MaxGroupSize = bl->MaxGroupSize;
        uint32 startId = fields[1].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
            data.TeamStartLoc[TEAM_ALLIANCE].SetPosition(start->Loc);
        else if (!MS::Battlegrounds::IsRandomGeneratedBg(data.bgTypeId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `AllianceStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        startId = fields[2].GetUInt32();
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(startId))
            data.TeamStartLoc[TEAM_HORDE].SetPosition(start->Loc);
        else if (!MS::Battlegrounds::IsRandomGeneratedBg(data.bgTypeId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `battleground_template` for id %u have non-existed WorldSafeLocs.dbc id %u in field `HordeStartLoc`. BG not created.", data.bgTypeId, startId);
            continue;
        }

        uint8 selectionWeight = fields[3].GetUInt8();
        data.scriptId = sObjectMgr->GetScriptId(fields[4].GetCString());

        if (data.IsArena && !data.IsBrawl)
        {
            if (data.bgTypeId != MS::Battlegrounds::BattlegroundTypeId::ArenaAll)
                _selectionWeights[MS::Battlegrounds::IternalPvpTypes::Arena][data.bgTypeId] = selectionWeight;
        }
        else if (data.IsBrawl)
        {
            if (data.bgTypeId != MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll && data.bgTypeId != MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
            {
                if (data.MaxPlayersPerTeam == 15)
                {
                    for (uint32 map : {1672, 617, 1505, 572, 1134, 980, 1552, 1504})
                        if (data.MapID == map)
                        {
                            _selectionWeights[MS::Battlegrounds::IternalPvpTypes::Brawl][data.bgTypeId] = selectionWeight;
                            break;
                        }
                }
                else if (data.MaxPlayersPerTeam == 6)
                {
                    for (uint32 map : {489, 998, 727})
                        if (data.MapID == map)
                        {
                            _selectionWeights[MS::Battlegrounds::IternalPvpTypes::BrawlSix][data.bgTypeId] = selectionWeight;
                            break;
                        }
                }
            }
                
        }
        ///  TODO
        else if (bl->MapID[1] <= 0 && bl->MapID[0] != 1101) // no deathmatch
            _selectionWeights[MS::Battlegrounds::IternalPvpTypes::Battleground][data.bgTypeId] = selectionWeight;

        CreateBattleground(data);
        ++count;
    } 
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u battlegrounds in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void BattlegroundMgr::SendToBattleground(Player* player, uint32 instanceId, uint16 bgTypeId)
{
    if (Battleground* bg = GetBattleground(instanceId, bgTypeId))
        player->TeleportTo(bg->GetMapId(), bg->GetTeamStartPosition(player->GetBGTeamId()));
    else
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "player %u is trying to port to non-existent bg instance %u", player->GetGUID().GetCounter(), instanceId);
}

BattlegroundQueue& BattlegroundMgr::GetBattlegroundQueue(uint8 bgQueueTypeId)
{
    return _battlegroundQueues[bgQueueTypeId];
}

void BattlegroundMgr::SendAreaSpiritHealerQuery(Player* player, Battleground* bg, ObjectGuid const& guid)
{
    uint32 ressurrectTimer = bg->IsBrawl() ? RESURRECTION_INTERVAL_BRAWL : RESURRECTION_INTERVAL;

    WorldPackets::Battleground::AreaSpiritHealerTime healerTime;
    healerTime.HealerGuid = guid;
    healerTime.TimeLeft = ressurrectTimer - bg->GetLastResurrectTime() == -1 ? 0 : ressurrectTimer - bg->GetLastResurrectTime();
    player->SendDirectMessage(healerTime.Write());
}

std::map<uint16, uint8>* BattlegroundMgr::GetSelectionWeight(uint8 iternalPvpType)
{
    if (iternalPvpType >= MS::Battlegrounds::IternalPvpTypes::Max)
        return nullptr;

    return &_selectionWeights[iternalPvpType];
}

void BattlegroundMgr::ToggleTesting()
{
    _testing = !_testing;
    if (_testing)
        sWorld->SendWorldText(LANG_DEBUG_BG_ON);
    else
        sWorld->SendWorldText(LANG_DEBUG_BG_OFF);
}

void BattlegroundMgr::ScheduleQueueUpdate(QueueSchedulerItem* data)
{
    bool found = false;
    for (auto const& v : _queueUpdateScheduler)
    {
        if (v->MatchMakingRating == data->MatchMakingRating && v->JoinType == data->JoinType && v->BgQueueTypeID == data->BgQueueTypeID && v->BgTypeID == data->BgTypeID && v->BracketID == data->BracketID)
        {
            found = true;
            break;
        }
    }

    if (!found)
        _queueUpdateScheduler.push_back(data);
}

uint32 BattlegroundMgr::GetMaxRatingDifference() const
{
    // this is for stupid people who can't use brain and set max rating difference to 0
    uint32 diff = sWorld->getIntConfig(CONFIG_ARENA_MAX_RATING_DIFFERENCE);
    if (diff == 0)
        diff = 5000;
    return diff;
}

uint32 BattlegroundMgr::GetRatingDiscardTimer() const
{
    return sWorld->getIntConfig(CONFIG_ARENA_RATING_DISCARD_TIMER);
}

uint32 BattlegroundMgr::GetPrematureFinishTime() const
{
    return sWorld->getIntConfig(CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER);
}

void BattlegroundMgr::LoadBattleMastersEntry()
{
    uint32 oldMSTime = getMSTime();

    _battleMastersMap.clear();

    QueryResult result = WorldDatabase.Query("SELECT entry, bg_template FROM battlemaster_entry");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 battlemaster entries. DB table `battlemaster_entry` is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId = fields[1].GetUInt32();
        if (!sBattlemasterListStore.LookupEntry(bgTypeId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `battlemaster_entry` contain entry %u for not existed battleground type %u, ignored.", entry, bgTypeId);
            continue;
        }

        _battleMastersMap[entry] = uint16(bgTypeId);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u battlemaster entries in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

uint16 BattlegroundMgr::GetBattleMasterBG(uint32 entry) const
{
    auto itr = _battleMastersMap.find(entry);
    if (itr != _battleMastersMap.end())
        return itr->second;
    return MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch;
}

void BattlegroundMgr::LoadPvpRewards()
{
    /*
    http://www.mmo-champion.com/content/6046-Oct-19-Hotfixes-7-1-PvP-Reward-Changes-Blue-Posts-BlizzCon-Virtual-Ticket-Hosts#71pvprewards

    Activity	                        Honor	Artifact Power
    Rated BG Win (First of the Day)	    600	    800
    Rated BG Win	                    300	    400

    Random BG Win (First of the Day)    300	    400
    Random BG Win	                    150	    200

    Skirmish Win (First of the Day)	    160	    100
    Skirmish Win	                    80	    25

    2v2 Win (First of the Day)	        200	    120
    2v2 Win	                            100	    40

    3v3 Win (First of the Day)	        200	    180
    3v3 Wins	                        100	    60

    Last Week's Rating	Base Item Level	Appearance
    0 - 1399	        840	            Gladiator
    1400 - 1599	        850	            Gladiator
    1600 - 1799	        860	            Gladiator
    1800 - 1999	        865	            Gladiator
    2000 - 2199	        870	            Elite
    2200 - 2399	        875	            Elite
    2400+	            880	            Elite
    */

    uint32 oldMSTime = getMSTime();

    _pvpRewardsContainer.clear();

    if (QueryResult result = WorldDatabase.Query("SELECT `Type`,`BracketLevel`,`BaseLevel`,`ElitLevel`,`BonusBaseLevel`,`ChanceBonusLevel`,`ChestA`,`ChestH`,`ChestChance`,`QuestAFirst`,`QuestAWin`,`QuestAlose`,`QuestHFirst`,`QuestHWin`,`QuestHLose`,`ItemCAFirst`,`ItemCAWin`,`ItemCALose`,`ItemsChance`,`ItemsA`,`ItemsH`,`ItemsAElit`,`ItemsHElit`,`Relics`,`RateLegendary`,`RelicsElit` FROM pvp_reward"))
    {
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            PvpReward& _reward = _pvpRewardsContainer[fields[i++].GetUInt8()];
            _reward.BracketLevel = fields[i++].GetUInt8();
            _reward.BaseLevel = fields[i++].GetUInt16();
            _reward.ElitLevel = fields[i++].GetUInt16();
            _reward.BonusBaseLevel = fields[i++].GetUInt16();
            _reward.ChanceBonusLevel = fields[i++].GetFloat();

            _reward.ChestA = fields[i++].GetUInt32();
            _reward.ChestH = fields[i++].GetUInt32();
            _reward.ChestChance = fields[i++].GetFloat();

            _reward.QuestAFirst = fields[i++].GetUInt32();
            _reward.QuestAWin = fields[i++].GetUInt32();
            _reward.QuestAlose = fields[i++].GetUInt32();
            _reward.QuestHFirst = fields[i++].GetUInt32();
            _reward.QuestHWin = fields[i++].GetUInt32();
            _reward.QuestHLose = fields[i++].GetUInt32();

            _reward.ItemCAFirst = fields[i++].GetUInt32();
            _reward.ItemCAWin = fields[i++].GetUInt32();
            _reward.ItemCALose = fields[i++].GetUInt32();

            _reward.ItemsChance = fields[i++].GetFloat();

            Tokenizer itemA(fields[i++].GetString(), ' ');
            for (auto& x : itemA)
                _reward.ItemsA.emplace_back(atoi(x));

            Tokenizer itemH(fields[i++].GetString(), ' ');
            for (auto& x : itemH)
                _reward.ItemsH.emplace_back(atoi(x));

            Tokenizer itemAElit(fields[i++].GetString(), ' ');
            for (auto& x : itemAElit)
                _reward.ItemsAElit.emplace_back(atoi(x));

            Tokenizer itemHElit(fields[i++].GetString(), ' ');
            for (auto& x : itemHElit)
                _reward.ItemsHElit.emplace_back(atoi(x));

            Tokenizer relic(fields[i++].GetString(), ' ');
            for (auto& x : relic)
                _reward.Relics.emplace_back(atoi(x));

            _reward.RateLegendary = fields[i++].GetFloat();

            Tokenizer relicElit(fields[i++].GetString(), ' ');
            for (auto& x : relicElit)
                _reward.RelicsElit.emplace_back(atoi(x));

        } while (result->NextRow());
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u PvpReward in %u ms", static_cast<uint32>(_pvpRewardsContainer.size()), GetMSTimeDiffToNow(oldMSTime));
}

bool BattlegroundMgr::isTesting() const
{
    return _testing;
}

PvpReward* BattlegroundMgr::GetPvpReward(PvpRewardTypes type)
{
    return &_pvpRewardsContainer[type];
}

PvpRewardTypes BattlegroundMgr::GetPvpRewardType(Battleground* bg)
{
    if (bg->GetMapId() == 1101) // deathmatch
        return PvpReward_DeathMatch;

    if (bg->IsSkirmish())
        return PvpReward_Skirmish;

    if (bg->IsBrawl())
    {
        if (bg->GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll)
            return PvpReward_BrawlArena;
        else
            return PvpReward_Brawl;
    }

    if (bg->IsArena())
    {
        if (bg->GetJoinType() == MS::Battlegrounds::JoinType::Arena2v2)
            return PvpReward_Arena_2v2;
        if (bg->GetJoinType() == MS::Battlegrounds::JoinType::Arena1v1)
            return PvpReward_Arena_1v1;
        return PvpReward_Arena_3v3;
    }

    if (bg->IsRBG())
        return PvpReward_RBG;

    if (bg->IsBattleground())
        return PvpReward_BG;

    return PvpReward_Skirmish;
}

CreateBattlegroundData const* BattlegroundMgr::GetBattlegroundData(uint32 type)
{
    return Trinity::Containers::MapGetValuePtr(_battlegroundData, type);
}

bool BattlegroundMgr::HaveSpectatorData() const
{
    return !_spectatorData.empty();
}

void BattlegroundMgr::EraseSpectatorData(uint32 instanceID)
{
    _spectatorData.erase(instanceID);
}

void BattlegroundMgr::AddSpectatorData(uint32 instanceID, ObjectGuid const& guid)
{
    _spectatorData[instanceID].push_back(guid);
}

std::map<uint32, std::list <ObjectGuid>>& BattlegroundMgr::GetSpectatorData()
{
    return _spectatorData;
}

void BattlegroundMgr::InitWargame(Player* player, ObjectGuid opposingPartyMember, uint64 queueID, bool accept)
{
    if (player->InBattleground())
        return;

    auto group = player->GetGroup();
    if (!group || !group->IsLeader(player->GetGUID()))
        return;

    auto opposingPartyLeader = sObjectAccessor->FindPlayer(opposingPartyMember);
    if (!opposingPartyLeader)
        return;

    auto opposingGroup = opposingPartyLeader->GetGroup();
    if (!opposingGroup || !opposingGroup->IsLeader(opposingPartyLeader->GetGUID()))
        return;

    if (!opposingPartyLeader->HasWargameRequest())
        return;

    auto request = opposingPartyLeader->GetWargameRequest();
    if (request->QueueID != queueID || request->OpposingPartyMemberGUID != player->GetGUID())
        return;

    if (accept)
    {
        ///@TODO: Send notification to opposing party ?
        return;
    }

    uint16 bgTypeId = queueID - MS::Battlegrounds::QueueOffsets::Wargame;
    uint8 arenaType = 0;

    if (MS::Battlegrounds::CheckIsArenaTypeByBgTypeID(bgTypeId))
    {
        if (group->GetMembersCount() != opposingGroup->GetMembersCount())
            return;

        switch (group->GetMembersCount())
        {
            case 2:
                arenaType = MS::Battlegrounds::JoinType::Arena2v2;
                break;
            case 3:
                arenaType = MS::Battlegrounds::JoinType::Arena3v3;
                break;
            case 5:
                arenaType = MS::Battlegrounds::JoinType::Arena5v5;
                break;
            default:
                break;
        }
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeId, nullptr))
        return;

    auto battlegroundTemplate = GetBattlegroundTemplate(bgTypeId);
    if (!battlegroundTemplate)
        return;

    auto bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(battlegroundTemplate->GetMapId(), MAX_LEVEL);
    if (!bracketEntry)
        return;

    auto bGQueueTypeID = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bgTypeId, arenaType);
    auto& bGQueue = _battlegroundQueues[bGQueueTypeID];

    uint16 map = bgTypeId;

    if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::ArenaAll)
        map = bGQueue.GenerateRandomMap(bgTypeId);

    auto battlegroundInstance = CreateNewBattleground(bgTypeId, bracketEntry, arenaType, false, map, request->TournamentRules, true);
    if (!battlegroundInstance)
        return;

    auto prepareGroupToWargame = [&](Group* group1, uint32 team) -> void
    {
        auto groupQueueInfo = bGQueue.AddGroup(sObjectAccessor->FindPlayer(group1->GetLeaderGUID()), group1, battlegroundTemplate->GetTypeID(), bracketEntry, arenaType, false, false, WorldPackets::Battleground::IgnorMapInfo(), 0, team);
        auto avgTime = bGQueue.GetAverageQueueWaitTime(groupQueueInfo, bracketEntry->RangeIndex);

        for (auto itr = group1->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            auto member = itr->getSource();
            if (!member)
                continue;

            WorldPackets::Battleground::BattlefieldStatusQueued battlefieldStatus;
            BuildBattlegroundStatusQueued(&battlefieldStatus, battlegroundInstance, member, member->AddBattlegroundQueueId(bGQueueTypeID), groupQueueInfo->JoinTime, avgTime, groupQueueInfo->JoinType, true);
            member->SendDirectMessage(battlefieldStatus.Write());
        }

        bGQueue.InviteGroupToBG(groupQueueInfo, battlegroundInstance, team);
    };

    prepareGroupToWargame(group, HORDE);
    prepareGroupToWargame(opposingGroup, ALLIANCE);

    battlegroundInstance->StartBattleground();
}

uint8 BattlegroundMgr::GetBrawlIternalGroup() const
{
    tm time{}; // time of first brawl
    time.tm_hour = 8;
    time.tm_min = 0;
    time.tm_mday = 4;
    time.tm_year = 2018 - 1900;
    time.tm_mon = 8;

    time_t start = mktime(&time);
    time_t end = start + 168 * 60 * 60;
    time_t now = std::time(NULL);

    uint32 mod = uint32(ceil((double(now) - double(end)) / double(168 * 2 * 60 * 60)));

    start += 168 * 2 * mod * 60 * 60;
    end += 168 * 2 * mod * 60 * 60;

    if (start <= now && now <= end)
        return mod % 9;

    return 0;
}

bool BattlegroundMgr::IsBrawlActive(uint32& expirationTime) const
{
    tm time{}; // time of first brawl
    time.tm_hour = 8;
    time.tm_min = 0;
    time.tm_mday = 4;
    time.tm_year = 2018 - 1900;
    time.tm_mon = 8;

    time_t start = mktime(&time);
    time_t end = start + 168 * 60 * 60;
    time_t now = std::time(NULL);

    uint32 mod = uint32(ceil((double(now) - double(end)) / double(168 * 2 * 60 * 60)));

    start += 168 * 2 * mod * 60 * 60;
    end += 168 * 2 * mod * 60 * 60;

    if (start <= now && now <= end)
    {
        expirationTime = end - now;
        return true;
    }

    return false;
}

void BattlegroundMgr::InitializeBrawlData()
{
    _brawlTemplatesContainer[0] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble, 257, 322, "PvP Brawl: Warsong scramle", "������ ������", "- ������ ������� ����� ��������� ������ - ���������� ��������� ���� �� ���- ���������� ������ �� ���������� ����������", MS::Battlegrounds::BrawlTypes::Battleground); // 239
    _brawlTemplatesContainer[1] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundShadoPan, 268, 329, "PvP Brawl: Shado-Pan Showdown", "����� 5 �� 5", " � ����� 5 �� 5 � ������ ����� - ���� ������� ���������, ����� �������� ������ � �������� ���������� ������, ����� ��������� ��������", MS::Battlegrounds::BrawlTypes::Lfg); // 240

    // temp
    _brawlTemplatesContainer[2] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter, 262, 321, "PvP Brawl: Arathi Blizzard", "������� ����", "- ��������� ������� - ��������� ���������� - ���������� ����������", MS::Battlegrounds::BrawlTypes::Battleground); // 240
    _brawlTemplatesContainer[3] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm, 255, 324, "PvP Brawl: Gravity Lapse", "���������� ����", "- ���������� ��������� ��������- �������� ����� ���������� ������� - ��� ��������� ������ �������� � ������", MS::Battlegrounds::BrawlTypes::Battleground); // 239
    //

    _brawlTemplatesContainer[4] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlArenaAll, 265, 327, "PvP Brawl: Packed House", "��� 15�15", "- ��������� ������� - 15 ������� �� ������� - ��������� �������, ������� ����������� ������", MS::Battlegrounds::BrawlTypes::Arena); // 239
    _brawlTemplatesContainer[5] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate, 264, 323, "PvP Brawl: Temple of Hotmogu", "", "", MS::Battlegrounds::BrawlTypes::Battleground); // 240
    _brawlTemplatesContainer[6] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix, 267, 348, "Brawl: the magnificent six", "���� ��� 6 �� 6", "� 6 ������� �� ������� � ����� ������ ����", MS::Battlegrounds::BrawlTypes::Battleground); // 240
    _brawlTemplatesContainer[7] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundArathiBasinWinter, 262, 321,"PvP Brawl: Arathi Blizzard", "������� ����", "- ��������� ������� - ��������� ���������� - ���������� ����������", MS::Battlegrounds::BrawlTypes::Battleground); // 240
    _brawlTemplatesContainer[8] = BrawlData(MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm, 255, 324, "PvP Brawl: Gravity Lapse", "���������� ����", "- ���������� ��������� ��������- �������� ����� ���������� ������� - ��� ��������� ������ �������� � ������", MS::Battlegrounds::BrawlTypes::Battleground); // 239
   

    //_brawlTemplatesContainer[2] = BrawlData( 0 /*MS::Battlegrounds::BattlegroundTypeId::BrawlBattleground6 or BrawlBattleground5 */, 263, 325, "PvP Brawl: Deepwind Dunk", "", "", MS::Battlegrounds::BrawlTypes::Battleground); // 240
    //_brawlTemplatesContainer[3] = BrawlData( 0 /*MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundSouthshoreVsTarrenMill*/, 256, 328, "PvP Brawl: Southshore vs. Tarren Mill", "", "", MS::Battlegrounds::BrawlTypes::Battleground); // 238
}

uint16 BattlegroundMgr::GetAtiveBrawlBgTypeId()
{
    return GetBrawlData().BgTypeId;
}

BrawlData BattlegroundMgr::GetBrawlData()
{
    return _brawlTemplatesContainer[GetBrawlIternalGroup()];
}

void BattlegroundMgr::AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
{
    m_Functions.AddDelayedEvent(timeOffset, std::move(function));
}
