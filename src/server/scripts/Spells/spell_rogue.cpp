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
 * Scripts for spells with SPELLFAMILY_ROGUE and SPELLFAMILY_GENERIC spells used by rogue players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_rog_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "PathGenerator.h"
#include "AreaTriggerAI.h"
#include "AreaTrigger.h"

enum RogueSpells
{
    SkullAndCrossbones = 199603,
    GrandMelee         = 193358,
    RuthlessPrecision  = 193357,
    Broadside          = 193356,
    TrueBearing        = 193359,
    BuriedTreasure     = 199600
};

// Cheat Death - 31230
class spell_rog_cheat_death : public SpellScriptLoader
{
    public:
        spell_rog_cheat_death() : SpellScriptLoader("spell_rog_cheat_death") { }

        class spell_rog_cheat_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_cheat_death_AuraScript);

            void CalculateAmount(AuraEffect const* /*AuraEffect**/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                if (Unit* target = GetTarget())
                {
                    if (dmgInfo.GetDamage() < target->GetHealth())
                        return;

                    if (target->ToPlayer()->HasAura(45181))
                        return;
                    
                    target->CastSpell(target, 45182, true);
                    target->CastSpell(target, 45181, true);

                    uint32 health7 = target->CountPctFromMaxHealth(7);

                    // hp > 7% - absorb hp till 7%
                    if (target->GetHealth() > health7)
                        absorbAmount = dmgInfo.GetDamage() - target->GetHealth() + health7;
                    // hp lower than 7% - absorb everything
                    else
                        absorbAmount = dmgInfo.GetDamage();
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_rog_cheat_death_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_rog_cheat_death_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_cheat_death_AuraScript();
        }
};

// Blade Flurry - 13877
class spell_rog_blade_flurry : public SpellScriptLoader
{
    public:
        spell_rog_blade_flurry() : SpellScriptLoader("spell_rog_blade_flurry") { }

        class spell_rog_blade_flurry_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_blade_flurry_AuraScript);

            void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
            {
                PreventDefaultAction();

                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (eventInfo.GetActor()->GetGUID() != GetTarget()->GetGUID())
                    return;

                if (Player* _player = caster->ToPlayer())
                {
                    float damage = eventInfo.GetDamageInfo()->GetDamage();
                    SpellInfo const* spellInfo = eventInfo.GetDamageInfo()->GetSpellInfo();

                    if (!damage || eventInfo.GetDamageInfo()->GetDamageType() == DOT)
                        return;

                    if (spellInfo && !spellInfo->CanTriggerBladeFlurry())
                        return;

                    int32 percent = GetSpellInfo()->Effects[2]->BasePoints;
                    if(AuraEffect* aurEff = caster->GetAuraEffect(226318, EFFECT_0)) // Shivarran Symmetry
                        percent += aurEff->GetAmount();

                    damage = (damage * percent) / 100.f;

                    if (Unit* target = _player->SelectNearbyTarget(eventInfo.GetActionTarget()))
                        _player->CastCustomSpell(target, 22482, &damage, NULL, NULL, false);
                }
            }

            void Register() override
            {
                OnEffectProc += AuraEffectProcFn(spell_rog_blade_flurry_AuraScript::OnProc, EFFECT_0, SPELL_AURA_MOD_POWER_REGEN_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_blade_flurry_AuraScript();
        }
};

// Cloak of Shadows - 31224
class spell_rog_cloak_of_shadows : public SpellScriptLoader
{
    public:
        spell_rog_cloak_of_shadows() : SpellScriptLoader("spell_rog_cloak_of_shadows") { }

        class spell_rog_cloak_of_shadows_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_cloak_of_shadows_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                const SpellInfo* m_spellInfo = GetSpellInfo();

                float bp0 = 0.f;
                if (AuraEffect* aurEff = caster->GetAuraEffect(202533, EFFECT_0)) // Ghostly Shell
                    bp0 = aurEff->GetAmount();

                if (Player* _player = caster->ToPlayer())
                {
                    uint32 dispelMask = SpellInfo::GetDispelMask(DISPEL_ALL);
                    Unit::AuraApplicationMap& Auras = _player->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::iterator iter = Auras.begin(); iter != Auras.end();)
                    {
                        // remove all harmful spells on you...
                        SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
                        if ((spell->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE // only affect magic spells
                            || (spell->GetDispelMask() & dispelMask) || (spell->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC))
                            // ignore positive and passive auras
                            && !iter->second->IsPositive() && !iter->second->GetBase()->IsPassive() && m_spellInfo->CanDispelAura(spell))
                        {
                            _player->RemoveAura(iter);
                            if (bp0)
                                caster->CastCustomSpell(caster, 202536, &bp0, NULL, NULL, true);
                        }
                        else
                            ++iter;
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_rog_cloak_of_shadows_SpellScript::HandleOnHit, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_cloak_of_shadows_SpellScript();
        }
};

// Called by Stealth / Shadow Dance shapeshift - 158188
// Nightstalker - 14062
class spell_rog_nightstalker : public SpellScriptLoader
{
    public:
        spell_rog_nightstalker() : SpellScriptLoader("spell_rog_nightstalker") { }

        class spell_rog_nightstalker_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_nightstalker_AuraScript);

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(14062))
                        caster->CastSpell(caster, 130493, true);
                }
            }

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* aura = caster->GetAura(130493))
                        aura->SetAuraTimer(200);
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_rog_nightstalker_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_rog_nightstalker_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_nightstalker_AuraScript();
        }
};

// Called by Deadly Poison - 2818
// Deadly Poison : Instant damage - 113780
class spell_rog_deadly_poison_instant_damage : public SpellScriptLoader
{
    public:
        spell_rog_deadly_poison_instant_damage() : SpellScriptLoader("spell_rog_deadly_poison_instant_damage") { }

        class spell_rog_deadly_poison_instant_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_deadly_poison_instant_damage_SpellScript);

            void HandleOnCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = GetExplTargetUnit())
                        if (target->HasAura(2818, _player->GetGUID()))
                            _player->CastSpell(target, 113780, true);
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_rog_deadly_poison_instant_damage_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_deadly_poison_instant_damage_SpellScript();
        }
};

// Poisoned Knife - 185565
class spell_rog_poison_knife : public SpellScript
{
    PrepareSpellScript(spell_rog_poison_knife);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
        {
            if (auto target = GetHitUnit())
            {
                if (caster->HasAura(2823)) // Deadly Poison
                    caster->CastSpell(target, 2818, true);
                if (caster->HasAura(3408)) // Crippling Poison
                    caster->CastSpell(target, 3409, true);
                if (caster->HasAura(8679)) // Wound Poison
                    caster->CastSpell(target, 8680, true);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_poison_knife::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Pick Pocket - 921
class spell_rog_pick_pocket : public SpellScriptLoader
{
    public:
        spell_rog_pick_pocket() : SpellScriptLoader("spell_rog_pick_pocket") { }

        class spell_rog_pick_pocket_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_pick_pocket_SpellScript);

            void HandleOnCast()
            {
                if (auto caster = GetCaster())
                    if (auto target = GetExplTargetUnit())
                        if (caster->HasAura(63268))
                            target->CastSpell(caster, 121308, true);
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_rog_pick_pocket_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_pick_pocket_SpellScript();
        }
};

// Saber Slash - 193315
class spell_rog_saber_slash : public SpellScriptLoader
{
    public:
        spell_rog_saber_slash() : SpellScriptLoader("spell_rog_saber_slash") {}

        class spell_rog_saber_slash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_saber_slash_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAurasDueToSpell(193315);
                }

                return SPELL_CAST_OK;
            }

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        ObjectGuid targetGUID = target->GetGUID();

                        if (caster->HasAura(202754))
                        {
                            caster->CastSpellDelay(target, 197834, true, 400);
                            caster->RemoveAurasDueToSpell(202754);
                            return;
                        }

                        if (SpellInfo const* spellInfo = GetSpellInfo())
                        {
                            float chance = spellInfo->Effects[EFFECT_4]->BasePoints;

                            chance = caster->ApplyEffectModifiers(spellInfo, EFFECT_4, chance);

                            if (AuraEffect* auraEff = caster->GetAuraEffect(SkullAndCrossbones, EFFECT_0))
                                chance += auraEff->GetBaseAmount();

                            if (roll_chance_f(chance))
                                caster->CastSpellDelay(target, 197834, true, 400);
                        }
                    }
                }
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_saber_slash_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_rog_saber_slash_SpellScript::HandleOnHit, EFFECT_2, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_saber_slash_SpellScript();
        }
};

// Killing Spree - 51690
class spell_rog_killing_spree : public SpellScriptLoader
{
    public:
        spell_rog_killing_spree() : SpellScriptLoader("spell_rog_killing_spree") { }

        class spell_rog_killing_spree_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_killing_spree_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (Unit* target = _player->GetSelectedUnit())
                        if (_player->IsValidAttackTarget(target))
                            if (_player->IsWithinDist(target, 10.0f))
                                return SPELL_CAST_OK;

                return SPELL_FAILED_BAD_TARGETS;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                std::list<WorldObject*> targetTemp = targets;
                targets.clear();

                if (Unit* caster = GetCaster())
                {
                    if(caster->HasAura(13877))
                    {
                        if (targetTemp.size() > 6)
                            targetTemp.resize(6);
                    }
                    else
                    {
                        if (Player* _player = caster->ToPlayer())
                        {
                            if (Unit* target = _player->GetSelectedUnit())
                            {
                                targetTemp.clear();
                                targetTemp.push_back(target);
                            }
                            else
                                targetTemp.resize(1);
                        }
                    }
                }

                GuidList targetList;
                for (std::list<WorldObject*>::iterator itr = targetTemp.begin(); itr != targetTemp.end(); ++itr)
                    if(WorldObject* object = (*itr))
                        targetList.push_back(object->GetGUID());

                GetSpell()->SetEffectTargets(targetList);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_killing_spree_SpellScript::CheckCast);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rog_killing_spree_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_killing_spree_SpellScript();
        }

        class spell_rog_killing_spree_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_killing_spree_AuraScript);

            Position pos;
            void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                {
                    Unit* target = GetTarget();
                    GuidList targets = GetAura()->GetEffectTargets();

                    if(!targets.empty())
                    {
                        ObjectGuid targetGuid = Trinity::Containers::SelectRandomContainerElement(targets);
                        if(Unit* effectTarget = ObjectAccessor::GetUnit(*caster, targetGuid))
                            target = effectTarget;
                    }
                    if(target)
                    {
                        caster->CastSpell(target, 57840, true);
                        caster->CastSpell(target, 57841, true);
                    }
                }
            }

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->CastSpell(caster, 61851, true);
                    caster->GetPosition(&pos);
                }
            }

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(63252))
                        caster->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_killing_spree_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectApply += AuraEffectApplyFn(spell_rog_killing_spree_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_rog_killing_spree_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_killing_spree_AuraScript();
        }
};

// Blade Flurry AOE - 22482
class spell_rog_blade_flurry_aoe : public SpellScriptLoader
{
    public:
        spell_rog_blade_flurry_aoe() : SpellScriptLoader("spell_rog_blade_flurry_aoe") { }

        class spell_rog_blade_flurry_aoe_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_blade_flurry_aoe_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if(Unit* target = _player->GetSelectedUnit())
                        targets.remove(target);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rog_blade_flurry_aoe_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_blade_flurry_aoe_SpellScript();
        }
};

// Roll the Bones - 193316
class spell_rog_roll_the_bones : public SpellScriptLoader
{
    public:
        spell_rog_roll_the_bones() : SpellScriptLoader("spell_rog_roll_the_bones") { }

        class spell_rog_roll_the_bones_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_roll_the_bones_AuraScript);

            void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<uint32> buffList = {SkullAndCrossbones, GrandMelee, RuthlessPrecision, Broadside, TrueBearing, BuriedTreasure};
                    for (auto const spellId : buffList)
                        caster->RemoveAurasDueToSpell(spellId);

                    uint8 count = caster->HasAura(240837) ? 2 : 1;

                    if (roll_chance_f(3.f))
                        count = 6;
                    else if (roll_chance_f(11.f))
                        count = 3;
                    else if (roll_chance_f(33.f))
                        count = 2;

                    SpellCastTargets _targets;
                    _targets.SetCaster(caster);
                    _targets.SetUnitTarget(caster);
                    SpellPowerCost _cost = GetAura()->m_powerCost;

                    Trinity::Containers::RandomResizeList(buffList, count);
                    for (auto const spellId : buffList)
                    {
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                        {
                            caster->AddDelayedEvent(100, [caster, _targets, spellInfo, _cost]() -> void
                            {
                                if (!caster)
                                    return;

                                CustomSpellValues values;
                                TriggerCastData triggerData;
                                triggerData.triggerFlags = TRIGGERED_FULL_MASK;
                                triggerData.originalCaster = caster->GetGUID();
                                triggerData.SubType = SPELL_CAST_TYPE_MISSILE;
                                triggerData.powerCost = _cost;

                                caster->CastSpell(_targets, spellInfo, &values, triggerData);
                            });
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_rog_roll_the_bones_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_roll_the_bones_AuraScript();
        }
};

// Roll the Bones aura - 193356, 193357, 193358, 193359, 199600, 199603
class spell_rog_roll_the_bones_aura : public SpellScriptLoader
{
    public:
        spell_rog_roll_the_bones_aura() : SpellScriptLoader("spell_rog_roll_the_bones_aura") { }

        class spell_rog_roll_the_bones_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_roll_the_bones_aura_AuraScript);

            void CalculateMaxDuration(int32& duration)
            {
                if (Unit* caster = GetCaster())
                    if (Aura* aura = caster->GetAura(193316)) // Roll the Bones
                        duration = aura->GetDuration();
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_rog_roll_the_bones_aura_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_roll_the_bones_aura_AuraScript();
        }
};

// Grappling Hook - 195457
class spell_rog_grappling_hook : public SpellScriptLoader
{
    public:
        spell_rog_grappling_hook() : SpellScriptLoader("spell_rog_grappling_hook") { }

        class spell_rog_grappling_hook_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_grappling_hook_SpellScript);

            SpellCastResult CheckElevation()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return SPELL_FAILED_NOPATH;

                WorldLocation* dest = const_cast<WorldLocation*>(GetExplTargetDest());

                if (caster->HasAuraType(SPELL_AURA_MOD_ROOT) || caster->HasAuraType(SPELL_AURA_MOD_ROOTED))
                    return SPELL_FAILED_ROOTED;

                if (!caster->IsWithinLOS(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ()))
                    return SPELL_FAILED_LINE_OF_SIGHT;

                // battleground <bad> positions
                if (caster->GetMap()->IsBattleground())
                {
                    switch (caster->GetMapId())
                    {
                    case 489:
                        if ((dest->GetPositionZ() >= 375.0f) || (caster->GetPositionZ() <= 361.0f && dest->GetPositionZ() >= 364.0f && dest->GetPositionY() <= 1370.0f)
                            || (dest->GetPositionY() <= 1266.0f) || (dest->GetPositionX() <= 991.0f && dest->GetPositionZ() >= 368.0f)
                            || (dest->GetPositionY() <= 1396.0f && dest->GetPositionZ() >= 353.0f) || (dest->GetPositionY() >= 1652.0f && dest->GetPositionZ() >= 354.0f))
                            return SPELL_FAILED_NOPATH;
                        break;
                    case 529:
                        if (caster->GetCurrentAreaID() == 3424)
                            if ((dest->GetPositionX() <= 1199.0f && dest->GetPositionY() <= 1199.0f && dest->GetPositionZ() >= -51.0f) || (dest->GetPositionZ() >= 13.9f))
                                return SPELL_FAILED_NOPATH;
                        if (caster->GetCurrentAreaID() == 3421)
                            if ((dest->GetPositionZ() >= -40.0f))
                                return SPELL_FAILED_NOPATH;
                        if (caster->GetCurrentAreaID() == 3422)
                            if ((dest->GetPositionZ() >= 18.8f))
                                return SPELL_FAILED_NOPATH;
                        break;
                    case 726:
                        if (((dest->GetPositionX() < 1855.0f && dest->GetPositionX() > 1534.0f) && (dest->GetPositionY() < 183.0f && dest->GetPositionY() > 78.0f) && dest->GetPositionZ() >= -4.8f)
                            || ((dest->GetPositionX() < 1926.0f && dest->GetPositionX() > 1818.0f) && (dest->GetPositionY() < 510.0f && dest->GetPositionY() > 407.0f) && dest->GetPositionZ() >= -10.0f))
                            return SPELL_FAILED_NOPATH;
                        break;
                    case 998:
                        if ((dest->GetPositionX() < 1733.0f && dest->GetPositionY() > 1394.0f && dest->GetPositionZ() >= 23.4f)
                            || (dest->GetPositionX() > 1824.0f && dest->GetPositionZ() > 20.4f)
                            || ((dest->GetPositionX() > 1719.0f && dest->GetPositionX() < 1846.7f) && (dest->GetPositionY() > 1253.0f
                                && dest->GetPositionY() < 1416.0f) && dest->GetPositionZ() >= 11.0f))
                            return SPELL_FAILED_NOPATH;
                        break;
                    case 607:
                        if (((dest->GetPositionX() < 1475.0f && dest->GetPositionX() > 1383.7f) && (dest->GetPositionY() < -177.0f && dest->GetPositionY() > -262.0f) && dest->GetPositionZ() >= 38.0f)
                            || ((dest->GetPositionX() < 1468.0f && dest->GetPositionX() > 1357.7f) && (dest->GetPositionY() < 160.0f && dest->GetPositionY() > 59.0f) && dest->GetPositionZ() >= 36.0f)
                            || ((dest->GetPositionX() < 1262.0f && dest->GetPositionX() > 1158.0f) && (dest->GetPositionY() < 137.0f && dest->GetPositionY() > 29.0f) && dest->GetPositionZ() >= 58.1f)
                            || ((dest->GetPositionX() < 1277.0f && dest->GetPositionX() > 1169.0f) && (dest->GetPositionY() < -149.0f && dest->GetPositionY() > -272.0f) && dest->GetPositionZ() >= 58.0f)
                            || ((dest->GetPositionX() < 1092.0f && dest->GetPositionX() > 1012.0f) && (dest->GetPositionY() < -55.0f && dest->GetPositionY() > -154.0f) && dest->GetPositionZ() >= 89.77f))
                            return SPELL_FAILED_NOPATH;
                        break;
                    }
                }

                float limit = 60.0f;
                PathGenerator* m_path = new PathGenerator(caster);
                bool result = m_path->CalculatePath(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), false);
                if (m_path->GetPathType() & PATHFIND_SHORT)
                    return SPELL_FAILED_OUT_OF_RANGE;
                else if (!result || m_path->GetPathType() & PATHFIND_NOPATH)
                    return SPELL_FAILED_NOPATH;
                else if (m_path->GetTotalLength() > limit)
                    return SPELL_FAILED_OUT_OF_RANGE;
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_grappling_hook_SpellScript::CheckElevation);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_grappling_hook_SpellScript();
        }
};

// Exsanguinate - 200806
class spell_rog_exsanguinate : public SpellScriptLoader
{
    public:
        spell_rog_exsanguinate() : SpellScriptLoader("spell_rog_exsanguinate") { }

        class spell_rog_exsanguinate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_exsanguinate_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        uint32 mechanicMask = (1 << MECHANIC_BLEED);
                        Unit::AuraApplicationMap& appliedAuras = target->GetAppliedAuras();

                        for (Unit::AuraApplicationMap::iterator iter = appliedAuras.begin(); iter != appliedAuras.end(); ++iter)
                        {
                            if (Aura* aura = iter->second->GetBase())
                            {
                                if (aura->GetCasterGUID() == caster->GetGUID())
                                {
                                    SpellInfo const* spellInfo = aura->GetSpellInfo();
                                    if (!(spellInfo->EffectMechanicMask & mechanicMask))
                                        continue;

                                    for (AuraEffect* auraEffect : aura->GetAuraEffects())
                                    {
                                        if (auraEffect)
                                            if (mechanicMask & (1 << spellInfo->Effects[auraEffect->GetEffIndex()]->Mechanic))
                                            {
                                                auraEffect->SetAmplitude(auraEffect->GetPeriod()/2.5f);
                                                auraEffect->SetPeriodicTimer(auraEffect->GetPeriodicTimer()/2.5f);
                                                auraEffect->SetPeriodMod(auraEffect->GetPeriodMod() - auraEffect->GetPeriod());
                                            }
                                    }
                                    int32 _duration = aura->GetDuration()/2.5f;
                                    if (_duration > aura->GetMaxDuration())
                                        aura->SetMaxDuration(_duration);
                                    aura->SetDuration(_duration);
                                }
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_rog_exsanguinate_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_exsanguinate_SpellScript();
        }
};

// Shuriken Storm - 197835
class spell_rog_shuriken_storm : public SpellScriptLoader
{
    public:
        spell_rog_shuriken_storm() : SpellScriptLoader("spell_rog_shuriken_storm") { }

        class spell_rog_shuriken_storm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_shuriken_storm_SpellScript);

            bool bonusActive = false;

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (bonusActive)
                    {
                        int32 damage = GetHitDamage();
                        damage += CalculatePct(damage, GetSpellInfo()->Effects[EFFECT_2]->BasePoints);
                        SetHitDamage(damage);
                    }
                    caster->CastSpell(caster, 212743, true); // combopoint spell
                }
            }

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(245639)) // Shuriken Combo
                    {
                        uint8 count = GetSpell()->GetTargetCount();
                        if (count > 1)
                        {
                            for (int i = 0; i < count - 1; i++)
                                caster->CastSpell(caster, 245640, true);
                        }
                    }
                }
            }

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(197610) && (caster->GetShapeshiftForm() == FORM_STEALTH || caster->HasAura(185313)))
                        bonusActive = true;

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_rog_shuriken_storm_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_rog_shuriken_storm_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                AfterCast += SpellCastFn(spell_rog_shuriken_storm_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_shuriken_storm_SpellScript();
        }
};

// Rupture 1943, for Venomous Wounds - 79134
class spell_rog_venomous_wounds : public SpellScriptLoader
{
    public:
        spell_rog_venomous_wounds() : SpellScriptLoader("spell_rog_venomous_wounds") { }

        class spell_rog_venomous_wounds_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_venomous_wounds_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                    {
                        if (Aura* rupture = aurEff->GetBase())
                        {
                            int32 perc = rupture->GetDuration() * 100 / rupture->GetMaxDuration();
                            float bp = CalculatePct(caster->GetMaxPower(POWER_ENERGY), perc);
                            caster->CastCustomSpell(caster, 51637, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_rog_venomous_wounds_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override 
        {
            return new spell_rog_venomous_wounds_AuraScript();
        }
};

// Turn the Tables (Honor Talent) - 198023
class spell_rog_turn_the_tables : public SpellScriptLoader
{
    public:
        spell_rog_turn_the_tables() : SpellScriptLoader("spell_rog_turn_the_tables") { }

        class spell_rog_turn_the_tables_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_turn_the_tables_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasAuraWithMechanic((1<<MECHANIC_STUN)))
                    {
                        caster->CastSpell(caster, 198027, true);
                        GetAura()->Remove();
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_turn_the_tables_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_turn_the_tables_AuraScript();
        }
};

// Creeping Venom (Honor Talent) - 198097
class spell_rog_creeping_venom : public SpellScriptLoader
{
    public:
        spell_rog_creeping_venom() : SpellScriptLoader("spell_rog_creeping_venom") { }

        class spell_rog_creeping_venom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_creeping_venom_AuraScript);

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetTarget())
                        if (target->isMoving() && target->HasAura(3409, caster->GetGUID()))
                            GetAura()->RefreshDuration();
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_rog_creeping_venom_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_creeping_venom_AuraScript();
        }
};

// Fan of Knives - 51723
class spell_rog_fan_of_knives : public SpellScriptLoader
{
    public:
        spell_rog_fan_of_knives() : SpellScriptLoader("spell_rog_fan_of_knives") { }

        class spell_rog_fan_of_knives_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_fan_of_knives_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if(!caster || !target)
                    return;

                AuraEffect* aurEff0 = caster->GetAuraEffect(198128, EFFECT_0); // Flying Daggers (Honor Talent)
                AuraEffect* aurEff1 = caster->GetAuraEffect(198128, EFFECT_1); // Flying Daggers (Honor Talent)
                if (aurEff0 && aurEff1 && GetSpell()->GetTargetCount() >= aurEff1->GetAmount())
                    SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), aurEff0->GetAmount()));

                if (AuraEffect* aurEff = caster->GetAuraEffect(192376, EFFECT_0)) // Poison Knives
                {
                    float bp0 = CalculatePct(target->GetRemainingPeriodicAmount(caster->GetGUID(), 2818, SPELL_AURA_PERIODIC_DAMAGE, EFFECT_0, 0.f), aurEff->GetAmount());
                    caster->CastCustomSpell(target, 192380, &bp0, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_rog_fan_of_knives_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_fan_of_knives_SpellScript();
        }
};

// Envenom - 32645
class spell_rog_envenom : public SpellScriptLoader
{
    public:
        spell_rog_envenom() : SpellScriptLoader("spell_rog_envenom") { }

        class spell_rog_envenom_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_envenom_SpellScript);

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                AuraEffect* aurEff = caster->GetAuraEffect(198145, EFFECT_0); // System Shock (Honor Talent)
                if (aurEff && GetSpell()->GetComboPoints() >= aurEff->GetAmount())
                    if (target->HasAura(2818, caster->GetGUID()) && (target->HasAura(703, caster->GetGUID()) && target->HasAura(1943, caster->GetGUID())))
                        caster->CastSpell(target, 198222, true);

                if (AuraEffect* aurEffI = caster->GetAuraEffect(211673, EFFECT_0)) // Item - Rogue T19 Assassination 4P Bonus
                {
                    Unit::AuraList mechanicAuras;
                    target->GetAuraEffectsByMechanic(1<<MECHANIC_BLEED, mechanicAuras, caster->GetGUID());
                    if (!mechanicAuras.empty())
                    {
                        int32 damage = GetHitDamage();
                        damage += CalculatePct(damage, mechanicAuras.size() * aurEffI->GetAmount());
                        SetHitDamage(damage);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_rog_envenom_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_envenom_SpellScript();
        }
};

// Cold Blood (Honor Talent) - 213983
class spell_rog_cold_blood : public SpellScriptLoader
{
    public:
        spell_rog_cold_blood() : SpellScriptLoader("spell_rog_cold_blood") { }

        class spell_rog_cold_blood_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rog_cold_blood_SpellScript);

            void HandleDamage(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[effIndex]->CalcValue(GetCaster())));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_rog_cold_blood_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_rog_cold_blood_SpellScript();
        }
};

// Cut to the Chase (Honor Talent) - 197020
class areatrigger_at_cut_to_the_chase : public AreaTriggerScript
{
    public:
        areatrigger_at_cut_to_the_chase() : AreaTriggerScript("areatrigger_at_cut_to_the_chase") { }

    struct areatrigger_at_cut_to_the_chaseAI : AreaTriggerAI
    {
        areatrigger_at_cut_to_the_chaseAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        void ActionOnUpdate(GuidList& affectedPlayers) override
        {
            Unit* caster = at->GetCaster();
            if (!caster || caster->HasAura(197023))
                return;

            float maxSpeed = 0.0f;
            for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
            {
                next = itr;
                ++next;

                Unit* unit = ObjectAccessor::GetUnit(*at, *itr);
                if (!unit || unit == caster)
                    continue;
                float speed = unit->GetSpeedRate(MOVE_RUN);

                if (unit->HasAuraType(SPELL_AURA_MOD_SPEED_NO_CONTROL))
                    continue;

                if (speed > maxSpeed)
                    maxSpeed = speed;
            }

            if (maxSpeed == 0.0f)
                return;

            if (maxSpeed > caster->GetSpeedRate(MOVE_RUN))
            {
                float bp0 = int32(maxSpeed * 100) + 5;
                caster->CastCustomSpell(caster, 197023, &bp0, NULL, NULL, true);
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_cut_to_the_chaseAI(areatrigger);
    }
};

// Nightblade - 195452
class spell_rog_nightblade : public SpellScriptLoader
{
    public:
        spell_rog_nightblade() : SpellScriptLoader("spell_rog_nightblade") { }

        class spell_rog_nightblade_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_rog_nightblade_AuraScript);

            void CalculateMaxDuration(int32& duration)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(211661, EFFECT_0)) // Item - Rogue T19 Subtlety 2P Bonus
                        duration += aurEff->GetAmount() * GetAura()->GetComboPoints() * IN_MILLISECONDS;
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_rog_nightblade_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_rog_nightblade_AuraScript();
        }
};

// Run Through - 2098
class spell_rog_cut_to_the_chase : public SpellScript
{
    PrepareSpellScript(spell_rog_cut_to_the_chase);

    void HandleDamage(SpellEffIndex effIndex)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = caster->GetAura(251783)) // Item - Rogue T21 Outlaw 4P Bonus - Run Through Grants Random RtB Buff
            {
                if (AuraEffect* eff1 = aura->GetEffect(EFFECT_1))
                {
                    if (roll_chance_f(eff1->GetAmount()))
                    {
                        if (AuraEffect* eff2 = aura->GetEffect(EFFECT_2))
                        {
                            std::vector<uint32> auraList;

                            for (auto itr : {SkullAndCrossbones, GrandMelee, RuthlessPrecision, Broadside, TrueBearing, BuriedTreasure})
                            {
                                if (!caster->HasAura(itr))
                                    auraList.push_back(itr);
                            }

                            if (!auraList.empty())
                            {
                                if (auraList.size() > 1)
                                    Trinity::Containers::RandomResizeList(auraList, 1);

                                uint32 spellId = *auraList.begin();
                                int32 dur = eff2->GetAmount() * IN_MILLISECONDS;

                                caster->AddAura(spellId, caster, nullptr, 0, dur, dur);
                            }
                        }
                    }
                }
            }

            if (AuraEffect* aurEff = caster->GetAuraEffect(212539, EFFECT_0)) // Thraxi's Tricksy Treads
            {
                float perc = CalculatePct((caster->GetSpeedRate(MOVE_RUN) - 1.f) * 100, aurEff->GetAmount());
                if (perc <= 0)
                    return;

                int32 damage = GetHitDamage();
                damage += CalculatePct(damage, perc);
                SetHitDamage(damage);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_rog_cut_to_the_chase::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Plunder Armor (PvP Talent) - 198529
class spell_rog_plunder_armor : public SpellScript
{
    PrepareSpellScript(spell_rog_plunder_armor);

    void HandleOnCast()
    {
        if (auto caster = GetCaster())
            if (auto target = GetExplTargetUnit())
                if (target->IsPlayer())
                    target->CastSpell(caster, 208535, true);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_rog_plunder_armor::HandleOnCast);
    }
};

// Control is King - 212217
struct areatrigger_rog_control_is_king : public AreaTriggerAI
{
    areatrigger_rog_control_is_king(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

    enum MyEnum
    {
        ControlIsKing = 212219
    };

    bool isActivated = false;

    uint32 CallSpecialFunction(uint32 Num) override
    {
        isActivated = Num;

        if (GuidList* GUIDList = at->GetAffectedPlayers())
        {
            if (GUIDList->empty())
                return 0;

            ObjectGuid casterGUID = at->GetCasterGUID();

            for (auto& itr : *GUIDList)
            {
                if (Unit* unit = ObjectAccessor::GetUnit(*at, itr))
                {
                    if (Aura* aura = unit->GetAura(ControlIsKing, casterGUID))
                        aura->SetAuraAttribute(AURA_ATTR_IS_NOT_ACTIVE, !Num);
                }
            }
        }
        return 0;
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (Aura* aura = caster->AddAura(ControlIsKing, unit))
            {
                if (!isActivated)
                    aura->SetAuraAttribute(AURA_ATTR_IS_NOT_ACTIVE);
            }
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAurasDueToSpell(ControlIsKing, at->GetCasterGUID());
    }

    void BeforeRemove(Unit* unit) override
    {
        unit->RemoveAurasDueToSpell(ControlIsKing, at->GetCasterGUID());
    }
};

// Shadow Strike - 185438
class spell_rog_shadow_strike : public SpellScript
{
    PrepareSpellScript(spell_rog_shadow_strike);

    float dist = NOMINAL_MELEE_RANGE - MIN_MELEE_REACH;

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        auto target = GetExplTargetUnit();

        if (!caster || !target || (caster->GetDistance(target) > dist && caster->HasUnitState(UNIT_STATE_ROOT)))
            return SPELL_FAILED_OUT_OF_RANGE;

        float delta_z = fabs(target->GetPositionZ()) - fabs(caster->GetPositionZ());
        if (caster->GetMap()->IsBattleground())
        {
            if (target->GetEntry() == 27894)
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            else if (target->FindNearestCreature(27894, 11.0f) && delta_z > 4.0f)
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        return SPELL_CAST_OK;
    }

    void HandleOnCast()
    {
        if (auto caster = GetCaster())
            if (auto target = GetExplTargetUnit())
                if (caster->HasAura(231718) && (!(caster->GetDistance(target) <= dist || caster->HasUnitState(UNIT_STATE_ROOT))))
                    caster->CastSpell(target, 138916, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_rog_shadow_strike::HandleOnCast);
        OnCheckCast += SpellCheckCastFn(spell_rog_shadow_strike::CheckElevation);
    }
};

//178236, 178153
class spell_rog_death_from_above_jump : public SpellScript
{
    PrepareSpellScript(spell_rog_death_from_above_jump);

    void HandleScript(SpellEffIndex /*effectIndex*/)
    {
        if (!GetCaster())
            return;

        if (GetId() == 178236)
        {
            Position offset = { 0.0f, 0.0f, 8.0f, 0.0f };
            GetSpell()->destTarget->RelocateOffset(offset);
        }
        else if (GetId() == 178153)
        {
            if (auto target = GetExplTargetUnit())
            {
                Position pos;
                GetCaster()->GetPosition(&pos);
                pos.m_positionZ = GetCaster()->GetHeight(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                GetCaster()->MovePositionToFirstCollision(pos, GetCaster()->GetExactDist2d(target) - target->GetObjectSize(), GetCaster()->GetRelativeAngle(target));
                GetSpell()->destTarget->Relocate(pos);
            }
        }
    }

    void Register()
    {
        OnEffectLaunch += SpellEffectFn(spell_rog_death_from_above_jump::HandleScript, EFFECT_0, SPELL_EFFECT_JUMP_DEST);
        OnEffectLaunch += SpellEffectFn(spell_rog_death_from_above_jump::HandleScript, EFFECT_1, SPELL_EFFECT_JUMP_DEST);
    }
};

// Curse of the Dreadblades - 214862
class spell_rog_curse_of_the_dreadblades_proc : public AuraScript
{
    PrepareAuraScript(spell_rog_curse_of_the_dreadblades_proc);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetProcTarget())
            {
                if (target->GetCreatureType() == CREATURE_TYPE_HUMANOID)
                    caster->CastSpell(caster, 213738, true);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_rog_curse_of_the_dreadblades_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 207736 - Shadowy Duel (Honor Talent)
class spell_rog_shadowy_duel_main : public SpellScript
{
    PrepareSpellScript(spell_rog_shadowy_duel_main);

    SpellCastResult CheckElevation()
    {
        auto caster = GetCaster();
        if (!caster)
            return SPELL_FAILED_NO_VALID_TARGETS;

        if (auto target = GetExplTargetUnit())
        {
            if (target->HasAuraType(SPELL_AURA_BATTLEGROUND_FLAG) || target->HasAuraType(SPELL_AURA_BATTLEGROUND_FLAG_2))
                return SPELL_FAILED_NO_VALID_TARGETS;
        }
        return SPELL_CAST_OK;
    }

    void HandleOnCast()
    {
        if (auto caster = GetCaster())
        {
            if (auto target = GetExplTargetUnit())
            {
                caster->CastSpell(target, 210558, true); // stalked enemy
                target->CastSpell(caster, 210558, true); // stalked caster
                caster->CastSpell(caster, 207756, true); // invisibles and visual
                target->CastSpell(target, 207756, true);
            }
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_rog_shadowy_duel_main::CheckElevation);
        OnCast += SpellCastFn(spell_rog_shadowy_duel_main::HandleOnCast);
    }
};

void AddSC_rogue_spell_scripts()
{
    new spell_rog_cheat_death();
    new spell_rog_blade_flurry();
    new spell_rog_cloak_of_shadows();
    new spell_rog_nightstalker();
    new spell_rog_deadly_poison_instant_damage();
    RegisterSpellScript(spell_rog_poison_knife);
    new spell_rog_pick_pocket();
    new spell_rog_killing_spree();
    new spell_rog_blade_flurry_aoe();
    new spell_rog_roll_the_bones();
    new spell_rog_roll_the_bones_aura();
    new spell_rog_grappling_hook();
    new spell_rog_exsanguinate();
    new spell_rog_shuriken_storm();
    new spell_rog_venomous_wounds();
    new spell_rog_turn_the_tables();
    new spell_rog_creeping_venom();
    new spell_rog_fan_of_knives();
    new spell_rog_envenom();
    new spell_rog_cold_blood();
    new areatrigger_at_cut_to_the_chase();
    new spell_rog_nightblade();
    RegisterSpellScript(spell_rog_cut_to_the_chase);
    RegisterSpellScript(spell_rog_plunder_armor);
    new spell_rog_saber_slash();
    RegisterSpellScript(spell_rog_shadow_strike);
    RegisterAreaTriggerAI(areatrigger_rog_control_is_king);
    RegisterSpellScript(spell_rog_death_from_above_jump);
    RegisterAuraScript(spell_rog_curse_of_the_dreadblades_proc);
    RegisterSpellScript(spell_rog_shadowy_duel_main);
}
