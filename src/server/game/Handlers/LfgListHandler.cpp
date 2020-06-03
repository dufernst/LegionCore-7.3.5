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

#include "WorldSession.h"
#include "LFGListMgr.h"
#include "Group.h"
#include "LfgListPackets.h"
#include "SocialMgr.h"
#include "Chat.h"

void WorldSession::HandleRequestLfgListBlackList(WorldPackets::LfgList::RequestLfgListBlacklist& /*packet*/)
{
    SendPacket(WorldPackets::LfgList::LfgListUpdateBlacklist().Write()); /// Activity and Reason loop - We dont need it
}

void WorldSession::HandleLfgListSearch(WorldPackets::LfgList::LfgListSearch& packet)
{
    WorldPackets::LfgList::LfgListSearchResults results;
    if (!sGroupFinderCategoryStore.LookupEntry(packet.CategoryID))
    {
        SendPacket(results.Write());
        return;
    }

    auto list = sLFGListMgr->GetFilteredList(packet.CategoryID, packet.SearchTerms, packet.LanguageSearchFilter, GetPlayer());
    results.AppicationsCount = list.size();

    for (auto& lfgEntry : list)
    {
        WorldPackets::LfgList::ListSearchResult result;
        auto group = lfgEntry->ApplicationGroup;
        if (!group)
            continue;

        auto leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
        if (!leader)
            continue;

        if (lfgEntry->PrivateGroup)
            if ((!sSocialMgr->HasInFriendsList(GetPlayer(), group->GetLeaderGUID()) || 
                !sSocialMgr->HasInFriendsList(leader, GetPlayer()->GetGUID())) && 
                (GetPlayer()->GetGuildId() == 0 ||GetPlayer()->GetGuildId() != leader->GetGuildId()))
                continue;

        auto activityID = lfgEntry->GroupFinderActivityData->ID;

        result.ApplicationTicket.RequesterGuid = group->GetGUID();
        result.ApplicationTicket.Id = group->GetGUIDLow();
        result.ApplicationTicket.Type = WorldPackets::LFG::RideType::LfgListApplication;
        result.ApplicationTicket.Time = lfgEntry->CreationTime;
        result.UnkGuid1 = group->GetLeaderGUID();
        result.UnkGuid2 = group->GetLeaderGUID();
        result.UnkGuid3 = group->GetLeaderGUID();
        result.UnkGuid4 = group->GetLeaderGUID();
        result.BNetFriendsGuids = sSocialMgr->GetBNetFriendsGuids(activityID);
        result.NumCharFriendsGuids = sSocialMgr->GetCharFriendsGuids(GetPlayer(), activityID);
        result.NumGuildMateGuids = sSocialMgr->GetGuildMateGuids(activityID);
        result.VirtualRealmAddress = GetVirtualRealmAddress();
        result.CompletedEncounters = 0;
        result.Age = lfgEntry->CreationTime;
        result.ResultID = 3;
        result.ApplicationStatus = AsUnderlyingType(LFGListApplicationStatus::None);

        for (auto const& member : group->GetMemberSlots())
        {
            uint8 role = member.Roles >= 2 ? std::log2(member.Roles) - 1 : member.Roles;
            result.Members.emplace_back(member.Class, role);
        }
        //for (auto const& member : lfgEntry->ApplicationsContainer)
        //    if (auto applicant = member.second.GetPlayer())
        //        result.Members.emplace_back(applicant->getClass(), member.second.RoleMask);

        result.JoinRequest.ActivityID = activityID;
        result.JoinRequest.ItemLevel = lfgEntry->ItemLevel;
        result.JoinRequest.HonorLevel = lfgEntry->HonorLevel;
        result.JoinRequest.GroupName = lfgEntry->GroupName;
        result.JoinRequest.Comment = lfgEntry->Comment;
        result.JoinRequest.VoiceChat = lfgEntry->VoiceChat;
        result.JoinRequest.AutoAccept = lfgEntry->AutoAccept;
        result.JoinRequest.QuestID = lfgEntry->QuestID;

        results.SearchResults.emplace_back(result);
    }

    SendPacket(results.Write());
}

void WorldSession::HandleLfgListJoin(WorldPackets::LfgList::LfgListJoin& packet)
{
    auto list = new LFGListEntry;
    list->GroupFinderActivityData = sGroupFinderActivityStore.LookupEntry(packet.Request.ActivityID);
    list->ItemLevel = packet.Request.ItemLevel;
    list->AutoAccept = packet.Request.AutoAccept;
    list->GroupName = packet.Request.GroupName;
    list->Comment = packet.Request.Comment;
    list->VoiceChat = packet.Request.VoiceChat;
    list->HonorLevel = packet.Request.HonorLevel;
    if (packet.Request.QuestID.is_initialized())
        list->QuestID = *packet.Request.QuestID;
    list->ApplicationGroup = nullptr;
    list->PrivateGroup = packet.Request.PrivateGroup;
    sLFGListMgr->Insert(list, GetPlayer());
}

void WorldSession::HandleLfgListLeave(WorldPackets::LfgList::LfgListLeave& packet)
{
    auto entry = sLFGListMgr->GetEntrybyGuidLow(packet.ApplicationTicket.Id);
    if (!entry || !entry->ApplicationGroup->IsLeader(GetPlayer()->GetGUID()))
        return;

    sLFGListMgr->Remove(packet.ApplicationTicket.Id, GetPlayer());
}

void WorldSession::HandleLfgListInviteResponse(WorldPackets::LfgList::LfgListInviteResponse& packet)
{
    sLFGListMgr->ChangeApplicantStatus(sLFGListMgr->GetApplicationByID(packet.ApplicantTicket.Id), packet.Accept ? LFGListApplicationStatus::InviteAccepted : LFGListApplicationStatus::InviteDeclined);
}

void WorldSession::HandleLfgListGetStatus(WorldPackets::LfgList::LfgListGetStatus& /*packet*/)
{
}

void WorldSession::HandleLfgListApplyToGroup(WorldPackets::LfgList::LfgListApplyToGroup& packet)
{
    if (GetPlayer()->GetGroup()) // hack, i don't know, how do it, because we need rolechek and result of rolecheck input there
        ChatHandler(GetPlayer()).PSendSysMessage("You can't join it while you in group!");
    else
        sLFGListMgr->OnPlayerApplyForGroup(GetPlayer(), &packet.application.ApplicationTicket, packet.application.ActivityID, packet.application.Comment, packet.application.Role);
}

void WorldSession::HandleLfgListCancelApplication(WorldPackets::LfgList::LfgListCancelApplication& packet)
{
    if (auto entry = sLFGListMgr->GetEntryByApplicant(packet.ApplicantTicket))
        sLFGListMgr->ChangeApplicantStatus(entry->GetApplicant(packet.ApplicantTicket.Id), LFGListApplicationStatus::Cancelled);
}

void WorldSession::HandleLfgListDeclineApplicant(WorldPackets::LfgList::LfgListDeclineApplicant& packet)
{
    if (!_player->GetGroup()->IsAssistant(_player->GetGUID()) && !_player->GetGroup()->IsLeader(_player->GetGUID()))
        return;

    if (auto entry = sLFGListMgr->GetEntrybyGuidLow(packet.ApplicantTicket.Id))
        sLFGListMgr->ChangeApplicantStatus(entry->GetApplicant(packet.ApplicationTicket.Id), LFGListApplicationStatus::Declined);
}

void WorldSession::HandleLfgListInviteApplicant(WorldPackets::LfgList::LfgListInviteApplicant& packet)
{
    if (!_player->GetGroup()->IsAssistant(_player->GetGUID()) && !_player->GetGroup()->IsLeader(_player->GetGUID()))
        return;

    //packet.Applicant
    //packet.ApplicationTicket
    if (auto entry = sLFGListMgr->GetEntrybyGuidLow(packet.ApplicantTicket.Id))
    {
        auto applicant = entry->GetApplicant(packet.ApplicationTicket.Id);
        applicant->RoleMask = (*packet.Applicant.begin()).Role;

        sLFGListMgr->ChangeApplicantStatus(applicant, LFGListApplicationStatus::Invited);
    }
}

void WorldSession::HandleLfgListUpdateRequest(WorldPackets::LfgList::LfgListUpdateRequest& packet)
{
    auto entry = sLFGListMgr->GetEntrybyGuidLow(packet.Ticket.Id);
    if (!entry || !entry->ApplicationGroup->IsLeader(_player->GetGUID()))
        return;

    entry->AutoAccept = packet.UpdateRequest.AutoAccept;
    entry->GroupName = packet.UpdateRequest.GroupName;
    entry->Comment = packet.UpdateRequest.Comment;
    entry->VoiceChat = packet.UpdateRequest.VoiceChat;
    entry->HonorLevel = packet.UpdateRequest.HonorLevel;
    if (packet.UpdateRequest.QuestID.is_initialized())
        entry->QuestID = *packet.UpdateRequest.QuestID;

    if (packet.UpdateRequest.ItemLevel < sLFGListMgr->GetPlayerItemLevelForActivity(entry->GroupFinderActivityData, _player))
        entry->ItemLevel = packet.UpdateRequest.ItemLevel;
    entry->PrivateGroup = packet.UpdateRequest.PrivateGroup;

    sLFGListMgr->AutoInviteApplicantsIfPossible(entry);
    sLFGListMgr->SendLFGListStatusUpdate(entry);
}
