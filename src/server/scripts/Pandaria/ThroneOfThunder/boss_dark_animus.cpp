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

#include "throne_of_thunder.h"

enum eSpells
{
    //Dark Animus
    SPELL_TOUCH_OF_THE_ANIMUS  = 138659, 
    SPELL_ANIMA_FONT           = 138697, 
    SPELL_INTERRUPTING_JOLT    = 138763, 
    SPELL_FULL_POWER           = 138749,
    //Massive anima golem
    SPELL_EXPLOSIVE_SLAM       = 138569,
};

enum sEvents
{
    //Dark Animus
    EVENT_TOUCH_OF_THE_ANIMUS  = 1,
    EVENT_ANIMA_FONT           = 2,
    EVENT_INTERRUPTING_JOLT    = 3,
    EVENT_FULL_POWER           = 4,
    //Massive anima golem
    EVENT_EXPLOSIVE_SLAM       = 5,
};

enum Actions
{
    ACTION_FULL_POWER          = 1,
};

class boss_dark_animus : public CreatureScript
{
    public:
        boss_dark_animus() : CreatureScript("boss_dark_animus") {}

        struct boss_dark_animusAI : public BossAI
        {
            boss_dark_animusAI(Creature* creature) : BossAI(creature, DATA_DARK_ANIMUS)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            bool fullpower;

            void Reset()
            {
                _Reset();
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOUCH_OF_THE_ANIMUS);
                fullpower = false;
                me->SetReactState(REACT_DEFENSIVE);
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                events.RescheduleEvent(EVENT_TOUCH_OF_THE_ANIMUS, 20000);
                events.RescheduleEvent(EVENT_ANIMA_FONT, 26000);
                events.RescheduleEvent(EVENT_INTERRUPTING_JOLT, 30000);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_FULL_POWER && !fullpower)
                {
                    fullpower = true;
                    me->MonsterTextEmote("FULL POWER ACTIVE", ObjectGuid::Empty, true);
                    events.RescheduleEvent(EVENT_FULL_POWER, 4000);
                }
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOUCH_OF_THE_ANIMUS);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_TOUCH_OF_THE_ANIMUS:
                        {
                            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                            if (!ThreatList.empty())
                            {
                                for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); itr++)
                                {
                                    if (itr == ThreatList.begin())
                                        continue;
                                    
                                    if (Unit* target = me->GetUnit(*me, (*itr)->getUnitGuid()))
                                    {
                                        if (!target->HasAura(SPELL_TOUCH_OF_THE_ANIMUS))
                                        {
                                            DoCast(target, SPELL_TOUCH_OF_THE_ANIMUS);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_TOUCH_OF_THE_ANIMUS, 20000);
                        break;
                    case EVENT_ANIMA_FONT:
                        {
                            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                            if (!ThreatList.empty())
                            {
                                for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); itr++)
                                {
                                    if (itr == ThreatList.begin())
                                        continue;
                                    
                                    if (Unit* target = me->GetUnit(*me, (*itr)->getUnitGuid()))
                                    {
                                        if (target->HasAura(SPELL_TOUCH_OF_THE_ANIMUS))
                                            DoCast(target, SPELL_ANIMA_FONT);
                                    }
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_ANIMA_FONT, 26000);
                        break;
                    case EVENT_INTERRUPTING_JOLT:
                        DoCastAOE(SPELL_INTERRUPTING_JOLT);
                        events.RescheduleEvent(SPELL_INTERRUPTING_JOLT, 30000);
                        break;
                    case EVENT_FULL_POWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            DoCast(target, SPELL_FULL_POWER);
                        events.RescheduleEvent(EVENT_FULL_POWER, 4000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_dark_animusAI(creature);
        }
};

class npc_massive_anima_golem : public CreatureScript
{
    public:
        npc_massive_anima_golem() : CreatureScript("npc_massive_anima_golem") {}

        struct npc_massive_anima_golemAI : public ScriptedAI
        {
            npc_massive_anima_golemAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events;

            void Reset()
            {
                events.Reset();
                me->SetReactState(REACT_DEFENSIVE);
            }

            void EnterCombat(Unit* who)
            {
                if (instance)
                    instance->SetBossState(DATA_DARK_ANIMUS, IN_PROGRESS);
                DoZoneInCombat(me, 150.0f);
                events.RescheduleEvent(EVENT_EXPLOSIVE_SLAM, 10000);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (instance)
                {
                    if (Creature* animus = me->GetCreature(*me, instance->GetGuidData(NPC_DARK_ANIMUS)))
                        animus->AI()->DoAction(ACTION_FULL_POWER);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_EXPLOSIVE_SLAM)
                    {
                        if (me->getVictim())
                            DoCast(me->getVictim(), SPELL_EXPLOSIVE_SLAM);
                        events.RescheduleEvent(EVENT_EXPLOSIVE_SLAM, 25000);
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_massive_anima_golemAI(creature);
        }
};

class npc_large_anima_golem : public CreatureScript
{
    public:
        npc_large_anima_golem() : CreatureScript("npc_large_anima_golem") {}

        struct npc_large_anima_golemAI : public ScriptedAI
        {
            npc_large_anima_golemAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }

            InstanceScript* instance;

            void Reset(){}
            
            void EnterCombat(Unit* who){}
            
            void JustDied(Unit* /*killer*/){}
            
            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_large_anima_golemAI(creature);
        }
};

class npc_anima_golem : public CreatureScript
{
    public:
        npc_anima_golem() : CreatureScript("npc_anima_golem") {}

        struct npc_anima_golemAI : public ScriptedAI
        {
            npc_anima_golemAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }

            InstanceScript* instance;

            void Reset(){}
            
            void EnterCombat(Unit* who){}
            
            void JustDied(Unit* /*killer*/){}
            
            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_anima_golemAI(creature);
        }
};


void AddSC_boss_dark_animus()
{
    new boss_dark_animus();
    new npc_massive_anima_golem();
    new npc_large_anima_golem();
    new npc_anima_golem();
}
