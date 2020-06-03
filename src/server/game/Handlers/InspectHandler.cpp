/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "InspectPackets.h"
#include "GuildMgr.h"
#include "Bracket.h"

void WorldSession::HandleInspect(WorldPackets::Inspect::Inspect& packet)
{
    Player* player = ObjectAccessor::FindPlayer(packet.Target);
    if (!player)
        return;

    if (!GetPlayer()->IsSpectator() && GetPlayer()->IsValidAttackTarget(player))
        return;

    uint8 index = player->GetActiveTalentGroup();

    WorldPackets::Inspect::InspectResult inspectResult;

    for (uint8 i = 0; i < EQUIPMENT_SLOT_END; ++i)
        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            inspectResult.Items.emplace_back(item, i);

    inspectResult.ClassID = player->getClass();
    inspectResult.GenderID = player->getGender();
    inspectResult.InspecteeGUID = packet.Target;
    inspectResult.SpecializationID = player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID);

    if (sWorld->getBoolConfig(CONFIG_TALENTS_INSPECTING) || GetPlayer()->isGameMaster())
    {
        for (auto const& v : *player->GetTalentMap(index))
            // if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(v.first))
                // if (spell->talentId)
                    // inspectResult.Talents.push_back(spell->talentId);
            if (v.second != PLAYERSPELL_REMOVED)
                inspectResult.Talents.push_back(v.first);

        for (auto const& v : *player->GetPvPTalentMap(index))
            // if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(v.first))
                // if (spell->talentId)
                    // inspectResult.PvPTalents.push_back(spell->talentId);
            if (v.second != PLAYERSPELL_REMOVED)
                inspectResult.PvPTalents.push_back(v.first);

        for (auto const& glyph : player->GetGlyphs(index))
            inspectResult.Glyphs.push_back(glyph);
    }

    if (Guild const* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
    {
        inspectResult.GuildData = boost::in_place();
        inspectResult.GuildData->GuildGUID = guild->GetGUID();
        inspectResult.GuildData->NumGuildMembers = guild->GetMembersCount();
        inspectResult.GuildData->AchievementPoints = guild->GetAchievementMgr().GetAchievementPoints();
    }

    SendPacket(inspectResult.Write());
}

void WorldSession::HandleRequestHonorStats(WorldPackets::Inspect::RequestHonorStats& packet)
{
    Player* player = ObjectAccessor::FindPlayer(packet.TargetGUID);
    if (!player)
    {
        TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "WorldSession::HandleRequestHonorStats: Target %s not found.", packet.TargetGUID.ToString().c_str());
        return;
    }

    WorldPackets::Inspect::InspectHonorStats honorStats;
    honorStats.PlayerGUID  = packet.TargetGUID;
    honorStats.LifetimeHK  = player->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);
    honorStats.YesterdayHK = player->GetUInt16Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, 1);
    honorStats.TodayHK     = player->GetUInt16Value(PLAYER_FIELD_YESTERDAY_HONORABLE_KILLS, 0);
    honorStats.LifetimeMaxRank = 0; /// @todo
    SendPacket(honorStats.Write());
}

void WorldSession::HandleInspectPVP(WorldPackets::Inspect::InspectPVPRequest& packet)
{
    Player* player = ObjectAccessor::FindPlayer(packet.InspectTarget);
    if (!player)
        return;

    WorldPackets::Inspect::InspectPVPResponse response;
    response.ClientGUID = packet.InspectTarget;

    for (uint8 i = MS::Battlegrounds::BracketType::Arena2v2; i < MS::Battlegrounds::BracketType::Max; ++i)
    {
        Bracket* bracket = player->getBracket(i);
        ASSERT(bracket);

        WorldPackets::Inspect::PVPBracketData data;
        data.Rating = bracket->getRating();
        data.Rank = 0;
        data.WeeklyPlayed = bracket->GetBracketInfo(BRACKET_WEEK_GAMES);
        data.WeeklyWon = bracket->GetBracketInfo(BRACKET_WEEK_WIN);
        data.SeasonPlayed = bracket->GetBracketInfo(BRACKET_SEASON_GAMES);
        data.SeasonWon = bracket->GetBracketInfo(BRACKET_SEASON_WIN);
        data.WeeklyBestRating = bracket->GetBracketInfo(BRACKET_WEEK_BEST);
        data.Bracket = i;
        response.Bracket.push_back(data);
    }

    SendPacket(response.Write());
}

void WorldSession::HandleQueryInspectAchievements(WorldPackets::Inspect::QueryInspectAchievements& inspect)
{
    if (Player* player = ObjectAccessor::FindPlayer(inspect.Guid))
        player->GetAchievementMgr()->SendAchievementInfo(_player);
}
