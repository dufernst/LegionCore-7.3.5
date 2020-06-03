/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "PassiveAI.h"
#include "BattlegroundIsleOfConquest.h"

// TO-DO: This should be done with SmartAI, but yet it does not correctly support vehicles's AIs.
//        Even adding ReactState Passive we still have issues using SmartAI.

class npc_four_car_garage : public CreatureScript
{
    public:
        npc_four_car_garage() : CreatureScript("npc_four_car_garage") {}

        struct npc_four_car_garageAI : public NullCreatureAI
        {
            npc_four_car_garageAI(Creature* creature) : NullCreatureAI(creature) { }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
            {
                if (apply)
                {
                    uint32 spellId = 0;

                    switch (me->GetEntry())
                    {
                        case NPC_DEMOLISHER:
                            spellId = SPELL_DRIVING_CREDIT_DEMOLISHER;
                            break;
                        case NPC_GLAIVE_THROWER_A:
                        case NPC_GLAIVE_THROWER_H:
                            spellId = SPELL_DRIVING_CREDIT_GLAIVE;
                            break;
                        case NPC_SIEGE_ENGINE_H:
                        case NPC_SIEGE_ENGINE_A:
                            spellId = SPELL_DRIVING_CREDIT_SIEGE;
                            break;
                        case NPC_CATAPULT:
                            spellId = SPELL_DRIVING_CREDIT_CATAPULT;
                            break;
                        default:
                            return;
                    }

                    me->CastSpell(who, spellId, true);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_four_car_garageAI(creature);
        }
};

/*#####
## Bosses Isle Of Conquest (A/H)
## NPC_HIGH_COMMANDER_HALFORD_WYRMBANE = 34924, // Alliance Boss
## NPC_OVERLORD_AGMAR = 34922, // Horde Boss
## Comment: AI Basico segun WOWHEAD, para una principal solucion del exploit de LOS del BG.
##          Futuro Sniffed de blizzard por (Edward). para sonidos textos y reales spells.
#####*/

enum BossSpells
{
    SPELL_MORTAL_STRIKE     = 58460,
    SPELL_CRUSHING_LEAP     = 68506,
    SPELL_DAGGER_THROW      = 67280,
    SPELL_RAGE              = 66776
};

enum BossEvents
{
    EVENT_MORTAL_STRIKE     = 1,
    EVENT_DAGGER_THROW      = 2,
    EVENT_CRUSHING_LEAP     = 3,
    EVENT_CHECK_ROOM        = 4
};

class boss_isle_of_conquest : public CreatureScript
{
    public:
        boss_isle_of_conquest() : CreatureScript("boss_isle_of_conquest") { }

        struct boss_isle_of_conquestAI : public ScriptedAI
        {
            boss_isle_of_conquestAI(Creature *creature) : ScriptedAI(creature) {}

            void Reset() override
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_MORTAL_STRIKE, 8000);
                _events.ScheduleEvent(EVENT_DAGGER_THROW, 2000);
                _events.ScheduleEvent(EVENT_CRUSHING_LEAP, 6000);
                _events.ScheduleEvent(EVENT_CHECK_ROOM, 5000);
                me->RemoveAurasDueToSpell(SPELL_RAGE);
            }

            void EnterCombat(Unit * who) override
            {
                if (!me->IsWithinLOSInMap(who))
                    EnterEvadeMode();
            }
            
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MORTAL_STRIKE:
                            DoCastVictim(SPELL_MORTAL_STRIKE);
                            _events.ScheduleEvent(EVENT_MORTAL_STRIKE, urand(10000,20000));
                            break;
                        case EVENT_DAGGER_THROW:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_DAGGER_THROW);
                            _events.ScheduleEvent(EVENT_DAGGER_THROW, urand(7000,12000));
                            break;
                        case EVENT_CRUSHING_LEAP:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_CRUSHING_LEAP);
                            _events.ScheduleEvent(EVENT_DAGGER_THROW, urand(12000,16000));
                            break;
                        case EVENT_CHECK_ROOM:
                        {
                            float x = 0.0f, y = 0.0f;

                            me->GetPosition(x, y);

                            float z = me->GetPositionZ();
                            float home_z = me->GetHomePosition().GetPositionZ();

                            if (z > home_z + 10.0f)
                            {
                                EnterEvadeMode();
                                return;
                            }
                            
                            switch (me->GetEntry())
                            {
                                case NPC_OVERLORD_AGMAR:
                                {
                                    if (x > 1348.0f || x < 1283.0f || y < -800.0f || y > -730.0f)
                                    {
                                        if (!me->HasAura(SPELL_RAGE))
                                            DoCast(me, SPELL_RAGE);
                                    }
                                    else
                                        me->RemoveAurasDueToSpell(SPELL_RAGE);
                                    break;
                                }
                                case NPC_HIGH_COMMANDER_HALFORD_WYRMBANE:
                                {
                                    if (x > 288.0f || x < 216.0f || y < -863.0f || y > -800.0f)
                                    {
                                        if (!me->HasAura(SPELL_RAGE))
                                            DoCast(me, SPELL_RAGE);
                                    }
                                    else
                                        me->RemoveAurasDueToSpell(SPELL_RAGE);
                                    break;
                                }
                            }
                            
                            _events.ScheduleEvent(EVENT_CHECK_ROOM, 2000);
                            break;
                        }
                    }
                }

                DoMeleeAttackIfReady();
            }
            
        private:
            EventMap _events;
        };

        CreatureAI *GetAI(Creature *creature) const override
        {
            return new boss_isle_of_conquestAI(creature);
        }
};

void AddSC_isle_of_conquest()
{
    new npc_four_car_garage();
    new boss_isle_of_conquest();
}
