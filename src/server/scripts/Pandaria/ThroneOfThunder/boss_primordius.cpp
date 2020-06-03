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
    SPELL_PRIMORDIAL_STRIKE          = 136037,
    SPELL_MALFORMED_BLOOD            = 136050,
    SPELL_MUTATED_ABOMINATION        = 140544,
    SPELL_MUTATE_PRIMORDIUS          = 136203,
    SPELL_EVOLUTION                  = 136209,

    //Mutate
    SPELL_VOLATILE_PATHOGEN_DUMMY    = 136225,
    SPELL_METABOLIC_BOOST            = 136245,
    SPELL_VENTRAL_SACS               = 136210,
    SPELL_GAS_BLADDER_DUMMY          = 136215,
    SPELL_ACIDIC_SPINES              = 136218,
    SPELL_ERUPTING_PUSTULES          = 136246,

    SPELL_MUTAGENIC_POOL_AT          = 136049,
    SPELL_MUTATION                   = 136178, //mutagenic pool hit

    SPELL_VOLATILE_POOL_AT           = 140506,
    SPELL_VOLATILE_MUTATE            = 141094, //dmg aura
    SPELL_VOLATILE_MUTATE_PRIMORDIUS = 140509, //heal

    SPELL_CAUSTIC_GAS                = 136216,
    SPELL_ACIDIC_EXPLOSION           = 136219,
    SPELL_VOLATILE_PATHOGEN          = 136228,
    SPELL_ENRAGE                     = 144369,

    //Player Mutate - buff
    SPELL_THICK_BONES                = 136184,
    SPELL_CLEAR_MIND                 = 136186,
    SPELL_IMPROVED_SYNAPSES          = 136182,
    SPELL_KEEN_EYESIGHT              = 136180,

    //Player Mutate - debuff
    SPELL_FRAGILE_BONES              = 136185,
    SPELL_CLOUDED_MIND               = 136187,
    SPELL_DULLED_SYNAPSES            = 136183,
    SPELL_IMPAIRED_EYESIGHT          = 136181,

    SPELL_FULLY_MUTATED              = 140546,
};

enum eEvents
{
    EVENT_PRIMORDIAL_STRIKE = 1,
    EVENT_MALFORMED_BLOOD,
    EVENT_MOVE_TO_PRIMORDIUS,
    EVENT_MOVE_TO_PRIMORDIUS_VOLATILE,
    EVENT_VOLATILE_PATHOGEN,
    EVENT_CAUSTIC_GAS,
};

Position livingfluidspawnpos[10] =
{
    //north
    {5627.40f, 4595.10f, 55.3662f, 2.16f},
    {5653.54f, 4620.64f, 55.3662f, 2.61f},
    {5661.98f, 4656.07f, 55.3657f, 3.12f},
    {5653.28f, 4691.50f, 55.3672f, 3.66f},
    {5627.72f, 4717.62f, 55.3656f, 4.14f},
    //south
    {5557.15f, 4594.56f, 55.3659f, 1.03f},
    {5531.23f, 4620.69f, 55.3655f, 0.49f},
    {5521.19f, 4656.40f, 55.3662f, 6.23f},
    {5531.73f, 4691.02f, 55.3660f, 5.73f},
    {5556.86f, 4717.05f, 55.3653f, 5.20f},
};

uint32 const mutatepells[6] =
{
    SPELL_VOLATILE_PATHOGEN_DUMMY,
    SPELL_METABOLIC_BOOST,
    SPELL_VENTRAL_SACS,
    SPELL_GAS_BLADDER_DUMMY,
    SPELL_ACIDIC_SPINES,
    SPELL_ERUPTING_PUSTULES,
};

uint32 const pimprovedmutatespells[4] =
{
    SPELL_THICK_BONES,
    SPELL_CLEAR_MIND,
    SPELL_IMPROVED_SYNAPSES,
    SPELL_KEEN_EYESIGHT,
};

uint32 const pimpairmutatespells[4] =
{
    SPELL_FRAGILE_BONES,
    SPELL_CLOUDED_MIND,
    SPELL_DULLED_SYNAPSES,
    SPELL_IMPAIRED_EYESIGHT,
};

enum CreatureTexts
{
    SAY_PULL = 1,   //Прекрасная плоть, да-да-да, отдайте её нам!…  35742
    SAY_EVOLUTION,  //Она разрывает нас изнутри!  36112
    SAY_DIED,       //Снова нас разрывают на куски… снова в эту холодную тьму…  35743
};

class boss_primordius : public CreatureScript
{
public:
    boss_primordius() : CreatureScript("boss_primordius") {}

    struct boss_primordiusAI : public BossAI
    {
        boss_primordiusAI(Creature* creature) : BossAI(creature, DATA_PRIMORDIUS)
        {
            instance = creature->GetInstanceScript();
            me->ModifyAuraState(AURA_STATE_UNKNOWN22, true);
        }
        InstanceScript* instance;
        uint32 summonlivingfluids;
        uint32 enragetimer;
        uint32 updatepower;

        void Reset()
        {
            _Reset();
            updatepower = 0;
            enragetimer = 0;
            summonlivingfluids = 0;
            me->SetCreateMana(60);
            me->SetMaxPower(POWER_MANA, 60);
            me->SetPower(POWER_MANA, 0);
            me->RemoveAurasDueToSpell(SPELL_ENRAGE);
            me->RemoveAurasDueToSpell(SPELL_EVOLUTION);
            RemoveMutationFromPlayers();

            for (uint8 n = 0; n < 6; n++)
                me->RemoveAurasDueToSpell(mutatepells[n]);

            if (!me->HasAura(SPELL_MUTATED_ABOMINATION))
                DoCast(me, SPELL_MUTATED_ABOMINATION, true);

        }

        void RemoveMutationFromPlayers()
        {
            for (uint8 b = 0; b < 4; b++)
                instance->DoRemoveAurasDueToSpellOnPlayers(pimprovedmutatespells[b]);

            for (uint8 m = 0; m < 4; m++)
                instance->DoRemoveAurasDueToSpellOnPlayers(pimpairmutatespells[m]);

            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FULLY_MUTATED);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MALFORMED_BLOOD);
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            Talk(SAY_PULL);
            updatepower = 1000;
            enragetimer = 480000;
            summonlivingfluids = 11000;
            events.RescheduleEvent(EVENT_MALFORMED_BLOOD, 8000);
            events.RescheduleEvent(EVENT_PRIMORDIAL_STRIKE, 16000);

            if (!me->HasAura(SPELL_MUTATED_ABOMINATION))
                DoCast(me, SPELL_MUTATED_ABOMINATION, true);

            //need trigger for spawn, and fast despawn AT(pool from living fluids)
            Position pos;
            me->GetPosition(&pos);
            me->SummonCreature(NPC_AT_CASTER_STALKER, pos);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            Talk(SAY_DIED);
            RemoveMutationFromPlayers();
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_MUTATE:
                DoCast(me, data, true);
                switch (data)
                {
                case SPELL_VOLATILE_PATHOGEN_DUMMY:
                    events.RescheduleEvent(EVENT_VOLATILE_PATHOGEN, 27000);
                    break;
                case SPELL_GAS_BLADDER_DUMMY:
                    events.RescheduleEvent(EVENT_CAUSTIC_GAS, 14000);
                    break;
                }
                break;
            case DATA_REMOVE_MUTATE:
                switch (data)
                {
                case SPELL_VOLATILE_PATHOGEN_DUMMY:
                    events.CancelEvent(EVENT_VOLATILE_PATHOGEN);
                    break;
                case SPELL_GAS_BLADDER_DUMMY:
                    events.CancelEvent(EVENT_CAUSTIC_GAS);
                    break;
                }
                break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (enragetimer)
            {
                if (enragetimer <= diff)
                {
                    enragetimer = 0;
                    DoCast(me, SPELL_ENRAGE, true);
                }
                else
                    enragetimer -= diff;
            }

            if (updatepower)
            {
                if (updatepower <= diff)
                {
                    if (me->GetPower(POWER_MANA) <= 59)
                        me->SetPower(POWER_MANA, me->GetPower(POWER_MANA) + 1, true);

                    if (me->GetPower(POWER_MANA) == 60)
                    {
                        DoCast(me, SPELL_EVOLUTION);
                        updatepower = 3000;
                    }
                    else
                        updatepower = 1000;
                }
                else
                    updatepower -= diff;
            }

            if (summonlivingfluids)
            {
                if (summonlivingfluids <= diff)
                {
                    for (uint8 n = 0; n < 10; n++)
                        me->SummonCreature(NPC_LIVING_FLUID, livingfluidspawnpos[n]);

                    summonlivingfluids = 15000;
                }
                else
                    summonlivingfluids -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MALFORMED_BLOOD:
                    if (Unit* target = me->getVictim())
                        target->CastSpell(target, SPELL_MALFORMED_BLOOD, true);
                    events.RescheduleEvent(EVENT_MALFORMED_BLOOD, 12000);
                    break;
                case EVENT_PRIMORDIAL_STRIKE:
                    if (Unit* target = me->getVictim())
                    {
                        me->SetFacingToObject(target);
                        DoCast(target, SPELL_PRIMORDIAL_STRIKE);
                    }
                    events.RescheduleEvent(EVENT_PRIMORDIAL_STRIKE, 24000);
                    break;
                //Mutation events
                case EVENT_VOLATILE_PATHOGEN:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                        DoCast(target, SPELL_VOLATILE_PATHOGEN);
                    events.RescheduleEvent(EVENT_VOLATILE_PATHOGEN, 27000);
                    break;
                case EVENT_CAUSTIC_GAS:
                    DoCast(me, SPELL_CAUSTIC_GAS);
                    events.RescheduleEvent(EVENT_CAUSTIC_GAS, 14000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_primordiusAI(creature);
    }
};

//69069
class npc_living_fluid : public CreatureScript
{
public:
    npc_living_fluid() : CreatureScript("npc_living_fluid") {}

    struct npc_living_fluidAI : public ScriptedAI
    {
        npc_living_fluidAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* pInstance;
        EventMap events;
        bool done;

        void Reset()
        {
            done = false;
            events.RescheduleEvent(EVENT_MOVE_TO_PRIMORDIUS, 1000);
        }

        void EnterCombat(Unit* who){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth() && !done)
            {
                done = true;
                events.Reset();
                damage = 0;
                me->StopMoving();
                me->GetMotionMaster()->Clear(false);
                uint8 mod = urand(0, 9);
                if (mod == 9)
                {
                    me->SetDisplayId(11686);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    DoCast(me, SPELL_VOLATILE_POOL_AT, true);
                    events.RescheduleEvent(EVENT_MOVE_TO_PRIMORDIUS_VOLATILE, 500);
                }
                else
                {
                    if (Creature* atcasterstalker = me->GetCreature(*me, pInstance->GetGuidData(NPC_AT_CASTER_STALKER)))
                        atcasterstalker->CastSpell(me, SPELL_MUTAGENIC_POOL_AT, true);
                    me->DespawnOrUnsummon();
                }
            }

            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MOVE_TO_PRIMORDIUS:
                    if (Creature* primordius = me->FindNearestCreature(NPC_PRIMORDIUS, 100.0f, true))
                    {
                        if (me->GetExactDist2d(primordius) <= 7.0f)
                        {
                            me->StopMoving();
                            me->GetMotionMaster()->Clear(false);
                            if (Creature* primordius = me->FindNearestCreature(NPC_PRIMORDIUS, 10.0f, true))
                                DoCast(primordius, SPELL_MUTATE_PRIMORDIUS, true);
                            me->DespawnOrUnsummon();
                        }
                        else
                        {
                            me->GetMotionMaster()->Clear(false);
                            me->GetMotionMaster()->MoveJump(primordius->GetPositionX(), primordius->GetPositionY(), me->GetPositionZ(), 2.0f, 0.0f, 5);
                            events.RescheduleEvent(EVENT_MOVE_TO_PRIMORDIUS, 1000);
                        }
                    }
                    break;
                case EVENT_MOVE_TO_PRIMORDIUS_VOLATILE:
                    if (Creature* primordius = me->FindNearestCreature(NPC_PRIMORDIUS, 100.0f, true))
                    {
                        if (me->GetExactDist2d(primordius) <= 7.0f)
                        {
                            me->StopMoving();
                            me->GetMotionMaster()->Clear(false);
                            if (Creature* primordius = me->FindNearestCreature(NPC_PRIMORDIUS, 10.0f, true))
                                DoCast(primordius, SPELL_VOLATILE_MUTATE_PRIMORDIUS, true);
                            me->DespawnOrUnsummon();
                        }
                        else
                        {
                            me->GetMotionMaster()->Clear(false);
                            me->GetMotionMaster()->MoveJump(primordius->GetPositionX(), primordius->GetPositionY(), me->GetPositionZ(), 1.0f, 0.0f, 5);
                            events.RescheduleEvent(EVENT_MOVE_TO_PRIMORDIUS_VOLATILE, 1000);
                        }
                    }
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_living_fluidAI(creature);
    }
};

//69081
class npc_areatrigger_stalker_caster : public CreatureScript
{
public:
    npc_areatrigger_stalker_caster() : CreatureScript("npc_areatrigger_stalker_caster") { }

    struct npc_areatrigger_stalker_casterAI : public ScriptedAI
    {
        npc_areatrigger_stalker_casterAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_areatrigger_stalker_casterAI(pCreature);
    }
};

//136209
class spell_primordius_evolution : public SpellScriptLoader
{
public:
    spell_primordius_evolution() : SpellScriptLoader("spell_primordius_evolution") { }

    class spell_primordius_evolution_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_primordius_evolution_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                uint8 buffcount = 0;
                std::vector<uint32>evolutionspelllist;
                evolutionspelllist.clear();

                for (uint8 n = 0; n < 6; n++)
                    if (GetCaster()->HasAura(mutatepells[n]))
                        buffcount++;

                if (!buffcount)
                {
                    for (uint8 n = 0; n < 6; n++)
                        evolutionspelllist.push_back(mutatepells[n]);

                    if (!evolutionspelllist.empty())
                    {
                        std::vector<uint32>::const_iterator Itr = evolutionspelllist.begin();
                        std::advance(Itr, urand(0, evolutionspelllist.size() - 1));
                        GetCaster()->CastSpell(GetCaster(), *Itr, true);
                    }
                }
                else if (buffcount >= 3)
                {
                    //push in list active mutations
                    for (uint8 n = 0; n < 6; n++)
                        if (GetCaster()->HasAura(mutatepells[n]))
                            evolutionspelllist.push_back(mutatepells[n]);

                    if (!evolutionspelllist.empty())
                    {
                        //remove random mutation
                        uint32 removedmutation = 0;
                        std::vector<uint32>::const_iterator Itr = evolutionspelllist.begin();
                        std::advance(Itr, urand(0, evolutionspelllist.size() - 1));
                        removedmutation = *Itr;
                        GetCaster()->RemoveAurasDueToSpell(removedmutation);
                        evolutionspelllist.clear();

                        for (uint8 n = 0; n < 6; n++)
                            evolutionspelllist.push_back(mutatepells[n]);

                        //active new random mutation
                        if (!evolutionspelllist.empty())
                        {
                            std::random_shuffle(evolutionspelllist.begin(), evolutionspelllist.end());
                            for (std::vector<uint32>::const_iterator itr = evolutionspelllist.begin(); itr != evolutionspelllist.end(); itr++)
                            {
                                if (!GetCaster()->HasAura(*itr) && removedmutation != (*itr))
                                {
                                    GetCaster()->CastSpell(GetCaster(), *itr, true);
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (buffcount < 3)
                {
                    for (uint8 n = 0; n < 6; n++)
                        evolutionspelllist.push_back(mutatepells[n]);

                    //active new random mutation
                    if (!evolutionspelllist.empty())
                    {
                        std::random_shuffle(evolutionspelllist.begin(), evolutionspelllist.end());
                        for (std::vector<uint32>::const_iterator itr = evolutionspelllist.begin(); itr != evolutionspelllist.end(); itr++)
                        {
                            if (!GetCaster()->HasAura(*itr))
                            {
                                GetCaster()->CastSpell(GetCaster(), *itr, true);
                                break;
                            }
                        }
                    }
                }
                GetCaster()->ToCreature()->AI()->Talk(SAY_EVOLUTION);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_primordius_evolution_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_primordius_evolution_SpellScript();
    }
};

//136218
class spell_acidic_spines : public SpellScriptLoader
{
public:
    spell_acidic_spines() : SpellScriptLoader("spell_acidic_spines") { }

    class spell_acidic_spines_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_acidic_spines_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
            {
                std::list<Player*>pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 100.0f);
                if (!pllist.empty())
                {
                    std::list<Player*>::const_iterator itr = pllist.begin();
                    std::advance(itr, urand(0, pllist.size() - 1));
                    GetCaster()->CastSpell(*itr, SPELL_ACIDIC_EXPLOSION);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_acidic_spines_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };


    AuraScript* GetAuraScript() const
    {
        return new spell_acidic_spines_AuraScript();
    }
};

//136225, 136215
class spell_primordius_mutate : public SpellScriptLoader
{
public:
    spell_primordius_mutate() : SpellScriptLoader("spell_primordius_mutate") { }

    class spell_primordius_mutate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_primordius_mutate_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                uint32 data = GetSpellInfo()->Id;
                GetCaster()->ToCreature()->AI()->SetData(DATA_MUTATE, data);
            }
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH && GetCaster() && GetCaster()->ToCreature())
            {
                uint32 data = GetSpellInfo()->Id;
                GetCaster()->ToCreature()->AI()->SetData(DATA_REMOVE_MUTATE, data);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_primordius_mutate_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_primordius_mutate_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_primordius_mutate_AuraScript();
    }
};

//136178
class spell_player_mutate : public SpellScriptLoader
{
public:
    spell_player_mutate() : SpellScriptLoader("spell_player_mutate") { }

    class spell_player_mutate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_player_mutate_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetHitUnit() && GetHitUnit()->ToPlayer())
            {
                uint8 mutatestage = 0;
                std::vector<uint32>pmutatelist;
                pmutatelist.clear();

                for (uint8 n = 0; n < 4; n++)
                    if (GetHitUnit()->HasAura(pimprovedmutatespells[n]))
                        mutatestage += GetHitUnit()->GetAura(pimprovedmutatespells[n])->GetStackAmount();

                if (mutatestage < 5)
                {
                    for (uint8 n = 0; n < 4; n++)
                        pmutatelist.push_back(pimprovedmutatespells[n]);

                    std::vector<uint32>::const_iterator itr = pmutatelist.begin();
                    std::advance(itr, urand(0, pmutatelist.size() - 1));
                    GetHitUnit()->CastSpell(GetHitUnit(), *itr, true);
                    if (mutatestage == 4)
                        GetHitUnit()->CastSpell(GetHitUnit(), SPELL_FULLY_MUTATED, true);
                }
                else if (mutatestage >= 5)
                {
                    for (uint8 n = 0; n < 4; n++)
                        pmutatelist.push_back(pimpairmutatespells[n]);

                    std::vector<uint32>::const_iterator Itr = pmutatelist.begin();
                    std::advance(Itr, urand(0, pmutatelist.size() - 1));
                    GetHitUnit()->CastSpell(GetHitUnit(), *Itr, true);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_player_mutate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_player_mutate_SpellScript();
    }
};

//136184, 136186, 136182, 136180
class spell_player_impovered_mutate : public SpellScriptLoader
{
public:
    spell_player_impovered_mutate() : SpellScriptLoader("spell_player_impovered_mutate") { }

    class spell_player_impovered_mutate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_player_impovered_mutate_AuraScript);

        void BeforeDispel(DispelInfo* dispelData)
        {
            if (GetUnitOwner())
                GetUnitOwner()->RemoveAurasDueToSpell(SPELL_FULLY_MUTATED);
        }

        void Register()
        {
            OnDispel += AuraDispelFn(spell_player_impovered_mutate_AuraScript::BeforeDispel);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_player_impovered_mutate_AuraScript();
    }
};

//136185, 136187, 136183, 136181
class spell_player_impair_mutate : public SpellScriptLoader
{
public:
    spell_player_impair_mutate() : SpellScriptLoader("spell_player_impair_mutate") { }

    class spell_player_impair_mutate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_player_impair_mutate_AuraScript);

        void BeforeDispel(DispelInfo* dispelData)
        {
            if (GetUnitOwner())
            {
                for (uint8 n = 0; n < 4; n++)
                    GetUnitOwner()->RemoveAurasDueToSpell(pimprovedmutatespells[n]);

                for (uint8 n = 0; n < 4; n++)
                    GetUnitOwner()->RemoveAurasDueToSpell(pimpairmutatespells[n]);

                GetUnitOwner()->RemoveAurasDueToSpell(SPELL_FULLY_MUTATED);
            }
        }

        void Register()
        {
            OnDispel += AuraDispelFn(spell_player_impair_mutate_AuraScript::BeforeDispel);
        }

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_player_impair_mutate_AuraScript();
    }
};

//140508
class spell_player_volatile_mutate : public SpellScriptLoader
{
public:
    spell_player_volatile_mutate() : SpellScriptLoader("spell_player_volatile_mutate") { }

    class spell_player_volatile_mutate_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_player_volatile_mutate_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetHitUnit())
                GetHitUnit()->CastSpell(GetHitUnit(), pimpairmutatespells[urand(0, 3)], true);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_player_volatile_mutate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_player_volatile_mutate_SpellScript();
    }
};

void AddSC_boss_primordius()
{
    new boss_primordius();
    new npc_living_fluid();
    new npc_areatrigger_stalker_caster();
    new spell_primordius_evolution();
    new spell_acidic_spines();
    new spell_primordius_mutate();
    new spell_player_mutate();
    new spell_player_impovered_mutate();
    new spell_player_impair_mutate();
    new spell_player_volatile_mutate();
}
