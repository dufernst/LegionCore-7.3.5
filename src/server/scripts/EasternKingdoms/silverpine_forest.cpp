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
SDName: Silverpine_Forest
SD%Complete: 100
SDComment: Quest support: 435
SDCategory: Silverpine Forest
EndScriptData */

/* ContentData
npc_deathstalker_erland
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"

/*######
## npc_deathstalker_erland
######*/

enum eErland
{
    SAY_QUESTACCEPT     = 22,
    SAY_START           = 23,
    SAY_AGGRO           = 24,
    SAY_LAST            = 25,

    SAY_THANKS          = 26,
    SAY_RANE            = 27,
    SAY_ANSWER          = 28,
    SAY_MOVE_QUINN      = 29,

    SAY_GREETINGS       = 30,
    SAY_QUINN           = 31,
    SAY_ON_BYE          = 32,

    QUEST_ESCORTING     = 435,
    NPC_RANE            = 1950,
    NPC_QUINN           = 1951
};

class npc_deathstalker_erland : public CreatureScript
{
public:
    npc_deathstalker_erland() : CreatureScript("npc_deathstalker_erland") { }

    struct npc_deathstalker_erlandAI : public npc_escortAI
    {
        npc_deathstalker_erlandAI(Creature* creature) : npc_escortAI(creature) {}

        void WaypointReached(uint32 waypointId)
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    Talk(SAY_START, player->GetGUID());
                    break;
                case 13:
                    Talk(SAY_LAST, player->GetGUID());
                    player->GroupEventHappens(QUEST_ESCORTING, me);
                    break;
                case 14:
                    Talk(SAY_THANKS, player->GetGUID());
                    break;
                case 15:
                    if (auto Rane = me->FindNearestCreature(NPC_RANE, 20))
                        Rane->AI()->Talk(SAY_RANE);
                    break;
                case 16:
                    Talk(SAY_ANSWER);
                    break;
                case 17:
                    Talk(SAY_MOVE_QUINN);
                    break;
                case 24:
                    Talk(SAY_GREETINGS);
                    break;
                case 25:
                    if (auto Quinn = me->FindNearestCreature(NPC_QUINN, 20))
                        Quinn->AI()->Talk(SAY_QUINN);
                    break;
                case 26:
                    Talk(SAY_ON_BYE);
                    break;
            }
        }

        void Reset() {}

        void EnterCombat(Unit* who)
        {
            Talk(SAY_AGGRO, who->GetGUID());
        }
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_ESCORTING)
        {
            creature->AI()->Talk(SAY_QUESTACCEPT, player->GetGUID());

            if (npc_escortAI* pEscortAI = CAST_AI(npc_deathstalker_erland::npc_deathstalker_erlandAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_deathstalker_erlandAI(creature);
    }
};

void AddSC_silverpine_forest()
{
    new npc_deathstalker_erland();
}
