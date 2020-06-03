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

#include "ArenaScore.h"

ArenaScore::ArenaScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), TeamID(team)
{
}

ArenaTeamScore::ArenaTeamScore() : OldRating(0), NewRating(0), MatchmakerRating(0)
{
}

void ArenaTeamScore::Reset()
{
    OldRating = 0;
    NewRating = 0;
    MatchmakerRating = 0;
}

void ArenaTeamScore::Assign(int32 oldRating, int32 newRating, uint32 matchMakerRating)
{
    OldRating = oldRating;
    NewRating = newRating;
    MatchmakerRating = matchMakerRating;
}
