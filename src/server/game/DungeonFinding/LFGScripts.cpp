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

/*
 * Interaction between core and LFGScripts
 */

#include "Common.h"
#include "SharedDefines.h"
#include "Player.h"
#include "Group.h"
#include "LFGScripts.h"
#include "LFGMgr.h"
#include "ScriptMgr.h"
#include "ObjectAccessor.h"

namespace lfg
{

LFGPlayerScript::LFGPlayerScript() : PlayerScript("LFGPlayerScript") { }

void LFGPlayerScript::OnLogout(Player* player)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    if (!player->GetGroup())
        sLFGMgr->LeaveLfg(player->GetGUID());
}

void LFGPlayerScript::OnLogin(Player* player)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    // Temporal: Trying to determine when group data and LFG data gets desynched
    ObjectGuid guid = player->GetGUID();
    ObjectGuid gguid;

    if (Group const* group = player->GetGroup())
    {
        gguid = group->GetGUID();
        sLFGMgr->SetupGroupMember(guid, group->GetGUID());
    }

    if (LFGDungeonData const* dungeonData = sLFGMgr->GetLFGDungeon(sLFGMgr->GetDungeon(gguid)))
        player->EnterInTimeWalk(dungeonData->dbc, true);
    /// @todo - Restore LfgPlayerData and send proper status to player if it was in a group
}

void LFGPlayerScript::OnMapChanged(Player* player)
{
    Map const* map = player->GetMap();
    uint32 queueId = 0;

    Group* group = player->GetGroup();
    if (sLFGMgr->inLfgDungeonMap(group ? group->GetGUID() : player->GetGUID(), map->GetId(), map->GetDifficultyID(), queueId))
    {
        // This function is also called when players log in
        // if for some reason the LFG system recognises the player as being in a LFG dungeon,
        // but the player was loaded without a valid group, we'll teleport to homebind to prevent
        // crashes or other undefined behaviour
        if (!group)
        {
            sLFGMgr->LeaveLfg(player->GetGUID(), queueId);
            player->RemoveAurasDueToSpell(LFG_SPELL_LUCK_OF_THE_DRAW);
            player->TeleportTo(player->m_homebindMapId, player->m_homebindX, player->m_homebindY, player->m_homebindZ, 0.0f);
            TC_LOG_ERROR(LOG_FILTER_LFG, "LFGPlayerScript::OnMapChanged, Player %s (%u) is in LFG dungeon map but does not have a valid group! Teleporting to homebind.", player->GetName(), player->GetGUIDLow());
            return;
        }

        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            if (Player* member = itr->getSource())
                player->GetSession()->SendNameQueryOpcode(member->GetGUID());

        if (sLFGMgr->selectedRandomLfgDungeon(player->GetGUID(), queueId))
            player->CastSpell(player, LFG_SPELL_LUCK_OF_THE_DRAW, true);

        if (LFGDungeonData const* dungeonData = sLFGMgr->GetLFGDungeon(sLFGMgr->GetDungeon(group->GetGUID())))
            player->EnterInTimeWalk(dungeonData->dbc, true);
    }
    else
    {
        player->EnterInTimeWalk(nullptr);
        player->RemoveAurasDueToSpell(LFG_SPELL_LUCK_OF_THE_DRAW);
    }
}

LFGGroupScript::LFGGroupScript() : GroupScript("LFGGroupScript") { }

void LFGGroupScript::OnAddMember(Group* group, ObjectGuid const& guid)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER) || !group->isLFGGroup())
        return;

    ObjectGuid gguid = group->GetGUID();
    ObjectGuid leader = group->GetLeaderGUID();
    uint32 queueId = sLFGMgr->GetQueueId(gguid);

    if (leader == guid)
    {
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnAddMember [%s]: added [%s] leader [%s]", gguid.ToString().c_str(), guid.ToString().c_str(), leader.ToString().c_str());
        sLFGMgr->SetLeader(gguid, guid);
    }
    else
    {
        LfgState gstate = sLFGMgr->GetState(gguid, queueId);
        LfgState state = sLFGMgr->GetState(guid, queueId);
        TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnAddMember [%s]: added [%s] leader [%s] gstate: %u, state: %u", gguid.ToString().c_str(), guid.ToString().c_str(), leader.ToString().c_str(), gstate, state);

        if (state == LFG_STATE_QUEUED)
            sLFGMgr->LeaveLfg(guid, queueId);

        if (gstate == LFG_STATE_QUEUED)
            sLFGMgr->LeaveLfg(gguid, queueId);
    }

    sLFGMgr->SetGroup(guid, gguid, queueId);
    sLFGMgr->AddPlayerToGroup(gguid, guid);
}

void LFGGroupScript::OnRemoveMember(Group* group, ObjectGuid const& guid, RemoveMethod method, ObjectGuid const& kicker, char const* reason)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER) || !group->isLFGGroup())
        return;

    ObjectGuid gguid = group->GetGUID();

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnRemoveMember [%s]: remove [%s] Method: %d Kicker: [%s] Reason: %s", gguid.ToString().c_str(), guid.ToString().c_str(), method, kicker.ToString().c_str(), (reason ? reason : ""));

    uint32 queueId = sLFGMgr->GetQueueId(gguid);

    sLFGMgr->StartAllOtherQueue(guid, queueId);

    if (method == GROUP_REMOVEMETHOD_KICK)        // Player have been kicked
    {
        /// @todo - Update internal kick cooldown of kicker
        std::string str_reason = "";
        if (reason)
            str_reason = std::string(reason);
        sLFGMgr->InitBoot(gguid, kicker, guid, str_reason);
        return;
    }

    LfgState state = sLFGMgr->GetState(gguid, queueId);

    // If group is being formed after proposal success do nothing more
    if (state == LFG_STATE_PROPOSAL && method == GROUP_REMOVEMETHOD_DEFAULT)
    {
        // LfgData: Remove player from group
        sLFGMgr->SetGroup(guid, ObjectGuid::Empty, queueId);
        sLFGMgr->SetLfgGroup(guid, ObjectGuid::Empty, queueId);
        sLFGMgr->RemovePlayerFromGroup(gguid, guid);
        return;
    }

    sLFGMgr->LeaveLfg(guid, queueId);
    sLFGMgr->SetGroup(guid, ObjectGuid::Empty, queueId);
    uint8 players = sLFGMgr->RemovePlayerFromGroup(gguid, guid);

    if (Player* player = ObjectAccessor::FindPlayer(guid))
    {
        if (method == GROUP_REMOVEMETHOD_LEAVE && state == LFG_STATE_DUNGEON && players >= sLFGMgr->GetVotesNeededForKick(group->GetGUID()))
            player->CastSpell(player, LFG_SPELL_DUNGEON_DESERTER, true);
        //else if (state == LFG_STATE_BOOT)
            // Update internal kick cooldown of kicked

        sLFGMgr->SendLfgUpdateParty(guid, LfgUpdateData(LFG_UPDATETYPE_LEADER_UNK1, sLFGMgr->GetSelectedDungeons(guid, queueId)));
        if (player->GetMap()->IsDungeon())            // Teleport player out the dungeon
            sLFGMgr->TeleportPlayer(player, true);
    }

    ObjectGuid lguid = sLFGMgr->GetLeader(gguid);
    if (state != LFG_STATE_FINISHED_DUNGEON && lguid != guid) // Need more players to finish the dungeon
        if (Player* leader = ObjectAccessor::FindPlayer(sLFGMgr->GetLeader(gguid)))
            leader->SendLfgOfferContinue(sLFGMgr->GetDungeon(gguid, false));

    sLFGMgr->SetLfgGroup(guid, ObjectGuid::Empty, queueId);
}

void LFGGroupScript::OnDisband(Group* group)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    ObjectGuid gguid = group->GetGUID();
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnDisband %s", gguid.ToString().c_str());

    sLFGMgr->RemoveGroupData(gguid);
}

void LFGGroupScript::OnChangeLeader(Group* group, ObjectGuid const& newLeaderGuid, ObjectGuid const& oldLeaderGuid)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    ObjectGuid gguid = group->GetGUID();

    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnChangeLeader %s: old %s new %s", gguid.ToString().c_str(), newLeaderGuid.ToString().c_str(), oldLeaderGuid.ToString().c_str());
    sLFGMgr->SetLeader(gguid, newLeaderGuid);
}

void LFGGroupScript::OnInviteMember(Group* group, ObjectGuid const& guid)
{
    if (!sLFGMgr->isOptionEnabled(LFG_OPTION_ENABLE_DUNGEON_FINDER | LFG_OPTION_ENABLE_RAID_BROWSER))
        return;

    ObjectGuid gguid = group->GetGUID();
    ObjectGuid leader = group->GetLeaderGUID();
    uint32 queueId = sLFGMgr->GetQueueId(gguid);
    TC_LOG_DEBUG(LOG_FILTER_LFG, "LFGScripts::OnInviteMember %s: invite %s leader %s", gguid.ToString().c_str(), guid.ToString().c_str(), leader.ToString().c_str());
    // No gguid ==  new group being formed
    // No leader == after group creation first invite is new leader
    // leader and no gguid == first invite after leader is added to new group (this is the real invite)
    if (!leader.IsEmpty() && gguid.IsEmpty())
        sLFGMgr->LeaveLfg(leader, queueId);
}

} // namespace lfg
