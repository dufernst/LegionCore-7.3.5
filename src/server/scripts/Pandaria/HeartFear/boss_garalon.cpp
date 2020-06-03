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

enum eSpells
{
    SPELL_FURIOUS_SWIPE             = 122735, //Cleave
    SPELL_PHEROMONES                = 123808,
    SPELL_FURY                      = 122754,
    SPELL_CRUSH                     = 122774,
    SPELL_MEND_LEG                  = 123495,
    SPELL_DAMAGED                   = 123818,
    SPELL_AURA_MULTI_CANCELLER      = 128898,
    SPELL_BERSERK                   = 26662,

    //Pheromone
    SPELL_PHEROMONE_TRAIL           = 123106,

    //Legs
    SPELL_BROKEN_LEG                = 123500,
    SPELL_BROKEN_LEG_AURA_STACK     = 122786,

    SPELL_RIDE_VEH_RIGHT_LEG        = 122757, //123430
    SPELL_RIDE_VEH_LEFT_LEG         = 123424, //123431
    SPELL_RIDE_VEH_BACK_RIGHT_LEG   = 123427, //123433
    SPELL_RIDE_VEH_BACK_LEFT_LEG    = 123425, //123432
};

enum eEvents
{
    EVENT_FURIOUS_SWIPE     = 1,
    EVENT_CHECK_HIT_SWIPE   = 2,
    EVENT_MEND_LEG          = 3,
    EVENT_CRUSH             = 4,
    EVENT_CRUSH_DELAY       = 5,
    EVENT_BERSERK           = 6,
    EVENT_CHECK_VICTIM      = 7,
};

uint32 legSpells[4] =
{
    SPELL_RIDE_VEH_RIGHT_LEG,
    SPELL_RIDE_VEH_LEFT_LEG,
    SPELL_RIDE_VEH_BACK_RIGHT_LEG,
    SPELL_RIDE_VEH_BACK_LEFT_LEG
};

class boss_garalon : public CreatureScript
{
    public:
        boss_garalon() : CreatureScript("boss_garalon") {}

        struct boss_garalonAI : public BossAI
        {
            boss_garalonAI(Creature* creature) : BossAI(creature, DATA_GARALON)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            uint8 hitcount, stack;
            bool bPhase2, crush;

            void Reset()
            {
                _Reset();
                events.Reset();
                hitcount = 0;
                bPhase2 = false;
                crush = false;
                me->RemoveAllAuras();
                DoCast(me, SPELL_AURA_MULTI_CANCELLER, true);
                instance->DoRemoveAurasDueToSpellOnPlayers(123081);
                instance->DoRemoveAurasDueToSpellOnPlayers(122835);
            }

            void EnterCombat(Unit* who)
            {
                _EnterCombat();
                DoCast(SPELL_PHEROMONES);
                events.ScheduleEvent(EVENT_CHECK_VICTIM, 2000);
                events.ScheduleEvent(EVENT_FURIOUS_SWIPE, 10000);
                events.ScheduleEvent(EVENT_MEND_LEG, 30000);
                events.ScheduleEvent(EVENT_BERSERK, 7 * MINUTE * IN_MILLISECONDS);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_CRUSH, 35000);
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                DoCast(me, SPELL_AURA_MULTI_CANCELLER, true);
            }

            void PassengerBoarded(Unit* passenger, int8 seatId, bool apply)
            {
                passenger->CastSpell(passenger, legSpells[seatId], true);
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell)
            {
                if (target->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_FURIOUS_SWIPE)
                    hitcount++;
            }

            void DoAction(const int32 action)
            {
                if (action == ACTION_1 && !IsHeroic() && !crush)
                {
                    crush = true;
                    events.ScheduleEvent(EVENT_CRUSH_DELAY, 2000);
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                if (me->HealthBelowPct(33) && !bPhase2)
                {
                    bPhase2 = true;
                    DoCast(me, SPELL_DAMAGED, true);
                    events.CancelEvent(EVENT_CRUSH);
                }
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                if (me->GetExactDist2d(who) < 10.0f && !crush)
                {
                    crush = true;
                    events.ScheduleEvent(EVENT_CRUSH_DELAY, 1000);
                }
            }

            void UpdateAI(uint32 diff)
            {
                if (!UpdateVictim())
                {
                    if (me->isInCombat())
                        EnterEvadeMode();
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventid = events.ExecuteEvent())
                {
                    switch (eventid)
                    {
                        case EVENT_FURIOUS_SWIPE:
                            if (me->getVictim())
                                DoCast(me->getVictim(), SPELL_FURIOUS_SWIPE);
                            events.ScheduleEvent(EVENT_CHECK_HIT_SWIPE, 3000);
                            events.ScheduleEvent(EVENT_FURIOUS_SWIPE, 10000);
                            break;
                        case EVENT_CHECK_HIT_SWIPE:
                            if (hitcount < 2)
                                DoCast(me, SPELL_FURY);
                            hitcount = 0;
                            break;
                        case EVENT_MEND_LEG:
                        {
                            DoCast(SPELL_MEND_LEG);
                            if (Aura* aura = me->GetAura(SPELL_BROKEN_LEG_AURA_STACK))
                            {
                                stack = aura->GetStackAmount();
                                if (stack > 1)
                                    aura->SetStackAmount(stack - 1);
                                else
                                    aura->Remove();
                            }
                            events.ScheduleEvent(EVENT_MEND_LEG, 30000);
                            break;
                        }
                        case EVENT_CRUSH:
                            DoCast(SPELL_CRUSH);
                            events.ScheduleEvent(EVENT_CRUSH, 35000);
                            break;
                        case EVENT_CRUSH_DELAY:
                            crush = false;
                            DoCast(SPELL_CRUSH);
                            break;
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            break;
                        case EVENT_CHECK_VICTIM:
                            if (!me->getVictim())
                                EnterEvadeMode();
                            events.ScheduleEvent(EVENT_CHECK_VICTIM, 3000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_garalonAI(creature);
        }
};

//63053
class npc_garalons_leg : public CreatureScript
{
    public:
        npc_garalons_leg() : CreatureScript("npc_garalons_leg") {}

        struct npc_garalons_legAI : public ScriptedAI
        {
            npc_garalons_legAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;
            bool fakeDeath;

            void Reset()
            {
                fakeDeath = false;
            }

            void EnterCombat(Unit* who) {}

            void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
            {
                Creature* summoner = instance->instance->GetCreature(instance->GetGuidData(NPC_GARALON));
                if (summoner && !summoner->isInCombat())
                    summoner->Attack(attacker, true);

                if (me->GetHealth() <= damage)
                {
                    if (!fakeDeath)
                    {
                        fakeDeath = true;
                        DoCast(me, SPELL_BROKEN_LEG, true);
                        if (summoner)
                            summoner->CastSpell(summoner, SPELL_BROKEN_LEG_AURA_STACK, true);

                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                    damage = 0;
                }
            }

            void SpellHit(Unit* attacker, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_MEND_LEG)
                {
                    me->SetHealth(me->GetMaxHealth());
                    me->RemoveAurasDueToSpell(SPELL_BROKEN_LEG);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    fakeDeath = false;
                }
            }

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_garalons_legAI(creature);
        }
};

//63191
class npc_garalon_crusher : public CreatureScript
{
    public:
        npc_garalon_crusher() : CreatureScript("npc_garalon_crusher") {}

        struct npc_garalon_crusherAI : public ScriptedAI
        {
            npc_garalon_crusherAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
            }

            InstanceScript* instance;

            void Reset() {}

            void EnterCombat(Unit* who) {}

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_garalon_crusherAI(creature);
        }
};

//63021
class npc_pheromone_trail : public CreatureScript
{
    public:
        npc_pheromone_trail() : CreatureScript("npc_pheromone_trail") {}

        struct npc_pheromone_trailAI : public ScriptedAI
        {
            npc_pheromone_trailAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            }

            InstanceScript* instance;

            void Reset() {}

            void IsSummonedBy(Unit* summoner)
            {
                DoCast(me, SPELL_PHEROMONE_TRAIL);
            }

            void EnterCombat(Unit* who) {}

            void SpellHit(Unit* /*attacker*/, const SpellInfo* spell)
            {
                if (spell->Id == 128898)
                    me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_pheromone_trailAI(creature);
        }
};

//123100
class spell_garalon_pheromones_friendly : public SpellScriptLoader
{
    public:
        spell_garalon_pheromones_friendly() : SpellScriptLoader("spell_garalon_pheromones_friendly") { }

        class spell_garalon_pheromones_friendly_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_garalon_pheromones_friendly_SpellScript);

            void HandleScript()
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->RemoveAurasDueToSpell(122835);
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* garalon = instance->instance->GetCreature(instance->GetGuidData(NPC_GARALON)))
                        garalon->AI()->DoAction(ACTION_1);
            }

            void Register()
            {
                OnHit += SpellHitFn(spell_garalon_pheromones_friendly_SpellScript::HandleScript);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_garalon_pheromones_friendly_SpellScript();
        }
};

void AddSC_boss_garalon()
{
    new boss_garalon();
    new npc_garalons_leg();
    new npc_garalon_crusher();
    new npc_pheromone_trail();
    new spell_garalon_pheromones_friendly();
}