/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Burning_Steppes
SD%Complete: 0
SDComment:
SDCategory: Burning Steppes
EndScriptData */


#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"

class at_wod_dark_portal : public AreaTriggerScript
{
public:
    at_wod_dark_portal() : AreaTriggerScript("at_wod_dark_portal") { }

    enum spells
    {
        SPELL_TIME_SHIFT = 176111,

        SPELL_ASHRAN_TELE = 176809,
        SPELL_ASHRAN_HORDE = 173143,  //5352.16 Y: -3944.96 Z: 32.7331 O: 3.921144
        SPELL_ASHRAN_ALLIANCE = 167220,
        SPELL_TELE_OUT_ALLIANCE = 167221, //Teleport Out: Alliance / 2308.57 Y: 447.469 Z: 5.11977 O: 2.199202
        SPELL_TELE_OUT_HORDE = 167220,//Teleport Out: Alliance

        SPELL_TELE_INTRO = 167771,
        MOVIE_HORDE = 185,
        MOVIE_ALLIANCE = 187,

        QUEST__A = 35884,
        QUEST__H = 34446,
    };

    bool OnTrigger(Player* player, const AreaTriggerEntry* /*at*/, bool /*enter*/)
    {
        if (player->getLevel() < 90 || player->HasAura(SPELL_TIME_SHIFT))
            return false;       //tele to outlend

        if (player->GetQuestStatus(QUEST__A) == QUEST_STATUS_REWARDED ||
            player->GetQuestStatus(QUEST__H) == QUEST_STATUS_REWARDED)
        {
            player->CastSpell(player, player->GetTeam() == HORDE ? SPELL_TELE_OUT_HORDE : SPELL_TELE_OUT_ALLIANCE);
        }
        else
            player->CastSpell(player, SPELL_TELE_INTRO);

        return true;
    }
};

void AddSC_burning_steppes()
{
    new at_wod_dark_portal();
}
