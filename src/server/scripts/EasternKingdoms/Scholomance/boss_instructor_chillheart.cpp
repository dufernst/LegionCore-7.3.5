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
SDName: Boss_instructormalicia
SD%Complete: 100
SDComment:
SDCategory: Scholomance
EndScriptData */

//Working only first phase(Sanmay)

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "scholomance.h"

enum Spells
{ 
    SPELL_ICE_WALL_VISUAL    = 111524, //for trigger
    SPELL_FIRE_BOOK_VISUAL   = 111574, //for trigger
    SPELL_ICE_WRATH          = 111610,
    SPELL_ICE_ZONE           = 111239,
    SPELL_TOUCH_OF_THE_GRAVE = 111606,
    SPELL_WRACK_SOUL         = 111631,
};

//enum Special Creature
//book            = 59707
//book trigger    = 58635
//icewall trgger  = 59929
//vehicle trigger = 58662

class boss_instructor_chillheart : public CreatureScript
{
public:
    boss_instructor_chillheart() : CreatureScript("boss_instructor_chillheart") { }

    struct boss_instructor_chillheartAI : public BossAI
    {
        boss_instructor_chillheartAI(Creature* creature) : BossAI(creature, DATA_INSTRUCTOR)
        {
            InstanceScript* instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        uint32 wrathtimer;
        uint32 zonetimer;
        uint32 touchtimer;
        uint32 wracktimer;
        
        void Reset()
        {
            _Reset();
            me->SetReactState(REACT_DEFENSIVE);
            wrathtimer = 0;
            zonetimer  = 0;
            touchtimer = 0;
            wracktimer = 0;
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            touchtimer = 5000;
            wrathtimer = 8000;
            wracktimer = 12000;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (wracktimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                    DoCast(target, SPELL_WRACK_SOUL);
                wracktimer = 12000;
            }
            else
                wracktimer -= diff;

            if (wrathtimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                    DoCast(target, SPELL_ICE_WRATH);
                wrathtimer = 10000;
            }
            else
                wrathtimer -= diff;

            if (touchtimer <= diff)
            {
                DoCast(me, SPELL_TOUCH_OF_THE_GRAVE);
                touchtimer = 20000;
            }
            else
                touchtimer -= diff;
           
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_instructor_chillheartAI (creature);
    }

};

void AddSC_boss_instructor_chillheart()
{
    new boss_instructor_chillheart();
}
