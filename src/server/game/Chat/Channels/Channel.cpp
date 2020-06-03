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
 
#include <cwctype>

#include "AccountMgr.h"
#include "Channel.h"
#include "ChannelPackets.h"
#include "ChannelAppenders.h"
#include "Chat.h"
#include "ChatPackets.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "SocialMgr.h"
#include "WordFilterMgr.h"
#include "World.h"
#include "GridNotifiersImpl.h"

Channel::Channel(std::string const& name, uint32 channel_id, uint32 Team) : _channelFlags(0), _channelId(channel_id), _channelName(name), _channelPassword(""), _announceEnabled(false), _special(false), _ownershipEnabled(true), m_Team(Team)
{
    _isSaved = false;

    if (IsWorld())
        _announceEnabled = false;

    // set special flags if built-in channel
    if (ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(channel_id)) // check whether it's a built-in channel
    {
        _announceEnabled = false;                                 // no join/leave announces
        _ownershipEnabled = false;                                // no ownership handout

        _channelFlags |= CHANNEL_FLAG_GENERAL;                    // for all built-in channels

        if (ch->Flags & CHANNEL_DBC_FLAG_TRADE)
            _channelFlags |= CHANNEL_FLAG_TRADE;

        if (ch->Flags & CHANNEL_DBC_FLAG_CITY_ONLY2)        // for city only channels
            _channelFlags |= CHANNEL_FLAG_CITY;

        if (ch->Flags & CHANNEL_DBC_FLAG_LFG)               // for LFG channel
            _channelFlags |= CHANNEL_FLAG_LFG;
        else                                                // for all other channels
            _channelFlags |= CHANNEL_FLAG_NOT_LFG | CHANNEL_FLAG_UNK2;
    }
    else if (!stricmp(_channelName.c_str(), "world") || !stricmp(_channelName.c_str(), "all"))
    {
        _announceEnabled = false;
        _special = true;
    }
    else                                                    // it's custom channel
    {
        _channelFlags |= CHANNEL_FLAG_CUSTOM;

        // If storing custom channels in the db is enabled either load or save the channel
        if (sWorld->getBoolConfig(CONFIG_PRESERVE_CUSTOM_CHANNELS))
        {
            PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHANNEL);
            stmt->setString(0, name);
            stmt->setUInt32(1, m_Team);
            PreparedQueryResult result = CharacterDatabase.Query(stmt);

            if (result) //load
            {
                Field* fields = result->Fetch();
                _announceEnabled = fields[0].GetBool();
                _ownershipEnabled = fields[1].GetBool();
                _channelPassword  = fields[2].GetString();
                const char* db_BannedList = fields[3].GetCString();

                if (db_BannedList)
                {
                    Tokenizer tokens(db_BannedList, ' ');
                    for (auto token : tokens)
                    {
                        std::string bannedGuidStr(token);
                        ObjectGuid banned_guid;
                        banned_guid.SetRawValue(uint64(strtoull(bannedGuidStr.substr(0, 16).c_str(), nullptr, 16)), uint64(strtoull(bannedGuidStr.substr(16).c_str(), nullptr, 16)));

                        if (!banned_guid.IsEmpty())
                        {
                            TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "Channel(%s) loaded _bannedStore guid: %s", name.c_str(), banned_guid.ToString().c_str());
                            _bannedStore.insert(banned_guid);
                        }
                    }
                }
            }
            else // save
            {
                stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHANNEL);
                stmt->setString(0, name);
                stmt->setUInt32(1, m_Team);
                CharacterDatabase.Execute(stmt);
                TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "Channel(%s) saved in database", name.c_str());
            }

            _isSaved = true;
        }
    }
}

bool Channel::PlayerInfo::HasFlag(uint8 flag) const
{
    return (flags & flag) != 0;
}

void Channel::PlayerInfo::SetFlag(uint8 flag)
{
    flags |= flag;
}

bool Channel::PlayerInfo::IsOwner() const
{
    return (flags & MEMBER_FLAG_OWNER) != 0;
}

void Channel::PlayerInfo::SetOwner(bool state)
{
    if (state)
        flags |= MEMBER_FLAG_OWNER;
    else
        flags &= ~MEMBER_FLAG_OWNER;
}

bool Channel::PlayerInfo::IsModerator() const
{
    return (flags & MEMBER_FLAG_MODERATOR) != 0;
}

void Channel::PlayerInfo::SetModerator(bool state)
{
    if (state)
        flags |= MEMBER_FLAG_MODERATOR;
    else
        flags &= ~MEMBER_FLAG_MODERATOR;
}

bool Channel::PlayerInfo::IsMuted() const
{
    return (flags & MEMBER_FLAG_MUTED) != 0;
}

void Channel::PlayerInfo::SetMuted(bool state)
{
    if (state)
        flags |= MEMBER_FLAG_MUTED;
    else
        flags &= ~MEMBER_FLAG_MUTED;
}

bool Channel::IsOn(ObjectGuid who) const
{
    return _playersStore.find(who) != _playersStore.end();
}

bool Channel::IsBanned(ObjectGuid guid) const
{
    return _bannedStore.find(guid) != _bannedStore.end();
}

bool Channel::IsWorld() const
{
    std::string lowername;
    uint32 nameLength = _channelName.length();
    for (uint32 i = 0; i < nameLength; ++i)
        lowername.push_back(std::towlower(_channelName[i]));

    return lowername == "world" || lowername == "all";
}

void Channel::UpdateChannelInDB() const
{
    if (_isSaved)
    {
        std::ostringstream banlist;
        for (auto iter : _bannedStore)
            banlist << iter << ' ';

        std::string banListStr = banlist.str();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHANNEL);
        stmt->setBool(0, _announceEnabled);
        stmt->setBool(1, _ownershipEnabled);
        stmt->setString(2, _channelPassword);
        stmt->setString(3, banListStr);
        stmt->setString(4, _channelName);
        stmt->setUInt32(5, m_Team);
        CharacterDatabase.Execute(stmt);

        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "Channel(%s) updated in database", _channelName.c_str());
    }
}

void Channel::UpdateChannelUseageInDB() const
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHANNEL_USAGE);
    stmt->setString(0, _channelName);
    stmt->setUInt32(1, m_Team);
    CharacterDatabase.Execute(stmt);
}

uint8 Channel::GetPlayerFlags(ObjectGuid p) const
{
    auto p_itr = _playersStore.find(p);
    if (p_itr == _playersStore.end())
        return 0;

    return p_itr->second.flags;
}

void Channel::CleanOldChannelsInDB()
{
    if (sWorld->getIntConfig(CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION) > 0)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_CHANNELS);
        stmt->setUInt32(0, sWorld->getIntConfig(CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION) * DAY);
        CharacterDatabase.Execute(stmt);

        TC_LOG_DEBUG(LOG_FILTER_CHATSYS, "Cleaned out unused custom chat channels.");
    }
}

void Channel::JoinChannel(Player* player, std::string const& pass, bool /*clientRequest*/)
{
    ObjectGuid const& guid = player->GetGUID();
    if (IsOn(guid))
    {
        if (!IsConstant())
        {
            PlayerAlreadyMemberAppend appender(guid);
            ChannelNameBuilder<PlayerAlreadyMemberAppend> builder(this, appender);
            SendToOne(builder, guid);
        }
        return;
    }

    if (IsBanned(guid))
    {
        BannedAppend appender;
        ChannelNameBuilder<BannedAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (!_channelPassword.empty() && pass != _channelPassword)
    {
        WrongPasswordAppend appender;
        ChannelNameBuilder<WrongPasswordAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (HasFlag(CHANNEL_FLAG_LFG) && sWorld->getBoolConfig(CONFIG_RESTRICTED_LFG_CHANNEL) && AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) && player->GetGroup())
    {
        NotInLFGAppend appender;
        ChannelNameBuilder<NotInLFGAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    player->JoinedChannel(this);

    if (_announceEnabled && (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        JoinedAppend appender(guid);
        ChannelNameBuilder<JoinedAppend> builder(this, appender);
        SendToAll(builder);
    }

    PlayerInfo pinfo;
    pinfo.player = guid;
    pinfo.flags = MEMBER_FLAG_NONE;
    _playersStore[guid] = pinfo;
    PlayerInfo& playerInfo = _playersStore[guid];

    auto builder = [&](LocaleConstant /*locale*/)
    {
        auto notify = new WorldPackets::Channel::ChannelNotifyJoined();
        notify->ChannelWelcomeMsg = "";
        notify->ChatChannelID = _channelId;
        notify->InstanceID = 0;
        notify->_ChannelFlags = _channelFlags;
        notify->_Channel = _channelName;
        return notify;
    };

    SendToOne(builder, guid);

    
    if (!IsConstant()) // Custom channel handling
    {
        if (!_playersStore.empty())
            UpdateChannelUseageInDB();

        if (!_ownerGuid && _ownershipEnabled) // If the channel has no owner yet and ownership is allowed, set the new owner.
        {
            SetOwner(guid, (_playersStore.size() > 1));
            playerInfo.SetModerator(true);
        }
    }
}

void Channel::LeaveChannel(Player* player, bool send, bool clientRequest)
{
    ObjectGuid const& guid = player->GetGUID();
    if (!IsOn(guid))
    {
        if (send)
        {
            NotMemberAppend appender;
            ChannelNameBuilder<NotMemberAppend> builder(this, appender);
            SendToOne(builder, guid);
        }
        return;
    }

    player->LeftChannel(this);

    if (send)
    {
        auto builder = [&](LocaleConstant locale)
        {
            auto* notify = new WorldPackets::Channel::ChannelNotifyLeft();
            notify->Channel = GetName();
            notify->ChatChannelID = 0;
            notify->Suspended = _channelId == 2 && !clientRequest;
            return notify;
        };

        SendToOne(builder, guid);
    }

    PlayerInfo& info = _playersStore[guid];
    bool changeowner = info.IsOwner();
    _playersStore.erase(guid);

    if (_announceEnabled && (!player || !AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        LeftAppend appender(guid);
        ChannelNameBuilder<LeftAppend> builder(this, appender);
        SendToAll(builder);
    }

    LeaveNotify(guid);

    if (!IsConstant())
    {
        UpdateChannelUseageInDB();

        // If the channel owner left and there are still playersStore inside, pick a new owner do not pick invisible gm owner unless there are only invisible gms in that channel (rare)
        if (changeowner && _ownershipEnabled && !_playersStore.empty())
        {
            ObjectGuid newowner = _playersStore.begin()->second.player;
            _playersStore[newowner].SetModerator(true);
            SetOwner(newowner);
        }
    }
}

void Channel::KickOrBan(Player const* player, std::string const& badname, bool ban)
{
    ObjectGuid const& good = player->GetGUID();

    if (!IsOn(good))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    AccountTypes sec = player->GetSession()->GetSecurity();

    PlayerInfo& info = _playersStore[good];
    if (!info.IsModerator() && !AccountMgr::IsModeratorAccount(sec))
    {
        NotModeratorAppend appender;
        ChannelNameBuilder<NotModeratorAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    Player* bad = ObjectAccessor::FindPlayerByName(badname);
    ObjectGuid const& victim = bad ? bad->GetGUID() : ObjectGuid::Empty;
    if (!victim || !IsOn(victim))
    {
        PlayerNotFoundAppend appender(badname);
        ChannelNameBuilder<PlayerNotFoundAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    bool changeowner = _ownerGuid == victim;

    if (!AccountMgr::IsModeratorAccount(sec) && changeowner && good != _ownerGuid)
    {
        NotOwnerAppend appender;
        ChannelNameBuilder<NotOwnerAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    if (ban && !IsBanned(victim))
    {
        _bannedStore.insert(victim);
        UpdateChannelInDB();

        if (!(AccountMgr::IsModeratorAccount(sec) && sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
        {
            PlayerBannedAppend appender(good, victim);
            ChannelNameBuilder<PlayerBannedAppend> builder(this, appender);
            SendToAll(builder);
        }
    }
    else if (!(AccountMgr::IsModeratorAccount(sec) && sWorld->getBoolConfig(CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL)))
    {
        PlayerKickedAppend appender(good, victim);
        ChannelNameBuilder<PlayerKickedAppend> builder(this, appender);
        SendToAll(builder);
    }

    _playersStore.erase(victim);
    bad->LeftChannel(this);

    if (changeowner && _ownershipEnabled && !_playersStore.empty())
    {
        info.SetModerator(true);
        SetOwner(good);
    }
}

void Channel::UnBan(Player const* player, std::string const& badname)
{
    ObjectGuid const& good = player->GetGUID();

    if (!IsOn(good))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    PlayerInfo& info = _playersStore[good];
    if (!info.IsModerator() && !AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()))
    {
        NotModeratorAppend appender;
        ChannelNameBuilder<NotModeratorAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    Player* bad = ObjectAccessor::FindPlayerByName(badname);
    ObjectGuid victim = bad ? bad->GetGUID() : ObjectGuid::Empty;

    if (victim.IsEmpty() || !IsBanned(victim))
    {
        PlayerNotFoundAppend appender(badname);
        ChannelNameBuilder<PlayerNotFoundAppend> builder(this, appender);
        SendToOne(builder, good);
        return;
    }

    _bannedStore.erase(victim);

    PlayerUnbannedAppend appender(good, victim);
    ChannelNameBuilder<PlayerUnbannedAppend> builder(this, appender);
    SendToAll(builder);

    UpdateChannelInDB();
}

void Channel::Password(Player const* player, std::string const& pass)
{
    ObjectGuid const& guid = player->GetGUID();
    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo& info = _playersStore[guid];
    if (!info.IsModerator() && !AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()))
    {
        NotModeratorAppend appender;
        ChannelNameBuilder<NotModeratorAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    _channelPassword = pass;

    PasswordChangedAppend appender(guid);
    ChannelNameBuilder<PasswordChangedAppend> builder(this, appender);
    SendToAll(builder);

    UpdateChannelInDB();
}

void Channel::SetMode(Player const* player, std::string const& p2n, bool mod, bool set)
{
    ObjectGuid const& guid = player->GetGUID();
    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo& info = _playersStore[guid];
    if (!info.IsModerator() && !AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()))
    {
        NotModeratorAppend appender;
        ChannelNameBuilder<NotModeratorAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (guid == _ownerGuid && p2n == player->GetName() && mod)
        return;

    Player* newp = ObjectAccessor::FindPlayerByName(p2n);
    ObjectGuid victim = newp ? newp->GetGUID() : ObjectGuid::Empty;

    // allow make moderator from another team only if both is GMs at this moment this only way to show channel post for GM from another team
    if (newp && (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) || !AccountMgr::IsModeratorAccount(newp->GetSession()->GetSecurity())) &&
        player->GetTeam() != newp->GetTeam() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        PlayerNotFoundAppend appender(p2n);
        ChannelNameBuilder<PlayerNotFoundAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (_ownerGuid == victim && _ownerGuid != guid)
    {
        NotOwnerAppend appender;
        ChannelNameBuilder<NotOwnerAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (newp)
    {
        if (mod)
            SetModerator(newp->GetGUID(), set);
        else
            SetMute(newp->GetGUID(), set);
    }
}

void Channel::_SetOwner(Player const* player, std::string const& newname)
{
    ObjectGuid const& guid = player->GetGUID();

    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (!AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()) && guid != _ownerGuid)
    {
        NotOwnerAppend appender;
        ChannelNameBuilder<NotOwnerAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    Player* newp = ObjectAccessor::FindPlayerByName(newname);
    ObjectGuid victim = newp ? newp->GetGUID() : ObjectGuid::Empty;

    if (newp && newp->GetTeam() != player->GetTeam() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        PlayerNotFoundAppend appender(newname);
        ChannelNameBuilder<PlayerNotFoundAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo& info = _playersStore[victim];
    info.SetModerator(true);
    SetOwner(victim);
}

void Channel::SendWhoOwner(Player const* player)
{
    ObjectGuid const& guid = player->GetGUID();
    if (IsOn(guid))
    {
        ChannelOwnerAppend appender(this, _ownerGuid);
        ChannelNameBuilder<ChannelOwnerAppend> builder(this, appender);
        SendToOne(builder, guid);
    }
    else
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
    }
}

void Channel::List(Player const* player)
{
    ObjectGuid const& guid = player->GetGUID();
    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    WorldPackets::Channel::ChannelListResponse list;
    list._Display = true; /// always true?
    list._Channel = GetName();
    list._ChannelFlags = GetFlags();

    uint32 gmLevelInWhoList = sWorld->getIntConfig(CONFIG_GM_LEVEL_IN_WHO_LIST);

    list._Members.reserve(_playersStore.size());
    for (auto const& i : _playersStore)
    {
        Player* member = ObjectAccessor::FindPlayer(i.first);

        // PLAYER can't see MODERATOR, GAME MASTER, ADMINISTRATOR characters: MODERATOR, GAME MASTER, ADMINISTRATOR can see all
        if (member && (!AccountMgr::IsPlayerAccount(player->GetSession()->GetSecurity()) || member->GetSession()->GetSecurity() <= AccountTypes(gmLevelInWhoList)) && member->IsVisibleGloballyFor(player))
            list._Members.emplace_back(i.second.player, GetVirtualRealmAddress(), i.second.flags);
    }

    player->SendDirectMessage(list.Write());
}

void Channel::Announce(Player const* player)
{
    ObjectGuid const& guid = player->GetGUID();

    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo const& playerInfo = _playersStore[guid];
    if (!playerInfo.IsModerator() && !AccountMgr::IsModeratorAccount(player->GetSession()->GetSecurity()))
    {
        NotModeratorAppend appender;
        ChannelNameBuilder<NotModeratorAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    _announceEnabled = !_announceEnabled;

    WorldPackets::Channel::ChannelNotify notify;
    if (_announceEnabled)
    {
        AnnouncementsOnAppend appender(guid);
        ChannelNameBuilder<AnnouncementsOnAppend> builder(this, appender);
        SendToAll(builder);
    }
    else
    {
        AnnouncementsOffAppend appender(guid);
        ChannelNameBuilder<AnnouncementsOffAppend> builder(this, appender);
        SendToAll(builder);
    }

    UpdateChannelInDB();
}

void Channel::Say(ObjectGuid const& guid, std::string const& what, uint32 lang, bool isSpamm)
{
    if (what.empty())
        return;

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (player)
        lang = player->GetTeam() == HORDE ? LANG_ORCISH : LANG_COMMON;

    if (sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
        lang = LANG_UNIVERSAL;

    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo const& playerInfo = _playersStore[guid];
    if (playerInfo.IsMuted())
    {
        MutedAppend appender;
        ChannelNameBuilder<MutedAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    auto builder = [&](LocaleConstant locale)
    {
        auto packet = new WorldPackets::Chat::Chat();
        if (player)
            packet->Initialize(CHAT_MSG_CHANNEL, Language(lang), player, player, what, 0, GetName());
        else
        {
            packet->Initialize(CHAT_MSG_CHANNEL, Language(lang), nullptr, nullptr, what, 0, GetName());
            packet->SenderGUID = guid;
            packet->TargetGUID = guid;
        }

        return packet;
    };

    if (isSpamm)
        SendToOne(builder, guid);
    else
        SendToAll(builder, !playerInfo.IsModerator() ? guid : ObjectGuid::Empty);
}

void Channel::Invite(Player const* player, std::string const& newname)
{
    if (sWorld->getBoolConfig(CONFIG_WORD_FILTER_ENABLE))
        if (!sWordFilterMgr->FindBadWord(newname, true).empty())
            return;

    ObjectGuid const& guid = player->GetGUID();
    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    Player* newp = ObjectAccessor::FindPlayerByName(newname);
    if (!newp || !newp->isGMVisible())
    {
        PlayerNotFoundAppend appender(newname);
        ChannelNameBuilder<PlayerNotFoundAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (IsBanned(newp->GetGUID()))
    {
        PlayerInviteBannedAppend appender(newname);
        ChannelNameBuilder<PlayerInviteBannedAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (newp->GetTeam() != player->GetTeam() && !sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL))
    {
        InviteWrongFactionAppend appender;
        ChannelNameBuilder<InviteWrongFactionAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (IsOn(newp->GetGUID()))
    {
        PlayerAlreadyMemberAppend appender(newp->GetGUID());
        ChannelNameBuilder<PlayerAlreadyMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    if (!newp->GetSocial()->HasIgnore(guid))
    {
        InviteAppend appender(guid);
        ChannelNameBuilder<InviteAppend> builder(this, appender);
        SendToOne(builder, newp->GetGUID());
    }

    PlayerInvitedAppend appender(newp->GetName());
    ChannelNameBuilder<PlayerInvitedAppend> builder(this, appender);
    SendToOne(builder, guid);
}

void Channel::SetOwner(ObjectGuid const& guid, bool exclaim)
{
    if (!_ownerGuid.IsEmpty())
    {
        auto itr = _playersStore.find(_ownerGuid);
        if (itr != _playersStore.end())
            itr->second.SetOwner(false);
    }

    _ownerGuid = guid;
    if (!_ownerGuid.IsEmpty())
    {
        uint8 oldFlag = GetPlayerFlags(_ownerGuid);
        auto itr = _playersStore.find(_ownerGuid);
        if (itr == _playersStore.end())
            return;

        itr->second.SetModerator(true);
        itr->second.SetOwner(true);

        ModeChangeAppend appender(_ownerGuid, oldFlag, GetPlayerFlags(_ownerGuid));
        ChannelNameBuilder<ModeChangeAppend> builder(this, appender);
        SendToAll(builder);

        if (exclaim)
        {
            OwnerChangedAppend ownerChangedAppender(_ownerGuid);
            ChannelNameBuilder<OwnerChangedAppend> ownerChangedBuilder(this, ownerChangedAppender);
            SendToAll(ownerChangedBuilder);
        }

        UpdateChannelInDB();
    }
}

void Channel::DeclineInvite(Player const* /*player*/)
{
}

void Channel::SilenceAll(Player const* /*player*/, std::string const& /*name*/)
{
}

void Channel::UnsilenceAll(Player const* /*player*/, std::string const& /*name*/)
{
}

void Channel::JoinNotify(ObjectGuid const& guid)
{
    if (IsConstant())
    {
        auto builder = [&](LocaleConstant locale)
        {
            auto userlistAdd = new WorldPackets::Channel::UserlistAdd();
            userlistAdd->AddedUserGUID = guid;
            userlistAdd->_ChannelFlags = GetFlags();
            userlistAdd->UserFlags = GetPlayerFlags(guid);
            userlistAdd->ChannelID = GetChannelId();
            userlistAdd->ChannelName = GetName();
            return userlistAdd;
        };

        SendToAllButOne(builder, guid);
    }
    else
    {
        auto builder = [&](LocaleConstant locale)
        {
            auto* userlistUpdate = new WorldPackets::Channel::UserlistUpdate();
            userlistUpdate->UpdatedUserGUID = guid;
            userlistUpdate->_ChannelFlags = GetFlags();
            userlistUpdate->UserFlags = GetPlayerFlags(guid);
            userlistUpdate->ChannelID = GetChannelId();
            userlistUpdate->ChannelName = GetName();
            return userlistUpdate;
        };

        SendToAll(builder);
    }
}

void Channel::LeaveNotify(ObjectGuid const& guid)
{
    auto builder = [&](LocaleConstant locale)
    {
        auto userlistRemove = new WorldPackets::Channel::UserlistRemove();
        userlistRemove->RemovedUserGUID = guid;
        userlistRemove->_ChannelFlags = GetFlags();
        userlistRemove->ChannelID = GetChannelId();
        userlistRemove->ChannelName = GetName();
        return userlistRemove;
    };

    if (IsConstant())
        SendToAllButOne(builder, guid);
    else
        SendToAll(builder);
}

void Channel::SetModerator(ObjectGuid const& guid, bool set)
{
    if (!IsOn(guid))
        return;

    PlayerInfo& playerInfo = _playersStore[guid];
    if (playerInfo.IsModerator() != set)
    {
        uint8 oldFlag = GetPlayerFlags(guid);
        playerInfo.SetModerator(set);

        ModeChangeAppend appender(guid, oldFlag, GetPlayerFlags(guid));
        ChannelNameBuilder<ModeChangeAppend> builder(this, appender);
        SendToAll(builder);
    }
}

void Channel::SetMute(ObjectGuid const& guid, bool set)
{
    if (!IsOn(guid))
        return;

    PlayerInfo& playerInfo = _playersStore[guid];
    if (playerInfo.IsMuted() != set)
    {
        uint8 oldFlag = GetPlayerFlags(guid);
        playerInfo.SetMuted(set);

        ModeChangeAppend appender(guid, oldFlag, GetPlayerFlags(guid));
        ChannelNameBuilder<ModeChangeAppend> builder(this, appender);
        SendToAll(builder);
    }
}

void Channel::AddonSay(ObjectGuid const& guid, std::string const& prefix, std::string const& what)
{
    if (what.empty())
        return;

    if (!IsOn(guid))
    {
        NotMemberAppend appender;
        ChannelNameBuilder<NotMemberAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    PlayerInfo const& playerInfo = _playersStore[guid];
    if (playerInfo.IsMuted())
    {
        MutedAppend appender;
        ChannelNameBuilder<MutedAppend> builder(this, appender);
        SendToOne(builder, guid);
        return;
    }

    auto builder = [&](LocaleConstant locale)
    {
        auto packet = new WorldPackets::Chat::Chat();
        if (auto player = ObjectAccessor::FindPlayer(guid))
            packet->Initialize(CHAT_MSG_CHANNEL, LANG_ADDON, player, player, what, 0, GetName(), DEFAULT_LOCALE, prefix);
        else
        {
            packet->Initialize(CHAT_MSG_CHANNEL, LANG_ADDON, nullptr, nullptr, what, 0, GetName(), DEFAULT_LOCALE, prefix);
            packet->SenderGUID = guid;
            packet->TargetGUID = guid;
        }

        return packet;
    };

    SendToAllWithAddon(builder, prefix, !_playersStore[guid].IsModerator() ? guid : ObjectGuid::Empty);
}

template <class Builder>
void Channel::SendToAll(Builder& builder, ObjectGuid const& guid) const
{
    Trinity::LocalizedPacketDo<Builder> localizer(builder);

    for (auto const& i : _playersStore)
        if (auto player = ObjectAccessor::FindPlayer(i.first))
            if (guid.IsEmpty() || !player->GetSocial()->HasIgnore(guid))
                localizer(player);
}

template <class Builder>
void Channel::SendToAllButOne(Builder& builder, ObjectGuid const& who) const
{
    Trinity::LocalizedPacketDo<Builder> localizer(builder);

    for (auto const& i : _playersStore)
        if (i.first != who)
            if (auto player = ObjectAccessor::FindPlayer(i.first))
                localizer(player);
}

template <class Builder>
void Channel::SendToOne(Builder& builder, ObjectGuid const& who) const
{
    Trinity::LocalizedPacketDo<Builder> localizer(builder);

    if (auto player = ObjectAccessor::FindPlayer(who))
        localizer(player);
}

template <class Builder>
void Channel::SendToAllWithAddon(Builder& builder, std::string const& addonPrefix, ObjectGuid const& guid /*= ObjectGuid::Empty*/) const
{
    Trinity::LocalizedPacketDo<Builder> localizer(builder);

    for (auto const& i : _playersStore)
        if (auto player = ObjectAccessor::FindPlayer(i.first))
            if (player->GetSession()->IsAddonRegistered(addonPrefix) && (guid.IsEmpty() || !player->GetSocial()->HasIgnore(guid)))
                localizer(player);
}
