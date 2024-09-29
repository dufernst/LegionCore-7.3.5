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

#ifndef TRINITY_FORMULAS_H
#define TRINITY_FORMULAS_H

#include "GameTables.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "World.h"

namespace Trinity
{
    inline uint32 GetExpansionForLevel(uint32 level)
    {
        if (level < 60)
            return EXPANSION_CLASSIC;
        if (level < 70)
            return EXPANSION_THE_BURNING_CRUSADE;
        if (level < 80)
            return EXPANSION_WRATH_OF_THE_LICH_KING;
        if (level < 85)
            return EXPANSION_CATACLYSM;
        if (level < 90)
            return EXPANSION_MISTS_OF_PANDARIA;
        if (level < 100)
            return EXPANSION_WARLORDS_OF_DRAENOR;
        if (level < 110)
            return EXPANSION_LEGION;
        return CURRENT_EXPANSION;
    }

    namespace Honor
    {
        inline float hk_honor_at_level_f(uint8 level, float multiplier = 1.0f)
        {
            float honor = multiplier * level * 1.55f;
            sScriptMgr->OnHonorCalculation(honor, level, multiplier);
            return honor * 2.4; // http://www.wowwiki.com/Honorable_kill#Honorable_kills 1 old points = 0.024 new points
        }

        inline uint32 hk_honor_at_level(uint8 level, float multiplier = 1.0f)
        {
            return uint32(ceil(hk_honor_at_level_f(level, multiplier)));
        }
    }
    namespace XP
    {
        inline uint8 GetGrayLevel(uint8 pl_level)
        {
            uint8 level;

            if (pl_level <= 5)
                level = 0;
            else if (pl_level <= 39)
                level = pl_level - 5 - pl_level / 10;
            else if (pl_level <= 59)
                level = pl_level - 1 - pl_level / 5;
            else
                level = pl_level - 9;

            sScriptMgr->OnGrayLevelCalculation(level, pl_level);
            return level;
        }

        inline XPColorChar GetColorCode(uint8 pl_level, uint8 mob_level)
        {
            XPColorChar color;

            if (mob_level >= pl_level + 5)
                color = XP_RED;
            else if (mob_level >= pl_level + 3)
                color = XP_ORANGE;
            else if (mob_level >= pl_level - 2)
                color = XP_YELLOW;
            else if (mob_level > GetGrayLevel(pl_level))
                color = XP_GREEN;
            else
                color = XP_GRAY;

            sScriptMgr->OnColorCodeCalculation(color, pl_level, mob_level);
            return color;
        }

        inline uint8 GetZeroDifference(uint8 pl_level)
        {
            uint8 diff;

            if (pl_level < 4)
                diff = 5;
            else if (pl_level < 10)
                diff = 6;
            else if (pl_level < 12)
                diff = 7;
            else if (pl_level < 16)
                diff = 8;
            else if (pl_level < 20)
                diff = 9;
            else if (pl_level < 30)
                diff = 11;
            else if (pl_level < 40)
                diff = 12;
            else if (pl_level < 45)
                diff = 13;
            else if (pl_level < 50)
                diff = 14;
            else if (pl_level < 55)
                diff = 15;
            else if (pl_level < 60)
                diff = 16;
            else
                diff = 17;

            sScriptMgr->OnZeroDifferenceCalculation(diff, pl_level);
            return diff;
        }

        inline uint32 BaseGain(uint8 pl_level, uint8 mob_level)
        {
            uint32 baseGain;

            GtXpEntry const* xpPlayer = sXpGameTable.GetRow(pl_level);
            GtXpEntry const* xpMob = sXpGameTable.GetRow(mob_level);

            if (mob_level >= pl_level)
                baseGain = uint32(round(xpPlayer->PerKill * (1 + 0.05f * std::min(mob_level - pl_level, 4))));
            else
            {
                if (mob_level > GetGrayLevel(pl_level))
                    baseGain = uint32(round(xpMob->PerKill * ((1 - ((pl_level - mob_level) / float(GetZeroDifference(pl_level)))) * (xpMob->Divisor / xpPlayer->Divisor))));
                else
                    baseGain = 0;
            }

            sScriptMgr->OnBaseGainCalculation(baseGain, pl_level, mob_level);
            return baseGain;
        }

        inline uint32 Gain(Player* player, Unit* u)
        {
            Creature* creature = u->ToCreature();
            uint32 gain;

            if (u->IsCreature() && (creature->isTotem() || creature->isPet() ||
                creature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_XP_AT_KILL) ||
                creature->GetCreatureTemplate()->Type == CREATURE_TYPE_CRITTER)
                gain = 0;
            else
            {
                gain = BaseGain(player->getLevel(), u->getLevelForXPReward(player));

                if (gain != 0 && u->IsCreature() && creature->isElite())
                {
                    // Players get only 10% xp for killing creatures of lower expansion levels than himself
                    if ((uint32(creature->GetCreatureTemplate()->RequiredExpansion) < GetExpansionForLevel(player->getLevel())))
                        gain = uint32(round(gain / 10.0f));

                    // Elites in instances have a 2.75x XP bonus instead of the regular 2x world bonus.
                    if (u->GetMap() && u->GetMap()->IsDungeon())
                        gain = uint32(gain * 2.75f);
                    else
                        gain *= 2;
                }

                float KillXpRate = 1.0f;
                if (float rate = player->GetSession()->GetPersonalXPRate())
                    KillXpRate = rate;
                else if(player->GetPersonnalXpRate())
                    KillXpRate = player->GetPersonnalXpRate();
                else
                    KillXpRate = GetRateInfo(creature);

                gain = uint32(gain * KillXpRate);
            }

            sScriptMgr->OnGainCalculation(gain, player, u);
            return gain;
        }

        inline float xp_in_group_rate(uint32 count, bool isRaid)
        {
            float rate;

            if (isRaid)
            {
                // FIXME: Must apply decrease modifiers depending on raid size.
                rate = 1.0f;
            }
            else
            {
                switch (count)
                {
                    case 0:
                    case 1:
                    case 2:
                        rate = 1.0f;
                        break;
                    case 3:
                        rate = 1.166f;
                        break;
                    case 4:
                        rate = 1.3f;
                        break;
                    case 5:
                    default:
                        rate = 1.4f;
                }
            }

            sScriptMgr->OnGroupRateCalculation(rate, count, isRaid);
            return rate;
        }
    }
}

#endif
