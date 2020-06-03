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

#ifndef __BRACKET_H
#define __BRACKET_H

#include "Common.h"
#include "Player.h"

enum BracketState
{
    BRACKET_UNCHANGED   = 0,
    BRACKET_CHANGED     = 1,
    BRACKET_NEW         = 2,
    BRACKET_REMOVED     = 3     //not removed just set count == 0
};

class Bracket
{
public:
    Bracket(ObjectGuid guid, uint8 type);
    ~Bracket() = default;

    void InitStats(uint16 rating, uint16 mmr, uint32 games, uint32 wins, uint32 week_games, uint32 week_wins, uint16 best_week, uint16 best, uint16 bestWeekLast);

    uint16 getRating() const { return m_rating; }
    int16 getRatingLastChange() const { return m_ratingLastChange; }
    uint16 getMMV() const { return m_mmv; }
    int16 getLastMMRChange() const{ return m_mmr_lastChage; }

    void SaveStats(SQLTransaction* trans = nullptr);

    uint16 FinishGame(bool win, uint16 opponents_mmv, bool winnerNone = false);
    uint32 GetBracketInfo(BracketInfoType i) const { return values[i]; }
    uint16 GetSlotByType();

    void SetState(uint8 s) { m_state = s; }

    void ResetWeekly();

private:

    uint32 values[BRACKET_END];                 //used for store data from Player::PLAYER_FIELD_PVP_INFO

    uint16 m_rating;
    int16 m_ratingLastChange;
    uint16 m_mmv;
    int16 m_mmr_lastChage;
    uint8 m_Type;

    ObjectGuid m_owner;
    uint8 m_state;
};

#endif
