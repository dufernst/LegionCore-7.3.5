/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "BattlegroundScore.h"

uint32 BattlegroundScore::GetScore(uint32 type) const
{
    switch (type)
    {
        case SCORE_KILLING_BLOWS:
            return KillingBlows;
        case SCORE_DEATHS:
            return Deaths;
        case SCORE_HONORABLE_KILLS:
            return HonorableKills;
        case SCORE_BONUS_HONOR:
            return BonusHonor;
        case SCORE_DAMAGE_DONE:
            return DamageDone;
        case SCORE_HEALING_DONE:
            return HealingDone;
    }
    return 0;
}

BattlegroundScore::BattlegroundScore(ObjectGuid playerGuid, TeamId team) : PlayerGuid(playerGuid), TeamID(MS::Battlegrounds::GetOtherTeamID(team)), KillingBlows(0), Deaths(0), HonorableKills(0), BonusHonor(0), DamageDone(0), HealingDone(0)
{
}

void BattlegroundScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
        case SCORE_KILLING_BLOWS:
            KillingBlows += value;
            break;
        case SCORE_DEATHS:
            Deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:
            HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:
            BonusHonor += value;
            break;
        case SCORE_DAMAGE_DONE:
            DamageDone += value;
            break;
        case SCORE_HEALING_DONE:
            HealingDone += value;
            break;
        default:
            ASSERT(false && "Not implemented Battleground score type!");
            break;
    }
}
