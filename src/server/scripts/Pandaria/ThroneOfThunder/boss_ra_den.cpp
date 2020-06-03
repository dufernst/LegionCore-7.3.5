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
    //Ra Den and Corrupted Sphere
    SPELL_MATERIALS_OF_CREATION  = 138321,
    SPELL_IMBUED_WITH_VITA       = 138332,
    SPELL_IMBUED_WITH_ANIMA      = 138331,
    SPELL_UNLEASHED_VITA         = 138330,
    SPELL_UNLEASHED_ANIMA        = 138329,
    SPELL_FATAL_STRIKE           = 138334,
    SPELL_MURDEROUS_STRIKE       = 138333,
    SPELL_LINGERING_ENERGIES     = 138450,
    SPELL_S_CRACKLING_STALKER    = 138339,
    SPELL_S_SANGUINE_HORROR      = 138338,
    SPELL_RUIN                   = 139073,
    SPELL_RUIN_BOLT              = 139087,
    
    //Crackling Stalker
    SPELL_CRACKLE                = 138340,
    SPELL_CAUTERIZING_FLARE      = 138337,
    //Sanguine Horror
    SPELL_SANGUINE_VOLLEY        = 138341,
};

enum sEvents
{
    //Ra Den
    EVENT_MATERIALS_OF_CREATION  = 1,
    EVENT_SUMMON                 = 2,
    EVENT_RUIN_BOLT              = 3,
    //Corrupted Sphere
    EVENT_MOVING                 = 3,
    //Crackling Stalker
    EVENT_CRACKLE                = 4,
    //Sanguine Horror
    EVENT_SANGUINE_VOLLEY        = 5,
};

enum sActions
{
    //Ra Den
    ACTION_EVENTS_RESET          = 1,
    ACTION_EVENTS_RESET_2        = 2,
};

enum ssData
{
    DATA_SEND_DMG                = 1,
};

class boss_ra_den : public CreatureScript
{
    public:
        boss_ra_den() : CreatureScript("boss_ra_den") {}

        struct boss_ra_denAI : public BossAI
        {
            boss_ra_denAI(Creature* creature) : BossAI(creature, DATA_RA_DEN)
            {
                instance = creature->GetInstanceScript();
                if (!me->GetMap()->IsHeroic())
                    me->DespawnOrUnsummon();
            }

            InstanceScript* instance;
            uint32 checkpower;
            bool phasetwo;
            int32 dmg;

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveAurasDueToSpell(SPELL_LINGERING_ENERGIES);
                me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_VITA);
                me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_ANIMA);
				me->RemoveAurasDueToSpell(SPELL_RUIN);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNLEASHED_ANIMA);
                me->setPowerType(POWER_ENERGY);
                me->SetMaxPower(POWER_ENERGY, 100);
                me->SetPower(POWER_ENERGY, 0);
                checkpower = 0;
                phasetwo = false;
                dmg = 0;
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                checkpower = 1000;
                events.RescheduleEvent(EVENT_MATERIALS_OF_CREATION, 12000);
                events.RescheduleEvent(EVENT_SUMMON, 20000);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (HealthBelowPct(40) && !phasetwo && instance)
                {
                    phasetwo = true;
                    events.Reset();
                    checkpower = 0;
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_VITA);
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_ANIMA);
                    me->SetPower(POWER_ENERGY, 0);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNLEASHED_ANIMA);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MoveCharge(5448.52f, 4656.22f, -2.4769f, 10.0f, 1);
                }
            }

            void MovementInform(uint32 type, uint32 pointId)
            {
                if (type == POINT_MOTION_TYPE)
                {
                    if (pointId == 1 && instance)
                    {
                        if (!instance->IsWipe())
                        {
                            me->StopMoving();
                            me->GetMotionMaster()->Clear(false);
                            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                            me->SetReactState(REACT_AGGRESSIVE);
                            DoZoneInCombat(me, 200.0f);
                            DoCast(me, SPELL_RUIN);
                            events.RescheduleEvent(EVENT_RUIN_BOLT, 3000);
                        }
                    }
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_EVENTS_RESET:
                    if (instance)
                        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNLEASHED_ANIMA);
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_VITA);
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_ANIMA);
                    me->SetPower(POWER_ENERGY, 0); 
                    me->AddAura(SPELL_LINGERING_ENERGIES, me);
                    DoCastAOE(SPELL_UNLEASHED_VITA, true);
                    break;
                case ACTION_EVENTS_RESET_2:
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_VITA);
                    me->RemoveAurasDueToSpell(SPELL_IMBUED_WITH_ANIMA);
                    me->SetPower(POWER_ENERGY, 0); 
                    me->AddAura(SPELL_LINGERING_ENERGIES, me);
                    if (instance)
                        instance->DoAddAuraOnPlayers(SPELL_UNLEASHED_ANIMA);
                    break;
                }
            }

            uint32 GetData(uint32 type) const
            {
                if (type == DATA_SEND_DMG)
                    return dmg;
                else
                    return 0;
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (checkpower)
                {
                    if (checkpower <= diff)
                    {
                        if (me->GetPower(POWER_ENERGY) == 100 && me->getVictim())
                        {
                            if (me->HasAura(SPELL_IMBUED_WITH_VITA))
                                DoCast(me->getVictim(), SPELL_FATAL_STRIKE);
                            else if (me->HasAura(SPELL_IMBUED_WITH_ANIMA) && me->GetMap()->GetDifficultyID() == DIFFICULTY_25_HC)
                            {
                                dmg = 0;
                                dmg = me->getVictim()->GetHealth();
                                DoCast(me->getVictim(), SPELL_MURDEROUS_STRIKE);
                                me->SetPower(POWER_ENERGY, 0);
                            }
                        }
                        checkpower = 1000;
                    }
                    else
                        checkpower -= diff;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_MATERIALS_OF_CREATION:
                        DoCast(me, SPELL_MATERIALS_OF_CREATION);
                        events.RescheduleEvent(EVENT_MATERIALS_OF_CREATION, 35000);
                        break;
                    case EVENT_SUMMON:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                        {
                            if (me->HasAura(SPELL_IMBUED_WITH_VITA))
                                DoCast(target, SPELL_S_CRACKLING_STALKER);
                            else if (me->HasAura(SPELL_IMBUED_WITH_ANIMA))
                                DoCast(target, SPELL_S_SANGUINE_HORROR);
                        }
                        events.RescheduleEvent(EVENT_SUMMON, 41000);
                        break;
                    case EVENT_RUIN_BOLT:
                        if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                            DoCast(me->getVictim(), SPELL_RUIN_BOLT);
                        events.RescheduleEvent(EVENT_RUIN_BOLT, 3000);
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_ra_denAI(creature);
        }
};

void CallRaDenAndUseSphere(InstanceScript* instance, Creature* caller, uint32 callerEntry, bool died)
{
    if (instance && caller)
    {
        if (caller->ToTempSummon())
        {
            if (Unit* raden = caller->ToTempSummon()->GetSummoner())
            {
                if (raden->isAlive())
                {
                    if (died)
                    {
                        switch (callerEntry)
                        {
                        case NPC_CORRUPTED_ANIMA:
                            if (Creature* cv = caller->GetCreature(*caller, instance->GetGuidData(NPC_CORRUPTED_VITA)))
                            {
                                if (cv->isAlive())
                                {
                                    caller->DespawnOrUnsummon();
                                    cv->DespawnOrUnsummon();
                                    raden->ToCreature()->AI()->DoAction(ACTION_EVENTS_RESET);
                                    raden->AddAura(SPELL_IMBUED_WITH_VITA, raden);
                                }
                            }
                            break;
                        case NPC_CORRUPTED_VITA:
                            if (Creature* ca = caller->GetCreature(*caller, instance->GetGuidData(NPC_CORRUPTED_ANIMA)))
                            {
                                if (ca->isAlive())
                                {
                                    caller->DespawnOrUnsummon();
                                    ca->DespawnOrUnsummon();
                                    raden->ToCreature()->AI()->DoAction(ACTION_EVENTS_RESET_2);
                                    raden->AddAura(SPELL_IMBUED_WITH_ANIMA, raden);
                                }
                            }
                            break;
                        }
                    }
                    else
                    {
                        switch (callerEntry)
                        {
                        case NPC_CORRUPTED_ANIMA:
                            caller->DespawnOrUnsummon();
                            raden->ToCreature()->AI()->DoAction(ACTION_EVENTS_RESET_2);
                            raden->AddAura(SPELL_IMBUED_WITH_ANIMA, raden);
                            break;
                        case NPC_CORRUPTED_VITA:
                            caller->DespawnOrUnsummon();
                            raden->ToCreature()->AI()->DoAction(ACTION_EVENTS_RESET);
                            raden->AddAura(SPELL_IMBUED_WITH_VITA, raden);
                            break;
                        }
                    }
                }
            }
        }
    }
}

//69957, 69958
class npc_corrupted_sphere : public CreatureScript
{
    public:
        npc_corrupted_sphere() : CreatureScript("npc_corrupted_sphere") {}

        struct npc_corrupted_sphereAI : public ScriptedAI
        {
            npc_corrupted_sphereAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
            }

            InstanceScript* pInstance;
            EventMap events;
            float newx, lastx, newy, lasty;

            void Reset()
            {
                newx, lastx, newy, lasty = 0;
                events.RescheduleEvent(EVENT_MOVING, 1000);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth())
                {
                    if (pInstance)
                        CallRaDenAndUseSphere(pInstance, me, me->GetEntry(), true);
                }
            }
            
            void MoveToRaDen()
            {
                if (me->ToTempSummon() && pInstance)
                {
                    if (Unit* raden = me->ToTempSummon()->GetSummoner())
                    {
                        if (raden->isAlive())
                        {
                            if (me->GetDistance(raden) <= 5.0f)
                                CallRaDenAndUseSphere(pInstance, me, me->GetEntry(), false);
                            else
                            {
                                float x, y, z;
                                raden->GetPosition(x, y, z);
                                newx = x;
                                newy = y;
                                if (newx != lastx || newy != lasty)
                                {
                                    me->GetMotionMaster()->Clear(false);
                                    switch (me->GetEntry())
                                    {
                                    case NPC_CORRUPTED_ANIMA:
                                        me->GetMotionMaster()->MoveCharge(newx, newy, z + 2.0f, 6.0f);
                                        break;
                                    case NPC_CORRUPTED_VITA:
                                        me->GetMotionMaster()->MoveCharge(newx, newy, z + 7.0f, 6.0f);
                                        break;
                                    }
                                }
                                lastx = newx;
                                lasty = newy;
                                events.RescheduleEvent(EVENT_MOVING, 1000);
                            }
                        }
                    }
                }
            }
            
            void UpdateAI(uint32 diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_MOVING)
                        MoveToRaDen();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_corrupted_sphereAI(creature);
        }
};

//69871
class npc_sanguine_horror : public CreatureScript
{
    public:
        npc_sanguine_horror() : CreatureScript("npc_sanguine_horror") {}

        struct npc_sanguine_horrorAI : public ScriptedAI
        {
            npc_sanguine_horrorAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* pInstance;
            EventMap events;

            void Reset()
            {
                events.Reset();
                if (Player* pl = me->FindNearestPlayer(100.0f, true))
                    AttackStart(pl);
            }

            void EnterCombat(Unit* who)
            {
                DoZoneInCombat(me, 100.0f);
                events.RescheduleEvent(EVENT_SANGUINE_VOLLEY, 3000);
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_SANGUINE_VOLLEY)
                    {
                        if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                            DoCast(me, SPELL_SANGUINE_VOLLEY);
                        events.RescheduleEvent(EVENT_SANGUINE_VOLLEY, 3000);
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_sanguine_horrorAI(creature);
        }
};

//69872
class npc_crackling_stalker : public CreatureScript
{
    public:
        npc_crackling_stalker() : CreatureScript("npc_crackling_stalker") {}

        struct npc_crackling_stalkerAI : public ScriptedAI
        {
            npc_crackling_stalkerAI(Creature* creature) : ScriptedAI(creature)
            {
                pInstance = creature->GetInstanceScript();
            }

            InstanceScript* pInstance;
            EventMap events;
            bool done;

            void Reset()
            {
                events.Reset();
                done = false;
                if (Player* pl = me->FindNearestPlayer(100.0f, true))
                    AttackStart(pl);
            }

            void EnterCombat(Unit* who)
            {
                DoZoneInCombat(me, 100.0f);
                events.RescheduleEvent(EVENT_CRACKLE, 10000);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (damage >= me->GetHealth() && !done)
                {
                    done = true;
                    DoCast(me, SPELL_CAUTERIZING_FLARE);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    if (eventId == EVENT_CRACKLE)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                            DoCast(target, SPELL_CRACKLE);
                        events.RescheduleEvent(EVENT_CRACKLE, 15000);
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_crackling_stalkerAI(creature);
        }
};

//138321
class spell_material_of_creation : public SpellScriptLoader
{
    public:
        spell_material_of_creation() : SpellScriptLoader("spell_material_of_creation") { }
        
        class spell_material_of_creation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_material_of_creation_SpellScript);
            
            void OnAfterCast()
            {
                if (GetCaster() && GetCaster()->ToCreature())
                {
                    uint8 pos = urand(0, 1);
                    float x, x2, y, y2;
                    switch (pos)
                    {
                    case 0:
                        GetCaster()->GetNearPoint2D(x, y, 40.0f, 1.570796326795f); 
                        GetCaster()->GetNearPoint2D(x2, y2, 40.0f, M_PI + 1.570796326795f);
                        GetCaster()->SummonCreature(NPC_CORRUPTED_ANIMA, x, y, GetCaster()->GetPositionZ() + 2.0f);
                        GetCaster()->SummonCreature(NPC_CORRUPTED_VITA, x2, y2, GetCaster()->GetPositionZ() + 7.0f);
                        break;
                    case 1:
                        GetCaster()->GetNearPoint2D(x, y, 40.0f, 0.0f); 
                        GetCaster()->GetNearPoint2D(x2, y2, 40.0f, M_PI);
                        GetCaster()->SummonCreature(NPC_CORRUPTED_ANIMA, x, y, GetCaster()->GetPositionZ() + 2.0f);
                        GetCaster()->SummonCreature(NPC_CORRUPTED_VITA, x2, y2, GetCaster()->GetPositionZ() + 7.0f);
                        break;
                    }
                }
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_material_of_creation_SpellScript::OnAfterCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_material_of_creation_SpellScript();
        }
};

enum SSpells
{
    //Palladin                
    SPELL_SHIELD_OF_THE_RIGHTEOUS  = 132403,
    //Warrior
    SPELL_SHIELD_BLOCK             = 132404,
    //Druid
    SPELL_SAVAGE_DEFENSE           = 132402,
    //Monk
    SPELL_SHUFFLE                  = 115307,
    //Death Knight
    SPELL_BLOOD_SHIELD             = 77535,
};

uint32 SafeSpells[5] = 
{
    SPELL_SHIELD_OF_THE_RIGHTEOUS,
    SPELL_SHIELD_BLOCK,
    SPELL_SAVAGE_DEFENSE,
    SPELL_SHUFFLE,
    SPELL_BLOOD_SHIELD,      
};

//138334
class spell_fatal_strike : public SpellScriptLoader
{
    public:
        spell_fatal_strike() : SpellScriptLoader("spell_fatal_strike") { }

        class spell_fatal_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_fatal_strike_SpellScript);

            void DealDamage()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                for (uint8 n = 0; n <= 4; n++)
                {
                    if (GetHitUnit()->HasAura(SafeSpells[n]))
                    {
                        SetHitDamage(500000);
                        return;
                    }
                }
                GetCaster()->Kill(GetHitUnit(), true);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_fatal_strike_SpellScript::DealDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_fatal_strike_SpellScript();
        }
};

//138333
class spell_murderous_strike : public SpellScriptLoader
{
    public:
        spell_murderous_strike() : SpellScriptLoader("spell_murderous_strike") { }

        class spell_murderous_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_murderous_strike_AuraScript);

            void HandleTick(AuraEffect const* aurEff, float &amount, Unit* target)
            {
                if (GetCaster() && GetCaster()->ToCreature())
                    amount = GetCaster()->ToCreature()->AI()->GetData(DATA_SEND_DMG);
            }

            void Register()
            {
                DoEffectChangeTickDamage += AuraEffectChangeTickDamageFn(spell_murderous_strike_AuraScript::HandleTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_murderous_strike_AuraScript();
        }
};

//138329
class spell_unleashed_anima : public SpellScriptLoader
{
    public:
        spell_unleashed_anima() : SpellScriptLoader("spell_unleashed_anima") { }

        class spell_unleashed_anima_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_unleashed_anima_AuraScript);
            
            void HandlePeriodicTick(AuraEffect const * /*aurEff*/)
            {
                if (GetTarget())
                {
                    if (GetTarget()->GetMap()->GetId() == 1098)
                    {
                        if (Creature* rd = GetTarget()->FindNearestCreature(NPC_RA_DEN, 100, true))
                        {
                            if (rd->isInCombat())
                                return;
                        }
                    }
                    GetTarget()->RemoveAurasDueToSpell(SPELL_UNLEASHED_ANIMA);
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_unleashed_anima_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_unleashed_anima_AuraScript();
        }
};

void AddSC_boss_ra_den()
{
    new boss_ra_den();
    new npc_corrupted_sphere();
    new npc_sanguine_horror();
    new npc_crackling_stalker();
    new spell_material_of_creation();
    new spell_fatal_strike();
    new spell_murderous_strike();
    new spell_unleashed_anima();
}
