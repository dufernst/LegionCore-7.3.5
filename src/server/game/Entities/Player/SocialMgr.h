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

#ifndef __TRINITY_SOCIALMGR_H
#define __TRINITY_SOCIALMGR_H

#include "DatabaseEnv.h"
#include "Common.h"
#include "ObjectGuid.h"
#include "HashFuctor.h"

class SocialMgr;
class PlayerSocial;
class Player;
class WorldPacket;

enum FriendStatus
{
    FRIEND_STATUS_OFFLINE   = 0x00,
    FRIEND_STATUS_ONLINE    = 0x01,
    FRIEND_STATUS_AFK       = 0x02,
    FRIEND_STATUS_DND       = 0x04,
    FRIEND_STATUS_RAF       = 0x08
};

enum SocialFlag
{
    SOCIAL_FLAG_FRIEND      = 0x01,
    SOCIAL_FLAG_IGNORED     = 0x02,
    SOCIAL_FLAG_MUTED       = 0x04,                          // guessed
    SOCIAL_FLAG_UNK         = 0x08,                          // Unknown - does not appear to be RaF

    SOCIAL_FLAG_ALL         = SOCIAL_FLAG_FRIEND | SOCIAL_FLAG_IGNORED | SOCIAL_FLAG_MUTED
};

struct FriendInfo
{
    FriendInfo(uint8 flags = 0);
    FriendInfo(uint8 flags, std::string const& note);

    FriendStatus Status;
    uint32 Area;
    uint32 LfgListActivityID;
    std::string Note;
    uint8 Level;
    uint8 Class;
    uint8 Flags;
};

typedef cds::container::FeldmanHashMap< cds::gc::HP, ObjectGuid, FriendInfo, guidTraits > PlayerSocialMap;
typedef std::map<ObjectGuid, PlayerSocial> SocialMap;

/// Results of friend related commands
enum FriendsResult : uint8
{
    FRIEND_DB_ERROR         = 0x00,
    FRIEND_LIST_FULL        = 0x01,
    FRIEND_ONLINE           = 0x02,
    FRIEND_OFFLINE          = 0x03,
    FRIEND_NOT_FOUND        = 0x04,
    FRIEND_REMOVED          = 0x05,
    FRIEND_ADDED_ONLINE     = 0x06,
    FRIEND_ADDED_OFFLINE    = 0x07,
    FRIEND_ALREADY          = 0x08,
    FRIEND_SELF             = 0x09,
    FRIEND_ENEMY            = 0x0A,
    FRIEND_IGNORE_FULL      = 0x0B,
    FRIEND_IGNORE_SELF      = 0x0C,
    FRIEND_IGNORE_NOT_FOUND = 0x0D,
    FRIEND_IGNORE_ALREADY_S = 0x0E,
    FRIEND_IGNORE_ADDED_S   = 0x0F,
    FRIEND_IGNORE_REMOVED   = 0x10,
    FRIEND_IGNORE_AMBIGUOUS = 0x11,                         // That name is ambiguous, type more of the player's server name
    FRIEND_MUTE_FULL        = 0x12,
    FRIEND_MUTE_SELF        = 0x13,
    FRIEND_MUTE_NOT_FOUND   = 0x14,
    FRIEND_MUTE_ALREADY_S   = 0x15,
    FRIEND_MUTE_ADDED_S     = 0x16,
    FRIEND_MUTE_REMOVED     = 0x17,
    FRIEND_MUTE_AMBIGUOUS   = 0x18,                         // That name is ambiguous, type more of the player's server name
    FRIEND_UNK1             = 0x19,                         // no message at client
    FRIEND_UNK2             = 0x1A,
    FRIEND_UNK3             = 0x1B,
    FRIEND_UNKNOWN          = 0x1C                          // Unknown friend response from server
};

#define SOCIALMGR_FRIEND_LIMIT  50u
#define SOCIALMGR_IGNORE_LIMIT  50u

class PlayerSocial
{
    friend class SocialMgr;
    public:
        PlayerSocial();
        ~PlayerSocial();

        // adding/removing
        bool AddToSocialList(ObjectGuid const& guid, SocialFlag flag);
        void RemoveFromSocialList(ObjectGuid const& guid, SocialFlag flag);
        void SetFriendNote(ObjectGuid const& friendGuid, std::string note);

        // Packet send's
        void SendSocialList(Player* player, uint32 flags);

        // Misc
        bool HasFriend(ObjectGuid const& friend_guid);
        bool HasIgnore(ObjectGuid const& ignore_guid);

        ObjectGuid GetPlayerGUID() const { return m_playerGUID; }
        void SetPlayerGUID(ObjectGuid const& guid) { m_playerGUID = guid; }
        uint32 GetNumberOfSocialsWithFlag(SocialFlag flag);

        std::recursive_mutex m_social_lock;
    private:
        bool _HasContact(ObjectGuid const& guid, SocialFlag flags);

        PlayerSocialMap m_playerSocialMap;
        ObjectGuid m_playerGUID;
};

class SocialMgr
{
        SocialMgr();
        ~SocialMgr();

    public:
        static SocialMgr* instance();

        // Misc
        void RemovePlayerSocial(ObjectGuid const& guid);

        void GetFriendInfo(Player* player, ObjectGuid const& friendGUID, FriendInfo &friendInfo);
        // Packet management
        void SendFriendStatus(Player* player, FriendsResult result, ObjectGuid const& friend_guid, bool broadcast);
        void BroadcastToFriendListers(Player* player, WorldPacket const* packet);
        // Loading
        PlayerSocial *LoadFromDB(PreparedQueryResult result, ObjectGuid const& guid);

        std::vector<Player*> GetVisibleFriendsContaier(Player* player, bool online = false, uint32 lfgListActivityID = 0);
        GuidList GetBNetFriendsGuids(uint32 lfgListActivityID);
        GuidList GetCharFriendsGuids(Player* player, uint32 lfgListActivityID);
        GuidList GetGuildMateGuids(uint32 lfgListActivityID);
        bool HasInFriendsList(Player* player, ObjectGuid guid);
    private:
        SocialMap m_socialMap;
        std::recursive_mutex m_social_lock;
        GuidList _friendList;
        GuidList _guildMateList;
};

#define sSocialMgr SocialMgr::instance()
#endif

