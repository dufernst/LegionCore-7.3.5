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

#include "GuildFinderMgr.h"
#include "GuildMgr.h"
#include "World.h"
#include "GuildPackets.h"
#include "DatabaseEnv.h"

MembershipRequest::MembershipRequest(MembershipRequest const& settings) : _comment(settings.GetComment())
{
    _availability = settings.GetAvailability();
    _classRoles = settings.GetClassRoles();
    _playStyle = settings.GetPlayStyle();
    _guildId = settings.GetGuildGuid();
    _playerGUID = settings.GetPlayerGUID();
    _time = settings.GetSubmitTime();
}

MembershipRequest::MembershipRequest(ObjectGuid const& playerGUID, ObjectGuid const& guildId, uint32 availability, uint32 classRoles, uint32 playStyle, std::string& comment, time_t submitTime) : _guildId(guildId), _playerGUID(playerGUID), _time(submitTime), _comment(comment), _availability(availability), _classRoles(classRoles), _playStyle(playStyle) {}
MembershipRequest::MembershipRequest() : _time(time(nullptr)), _availability(0), _classRoles(0), _playStyle(0) { }

uint8 MembershipRequest::GetClass() const
{
    if (auto const& nameData = sWorld->GetCharacterInfo(_playerGUID))
        return nameData->Class;

    return 0;
}

uint8 MembershipRequest::GetLevel() const
{
    if (auto const& nameData = sWorld->GetCharacterInfo(_playerGUID))
        return nameData->Level;

    return 1;
}

std::string const& MembershipRequest::GetName() const
{
    if (auto const& nameData = sWorld->GetCharacterInfo(_playerGUID))
        return nameData->Name;

    return "";
}

uint8 MembershipRequest::GetGender() const
{
    if (auto const& nameData = sWorld->GetCharacterInfo(_playerGUID))
        return nameData->Sex;

    return 0;
}

time_t MembershipRequest::GetSubmitTime() const
{
    return _time;
}

time_t MembershipRequest::GetExpiryTime() const
{
    return time_t(_time + 30 * 24 * 3600);
}

std::string const& MembershipRequest::GetComment() const
{
    return _comment;
}

LFGuildPlayer::LFGuildPlayer()
{
    _guid.Clear();
    _roles = 0;
    _availability = 0;
    _playStyle = 0;
    _level = 0;
}

LFGuildPlayer::LFGuildPlayer(ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level)
{
    _guid = guid;
    _roles = role;
    _availability = availability;
    _playStyle = playStyle;
    _level = level;
}

LFGuildPlayer::LFGuildPlayer(ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level, std::string& comment) : _comment(comment)
{
    _guid = guid;
    _roles = role;
    _availability = availability;
    _playStyle = playStyle;
    _level = level;
}

LFGuildPlayer::LFGuildPlayer(LFGuildPlayer const& settings): _comment(settings.GetComment())
{
    _guid = settings.GetGUID();
    _roles = settings.GetClassRoles();
    _availability = settings.GetAvailability();
    _playStyle = settings.GetPlayStyle();
    _level = settings.GetLevel();
}

LFGuildSettings::LFGuildSettings(): LFGuildPlayer(), _team(TEAM_ALLIANCE), _listed(false)
{
}

LFGuildSettings::LFGuildSettings(bool listed, TeamId team): LFGuildPlayer(), _team(team), _listed(listed)
{
}

LFGuildSettings::LFGuildSettings(bool listed, TeamId team, ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level): LFGuildPlayer(guid, role, availability, playStyle, level), _team(team), _listed(listed)
{
}

LFGuildSettings::LFGuildSettings(bool listed, TeamId team, ObjectGuid const& guid, uint8 role, uint8 availability, uint8 playStyle, uint8 level, std::string& comment): LFGuildPlayer(guid, role, availability, playStyle, level, comment), _team(team), _listed(listed)
{
}

LFGuildSettings::LFGuildSettings(LFGuildSettings const& settings): LFGuildPlayer(settings), _team(settings.GetTeam()), _listed(settings.IsListed())
{
}

GuildFinderMgr::GuildFinderMgr() = default;

GuildFinderMgr::~GuildFinderMgr() = default;

void GuildFinderMgr::LoadFromDB()
{
    LoadGuildSettings();
    LoadMembershipRequests();
}

void GuildFinderMgr::LoadGuildSettings()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild finder guild-related settings...");
    //                                                           0                1             2                  3             4           5             6         7
    QueryResult result = CharacterDatabase.Query("SELECT gfgs.guildId, gfgs.availability, gfgs.classRoles, gfgs.interests, gfgs.level, gfgs.listed, gfgs.comment, c.race "
        "FROM guild_finder_guild_settings gfgs LEFT JOIN guild_member gm ON gm.guildid=gfgs.guildId LEFT JOIN characters c ON c.guid = gm.guid LIMIT 1");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild finder guild-related settings. Table `guild_finder_guild_settings` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 oldMSTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();
        ObjectGuid guildId = ObjectGuid::Create<HighGuid::Guild>(fields[0].GetUInt64());
        uint8 availability = fields[1].GetUInt8();
        uint8 classRoles = fields[2].GetUInt8();
        uint8 playStyle = fields[3].GetUInt8();
        uint8 level = fields[4].GetUInt8();
        bool listed = (fields[5].GetUInt8() != 0);
        std::string comment = fields[6].GetString();

        TeamId guildTeam = TEAM_NEUTRAL;
        if (ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(fields[7].GetUInt8()))
            guildTeam = static_cast<TeamId>(raceEntry->Alliance);

        _guildSettings[guildId] = LFGuildSettings(listed, guildTeam, guildId, classRoles, availability, playStyle, level, comment);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild finder guild-related settings in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildFinderMgr::LoadMembershipRequests()
{
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild finder membership requests...");
    //                                                      0         1           2            3           4         5         6
    QueryResult result = CharacterDatabase.Query("SELECT guildId, playerGuid, availability, classRole, interests, comment, submitTime FROM guild_finder_applicant");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild finder membership requests. Table `guild_finder_applicant` is empty.");
        return;
    }

    uint32 count = 0;
    uint32 oldMSTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();
        ObjectGuid guildId = ObjectGuid::Create<HighGuid::Guild>(fields[0].GetUInt64());
        ObjectGuid playerId = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt64());
        uint8 availability = fields[2].GetUInt8();
        uint8 classRoles = fields[3].GetUInt8();
        uint8 playStyle = fields[4].GetUInt8();
        std::string comment = fields[5].GetString();
        uint32 submitTime = fields[6].GetUInt32();

        MembershipRequest request(playerId, guildId, availability, classRoles, playStyle, comment, time_t(submitTime));

        _membershipRequests[guildId].push_back(request);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild finder membership requests in %u ms.", count, GetMSTimeDiffToNow(oldMSTime));
}

GuildFinderMgr* GuildFinderMgr::instance()
{
    static GuildFinderMgr instance;
    return &instance;
}

void GuildFinderMgr::AddMembershipRequest(ObjectGuid const& guildGuid, MembershipRequest const& request)
{
    _membershipRequests[guildGuid].push_back(request);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_FINDER_APPLICANT);
    stmt->setUInt64(0, request.GetGuildGuid().GetCounter());
    stmt->setUInt64(1, request.GetPlayerGUID().GetCounter());
    stmt->setUInt8(2, request.GetAvailability());
    stmt->setUInt8(3, request.GetClassRoles());
    stmt->setUInt8(4, request.GetPlayStyle());
    stmt->setString(5, request.GetComment());
    stmt->setUInt32(6, request.GetSubmitTime());
    trans->Append(stmt);
    CharacterDatabase.CommitTransaction(trans);

    if (Player* player = ObjectAccessor::FindPlayer(request.GetPlayerGUID()))
        SendMembershipRequestListUpdate(*player);

    if (Guild* guild = sGuildMgr->GetGuildById(guildGuid.GetCounter()))
        SendApplicantListUpdate(*guild);
}

void GuildFinderMgr::RemoveAllMembershipRequestsFromPlayer(ObjectGuid const& playerId)
{
    for (auto& itr : _membershipRequests)
    {
        auto itr2 = itr.second.begin();
        for (; itr2 != itr.second.end(); ++itr2)
            if (itr2->GetPlayerGUID() == playerId)
                break;

        if (itr2 == itr.second.end())
            return;

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
        stmt->setUInt64(0, itr2->GetGuildGuid().GetCounter());
        stmt->setUInt64(1, itr2->GetPlayerGUID().GetCounter());
        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);
        itr.second.erase(itr2);

        if (Guild* guild = sGuildMgr->GetGuildById(itr.first.GetCounter()))
            SendApplicantListUpdate(*guild);
    }
}

void GuildFinderMgr::RemoveMembershipRequest(ObjectGuid const& playerId, ObjectGuid const& guildId)
{
    auto itr = _membershipRequests[guildId].begin();
    for (; itr != _membershipRequests[guildId].end(); ++itr)
        if (itr->GetPlayerGUID() == playerId)
            break;

    if (itr == _membershipRequests[guildId].end())
        return;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
    stmt->setUInt64(0, itr->GetGuildGuid().GetCounter());
    stmt->setUInt64(1, itr->GetPlayerGUID().GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);

    _membershipRequests[guildId].erase(itr);

    if (Player* player = ObjectAccessor::FindPlayer(playerId))
        SendMembershipRequestListUpdate(*player);

    if (Guild* guild = sGuildMgr->GetGuildById(guildId.GetCounter()))
        SendApplicantListUpdate(*guild);
}

std::list<MembershipRequest> GuildFinderMgr::GetAllMembershipRequestsForPlayer(ObjectGuid const& playerGuid)
{
    std::list<MembershipRequest> resultSet;
    for (MembershipRequestStore::const_iterator itr = _membershipRequests.begin(); itr != _membershipRequests.end(); ++itr)
    {
        auto const& guildReqs = itr->second;
        for (const auto& guildReq : guildReqs)
        {
            if (guildReq.GetPlayerGUID() == playerGuid)
            {
                resultSet.push_back(guildReq);
                break;
            }
        }
    }
    return resultSet;
}

uint8 GuildFinderMgr::CountRequestsFromPlayer(ObjectGuid const& playerId)
{
    uint8 result = 0;
    for (MembershipRequestStore::const_iterator itr = _membershipRequests.begin(); itr != _membershipRequests.end(); ++itr)
    {
        for (const auto& itr2 : itr->second)
        {
            if (itr2.GetPlayerGUID() != playerId)
                continue;

            ++result;
            break;
        }
    }
    return result;
}

LFGuildStore GuildFinderMgr::GetGuildsMatchingSetting(LFGuildPlayer& settings, TeamId faction)
{
    LFGuildStore resultSet;
    for (LFGuildStore::const_iterator itr = _guildSettings.begin(); itr != _guildSettings.end(); ++itr)
    {
        LFGuildSettings const& guildSettings = itr->second;

        if (guildSettings.GetTeam() != faction)
            continue;

        if (!(guildSettings.GetAvailability() & settings.GetAvailability()))
            continue;

        if (!(guildSettings.GetClassRoles() & settings.GetClassRoles()))
            continue;

        if (!(guildSettings.GetPlayStyle() & settings.GetPlayStyle()))
            continue;

        if (!(guildSettings.GetLevel() & settings.GetLevel()))
            continue;

        resultSet.insert(std::make_pair(itr->first, guildSettings));
    }

    return resultSet;
}

bool GuildFinderMgr::HasRequest(ObjectGuid const& playerId, ObjectGuid const& guildId)
{
    for (auto const& itr : _membershipRequests[guildId])
        if (itr.GetPlayerGUID() == playerId)
            return true;

    return false;
}

void GuildFinderMgr::SetGuildSettings(ObjectGuid const& guildGuid, LFGuildSettings const& settings)
{
    _guildSettings[guildGuid] = settings;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_GUILD_FINDER_GUILD_SETTINGS);
    stmt->setUInt64(0, settings.GetGUID().GetCounter());
    stmt->setUInt8(1, settings.GetAvailability());
    stmt->setUInt8(2, settings.GetClassRoles());
    stmt->setUInt8(3, settings.GetPlayStyle());
    stmt->setUInt8(4, settings.GetLevel());
    stmt->setUInt8(5, settings.IsListed());
    stmt->setString(6, settings.GetComment());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

LFGuildSettings GuildFinderMgr::GetGuildSettings(ObjectGuid const& guildGuid)
{
    return _guildSettings.find(guildGuid) != _guildSettings.end() ? _guildSettings[guildGuid] : LFGuildSettings();
}

void GuildFinderMgr::DeleteGuild(ObjectGuid const& guildId)
{
    auto itr = _membershipRequests[guildId].begin();
    while (itr != _membershipRequests[guildId].end())
    {
        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        ObjectGuid applicant = itr->GetPlayerGUID();

        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_APPLICANT);
        stmt->setUInt64(0, itr->GetGuildGuid().GetCounter());
        stmt->setUInt64(1, applicant.GetCounter());
        trans->Append(stmt);

        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GUILD_FINDER_GUILD_SETTINGS);
        stmt->setUInt64(0, itr->GetGuildGuid().GetCounter());
        trans->Append(stmt);

        CharacterDatabase.CommitTransaction(trans);
        _membershipRequests[guildId].erase(itr);

        if (Player* player = ObjectAccessor::FindPlayer(applicant))
            SendMembershipRequestListUpdate(*player);
    }

    _membershipRequests.erase(guildId);
    _guildSettings.erase(guildId);

    if (Guild* guild = sGuildMgr->GetGuildById(guildId.GetCounter()))
        SendApplicantListUpdate(*guild);
}

std::vector<MembershipRequest> GuildFinderMgr::GetAllMembershipRequestsForGuild(ObjectGuid const& guildGuid)
{
    return _membershipRequests.find(guildGuid) != _membershipRequests.end() ? _membershipRequests[guildGuid] : std::vector<MembershipRequest>();
}

void GuildFinderMgr::SendApplicantListUpdate(Guild& guild)
{
    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr->GetAllMembershipRequestsForGuild(guild.GetGUID());

    WorldPackets::Guild::LFGuildRecruits recruit;
    //recruit.UpdateTime 
    recruit.Recruits.reserve(recruitsList.size());
    for (auto const& x : recruitsList)
    {
        WorldPackets::Guild::LFGuildRecruitData data;
        data.RecruitGUID = x.GetPlayerGUID();
        data.RecruitVirtualRealm = GetVirtualRealmAddress();
        data.CharacterClass = x.GetClass();
        data.CharacterGender = x.GetGender();
        data.CharacterLevel = x.GetLevel();
        data.ClassRoles = x.GetClassRoles();
        data.PlayStyle = x.GetPlayStyle();
        data.Availability = x.GetAvailability();
        data.SecondsSinceCreated = time(nullptr) - x.GetSubmitTime();
        data.SecondsUntilExpiration = x.GetExpiryTime() - time(nullptr);
        data.Name = x.GetName();
        data.Comment = x.GetComment();
        recruit.Recruits.push_back(data);
    }

    if (Player* player = ObjectAccessor::FindPlayer(guild.GetLeaderGUID()))
        player->SendDirectMessage(recruit.Write());

    guild.BroadcastPacketToRank(recruit.Write(), GR_OFFICER);
}

void GuildFinderMgr::SendMembershipRequestListUpdate(Player& player)
{
    std::vector<MembershipRequest> recruitsList = sGuildFinderMgr->GetAllMembershipRequestsForGuild(ObjectGuid::Create<HighGuid::Guild>(player.GetGuildId()));

    WorldPackets::Guild::LFGuildRecruits recruits;
    //recruit.UpdateTime 
    recruits.Recruits.reserve(recruitsList.size());
    for (auto const& x : recruitsList)
    {
        WorldPackets::Guild::LFGuildRecruitData data;
        data.RecruitGUID = x.GetPlayerGUID();
        data.RecruitVirtualRealm = GetVirtualRealmAddress();
        data.CharacterClass = x.GetClass();
        data.CharacterGender = x.GetGender();
        data.CharacterLevel = x.GetLevel();
        data.ClassRoles = x.GetClassRoles();
        data.PlayStyle = x.GetPlayStyle();
        data.Availability = x.GetAvailability();
        data.SecondsSinceCreated = time(nullptr) - x.GetSubmitTime();
        data.SecondsUntilExpiration = x.GetExpiryTime() - time(nullptr);
        data.Name = x.GetName();
        data.Comment = x.GetComment();
        recruits.Recruits.push_back(data);
    }

    player.SendDirectMessage(recruits.Write());
}
