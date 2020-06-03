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

#include "BattlefieldMgr.h"
#include "BattlegroundDefines.h"
#include "BattlegroundMgr.h"
#include "Chat.h"
#include "DisableMgr.h"
#include "GameTables.h"
#include "NPCPackets.h"
#include "OutdoorPvPMgr.h"
#include "WowTime.hpp"
#include "GameEventMgr.h"
#include "LFGMgr.h"

void WorldSession::HandleBattlemasterHello(WorldPackets::NPC::Hello& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetMap()->GetCreature(packet.Unit);
    if (!unit)
        return;

    if (!unit->isBattleMaster())                             // it's not battlemaster
        return;

    unit->StopMoving();

    uint16 bgTypeId = sBattlegroundMgr->GetBattleMasterBG(unit->GetEntry());

    if (!player->GetBGAccessByLevel(bgTypeId))
    {
        // temp, must be gossip message...
        SendNotification(LANG_YOUR_BG_LEVEL_REQ_ERROR);
        return;
    }

    sBattlegroundMgr->SendBattlegroundList(player, packet.Unit, bgTypeId);
}

void WorldSession::HandleBattlemasterJoin(WorldPackets::Battleground::Join& packet)
{
    Player* player = GetPlayer();
    uint16 queueID = packet.QueueID - MS::Battlegrounds::QueueOffsets::Battleground;

    if (!sBattlemasterListStore.LookupEntry(queueID))
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Battleground: invalid bgtype (%u) received. possible cheater? player guid %u", queueID, _player->GetGUIDLow());
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, queueID))
    {
        ChatHandler(this).PSendSysMessage(LANG_BG_DISABLED);
        return;
    }

    if (player->InBattleground())
        return;

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(queueID);
    if (!bg)
        return;

    PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(queueID);

    if (queueID == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch)
        packet.JoinAsGroup = false;
    
    if (!packet.JoinAsGroup )
    {
        if (player->isUsingLfg())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (!player->CanJoinToBattleground(MS::Battlegrounds::IternalPvpTypes::Battleground))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (player->GetBattlegroundQueueIndex(MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom)) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_RANDOM_BG);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (player->InBattlegroundQueue() && queueID == MS::Battlegrounds::BattlegroundTypeId::BattlegroundRandom)
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_IN_NON_RANDOM_BG);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            return;

        if (!player->HasFreeBattlegroundQueueId())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_TOO_MANY_QUEUES);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        bool isRating = queueID == MS::Battlegrounds::BattlegroundTypeId::BattlegroundDeathMatch;
        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, nullptr, queueID, bracketEntry, 0, isRating, false, packet.BlacklistMap);

        player->SetQueueRoleMask(bracketEntry->RangeIndex, packet.RolesMask);

        WorldPackets::Battleground::BattlefieldStatusQueued queued;
        sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, player, player->AddBattlegroundQueueId(bgQueueTypeId), ginfo->JoinTime, bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex), ginfo->JoinType, false);
        SendPacket(queued.Write());
    }
    else
    {
        Group* grp = player->GetGroup();
        if (!grp)
            return;

        if (grp->GetLeaderGUID() != player->GetGUID())
            return;

        ObjectGuid errorGuid;
        auto err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 0, false, 0, errorGuid);
        packet.JoinAsGroup = (grp->GetMembersCount() >= bg->GetMinPlayersPerTeam());

        if (!err)
        {
            sLFGMgr->InitiBattlgroundCheckRoles(grp, player->GetGUID(), queueID, packet.RolesMask, bgQueueTypeId, packet.BlacklistMap);
            return;
        }

        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member)
                continue;

            if (err)
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, err, &errorGuid);
                member->SendDirectMessage(battlefieldStatus.Write());
                continue;
            }
        }

        if (err)
            return;
    }

    sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, 0, bgQueueTypeId, queueID, bracketEntry->RangeIndex, Roles(packet.RolesMask), bracketEntry->MinLevel));
}

void WorldSession::HandlePVPLogData(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    Player* player = GetPlayer();
    Battleground* bg = player->GetBattleground();
    if (!bg)
        return;

    if (bg->IsArena())
        return;

    WorldPackets::Battleground::PVPLogData pvpLogData;
    bg->BuildPvPLogDataPacket(pvpLogData);
    SendPacket(pvpLogData.Write());
}

void WorldSession::HandleBattlefieldList(WorldPackets::Battleground::ListClient& packet)
{
    if (sBattlemasterListStore.LookupEntry(packet.ListID))
        sBattlegroundMgr->SendBattlegroundList(GetPlayer(), ObjectGuid::Empty, uint16(packet.ListID));
}

const uint32 DisableSpecs[]
{
    SPEC_PALADIN_HOLY,
    SPEC_PALADIN_PROTECTION,

    SPEC_WARRIOR_PROTECTION,

    SPEC_DRUID_BEAR,
    SPEC_DRUID_RESTORATION,

    SPEC_DK_BLOOD,

    SPEC_PRIEST_DISCIPLINE,
    SPEC_PRIEST_HOLY,

    SPEC_SHAMAN_RESTORATION,

    SPEC_MONK_MISTWEAVER,
    SPEC_MONK_BREWMASTER,

    SPEC_DEMON_HUNER_VENGEANCE
};

static auto Arena1v1CheckTalents = [](Player* player) -> bool // Return false, if player have tank/heal spec
{
    if (!player)
        return false;

/*
    if (player->getLevel() < 110)
        return false;

    for (auto curSpec : DisableSpecs)
    {
        if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == curSpec)
        {
            ChatHandler(player).PSendSysMessage("You can't use tank/heal spec");
            return false;
        }
    }*/

    return true;
};

void WorldSession::HandleBattleFieldPort(WorldPackets::Battleground::Port& packet)
{
    if (packet.Ticket.Id > PLAYER_MAX_BATTLEGROUND_QUEUES)
        return;

    Player* player = GetPlayer();
    if (!player->InBattlegroundQueue())
        return;

    uint8 bgQueueTypeId = player->GetBattlegroundQueueTypeId(packet.Ticket.Id);
    if (bgQueueTypeId == MS::Battlegrounds::BattlegroundQueueTypeId::None)
        return;

    if (bgQueueTypeId > MS::Battlegrounds::BattlegroundQueueTypeId::Max)
        return;

    uint16 bgTypeId = MS::Battlegrounds::GetBgTemplateIdByQueueTypeID(bgQueueTypeId);
    if (bgQueueTypeId == MS::Battlegrounds::BattlegroundQueueTypeId::Brawl)
        bgTypeId = sBattlegroundMgr->GetAtiveBrawlBgTypeId();


    if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::None || bgTypeId >= MS::Battlegrounds::BattlegroundTypeId::Max)
        return;

    BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
    GroupQueueInfo ginfo;
    if (!bgQueue.GetPlayerGroupInfoData(player->GetGUID(), &ginfo))
        return;

    if (!ginfo.IsInvitedToBGInstanceGUID && packet.AcceptedInvite)
        return;

    Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, MS::Battlegrounds::IsRandomGeneratedBg(bgTypeId) ? MS::Battlegrounds::BattlegroundTypeId::None : ginfo.NativeBgTypeId);
    if (!bg && !packet.AcceptedInvite)
        bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return;

    // get real bg type
    bgTypeId = bg->GetTypeID();

    PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    if (ginfo.JoinType == 2 || ginfo.JoinType == 3)
    {
        auto group = player->GetGroup();
        if (group && (!group->GetMaxCountOfRolesForArenaQueue(ROLES_HEALER) || !group->GetMaxCountOfRolesForArenaQueue(ROLES_TANK)))
            return;
    }

    if (packet.AcceptedInvite && (ginfo.JoinType == 0))
    {
        if (!player->CanJoinToBattleground(bg->IsArena() || bg->IsSkirmish() ? MS::Battlegrounds::IternalPvpTypes::Arena : MS::Battlegrounds::IternalPvpTypes::Battleground))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            SendPacket(battlefieldStatus.Write());

            packet.AcceptedInvite = false;
        }

        if (player->getLevel() > bg->GetMaxLevel())
            packet.AcceptedInvite = false;
    }
    
    if (bg->GetStatus() == STATUS_WAIT_LEAVE) // if bg just end, then leave
        packet.AcceptedInvite = false;

    if (packet.AcceptedInvite)
    {
        if (!player->IsInvitedForBattlegroundQueueType(bgQueueTypeId))
            return;

        if ((bgQueueTypeId == MS::Battlegrounds::BattlegroundQueueTypeId::Arena1v1 || bgQueueTypeId == MS::Battlegrounds::BattlegroundQueueTypeId::BattlegroundDeathMatch) && !Arena1v1CheckTalents(player))
        {
            ChatHandler(player).PSendSysMessage("You can't join on this specialization!");
            return;
        }

        if (bgQueueTypeId == MS::Battlegrounds::BattlegroundQueueTypeId::ArenaSoloQ3v3 && ginfo.index && (ginfo.index != (player->GetRoleForSoloQ() + MS::Battlegrounds::QueueGroupTypes::NormalHorde)))
        {
            switch (ginfo.index)
            {
                case 4:
                    ChatHandler(player).PSendSysMessage("You can't join on this specialization! Needed melee spec!");
                    break;
                case 5:
                    ChatHandler(player).PSendSysMessage("You can't join on this specialization! Needed range spec!");
                    break;
                case 6:
                    ChatHandler(player).PSendSysMessage("You can't join on this specialization! Needed heal spec!");
                    break;
                default:
                    break;
            }
            return;
        }

        if (!player->InBattleground())
            player->SetBattlegroundEntryPoint();

        if (!player->isAlive())
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }

        if (player->isInFlight())
        {
            player->GetMotionMaster()->MovementExpired();
            player->CleanupAfterTaxiFlight();
        }

        WorldPackets::Battleground::BattlefieldStatusActive battlefieldStatus;
        sBattlegroundMgr->BuildBattlegroundStatusActive(&battlefieldStatus, bg, player, packet.Ticket.Id, player->GetBattlegroundQueueJoinTime(bgQueueTypeId), ginfo.JoinType);
        player->SendDirectMessage(battlefieldStatus.Write());

        bgQueue.RemovePlayer(player->GetGUID(), false);

        if (Battleground* currentBg = player->GetBattleground())
            currentBg->RemovePlayerAtLeave(player->GetGUID(), false, true);

        player->SetBattlegroundId(bg->GetInstanceID(), bgTypeId);
        player->SetBGTeam(ginfo.Team);
        sBattlegroundMgr->SendToBattleground(player, ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
    }
    else // leave queue
    {
        if (Group * group = player->GetGroup()) // leave group of leaver too
        {
            if (group->GetLeaderGUID() == player->GetGUID())
            {
                for (auto itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    auto member = itr->getSource();
                    if (!member)
                        continue;

                    WorldPackets::Battleground::BattlefieldStatusNone none;
                    sBattlegroundMgr->BuildBattlegroundStatusNone(&none, member, packet.Ticket.Id, member->GetBattlegroundQueueJoinTime(bgQueueTypeId));
                    member->GetSession()->SendPacket(none.Write());

                    WorldPackets::Battleground::BattlefieldStatusFailed failed;
                    sBattlegroundMgr->BuildBattlegroundStatusFailed(&failed, bg, member, packet.Ticket.Id, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LEAVE_QUEUE);
                    member->GetSession()->SendPacket(failed.Write());

                    if (bg && bg->IsArena() && bg->IsRated() && bg->GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1)
                        if (bg->GetStatus() == STATUS_WAIT_JOIN || bg->GetStatus() == STATUS_IN_PROGRESS)
                            member->SendOperationsAfterDelay(OAD_ARENA_DESERTER);

                    member->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
                    bgQueue.RemovePlayer(member->GetGUID(), true);
                }
            }
        }
        else
        {
            WorldPackets::Battleground::BattlefieldStatusNone none;
            sBattlegroundMgr->BuildBattlegroundStatusNone(&none, player, packet.Ticket.Id, player->GetBattlegroundQueueJoinTime(bgQueueTypeId));
            SendPacket(none.Write());

            WorldPackets::Battleground::BattlefieldStatusFailed failed;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&failed, bg, player, packet.Ticket.Id, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LEAVE_QUEUE);
            SendPacket(failed.Write());

            if (bg && bg->IsArena() && bg->IsRated() && bg->GetJoinType() != MS::Battlegrounds::JoinType::Arena1v1)
                if (bg->GetStatus() == STATUS_WAIT_JOIN || bg->GetStatus() == STATUS_IN_PROGRESS)
                    player->SendOperationsAfterDelay(OAD_ARENA_DESERTER);

            player->RemoveBattlegroundQueueId(bgQueueTypeId);  // must be called this way, because if you move this call to queue->removeplayer, it causes bugs
            bgQueue.RemovePlayer(player->GetGUID(), true);
        }

        if (!ginfo.JoinType)
            sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(ginfo.MatchmakerRating, ginfo.JoinType, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex));
    }
}

void WorldSession::HandleLeaveBattlefield(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    // not allow leave battleground in combat
    if (_player->isInCombat())
        if (Battleground* bg = _player->GetBattleground())
            if (bg->GetStatus() != STATUS_WAIT_LEAVE)
                return;

    _player->LeaveBattleground();
}

void WorldSession::HandleBattlefieldStatus(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    Player* player = GetPlayer();

    for (uint8 i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
    {
        uint8 bgQueueTypeId = player->GetBattlegroundQueueTypeId(i);
        if (!bgQueueTypeId)
            continue;

        uint8 joinType = MS::Battlegrounds::GetBgJoinTypeByQueueTypeID(bgQueueTypeId);

        auto bg = player->GetBattleground();
        if (bg && bg->GetJoinType() == joinType)
        {
            WorldPackets::Battleground::BattlefieldStatusActive battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusActive(&battlefieldStatus, bg, player, i, player->GetBattlegroundQueueJoinTime(bgQueueTypeId), joinType);
            SendPacket(battlefieldStatus.Write());
            continue;
        }

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(player->GetGUID(), &ginfo))
            continue;

        if (ginfo.IsInvitedToBGInstanceGUID)
        {
            bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, ginfo.NativeBgTypeId);
            if (!bg)
                continue;

            WorldPackets::Battleground::BattlefieldStatusNeedConfirmation battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusNeedConfirmation(&battlefieldStatus, bg, player, i, player->GetBattlegroundQueueJoinTime(bgQueueTypeId), getMSTimeDiff(getMSTime(), ginfo.RemoveInviteTime), joinType);
            SendPacket(battlefieldStatus.Write());
        }
        else
        {
            bg = sBattlegroundMgr->GetBattlegroundTemplate(ginfo.NativeBgTypeId);
            if (!bg)
                continue;

            PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
            if (!bracketEntry)
                continue;

            WorldPackets::Battleground::BattlefieldStatusQueued queued;
            sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, player, i, ginfo.JoinTime, bgQueue.GetAverageQueueWaitTime(&ginfo, bracketEntry->RangeIndex), joinType, ginfo.Players.size() > 1);
            SendPacket(queued.Write());
        }
    }
}

void WorldSession::JoinBracket(uint8 bracketType, uint8 rolesMask /*= ROLES_DEFAULT*/, uint16 extraBgTypeId /*= 0*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (player->InBattleground())
        return;

    uint16 bgTypeId = MS::Battlegrounds::BattlegroundTypeId::None;
    switch (bracketType)
    {
        case MS::Battlegrounds::BracketType::Arena2v2:
        case MS::Battlegrounds::BracketType::Arena3v3:
            bgTypeId = MS::Battlegrounds::BattlegroundTypeId::ArenaAll;
            break;
        case MS::Battlegrounds::BracketType::RatedBattleground:
            bgTypeId = MS::Battlegrounds::BattlegroundTypeId::RatedBattleground;
            break;
        case MS::Battlegrounds::BracketType::Brawl:
            bgTypeId = extraBgTypeId;
            break;
        default:
            break;
    }

    if (bgTypeId == MS::Battlegrounds::BattlegroundTypeId::None)
        return;

    auto bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
    {
        TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "Battleground: template bg not found BgTypeID %u", bgTypeId);
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, bgTypeId))
    {
        ChatHandler(this).PSendSysMessage(LANG_ARENA_DISABLED);
        return;
    }

    bgTypeId = bg->GetTypeID();

    auto bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    auto jointype = MS::Battlegrounds::GetJoinTypeByBracketSlot(bracketType);
    auto bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bgTypeId, jointype);

    if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
        return;

    auto& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);

    if (bracketType != MS::Battlegrounds::BracketType::Brawl)
    {
        Group* grp = player->GetGroup();
        if (!grp || grp->GetLeaderGUID() != player->GetGUID())
            return;

        ObjectGuid errorGuid;
        if (uint8 err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, jointype, true, bracketType, errorGuid))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed failed;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&failed, bg, player, 0, err, &errorGuid);
            SendPacket(failed.Write());
            return;
        }

        uint32 matchmakerRating = grp->GetAverageMMR(bracketType);
        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, grp, bgTypeId, bracketEntry, jointype, true, false, WorldPackets::Battleground::IgnorMapInfo(), matchmakerRating);
        uint32 avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex);
        uint32 joinTime = ginfo->JoinTime;

        for (auto itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            auto member = itr->getSource();
            if (!member)
                continue;

            WorldPackets::Battleground::BattlefieldStatusQueued queued;
            sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, member, member->AddBattlegroundQueueId(bgQueueTypeId), joinTime, avgTime, ginfo->JoinType, true);
            member->SendDirectMessage(queued.Write());
        }

        sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(matchmakerRating, jointype, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex, static_cast<Roles>(rolesMask)));
        return;
    }

    // if brawls
    if (auto grp = player->GetGroup())
    {
        if (grp->GetLeaderGUID() != player->GetGUID())
            return;

        ObjectGuid errorGuid;
        if (auto err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, 0, false, bracketType, errorGuid))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed failed;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&failed, bg, player, 0, err, &errorGuid);
            SendPacket(failed.Write());
            return;
        }

        sLFGMgr->InitiBattlgroundCheckRoles(grp, player->GetGUID(), bgTypeId, rolesMask, bgQueueTypeId, WorldPackets::Battleground::IgnorMapInfo(), false);
        return;

       /* auto ginfo = bgQueue.AddGroup(player, grp, bgTypeId, bracketEntry);
        auto avgTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex);
        auto joinTime = ginfo->JoinTime;

        for (auto itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            auto member = itr->getSource();
            if (!member)
                continue;

            WorldPackets::Battleground::BattlefieldStatusQueued queued;
            sBattlegroundMgr->BuildBattlegroundStatusQueued(&queued, bg, member, member->AddBattlegroundQueueId(bgQueueTypeId), joinTime, avgTime, 0, true);
            auto const& message = queued.Write();
            SendPacket(message);
            member->SendDirectMessage(message);
        }*/
    }
    else
    {
        if (player->isUsingLfg())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (!player->CanJoinToBattleground(MS::Battlegrounds::IternalPvpTypes::Brawl))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        player->SetQueueRoleMask(bracketEntry->RangeIndex, rolesMask);

        auto ginfo = bgQueue.AddGroup(player, nullptr, bgTypeId, bracketEntry);

        WorldPackets::Battleground::BattlefieldStatusQueued battlefieldStatus;
        sBattlegroundMgr->BuildBattlegroundStatusQueued(&battlefieldStatus, bg, player, player->AddBattlegroundQueueId(bgQueueTypeId), ginfo->JoinTime, bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex), 0, false);
        SendPacket(battlefieldStatus.Write());
    }

    sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, 0, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex, static_cast<Roles>(rolesMask), bracketEntry->MinLevel));
}

void WorldSession::HandleBattlemasterJoinArena(WorldPackets::Battleground::JoinArena& packet)
{
    JoinBracket(packet.TeamSizeIndex, packet.Roles);
}

void WorldSession::HandleBattlemasterJoinBrawl(WorldPackets::Battleground::BattlemasterJoinBrawl& packet)
{
    uint32 temp = 0;
    if (!sBattlegroundMgr->IsBrawlActive(temp))
        return;

    auto data = sBattlegroundMgr->GetBrawlData();
    JoinBracket(MS::Battlegrounds::BracketType::Brawl, packet.RolesMask, data.BgTypeId);
}

void WorldSession::HandleJoinRatedBattleground(WorldPackets::Battleground::JoinRatedBattleground& /*packet*/)
{
    JoinBracket(MS::Battlegrounds::BracketType::RatedBattleground);
}

void WorldSession::HandleRequestRatedInfo(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    GetPlayer()->SendPvpRatedStats();
}

void WorldSession::HandleRequestPvpOptions(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    WorldPackets::Battleground::PVPOptionsEnabled options;
    options.RatedArenas = true;
    options.ArenaSkirmish = true;
    options.PugBattlegrounds = true;
    options.WargameBattlegrounds = true;
    options.WargameArenas = true;
    options.RatedBattlegrounds = true;
    SendPacket(options.Write());
}

void WorldSession::HandleRequestPvpReward(WorldPackets::Battleground::RequestPVPRewards& /*packet*/)
{
    GetPlayer()->SendPvpRewards();
}

//! This is const data used for calc some field for SMSG_RATED_BATTLEFIELD_INFO 
void WorldSession::HandlePersonalRatedInfoRequest(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    WorldPackets::Battleground::ConquestFormulaConstants constants;
    constants.PvpMinCPPerWeek = 1500;
    constants.PvpMaxCPPerWeek = 3000;
    constants.PvpCPBaseCoefficient = 1511.26f;
    constants.PvpCPExpCoefficient = 1639.28f;
    constants.PvpCPNumerator = 0.00412f;
    SendPacket(constants.Write());
}

void WorldSession::HandleAreaSpiritHealerQuery(WorldPackets::Battleground::AreaSpiritHealerQuery& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetMap()->GetCreature(packet.HealerGuid);
    if (!unit)
        return;

    if (!unit->isSpiritService())
        return;

    if (Battleground* bg = player->GetBattleground())
        sBattlegroundMgr->SendAreaSpiritHealerQuery(player, bg, packet.HealerGuid);

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetCurrentZoneID()))
        bf->SendAreaSpiritHealerQuery(player, packet.HealerGuid);

    if (auto outdoorPvP = player->GetOutdoorPvP())
        outdoorPvP->SendAreaSpiritHealerQueryOpcode(player, packet.HealerGuid);
}

void WorldSession::HandleAreaSpiritHealerQueue(WorldPackets::Battleground::AreaSpiritHealerQueue& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Creature* unit = player->GetMap()->GetCreature(packet.HealerGuid);
    if (!unit)
        return;

    if (!unit->isSpiritService())
        return;

    if (Battleground* bg = player->GetBattleground())
        bg->AddPlayerToResurrectQueue(packet.HealerGuid, player->GetGUID());

    if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(player->GetCurrentZoneID()))
    {
        bf->RemovePlayerFromResurrectQueue(player->GetGUID());
        bf->AddPlayerToResurrectQueue(packet.HealerGuid, player->GetGUID());
    }

    if (auto outdoorPvP = player->GetOutdoorPvP())
        outdoorPvP->AddPlayerToResurrectQueue(packet.HealerGuid, player->GetGUID());
}

void WorldSession::HandleJoinSkirmish(WorldPackets::Battleground::JoinSkirmish& packet)
{
    Player* player = GetPlayer();
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(MS::Battlegrounds::BattlegroundTypeId::ArenaAll);
    if (!bg)
        return;

    if (!MS::Battlegrounds::IsSkirmishJoinTypes(packet.Bracket))
        return;

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_BATTLEGROUND, MS::Battlegrounds::BattlegroundTypeId::ArenaAll))
    {
        ChatHandler(this).PSendSysMessage(LANG_ARENA_DISABLED);
        return;
    }

    uint16 bgTypeId = bg->GetTypeID();

    if (player->InBattleground())
        return;

    PVPDifficultyEntry const* bracketEntry = sDB2Manager.GetBattlegroundBracketByLevel(bg->GetMapId(), player->getLevel());
    if (!bracketEntry)
        return;

    uint8 jointype = MS::Battlegrounds::JoinType::Arena2v2;
    uint8 bgQueueTypeId = MS::Battlegrounds::GetBgQueueTypeIdByBgTypeID(bgTypeId, jointype);

    if (!packet.JoinAsGroup)
    {
        if (player->isUsingLfg())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (!player->CanJoinToBattleground(MS::Battlegrounds::IternalPvpTypes::Skirmish))
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (!player->isAlive())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DEAD);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        if (player->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
            return;

        if (!player->HasFreeBattlegroundQueueId())
        {
            WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
            sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, player, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_TOO_MANY_QUEUES);
            SendPacket(battlefieldStatus.Write());
            return;
        }

        player->SetQueueRoleMask(bracketEntry->RangeIndex, packet.RolesMask);

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(bgQueueTypeId);
        GroupQueueInfo* ginfo = bgQueue.AddGroup(player, nullptr, bgTypeId, bracketEntry, jointype);

        WorldPackets::Battleground::BattlefieldStatusQueued battlefieldStatus;
        sBattlegroundMgr->BuildBattlegroundStatusQueued(&battlefieldStatus, bg, player, player->AddBattlegroundQueueId(bgQueueTypeId), ginfo->JoinTime, bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->RangeIndex), jointype, false);
        SendPacket(battlefieldStatus.Write());
    }
    else
    {
        Group* grp = player->GetGroup();
        if (!grp)
            return;

        if (grp->GetLeaderGUID() != player->GetGUID())
            return;

        for (auto const& v : grp->GetMemberSlots())
        {
            auto const& member = ObjectAccessor::FindPlayer(v.Guid);
            if (!member)
                continue;

            if (member->isUsingLfg())
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_LFG_CANT_USE_BATTLEGROUND);
                auto const& message = battlefieldStatus.Write();
                SendPacket(message);
                member->SendDirectMessage(message);
                return;
            }

            if (!member->CanJoinToBattleground(MS::Battlegrounds::IternalPvpTypes::Skirmish))
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS);
                auto const& message = battlefieldStatus.Write();
                SendPacket(message);
                member->SendDirectMessage(message);
                return;
            }

            if (!member->isAlive())
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_GROUP_JOIN_BATTLEGROUND_DEAD);
                auto const& message = battlefieldStatus.Write();
                SendPacket(message);
                member->SendDirectMessage(message);
                return;
            }

            if (member->GetBattlegroundQueueIndex(bgQueueTypeId) < PLAYER_MAX_BATTLEGROUND_QUEUES)
                return;

            if (!member->HasFreeBattlegroundQueueId())
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, MS::Battlegrounds::GroupJoinBattlegroundResult::ERR_BATTLEGROUND_TOO_MANY_QUEUES);
                auto const& message = battlefieldStatus.Write();
                SendPacket(message);
                member->SendDirectMessage(message);
                return;
            }
        }

        ObjectGuid errorGuid;
        uint8 err = grp->CanJoinBattlegroundQueue(bg, bgQueueTypeId, jointype, false, 0, errorGuid);

        if (!err)
        {
            sLFGMgr->InitiBattlgroundCheckRoles(grp, player->GetGUID(), bgTypeId, packet.RolesMask, bgQueueTypeId, WorldPackets::Battleground::IgnorMapInfo(), true);
            return;
        }

        for (GroupReference* itr = grp->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->getSource();
            if (!member)
                continue;

            if (err)
            {
                WorldPackets::Battleground::BattlefieldStatusFailed battlefieldStatus;
                sBattlegroundMgr->BuildBattlegroundStatusFailed(&battlefieldStatus, bg, member, 0, err, &errorGuid);
                member->SendDirectMessage(battlefieldStatus.Write());
                continue;
            }
        }

        if (err)
            return;
    }

    sBattlegroundMgr->ScheduleQueueUpdate(new QueueSchedulerItem(0, jointype, bgQueueTypeId, bgTypeId, bracketEntry->RangeIndex, Roles(packet.RolesMask)));
}

void WorldSession::HandleHearthAndResurrect(WorldPackets::Battleground::HearthAndResurrect& /*packet*/)
{
    if (_player->isInFlight())
        return;

    AreaTableEntry const* atEntry = sAreaTableStore.LookupEntry(_player->GetCurrentAreaID());
    if (!atEntry || !(atEntry->Flags[0] & AREA_FLAG_CAN_HEARTH_AND_RESURRECT))
        return;

    Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(_player->GetCurrentZoneID());
    if (!bf || !bf->IsWarTime())
        return;

    _player->BuildPlayerRepop();
    _player->ResurrectPlayer(1.0f);
    _player->TeleportTo(_player->m_homebindMapId, _player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, _player->GetOrientation());
}

void WorldSession::HandleUpdatePrestigeLevel(WorldPackets::Battleground::NullCmsg& /*packet*/)
{
    Player* player = GetPlayer();

    HonorInfo* honorInfo = player->GetHonorInfo();
    if (honorInfo->HonorLevel != HonorInfo::MaxHonorLevel || honorInfo->PrestigeLevel >= sWorld->getIntConfig(CONFIG_MAX_PRESTIGE_LEVEL))
        return;

    if (!honorInfo->IncreasePrestigeLevel())
        return;

    honorInfo->HonorLevel = 0;
    honorInfo->NextHonorLevel = 1;

    player->UpdateAchievementCriteria(CRITERIA_TYPE_PRESTIGE_LEVEL_UP, honorInfo->PrestigeLevel);

    if (GtHonorLevelEntry const* data = sHonorLevelGameTable.GetRow(honorInfo->NextHonorLevel))
        honorInfo->NextHonorAtLevel = data->Prestige[honorInfo->PrestigeLevel];

    player->UpdateHonorFields(true); // Update data fo honor level

    player->SetUInt32Value(PLAYER_FIELD_HONOR, honorInfo->CurrentHonorAtLevel);
    player->SetUInt32Value(PLAYER_FIELD_HONOR_LEVEL, honorInfo->HonorLevel);
    player->SetUInt32Value(PLAYER_FIELD_HONOR_NEXT_LEVEL, honorInfo->NextHonorAtLevel);
    player->SetUInt32Value(PLAYER_FIELD_PRESTIGE, honorInfo->PrestigeLevel);

    player->RemoveAllPvPTalent();
    player->SendTalentsInfoData(false);
}

void WorldSession::HandleAcceptWargameInvite(WorldPackets::Battleground::AcceptWargameInvite& packet)
{
    ObjectGuid playerGUID = _player->GetGUID();
    ObjectGuid opposingPartyMember = packet.OpposingPartyMember;
    uint64 queueID = packet.QueueID;
    bool accept = packet.Accept;
    sBattlegroundMgr->AddDelayedEvent(100, [playerGUID, opposingPartyMember, queueID, accept]() -> void
    {
        if (auto player = sObjectAccessor->FindPlayer(playerGUID))
            sBattlegroundMgr->InitWargame(player, opposingPartyMember, queueID, accept);
    });
}

void WorldSession::HandleStartWarGame(WorldPackets::Battleground::StartWargame& packet)
{
    if (!_player->GetGroup())
        return;

    if (_player->GetGroup()->GetLeaderGUID() != _player->GetGUID())
        return;

    Player* opposingPartyLeader = sObjectAccessor->FindPlayer(packet.OpposingPartyMember);
    if (!opposingPartyLeader)
        return;

    if (!opposingPartyLeader->GetGroup())
        return;

    if (opposingPartyLeader->GetGroup()->GetLeaderGUID() != opposingPartyLeader->GetGUID())
        return;

    if (_player->HasWargameRequest())
        return;

    auto request = new WargameRequest();
    request->OpposingPartyMemberGUID = packet.OpposingPartyMember;
    request->TournamentRules = packet.TournamentRules;
    request->CreationDate = time(nullptr);
    request->QueueID = packet.QueueID;

    _player->SetWargameRequest(request);

    WorldPackets::Battleground::CheckWargameEntry response;
    response.OpposingPartyBnetAccountID = _player->GetSession()->GetBattlenetAccountGUID();
    response.OpposingPartyMember = _player->GetGUID();
    response.QueueID = packet.QueueID;
    response.TimeoutSeconds = 60;
    response.TournamentRules = packet.TournamentRules;
    opposingPartyLeader->SendDirectMessage(response.Write());
}

void WorldSession::HandleRequestPvpBrawlInfo(WorldPackets::Battleground::RequestPvpBrawlInfo& /*packet*/)
{
    uint32 expirationTimer = MS::Utilities::Globals::InSeconds::Week;
    auto isHolidayActive = sBattlegroundMgr->IsBrawlActive(expirationTimer);

    WorldPackets::Battleground::RequestPvpBrawlInfoResponse response;
    response.IsActive = isHolidayActive;

    if (isHolidayActive)
    {
        response.TimeToEnd = expirationTimer;
        auto data = sBattlegroundMgr->GetBrawlData();
        response.BattlegroundTypeId = data.BgTypeId;
        response.Name = data.Name;
        response.Description = "";
        response.Description2 = "";
        response.BrawlType = 0;

        if(auto info = sHolidayNamesStore.LookupEntry(data.HolidayNameId))
            if (strlen(info->Name->Str[GetSessionDbLocaleIndex()]))
                response.Name = info->Name->Str[GetSessionDbLocaleIndex()];

        if (auto info = sAdventureJournalStore.LookupEntry(data.JournalId))
            if (strlen(info->Description->Str[GetSessionDbLocaleIndex()]))
                response.Description2 = info->Description->Str[GetSessionDbLocaleIndex()];
    }

    SendPacket(response.Write());
}

void WorldSession::HandleRequstCrowdControlSpell(WorldPackets::Battleground::RequstCrowdControlSpell& packet)
{
    if (Battleground* bg = _player->GetBattleground())
    {
        bool foundPlayer = false;

        for (auto itr : bg->GetPlayers())
        {
            if (itr.first == _player->GetGUID())
                continue;

            if (itr.first == packet.PlayerGuid)
            {
                foundPlayer = true;
                break;
            }
        }

        if (!foundPlayer)
            return;

        if (Player* opponent = ObjectAccessor::FindPlayer(packet.PlayerGuid))
        {
            WorldPackets::Battleground::ArenaCrowdControlSpells response;
            response.PlayerGuid = opponent->GetGUID();
            response.CrowdControlSpellID = 195710;

            for (uint32 i = 0; i < MAX_TALENT_COLUMNS; ++i)
            {
                for (auto const& talent : sDB2Manager.GetPvpTalentByPosition(opponent->getClass(), 0, i))
                {
                    if (!opponent->HasPvPTalent(talent->SpellID))
                        continue;

                    response.CrowdControlSpellID = talent->SpellID;
                    break;
                }
            }

            opponent->SetCrowdControlSpellId(response.CrowdControlSpellID);

            SendPacket(response.Write());
        }
    }
}
