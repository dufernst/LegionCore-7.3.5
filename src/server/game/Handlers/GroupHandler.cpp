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

#include "PartyPackets.h"
#include "GlobalFunctional.h"
#include "GroupMgr.h"
#include "SocialMgr.h"
#include "BattlegroundMgr.h"

class Aura;

void WorldSession::SendPartyResult(PartyOperation operation, std::string const& member, PartyResult res, uint32 val /* = 0 */)
{
    WorldPackets::Party::PartyCommandResult packet;
    packet.Name = member;
    packet.Command = operation;
    packet.Result = res;
    packet.ResultData = val;
    SendPacket(packet.Write());
}

void WorldSession::HandlePartyInvite(WorldPackets::Party::PartyInviteClient& packet)
{
    Player* c_player = GetPlayer();
    if (!c_player)
        return;

    if (c_player->IsSpectator())
        return;

    // cheating
    if (!normalizePlayerName(packet.TargetName))
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    Player* player = sObjectAccessor->FindPlayerByName(packet.TargetName);
    if (!player)
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    // restrict invite to GMs
    if (!sWorld->getBoolConfig(CONFIG_ALLOW_GM_GROUP) && !c_player->isGameMaster() && player->isGameMaster())
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    if (player->HasPlayerExtraFlag(PLAYER_EXTRA_INVISIBLE_STATUS) && !c_player->isGameMaster())
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_BAD_PLAYER_NAME_S);
        return;
    }

    // can't group with
    if (!c_player->isGameMaster() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && c_player->GetTeam() != player->GetTeam()  
        || player->GetMapId() == 1101 || c_player->GetMapId() == 1101) // deathmatch
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_PLAYER_WRONG_FACTION);
        return;
    }
    if (c_player->InInstance() && player->InInstance() && c_player->GetInstanceId() != player->GetInstanceId() && c_player->GetMapId() == player->GetMapId())
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_TARGET_NOT_IN_INSTANCE_S);
        return;
    }

    if (player->InInstance() && player->GetDungeonDifficultyID() != c_player->GetDungeonDifficultyID())
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_IGNORING_YOU_S);
        return;
    }

    if (player->GetSocial()->HasIgnore(c_player->GetGUID()))
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_IGNORING_YOU_S);
        return;
    }

    Group* group = c_player->GetGroup();
    if (group && group->isBGGroup())
        group = c_player->GetOriginalGroup();

    Group* group2 = player->GetGroup();
    if (group2 && group2->isBGGroup())
        group2 = player->GetOriginalGroup();

    if (group2 || player->GetGroupInvite())
    {
        SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_ALREADY_IN_GROUP_S);

        //! tell the player that they were invited but it failed as they were already in a group
        if (group2) 
        {
            WorldPackets::Party::PartyInvite partyInvite;
            partyInvite.Initialize(c_player, packet.ProposedRoles, false);
            player->SendDirectMessage(partyInvite.Write());
        }
        return;
    }

    if (group)
    {
        // not have permissions for invite
        if (!group->IsLeader(c_player->GetGUID()) && !group->IsAssistant(c_player->GetGUID()) && !(group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT))
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_NOT_LEADER);
            return;
        }
        // not have place
        if (group->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
            return;
        }
    }

    // ok, but group not exist, start a new group
    // but don't create and save the group to the DB until
    // at least one person joins
    if (!group)
    {
        group = new Group;
        // new group: if can't add then delete
        if (!group->AddLeaderInvite(c_player))
        {
            delete group;
            return;
        }
        if (!group->AddInvite(player))
        {
            group->RemoveAllInvites();
            delete group;
            return;
        }
    }
    else
    {
        // already existed group: if can't add then just leave
        if (!group->AddInvite(player))
            return;
    }

    WorldPackets::Party::PartyInvite partyInvite;
    partyInvite.Initialize(c_player, packet.ProposedRoles, true);
    player->SendDirectMessage(partyInvite.Write());

    SendPartyResult(PARTY_OP_INVITE, packet.TargetName, ERR_PARTY_RESULT_OK);
}

void WorldSession::HandlePartyInviteResponse(WorldPackets::Party::PartyInviteResponse& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Group* group = player->GetGroupInvite();
    if (!group)
        return;

    if (packet.Accept)
    {
        // Remove player from invitees in any case
        group->RemoveInvite(player);

        if (group->GetLeaderGUID() == player->GetGUID())
        {
            TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "HandleGroupAcceptOpcode: player %s(%d) tried to accept an invite to his own group", player->GetName(), player->GetGUIDLow());
            return;
        }

        // Group is full
        if (group->IsFull())
        {
            SendPartyResult(PARTY_OP_INVITE, "", ERR_GROUP_FULL);
            return;
        }

        Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());

        // Forming a new group, create it
        if (!group->IsCreated())
        {
            // This can happen if the leader is zoning. To be removed once delayed actions for zoning are implemented
            if (!leader)
            {
                group->RemoveAllInvites();
                return;
            }

            // If we're about to create a group there really should be a leader present
            ASSERT(leader);
            group->RemoveInvite(leader);
            group->Create(leader);
            sGroupMgr->AddGroup(group);
        }

        // Everything is fine, do it, PLAYER'S GROUP IS SET IN ADDMEMBER!!!
        if (!group->AddMember(player))
            return;

        group->BroadcastGroupUpdate();
    }
    else
    {
        // Remember leader if online (group pointer will be invalid if group gets disbanded)
        Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());

        // uninvite, group can be deleted
        player->UninviteFromGroup();

        if (!leader || !leader->GetSession())
            return;

        leader->SendDirectMessage(WorldPackets::Party::GroupDecline(player->GetName()).Write());
    }
}

void WorldSession::HandlePartyUninvite(WorldPackets::Party::PartyUninvite& packet)
{
    if (packet.TargetGUID == GetPlayer()->GetGUID())
        return;

    PartyResult res = GetPlayer()->CanUninviteFromGroup();
    if (res != ERR_PARTY_RESULT_OK)
    {
        SendPartyResult(PARTY_OP_UNINVITE, "", res);
        return;
    }

    Group* grp = GetPlayer()->GetGroup();
    if (!grp)
        return;

    if (auto player = GetPlayer())
        if (!player->CanKickFromChallenge())
            return;

    if (grp->IsLeader(packet.TargetGUID))
    {
        SendPartyResult(PARTY_OP_UNINVITE, "", ERR_NOT_LEADER);
        return;
    }

    if (grp->IsMember(packet.TargetGUID))
    {
        Player::RemoveFromGroup(grp, packet.TargetGUID, GROUP_REMOVEMETHOD_KICK, GetPlayer()->GetGUID(), packet.Reason.c_str());
        return;
    }

    if (Player* player = grp->GetInvited(packet.TargetGUID))
    {
        player->UninviteFromGroup();
        return;
    }

    SendPartyResult(PARTY_OP_UNINVITE, "", ERR_TARGET_NOT_IN_GROUP_S);
}

void WorldSession::HandleSetPartyLeader(WorldPackets::Party::SetPartyLeader& packet)
{
    Player* player = ObjectAccessor::FindPlayer(packet.TargetGUID);
    Group* group = GetPlayer()->GetGroup();

    if (!group || !player)
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()) || player->GetGroup() != group)
        return;

    // Prevent exploits with instance saves
    for (GroupReference *itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* plr = itr->getSource())
            if (plr->GetMap() && plr->GetMap()->Instanceable())
                return;

    group->ChangeLeader(packet.TargetGUID, packet.PartyIndex);
    group->SendUpdate();
}

void WorldSession::HandleSetRole(WorldPackets::Party::SetRole& packet)
{
    Group* group = GetPlayer()->GetGroup();
    uint8 oldRole = group ? group->GetLfgRoles(packet.TargetGUID) : 0;
    if (oldRole == packet.Role)
        return;

    WorldPackets::Party::RoleChangedInform roleChangedInform;
    roleChangedInform.PartyIndex = packet.PartyIndex;
    roleChangedInform.From = GetPlayer()->GetGUID();
    roleChangedInform.ChangedUnit = packet.TargetGUID;
    roleChangedInform.OldRole = oldRole;
    roleChangedInform.NewRole = packet.Role;

    if (group)
    {
        group->BroadcastPacket(roleChangedInform.Write(), false);
        group->SetLfgRoles(packet.TargetGUID, packet.Role);
    }
    else
        SendPacket(roleChangedInform.Write());
}

void WorldSession::HandleLeaveGroup(WorldPackets::Party::LeaveGroup& /*packet*/)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Group* grp = player->GetGroup();
    if (!grp)
        return;

    if (player->InBattleground())
    {
        SendPartyResult(PARTY_OP_INVITE, "", ERR_INVITE_RESTRICTED);
        return;
    }

    SendPartyResult(PARTY_OP_LEAVE, player->GetName(), ERR_PARTY_RESULT_OK);
    player->RemoveFromGroup(GROUP_REMOVEMETHOD_LEAVE);
}

void WorldSession::HandleSetLootMethod(WorldPackets::Party::SetLootMethod& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group || group->RollIsActive())
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()) || group->isLFGGroup())
        return;

    if (packet.LootThreshold < ITEM_QUALITY_UNCOMMON || packet.LootThreshold > ITEM_QUALITY_ARTIFACT)
        return;

    if (packet.LootMethod == MASTER_LOOT && !group->IsMember(packet.LootMasterGUID))
        return;

    group->SetLootMethod(static_cast<LootMethod>(packet.LootMethod));
    group->SetLooterGuid(packet.LootMasterGUID);
    group->SetLootThreshold(static_cast<ItemQualities>(packet.LootThreshold));
    group->SendUpdate();
}

void WorldSession::HandleMinimapPing(WorldPackets::Party::MinimapPingClient& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (!player->GetGroup())
        return;

    WorldPackets::Party::MinimapPing minimapPing;
    minimapPing.Sender = player->GetGUID();
    minimapPing.PositionX = packet.PositionX;
    minimapPing.PositionY = packet.PositionY;
    player->GetGroup()->BroadcastPacket(minimapPing.Write(), true, -1, player->GetGUID());
}

void WorldSession::HandleUpdateRaidTarget(WorldPackets::Party::UpdateRaidTarget& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Group* group = player->GetGroup();
    if (!group)
        return;

    if (packet.Symbol == -1)
        group->SendTargetIconList(packet.PartyIndex);
    else if (group->IsLeader(player->GetGUID()) || group->IsAssistant(player->GetGUID()))
        group->SetTargetIcon(packet.Symbol, packet.Target, player->GetGUID(), packet.PartyIndex);
}

void WorldSession::HandleConvertRaid(WorldPackets::Party::ConvertRaid& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group || group->InChallenge())
        return;

    if (_player->InBattleground())
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()) || group->GetMembersCount() < 2)
        return;

    SendPartyResult(PARTY_OP_INVITE, "", ERR_PARTY_RESULT_OK);

    if (packet.Raid && !group->isRaidGroup())
        group->ConvertToRaid();
    else
        group->ConvertToGroup();
}

void WorldSession::HandleChangeSubGroup(WorldPackets::Party::ChangeSubGroup& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (packet.NewSubGroup >= MAX_RAID_SUBGROUPS)
        return;

    ObjectGuid senderGuid = GetPlayer()->GetGUID();
    if (!group->IsLeader(senderGuid) && !group->IsAssistant(senderGuid) && !(group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT))
        return;

    if (!group->HasFreeSlotSubGroup(packet.NewSubGroup))
        return;

    group->ChangeMembersGroup(packet.TargetGUID, packet.NewSubGroup);
}

//! ToDo: Write swipe command.
void WorldSession::HandleSwapSubGroups(WorldPackets::Party::SwapSubGroups& /*packet*/)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    group->SendUpdate();
}

void WorldSession::HandleSetEveryoneIsAssistant(WorldPackets::Party::SetEveryoneIsAssistant& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()))
        return;

    group->ChangeFlagEveryoneAssistant(packet.EveryoneIsAssistant);
}

void WorldSession::HandleSetAssistantLeader(WorldPackets::Party::SetAssistantLeader& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()))
        return;

    group->SetGroupMemberFlag(packet.Target, packet.Apply, MEMBER_FLAG_ASSISTANT);
    group->SendUpdate();
}

void WorldSession::HandleSetPartyAssignment(WorldPackets::Party::SetPartyAssignment& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    ObjectGuid senderGuid = GetPlayer()->GetGUID();
    if (!group->IsLeader(senderGuid) && !group->IsAssistant(senderGuid) && !(group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT))
        return;

    switch (packet.Assignment)
    {
        case GROUP_ASSIGN_MAINASSIST:
            group->RemoveUniqueGroupMemberFlag(MEMBER_FLAG_MAINASSIST);
            group->SetGroupMemberFlag(packet.Target, packet.Set, MEMBER_FLAG_MAINASSIST);
            break;
        case GROUP_ASSIGN_MAINTANK:
            group->RemoveUniqueGroupMemberFlag(MEMBER_FLAG_MAINTANK);
            group->SetGroupMemberFlag(packet.Target, packet.Set, MEMBER_FLAG_MAINTANK);
        default:
            break;
    }

    group->SendUpdate();
}

void WorldSession::HandleDoReadyCheck(WorldPackets::Party::DoReadyCheck& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()) && !(group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT))
        return;

    group->SetReadyCheckCount(1);

    WorldPackets::Party::ReadyCheckStarted readyCheckStarted;
    readyCheckStarted.PartyGUID = group->GetGUID();
    readyCheckStarted.PartyIndex = packet.PartyIndex;
    readyCheckStarted.InitiatorGUID = GetPlayer()->GetGUID();
    readyCheckStarted.Duration = READY_CHECK_DURATION;
    group->BroadcastPacket(readyCheckStarted.Write(), false, -1);

    group->OfflineReadyCheck();
}

void WorldSession::HandleReadyCheckResponse(WorldPackets::Party::ReadyCheckResponseClient& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    group->SetReadyCheckCount(group->GetReadyCheckCount() + 1);

    WorldPackets::Party::ReadyCheckResponse response;
    response.PartyGUID = group->GetGUID();
    response.Player = GetPlayer()->GetGUID();
    response.IsReady = packet.IsReady;
    group->BroadcastPacket(response.Write(), true);

    if (group->GetReadyCheckCount() >= group->GetMembersCount())
    {
        WorldPackets::Party::ReadyCheckCompleted readyCheckCompleted;
        readyCheckCompleted.PartyIndex = packet.PartyIndex;
        readyCheckCompleted.PartyGUID = group->GetGUID();
        group->BroadcastPacket(readyCheckCompleted.Write(), true);
    }
}

void WorldSession::HandleRequestPartyJoinUpdates(WorldPackets::Party::RequestPartyJoinUpdates& packet)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    group->SendUpdate();
    group->SendTargetIconList(packet.PartyIndex);
    group->SendRaidMarkersChanged(this, packet.PartyIndex);
}

void WorldSession::HandleRequestPartyMemberStats(WorldPackets::Party::RequestPartyMemberStats& packet)
{
    WorldPackets::Party::PartyMemberStats partyMemberStats;

    Player* player = ObjectAccessor::FindPlayer(packet.TargetGUID);
    if (!player)
    {
        if (sBattlegroundMgr->isTesting())
            partyMemberStats.Initialize(packet.TargetGUID);
        else
        {
            partyMemberStats.MemberStats.GUID = packet.TargetGUID;
            partyMemberStats.MemberStats.Status = MEMBER_STATUS_OFFLINE;
        }
    }
    else
        partyMemberStats.Initialize(player);

    SendPacket(partyMemberStats.Write());
}

void WorldSession::HandleOptOutOfLoot(WorldPackets::Party::OptOutOfLoot& packet)
{
    GetPlayer()->SetPassOnGroupLoot(packet.PassOnLoot);
}

void WorldSession::HandleInitiateRolePoll(WorldPackets::Party::InitiateRolePoll& packet)
{
    if (!GetPlayer()->GetGroup())
        return;

    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (!group->IsLeader(GetPlayer()->GetGUID()) && !group->IsAssistant(GetPlayer()->GetGUID()) && !(group->GetGroupFlags() & GROUP_FLAG_EVERYONE_ASSISTANT))
    {
        SendPartyResult(PARTY_OP_INVITE, "", ERR_NOT_LEADER);
        return;
    }

    WorldPackets::Party::RolePollInform rolePollInform;
    rolePollInform.From = GetPlayer()->GetGUID();
    rolePollInform.PartyIndex = packet.PartyIndex;
    group->BroadcastPacket(rolePollInform.Write(), true);
}

void WorldSession::HandleClearRaidMarker(WorldPackets::Party::ClearRaidMarker& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    Group* group = player->GetGroup();
    if (!group)
        return;

    if (group->isRaidGroup() && !group->IsLeader(player->GetGUID()) && !group->IsAssistant(player->GetGUID()))
        return;

    group->DeleteRaidMarker(packet.MarkerId);
}
