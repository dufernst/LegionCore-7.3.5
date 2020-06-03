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

#include "ChannelPackets.h"
#include "ChannelMgr.h"
#include "Chat.h"
#include "GlobalFunctional.h"

void WorldSession::HandleJoinChannel(WorldPackets::Channel::JoinChannel& packet)
{
    Player* player = GetPlayer();
    if (!player)
        return;

    if (packet.ChatChannelId)
    {
        ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(packet.ChatChannelId);
        if (!channel)
            return;

        AreaTableEntry const* zone = sAreaTableStore.LookupEntry(player->GetCurrentZoneID());
        if (!zone || !player->CanJoinConstantChannelInZone(channel, zone))
            return;
    }

    if (packet.ChannelName.empty())
        return;

    if (isdigit(packet.ChannelName[0]))
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        cMgr->team = player->GetTeam();
        if (Channel* channel = cMgr->GetJoinChannel(packet.ChannelName, packet.ChatChannelId))
            channel->JoinChannel(player, packet.Password, true);
    }
}

void WorldSession::HandleLeaveChannel(WorldPackets::Channel::LeaveChannel& packet)
{
    Player* player = GetPlayer();
    if (packet.ChannelName.empty() || !player)
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
    {
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->LeaveChannel(player, true, true);

        cMgr->LeftChannel(packet.ChannelName);
    }
}

void WorldSession::HandleChannelCommandAnnounce(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->Announce(player);
}

void WorldSession::HandleChannelCommandDeclineInvite(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->DeclineInvite(player);
}

void WorldSession::HandleChannelCommandList(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->List(player);
}

void WorldSession::HandleChannelCommandSendWhoOwner(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->SendWhoOwner(player);
}

void WorldSession::HandleChannelPlayerCommandBan(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->Ban(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandInvite(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->Invite(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandKick(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->Kick(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandSetModerator(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->_SetModerator(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandSetMute(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->_SetMute(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandSetOwner(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->_SetOwner(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandSilenceAll(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->SilenceAll(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandUnBan(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->UnBan(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandUnsetModerator(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->UnsetModerator(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandUnsetMute(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->UnsetMute(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandUnsilenceAll(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() >= MAX_CHANNEL_NAME_STR || !player)
        return;

    if (!normalizePlayerName(packet.Name))
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->UnsilenceAll(player, packet.Name);
}

void WorldSession::HandleChannelPlayerCommandPassword(WorldPackets::Channel::ChannelPlayerCommand& packet)
{
    Player* player = GetPlayer();
    if (packet.Name.length() > MAX_CHANNEL_PASS_STR || !player)
        return;

    if (packet.ChannelName.empty())
        return;

    if (!ChatHandler(this).isValidChatMessage(packet.ChannelName.c_str()))
        return;

    if (ChannelMgr* cMgr = channelMgr(player->GetTeam()))
        if (Channel* channel = cMgr->GetChannel(packet.ChannelName, player))
            channel->Password(player, packet.Name);
}
