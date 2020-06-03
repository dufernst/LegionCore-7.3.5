/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/*
 * Scripts for spells with SPELLFAMILY_PALADIN and SPELLFAMILY_GENERIC spells used by paladin players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pal_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Group.h"
#include "GridNotifiers.h"
#include "AreaTriggerAI.h"
#include "AreaTrigger.h"

enum PaladinSpells
{
    PALADIN_SPELL_FORBEARANCE                    = 25771,
    PALADIN_SPELL_HOLY_PRISM_ALLIES              = 114871,
    PALADIN_SPELL_HOLY_PRISM_ENNEMIES            = 114852,
    PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL       = 114862,
    PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2     = 114870,
    PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL         = 121551,
    PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2       = 121552,
    PALADIN_SPELL_ARCING_LIGHT_HEAL              = 119952,
    PALADIN_SPELL_ARCING_LIGHT_DAMAGE            = 114919,
    PALADIN_SPELL_ARDENT_DEFENDER_HEAL           = 66235,

    HealingStorm                                 = 193058,
    HealingStormHeal                             = 215257,
    DivineTempestAura                            = 186773,
    DivineTempest                                = 186775,
    DivineStormDamage                            = 224239,
    BlessingOfTheAshbringer                      = 242981,
    GreaterBlessingOfKings                       = 203538,
    GreaterBlessingOfWisdom                      = 203539,
    BeaconOfLight                                = 53563,
    BeaconOfFaith                                = 156910,
    BeaconOfVirtue                               = 200025,
    TheLightSavesBuff                            = 200423,
    TheLightSavesDebuff                          = 211426,
    DevotionAura                                 = 210320,
    AuraOfSacrifice                              = 210372,

    HealingStormMaxTarget                        = 6,
    UpdateTimer                                  = 2000,
};

// 6940 - Hand of Sacrifice
class spell_pal_hand_of_sacrifice : public AuraScript
{
    PrepareAuraScript(spell_pal_hand_of_sacrifice);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & /*dmgInfo*/, float & absorbAmount)
    {
        absorbAmount = 0;
    }

    void SplitDamage(AuraEffect* aurEff, DamageInfo & dmgInfo, float& absorbAmount)
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->GetHealthPct() < GetSpellInfo()->Effects[EFFECT_2]->BasePoints)
                aurEff->GetBase()->Remove();
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_hand_of_sacrifice::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_hand_of_sacrifice::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectSplitDamage += AuraEffectAbsorbFn(spell_pal_hand_of_sacrifice::SplitDamage, EFFECT_0, SPELL_AURA_SPLIT_DAMAGE_PCT);
    }
};

// Shield of the Righteous - 53600
class spell_pal_shield_of_the_righteous : public SpellScriptLoader
{
    public:
        spell_pal_shield_of_the_righteous() : SpellScriptLoader("spell_pal_shield_of_the_righteous") { }

        class spell_pal_shield_of_the_righteous_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_shield_of_the_righteous_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* spellShield = caster->GetAura(132403))
                    {
                        spellShield->SetDuration(spellShield->GetDuration() + spellShield->GetMaxDuration());
                        spellShield->RecalculateAmountOfEffects(true);
                    }
                    else
                        caster->CastSpell(caster, 132403, true);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_shield_of_the_righteous_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_shield_of_the_righteous_SpellScript();
        }
};

// Divine Tempest - 186773
class areatrigger_at_divine_tempest : public AreaTriggerScript
{
    public:
    areatrigger_at_divine_tempest() : AreaTriggerScript("areatrigger_at_divine_tempest") {}

    struct areatrigger_at_divine_tempestAI : AreaTriggerAI
    {
        areatrigger_at_divine_tempestAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        bool hasHealingStorm;
        std::vector<uint64> targetGUID;

        void OnCreate() override
        {
            targetGUID.clear();

            if (Unit* caster = at->GetCaster())
            {
                if (Unit* owner = caster->GetOwner())
                {
                    hasHealingStorm = owner->HasAura(HealingStorm);
                }
                else
                {
                    hasHealingStorm = caster->HasAura(HealingStorm);
                }
            }
        }

        void OnUnitEnter(Unit* unit) override
        {
            if (Unit* caster = at->GetCaster())
            {
                if (unit->isTotem() || unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
                    return;

                if (!caster->IsValidAttackTarget(unit))
                {
                    if (hasHealingStorm && targetGUID.size() < HealingStormMaxTarget)
                    {
                        uint64 curGUID = unit->GetGUID().GetGUIDLow();

                        for (auto itr : targetGUID)
                        {
                            if (itr == curGUID)
                                return;
                        }

                        Unit* paladinCaster = caster;

                        if (Unit* owner = caster->GetOwner())
                            paladinCaster = owner;

                        if (paladinCaster->IsInRaidWith(unit))
                        {
                            paladinCaster->CastSpell(unit, HealingStormHeal, true);
                            targetGUID.push_back(curGUID);
                        }
                    }

                    return;
                }

                caster->CastSpell(unit, DivineStormDamage, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER), NULL, NULL, caster->GetOwnerGUID());
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_divine_tempestAI(areatrigger);
    }
};

// Divine Storm - 53385
class spell_pal_divine_storm : public SpellScriptLoader
{
    public:
    spell_pal_divine_storm() : SpellScriptLoader("spell_pal_divine_storm") {}

    class spell_pal_divine_storm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_divine_storm_SpellScript);

        bool hasDivineTempest;
        bool hasHealingStorm;

        void HandleBeforeCast()
        {
            hasDivineTempest = false;

            if (Unit* caster = GetCaster())
            {
                if (Unit* owner = caster->GetOwner())
                {
                    if (owner->HasAura(DivineTempestAura))
                    {
                        caster->CastSpell(caster, DivineTempest, true);
                        hasDivineTempest = true;
                    }
                    else
                    {
                        hasHealingStorm = owner->HasAura(HealingStorm);
                    }
                }
                else
                {
                    if (caster->HasAura(DivineTempestAura))
                    {
                        caster->CastSpell(caster, DivineTempest, true);
                        hasDivineTempest = true;
                    }
                    else
                    {
                        hasHealingStorm = caster->HasAura(HealingStorm);
                    }
                }
            }
        }

        void HandleOnCast()
        {
            if (Unit* caster = GetCaster())
            {
                if (!hasDivineTempest)
                    caster->SendPlaySpellVisualKit(0, 73892);
            }
        }

        void FilterEnemy(std::list<WorldObject*>& unitList)
        {
            if (hasDivineTempest)
                unitList.clear();
        }

        void FilterTarget(std::list<WorldObject*>& unitList)
        {
            if (Unit* caster = GetCaster())
            {
                if (hasDivineTempest || !hasHealingStorm)
                {
                    unitList.clear();
                    unitList.push_back(caster);
                    return;
                }

                unitList.sort(Trinity::UnitHealthState(true));

                if (unitList.size() > HealingStormMaxTarget)
                {
                    std::list<WorldObject*> tempList = unitList;
                    uint8 targetCount = 0;
                    unitList.clear();

                    for (auto itr : tempList)
                    {
                        if (targetCount >= HealingStormMaxTarget)
                            break;

                        unitList.push_back(itr);
                        targetCount++;
                    }
                }
            }
        }

        void HandleHeal(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetHitUnit())
                {
                    if (Unit* owner = caster->GetOwner())
                        caster = owner;

                    if (hasHealingStorm)
                        caster->CastSpell(target, HealingStormHeal, true);
                }
            }
        }

        void HandleDamage(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetHitUnit())
                    caster->CastSpell(target, DivineStormDamage, true, NULL, NULL, caster->GetOwnerGUID());
            }
        }

        void Register() override
        {
            BeforeCast += SpellCastFn(spell_pal_divine_storm_SpellScript::HandleBeforeCast);
            OnCast += SpellCastFn(spell_pal_divine_storm_SpellScript::HandleOnCast);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_SpellScript::FilterEnemy, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_divine_storm_SpellScript::FilterTarget, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
            OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_DUMMY);
            OnEffectHitTarget += SpellEffectFn(spell_pal_divine_storm_SpellScript::HandleHeal, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pal_divine_storm_SpellScript();
    }
};

// Hand of Protection - 1022 and Blessing of Spellwarding - 204018
class spell_pal_hand_of_protection : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_protection() : SpellScriptLoader("spell_pal_hand_of_protection") { }

        class spell_pal_hand_of_protection_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_hand_of_protection_SpellScript);

            SpellCastResult CheckForbearance()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                {
                    if (target->HasAura(PALADIN_SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;
                    if (!caster || (caster->HasUnitState(UNIT_STATE_CONTROLLED) && caster != target))
                        return SPELL_FAILED_BAD_TARGETS;
                }

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, PALADIN_SPELL_FORBEARANCE, true);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_hand_of_protection_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_hand_of_protection_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_hand_of_protection_SpellScript();
        }
};

// Divine Shield - 642
class spell_pal_divine_shield : public SpellScriptLoader
{
    public:
        spell_pal_divine_shield() : SpellScriptLoader("spell_pal_divine_shield") { }

        class spell_pal_divine_shield_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_shield_SpellScript);

            SpellCastResult CheckForbearance()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(PALADIN_SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, PALADIN_SPELL_FORBEARANCE, true);
            }

            //void HandleHeal(SpellEffIndex /*effIndex*/)
            //{
            //    if(Unit* caster = GetCaster())
            //    {
            //        if(caster->HasAura(146956))
            //        {
            //            if(uint32 count = GetSpell()->GetCountDispel())
            //            {
            //                int32 _heal = caster->CountPctFromMaxHealth(10);
            //                if(count > 5)
            //                    count = 5;
            //                SetHitHeal(int32(_heal * count));
            //            }
            //        }
            //    }
            //}

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_divine_shield_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_divine_shield_SpellScript::HandleOnHit);
                //OnEffectHitTarget += SpellEffectFn(spell_pal_divine_shield_SpellScript::HandleHeal, EFFECT_3, SPELL_EFFECT_HEAL_PCT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_divine_shield_SpellScript();
        }
};

// Light's Hammer (periodic dummy for npc) - 114918
class spell_pal_lights_hammer : public SpellScriptLoader
{
    public:
        spell_pal_lights_hammer() : SpellScriptLoader("spell_pal_lights_hammer") { }

        class spell_pal_lights_hammer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_lights_hammer_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (GetCaster())
                {
                    if (GetCaster()->GetOwner())
                    {
                        GetCaster()->CastSpell(GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), PALADIN_SPELL_ARCING_LIGHT_HEAL, true, 0, NULL, GetCaster()->GetOwner()->GetGUID());
                        GetCaster()->CastSpell(GetCaster()->GetPositionX(), GetCaster()->GetPositionY(), GetCaster()->GetPositionZ(), PALADIN_SPELL_ARCING_LIGHT_DAMAGE, true, 0, NULL, GetCaster()->GetOwner()->GetGUID());
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_lights_hammer_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_lights_hammer_AuraScript();
        }
};

// called by Holy Prism (heal) - 114871
// Holy Prism visual for other targets
class spell_pal_holy_prism_heal : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism_heal() : SpellScriptLoader("spell_pal_holy_prism_heal") { }

        class spell_pal_holy_prism_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_heal_SpellScript);

            std::list<WorldObject*> targetList;
            Unit* unitTarget;

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    if (unitTarget)
                        unitTarget->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2, true);
                }
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                unitTarget = GetHitUnit();
            }

            void FilterEnemy(std::list<WorldObject*>& unitList)
            {
                if (Unit* caster = GetCaster())
                {
                    for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end();)
                    {
                        if (caster->IsFriendlyTo((*itr)->ToUnit()))
                            unitList.erase(itr++);
                        else
                            ++itr;
                    }
                }

                if(unitList.size() > 5)
                    Trinity::Containers::RandomResizeList(unitList, 5);
                targetList = unitList;
            }

            void FilterScript(std::list<WorldObject*>& unitList)
            {
                unitList.clear();
                unitList = targetList;
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_heal_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_heal_SpellScript::FilterEnemy, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_heal_SpellScript::FilterScript, EFFECT_2, TARGET_UNIT_DEST_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_prism_heal_SpellScript();
        }
};

// called by Holy Prism (damage) - 114852
// Holy Prism visual for other targets
class spell_pal_holy_prism_damage : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism_damage() : SpellScriptLoader("spell_pal_holy_prism_damage") { }

        class spell_pal_holy_prism_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_damage_SpellScript);

            std::list<WorldObject*> targetList;
            Unit* unitTarget;

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                unitTarget = GetHitUnit();
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    if (unitTarget)
                        unitTarget->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2, true);
            }

            void FilterEnemy(std::list<WorldObject*>& unitList)
            {
                if (Unit* caster = GetCaster())
                {
                    for (std::list<WorldObject*>::iterator itr = unitList.begin(); itr != unitList.end();)
                    {
                        if (caster->IsFriendlyTo((*itr)->ToUnit()))
                            ++itr;
                        else
                            unitList.erase(itr++);
                    }
                }

                if(unitList.size() > 5)
                    Trinity::Containers::RandomResizeList(unitList, 5);
                targetList = unitList;
            }

            void FilterScript(std::list<WorldObject*>& unitList)
            {
                unitList.clear();
                unitList = targetList;
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnEffectHitTarget += SpellEffectFn(spell_pal_holy_prism_damage_SpellScript::HandleHeal, EFFECT_1, SPELL_EFFECT_HEAL);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_damage_SpellScript::FilterEnemy, EFFECT_1, TARGET_UNIT_DEST_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_holy_prism_damage_SpellScript::FilterScript, EFFECT_2, TARGET_UNIT_DEST_AREA_ENTRY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_prism_damage_SpellScript();
        }
};

// called by Holy Prism (visual damage) - 114862 or Holy Prism (visual heal) - 121551
// Holy Prism (damage) - 114852 or Holy Prism (heal) - 114871
class spell_pal_holy_prism_effect : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism_effect() : SpellScriptLoader("spell_pal_holy_prism_effect") { }

        class spell_pal_holy_prism_effect_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_effect_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // damage
                        if (GetSpellInfo()->Id == 114862)
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_ENNEMIES, true);
                        // heal
                        else if (GetSpellInfo()->Id == 121551)
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_ALLIES, true);
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_holy_prism_effect_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_prism_effect_SpellScript();
        }
};

// Holy Prism - 114165
class spell_pal_holy_prism : public SpellScriptLoader
{
    public:
        spell_pal_holy_prism() : SpellScriptLoader("spell_pal_holy_prism") { }

        class spell_pal_holy_prism_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_prism_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (_player->IsFriendlyTo(target))
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_HEAL_VISUAL_2, true);
                        }
                        else
                        {
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL, true);
                            _player->CastSpell(target, PALADIN_SPELL_HOLY_PRISM_DAMAGE_VISUAL_2, true);
                        }
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_holy_prism_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_prism_SpellScript();
        }
};

// Ardent Defender - 31850
class spell_pal_ardent_defender : public SpellScriptLoader
{
    public:
        spell_pal_ardent_defender() : SpellScriptLoader("spell_pal_ardent_defender") { }

        class spell_pal_ardent_defender_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_ardent_defender_AuraScript);

            uint32 absorbPct, healPct;

            bool Load() override
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue();
                absorbPct = GetSpellInfo()->Effects[EFFECT_0]->CalcValue();
                return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                Unit* victim = GetTarget();
                int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
                // If damage kills us
                if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(PALADIN_SPELL_ARDENT_DEFENDER_HEAL))
                {
                    // Cast healing spell, completely avoid damage
                    absorbAmount = dmgInfo.GetDamage();

                    float healAmount = int32(victim->CountPctFromMaxHealth(healPct));
                    victim->CastCustomSpell(victim, PALADIN_SPELL_ARDENT_DEFENDER_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
                    victim->ToPlayer()->AddSpellCooldown(PALADIN_SPELL_ARDENT_DEFENDER_HEAL, 0, getPreciseTime() + 120.0);
                }
                else
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_ardent_defender_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_ardent_defender_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_ardent_defender_AuraScript();
        }
};

// Lay on Hands - 633
class spell_pal_lay_on_hands : public SpellScriptLoader
{
    public:
        spell_pal_lay_on_hands() : SpellScriptLoader("spell_pal_lay_on_hands") { }

        class spell_pal_lay_on_hands_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lay_on_hands_SpellScript);

            SpellCastResult CheckForbearance()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                    if (target->HasAura(PALADIN_SPELL_FORBEARANCE))
                        return SPELL_FAILED_TARGET_AURASTATE;

                return SPELL_CAST_OK;
            }

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetHitUnit())
                        _player->CastSpell(target, PALADIN_SPELL_FORBEARANCE, true);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_lay_on_hands_SpellScript::CheckForbearance);
                OnHit += SpellHitFn(spell_pal_lay_on_hands_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_lay_on_hands_SpellScript();
        }
};

// Holy Shock - 20473
class spell_pal_holy_shock : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock() : SpellScriptLoader("spell_pal_holy_shock") { }

        class spell_pal_holy_shock_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_SpellScript);

            SpellCastResult CheckCast()
            {
                if(Unit* caster = GetCaster())
                {
                    Unit* target = GetExplTargetUnit();
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(25912);
                    if(target && spellInfo && !target->IsFriendlyTo(caster))
                        if((spellInfo->CastingReq.FacingCasterFlags & SPELL_FACING_FLAG_INFRONT) && !caster->HasInArc(static_cast<float>(M_PI), target))
                            return SPELL_FAILED_UNIT_NOT_INFRONT;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pal_holy_shock_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_shock_SpellScript();
        }
};

// Holy Shield - 152261
class spell_pal_holy_shield : public SpellScriptLoader
{
    public:
        spell_pal_holy_shield() : SpellScriptLoader("spell_pal_holy_shield") { }

        class spell_pal_holy_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_holy_shield_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                if (Unit* caster = GetCaster())
                {
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), caster->GetBlockPercent()); // when blocking is absorbed 30% dmg or 40% when caster has perc 157488
                    Unit* attacker = dmgInfo.GetAttacker();
                    if (attacker != caster)
                    {
                        if (roll_chance_f(caster->GetUnitBlockChance(BASE_ATTACK, caster)))
                            caster->CastSpell(attacker, 157122, true);
                        else
                            absorbAmount = 0;
                    }
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_holy_shield_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_holy_shield_AuraScript::Absorb, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_holy_shield_AuraScript();
        }
};

// Divine Intervention - 213313
class spell_pal_divine_intervention : public SpellScriptLoader
{
    public:
        spell_pal_divine_intervention() : SpellScriptLoader("spell_pal_divine_intervention") { }

        class spell_pal_divine_intervention_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_divine_intervention_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* target = GetTarget())
                {
                    if (dmgInfo.GetDamage() < target->GetHealth())
                        return;

                    if (target->ToPlayer()->HasSpellCooldown(642) || target->HasAura(PALADIN_SPELL_FORBEARANCE))
                        return;

                    uint32 triggerFlags = TRIGGERED_FULL_MASK;
                    triggerFlags &= ~TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD;

                    target->CastSpell(target, 642, TriggerCastFlags(triggerFlags));
                    uint32 health20 = target->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->CalcValue(target));

                    // hp > 20% - absorb hp till 20%
                    if (target->GetHealth() > health20)
                        absorbAmount = dmgInfo.GetDamage() - target->GetHealth() + health20;
                    // hp lower than 10% - absorb everything
                    else
                        absorbAmount = dmgInfo.GetDamage();

                    float bp = health20 - target->GetHealth();
                    if (bp > 0)
                        target->CastCustomSpell(target, 184250, &bp, nullptr, nullptr, true);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_divine_intervention_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_divine_intervention_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_divine_intervention_AuraScript();
        }
};

// Greater Blessing of Kings - 203538
class spell_pal_greater_blessing_of_kings : public SpellScriptLoader
{
    public:
        spell_pal_greater_blessing_of_kings() : SpellScriptLoader("spell_pal_greater_blessing_of_kings") { }

        class spell_pal_greater_blessing_of_kings_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_greater_blessing_of_kings_AuraScript);

            uint32 absorb = 0;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
                if (Unit* caster = GetCaster())
                    absorb = int32(caster->GetSpellPowerDamage(SPELL_SCHOOL_MASK_HOLY) * 2.7f);
            }

            void CalculateDummy(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                    amount = int32(caster->GetSpellPowerDamage(SPELL_SCHOOL_MASK_HOLY) * 2.7f);
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (!absorb)
                    return;

                if (dmgInfo.GetDamage() > absorb)
                {
                    absorbAmount = absorb;
                    absorb = 0;
                }
                else
                {
                    absorbAmount = dmgInfo.GetDamage();
                    absorb -= dmgInfo.GetDamage();
                }
                if (AuraEffect* effect = GetAura()->GetEffect(EFFECT_0))
                    effect->ChangeAmount(absorb);
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                    absorb = int32(caster->GetSpellPowerDamage(SPELL_SCHOOL_MASK_HOLY) * 2.7f);
                if (AuraEffect* effect = GetAura()->GetEffect(EFFECT_0))
                    effect->ChangeAmount(absorb);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_greater_blessing_of_kings_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_greater_blessing_of_kings_AuraScript::CalculateDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_greater_blessing_of_kings_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_greater_blessing_of_kings_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_greater_blessing_of_kings_AuraScript();
        }
};

// Shield of Vengeance - 184662
class spell_pal_shield_of_vengeance : public SpellScriptLoader
{
    public:
        spell_pal_shield_of_vengeance() : SpellScriptLoader("spell_pal_shield_of_vengeance") { }

        class spell_pal_shield_of_vengeance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_shield_of_vengeance_AuraScript);

            float absorbA = 0.f;
            float absorbMax = 0.f;
            bool startRemove = false;

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                {
                    if (amount)
                        amount = absorbMax = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * GetSpellInfo()->Effects[EFFECT_1]->BasePoints) * amount;
                    else
                        amount = absorbMax = int32(caster->GetTotalAttackPowerValue(BASE_ATTACK) * GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
                }
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (startRemove)
                    return;
                startRemove = true;
                if (Unit* caster = GetCaster())
                    if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
                        caster->CastCustomSpell(caster, 184689, &absorbA, nullptr, nullptr, true);
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbA += dmgInfo.GetDamage();

                if (absorbA > absorbMax)
                    absorbA = absorbMax;
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_shield_of_vengeance_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_shield_of_vengeance_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_shield_of_vengeance_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_shield_of_vengeance_AuraScript();
        }
};

// Zeal - 217020
class spell_pal_zeal : public SpellScriptLoader
{
    public:
        spell_pal_zeal() : SpellScriptLoader("spell_pal_zeal") { }

        class spell_pal_zeal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_zeal_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (GetHitUnit() != GetExplTargetUnit())
                {
                    int32 perc = 100 - (GetSpellInfo()->Effects[EFFECT_5]->CalcValue(caster) * (GetSpell()->GetTargetCount() - 2));
                    if (perc <= 0)
                        SetHitDamage(CalculatePct(GetHitDamage(), 20));
                    else
                        SetHitDamage(CalculatePct(GetHitDamage(), perc));
                }

            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_zeal_SpellScript::HandleOnHit, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_zeal_SpellScript();
        }
};

// Divine Steed - 190784
class spell_pal_divine_steed : public SpellScriptLoader
{
    public:
        spell_pal_divine_steed() : SpellScriptLoader("spell_pal_divine_steed") { }

        class spell_pal_divine_steed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_divine_steed_SpellScript);

            void HandleOnHit()
            {
                if (auto caster = GetCaster())
                {
                    if (caster->HasAura(231996)) // Glyph of the Trusted Steed
                        caster->CastSpell(caster, 221883, true);
                    else if (caster->HasAura(254465)) // Glyph of the Valorous Charger's Bridle
                        caster->CastSpell(caster, 254471, true);
                    else if (caster->HasAura(254467)) // Glyph of the Vengeful Charger's Bridle
                        caster->CastSpell(caster, 254472, true);
                    else if (caster->HasAura(254469)) // Glyph of the Vigilant Charger's Bridle
                        caster->CastSpell(caster, 254473, true);
                    else if (caster->HasAura(254475)) // Glyph of the Golden Charger's Bridle
                        caster->CastSpell(caster, 254474, true);
                    else
                    {
                        switch (caster->getRace())
                        {
                            case RACE_TAUREN:
                                caster->CastSpell(caster, 221885, true);
                                break;
                            case RACE_BLOODELF:
                                caster->CastSpell(caster, 221886, true);
                                break;
                            case RACE_DRAENEI:
                            case RACE_LIGHTFORGED_DRAENEI:
                                caster->CastSpell(caster, 221887, true);
                                break;
                            default:
                                caster->CastSpell(caster, 221883, true);
                                break;
                        }
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_divine_steed_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_divine_steed_SpellScript();
        }
};

// Holy Wrath - 210220
class spell_pal_holy_wrath : public SpellScriptLoader
{
    public:
        spell_pal_holy_wrath() : SpellScriptLoader("spell_pal_holy_wrath") { }

        class spell_pal_holy_wrath_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_wrath_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* target = GetHitUnit())
                {
                    int32 bp = caster->GetMaxHealth() - caster->GetHealth();
                    if (bp > 0)
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            SetHitDamage(CalculatePct(bp, GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster)));
                        else
                            SetHitDamage(CalculatePct(bp, GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster)));
                    }
                }
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_holy_wrath_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_wrath_SpellScript();
        }
};

// Light of the Protector - 184092 and EXtalent Hand of the Protector - 213652
class spell_pal_light_of_the_protector : public SpellScriptLoader
{
    public:
        spell_pal_light_of_the_protector() : SpellScriptLoader("spell_pal_light_of_the_protector") { }

        class spell_pal_light_of_the_protector_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_light_of_the_protector_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                int32 bp = target->GetMaxHealth() - target->GetHealth();
                if (bp > 0)
                    SetEffectValue(CalculatePct(bp, GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster)));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_light_of_the_protector_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_light_of_the_protector_SpellScript();
        }
};

// Last Defender - 203791
class spell_pal_last_defender : public AuraScript
{
    PrepareAuraScript(spell_pal_last_defender);

    void CalculateAmountAbsorb(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        // Set absorbtion amount to unlimited
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
    {
        if (Aura* aura = GetAura())
        {
            if (AuraEffect const* eff = aura->GetEffect(EFFECT_4))
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), eff->GetAmount());
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_last_defender::CalculateAmountAbsorb, EFFECT_3, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_last_defender::Absorb, EFFECT_3, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// Last Defender - 203791
struct spell_pal_at_last_defender : public AreaTriggerAI
{
    spell_pal_at_last_defender(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    enum MyEnum
    {
        Timer = 500
    };

    int32 timeRemaining = 0;
    uint32 spellId;

    void OnCreate() override
    {
        spellId = at->GetSpellId();
    }

    void OnUnitEnter(Unit* /*unit*/) override
    {
        if (!timeRemaining)
            timeRemaining = Timer;
    }

    void OnUnitExit(Unit* /*unit*/) override
    {
        if (!timeRemaining)
            timeRemaining = Timer;
    }

    void OnUpdate(uint32 diff) override
    {
        if (!timeRemaining)
            return;

        timeRemaining -= diff;

        if (timeRemaining <= 0)
        {
            if (Unit* caster = at->GetCaster())
            {
                if (AuraEffect* eff = caster->GetAuraEffect(spellId, EFFECT_4))
                {
                    float amount = (1.f - pow(0.97f, at->GetAffectedPlayers()->size())) * 100.f;
                    eff->ChangeAmount(amount, false);
                }
            }

            timeRemaining = 0;
        }
    }
};

// Aura of Sacrifice - 183416
struct spell_pal_at_aura_of_sacrifice : public AreaTriggerAI
{
    spell_pal_at_aura_of_sacrifice(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    enum MyEnum
    {
        LongeTimer = 3000,
        Timer = 500,
    };

    uint32 spellId = AuraOfSacrifice;
    int32 timeRemaining = Timer;
    bool handleAura = true;
    ObjectGuid casterGUID = ObjectGuid::Empty;

    virtual void InitVariables(Unit* caster) {}
    virtual void SetAuraMastery(bool apply) {}
    virtual void HandlePlayerCount(Unit* caster, GuidList* affectedList) {}

    void OnCreate() override
    {
        if (Unit* caster = at->GetCaster())
        {
            casterGUID = caster->GetGUID();
            InitVariables(caster);
        }
    }

    bool CheckTarget(Unit* caster, Unit* target)
    {
        if (target->GetGUID() == casterGUID || caster->IsFriendlyTo(target))
            return true;

        return false;
    }

    bool CreateUnitList(std::list<Unit *>& unitList) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                if (Group* group = plr->GetGroup())
                {
                    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                    {
                        if (Player* player = itr->getSource())
                        {
                            if (player->isAlive())
                                unitList.push_back(player);
                        }
                    }
                }
                else
                    unitList.push_back(caster);
            }
        }
        return true;
    }

    void RemoveAuras(Unit* caster, bool removeInAffectedList)
    {
        std::list<Unit*> targetList;
        std::vector<uint32> auraList = {spellId};
        caster->TargetsWhoHasMyAuras(targetList, auraList);
        GuidList& affectedPlayers = *at->GetAffectedPlayers();
        caster->RemoveAurasDueToSpell(spellId, casterGUID);

        for (auto itr : targetList)
        {
            itr->RemoveAurasDueToSpell(spellId, casterGUID);

            if (removeInAffectedList)
                affectedPlayers.remove(itr->GetGUID());
        }
    }

    bool IsValidTarget(Unit* caster, Unit* target, AreaTriggerActionMoment /*actionM*/) override
    {
        if (!caster || !target || !caster->isAlive() || !CheckTarget(caster, target))
            return false;

        return true;
    }

    inline void SetTimer()
    {
        if (!handleAura)
        {
            if (timeRemaining > Timer)
                timeRemaining = Timer;

            handleAura = true;
        }
    }

    uint32 CallSpecialFunction(uint32 Num) override
    {
        SetAuraMastery(Num);
        SetTimer();
        return NULL;
    }

    void OnUnitEnter(Unit* /*unit*/) override
    {
        SetTimer();
    }

    void OnUnitExit(Unit* /*unit*/) override
    {
        SetTimer();
    }

    void OnRemove() override
    {
        if (Unit* caster = at->GetCaster())
            RemoveAuras(caster, false);
    }

    virtual void HandleAuras(Unit* caster, GuidList* affectedList)
    {
        for (auto affectedPlayer : *affectedList)
        {
            if (Unit* target = ObjectAccessor::GetUnit(*at, affectedPlayer))
                caster->CastSpell(target, AuraOfSacrifice, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
        }
    }

    void OnUpdate(uint32 diff) override
    {
        timeRemaining -= diff;

        if (timeRemaining <= 0)
        {
            timeRemaining = LongeTimer;

            if (Unit* caster = at->GetCaster())
            {
                if (!caster->isAlive())
                {
                    RemoveAuras(caster, true);
                    return;
                }

                if (handleAura)
                {
                    handleAura = false;
                    std::list<ObjectGuid> targetList;
                    std::vector<uint32> auraList = {spellId};
                    caster->TargetsWhoHasMyAuras(targetList, auraList);
                    GuidList* affectedPlayers = at->GetAffectedPlayers();

                    for (auto itr : targetList)
                    {
                        bool findTarget = false;

                        for (auto affectedPlayer : *affectedPlayers)
                        {
                            if (affectedPlayer == itr)
                            {
                                findTarget = true;
                                break;
                            }
                        }

                        if (!findTarget)
                        {
                            if (Unit* target = ObjectAccessor::GetUnit(*at, itr))
                                target->RemoveAurasDueToSpell(spellId, casterGUID);
                        }
                    }
                    HandleAuras(caster, affectedPlayers);
                }
                else
                {
                    GuidList& affectedPlayers = *at->GetAffectedPlayers();

                    for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
                    {
                        next = itr;
                        ++next;

                        if (Unit* target = ObjectAccessor::GetUnit(*at, *itr))
                        {
                            if (!CheckTarget(caster, target))
                            {
                                target->RemoveAurasDueToSpell(spellId, casterGUID);
                                affectedPlayers.erase(itr);
                            }
                        }
                    }
                    HandlePlayerCount(caster, &affectedPlayers);
                }
            }
        }
    }
};

// Devotion Aura - 183425
struct spell_pal_at_devotion_aura : public spell_pal_at_aura_of_sacrifice
{
    spell_pal_at_devotion_aura(AreaTrigger* areatrigger) : spell_pal_at_aura_of_sacrifice(areatrigger) {}

    float curAmount = 0.f;
    float baseAmount = 0.f;
    uint8 lastPlayerCount = 0;
    bool hasAuraMastery = false;

    void InitVariables(Unit* caster) override
    {
        baseAmount = at->GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
        spellId = DevotionAura;
        hasAuraMastery = caster->HasAura(31821);
    }

    void SetAuraMastery(bool apply) override
    {
        hasAuraMastery = apply;
    }

    void HandleAuras(Unit* caster, GuidList* affectedList) override
    {
       curAmount = hasAuraMastery ? baseAmount : (baseAmount / affectedList->size());

        for (auto affectedPlayer : *affectedList)
        {
            if (Unit* target = ObjectAccessor::GetUnit(*at, affectedPlayer))
            {
                if (AuraEffect* eff = target->GetAuraEffect(DevotionAura, EFFECT_0, casterGUID))
                {
                    eff->SetAmount(curAmount);
                }
                else
                {
                    caster->CastCustomSpell(target, DevotionAura, &curAmount, NULL, NULL, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
                }
            }
        }
    }

    void HandlePlayerCount(Unit* caster, GuidList* affectedList) override
    {
        if (lastPlayerCount != affectedList->size())
        {
            lastPlayerCount = affectedList->size();
            HandleAuras(caster, affectedList);
        }
    }
};

// Seraphim - 152262
class spell_pal_seraphim : public SpellScriptLoader
{
    public:
        spell_pal_seraphim() : SpellScriptLoader("spell_pal_seraphim") { }

        class spell_pal_seraphim_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_seraphim_AuraScript);

            void CalculateMaxDuration(int32& duration)
            {
                duration = 2000; // Default
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    int32 charges = 0;
                    if(SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(53600)) // Shield of the Righteous
                        charges = _player->GetChargesForSpell(spellInfo);
                    if (charges >= 2)
                    {
                        duration += 8000 * 2;
                        _player->ModSpellCharge(53600, -2);
                    }
                    else if (charges == 1)
                    {
                        duration += 8000;
                        _player->ModSpellCharge(53600, -1);
                    }
                }
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_pal_seraphim_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_seraphim_AuraScript();
        }
};

// Aura of Sacrifice - 210372
class spell_pal_aura_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_aura_of_sacrifice() : SpellScriptLoader("spell_pal_aura_of_sacrifice") { }

        class spell_pal_aura_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_aura_of_sacrifice_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* caster = GetCaster())
                    if (caster->GetHealthPct() > 75.0f && sSpellMgr->GetSpellInfo(183416))
                    {
                        absorbAmount = CalculatePct(dmgInfo.GetDamage(), sSpellMgr->GetSpellInfo(183416)->Effects[EFFECT_0]->BasePoints);
                        float directDamage = absorbAmount;
                        if (Unit* target = GetTarget())
                            target->CastCustomSpell(caster, 210380, &directDamage, nullptr, nullptr, true, nullptr, aurEff);
                    }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_aura_of_sacrifice_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_aura_of_sacrifice_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_aura_of_sacrifice_AuraScript();
        }
};

// Aegisjalmur, the Armguards of Awe - 225036
class spell_pal_aegisjalmur_the_armguards_of_awe : public SpellScriptLoader
{
    public:
        spell_pal_aegisjalmur_the_armguards_of_awe() : SpellScriptLoader("spell_pal_aegisjalmur_the_armguards_of_awe") { }

        class spell_pal_aegisjalmur_the_armguards_of_awe_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_aegisjalmur_the_armguards_of_awe_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                Unit* target = GetCaster();
                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                if (target->HasAura(225056))
                    return;

                if (target->HasAura(137027)) // Only for Retributions
                {
                    target->CastSpell(target, 225056, true);
                    float bp0 = 2.f;
                    target->CastCustomSpell(target, 184662, &bp0, NULL, NULL, true, NULL, aurEff);
                    absorbAmount = dmgInfo.GetDamage() - (target->GetHealth() - 1);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_aegisjalmur_the_armguards_of_awe_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_aegisjalmur_the_armguards_of_awe_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_aegisjalmur_the_armguards_of_awe_AuraScript();
        }
};

// Wake of Ashes - 205273
class spell_pal_wake_of_ashes : public SpellScriptLoader
{
    public:
        spell_pal_wake_of_ashes() : SpellScriptLoader("spell_pal_wake_of_ashes") { }

        class spell_pal_wake_of_ashes_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_wake_of_ashes_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                {
                    if (target->GetCreatureType() == CREATURE_TYPE_DEMON || target->GetCreatureType() == CREATURE_TYPE_UNDEAD)
                        GetCaster()->CastSpell(target, 205290, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pal_wake_of_ashes_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_wake_of_ashes_SpellScript();
        }
};

// Judgment - 20271
class spell_pal_judgment : public SpellScriptLoader
{
    public:
        spell_pal_judgment() : SpellScriptLoader("spell_pal_judgment") { }

        class spell_pal_judgment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_judgment_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster && !target)
                    return;

                if (AuraEffect* aurEff = caster->GetAuraEffect(231657, EFFECT_0)) // Judgment (lvl 2 for Protection)
                {
                    int32 second = aurEff->GetAmount() * IN_MILLISECONDS;
                    if (GetSpell()->IsCritForTarget(target))
                        second *= 2;
                    if (Player* _player = caster->ToPlayer())
                        _player->ModSpellChargeCooldown(53600, second);
                }

                if (caster->HasAura(231663)) // Judgment (lvl 2 for Retribution)
                {
                    caster->CastSpell(target, 197277, true);
                }

                if (caster->HasAura(231644)) // Judgment (lvl 2 for Holy)
                {
                    caster->CastSpell(target, 214222, true);
                }
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* _target = GetExplTargetUnit())
                    {
                        if (caster->HasAura(246806)) // Lawbringer (PvP Talent)
                        {
                            caster->CastSpell(_target, 246867, true);
                            caster->CastSpell(_target, 246807, true);
                        }
                    }
                }
            }
            void HandleJump(int32& AddJumpTarget)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(218178, EFFECT_1)) // Greater Judgment
                        AddJumpTarget += aurEff->GetAmount();

                    if (AuraEffect const* aurEff = caster->GetAuraEffect(231661, EFFECT_0)) // Judgment
                        AddJumpTarget += aurEff->GetAmount();

                    if (AuraEffect const* aurEff = caster->GetAuraEffect(238134, EFFECT_0)) // Judge Unworthy
                    {
                        if (Unit* unitTarget = GetExplTargetUnit())
                        {
                            if (unitTarget->HasAura(197277))
                            {
                                if (roll_chance_i(aurEff->GetAmount()))
                                    AddJumpTarget += 1;
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_pal_judgment_SpellScript::HandleOnCast);
                OnEffectHitTarget += SpellEffectFn(spell_pal_judgment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnObjectJumpTarget += SpellObjectJumpTargetFn(spell_pal_judgment_SpellScript::HandleJump, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_judgment_SpellScript();
        }
};

// Blessing of the Ashbringer - 238098
class spell_pal_blessing_of_the_ashbringer : public SpellScriptLoader
{
    public:
    spell_pal_blessing_of_the_ashbringer() : SpellScriptLoader("spell_pal_blessing_of_the_ashbringer") {}

    class spell_pal_blessing_of_the_ashbringer_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_blessing_of_the_ashbringer_AuraScript);

        int32 timer = UpdateTimer;

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            Remove(GetUnitOwner());
        }

        uint32 CallSpecialFunction(uint32 /*Num*/) override
        {
            if (!timer)
                timer = UpdateTimer;

            return 1;
        }

        void Apply(Unit* owner)
        {
            if (!owner->HasAura(BlessingOfTheAshbringer))
                owner->CastSpell(owner, BlessingOfTheAshbringer, true);
        }

        void Remove(Unit* owner)
        {
            owner->RemoveAurasDueToSpell(BlessingOfTheAshbringer);
        }

        void OnUpdate(uint32 diff)
        {
            if (!timer)
                return;

            timer -= diff;

            if (timer <= 0)
            {
                timer = 0;

                if (Unit* owner = GetUnitOwner())
                {
                    uint8 auraAmount = 0;

                    for (auto itr : owner->m_whoHasMyAuras)
                    {
                        if (auraAmount > 0)
                            break;

                        for (auto v : itr.second)
                        {
                            if (v == GreaterBlessingOfWisdom || v == GreaterBlessingOfKings)
                                auraAmount++;
                        }
                    }

                    switch (auraAmount)
                    {
                        case 0:
                        {
                            ObjectGuid ownerGUID = owner->GetGUID();

                            if (owner->HasAura(GreaterBlessingOfWisdom, ownerGUID) && owner->HasAura(GreaterBlessingOfKings, ownerGUID))
                            {
                                Apply(owner);
                                return;
                            }
                        }
                        case 1:
                        {
                            Remove(owner);
                            return;
                        }
                        default:
                        {
                            Apply(owner);
                            return;
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnAuraUpdate += AuraUpdateFn(spell_pal_blessing_of_the_ashbringer_AuraScript::OnUpdate);
            OnEffectRemove += AuraEffectRemoveFn(spell_pal_blessing_of_the_ashbringer_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_blessing_of_the_ashbringer_AuraScript();
    }
};

// The Light Saves - 200421
class spell_pal_the_light_saves_aura : public SpellScriptLoader
{
    public:
    spell_pal_the_light_saves_aura() : SpellScriptLoader("spell_pal_the_light_saves_aura") {}

    class spell_pal_the_light_saves_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_the_light_saves_aura_AuraScript);

        int32 timer = UpdateTimer;
        bool debuffChecked = false;

        void OnUpdate(uint32 diff)
        {
            timer -= diff;

            if (timer <= 0)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!debuffChecked)
                    {
                        if (Aura* debuff = caster->GetAura(TheLightSavesDebuff))
                            timer = debuff->GetDuration();

                        debuffChecked = true;
                        return;
                    }

                    std::list<Unit*> targetList;
                    std::vector<uint32> auraList = {BeaconOfLight, BeaconOfFaith, BeaconOfVirtue};
                    caster->TargetsWhoHasMyAuras(targetList, auraList);

                    bool bProc = false;
                    float pct = GetSpellInfo()->Effects[EFFECT_0]->BasePoints;

                    for (auto target : targetList)
                    {
                        if (target->GetHealthPct() < pct)
                        {
                            bProc = true;
                            break;
                        }
                    }

                    if (!bProc && caster->GetHealthPct() < pct)
                    {
                        ObjectGuid const& cGUID = caster->GetGUID();

                        if (caster->HasAura(BeaconOfLight, cGUID) || caster->HasAura(BeaconOfFaith, cGUID) || caster->HasAura(BeaconOfVirtue, cGUID))
                            bProc = true;
                    }
                    
                    if (bProc)
                    {
                        caster->CastSpell(caster, TheLightSavesBuff, true);
                        
                        if (Aura* aura = caster->AddAura(TheLightSavesDebuff, caster))
                        {
                            timer += aura->GetMaxDuration();
                            return;
                        }
                    }
                }
                timer += UpdateTimer;
            }
        }

        void Register() override
        {
            OnAuraUpdate += AuraUpdateFn(spell_pal_the_light_saves_aura_AuraScript::OnUpdate);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_the_light_saves_aura_AuraScript();
    }
};

// Blessed Hammer - 229976
class spell_pal_blessed_hammer : public SpellScriptLoader
{
    public:
        spell_pal_blessed_hammer() : SpellScriptLoader("spell_pal_blessed_hammer") { }

        class spell_pal_blessed_hammer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_blessed_hammer_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                Unit* caster = GetCaster();
                Unit* attacker = dmgInfo.GetAttacker();
                if (!caster || !attacker)
                    return;

                AuraEffect* aurEff = attacker->GetAuraEffect(204301, EFFECT_1);
                if (!aurEff || !aurEff->GetBase() || caster != aurEff->GetBase()->GetCaster())
                    return;

                absorbAmount = CalculatePct(dmgInfo.GetDamage(), aurEff->GetAmount());
                aurEff->GetBase()->Remove();
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_blessed_hammer_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_blessed_hammer_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_blessed_hammer_AuraScript();
        }
};

// Hardiness (Honor Talent) - 195416, 232047, 232243
class spell_all_hardiness : public SpellScriptLoader
{
    public:
        spell_all_hardiness() : SpellScriptLoader("spell_all_hardiness") { }

        class spell_all_hardiness_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_all_hardiness_AuraScript);

            uint32 absorbPct;
            uint32 healthPct;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster());
                healthPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (GetTarget()->GetHealthPct() >= healthPct)
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_all_hardiness_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_all_hardiness_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_all_hardiness_AuraScript();
        }
};

// Sparring (Honor Talent) - 195425, 232045, 232245
class spell_all_sparring : public SpellScriptLoader
{
    public:
        spell_all_sparring() : SpellScriptLoader("spell_all_sparring") { }

        class spell_all_sparring_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_all_sparring_AuraScript);

            uint32 absorbPct;
            uint32 _chance;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                _chance = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster());
                absorbPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (roll_chance_i(_chance))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_all_sparring_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_all_sparring_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_all_sparring_AuraScript();
        }
};

// Lawbringer (Honor Talent) - 246867
class spell_pal_lawbringer : public SpellScriptLoader
{
    public:
        spell_pal_lawbringer() : SpellScriptLoader("spell_pal_lawbringer") { }

        class spell_pal_lawbringer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_lawbringer_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster())));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_pal_lawbringer_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_lawbringer_SpellScript();
        }
};

// Hand of Hindrance - 183218
class spell_pal_hand_of_hindrance : public SpellScriptLoader
{
    public:
        spell_pal_hand_of_hindrance() : SpellScriptLoader("spell_pal_hand_of_hindrance") { }

        class spell_pal_hand_of_hindrance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_hand_of_hindrance_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* _player = GetCaster()->ToPlayer())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode != AURA_REMOVE_BY_EXPIRE)
                        if (AuraEffect const* aurEff = _player->GetAuraEffect(204934, EFFECT_0)) // Law and Order (Honor Talent)
                            _player->ModifySpellCooldown(183218, aurEff->GetAmount() * -1000);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_hand_of_hindrance_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_hand_of_hindrance_AuraScript();
        }
};

// Cleanse - 4987
class spell_pal_cleanse : public SpellScriptLoader
{
    public:
        spell_pal_cleanse() : SpellScriptLoader("spell_pal_cleanse") { }

        class spell_pal_cleanse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_cleanse_SpellScript);

            void OnSuccessfulDispel(SpellEffIndex /*effIndex*/)
            {
                auto target = GetHitUnit();
                auto caster = GetCaster();
                if (!target || !caster)
                    return;

                float distance = caster->HasAura(31821) ? 40.0f : 10.0f; 
                if (caster->HasAura(199330)) // Cleanse the Weak (Honor Talent)
                {
                    if (caster->GetDistance(target) <= distance)
                        caster->CastSpell(caster, 199360, true);
                }              
            }

            void Register() override
            {
                OnEffectSuccessfulDispel += SpellEffectFn(spell_pal_cleanse_SpellScript::OnSuccessfulDispel, EFFECT_0, SPELL_EFFECT_DISPEL);
                OnEffectSuccessfulDispel += SpellEffectFn(spell_pal_cleanse_SpellScript::OnSuccessfulDispel, EFFECT_1, SPELL_EFFECT_DISPEL);
                OnEffectSuccessfulDispel += SpellEffectFn(spell_pal_cleanse_SpellScript::OnSuccessfulDispel, EFFECT_2, SPELL_EFFECT_DISPEL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_cleanse_SpellScript();
        }
};

// Blessing of Sacrifice - 199448
class spell_pal_blessing_of_sacrifice : public SpellScriptLoader
{
    public:
        spell_pal_blessing_of_sacrifice() : SpellScriptLoader("spell_pal_blessing_of_sacrifice") { }

        class spell_pal_blessing_of_sacrifice_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_blessing_of_sacrifice_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = dmgInfo.GetDamage();
                float bp0 = absorbAmount / 4;

                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* aurEf = caster->GetAuraEffect(200327, EFFECT_0))
                        bp0 += CalculatePct(bp0, aurEf->GetAmount());

                    caster->CastCustomSpell(caster, 199450, &bp0, nullptr, nullptr, true, nullptr, aurEff);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_blessing_of_sacrifice_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_blessing_of_sacrifice_AuraScript::Absorb, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_blessing_of_sacrifice_AuraScript();
        }
};

// Cleanse the Weak (Honor Talent) - 199360
class spell_pal_cleanse_the_weak : public SpellScriptLoader
{
    public:
        spell_pal_cleanse_the_weak() : SpellScriptLoader("spell_pal_cleanse_the_weak") { }

        class spell_pal_cleanse_the_weak_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_cleanse_the_weak_SpellScript);

            void FilterEnemy(std::list<WorldObject*>& targetList)
            {
                std::list<WorldObject*> targetListFilter;
                if (Unit* caster = GetCaster())
                {
                    float distance = caster->HasAura(31821) ? 40.0f : 10.0f; 
                    for (std::list<WorldObject*>::iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                        if (caster->GetDistance(*itr) <= distance)
                            targetListFilter.push_back(*itr);
                }
                targetList = targetListFilter;
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_cleanse_the_weak_SpellScript::FilterEnemy, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_cleanse_the_weak_SpellScript::FilterEnemy, EFFECT_1, TARGET_UNIT_SRC_AREA_ALLY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_cleanse_the_weak_SpellScript::FilterEnemy, EFFECT_2, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_cleanse_the_weak_SpellScript();
        }
};

// Divine Shield | Guardian of the Forgotten Queen(Honor Talent) - 228050
class spell_pal_guardian_of_the_forgotten_queen : public SpellScriptLoader
{
    public:
    spell_pal_guardian_of_the_forgotten_queen() : SpellScriptLoader("spell_pal_guardian_of_the_forgotten_queen") {}

    class spell_pal_guardian_of_the_forgotten_queen_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_pal_guardian_of_the_forgotten_queen_SpellScript);

        void FilterTarget(std::list<WorldObject*>& targetList)
        {
            targetList.clear();

            if (Unit* caster = GetCaster())
            {
                if (Unit* owner = caster->GetOwner())
                {
                    for (auto j : owner->m_whoHasMyAuras)
                    {
                        for (auto v : j.second)
                        {
                            if (v == 228049)
                            {
                                if (Unit* target = ObjectAccessor::GetUnit(*caster, j.first))
                                    targetList.push_back(target);

                                return;
                            }
                        }
                    }

                    targetList.push_back(owner);
                }
                    
            }
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pal_guardian_of_the_forgotten_queen_SpellScript::FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_pal_guardian_of_the_forgotten_queen_SpellScript();
    }
};

// Unbreakable Will - 182497
class spell_pal_unbreakable_will : public AuraScript
{
    PrepareAuraScript(spell_pal_unbreakable_will);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            Unit::AuraList mechanicAuras;
            caster->GetAuraEffectsByMechanic(25588774, mechanicAuras);
            if (!mechanicAuras.empty())
            {
                caster->CastSpell(caster, 182496, true);
                caster->CastSpell(caster, 182531, true);
            }
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_pal_unbreakable_will::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Holy Shock - 25914
class spell_pal_holy_shock_heal : public SpellScriptLoader
{
    public:
        spell_pal_holy_shock_heal() : SpellScriptLoader("spell_pal_holy_shock_heal") { }

        class spell_pal_holy_shock_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_holy_shock_heal_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(200657, EFFECT_0)) // Power of the Silver Hand
                    {
                        SetHitHeal(GetHitHeal() + aurEff->GetAmount());
                        aurEff->GetBase()->Remove();
                    }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_holy_shock_heal_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_holy_shock_heal_SpellScript();
        }
};

// Flash of Light - 19750, Holy Light - 82326
class spell_pal_the_light_saves : public SpellScriptLoader
{
    public:
        spell_pal_the_light_saves() : SpellScriptLoader("spell_pal_the_light_saves") { }

        class spell_pal_the_light_saves_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_the_light_saves_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(TheLightSavesBuff, EFFECT_0)) // The Light Saves
                    {
                        SetHitHeal(GetHitHeal() + CalculatePct(GetHitHeal(), aurEff->GetAmount()));
                        aurEff->GetBase()->Remove();
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_the_light_saves_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_the_light_saves_SpellScript();
        }
};

// Forbearance - 25771
class spell_pal_forbearance : public SpellScriptLoader
{
    public:
        spell_pal_forbearance() : SpellScriptLoader("spell_pal_forbearance") { }

        class spell_pal_forbearance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_forbearance_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(209376, EFFECT_0)) // Forbearant Faithful
                        aurEff->ChangeAmount(-(aurEff->GetSpellInfo()->Effects[EFFECT_1]->BasePoints));
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(209376, EFFECT_0)) // Forbearant Faithful
                        aurEff->ChangeAmount(0);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_pal_forbearance_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_forbearance_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_forbearance_AuraScript();
        }
};

// Defender of Truth - 240059
class spell_pal_defender_of_truth : public SpellScriptLoader
{
    public:
        spell_pal_defender_of_truth() : SpellScriptLoader("spell_pal_defender_of_truth") { }

        class spell_pal_defender_of_truth_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_defender_of_truth_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(238097, EFFECT_0))
                        amount = caster->CountPctFromMaxHealth(aurEff->GetAmount());
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_defender_of_truth_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_defender_of_truth_AuraScript();
        }
};

// Aggramar's Stride - 207438
class spell_pal_aggramars_stride : public SpellScriptLoader
{
    public:
        spell_pal_aggramars_stride() : SpellScriptLoader("spell_pal_aggramars_stride") { }

        class spell_pal_aggramars_stride_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_aggramars_stride_AuraScript);

            float lastMax = 0.f;

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (Player* _player = caster->ToPlayer())
                {
                    float maxData = 0.0f;
                    float mastery = _player->GetFloatValue(PLAYER_FIELD_MASTERY) + _player->GetRatingBonusValue(CR_MASTERY);
                    float haste = (_player->GetFloatValue(UNIT_FIELD_MOD_HASTE) * 100.0f) - 100.0f;
                    float versatility = _player->GetFloatValue(PLAYER_FIELD_VERSATILITY);
                    float crit = _player->GetFloatValue(PLAYER_FIELD_CRIT_PERCENTAGE);

                    if(mastery > maxData)
                        maxData = mastery;
                    if(haste > maxData)
                        maxData = haste;
                    if(versatility > maxData)
                        maxData = versatility;
                    if(crit > maxData)
                        maxData = crit;

                    int32 maxDataInt32 = int32(maxData * 0.75f);
                    if (lastMax != maxDataInt32)
                    {
                        lastMax = maxDataInt32;
                        if (lastMax > 40.f)
                            lastMax = 40.f;
                        {
                            if (AuraEffect* aurEff = caster->GetAuraEffect(226974, EFFECT_0))
                                aurEff->ChangeAmount(lastMax);
                            else
                                caster->CastCustomSpell(caster, 226974, &lastMax, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pal_aggramars_stride_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_aggramars_stride_AuraScript();
        }
};

// Archimonde's Hatred Reborn - 235169
class spell_pal_archimondes_hatred_reborn : public SpellScriptLoader
{
    public:
        spell_pal_archimondes_hatred_reborn() : SpellScriptLoader("spell_pal_archimondes_hatred_reborn") { }

        class spell_pal_archimondes_hatred_reborn_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_archimondes_hatred_reborn_AuraScript);

            float absorb = 0.f;
            bool startRemove = false;

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                    amount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->BasePoints);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (startRemove)
                    return;
                startRemove = true;
                if (Unit* caster = GetCaster())
                    caster->CastCustomSpell(caster, 235188, &absorb, nullptr, nullptr, true);
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = dmgInfo.GetDamage();
                absorb += CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_pal_archimondes_hatred_reborn_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_archimondes_hatred_reborn_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pal_archimondes_hatred_reborn_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_archimondes_hatred_reborn_AuraScript();
        }
};

// Power of the Silver Hand - 200657
class spell_pal_power_of_the_silver_hand : public SpellScriptLoader
{
    public:
        spell_pal_power_of_the_silver_hand() : SpellScriptLoader("spell_pal_power_of_the_silver_hand") { }

        class spell_pal_power_of_the_silver_hand_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_power_of_the_silver_hand_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                amount += aurEff->GetOldBaseAmount();
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_power_of_the_silver_hand_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_power_of_the_silver_hand_AuraScript();
        }
};

// Greater Blessing of Wisdom - 203539
class spell_pal_greater_blessing_of_wisdom : public SpellScriptLoader
{
    public:
    spell_pal_greater_blessing_of_wisdom() : SpellScriptLoader("spell_pal_greater_blessing_of_wisdom") {}

    class spell_pal_greater_blessing_of_wisdom_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_pal_greater_blessing_of_wisdom_AuraScript);

        void HandleTick(AuraEffect const* aurEff, float& amount, Unit* /*target*/)
        {
            amount /= GetSpellInfo()->Effects[EFFECT_2]->BasePoints * IN_MILLISECONDS / float(aurEff->GetPeriod());
        }

        void Register() override
        {
            DoEffectChangeTickDamage += AuraEffectChangeTickDamageFn(spell_pal_greater_blessing_of_wisdom_AuraScript::HandleTick, EFFECT_0, SPELL_AURA_OBS_MOD_POWER);
            DoEffectChangeTickDamage += AuraEffectChangeTickDamageFn(spell_pal_greater_blessing_of_wisdom_AuraScript::HandleTick, EFFECT_1, SPELL_AURA_OBS_MOD_HEALTH);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_pal_greater_blessing_of_wisdom_AuraScript();
    }
};

// Bulwark of Order - 209388
class spell_pal_bulwark_of_order : public SpellScriptLoader
{
    public:
        spell_pal_bulwark_of_order() : SpellScriptLoader("spell_pal_bulwark_of_order") { }

        class spell_pal_bulwark_of_order_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pal_bulwark_of_order_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                if(Unit* caster = GetCaster())
                {
                    amount += aurEff->GetOldBaseAmount();
                    float pct20 = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->BasePoints); // 20% from max HP
                    if (pct20 < amount)
                        amount = pct20;
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pal_bulwark_of_order_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pal_bulwark_of_order_AuraScript();
        }
};

// Avenger Shield - 31935
class spell_pal_first_avenger : public SpellScriptLoader
{
    public:
        spell_pal_first_avenger() : SpellScriptLoader("spell_pal_first_avenger") { }
 
        class spell_pal_first_avenger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pal_first_avenger_SpellScript);
 
            void HandleOnHit()
            {
                if (Player* plr = GetCaster()->ToPlayer())
                {
                    if (plr->GetSelectedUnit() == GetHitUnit())
                    {
                        if (AuraEffect* aurEff = plr->GetAuraEffect(203776, EFFECT_0))
                        {
                            int32 damage = GetHitDamage();
                            damage += CalculatePct(damage, aurEff->GetAmount());
                            SetHitDamage(damage);
                        }
                    }
                }
            }
 
            void Register() override
            {
                OnHit += SpellHitFn(spell_pal_first_avenger_SpellScript::HandleOnHit);
            }
        };
 
        SpellScript* GetSpellScript() const override
        {
            return new spell_pal_first_avenger_SpellScript();
        }
};

// Hammer of Reckoning (PvP Talent) - 247675
class spell_pal_hammer_of_reckoning : public SpellScript
{
    PrepareSpellScript(spell_pal_hammer_of_reckoning);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
        {
            if (Aura* aur = caster->GetAura(247677))
            {
                if (aur->GetStackAmount() == GetSpellInfo()->Effects[EFFECT_1]->BasePoints)
                {
                    if (caster->HasSpell(231895))
                        caster->CastSpellDuration(caster, 231895, true, GetSpellInfo()->Effects[EFFECT_3]->BasePoints * IN_MILLISECONDS);
                    else
                        caster->CastSpellDuration(caster, 31884, true, GetSpellInfo()->Effects[EFFECT_2]->BasePoints * IN_MILLISECONDS);
                }
            }
            caster->RemoveAurasDueToSpell(247677);
            caster->RemoveAurasDueToSpell(247676);
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_pal_hammer_of_reckoning::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Item - Paladin T21 Protection 2P Bonus - 251869
class spell_pal_t21_prot : public AuraScript
{
    PrepareAuraScript(spell_pal_t21_prot);

    int32 update = 1000;

    void OnUpdate(uint32 diff)
    {
        if (!update)
            return;

        update -= diff;

        if (update <= 0)
        {
            if (auto caster = GetCaster())
            {
                float bp = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster);
                if (AuraEffect const* aurEff = caster->GetAuraEffect(251870, EFFECT_0)) // Item - Paladin T21 Protection 4P Bonus
                {
                    if (!caster->HasAura(132403))
                        bp += CalculatePct(bp, aurEff->GetAmount());
                }

                caster->CastCustomSpell(caster, 253331, &bp, nullptr, nullptr, true);
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
        {
            caster->RemoveAurasDueToSpell(253331);
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_pal_t21_prot::OnUpdate);
        OnEffectRemove += AuraEffectRemoveFn(spell_pal_t21_prot::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Divine Vision (Honor Talent) - 199324
class spell_pal_divine_vision : public AuraScript
{
    PrepareAuraScript(spell_pal_divine_vision);

    virtual void HandleSpecialFunction(AreaTrigger* AT, bool apply) {}

    void HandleAuras(bool apply)
    {
        if (Unit* caster = GetCaster())
        {
            if (AreaTrigger* AT = caster->GetAreaObject(caster->HasAura(183425) ? 183425 : 183416))
            {
                AT->CalculateRadius();
                HandleSpecialFunction(AT, apply);
            }
        }
    }

    void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        HandleAuras(true);
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        HandleAuras(false);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_pal_divine_vision::AfterApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_pal_divine_vision::AfterRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

// Aura Mastery - 31821
class spell_pal_aura_mastery : public spell_pal_divine_vision
{
    PrepareAuraScript(spell_pal_aura_mastery);

    void HandleSpecialFunction(AreaTrigger* AT, bool apply) override
    {
        if (AreaTriggerAI* ATAI = AT->AI())
            ATAI->CallSpecialFunction(apply);
    }
};

// Tyr's Deliverance - 214855
class spell_pal_tyr_deliverance : public AuraScript
{
    PrepareAuraScript(spell_pal_tyr_deliverance);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* target = eventInfo.GetProcTarget())
        {
            if (target->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_tyr_deliverance::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

// Wake of Ashes - 214083
class spell_pal_wake_of_ashes_proc : public AuraScript
{
    PrepareAuraScript(spell_pal_wake_of_ashes_proc);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetProcTarget())
            {
                if (caster->GetMapId() == 1220 || caster->GetMapId() == 1669)
                {
                    if (Creature* cre = target->ToCreature())
                    {
                        if (CreatureTemplate const* cinfo = cre->GetCreatureTemplate())
                        {
                            if (cinfo->Classification == CREATURE_CLASSIFICATION_NORMAL && (target->GetCreatureType() == CREATURE_TYPE_UNDEAD || target->GetCreatureType() == CREATURE_TYPE_DEMON))
                                if (target->GetHealthPct() <= 20)
                                    caster->CastSpell(target, 180324, true);
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pal_wake_of_ashes_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

void AddSC_paladin_spell_scripts()
{
    RegisterAuraScript(spell_pal_hand_of_sacrifice);
    new spell_pal_shield_of_the_righteous();
    new spell_pal_hand_of_protection();
    new spell_pal_divine_shield();
    new spell_pal_lights_hammer();
    new spell_pal_holy_prism_heal();
    new spell_pal_holy_prism_damage();
    new spell_pal_holy_prism_effect();
    new spell_pal_holy_prism();
    new spell_pal_ardent_defender();
    new spell_pal_lay_on_hands();
    new spell_pal_holy_shock();
    new spell_pal_holy_shield();
    new spell_pal_divine_intervention();
    new spell_pal_greater_blessing_of_kings();
    new spell_pal_shield_of_vengeance();
    new spell_pal_zeal();
    new spell_pal_divine_steed();
    new spell_pal_holy_wrath();
    new spell_pal_light_of_the_protector();
    new spell_pal_seraphim();
    new spell_pal_aura_of_sacrifice();
    new spell_pal_aegisjalmur_the_armguards_of_awe();
    new spell_pal_wake_of_ashes();
    new spell_pal_judgment();
    new spell_pal_blessed_hammer();
    new spell_all_hardiness();
    new spell_all_sparring();
    new spell_pal_lawbringer();
    new spell_pal_hand_of_hindrance();
    new spell_pal_cleanse();
    new spell_pal_blessing_of_sacrifice();
    new spell_pal_cleanse_the_weak();
    RegisterAuraScript(spell_pal_unbreakable_will);
    new spell_pal_holy_shock_heal();
    new spell_pal_the_light_saves();
    new spell_pal_forbearance();
    new spell_pal_defender_of_truth();
    new spell_pal_aggramars_stride();
    new spell_pal_archimondes_hatred_reborn();
    new spell_pal_power_of_the_silver_hand();
    new spell_pal_bulwark_of_order();
    new spell_pal_divine_storm();
    new areatrigger_at_divine_tempest();
    new spell_pal_first_avenger();
    new spell_pal_greater_blessing_of_wisdom();
    new spell_pal_blessing_of_the_ashbringer();
    new spell_pal_guardian_of_the_forgotten_queen();
    new spell_pal_the_light_saves_aura();
    RegisterSpellScript(spell_pal_hammer_of_reckoning);
    RegisterAuraScript(spell_pal_last_defender);
    RegisterAreaTriggerAI(spell_pal_at_last_defender);
    RegisterAreaTriggerAI(spell_pal_at_devotion_aura);
    RegisterAreaTriggerAI(spell_pal_at_aura_of_sacrifice);
    RegisterAuraScript(spell_pal_t21_prot);
    RegisterAuraScript(spell_pal_aura_mastery);
    RegisterAuraScript(spell_pal_divine_vision);
    RegisterAuraScript(spell_pal_tyr_deliverance);
    RegisterAuraScript(spell_pal_wake_of_ashes_proc);
}
