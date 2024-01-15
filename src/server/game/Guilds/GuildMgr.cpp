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

#include "Common.h"
#include "GuildMgr.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"

GuildMgr* GuildMgr::instance()
{
    static GuildMgr instance;
    return &instance;
}

GuildMgr::GuildMgr() : NextGuildId(UI64LIT(1))
{

}

GuildMgr::~GuildMgr()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        delete itr->second;
}

void GuildMgr::UnloadAll()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        itr->second->GetAchievementMgr().ClearMap();
}

void GuildMgr::AddGuild(Guild* guild)
{
    GuildStore[guild->GetId()] = guild;
}

void GuildMgr::RemoveGuild(ObjectGuid::LowType guildId)
{
    GuildStore.erase(guildId);
}

void GuildMgr::SaveGuilds()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        if (itr->second->GetMembersOnline()) // Save guild only with player active
            itr->second->SaveToDB(true);
    }
}

ObjectGuid::LowType GuildMgr::GenerateGuildId()
{
    if (NextGuildId >= std::numeric_limits<ObjectGuid::LowType>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GUILD, "Guild ids overflow!! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }
    return NextGuildId++;
}

void GuildMgr::SetNextGuildId(ObjectGuid::LowType Id)
{
    NextGuildId = Id;
}

std::vector<GuildReward> const& GuildMgr::GetGuildRewards() const
{
    return GuildRewards;
}

GuildChallengeRewardData const& GuildMgr::GetGuildChallengeRewardData() const
{
    return _challengeRewardData;
}

Guild* GuildMgr::GetGuildById(ObjectGuid::LowType guildId) const
{
    if (!guildId)
        return nullptr;

    return Trinity::Containers::MapGetValuePtr(GuildStore, guildId);
}

Guild* GuildMgr::GetGuildByGuid(ObjectGuid const& guid) const
{
    // Full guids are only used when receiving/sending data to client
    // everywhere else guild id is used
    if (guid.IsGuild())
        if (ObjectGuid::LowType guildId = guid.GetCounter())
            return GetGuildById(guildId);

    return nullptr;
}

Guild* GuildMgr::GetGuildByName(std::string const& guildName) const
{
    std::string search = guildName;
    std::transform(search.begin(), search.end(), search.begin(), ::toupper);
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        std::string gname = itr->second->GetName();
        std::transform(gname.begin(), gname.end(), gname.begin(), ::toupper);
        if (search == gname)
            return itr->second;
    }
    return nullptr;
}

std::string GuildMgr::GetGuildNameById(ObjectGuid::LowType const& guildId) const
{
    if (Guild* guild = GetGuildById(guildId))
        return guild->GetName();

    return "";
}

Guild* GuildMgr::GetGuildByLeader(ObjectGuid const& guid) const
{
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        if (itr->second->GetLeaderGUID() == guid)
            return itr->second;

    return nullptr;
}

void GuildMgr::LoadGuilds()
{
    // 1. Load all guilds
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guilds definitions...");
    {
        uint32 oldMSTime = getMSTime();

                                                     //          0          1       2            3           4              5              6             7
        QueryResult result = CharacterDatabase.Query("SELECT g.guildid, g.name, g.leaderguid, g.flags, g.EmblemStyle, g.EmblemColor, g.BorderStyle, g.BorderColor, "
                                                     //      8             9       10        11            12          13           14
                                                     "g.BackgroundColor, g.info, g.motd, g.createdate, g.BankMoney, g.level, COUNT(gbt.guildid) "
                                                     "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildid = gbt.guildid GROUP BY g.guildid ORDER BY g.guildid ASC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild definitions. DB table `guild` is empty.");
            return;
        }
        uint32 count = 0;
        do
        {
            Field* fields = result->Fetch();
            Guild* guild = new Guild();

            if (!guild->LoadFromDB(fields))
            {
                delete guild;
                continue;
            }

            AddGuild(guild);

            ++count;
        }
        while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    }

    // 2. Load all guild ranks
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild ranks...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild rank entries before loading the valid ones
        CharacterDatabase.DirectExecute("DELETE gr FROM guild_rank gr LEFT JOIN guild g ON gr.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                         0    1      2       3                4
        QueryResult result = CharacterDatabase.Query("SELECT guildid, rid, rname, rights, BankMoneyPerDay FROM guild_rank ORDER BY guildid ASC, rid ASC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild ranks. DB table `guild_rank` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadRankFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild ranks in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 3. Load all guild members
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild members...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild member entries before loading the valid ones
        CharacterDatabase.DirectExecute("DELETE gm FROM guild_member gm LEFT JOIN guild g ON gm.guildId = g.guildId WHERE g.guildId IS NULL");

                                                     //          0        1        2     3      4        5                   6
        QueryResult result = CharacterDatabase.Query("SELECT gm.guildid, gm.guid, `rank`, pnote, offnote, BankResetTimeMoney, BankRemMoney, "
                                                     //   7                  8                 9                  10                11                 12
                                                     "BankResetTimeTab0, BankRemSlotsTab0, BankResetTimeTab1, BankRemSlotsTab1, BankResetTimeTab2, BankRemSlotsTab2, "
                                                     //   13                 14                15                 16                17                 18
                                                     "BankResetTimeTab3, BankRemSlotsTab3, BankResetTimeTab4, BankRemSlotsTab4, BankResetTimeTab5, BankRemSlotsTab5, "
                                                     //   19                 20                21                 22
                                                     "BankResetTimeTab6, BankRemSlotsTab6, BankResetTimeTab7, BankRemSlotsTab7, "
                                                     //   23      24       25       26      27         28             29          30         31
                                                     "c.name, c.level, c.class, c.zone, c.account, c.logout_time, re.standing, AchPoint, c.gender, "
                                                     //   32   33          34         35            36          37          38         39
                                                     "profId1, profValue1, profRank1, recipesMask1, profId2, profValue2, profRank2, recipesMask2 "
                                                     "FROM guild_member gm LEFT JOIN characters c ON c.guid = gm.guid LEFT JOIN character_reputation re ON re.guid = gm.guid AND re.faction = 1168 ORDER BY guildid ASC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild members. DB table `guild_member` is empty.");
        }
        else
        {
            uint32 count = 0;

            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadMemberFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild members int %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 4. Load all guild bank tab rights
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading bank tab rights...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild bank right entries before loading the valid ones
        CharacterDatabase.DirectExecute("DELETE gbr FROM guild_bank_right gbr LEFT JOIN guild g ON gbr.guildId = g.guildId WHERE g.guildId IS NULL");

                                                     //       0        1      2    3        4
        QueryResult result = CharacterDatabase.Query("SELECT guildid, TabId, rid, gbright, SlotPerDay FROM guild_bank_right ORDER BY guildid ASC, TabId ASC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild bank tab rights. DB table `guild_bank_right` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankRightFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u bank tab rights in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 5. Load all event logs
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild event logs...");
    {
        uint32 oldMSTime = getMSTime();

        CharacterDatabase.DirectPExecute("DELETE FROM guild_eventlog WHERE LogGuid > %u", sWorld->getIntConfig(CONFIG_GUILD_EVENT_LOG_COUNT));

                                                     //          0        1        2          3            4            5        6
        QueryResult result = CharacterDatabase.Query("SELECT guildid, LogGuid, EventType, PlayerGuid1, PlayerGuid2, NewRank, TimeStamp FROM guild_eventlog ORDER BY TimeStamp DESC, LogGuid DESC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild event logs. DB table `guild_eventlog` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadEventLogFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild event logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 6. Load all bank event logs
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild bank event logs...");
    {
        uint32 oldMSTime = getMSTime();

        // Remove log entries that exceed the number of allowed entries per guild
        CharacterDatabase.DirectPExecute("DELETE FROM guild_bank_eventlog WHERE LogGuid > %u", sWorld->getIntConfig(CONFIG_GUILD_BANK_EVENT_LOG_COUNT));

                                                     //          0        1      2        3          4           5            6               7          8
        QueryResult result = CharacterDatabase.Query("SELECT guildid, TabId, LogGuid, EventType, PlayerGuid, ItemOrMoney, ItemStackCount, DestTabId, TimeStamp FROM guild_bank_eventlog ORDER BY TimeStamp DESC, LogGuid DESC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild bank event logs. DB table `guild_bank_eventlog` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankEventLogFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild bank event logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 7. Load all news event logs
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild news...");
    {
        uint32 oldMSTime = getMSTime();

        CharacterDatabase.DirectPExecute("DELETE FROM guild_newslog WHERE LogGuid > %u", sWorld->getIntConfig(CONFIG_GUILD_NEWS_LOG_COUNT));

        //      0        1        2          3           4      5      6
        QueryResult result = CharacterDatabase.Query("SELECT guildid, LogGuid, EventType, PlayerGuid, Flags, Value, Timestamp, Data FROM guild_newslog ORDER BY TimeStamp DESC, LogGuid DESC");

        if (!result)
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild bank event logs. DB table `guild_newslog` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadGuildNewsLogFromDB(fields);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild news logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 8. Load all guild bank tabs
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Loading guild bank tabs...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild bank tab entries before loading the valid ones
        CharacterDatabase.DirectExecute("DELETE gbt FROM guild_bank_tab gbt LEFT JOIN guild g ON gbt.guildId = g.guildId WHERE g.guildId IS NULL");

                                                     //         0        1      2        3        4
        QueryResult result = CharacterDatabase.Query("SELECT guildid, TabId, TabName, TabIcon, TabText FROM guild_bank_tab ORDER BY guildid ASC, TabId ASC");

        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild bank tabs. DB table `guild_bank_tab` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[0].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankTabFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild bank tabs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 9. Fill all guild bank tabs
    TC_LOG_INFO(LOG_FILTER_GUILD, "Filling bank tabs with items...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphan guild bank items
        CharacterDatabase.DirectExecute("DELETE gbi FROM guild_bank_item gbi LEFT JOIN guild g ON gbi.guildId = g.guildId WHERE g.guildId IS NULL");

        PreparedQueryResult result = CharacterDatabase.Query(CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_BANK_ITEMS));
        if (!result)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild bank tab items. DB table `guild_bank_item` or `item_instance` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint64 guildId = fields[50].GetUInt64();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankItemFromDB(fields);

                ++count;
            }
            while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild bank tab items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 10. Load guild achievements
    {
        for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        {
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_ACHIEVEMENT);
            stmt->setUInt64(0, itr->first);
            PreparedQueryResult achievementResult = CharacterDatabase.Query(stmt);
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUILD_ACHIEVEMENT_CRITERIA);
            stmt->setUInt64(0, itr->first);
            PreparedQueryResult criteriaResult = CharacterDatabase.Query(stmt);

            itr->second->GetAchievementMgr().LoadFromDB(achievementResult, criteriaResult);
        }
    }

    // 11. Update Guild Known Recipes
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        Guild* guild = itr->second;
        if (!guild)
            continue;

        guild->UpdateGuildRecipes();
    }

    // 12. Validate loaded guild data
    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "Validating data of loaded guilds...");
    {
        uint32 oldMSTime = getMSTime();

        for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end();)
        {
            Guild* guild = itr->second;
            if (guild)
            {
                volatile uint32 _guildId = guild->GetId();
                if(!guild->GetMemberCount())
                {
                    //Delete guild without member
                    CharacterDatabase.DirectPExecute("DELETE FROM guild WHERE `guildid` IN (%u)", _guildId);
                }

                if (!guild->Validate())
                {
                    GuildStore.erase(itr++);
                    delete guild;
                }
                else
                    ++itr;
            }
            else
                ++itr;
        }

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Validated data of loaded guilds in %u ms", GetMSTimeDiffToNow(oldMSTime));
    }

    /// 13. Loading guild challenges
    TC_LOG_INFO(LOG_FILTER_GENERAL, "Loading guild challenges...");
    {
        auto oldMSTime = getMSTime();
        if (auto result = CharacterDatabase.Query(CharacterDatabase.GetPreparedStatement(CHAR_LOAD_GUILD_CHALLENGES)))
        {
            uint32 count = 0;
            do
            {
                auto fields = result->Fetch();
                if (auto guild = GetGuildById(fields[0].GetInt32()))
                    guild->LoadGuildChallengesFromDB(fields);

                ++count;
            } while (result->NextRow());

            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild challenges in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }
}

void GuildMgr::LoadGuildXpForLevel()
{
    uint32 oldMSTime = getMSTime();

    GuildXPperLevel.resize(sWorld->getIntConfig(CONFIG_GUILD_MAX_LEVEL));
    for (uint8 level = 0; level < sWorld->getIntConfig(CONFIG_GUILD_MAX_LEVEL); ++level)
        GuildXPperLevel[level] = 0;

    //                                                 0        1
    QueryResult result  = WorldDatabase.Query("SELECT lvl, xp_for_next_level FROM guild_xp_for_level");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 xp for guild level definitions. DB table `guild_xp_for_level` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 level        = fields[0].GetUInt8();
        uint32 requiredXP   = fields[1].GetUInt64();

        if (level >= sWorld->getIntConfig(CONFIG_GUILD_MAX_LEVEL))
        {
            TC_LOG_INFO(LOG_FILTER_GENERAL, "Unused (> Guild.MaxLevel in worldserver.conf) level %u in `guild_xp_for_level` table, ignoring.", uint32(level));
            continue;
        }

        GuildXPperLevel[level] = requiredXP;
        ++count;

    } 
    while (result->NextRow());

    // fill level gaps
    for (uint8 level = 1; level < sWorld->getIntConfig(CONFIG_GUILD_MAX_LEVEL); ++level)
    {
        if (!GuildXPperLevel[level])
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Level %i does not have XP for guild level data. Using data of level [%i] + 1660000.", level+1, level);
            GuildXPperLevel[level] = GuildXPperLevel[level - 1] + 1660000;
        }
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u xp for guild level definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildMgr::LoadGuildRewards()
{
    GuildRewards.clear();
    uint32 oldMSTime = getMSTime();

    //                                                  0      1         2        3
    QueryResult result = WorldDatabase.Query("SELECT entry, standing, racemask, price FROM guild_rewards");

    if (!result)
    {
        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 guild reward definitions. DB table `guild_rewards` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        GuildReward reward;
        Field* fields = result->Fetch();
        reward.Entry = fields[0].GetUInt32();
        reward.Standing = fields[1].GetUInt8();
        reward.Racemask = fields[2].GetUInt64();
        reward.Price = fields[3].GetUInt64();

        if (!sObjectMgr->GetItemTemplate(reward.Entry))
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Guild rewards contains not existing item entry %u", reward.Entry);
            continue;
        }

        if (reward.Standing >= MAX_REPUTATION_RANK)
        {
            TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Guild rewards contains wrong reputation standing %u, max is %u", uint32(reward.Standing), MAX_REPUTATION_RANK - 1);
            continue;
        }

        QueryResult reqAchievementResult = WorldDatabase.PQuery("SELECT achievement FROM guild_rewards WHERE Entry = %u", reward.Entry);
        if (reqAchievementResult)
        {
            do
            {
                fields = reqAchievementResult->Fetch();

                uint32 requiredAchievementId = fields[0].GetUInt32();

                if (requiredAchievementId)
                {
                    if (!sAchievementStore.LookupEntry(requiredAchievementId))
                    {
                        TC_LOG_ERROR(LOG_FILTER_SERVER_LOADING, "Guild rewards contains not existing achievement entry %u", requiredAchievementId);
                        continue;
                    }

                    reward.AchievementsRequired.push_back(requiredAchievementId);
                }
            } while (reqAchievementResult->NextRow());
        }

        GuildRewards.push_back(reward);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u guild reward definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildMgr::LoadGuildChallengeRewardInfo()
{
    auto oldMSTime = getMSTime();

    auto result = WorldDatabase.Query("SELECT Type, Gold, Gold2, Count FROM guild_challenge_reward");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "%s >> Loaded 0 guild challenge reward data.", __FUNCTION__);
        return;
    }

    _challengeRewardData.reserve(result->GetRowCount());

    uint32 count = 0;

    do
    {
        auto fields = result->Fetch();
        auto type = fields[0].GetUInt32();
        if (type >= ChallengeMax)
        {
            TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "%s >> guild_challenge_reward has unknown challenge type %u, skip.", __FUNCTION__, type);
            continue;
        }

        GuildChallengeReward reward;
        reward.Gold = fields[1].GetUInt32();
        reward.Gold2 = fields[2].GetUInt32();
        reward.ChallengeCount = fields[3].GetUInt32();
        _challengeRewardData.push_back(reward);
        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "%s >> Loaded %u guild challenge reward data in %u ms.", __FUNCTION__, count, GetMSTimeDiffToNow(oldMSTime));
}
