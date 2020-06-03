/*
* Copyright (C) 2012-2013 JadeCore <http://www.pandashan.com/>
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

#include "GameObjectAI.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "terrace_of_endless_spring.h"
#include "GridNotifiers.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

enum eLeiShiSpells
{
    SPELL_AFRAID = 123181,
    SPELL_SPRAY = 123121,
    SPELL_HIDE = 123244,
    SPELL_HIDE_SUMMON = 123213,
    SPELL_HIDE_STACKS = 123233,
    SPELL_GET_AWAY = 123461,
    SPELL_GET_AWAY_TELEPORT = 123441,
    SPELL_PROTECT = 123250,
    SPELL_PROTECT_RESPAWN = 123493,
    SPELL_PROTECT_VISUAL = 123505,

    // This is for Heroic Mode
    SPELL_SCARY_FOG_CIRCLE = 123797,
    SPELL_SCARY_FOG_DOT = 123705,
    SPELL_SCARY_FOG_STACKS = 123712,

    SPELL_LEI_SHI_TRANSFORM = 127535,

    SPELL_LEI_SHI_RITUAL_OF_PURIFICATION = 126848,
};

enum eLeiShiEvents
{
    // Lei Shi
    EVENT_SPECIAL = 1,
    EVENT_SPRAY = 2,
    EVENT_GET_AWAY = 3,
    EVENT_GET_AWAY_CAST = 4,
    EVENT_PROTECT = 5,
    EVENT_HIDE = 6,

    // Lei Shi (hidden)
    EVENT_HIDDEN_SPRAY = 7,

    EVENT_ENRAGE = 8,
};

enum eLeiShiActions
{
    ACTION_ANIMATED_PROTECTOR_DIED,
    ACTION_ACTIVATE_ANIMATED_PROTECTORS,
    ACTION_TERMINATE_GET_AWAY_PHASE,
    ACTION_TERMINATE_HIDE_PHASE
};

enum eLeiShiSays
{
    TALK_AGGRO,
    TALK_HIDE,
    TALK_GET_AWAY,
    TALK_SLAY,
    TALK_DEFEATED
};

enum eLeiShiNpcs
{
    PROTECTOR = 62995,
};

Position leiShiPos = { -1017.93f, -2911.3f, 19.902f, 4.74f };

Position hidePositions[4] =
{
    { -990.678f, -2890.043f, 19.172f, 3.781f },
    { -994.835f, -2933.835f, 19.172f, 2.377f },
    { -1041.316f, -2932.84f, 19.172f, 0.733f },
    { -1045.101f, -2890.742f, 19.172f, 5.646f }
};

#define HEROIC_DIST_TO_VORTEX 21.0f
#define DIST_TO_SCARY_FOG_DOT 4.5f

class boss_lei_shi : public CreatureScript
{
public:
    boss_lei_shi() : CreatureScript("boss_lei_shi") { }

    struct boss_lei_shiAI : public BossAI
    {
        boss_lei_shiAI(Creature* creature) : BossAI(creature, DATA_LEI_SHI)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;
        EventMap events;

        std::list<Creature*> animatedProtectors;
        uint8 nextAfraidPct;
        uint8 nextProtectPct;

        uint8 endCombatPct;

        bool hidden;
        bool shielded;
        bool getAwayPhase;
        bool protectScheduled;
        bool leiShiFreed;

        bool canafraid;
        bool canprotect;

        float getAwayHealthPct;

        void Reset()
        {
            _Reset();
            events.Reset();

            summons.DespawnAll();

            hidden = false;
            getAwayPhase = false;
            shielded = false;
            protectScheduled = false;

            canafraid = true;
            canprotect = true;

            nextAfraidPct = 90;
            nextProtectPct = 80;

            me->RestoreDisplayId();
            me->CastSpell(me, SPELL_LEI_SHI_RITUAL_OF_PURIFICATION);
            me->RemoveAura(SPELL_AFRAID);
            me->RemoveAura(SPELL_HIDE);
            me->RemoveAura(SPELL_HIDE_STACKS);
            me->RemoveAura(SPELL_SCARY_FOG_CIRCLE);
            me->RemoveAura(SPELL_SCARY_FOG_DOT);
            me->RemoveAura(SPELL_SCARY_FOG_STACKS);
            me->RemoveAllAreaObjects();
        }

        void JustReachedHome()
        {
            _JustReachedHome();

            if (pInstance)
                pInstance->SetBossState(DATA_LEI_SHI, FAIL);
        }
        void SpellHitTarget(Unit* target, SpellInfo const* spell)
        {
            if (spell->Id == SPELL_SPRAY)
            {
                events.CancelEvent(EVENT_SPRAY);
                events.ScheduleEvent(EVENT_SPRAY, 400);
            }
        }
        void EnterCombat(Unit* attacker)
        {
            if (pInstance)
            {
                Reset();
                events.CancelEvent(EVENT_ENRAGE);

                //me->CastSpell(me, SPELL_AFRAID, true);
                pInstance->SetBossState(DATA_LEI_SHI, IN_PROGRESS);
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                DoZoneInCombat();

                //me->CastSpell(me, SPELL_SCARY_FOG_CIRCLE, true);

                Talk(TALK_AGGRO);

                me->SetReactState(REACT_AGGRESSIVE);
                me->ClearUnitState(UNIT_STATE_CANNOT_AUTOATTACK);

                events.CancelEvent(EVENT_SPRAY);
                // Events
                events.ScheduleEvent(EVENT_SPRAY, 2000);
                events.ScheduleEvent(EVENT_ENRAGE, 360000);
            }
        }
        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (!hidden)
                return;

            if (!caster)
                return;

            if (!spell)
                return;

            if (!spell->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE) &&
                !spell->HasEffect(SPELL_EFFECT_NORMALIZED_WEAPON_DMG) &&
                !spell->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE) &&
                !spell->HasEffect(SPELL_EFFECT_WEAPON_PERCENT_DAMAGE) &&
                !spell->HasEffect(SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL))
                return;

            if (auto hide = me->GetAura(SPELL_HIDE_STACKS))
            {
                hide->ModCharges(-1);

                switch (urand(0, 8))
                {
                    case 0:
                        me->NearTeleportTo(hidePositions[1].GetPositionX(),
                            hidePositions[1].GetPositionY(),
                            hidePositions[1].GetPositionZ(),
                            hidePositions[1].GetOrientation());
                        break;
                    case 1:
                        me->NearTeleportTo(hidePositions[2].GetPositionX(),
                            hidePositions[2].GetPositionY(),
                            hidePositions[2].GetPositionZ(),
                            hidePositions[2].GetOrientation());
                        break;
                    case 2:
                        me->NearTeleportTo(hidePositions[3].GetPositionX(),
                            hidePositions[3].GetPositionY(),
                            hidePositions[3].GetPositionZ(),
                            hidePositions[3].GetOrientation());
                        break;
                    case 3:
                        me->NearTeleportTo(hidePositions[0].GetPositionX(),
                            hidePositions[0].GetPositionY(),
                            hidePositions[0].GetPositionZ(),
                            hidePositions[0].GetOrientation());
                        break;
                }
            }
        }

        void JustDied(Unit* killer)
        {
            if (pInstance)
            {
                me->Respawn();
                me->SetHealth(me->GetMaxHealth());
                me->setFaction(35);


                me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation());
                summons.DespawnAll();
                pInstance->SetBossState(DATA_LEI_SHI, DONE);
                pInstance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                _JustDied();

                me->CastSpell(me, SPELL_LEI_SHI_TRANSFORM);
                Talk(TALK_DEFEATED);

                if (me->GetMap()->IsHeroic())
                    me->SummonGameObject(GOB_LEI_SHI_CHEST_HEROIC, -1017.58f, -2882.01f, 19.639f, 1.530082f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
                else
                    me->SummonGameObject(GOB_LEI_SHI_CHEST_NORMAL, -1017.58f, -2882.01f, 19.639f, 1.530082f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
            }
        }
        void DoAction(const int32 action)
        {
            if (action == ACTION_ANIMATED_PROTECTOR_DIED)
            {
                me->RemoveAura(SPELL_PROTECT);
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);

                shielded = false;
                summons.DespawnEntry(PROTECTOR);


                if (roll_chance_i(35))
                    events.ScheduleEvent(EVENT_HIDE, 5000);
                else
                    events.ScheduleEvent(EVENT_GET_AWAY, 5000);
            }
            if (action == ACTION_TERMINATE_GET_AWAY_PHASE)
            {
                getAwayPhase = false;
                me->RemoveAllAreaObjects();

                events.CancelEvent(EVENT_SPRAY);
                events.CancelEvent(EVENT_HIDE);
                events.CancelEvent(EVENT_GET_AWAY);

                events.ScheduleEvent(EVENT_SPRAY, 2000);

            }
            if (ACTION_TERMINATE_HIDE_PHASE)
            {
                me->RemoveAura(SPELL_HIDE);
                me->RemoveAura(SPELL_HIDE_STACKS);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RestoreDisplayId();
                hidden = false;

                //me->CastSpell(me, SPELL_SCARY_FOG_CIRCLE, true);

                // Only have Lei Shi (hidden) in summons
                summons.DespawnEntry(63099);

                events.CancelEvent(EVENT_HIDDEN_SPRAY);
                events.CancelEvent(EVENT_HIDE);
                events.CancelEvent(EVENT_GET_AWAY);
                events.CancelEvent(EVENT_SPRAY);

                events.ScheduleEvent(EVENT_SPRAY, 2000);
            }
        }
        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }
        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }
        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Talk(TALK_SLAY);
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
        {
            // afraid
            if (nextAfraidPct)
            {
                if (me->HealthBelowPctDamaged(nextAfraidPct, damage))
                {
                    if (me->HasAura(SPELL_AFRAID))
                    {
                        if (auto afraid = me->GetAuraEffect(SPELL_AFRAID, EFFECT_0))
                            afraid->ChangeAmount(afraid->GetAmount() + 6);

                        nextAfraidPct -= 10;
                    }
                    else
                    {
                        me->AddAura(SPELL_AFRAID, me);
                        nextAfraidPct -= 10;
                    }
                }
            }
            // protect
            if (nextProtectPct)
            {
                if (me->HealthBelowPctDamaged(nextProtectPct, damage))
                {
                    events.ScheduleEvent(EVENT_PROTECT, 500);
                    nextProtectPct -= 20;
                    canprotect = true;
                }
            }
            // get away
            if (getAwayPhase && me->HealthBelowPctDamaged(int32(getAwayHealthPct - 4.0f), damage))
            {
                me->RemoveAura(SPELL_GET_AWAY);

                me->RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
                events.ScheduleEvent(EVENT_SPRAY, 2000);
                getAwayPhase = false;
            }

        }
        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
                return;


            switch (events.ExecuteEvent())
            {
            case EVENT_SPRAY:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SPELL_SPRAY);
                break;
            }
            case EVENT_PROTECT:
            {
                me->CastSpell(me, SPELL_PROTECT, true);
                shielded = true;

                for (int i = 0; i <= 3; i++)
                {
                    Position pos;


                    me->GetRandomNearPosition(pos, frand(8.0f, 25.0f));
                    Creature* protectors = me->SummonCreature(PROTECTOR, pos, TEMPSUMMON_MANUAL_DESPAWN);
                }
                break;
            }
            case EVENT_HIDE:
            {
                events.CancelEvent(EVENT_SPRAY);
                events.ScheduleEvent(EVENT_HIDDEN_SPRAY, 2000);

                me->CastSpell(me->getVictim(), SPELL_HIDE_SUMMON);

                hidden = true;
                Talk(TALK_HIDE);

                me->CastSpell(me, SPELL_HIDE);
                break;

            }
            case EVENT_GET_AWAY:
            {
                if ((me->HasUnitState(UNIT_STATE_CASTING) && !getAwayPhase))
                {
                    events.ScheduleEvent(EVENT_GET_AWAY, 0);
                    break;
                }

                events.CancelEvent(EVENT_SPRAY);

                // Teleport Lei Shi to the center of the room
                Talk(TALK_GET_AWAY);
                me->CastSpell(me, SPELL_GET_AWAY_TELEPORT, true);
                getAwayPhase = true;
                getAwayHealthPct = me->GetHealthPct();
                events.ScheduleEvent(EVENT_GET_AWAY_CAST, 300);
                break;
            }
            case EVENT_GET_AWAY_CAST:
            {
                me->SetReactState(REACT_PASSIVE);
                me->AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
				me->CastSpell(me, SPELL_GET_AWAY);

                break;
            }
            case EVENT_ENRAGE:
                me->CastSpell(me, 26662);
                break;
            }

        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_lei_shiAI(creature);
    }
};

// Animated Protector - 62995
class mob_animated_protector : public CreatureScript
{
public:
    mob_animated_protector() : CreatureScript("mob_animated_protector") { }

    struct mob_animated_protectorAI : public ScriptedAI
    {
        mob_animated_protectorAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->Respawn();
        }

        InstanceScript* pInstance;

        void Reset()
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }
        void JustDied(Unit* killer)
        {
            if (Creature* leiShi = me->GetMap()->GetCreature(pInstance->GetGuidData(NPC_LEI_SHI)))
                if (leiShi->GetAI())
                    leiShi->AI()->DoAction(ACTION_ANIMATED_PROTECTOR_DIED);
        }
        void UpdateAI(const uint32 diff)
        {
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_animated_protectorAI(creature);
    }
};

// Get Away ! - 123461
class spell_get_away : public SpellScriptLoader
{
public:
    spell_get_away() : SpellScriptLoader("spell_get_away") { }

    class spell_get_away_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_get_away_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                if (caster->ToCreature())
                    if (caster->ToCreature()->AI())
                        caster->ToCreature()->AI()->DoAction(ACTION_TERMINATE_GET_AWAY_PHASE);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_get_away_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_get_away_AuraScript();
    }
};

// Hide - 123244
class spell_hide : public SpellScriptLoader
{
public:
    spell_hide() : SpellScriptLoader("spell_hide") { }

    class spell_hide_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hide_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->CastSpell(caster, SPELL_HIDE_STACKS, true);

                uint8 pos = urand(0, 3);

                caster->NearTeleportTo(hidePositions[pos].GetPositionX(),
                    hidePositions[pos].GetPositionY(),
                    hidePositions[pos].GetPositionZ(),
                    hidePositions[pos].GetOrientation());

                caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                caster->SetDisplayId(11686);
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                caster->RemoveAura(SPELL_HIDE_STACKS);
                caster->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                caster->RestoreDisplayId();
            }
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_hide_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_hide_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hide_AuraScript();
    }
};

// Hide (stacks) - 123233
class spell_hide_stacks : public SpellScriptLoader
{
public:
    spell_hide_stacks() : SpellScriptLoader("spell_hide_stacks") { }

    class spell_hide_stacks_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hide_stacks_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                if (caster->ToCreature())
                    if (caster->ToCreature()->AI())
                        caster->ToCreature()->AI()->DoAction(ACTION_TERMINATE_HIDE_PHASE);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_hide_stacks_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_hide_stacks_AuraScript();
    }
};

// Scary Fog (DoT) - 123705
class spell_scary_fog_dot : public SpellScriptLoader
{
public:
    spell_scary_fog_dot() : SpellScriptLoader("spell_scary_fog_dot") { }

    class spell_scary_fog_dot_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_scary_fog_dot_SpellScript);

        void CorrectRange(std::list<WorldObject*>& targets)
        {
            targets.clear();

            Map::PlayerList const& players = GetCaster()->GetMap()->GetPlayers();
            if (!players.isEmpty())
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->getSource())
                        if (player->GetExactDist2d(GetCaster()->GetPositionX(), GetCaster()->GetPositionY()) >= HEROIC_DIST_TO_VORTEX)
                            targets.push_back(player);

            for (auto itr : targets)
            {
                if (itr->ToUnit())
                {
                    if (auto scaryFog = GetCaster()->AddAura(SPELL_SCARY_FOG_STACKS, itr->ToUnit()))
                    {
                        scaryFog->SetDuration(35000);
                        scaryFog->SetMaxDuration(35000);
                    }
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_scary_fog_dot_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_scary_fog_dot_SpellScript();
    }

    class spell_scary_fog_dot_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_scary_fog_dot_AuraScript);

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* target = GetTarget())
                target->CastSpell(target, SPELL_SCARY_FOG_STACKS, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_scary_fog_dot_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_scary_fog_dot_AuraScript();
    }
};

// Scary Fog (stacks) - 123712
class spell_scary_fog_stacks : public SpellScriptLoader
{
public:
    spell_scary_fog_stacks() : SpellScriptLoader("spell_scary_fog_stacks") { }

    class spell_scary_fog_stacks_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_scary_fog_stacks_SpellScript);

        void CorrectTargets(std::list<WorldObject*>& targets)
        {
            for (auto itr : targets)
            {
                if (itr->ToUnit() && itr->ToUnit()->GetEntry() != NPC_LEI_SHI_HIDDEN)
                {
                    if (auto scary = GetCaster()->GetAura(SPELL_SCARY_FOG_STACKS))
                    {
                        if (auto scaryTarget = itr->ToUnit()->GetAura(SPELL_SCARY_FOG_STACKS))
                            scaryTarget->SetStackAmount(scary->GetStackAmount());
                        else if (auto scaryTarget = GetCaster()->AddAura(SPELL_SCARY_FOG_STACKS, itr->ToUnit()))
                            scaryTarget->SetStackAmount(scary->GetStackAmount());
                    }
                }
            }

            targets.clear();
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_scary_fog_stacks_SpellScript::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_scary_fog_stacks_SpellScript();
    }
};
// Lei Shi (hidden) - 63099
class mob_lei_shi_hidden : public CreatureScript
{
public:
    mob_lei_shi_hidden() : CreatureScript("mob_lei_shi_hidden") { }

    struct mob_lei_shi_hiddenAI : public ScriptedAI
    {
        mob_lei_shi_hiddenAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetUnitMovementFlags(MOVEMENTFLAG_ROOT);
            events.Reset();
            events.ScheduleEvent(EVENT_HIDDEN_SPRAY, 400);

            if (pInstance)
            {
                if (Creature* leiShi = me->GetMap()->GetCreature(pInstance->GetGuidData(NPC_LEI_SHI)))
                {
                    auto afraid = leiShi->GetAuraEffect(SPELL_AFRAID, EFFECT_0);
                    if (!afraid)
                        return;

                    if (!me->HasAura(SPELL_AFRAID))
                    {
                        if (auto newAfraid = me->AddAura(SPELL_AFRAID, me))
                            if (newAfraid->GetEffect(0))
                                newAfraid->GetEffect(0)->ChangeAmount(afraid->GetAmount());
                    }
                    else
                    {
                        if (auto newAfraid = me->GetAuraEffect(SPELL_AFRAID, EFFECT_0))
                            newAfraid->ChangeAmount(afraid->GetAmount());
                    }
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
            case EVENT_HIDDEN_SPRAY:
            {
                if (me->HasUnitState(UNIT_STATE_CASTING))
                {
                    events.ScheduleEvent(EVENT_HIDDEN_SPRAY, 0);
                    break;
                }

                ObjectGuid leiShiGuid = ObjectGuid::Empty;
                if (pInstance)
                    leiShiGuid = pInstance->GetGuidData(NPC_LEI_SHI);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f))
                    me->CastSpell(target, SPELL_SPRAY, false, NULL, nullptr, leiShiGuid);

                events.ScheduleEvent(EVENT_HIDDEN_SPRAY, 400);
                break;
            }
            default:
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_lei_shi_hiddenAI(creature);
    }
};

void AddSC_boss_lei_shi()
{
    // boss
    new boss_lei_shi();
    // mobs
    new mob_animated_protector();
    new mob_lei_shi_hidden();
    // spells
    new spell_get_away();
    new spell_hide();
    new spell_hide_stacks();
    new spell_scary_fog_dot();
    new spell_scary_fog_stacks();
}