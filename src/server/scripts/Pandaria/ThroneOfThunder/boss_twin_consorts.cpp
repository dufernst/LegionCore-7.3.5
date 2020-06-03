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
    SPELL_BEAST_OF_NIGHTMARE_T_A  = 137341, 
    SPELL_TEARS_OF_SUN            = 137405,
    SPELL_FAN_OF_FLAME            = 137408,
    SPELL_LIGHT_OF_DAY            = 137401,
    SPELL_ICE_COMET               = 137418,
};

enum eEvents
{
    EVENT_INTRO_DAY               = 1,
    EVENT_INTRO_TWILIGHT          = 2,
    EVENT_NIGHTMARE               = 3,
    EVENT_TEARS_OF_SUN            = 4,
    EVENT_TEARS_OF_SUN_END        = 5,
    EVENT_FAN_OF_FLAME            = 6,
    EVENT_LIGHT_OF_DAY            = 7,
    EVENT_ICE_COMET               = 8,
    EVENT_ICE_COMET_END           = 9,
};

enum sAction
{
    ACTION_INTRO_DAY              = 1,
    ACTION_INTRO_TWILIGHT         = 2,
};

const uint32 twinEntry[2] = 
{
    NPC_SULIN,
    NPC_LULIN,
};

struct generic_boss_twin_consortsAI : ScriptedAI
{
public:
    generic_boss_twin_consortsAI(Creature* creature) : ScriptedAI(creature){}

    void TwinsReset(InstanceScript* instance, Creature* caller, uint32 callerEntry)
    {
        if (instance && caller)
        {
            for (uint8 n = 0; n < 2; n++)
            {
                if (Creature* twin = caller->GetCreature(*caller, instance->GetGuidData(twinEntry[n])))
                {
                    if (callerEntry != twinEntry[n])
                    {
                        if (!twin->isAlive())
                        {
                            twin->Respawn();
                            twin->GetMotionMaster()->MoveTargetedHome();
                        }
                        else if (twin->isAlive() && twin->isInCombat())
                            twin->AI()->EnterEvadeMode();
                    }
                }
            }
            instance->SetBossState(DATA_TWIN_CONSORTS, NOT_STARTED);
        }
    }

    void TwinsEnterCombat(InstanceScript* instance, Creature* caller, uint32 callerEntry)
    {
        if (instance && caller)
        {
            switch (callerEntry)
            {
            case NPC_SULIN:
            {
                caller->AI()->DoZoneInCombat(caller, 150.0f);
                caller->AttackStop();
                if (caller->IsVisible())
                    caller->SetVisible(false);
                if (Creature* l = caller->GetCreature(*caller, instance->GetGuidData(NPC_LULIN)))
                {
                    if (l->isAlive() && !l->isInCombat())
                        l->AI()->DoZoneInCombat(l, 150.0f);
                }
            }
            break;
            case NPC_LULIN:
            {
                if (Creature* s = caller->GetCreature(*caller, instance->GetGuidData(NPC_SULIN)))
                {
                    if (s->isAlive() && !s->isInCombat())
                    {
                        s->AI()->DoZoneInCombat(s, 150.0f);
                        s->AttackStop();
                        if (s->IsVisible())
                            s->SetVisible(false);
                    }
                }
            }
            break;
            }
            instance->SetBossState(DATA_TWIN_CONSORTS, IN_PROGRESS);
        }
    }

    void TwinsDoneCheck(InstanceScript* instance, Creature* caller, uint32 callerEntry)
    {
        if (instance && caller)
        {
            uint8 donecount = 0;
            for (uint8 n = 0; n < 2; n++)
            {
                if (Creature* twin = caller->GetCreature(*caller, instance->GetGuidData(twinEntry[n])))
                {
                    if (!twin->isAlive())
                        donecount++;
                }
            }
            if (donecount == 2)
                instance->SetBossState(DATA_TWIN_CONSORTS, DONE);
            else
                caller->RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }
    }
};

class boss_twin_consorts : public CreatureScript
{
    public:
        boss_twin_consorts() : CreatureScript("boss_twin_consorts") {}

        struct boss_twin_consortsAI : public generic_boss_twin_consortsAI
        {
            boss_twin_consortsAI(Creature* creature) : generic_boss_twin_consortsAI(creature), summon(me)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            SummonList summon;
            EventMap events;

            void Reset()
            {
                events.Reset();
                summon.DespawnAll();
                me->SetVisible(true);
                if (instance)
                {
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BEAST_OF_NIGHTMARE_T_A);
                    TwinsReset(instance, me, me->GetEntry());
                }
                switch (me->GetEntry())
                {
                case NPC_SULIN:
                    me->SetReactState(REACT_PASSIVE);
                    break;
                case NPC_LULIN:
                    me->SetReactState(REACT_DEFENSIVE);
                    break;
                }
            }

            void EnterCombat(Unit* who)
            {
                TwinsEnterCombat(instance, me, me->GetEntry());
                switch (me->GetEntry())
                {
                case NPC_LULIN:
                    events.RescheduleEvent(EVENT_INTRO_DAY,   180000);
                    events.RescheduleEvent(EVENT_NIGHTMARE,    50000);
                    break;
                case NPC_SULIN:
                    events.RescheduleEvent(EVENT_TEARS_OF_SUN, 10000);
                    break;
                
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_INTRO_DAY:
                    //Proc Sulin
                    events.Reset();
                    me->SetVisible(true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                    events.RescheduleEvent(EVENT_FAN_OF_FLAME, 30000);
                    events.RescheduleEvent(EVENT_LIGHT_OF_DAY,  8000);
                    break;
                case ACTION_INTRO_TWILIGHT:
                    //Proc Lulin
                    events.Reset();
                    me->SetVisible(true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                    break;
                }
            }

            void JustSummoned(Creature* summons)
            {
                summon.Summon(summons);
            }

            void JustDied(Unit* /*killer*/)
            {
                TwinsDoneCheck(instance, me, me->GetEntry());
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_INTRO_DAY:
                        if (instance)
                        {
                            if (Creature* s = me->GetCreature(*me, instance->GetGuidData(NPC_SULIN)))
                            {
                                if (s->isAlive())
                                {
                                    events.Reset();
                                    me->AttackStop();
                                    me->SetReactState(REACT_PASSIVE);
                                    me->SetVisible(false);
                                    events.RescheduleEvent(EVENT_ICE_COMET, 21000);
                                    s->AI()->DoAction(ACTION_INTRO_DAY);
                                }
                            }
                        }
                        events.RescheduleEvent(EVENT_INTRO_TWILIGHT, 180000);
                        break;
                    case EVENT_ICE_COMET:
                        me->SetVisible(true);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                            DoCast(target, SPELL_ICE_COMET);
                        events.RescheduleEvent(EVENT_ICE_COMET_END, 1000);
                        break;
                    case EVENT_ICE_COMET_END:
                        me->SetVisible(false);
                        events.RescheduleEvent(EVENT_ICE_COMET, 21000);
                        break;
                    case EVENT_INTRO_TWILIGHT:
                        DoAction(ACTION_INTRO_TWILIGHT);
                        break;
                    case EVENT_NIGHTMARE:
                        if (me->getVictim())
                        {
                            if (Creature* beast = me->SummonCreature(NPC_BEAST_OF_NIGHTMARES, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()))
                            {
                                beast->AddAura(SPELL_BEAST_OF_NIGHTMARE_T_A, me->getVictim());
                                beast->AddThreat(me->getVictim(), 5000000.0f);
                                beast->AI()->AttackStart(me->getVictim());
                            }
                        }
                        break;
                    case EVENT_TEARS_OF_SUN:
                        me->SetVisible(true);
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                            DoCast(target, SPELL_TEARS_OF_SUN);
                        events.RescheduleEvent(EVENT_TEARS_OF_SUN_END, 1000);
                        break;
                    case EVENT_TEARS_OF_SUN_END:
                        me->SetVisible(false);
                        events.RescheduleEvent(EVENT_TEARS_OF_SUN, 40000);
                        break;
                    case EVENT_FAN_OF_FLAME:
                        if (me->getVictim())
                            DoCast(me->getVictim(), SPELL_FAN_OF_FLAME);
                        events.RescheduleEvent(EVENT_FAN_OF_FLAME, 12000);
                        break;
                    case EVENT_LIGHT_OF_DAY:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                            DoCast(target, SPELL_LIGHT_OF_DAY);
                        events.RescheduleEvent(EVENT_LIGHT_OF_DAY, 8000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_twin_consortsAI(creature);
        }
};

class npc_beast_of_nightmare : public CreatureScript
{
    public:
        npc_beast_of_nightmare() : CreatureScript("npc_beast_of_nightmare") {}

        struct npc_beast_of_nightmareAI : public ScriptedAI
        {
            npc_beast_of_nightmareAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;

            void Reset(){}
            
            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (!attacker->HasAura(SPELL_BEAST_OF_NIGHTMARE_T_A))
                    damage = 0;
            }

            void JustDied(Unit* killer)
            {
                killer->RemoveAurasDueToSpell(SPELL_BEAST_OF_NIGHTMARE_T_A);
                me->DespawnOrUnsummon(1000);
            }

            void UpdateAI(uint32 diff)
            {    
                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_beast_of_nightmareAI(creature);
        }
};

void AddSC_boss_twin_consorts()
{
    new boss_twin_consorts();
    new npc_beast_of_nightmare();
}
