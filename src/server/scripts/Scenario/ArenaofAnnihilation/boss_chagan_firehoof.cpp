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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "arena_of_annihilation.h"

enum Spells
{
    SPELL_SUMMON_BATU       = 122104,
    SPELL_HAMMER_TIME       = 123974,
    SPELL_TRAILBLAZE        = 123969,
    //Batu
    SPELL_YAK_BASH          = 120077,
    
    //Fire triggers
    SPELL_TRAILBLAZE_AURA   = 123976,
};

enum Events
{
    EVENT_HAMMER_TIME   = 1,
    EVENT_TRAILBLAZE    = 2,
    //Batu
    EVENT_POINT_HOME    = 3,
    EVENT_YAK_BASH      = 4,
};

class boss_chagan_firehoof : public CreatureScript
{
public:
    boss_chagan_firehoof() : CreatureScript("boss_chagan_firehoof") { }

    struct boss_chagan_firehoofAI : public BossAI
    {
        boss_chagan_firehoofAI(Creature* creature) : BossAI(creature, DATA_CHAGAN)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            DoCast(SPELL_SUMMON_BATU);
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_HAMMER_TIME, 14000);
            events.RescheduleEvent(EVENT_TRAILBLAZE, 8000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustSummoned(Creature* summoned) override
        {
            summons.Summon(summoned);
            summoned->AI()->DoAction(true);
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HAMMER_TIME:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_HAMMER_TIME);
                        events.RescheduleEvent(EVENT_HAMMER_TIME, 14000);
                        break;
                    case EVENT_TRAILBLAZE:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            DoCast(target, SPELL_TRAILBLAZE);
                        events.RescheduleEvent(EVENT_TRAILBLAZE, 20000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_chagan_firehoofAI (creature);
    }
};

class npc_chagan_batu : public CreatureScript
{
public:
    npc_chagan_batu() : CreatureScript("npc_chagan_batu") { }

    struct npc_chagan_batuAI : ScriptedAI
    {
        npc_chagan_batuAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->SetReactState(REACT_PASSIVE);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_YAK_BASH, 9000);
        }

        void DoAction(const int32 action) override
        {
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                    pGo->SetGoState(GO_STATE_READY);

                if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                {
                    passenger->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    passenger->ExitVehicle();
                    passenger->HandleEmoteCommand(EMOTE_ONESHOT_BATTLE_ROAR);
                    passenger->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                }
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_YAK_BASH:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_YAK_BASH);
                        events.RescheduleEvent(EVENT_YAK_BASH, 9000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_chagan_batuAI (creature);
    }
};
// 63544
class npc_chagan_trailblaze : public CreatureScript
{
public:
    npc_chagan_trailblaze() : CreatureScript("npc_chagan_trailblaze") { }

    struct npc_chagan_trailblazeAI : ScriptedAI
    {
        npc_chagan_trailblazeAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 FireTimer;
        uint32 orient;

        void Reset() override
        { 
            FireTimer = 0;
            orient = me->GetOrientation();
            
            switch (me->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL))
            {
                case 123968: //Front
                    me->SetOrientation(0 + orient);
                    break;
                case 123970: //Back
                    me->SetOrientation(M_PI + orient);
                    break;
                case 123972: //Right
                    me->SetOrientation(-M_PI/4 + orient);
                    break;
                case 123973: //Left
                    me->SetOrientation(M_PI/4 + orient);
                    break;
            }
            float x, y, z;
            me->GetClosePoint(x, y, z, me->GetObjectSize(), 100.0f);
            me->GetMotionMaster()->MovePoint(1, x, y, z);
        }
        
        void UpdateAI(uint32 diff) override
        {
            if (FireTimer <= diff)
            {
                DoCast(SPELL_TRAILBLAZE_AURA);
                FireTimer = 500;
            }
            else FireTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_chagan_trailblazeAI(creature);
    }
};

void AddSC_boss_chagan_firehoof()
{
    new boss_chagan_firehoof();
    new npc_chagan_batu();
    new npc_chagan_trailblaze();
}
