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

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "Common.h"
#include "LockedMap.h"
#include "Player.h"

namespace WorldPackets
{
    namespace Channel
    {
        class ChannelNotify;
    }
}

enum ChatNotify
{
    CHAT_JOINED_NOTICE                = 0x00,           //+ "%s joined channel.";
    CHAT_LEFT_NOTICE                  = 0x01,           //+ "%s left channel.";
    //CHAT_SUSPENDED_NOTICE             = 0x01,           // "%s left channel.";
    CHAT_YOU_JOINED_NOTICE            = 0x02,           //+ "Joined Channel: [%s]"; -- You joined
    //CHAT_YOU_CHANGED_NOTICE           = 0x02,           // "Changed Channel: [%s]";
    CHAT_YOU_LEFT_NOTICE              = 0x03,           //+ "Left Channel: [%s]"; -- You left
    CHAT_WRONG_PASSWORD_NOTICE        = 0x04,           //+ "Wrong password for %s.";
    CHAT_NOT_MEMBER_NOTICE            = 0x05,           //+ "Not on channel %s.";
    CHAT_NOT_MODERATOR_NOTICE         = 0x06,           //+ "Not a moderator of %s.";
    CHAT_PASSWORD_CHANGED_NOTICE      = 0x07,           //+ "[%s] Password changed by %s.";
    CHAT_OWNER_CHANGED_NOTICE         = 0x08,           //+ "[%s] Owner changed to %s.";
    CHAT_PLAYER_NOT_FOUND_NOTICE      = 0x09,           //+ "[%s] Player %s was not found.";
    CHAT_NOT_OWNER_NOTICE             = 0x0A,           //+ "[%s] You are not the channel owner.";
    CHAT_CHANNEL_OWNER_NOTICE         = 0x0B,           //+ "[%s] Channel owner is %s.";
    CHAT_MODE_CHANGE_NOTICE           = 0x0C,           //?
    CHAT_ANNOUNCEMENTS_ON_NOTICE      = 0x0D,           //+ "[%s] Channel announcements enabled by %s.";
    CHAT_ANNOUNCEMENTS_OFF_NOTICE     = 0x0E,           //+ "[%s] Channel announcements disabled by %s.";
    CHAT_MODERATION_ON_NOTICE         = 0x0F,           //+ "[%s] Channel moderation enabled by %s.";
    CHAT_MODERATION_OFF_NOTICE        = 0x10,           //+ "[%s] Channel moderation disabled by %s.";
    CHAT_MUTED_NOTICE                 = 0x11,           //+ "[%s] You do not have permission to speak.";
    CHAT_PLAYER_KICKED_NOTICE         = 0x12,           //? "[%s] Player %s kicked by %s.";
    CHAT_BANNED_NOTICE                = 0x13,           //+ "[%s] You are banned from that channel.";
    CHAT_PLAYER_BANNED_NOTICE         = 0x14,           //? "[%s] Player %s banned by %s.";
    CHAT_PLAYER_UNBANNED_NOTICE       = 0x15,           //? "[%s] Player %s unbanned by %s.";
    CHAT_PLAYER_NOT_BANNED_NOTICE     = 0x16,           //+ "[%s] Player %s is not banned.";
    CHAT_PLAYER_ALREADY_MEMBER_NOTICE = 0x17,           //+ "[%s] Player %s is already on the channel.";
    CHAT_INVITE_NOTICE                = 0x18,           //+ "%2$s has invited you to join the channel '%1$s'.";
    CHAT_INVITE_WRONG_FACTION_NOTICE  = 0x19,           //+ "Target is in the wrong alliance for %s.";
    CHAT_WRONG_FACTION_NOTICE         = 0x1A,           //+ "Wrong alliance for %s.";
    CHAT_INVALID_NAME_NOTICE          = 0x1B,           //+ "Invalid channel name";
    CHAT_NOT_MODERATED_NOTICE         = 0x1C,           //+ "%s is not moderated";
    CHAT_PLAYER_INVITED_NOTICE        = 0x1D,           //+ "[%s] You invited %s to join the channel";
    CHAT_PLAYER_INVITE_BANNED_NOTICE  = 0x1E,           //+ "[%s] %s has been banned.";
    CHAT_THROTTLED_NOTICE             = 0x1F,           //+ "[%s] The number of messages that can be sent to this channel is limited, please wait to send another message.";
    CHAT_NOT_IN_AREA_NOTICE           = 0x20,           //+ "[%s] You are not in the correct area for this channel."; -- The user is trying to send a chat to a zone specific channel, and they're not physically in that zone.
    CHAT_NOT_IN_LFG_NOTICE            = 0x21,           //+ "[%s] You must be queued in looking for group before joining this channel."; -- The user must be in the looking for group system to join LFG chat channels.
    CHAT_VOICE_ON_NOTICE              = 0x22,           //+ "[%s] Channel voice enabled by %s.";
    CHAT_VOICE_OFF_NOTICE             = 0x23,           //+ "[%s] Channel voice disabled by %s.";
    CHAT_TRIAL_RESTRICTED             = 0x24,
    CHAT_NOT_ALLOWED_IN_CHANNEL       = 0x25
};

enum ChannelFlags
{
    CHANNEL_FLAG_NONE       = 0x000,
    CHANNEL_FLAG_CUSTOM     = 0x001,
    // 0x02
    CHANNEL_FLAG_TRADE      = 0x004,
    CHANNEL_FLAG_NOT_LFG    = 0x008,
    CHANNEL_FLAG_GENERAL    = 0x010,
    CHANNEL_FLAG_CITY       = 0x020,
    CHANNEL_FLAG_LFG        = 0x040,
    CHANNEL_FLAG_VOICE      = 0x080,
    CHANNEL_FLAG_UNK2       = 0x100,
    // General                  0x11C = 0x04 | 0x08 | 0x10 | 0x100
    // Trade                    0x13C = 0x04 | 0x08 | 0x10 | 0x20 | 0x100
    // LocalDefence             0x11C = 0x04 | 0x08 | 0x10 | 0x100
    // LookingForGroup          0x54  = 0x04 | 0x10 | 0x40
};

enum ChannelDBCFlags
{
    CHANNEL_DBC_FLAG_NONE       = 0x00000,
    CHANNEL_DBC_FLAG_INITIAL    = 0x00001,              // General, Trade, LocalDefense, LFG
    CHANNEL_DBC_FLAG_ZONE_DEP   = 0x00002,              // General, Trade, LocalDefense, GuildRecruitment
    CHANNEL_DBC_FLAG_GLOBAL     = 0x00004,              // WorldDefense
    CHANNEL_DBC_FLAG_TRADE      = 0x00008,              // Trade, LFG
    CHANNEL_DBC_FLAG_CITY_ONLY  = 0x00010,              // Trade, GuildRecruitment, LFG
    CHANNEL_DBC_FLAG_CITY_ONLY2 = 0x00020,              // Trade, GuildRecruitment, LFG
    CHANNEL_DBC_FLAG_DEFENSE    = 0x10000,              // LocalDefense, WorldDefense
    CHANNEL_DBC_FLAG_GUILD_REQ  = 0x20000,              // GuildRecruitment
    CHANNEL_DBC_FLAG_LFG        = 0x40000,              // LFG
    CHANNEL_DBC_FLAG_UNK1       = 0x80000,              // General
};

enum ChannelMemberFlags
{
    MEMBER_FLAG_NONE        = 0x00,
    MEMBER_FLAG_OWNER       = 0x01,
    MEMBER_FLAG_MODERATOR   = 0x02,
    MEMBER_FLAG_VOICED      = 0x04,
    MEMBER_FLAG_MUTED       = 0x08,
    MEMBER_FLAG_CUSTOM      = 0x10,
    MEMBER_FLAG_MIC_MUTED   = 0x20,
    // 0x40
    // 0x80
};

class Channel
{
    struct PlayerInfo
    {
        ObjectGuid player;
        uint8 flags;

        bool HasFlag(uint8 flag) const;
        void SetFlag(uint8 flag);
        bool IsOwner() const;
        void SetOwner(bool state);
        bool IsModerator() const;
        void SetModerator(bool state);
        bool IsMuted() const;
        void SetMuted(bool state);
    };

    typedef Trinity::LockedMap<ObjectGuid, PlayerInfo> PlayerList;
    PlayerList _playersStore;
    GuidSet _bannedStore;
    ObjectGuid _ownerGuid;
    uint32 _channelFlags;
    uint32 _channelId;
    std::string _channelName;
    std::string _channelPassword;
    bool _announceEnabled;
    bool _special;
    bool _ownershipEnabled;
    bool _isSaved;


    bool IsOn(ObjectGuid who) const;
    bool IsBanned(ObjectGuid guid) const;

    bool IsWorld() const;

    void UpdateChannelInDB() const;
    void UpdateChannelUseageInDB() const;

    uint8 GetPlayerFlags(ObjectGuid p) const;

    void SetModerator(ObjectGuid const& guid, bool set);
    void SetMute(ObjectGuid const& guid, bool set);

    template <class Builder>
    void SendToAll(Builder& builder, ObjectGuid const& guid = ObjectGuid::Empty) const;

    template <class Builder>
    void SendToAllButOne(Builder& builder, ObjectGuid const& who) const;

    template <class Builder>
    void SendToOne(Builder& builder, ObjectGuid const& who) const;

    template <class Builder>
    void SendToAllWithAddon(Builder& builder, std::string const& addonPrefix, ObjectGuid const& guid = ObjectGuid::Empty) const;
public:
    uint32 m_Team;
    Channel(std::string const& name, uint32 channel_id, uint32 Team = 0);
    std::string GetName() const { return _channelName; }
    uint32 GetChannelId() const { return _channelId; }
    bool IsConstant() const { return _channelId != 0 || _special; }
    bool IsAnnounce() const { return _announceEnabled; }
    bool IsLFG() const { return GetFlags() & CHANNEL_FLAG_LFG; }
    std::string GetPassword() const { return _channelPassword; }
    void SetPassword(std::string const& npassword) { _channelPassword = npassword; }
    void SetAnnounce(bool nannounce) { _announceEnabled = nannounce; }
    uint32 GetNumPlayers() const { return _playersStore.size(); }
    uint8 GetFlags() const { return _channelFlags; }
    bool HasFlag(uint8 flag) const { return _channelFlags & flag; }

    void JoinChannel(Player* player, std::string const& pass, bool clientRequest = false);
    void LeaveChannel(Player* player, bool send = true, bool clientRequest = false);
    void KickOrBan(Player const* player, std::string const& badname, bool ban);
    void Kick(Player const* player, std::string const& badname) { KickOrBan(player, badname, false); }
    void Ban(Player const* player, std::string const& badname) { KickOrBan(player, badname, true); }
    void UnBan(Player const* player, std::string const& badname);
    void Password(Player const* player, std::string const& pass);
    void SetMode(Player const* player, std::string const& p2n, bool mod, bool set);
    void SetOwner(ObjectGuid const& p, bool exclaim = true);
    void _SetOwner(Player const* player, std::string const& newname);
    void SendWhoOwner(Player const* player);
    void _SetModerator(Player const* player, std::string const& newname) { SetMode(player, newname, true, true); }
    void UnsetModerator(Player const* player, std::string const& newname) { SetMode(player, newname, true, false); }
    void _SetMute(Player const* player, std::string const& newname) { SetMode(player, newname, false, true); }
    void UnsetMute(Player const* player, std::string const& newname) { SetMode(player, newname, false, false); }
    void List(Player const* player);
    void Announce(Player const* player);
    void Say(ObjectGuid const& p, std::string const& what, uint32 lang, bool isSpamm = false);
    void Invite(Player const* player, std::string const& newp);
    void DeclineInvite(Player const* player);
    void JoinNotify(ObjectGuid const& guid);                                    // invisible notify
    void LeaveNotify(ObjectGuid const& guid);                                          // invisible notify
    void SilenceAll(Player const* player, std::string const& name);
    void UnsilenceAll(Player const* player, std::string const& name);
    void SetOwnership(bool ownership) { _ownershipEnabled = ownership; };
    static void CleanOldChannelsInDB();

    void AddonSay(ObjectGuid const& guid, std::string const& prefix, std::string const& what);
};
#endif

