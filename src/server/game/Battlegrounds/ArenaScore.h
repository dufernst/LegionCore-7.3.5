/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_ARENA_SCORE_H
#define TRINITY_ARENA_SCORE_H

#include "BattlegroundScore.h"

struct ArenaScore : BattlegroundScore
{
    friend class Arena;

protected:
    ArenaScore(ObjectGuid playerGuid, TeamId team);

    void BuildObjectivesBlock(std::vector<int32>& /*stats*/) override { }

    TeamId TeamID;
};

struct ArenaTeamScore
{
    friend class Arena;
    friend class Battleground;

protected:
    ArenaTeamScore();

    virtual ~ArenaTeamScore() = default;

    void Reset();

    void Assign(int32 oldRating, int32 newRating, uint32 matchMakerRating);

    int32 OldRating;
    int32 NewRating;
    uint32 MatchmakerRating;
};

#endif // TRINITY_ARENA_SCORE_H
