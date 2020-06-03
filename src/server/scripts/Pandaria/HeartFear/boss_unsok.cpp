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

#include "heart_of_fear.h"
#include "Vehicle.h"

enum eSpells
{
    SPELL_AMBER_CARAPACE        = 122540,
    SPELL_AMBER_EXPLOSION       = 122402,
    SPELL_AMBER_SCALPEL         = 121994,
    SPELL_MASSIVE_STOMP         = 122408,
    SPELL_CONCENTRATED_MUTATION = 122556,
    SPELL_CORROSIVE_AURA        = 122348,
    SPELL_PARASITIC_GROWTH      = 121949,
    SPELL_RESHAPE_LIFE_MORPH    = 122370,
    SPELL_RESHAPE_LIFE_STUN_CH  = 122784,
    SPELL_RESHAPE_LIFE_SELF_DMG = 124136,
    SPELL_DESTROY_WILL          = 124824,
    SPELL_GRAB                  = 122415,
    SPELL_FLING                 = 122413,

    //Living Amber
    SPELL_FIXATE                = 122477, // Summoned and choose target
    SPELL_REDUCE_CRIT_CHANCE    = 64481,  //Crit chance -999%
    SPELL_FEIGN_DEATH           = 70628,
    SPELL_EXPLOSE               = 122532,
    SPELL_CLEARALLDEBUFFS       = 34098,
    SPELL_BURNING_AMBER         = 122503,

    //Amber Parasite
    SPELL_GROW                  = 121989, // Size +3% for each stack

    //Amber Pool Stalker
    SPELL_BUBBLING_AMBER_VIS    = 122977,
};

enum eEvents
{
    //Unsok
    EVENT_AMBER_SCALPEL         = 1,
    EVENT_PARASITIC_GROWTH      = 2,
    EVENT_RESHAPE_LIFE          = 3,
    EVENT_DESTROY_WILL          = 4,
    EVENT_SET_STATE             = 5,

    //Amber Monster
    EVENT_MASSIVE_STOMP         = 1,
    EVENT_AMBER_EXPLOSION       = 2,
    EVENT_GRAB                  = 3,
    EVENT_FLING                 = 5,
};

enum Actions
{
    ACTION_INTRO_P3             = 1,
};

enum sSummons
{
    NPC_AMBER_BEAM_STALKER      = 62510,
    NPC_LIVING_AMBER            = 62691,
    NPC_AMBER_PARASITE          = 62509,
    NPC_MUTATED_CONSTRUCT       = 62701,
    NPC_AMBER_MONSTROSITY       = 62711,
};

Position const amberStPos[4] =
{
    {-2548.23f, 768.92f, 582.99f, 5.47f},
    {-2409.56f, 770.11f, 582.99f, 3.93f},
    {-2409.71f, 629.93f, 582.99f, 2.35f},
    {-2549.83f, 629.96f, 582.99f, 0.77f}
};

class boss_unsok : public CreatureScript
{
    public:
        boss_unsok() : CreatureScript("boss_unsok") {}

        struct boss_unsokAI : public BossAI
        {
            boss_unsokAI(Creature* creature) : BossAI(creature, DATA_UNSOK)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            bool phasetwo, phasethree;

            void Reset()
            {
                _Reset();
                phasetwo = false;
                phasethree = false;
                me->RemoveAurasDueToSpell(SPELL_AMBER_CARAPACE);
                me->RemoveAurasDueToSpell(SPELL_CONCENTRATED_MUTATION);
                me->RemoveAurasDueToSpell(122547); //scale
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                events.ScheduleEvent(EVENT_AMBER_SCALPEL, 10000); //31:47
                events.ScheduleEvent(EVENT_PARASITIC_GROWTH, 35000);
                events.ScheduleEvent(EVENT_RESHAPE_LIFE, 15000);
                events.ScheduleEvent(EVENT_DESTROY_WILL, 2000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(70) && !phasetwo)
                {
                    phasetwo = true;
                    me->AddAura(SPELL_AMBER_CARAPACE, me);
                    if (Creature* amber = me->SummonCreature(NPC_AMBER_MONSTER, amberStPos[urand(0,3)]))
                        amber->AI()->DoZoneInCombat(amber, 100.0f);
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_DESTROY_WILL)
                {
                    uint8 alterPower;
                    if (target->HasAura(SPELL_RESHAPE_LIFE_MORPH))
                        if (alterPower = target->GetPower(POWER_ALTERNATE))
                        {
                            if (alterPower > 40)
                                target->SetPower(POWER_ALTERNATE, alterPower - 40);
                            else
                            {
                                if (alterPower <= 0)
                                    return;

                                target->SetPower(POWER_ALTERNATE, 0);
                            }
                        }
                }
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_INTRO_P3 && !phasethree)
                {
                    phasethree = true;
                    events.Reset();
                    DoStopAttack();
                    me->GetMotionMaster()->MovePoint(1, -2479.44f, 699.81f, 580.08f); //centr pos
                    //Talk();
                }
            }

            void MovementInform(uint32 type, uint32 id)
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (id == 1)
                {
                    me->RemoveAurasDueToSpell(SPELL_AMBER_CARAPACE);
                    DoCast(me, 122547, true); //Scale
                    DoCast(me, SPELL_CONCENTRATED_MUTATION, true);
                    for (uint8 i = 0; i < 4; i++)
                        me->SummonCreature(NPC_AMBER_POOL, amberStPos[i]);
                    events.ScheduleEvent(EVENT_SET_STATE, 4000);
                    events.ScheduleEvent(EVENT_PARASITIC_GROWTH, 35000);
                    events.ScheduleEvent(EVENT_RESHAPE_LIFE, 15000);
                    events.ScheduleEvent(EVENT_DESTROY_WILL, 2000);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_AMBER_SCALPEL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 40.0f, true))
                            {
                                if (target->HasAura(SPELL_RESHAPE_LIFE_MORPH))
                                {
                                    events.ScheduleEvent(EVENT_AMBER_SCALPEL, 0);
                                    return;
                                }
                                if (Creature* abeam = me->SummonCreature(NPC_AMBER_BEAM_STALKER, target->GetPositionX()+6, target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 12000))
                                {
                                    abeam->AI()->AttackStart(target);
                                    DoCast(abeam, SPELL_AMBER_SCALPEL);
                                }
                            }
                            events.ScheduleEvent(EVENT_AMBER_SCALPEL, 40000);
                            break;
                        case EVENT_PARASITIC_GROWTH:
                            DoCast(SPELL_PARASITIC_GROWTH);
                            events.ScheduleEvent(EVENT_PARASITIC_GROWTH, 35000);
                            break;
                        case EVENT_RESHAPE_LIFE:
                            if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(pTarget, SPELL_RESHAPE_LIFE_STUN_CH);
                            if (phasethree)
                                events.ScheduleEvent(EVENT_RESHAPE_LIFE, 35000);
                            break;
                        case EVENT_DESTROY_WILL:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_DESTROY_WILL, true);
                            events.ScheduleEvent(EVENT_DESTROY_WILL, 2000);
                            break;
                        case EVENT_SET_STATE:
                            me->SetReactState(REACT_AGGRESSIVE);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_unsokAI(creature);
        }
};

//62711
class npc_amber_monster : public CreatureScript
{
    public:
        npc_amber_monster() : CreatureScript("npc_amber_monster") {}

        struct npc_amber_monsterAI : public ScriptedAI
        {
            npc_amber_monsterAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_AGGRESSIVE);
            }

            InstanceScript* pInstance;
            EventMap events;
            Player* plrRide;

            void Reset() {}
            
            void EnterCombat(Unit* attacker)
            {
                events.ScheduleEvent(EVENT_MASSIVE_STOMP, 20000);
                events.ScheduleEvent(EVENT_AMBER_EXPLOSION, 54000);
                events.ScheduleEvent(EVENT_GRAB, 32000); //32s
                events.ScheduleEvent(EVENT_DESTROY_WILL, 2000);
            }

            void JustDied(Unit* killer)
            {
                if (pInstance)
                {
                    if (Creature* unsok = me->GetCreature(*me, pInstance->GetGuidData(NPC_UNSOK)))
                        unsok->AI()->DoAction(ACTION_INTRO_P3);
                }
            }

            void SpellHitTarget(Unit* target, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_DESTROY_WILL)
                {
                    uint8 alterPower;
                    if (target->HasAura(SPELL_RESHAPE_LIFE_MORPH))
                    {
                        if (alterPower = target->GetPower(POWER_ALTERNATE))
                        {
                            if (alterPower > 40)
                                target->SetPower(POWER_ALTERNATE, alterPower - 40);
                            else
                            {
                                if (alterPower <= 0)
                                    return;

                                target->SetPower(POWER_ALTERNATE, 0);
                            }
                        }
                    }
                }
                if (spell->Id == SPELL_FLING)
                {
                    if (!plrRide || target == plrRide)
                    {
                        if (me->GetVehicleKit())
                            me->GetVehicleKit()->RemoveAllPassengers();

                        return;
                    }

                    if (me->GetVehicleKit())
                        me->GetVehicleKit()->RemoveAllPassengers();

                    plrRide->CastSpell(target, 122420, true); //jump
                    plrRide = NULL;
                }
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
            {
                if (!apply || !who->ToPlayer())
                    return;

                plrRide = who->ToPlayer();
                events.ScheduleEvent(EVENT_FLING, 100);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_MASSIVE_STOMP:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_MASSIVE_STOMP);
                            events.ScheduleEvent(EVENT_MASSIVE_STOMP, 30000);
                            break;
                        case EVENT_AMBER_EXPLOSION:
                            DoCast(SPELL_AMBER_EXPLOSION);
                            events.ScheduleEvent(EVENT_AMBER_EXPLOSION, 46000);
                            break;
                        case EVENT_GRAB:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_GRAB);
                            events.ScheduleEvent(EVENT_GRAB, 32000);
                            break;
                        case EVENT_DESTROY_WILL:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_DESTROY_WILL, true);
                            events.ScheduleEvent(EVENT_DESTROY_WILL, 2000);
                            break;
                        case EVENT_FLING:
                            DoCast(SPELL_FLING);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_amber_monsterAI(creature);
        }
};

//62510
class npc_amberbeam_stalker : public CreatureScript
{
public:
    npc_amberbeam_stalker() : CreatureScript("npc_amberbeam_stalker") { }

    struct npc_amberbeam_stalkerAI : public ScriptedAI
    {
        npc_amberbeam_stalkerAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_SCHOOL_DAMAGE, true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* pInstance;
        uint32 sum;

        void Reset()
        {
            sum = 3000;
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (sum)
            {
                if (sum <= diff)
                {
                    sum = 3000;
                    if (Creature* lamber = me->SummonCreature(NPC_LIVING_AMBER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000))
                    {
                        lamber->AI()->DoZoneInCombat(lamber, 100.0f);
                        lamber->CastSpell(lamber, SPELL_CORROSIVE_AURA, true);
                    }
                }
                else 
                    sum -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_amberbeam_stalkerAI (pCreature);
    }
};

class npc_living_amber : public CreatureScript
{
public:
    npc_living_amber() : CreatureScript("npc_living_amber") { }

    struct npc_living_amberAI : public ScriptedAI
    {
        npc_living_amberAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_AGGRESSIVE);
        }

        InstanceScript* pInstance;
        bool explose;

        void Reset()
        {
            explose = false;
        }

        void IsSummonedBy(Unit* summoner)
        {
            DoCast(me, SPELL_REDUCE_CRIT_CHANCE, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
            {
                if (!explose)
                {
                    explose = true;
                    DoStopAttack();
                    DoCast(me, SPELL_CLEARALLDEBUFFS, true);
                    DoCast(me, SPELL_EXPLOSE, true);
                    DoCast(me, SPELL_BURNING_AMBER, true);
                    DoCast(me, SPELL_FEIGN_DEATH, true);
                    me->RemoveAurasDueToSpell(SPELL_CORROSIVE_AURA);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
                damage = 0;
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (spell->Id == 123156)
                me->DespawnOrUnsummon();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_living_amberAI (pCreature);
    }
};

//62762
class npc_amber_pool_stalker : public CreatureScript
{
public:
    npc_amber_pool_stalker() : CreatureScript("npc_amber_pool_stalker") { }

    struct npc_amber_pool_stalkerAI : public ScriptedAI
    {
        npc_amber_pool_stalkerAI(Creature* creature) : ScriptedAI(creature) 
        {
            me->SetReactState(REACT_PASSIVE);
        }

        InstanceScript* pInstance;
        EventMap events;
        Unit* owner;

        void Reset() {}
        
        void IsSummonedBy(Unit* summoner)
        {
            owner = summoner;
            DoCast(me, SPELL_BUBBLING_AMBER_VIS, true);
            events.ScheduleEvent(EVENT_1, 500);
            events.ScheduleEvent(EVENT_2, 4000);
            events.ScheduleEvent(EVENT_3, 5000);
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell)
        {
            if (spell->Id == 123014)
                if (target->HasAura(SPELL_RESHAPE_LIFE_MORPH))
                    DoCast(target, 123198, true);
        }

        void UpdateAI(uint32 diff) 
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_1:
                        me->CastSpell(owner, 124014, false); //visual beam
                        break;
                    case EVENT_2:
                        DoCast(123020); //dmg
                        events.ScheduleEvent(EVENT_2, 1000);
                        break;
                    case EVENT_3:
                        DoCast(123014); //Volatile Amber
                        events.ScheduleEvent(EVENT_3, 1000);
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_amber_pool_stalkerAI (pCreature);
    }
};

//121949
class spell_unsok_parasitic_growth : public SpellScriptLoader
{
    public:
        spell_unsok_parasitic_growth() : SpellScriptLoader("spell_unsok_parasitic_growth") { }

        class spell_unsok_parasitic_growth_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_unsok_parasitic_growth_AuraScript);

            uint32 healPct;

            bool Load()
            {
                healPct = 0;
                return true;
            }

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                healPct += 12.5;
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                if (AuraEffect* aurEff = GetAura()->GetEffect(EFFECT_0))
                {
                    int32 amount = aurEff->GetBaseAmount();
                    AddPct(amount, healPct);
                    aurEff->SetAmount(amount);
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_unsok_parasitic_growth_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_unsok_parasitic_growth_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_unsok_parasitic_growth_AuraScript();
        }
};

//122370
class spell_unsok_reshape_life : public SpellScriptLoader
{
    public:
        spell_unsok_reshape_life() : SpellScriptLoader("spell_unsok_reshape_life") { }

        class spell_unsok_reshape_life_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_unsok_reshape_life_AuraScript);
            uint8 alterPower;
            uint8 explosionTimer;
            uint16 m_checkTimer;

            bool Load()
            {
                m_checkTimer = 1000;
                explosionTimer = 0;
                return true;
            }

            void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
            {
                if (m_checkTimer <= diff)
                    m_checkTimer = 1000;
                else
                {
                    m_checkTimer -= diff;
                    return;
                }

                if (Player* plr = GetUnitOwner()->ToPlayer())
                {
                    alterPower = plr->GetPower(POWER_ALTERNATE);
                    if (alterPower > 0)
                        plr->SetPower(POWER_ALTERNATE, alterPower - 2);
                    else
                    {
                        if (InstanceScript* pInstance = plr->GetInstanceScript())
                            if (Creature* unsok = pInstance->instance->GetCreature(pInstance->GetGuidData(NPC_UNSOK)))
                                unsok->SummonCreature(NPC_MUTATED_CONSTRUCT, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ() + 1.0f);
                        plr->CastSpell(plr, SPELL_RESHAPE_LIFE_SELF_DMG, true);
                        plr->Kill(plr);
                    }
                    explosionTimer++;
                    if (explosionTimer > 20)
                    {
                        explosionTimer = 0;
                        plr->CastSpell(plr, 122398);
                    }
                }
            }

            void Register()
            {
                OnEffectUpdate += AuraEffectUpdateFn(spell_unsok_reshape_life_AuraScript::OnUpdate, EFFECT_3, SPELL_AURA_ENABLE_ALT_POWER);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_unsok_reshape_life_AuraScript();
        }
};

//123156
class spell_unsok_consume_amber : public SpellScriptLoader
{
    public:
        spell_unsok_consume_amber() : SpellScriptLoader("spell_unsok_consume_amber") { }

        class spell_unsok_consume_amber_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_unsok_consume_amber_SpellScript);

            SpellCastResult CheckCast()
            {
                if (!GetCaster())
                    return SPELL_FAILED_CUSTOM_ERROR;

                if (Creature* amber = GetCaster()->FindNearestCreature(NPC_LIVING_AMBER, 5.0f))
                    if (amber->HasAura(SPELL_FEIGN_DEATH))
                        return SPELL_CAST_OK;

                return SPELL_FAILED_CUSTOM_ERROR;
            }

            void Register()
            {
                OnCheckCast += SpellCheckCastFn(spell_unsok_consume_amber_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_unsok_consume_amber_SpellScript();
        }
};

//123060
class spell_unsok_break_free : public SpellScriptLoader
{
    public:
        spell_unsok_break_free() : SpellScriptLoader("spell_unsok_break_free") { }

        class spell_unsok_break_freeSpellScript : public SpellScript
        {
            PrepareSpellScript(spell_unsok_break_freeSpellScript);

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;

                GetCaster()->RemoveAurasDueToSpell(SPELL_RESHAPE_LIFE_MORPH);
                GetCaster()->SetHealth(GetCaster()->CountPctFromMaxHealth(20));
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_unsok_break_freeSpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_unsok_break_freeSpellScript();
        }
};

void AddSC_boss_unsok()
{
    new boss_unsok();
    new npc_amber_monster();
    new npc_amberbeam_stalker();
    new npc_living_amber();
    new npc_amber_pool_stalker();
    new spell_unsok_parasitic_growth();
    new spell_unsok_reshape_life();
    new spell_unsok_consume_amber();
    new spell_unsok_break_free();
}
