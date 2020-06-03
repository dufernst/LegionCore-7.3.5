/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
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

#include "LFGMgr.h"
#include "Group.h"
/*
UPDATE  `creature_template` SET  `faction` =  '14', `ScriptName` =  'boss_coren_direbrew' WHERE `entry` =23872 LIMIT 1 ;
UPDATE creature SET `spawntimesecs` = 604800 WHERE `id` =23872;
UPDATE  `quest_template` SET  `SpecialFlags` =  '9' WHERE  `entry` =25483 LIMIT 1 ;
*/
class boss_coren_direbrew : public CreatureScript
{
    public:
        boss_coren_direbrew() : CreatureScript("boss_coren_direbrew") { }

        struct boss_coren_direbrewAI : public BossAI
        {
            boss_coren_direbrewAI(Creature* creature) : BossAI(creature, 10) {}

            void Reset()
            {
            }

            void JustDied(Unit* /*victim*/)
            {
                if (instance)
                {
                    // LFG reward
                    Map::PlayerList const& players = instance->instance->GetPlayers();
                    LFGDungeonsEntry const* dungeon = sLfgDungeonsStore.LookupEntry(287); //lfg id

                    if (!players.isEmpty())
                    {
                        if (Group* group = players.begin()->getSource()->GetGroup())
                        if (group->isLFGGroup())
                            sLFGMgr->FinishDungeon(group->GetGUID(), dungeon->ID);
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_coren_direbrewAI(creature);
        }
};

void AddSC_coren_direbrew()
{
    new boss_coren_direbrew();
}
