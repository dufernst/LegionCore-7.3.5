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

#ifndef __TRINITY_GUILDFINDER_H
#define __TRINITY_GUILDFINDER_H

#include "Common.h"
#include "World.h"
#include "GuildMgr.h"
#include "ObjectGuid.h"

enum GuildFinderPlayStyle
{
    PLAY_STYLE_QUESTING      = 0x01,
    PLAY_STYLE_DUNGEONS      = 0x02,
    PLAY_STYLE_RAIDS         = 0x04,
    PLAY_STYLE_PVP           = 0x08,
    PLAY_STYLE_ROLE_PLAYING  = 0x10,

    ALL_PLAY_STYLES           = PLAY_STYLE_QUESTING | PLAY_STYLE_DUNGEONS | PLAY_STYLE_RAIDS | PLAY_STYLE_PVP | PLAY_STYLE_ROLE_PLAYING
};

enum GuildFinderOptionsAvailability
{
    AVAILABILITY_WEEKDAYS     = 0x1,
    AVAILABILITY_WEEKENDS     = 0x2,
    ALL_WEEK                  = AVAILABILITY_WEEKDAYS | AVAILABILITY_WEEKENDS
};

enum GuildFinderOptionsRoles
{
    GUILDFINDER_ROLE_TANK        = 0x1,
    GUILDFINDER_ROLE_HEALER      = 0x2,
    GUILDFINDER_ROLE_DPS         = 0x4,
    GUILDFINDER_ALL_ROLES        = GUILDFINDER_ROLE_TANK | GUILDFINDER_ROLE_HEALER | GUILDFINDER_ROLE_DPS
};

enum GuildFinderOptionsLevel
{
    ANY_FINDER_LEVEL   = 0x1,
    MAX_FINDER_LEVEL   = 0x2,
    ALL_GUILDFINDER_LEVELS = ANY_FINDER_LEVEL | MAX_FINDER_LEVEL
};

/// Holds all required informations about a membership request
struct MembershipRequest
{
    MembershipRequest(MembershipRequest const& settings);
    MembershipRequest(ObjectGuid const& playerGUID, ObjectGuid const& guildId, uint32 availability, uint32 classRoles, uint32 playStyle, std::string& comment, time_t submitTime);
    MembershipRequest();

    ObjectGuid GetGuildGuid() const { return _guildId; }
    ObjectGuid GetPlayerGUID() const { return _playerGUID; }
    uint8 GetAvailability() const { return _availability; }
    uint8 GetClassRoles() const { return _classRoles; }
    uint8 GetPlayStyle() const { return _playStyle; }

    uint8 GetClass() const;
    uint8 GetLevel() const;
    std::string const& GetName() const;
    uint8 GetGender() const;

    time_t GetSubmitTime() const;
    time_t GetExpiryTime() const; // Adding 30 days
    std::string const& GetComment() const;

private:
    ObjectGuid _guildId;
    ObjectGuid _playerGUID;
    time_t _time;
    std::string _comment;
    uint8 _availability;
    uint8 _classRoles;
    uint8 _playStyle;
};

struct LFGuildPlayer
{
    LFGuildPlayer();
    LFGuildPlayer(ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level);
    LFGuildPlayer(ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level, std::string& comment);
    LFGuildPlayer(LFGuildPlayer const& settings);

    ObjectGuid const& GetGUID() const { return _guid; }
    uint8 GetClassRoles() const { return _roles; }
    uint8 GetAvailability() const { return _availability; }
    uint8 GetPlayStyle() const { return _playStyle; }
    uint8 GetLevel() const { return _level; }
    std::string const& GetComment() const { return _comment; }

private:
    std::string _comment;
    ObjectGuid _guid;
    uint8 _roles;
    uint8 _availability;
    uint8 _playStyle;
    uint8 _level;
};

/// Holds settings for a guild in the finder system. Saved to database.
struct LFGuildSettings : public LFGuildPlayer
{
    LFGuildSettings();
    LFGuildSettings(bool listed, TeamId team);
    LFGuildSettings(bool listed, TeamId team, ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level);
    LFGuildSettings(bool listed, TeamId team, ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level, std::string& comment);
    LFGuildSettings(LFGuildSettings const& settings);

    bool IsListed() const { return _listed; }
    void SetListed(bool state) { _listed = state; }
    TeamId GetTeam() const { return _team; }
private:
    TeamId _team;
    bool _listed;
};

typedef std::map<ObjectGuid /* guildGuid */, LFGuildSettings> LFGuildStore;
typedef std::map<ObjectGuid /* guildGuid */, std::vector<MembershipRequest> > MembershipRequestStore;

class GuildFinderMgr
{
        GuildFinderMgr();
        ~GuildFinderMgr();

        LFGuildStore  _guildSettings;

        MembershipRequestStore _membershipRequests;

        void LoadGuildSettings();
        void LoadMembershipRequests();

    public:
        static GuildFinderMgr* instance();

        void LoadFromDB();

        void SetGuildSettings(ObjectGuid const& guildGuid, LFGuildSettings const& settings);

        LFGuildSettings GetGuildSettings(ObjectGuid const& guildGuid);

        void AddMembershipRequest(ObjectGuid const& guildGuid, MembershipRequest const& request);
        void RemoveAllMembershipRequestsFromPlayer(ObjectGuid const& playerId);
        void RemoveMembershipRequest(ObjectGuid const& playerId, ObjectGuid const& guildId);

        void DeleteGuild(ObjectGuid const&  guildId);

        std::vector<MembershipRequest> GetAllMembershipRequestsForGuild(ObjectGuid const& guildGuid);

        std::list<MembershipRequest> GetAllMembershipRequestsForPlayer(ObjectGuid const& playerGuid);
        LFGuildStore GetGuildsMatchingSetting(LFGuildPlayer& settings, TeamId faction);
        bool HasRequest(ObjectGuid const& playerId, ObjectGuid const& guildId);
        uint8 CountRequestsFromPlayer(ObjectGuid const& playerId);
        void SendApplicantListUpdate(Guild& guild);
        void SendMembershipRequestListUpdate(Player& player);
};

#define sGuildFinderMgr GuildFinderMgr::instance()

#endif // __TRINITY_GUILDFINDER_H