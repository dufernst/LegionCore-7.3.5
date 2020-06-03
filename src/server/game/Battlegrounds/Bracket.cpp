/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#include "BattlegroundMgr.h"
#include "Bracket.h"
#include "DatabaseEnv.h"

Bracket::Bracket(ObjectGuid guid, uint8 type) :
m_rating(0), m_ratingLastChange(0), m_mmr_lastChage(0), m_Type(type), m_owner(guid), m_state(BRACKET_NEW)
{
    m_mmv = sWorld->getIntConfig(CONFIG_ARENA_START_MATCHMAKER_RATING);
    memset(values, 0, sizeof(uint32) * BRACKET_END);
}

void Bracket::InitStats(uint16 rating, uint16 mmr, uint32 games, uint32 wins, uint32 week_games, uint32 week_wins, uint16 best_week, uint16 best, uint16 bestWeekLast)
{
    m_rating = rating;
    m_mmv = mmr;

    values[BRACKET_SEASON_GAMES] = games;
    values[BRACKET_SEASON_WIN] = wins;
    values[BRACKET_WEEK_GAMES] = week_games;
    values[BRACKET_WEEK_WIN] = week_wins;
    values[BRACKET_WEEK_BEST] = best_week;
    values[BRACKET_BEST] = best;
    values[BRACKET_WEEK_BEST_LAST] = bestWeekLast;

    m_state = BRACKET_UNCHANGED;
}

float GetChanceAgainst(int ownRating, int opponentRating)
{
    // Returns the chance to win against a team with the given rating, used in the rating adjustment calculation
    // ELO system
    return 1.0f / (1.0f + exp(log(10.0f) * static_cast<float>(static_cast<float>(opponentRating) - static_cast<float>(ownRating)) / 300.0f));
}

int GetRatingMod(int ownRating, int opponentRating, bool won, bool rbg = false)
{
    // Calc Arena Rating
    // 'Chance' calculation - to beat the opponent
    // This is a simulation. Not much info on how it really works
    float chance = GetChanceAgainst(ownRating, opponentRating);
    float won_mod = won ? 1.0f : 0.0f;

    // Calculate the rating modification
    float mod;

    // TODO: Replace this hack with using the confidence factor (limiting the factor to 2.0f)
    if (!rbg && won && ownRating < 1300)
    {
        if (ownRating < 1000)
            mod = 96.0f * (won_mod - chance);
        else
            mod = (48.0f + 48.0f * (1300.0f - float(ownRating)) / 300.0f) * (won_mod - chance);
    }
    else if (rbg && won && ownRating < 1500)
        mod = 192.0f * (won_mod - chance);
    else
    {
        float winbonus = 0.0f;
        if (won && chance < 0.4f)
            winbonus = (1.0f - chance / 0.4f) * 24.0f;

        mod = (24.0f + winbonus) * (won_mod - chance);
    }

    // in any way should be decrase
    if (!won && mod == 0.0f && ownRating > 0)
        return -1.0f;

    if (won)
        return static_cast<int>(RoundingFloatValue(std::max(mod, 1.f)));

    return static_cast<int>(RoundingFloatValue(mod));

}

int GetMatchmakerRatingMod(int ownRating, int opponentRating, bool won )
{
    // 'Chance' calculation - to beat the opponent
    // This is a simulation. Not much info on how it really works
    float chance = GetChanceAgainst(ownRating, opponentRating);
    float won_mod = won ? 1.0f : 0.0f;
    float mod = won_mod - chance;

    // Real rating modification
    mod *= 48.0f;

    return static_cast<int>(RoundingFloatValue(mod));
}

void Bracket::SaveStats(SQLTransaction* trans)
{
    int32 index = 0;
    PreparedStatement* stmt;

    switch (m_state)
    {
        case BRACKET_NEW:
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHARACTER_BRACKETS_STATS);
            stmt->setUInt64(index++, m_owner.GetCounter());
            stmt->setUInt8(index++, m_Type);
            stmt->setUInt16(index++, m_rating);
            stmt->setUInt16(index++, values[BRACKET_BEST]);
            stmt->setUInt16(index++, values[BRACKET_WEEK_BEST]);
            stmt->setUInt16(index++, m_mmv);
            stmt->setUInt32(index++, values[BRACKET_SEASON_GAMES]);
            stmt->setUInt32(index++, values[BRACKET_SEASON_WIN]);
            stmt->setUInt32(index++, values[BRACKET_WEEK_GAMES]);
            stmt->setUInt32(index++, values[BRACKET_WEEK_WIN]);
            break;
        case BRACKET_CHANGED:
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_BRACKETS_STATS);
            stmt->setUInt16(index++, m_rating);
            stmt->setUInt16(index++, values[BRACKET_BEST]);
            stmt->setUInt16(index++, values[BRACKET_WEEK_BEST]);
            stmt->setUInt16(index++, m_mmv);
            stmt->setUInt32(index++, values[BRACKET_SEASON_GAMES]);
            stmt->setUInt32(index++, values[BRACKET_SEASON_WIN]);
            stmt->setUInt32(index++, values[BRACKET_WEEK_GAMES]);
            stmt->setUInt32(index++, values[BRACKET_WEEK_WIN]);
            stmt->setUInt64(index++, m_owner.GetCounter());
            stmt->setUInt8(index++, m_Type);
            break;
        default:
            //Do nothing.
            return;
    }

    if (trans)
        (*trans)->Append(stmt);
    else
        CharacterDatabase.Execute(stmt);

    m_state = BRACKET_UNCHANGED;
}

uint16 Bracket::FinishGame(bool win, uint16 opponents_mmv, bool winnerNone /*false*/)
{
    values[BRACKET_SEASON_GAMES]++;
    values[BRACKET_WEEK_GAMES]++;

    if (win)
    {
        values[BRACKET_SEASON_WIN]++;
        values[BRACKET_WEEK_WIN]++;
    }

    m_ratingLastChange = winnerNone ? opponents_mmv * -1 : GetRatingMod(m_rating, opponents_mmv, win);
    m_rating = (m_rating + m_ratingLastChange) < 0 ? 0 : m_rating + m_ratingLastChange;
    m_mmr_lastChage = GetMatchmakerRatingMod(m_mmv, opponents_mmv, win);
    m_mmv = (m_mmv + m_mmr_lastChage) < 0 ? 0 : m_mmv + m_mmr_lastChage;

    if (m_rating > values[BRACKET_WEEK_BEST])
        values[BRACKET_WEEK_BEST] = m_rating;

    if (m_rating > values[BRACKET_BEST])
        values[BRACKET_BEST] = m_rating;

    if (Player* player = ObjectAccessor::FindPlayer(m_owner))
        player->UpdateAchievementCriteria(CRITERIA_TYPE_HIGHEST_PERSONAL_RATING, m_rating, GetSlotByType());

    if (m_state == BRACKET_UNCHANGED)
        m_state = BRACKET_CHANGED;

    SaveStats();

    return m_ratingLastChange;
}

uint16 Bracket::GetSlotByType()
{
    switch (m_Type)
    {
        case MS::Battlegrounds::BracketType::Arena1v1:
            return 1;
        case MS::Battlegrounds::BracketType::Arena2v2:
            return 2;
        case MS::Battlegrounds::BracketType::Arena3v3:
            return 3;
        default:
            break;
    }

    return 0xFF;
}

void Bracket::ResetWeekly()
{
    values[BRACKET_WEEK_BEST_LAST] = values[BRACKET_WEEK_BEST];
    values[BRACKET_WEEK_GAMES] = 0;
    values[BRACKET_WEEK_WIN] = 0;
    values[BRACKET_WEEK_BEST] = 0;
}
