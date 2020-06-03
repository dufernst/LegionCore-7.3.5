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

enum Says
{
    //Maki
    SAY_WHIRLPOOL   = 0,
};

enum Spells
{
    //Satay
    SPELL_SPEED_OF_THE_JINJA    = 125510,
    SPELL_SATAY_THROW           = 125517,

    //Kobo
    SPELL_NIMBUS                = 125581,
    SPELL_CIRCLE                = 125583,
    SPELL_FLYING_SERPENT_KICK   = 127806,
    SPELL_CYCLONE_KICK          = 125579,
    SPELL_TWISTER               = 131693,

    //Maki
    SPELL_CRASHING_SLASH        = 125568,
    SPELL_WHIRLPOOL             = 125564,
    SPELL_SAUROK_WATERFALL      = 125563,
};

enum Events
{
    EVENT_POINT_HOME        = 1,
    //Satay
    EVENT_SPEED_JINJA       = 2,
    EVENT_THROW             = 3,
    //Kobo
    EVENT_NIMBUS            = 4,
    EVENT_POINT_1           = 5,
    EVENT_POINT_2           = 6,
    EVENT_POINT_3           = 7,
    EVENT_POINT_4           = 8,
    EVENT_POINT_5           = 9,
    EVENT_FLYING_KICK       = 10,
    EVENT_CYCLONE_KICK      = 11,
    //Maki
    EVENT_CRASHING_SLASH    = 12,
    EVENT_WHIRLPOOL         = 13,
};

const Position spawnPos[2] = 
{
    {3727.38f, 541.86f, 639.69f, 6.15f},
    {3795.37f, 533.59f, 639.0f, 6.18f},
};

const Position CirclePos[4] =
{
    {3793.33f, 518.47f, 651.69f},
    {3781.26f, 535.27f, 651.69f},
    {3797.82f, 548.70f, 651.69f},
    {3810.26f, 531.93f, 651.69f},
};

const Position WaterPos[7] =
{
    {3820.97f, 499.60f, 647.64f, 2.20f},
    {3841.55f, 527.54f, 647.64f, 3.02f},
    {3754.85f, 564.94f, 647.64f, 5.62f},
    {3828.93f, 559.35f, 647.64f, 3.82f},
    {3801.23f, 578.68f, 647.64f, 4.60f},
    {3758.01f, 511.44f, 647.64f, 0.52f},
    {3791.59f, 485.88f, 647.64f, 1.46f},
};

class npc_scenario_gurgthock : public CreatureScript
{
public:
    npc_scenario_gurgthock() : CreatureScript("npc_scenario_gurgthock") { }

    struct npc_scenario_gurgthockAI : ScriptedAI
    {
        npc_scenario_gurgthockAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset() override {}

        void DoAction (const int32 action) override
        {
            switch (action)
            {
                case ACTION_1:
                    events.RescheduleEvent(EVENT_1, 2000);
                    break;
                case ACTION_2:
                    events.RescheduleEvent(EVENT_4, 5000);
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        Talk(SAY_START_EVENT_1);
                        events.RescheduleEvent(EVENT_2, 4000);
                        break;
                    case EVENT_2:
                        Talk(SAY_START_EVENT_2);
                        events.RescheduleEvent(EVENT_3, 5000);
                        break;
                    case EVENT_3:
                        Talk(SAY_START_EVENT_3);
                        break;
                    case EVENT_4:
                        Talk(SAY_SCENARIO_FINISH);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_scenario_gurgthockAI (creature);
    }
};

class boss_satay_byu : public CreatureScript
{
public:
    boss_satay_byu() : CreatureScript("boss_satay_byu") { }

    struct boss_satay_byuAI : public BossAI
    {
        boss_satay_byuAI(Creature* creature) : BossAI(creature, DATA_FINAL_STAGE)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            me->setPowerType(POWER_ENERGY);
            me->SetMaxPower(POWER_ENERGY, 100);
            me->SetPower(POWER_ENERGY, 100);
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_SPEED_JINJA, 8000);
            events.RescheduleEvent(EVENT_THROW, 2000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* gurgthock = instance->instance->GetCreature(instance->GetGuidData(NPC_GURGTHOCK)))
                gurgthock->AI()->Talk(SAY_SATAY_END);
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                    pGo->SetGoState(GO_STATE_READY);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_SPEED_OF_THE_JINJA))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_THROW:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_SATAY_THROW);
                        events.RescheduleEvent(EVENT_THROW, 2000);
                        break;
                    case EVENT_SPEED_JINJA:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_SPEED_OF_THE_JINJA);
                        events.RescheduleEvent(EVENT_SPEED_JINJA, 8000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_satay_byuAI (creature);
    }
};

class boss_cloudbender_kobo : public CreatureScript
{
public:
    boss_cloudbender_kobo() : CreatureScript("boss_cloudbender_kobo") { }

    struct boss_cloudbender_koboAI : public BossAI
    {
        boss_cloudbender_koboAI(Creature* creature) : BossAI(creature, DATA_FINAL_STAGE)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_NIMBUS, 20000);
            events.RescheduleEvent(EVENT_FLYING_KICK, 10000);
            events.RescheduleEvent(EVENT_CYCLONE_KICK, 16000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* gurgthock = instance->instance->GetCreature(instance->GetGuidData(NPC_GURGTHOCK)))
                gurgthock->AI()->Talk(SAY_KOBO_END);
            _JustDied();
            summons.DespawnAll();
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case 1:
                    if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                        pGo->SetGoState(GO_STATE_READY);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                case 2:
                    DoCast(SPELL_NIMBUS);
                    events.RescheduleEvent(EVENT_POINT_1, 0);
                    break;
                case 3:
                    events.RescheduleEvent(EVENT_POINT_2, 0);
                    break;
                case 4:
                    events.RescheduleEvent(EVENT_POINT_3, 0);
                    break;
                case 5:
                    events.RescheduleEvent(EVENT_POINT_4, 0);
                    break;
                case 6:
                    events.RescheduleEvent(EVENT_POINT_5, 0);
                    break;
                case 7:
                    me->SetCanFly(false);
                    me->RemoveAura(SPELL_NIMBUS);
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
            }
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
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_NIMBUS:
                        me->StopAttack();
                        me->GetMotionMaster()->MovePoint(2, centerPos);
                        events.RescheduleEvent(EVENT_NIMBUS, 67000);
                        break;
                    case EVENT_POINT_1:
                        me->SetCanFly(true);
                        me->GetMotionMaster()->MovePoint(3, CirclePos[0]);
                        break;
                    case EVENT_POINT_2:
                        me->GetMotionMaster()->MovePoint(4, CirclePos[1]);
                        break;
                    case EVENT_POINT_3:
                        me->GetMotionMaster()->MovePoint(5, CirclePos[2]);
                        break;
                    case EVENT_POINT_4:
                        me->GetMotionMaster()->MovePoint(6, CirclePos[3]);
                        break;
                    case EVENT_POINT_5:
                        me->GetMotionMaster()->MovePoint(7, centerPos);
                        break;
                    case EVENT_FLYING_KICK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true))
                            DoCast(target, SPELL_FLYING_SERPENT_KICK);
                        events.RescheduleEvent(EVENT_FLYING_KICK, 10000);
                        break;
                    case EVENT_CYCLONE_KICK:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_CYCLONE_KICK);
                        events.RescheduleEvent(EVENT_CYCLONE_KICK, 22000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_cloudbender_koboAI (creature);
    }
};

class boss_maki_waterblade : public CreatureScript
{
public:
    boss_maki_waterblade() : CreatureScript("boss_maki_waterblade") { }

    struct boss_maki_waterbladeAI : public BossAI
    {
        boss_maki_waterbladeAI(Creature* creature) : BossAI(creature, DATA_FINAL_STAGE)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset() override
        {
            events.Reset();
            me->SetReactState(REACT_PASSIVE);
            events.RescheduleEvent(EVENT_POINT_HOME, 2000);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_CRASHING_SLASH, 4000);
            events.RescheduleEvent(EVENT_WHIRLPOOL, 54000);
        }

        void EnterEvadeMode() override
        {
            _Reset();
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* gurgthock = instance->instance->GetCreature(instance->GetGuidData(NPC_GURGTHOCK)))
                gurgthock->AI()->Talk(SAY_MAKI_END);
            _JustDied();
        }

        void JustSummoned(Creature* summon) override
        {
            summon->AI()->DoCast(SPELL_SAUROK_WATERFALL);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == 1)
            {
                if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                    pGo->SetGoState(GO_STATE_READY);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                me->SetReactState(REACT_AGGRESSIVE);
            }
            if (type == POINT_MOTION_TYPE && id == 2)
            {
                DoCast(SPELL_WHIRLPOOL);
                for(uint8 i = 0; i < 7; i++)
                    me->SummonCreature(41206, WaterPos[i], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 14000);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim() && me->isInCombat())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) || me->HasAura(SPELL_WHIRLPOOL))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_POINT_HOME:
                        if (GameObject* pGo = instance->instance->GetGameObject(instance->GetGuidData(DATA_DOOR)))
                            pGo->SetGoState(GO_STATE_ACTIVE);
                        me->GetMotionMaster()->MovePoint(1, centerPos);
                        break;
                    case EVENT_CRASHING_SLASH:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_CRASHING_SLASH);
                        events.RescheduleEvent(EVENT_CRASHING_SLASH, 8000);
                        break;
                    case EVENT_WHIRLPOOL:
                        Talk(SAY_WHIRLPOOL);
                        me->StopAttack();
                        me->GetMotionMaster()->MovePoint(2, centerPos);
                        events.RescheduleEvent(EVENT_WHIRLPOOL, 80000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_maki_waterbladeAI (creature);
    }
};

class npc_kobo_twister : public CreatureScript
{
public:
    npc_kobo_twister() : CreatureScript("npc_kobo_twister") { }

    struct npc_kobo_twisterAI : public Scripted_NoMovementAI
    {
        npc_kobo_twisterAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
        }
        
        void Reset() override
        {
            DoCast(SPELL_TWISTER);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_kobo_twisterAI (creature);
    }
};

class go_gong_temple_tiger : public GameObjectScript
{
public:
    go_gong_temple_tiger() : GameObjectScript("go_gong_temple_tiger") {}

    uint8 chance;

    bool OnGossipHello(Player* /*pPlayer*/, GameObject* pGo) override
    {       
        InstanceScript* pInstance = pGo->GetInstanceScript();
        if (!pInstance)
            return false;

        if (pInstance->IsEncounterInProgress())
            return false;

        if (pInstance->GetBossState(DATA_SCAR_SHELL) != DONE)
        {
            pInstance->SetBossState(DATA_SCAR_SHELL, NOT_STARTED);
            pInstance->SetBossState(DATA_SCAR_SHELL, IN_PROGRESS);
            pGo->SummonCreature(NPC_SCAR_SHELL, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
        }

        else if (pInstance->GetBossState(DATA_SCAR_SHELL) == DONE && pInstance->GetBossState(DATA_JOLGRUM) != DONE)
        {
            pInstance->SetBossState(DATA_JOLGRUM, NOT_STARTED);
            pInstance->SetBossState(DATA_JOLGRUM, IN_PROGRESS);
            pGo->SummonCreature(NPC_JOLGRUM, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
        }

        else if (pInstance->GetBossState(DATA_JOLGRUM) == DONE && pInstance->GetBossState(DATA_LIUYANG) != DONE)
        {
            pInstance->SetBossState(DATA_LIUYANG, NOT_STARTED);
            pInstance->SetBossState(DATA_LIUYANG, IN_PROGRESS);
            pGo->SummonCreature(NPC_LIUYANG, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
        }

        else if (pInstance->GetBossState(DATA_LIUYANG) == DONE && pInstance->GetBossState(DATA_CHAGAN) != DONE)
        {
            pInstance->SetBossState(DATA_CHAGAN, NOT_STARTED);
            pInstance->SetBossState(DATA_CHAGAN, IN_PROGRESS);
            pGo->SummonCreature(NPC_FIREHOOF, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
        }

        else if (pInstance->GetBossState(DATA_CHAGAN) == DONE && pInstance->GetBossState(DATA_FINAL_STAGE) != DONE)
        {
            pInstance->SetBossState(DATA_FINAL_STAGE, NOT_STARTED);
            pInstance->SetBossState(DATA_FINAL_STAGE, IN_PROGRESS);

            if (pInstance->GetData(DATA_SATAY) != IN_PROGRESS || pInstance->GetData(DATA_KOBO) != IN_PROGRESS || pInstance->GetData(DATA_MAKI) != IN_PROGRESS)
                chance = urand(0, 2);
            
            if (chance == 0)
            {
                pInstance->SetData(DATA_SATAY, IN_PROGRESS);
                if (Creature* gurgthock = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_GURGTHOCK)))
                    gurgthock->AI()->Talk(SAY_SATAY_START);
                pGo->SummonCreature(NPC_SATAY_BYU, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            }
            if (chance == 1)
            {
                pInstance->SetData(DATA_KOBO, IN_PROGRESS);
                if (Creature* gurgthock = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_GURGTHOCK)))
                    gurgthock->AI()->Talk(SAY_KOBO_START);
                pGo->SummonCreature(NPC_KOBO, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            }
            if (chance == 2)
            {
                pInstance->SetData(DATA_MAKI, IN_PROGRESS);
                if (Creature* gurgthock = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_GURGTHOCK)))
                    gurgthock->AI()->Talk(SAY_MAKI_START);
                pGo->SummonCreature(NPC_MAKI, spawnPos[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10000);
            }
        }
        return false;
    }
};

class at_enter_arena_annihilation: public AreaTriggerScript
{
public:
    at_enter_arena_annihilation() : AreaTriggerScript("at_enter_arena_annihilation") { }

    bool OnTrigger(Player* pPlayer, const AreaTriggerEntry* /*pAt*/, bool /*enter*/) override
    {
        if (pPlayer->isGameMaster())
            return false;
        
        if (InstanceScript* pInstance = pPlayer->GetInstanceScript())
        {
            if (pInstance->GetData(DATA_START_EVENT) != DONE)
            {
                pInstance->SetData(DATA_START_EVENT, DONE);
                if (Creature* gurgthock = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_GURGTHOCK)))
                    gurgthock->AI()->DoAction(ACTION_1);
            }
        }
        return true;
    }
};

class spell_jade_lightning_strike : public SpellScriptLoader
{
    public:
        spell_jade_lightning_strike() : SpellScriptLoader("spell_jade_lightning_strike") { }
 
        class spell_jade_lightning_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_jade_lightning_strike_SpellScript);
 
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.empty())
                    return;
 
                WorldObject* target = Trinity::Containers::SelectRandomContainerElement(targets);
                GetSpell()->AddDestTarget(SpellDestination(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()), 0);
            }
 
            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_jade_lightning_strike_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };
 
        SpellScript* GetSpellScript() const override
        {
            return new spell_jade_lightning_strike_SpellScript();
        }
};

void AddSC_arena_of_annihilation()
{
    new npc_scenario_gurgthock();
    new boss_satay_byu();
    new boss_cloudbender_kobo();
    new boss_maki_waterblade();
    new npc_kobo_twister();
    //new go_gong_temple_tiger();
    //new at_enter_arena_annihilation();
    new spell_jade_lightning_strike();
}
