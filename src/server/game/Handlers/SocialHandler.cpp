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

#include "Common.h"
#include "WorldSession.h"
#include "SocialMgr.h"
#include "Opcodes.h"
#include "SocialPackets.h"
#include "Player.h"
#include "AccountMgr.h"
#include "ObjectMgr.h"
#include "GlobalFunctional.h"

void WorldSession::HandleContactListOpcode(WorldPackets::Social::SendContactList& packet)
{
    GetPlayer()->GetSocial()->SendSocialList(_player, packet.Flags);
}

void WorldSession::HandleAddFriend(WorldPackets::Social::AddFriend& packet)
{
    if (!normalizePlayerName(packet.Name))
        return;

    Player* player = GetPlayer();
    if (!player)
        return;

    ObjectGuid friendGuid;

    FriendsResult friendResult = FRIEND_NOT_FOUND;

    if (const CharacterInfo* nameData = sWorld->GetCharacterInfo(packet.Name))
    {
        friendGuid = ObjectGuid::Create<HighGuid::Player>(nameData->Guid);
        uint32 team = Player::TeamForRace(nameData->Race);
        uint32 friendAccountId = nameData->AccountId;

        if (!AccountMgr::IsPlayerAccount(GetSecurity()) || sWorld->getBoolConfig(CONFIG_ALLOW_GM_FRIEND) || AccountMgr::IsPlayerAccount(AccountMgr::GetSecurity(friendAccountId, realm.Id.Realm)))
        {
            if (!friendGuid.IsEmpty())
            {
                if (friendGuid == player->GetGUID())
                    friendResult = FRIEND_SELF;
                else if (player->GetTeam() != team && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND) && AccountMgr::IsPlayerAccount(GetSecurity()))
                    friendResult = FRIEND_ENEMY;
                else if (player->GetSocial()->HasFriend(friendGuid))
                    friendResult = FRIEND_ALREADY;
                else
                {
                    Player* pFriend = ObjectAccessor::FindPlayer(friendGuid);
                    if (pFriend && pFriend->IsInWorld() && pFriend->IsVisibleGloballyFor(player))
                        friendResult = FRIEND_ADDED_ONLINE;
                    else
                        friendResult = FRIEND_ADDED_OFFLINE;
                    if (!player->GetSocial()->AddToSocialList(friendGuid, SOCIAL_FLAG_FRIEND))
                    {
                        friendResult = FRIEND_LIST_FULL;
                        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WORLD: %s's friend list is full.", player->GetName());
                    }
                }
                player->GetSocial()->SetFriendNote(friendGuid, packet.Notes);
            }
        }
    }

    sSocialMgr->SendFriendStatus(player, friendResult, friendGuid, false);
}

void WorldSession::HandleDelFriendOpcode(WorldPackets::Social::DelFriend& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    player->GetSocial()->RemoveFromSocialList(packet.Player.Guid, SOCIAL_FLAG_FRIEND);
    sSocialMgr->SendFriendStatus(player, FRIEND_REMOVED, packet.Player.Guid, false);
}

void WorldSession::HandleAddIgnore(WorldPackets::Social::AddIgnore& packet)
{
    if (!normalizePlayerName(packet.Name))
        return;

    Player* player = GetPlayer();
    if (!player)
        return;

    ObjectGuid IgnoreGuid;

    FriendsResult ignoreResult = FRIEND_IGNORE_NOT_FOUND;

    if (auto nameData = sWorld->GetCharacterInfo(packet.Name))
    {
        IgnoreGuid = ObjectGuid::Create<HighGuid::Player>(nameData->Guid);

        if (IgnoreGuid)
        {
            if (IgnoreGuid == player->GetGUID())
                ignoreResult = FRIEND_IGNORE_SELF;
            else if (player->GetSocial()->HasIgnore(IgnoreGuid))
                ignoreResult = FRIEND_IGNORE_ALREADY_S;
            else
            {
                ignoreResult = FRIEND_IGNORE_ADDED_S;

                if (!player->GetSocial()->AddToSocialList(IgnoreGuid, SOCIAL_FLAG_IGNORED))
                    ignoreResult = FRIEND_IGNORE_FULL;
            }
        }
    }

    sSocialMgr->SendFriendStatus(player, ignoreResult, IgnoreGuid, false);
}

void WorldSession::HandleDelIgnoreOpcode(WorldPackets::Social::DelIgnore& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    player->GetSocial()->RemoveFromSocialList(packet.Player.Guid, SOCIAL_FLAG_IGNORED);
    sSocialMgr->SendFriendStatus(player, FRIEND_IGNORE_REMOVED, packet.Player.Guid, false);
}

void WorldSession::HandleSetContactNotesOpcode(WorldPackets::Social::SetContactNotes& packet)
{
    _player->GetSocial()->SetFriendNote(packet.Player.Guid, packet.Notes);
}

void WorldSession::HandleQuickJoinAutoAcceptRequests(WorldPackets::Social::QuickJoinAutoAcceptRequests& packet)
{ }

void WorldSession::HandleQuickJoinRequestInvite(WorldPackets::Social::QuickJoinRequestInvite& packet)
{ }

void WorldSession::HandleQuickJoinRespondToInvite(WorldPackets::Social::QuickJoinRespondToInvite& packet)
{ }

void WorldSession::HandleQuickJoinSignalToastDisplayed(WorldPackets::Social::QuickJoinSignalToastDisplayed& packet)
{ }

