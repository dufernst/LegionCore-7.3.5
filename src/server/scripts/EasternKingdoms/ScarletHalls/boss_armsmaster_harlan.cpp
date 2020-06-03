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
#include "scarlet_halls.h"

enum eSays
{
    SAY_AGGRO               = 0,
    SAY_SUMMON              = 1,
    SAY_SUMMON_EMOTE        = 2,
    SAY_BLADES_EVENT        = 3,
    SAY_BLADES_KILL_PLAYER  = 4,
    SAY_EVADE               = 5,
};

enum eSpells
{
    SPELL_DRAGON_REACH          = 111217,
    SPELL_CALL_REINFORCEMENTS   = 111755,
    SPELL_HEROIC_LEAP           = 111219,
    SPELL_BLADES_OF_LIGHT       = 111216,
    SPELL_BLADES_OF_LIGHT_DMG   = 111215,
    SPELL_BLADES_OF_LIGHT_RIDE  = 112955,
    SPELL_BERSERKER_RAGE        = 111221,

    //Trash
    SPELL_HEAVY_ARMOR           = 113959,
    SPELL_UNARMORED_FEMALE      = 113969,
    SPELL_UNARMORED_MALE        = 113970,

    SPELL_ACHIEV_MOSH_PIT       = 115674,
};

enum eEvents
{
    EVENT_DRAGONREACH       = 1, //7s
    EVENT_SUMMON_HELPERS    = 2, //20s
    EVENT_HEROIC_LEAP       = 3, //40s
    EVENT_BLADES_OF_LIGHT   = 4,
    EVENT_MOVE_PATH         = 5,
};

enum ePath
{
    HARLAN_PATH_1 = 5863200,
    HARLAN_PATH_2 = 5863201,
};

class boss_armsmaster_harlan : public CreatureScript
{
public:
    boss_armsmaster_harlan() : CreatureScript("boss_armsmaster_harlan") { }

    struct boss_armsmaster_harlanAI : public BossAI
    {
        boss_armsmaster_harlanAI(Creature* creature) : BossAI(creature, DATA_HARLAN)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;
        
        uint32 BladeHitCount;
        bool BladeActive;
        bool berserk;

        void Reset()
        {
            _Reset();
            me->GetMotionMaster()->MoveIdle();

            events.Reset();
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveAllAuras();
            BladeActive = false;
            berserk = false;
            BladeHitCount = 0;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            events.RescheduleEvent(EVENT_DRAGONREACH, 7000);
            events.RescheduleEvent(EVENT_SUMMON_HELPERS, 20000);
            events.RescheduleEvent(EVENT_HEROIC_LEAP, 40000);

            Talk(SAY_AGGRO);
        }

        void EnterEvadeMode()
        {
            Talk(SAY_EVADE);
            BossAI::EnterEvadeMode();
        }

        void KilledUnit(Unit* victim)
        {
            if (BladeActive)
                Talk(SAY_BLADES_KILL_PLAYER, victim->GetGUID());
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(50) && !berserk)
            {
                berserk = true;
                DoCast(SPELL_BERSERKER_RAGE);
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            me->GetMotionMaster()->Clear(false);
            if (BladeHitCount >= 8)
                instance->DoUpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, 115674);
            _JustDied();
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != WAYPOINT_MOTION_TYPE)
                return;

            if (id == 17)
            {
                BladeActive = false;
                me->RemoveAura(SPELL_BLADES_OF_LIGHT);
                me->RemoveAurasDueToSpell(SPELL_BLADES_OF_LIGHT_RIDE);
                me->SetReactState(REACT_AGGRESSIVE);
            }
        }

        void SpellHit(Unit* attacker, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_BLADES_OF_LIGHT_RIDE)
                BladeHitCount++;
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DRAGONREACH:
                        if (Unit* target = me->getVictim())
                            DoCast(target, SPELL_DRAGON_REACH);
                        events.RescheduleEvent(EVENT_DRAGONREACH, 7000);
                        break;
                    case EVENT_SUMMON_HELPERS:
                        Talk(SAY_SUMMON);
                        Talk(SAY_SUMMON_EMOTE);
                        DoCast(SPELL_CALL_REINFORCEMENTS);
                        events.RescheduleEvent(EVENT_SUMMON_HELPERS, 20000);
                        break;
                    case EVENT_HEROIC_LEAP:
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        DoCast(me, SPELL_HEROIC_LEAP, true);
                        me->GetMotionMaster()->Clear(false);
                        events.RescheduleEvent(EVENT_BLADES_OF_LIGHT, 2000);
                        break;
                    case EVENT_BLADES_OF_LIGHT:
                        Talk(SAY_BLADES_EVENT);
                        DoCast(SPELL_BLADES_OF_LIGHT);
                        events.RescheduleEvent(EVENT_MOVE_PATH, 1000);
                        break;
                    case EVENT_MOVE_PATH:
                        me->GetMotionMaster()->MovePath(urand(HARLAN_PATH_1, HARLAN_PATH_2), false);
                        BladeActive = true;
                        events.RescheduleEvent(EVENT_HEROIC_LEAP, 60000);
                        break;
                    default:
                        break;
                }
            }
            if (!me->HasAura(SPELL_BLADES_OF_LIGHT))
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_armsmaster_harlanAI (creature);
    }
};

// 58998
class npc_scarlet_defender : public CreatureScript
{
public:
    npc_scarlet_defender() : CreatureScript("npc_scarlet_defender") { }

    struct npc_scarlet_defenderAI : public CreatureAI
    {
        npc_scarlet_defenderAI(Creature* creature) : CreatureAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset()
        {
            DoCast(SPELL_HEAVY_ARMOR);
        }

        void EnterCombat(Unit* /*who*/)
        {
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
        return new npc_scarlet_defenderAI (creature);
    }
};

// 111394
class spell_blades_of_light : public SpellScriptLoader
{
    public:
        spell_blades_of_light() : SpellScriptLoader("spell_blades_of_light") { }

        class spell_blades_of_light_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_blades_of_light_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (!caster || !target)
                    return;

                if (!target->HasAura(SPELL_BLADES_OF_LIGHT_RIDE))
                    target->CastSpell(caster, SPELL_BLADES_OF_LIGHT_RIDE, true);

                caster->CastSpell(target, SPELL_BLADES_OF_LIGHT_DMG);
            }

            void Register()
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_blades_of_light_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_blades_of_light_SpellScript();
        }
};

// 113959
class spell_heavy_armor : public SpellScriptLoader
{
    public:
        spell_heavy_armor() : SpellScriptLoader("spell_heavy_armor") { }

        class spell_heavy_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_heavy_armor_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (caster->getGender() == GENDER_FEMALE)
                    caster->CastSpell(caster, SPELL_UNARMORED_FEMALE, true);
                else
                    caster->CastSpell(caster, SPELL_UNARMORED_MALE, true);
            }
            
            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_heavy_armor_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_heavy_armor_AuraScript();
        }
};

void AddSC_boss_armsmaster_harlan()
{
    new boss_armsmaster_harlan();
    new npc_scarlet_defender();
    new spell_blades_of_light();
    new spell_heavy_armor();
}
