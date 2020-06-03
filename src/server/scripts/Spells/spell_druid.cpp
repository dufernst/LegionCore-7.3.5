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
 * Scripts for spells with SPELLFAMILY_DRUID and SPELLFAMILY_GENERIC spells used by druid players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dru_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "Group.h"
#include "AreaTrigger.h"

enum DruidSpells
{
    GERMINATION  = 155777
};

// Binary predicate for sorting Units based on value of duration of an Aura
class AuraDurationCompareOrderPred
{
    public:
        AuraDurationCompareOrderPred(ObjectGuid caster, uint32 auraId, bool ascending = true) : m_caster(caster), m_aura(auraId), m_ascending(ascending) {}
        bool operator() (const Unit* a, const Unit* b) const
        {
            return m_ascending ? a->GetAura(m_aura, m_caster)->GetDuration() < b->GetAura(m_aura, m_caster)->GetDuration() :
                                    a->GetAura(m_aura, m_caster)->GetDuration() > b->GetAura(m_aura, m_caster)->GetDuration();
        }
    private:
        ObjectGuid m_caster;
        uint32 m_aura;
        const bool m_ascending;
};

// Incarnation: Tree of Life (Shapeshift) - 33891
// Incarnation: King of the Jungle - 102543
// Incarnation: Son of Ursoc - 102558
// Incarnation: Chosen of Elune (Shapeshift) - 102560
class spell_dru_incarnation : public SpellScriptLoader
{
    public:
        spell_dru_incarnation() : SpellScriptLoader("spell_dru_incarnation") { }

        class spell_dru_incarnation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_incarnation_AuraScript);

            void UpdateModel()
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                ShapeshiftForm form = target->GetShapeshiftForm();
                if (form == FORM_CAT || form == FORM_BEAR || form == FORM_MOONKIN)
                    if (uint32 model = target->GetModelForForm(form))
                        target->SetDisplayId(model);
            }

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                UpdateModel();

                Unit* target = GetTarget();
                if (!target)
                    return;

                switch (GetId())
                {
                    case 33891:     // Incarnation: Tree of Life (Shapeshift)
                        if (!target->HasAura(117679))
                            target->CastSpell(target, 117679, true);
                        break;
                    case 102543:    // Incarnation: King of the Jungle
                        if (!target->HasAura(768))
                            target->CastSpell(target, 768, true);       // activate Cat Form
                        break;
                    case 102558:    // Incarnation: Son of Ursoc
                        if (!target->HasAura(5487))
                            target->CastSpell(target, 5487, true);      // activate Bear Form
                        break;
                    case 102560:    // Incarnation: Chosen of Elune (Shapeshift)
                        if (!target->HasAura(24858))
                            target->CastSpell(target, 24858, true);     // activate Moonkin Form
                        break;
                }
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                UpdateModel();
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_incarnation_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_incarnation_AuraScript::OnApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_incarnation_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectApply += AuraEffectApplyFn(spell_dru_incarnation_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);

                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_incarnation_AuraScript();
        }
};

// Incarnation (Passive) - 117679
class spell_dru_incarnation_tree_of_life : public SpellScriptLoader
{
    public:
        spell_dru_incarnation_tree_of_life() : SpellScriptLoader("spell_dru_incarnation_tree_of_life") { }

        class spell_dru_incarnation_tree_of_life_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_incarnation_tree_of_life_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                target->CastSpell(target, 81097, true); // Incarnation: Tree of Life (Passive)
                target->CastSpell(target, 81098, true); // Incarnation: Tree of Life (Passive)
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                target->RemoveAurasDueToSpell(33891);
                target->RemoveAurasDueToSpell(81097);
                target->RemoveAurasDueToSpell(81098);
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_incarnation_tree_of_life_AuraScript::OnApply, EFFECT_0, SPELL_AURA_IGNORE_CD, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_incarnation_tree_of_life_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_IGNORE_CD, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_incarnation_tree_of_life_AuraScript();
        }
};

// Ferocious Bite - 22568
class spell_dru_ferocious_bite : public SpellScript
{
    PrepareSpellScript(spell_dru_ferocious_bite);

    enum MyEnum
    {
        ApexPredator = 252752
    };

    bool hasT21Feral4P = false;

    uint32 CallSpecialFunction(uint32 Num) override
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(ApexPredator)) // Apex Predator
            {
                hasT21Feral4P = true;
            }
            else if (Spell* spell = GetSpell())
            {
                Num *= float(spell->GetComboPoints()) / 5.f;
            }
        }
        return Num;
    }

    void TakePower(Powers power, int32 &amount)
    {
        if (power == POWER_COMBO_POINTS && hasT21Feral4P)
            amount = 0;
    }

    void Damage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (Unit* target = GetHitUnit())
        {
            float perc = hasT21Feral4P ? 2.f : 1.f;
            int32 damageN = GetHitDamage();

            if (!hasT21Feral4P)
            {
                int32 curValue = caster->GetPower(POWER_ENERGY);

                if (curValue >= 25)
                {
                    perc += 1.0f;
                }
                else if (curValue > 0)
                {
                    perc += curValue * 0.04f;
                }
            }

            SetHitDamage(int32(damageN * perc));

            if (AuraEffect* aurEff = caster->GetAuraEffect(236020, EFFECT_0)) // Ferocious Wound (Honor Talent)
                if (target && GetSpell()->GetComboPoints() >= aurEff->GetSpellInfo()->Effects[EFFECT_1]->BasePoints)
                    caster->CastSpell(target, 236021, true);

            if (caster->HasAura(231056) && (target->GetHealthPct() < 25.0f || caster->HasAura(202031))) // Sabertooth
            {
                if (Aura* rip = target->GetAura(1079, caster->GetGUID()))
                {
                    if (caster->HasAura(210666)) // Open Wounds
                        caster->CastSpell(target, 210670, true);

                    rip->RefreshTimers();
                    rip->SetAuraAttribute(AURA_ATTR_REAPPLIED_AURA);
                }
            }
        }
    }

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
        {
            if (hasT21Feral4P)
                caster->RemoveAurasDueToSpell(ApexPredator);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dru_ferocious_bite::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnTakePower += TakePowertFn(spell_dru_ferocious_bite::TakePower);
        AfterCast += SpellCastFn(spell_dru_ferocious_bite::HandleAfterCast);
    }
};

// Rip - 1079
class spell_dru_rip : public AuraScript
{
    PrepareAuraScript(spell_dru_rip);

    enum MyEnum
    {
        OpenWounds = 210670,
        KingOfTheJungleDummy = 203052,
        KingOfTheJungle = 203059
    };

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (auto caster = GetCaster())
        {
            if (caster->HasAura(210666)) // Open Wounds
            {
                if (auto target = GetUnitOwner())
                    caster->CastSpell(target, OpenWounds, true);
            }

            if (caster->HasAura(KingOfTheJungleDummy))
            {
                if (mode & AURA_EFFECT_HANDLE_REAPPLY)
                {
                    if (Aura* KingOfTheJungleBuff = caster->GetAura(KingOfTheJungle))
                    {
                        KingOfTheJungleBuff->RefreshDuration();
                        return;
                    }
                }

                caster->CastSpell(caster, KingOfTheJungle, true);
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        if (auto target = GetUnitOwner())
        {
            if (Aura* KingOfTheJungleBuff = caster->GetAura(KingOfTheJungle))
                KingOfTheJungleBuff->ModStackAmount(-1);

            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode == AURA_REMOVE_BY_DEATH)
            {
                if (auto plr = caster->ToPlayer())
                {
                    if (plr->HasAura(202021))
                        plr->RemoveSpellCooldown(5217, true);
                }
            }
            target->RemoveAurasDueToSpell(OpenWounds, caster->GetGUID());
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_dru_rip::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectRemove += AuraEffectRemoveFn(spell_dru_rip::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// Ashamane's Frenzy - 210722
class spell_dru_ashamanes_frenzy : public SpellScriptLoader
{
public:
    spell_dru_ashamanes_frenzy() : SpellScriptLoader("spell_dru_ashamanes_frenzy") {}

    class spell_dru_ashamanes_frenzy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_dru_ashamanes_frenzy_AuraScript);

        float dist;

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(210723))
                    {
                        dist = NOMINAL_MELEE_RANGE;
                        dist = caster->GetSpellMaxRangeForTarget(target, spellInfo);
                    }
                }
            }
        }

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetUnitOwner())
                {
                    uint32 triggeredCastFlags = TRIGGERED_FULL_MASK;
                    triggeredCastFlags &= ~(TRIGGERED_IGNORE_LOS | TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE |
                                            TRIGGERED_IGNORE_CASTER_AURASTATE | TRIGGERED_IGNORE_SET_FACING);

                    if (caster->GetDistance(target) < dist && caster->IsWithinLOSInMap(target))
                        caster->CastSpell(target, 210723, TriggerCastFlags(triggeredCastFlags));
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_ashamanes_frenzy_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectApply += AuraEffectApplyFn(spell_dru_ashamanes_frenzy_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_dru_ashamanes_frenzy_AuraScript();
    }
};

// Lifebloom - 33763, 188550 : Final heal
class spell_dru_lifebloom : public SpellScriptLoader
{
    public:
        spell_dru_lifebloom() : SpellScriptLoader("spell_dru_lifebloom") { }

        class spell_dru_lifebloom_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_lifebloom_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (Unit* caster = GetCaster())
                    {
                        if (Aura* aur = target->GetAura(GetSpellInfo()->Id, caster->GetGUID()))
                            if (aur->GetDuration() < 4000)
                                caster->CastSpell(target, 33778, true);
                    }
                }
            }

            void AfterRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                // Final heal only on duration end
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
                    return;

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                caster->CastSpell(GetTarget(), 33778, true);
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (Unit* caster = GetCaster())
                        caster->CastSpell(target, 33778, true);
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_lifebloom_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAPPLY);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_lifebloom_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
                AfterDispel += AuraDispelFn(spell_dru_lifebloom_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_lifebloom_AuraScript();
        }
};

// Cat Form - 768
class spell_dru_cat_form : public SpellScriptLoader
{
    public:
        spell_dru_cat_form() : SpellScriptLoader("spell_dru_cat_form") { }

        class spell_dru_cat_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_cat_form_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasAura(113636))
                    {
                        caster->CastSpell(caster, 113636, true);
                        caster->RemoveMovementImpairingEffects();
                    }

                    if(AuraEffect* dash = caster->GetAuraEffect(1850, EFFECT_0))
                        dash->ChangeAmount(70);

                    if (caster->HasAura(210650)) // Protection of Ashamane
                        caster->RemoveAura(210655);
                }
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* dash = caster->GetAuraEffect(1850, EFFECT_0))
                        dash->ChangeAmount(0);

                    if (caster->HasAura(5215))
                        caster->RemoveAura(5215);

                    if (caster->HasAura(210650) && !caster->HasAura(214274)) // Protection of Ashamane
                    {
                        caster->CastSpell(caster, 210655, true);
                        caster->CastSpell(caster, 214274, true); // dummy check cooldown
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_dru_cat_form_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dru_cat_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_cat_form_AuraScript();
        }
};

// Frenzied Regeneration - 22842
class spell_dru_frenzied_regeneration : public AuraScript
{
    PrepareAuraScript(spell_dru_frenzied_regeneration);

    void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            float bp0 = caster->CanPvPScalar() ? GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster) / 2 : GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster);
            int32 heal = int32(CalculatePct(caster->GetMaxHealth(), GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster)));
            int32 damageTaken = caster->GetDamageTakenInPastSecs(GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster), true, true);
            if (damageTaken > 3)
                heal += int32(CalculatePct(damageTaken, bp0));
            if (Player* plr = caster->ToPlayer())
                AddPct(heal, plr->GetFloatValue(PLAYER_FIELD_VERSATILITY) + plr->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));

            heal /= aurEff->GetTotalTicks();
            amount += heal;
        }
    }

    void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                if (AuraEffect const* aurEff = plr->GetAuraEffect(242236, EFFECT_0)) // Item - Druid T20 Guardian 2P Bonus
                {
                    int32 cdmod = CalculatePct(GetSpellInfo()->Category.ChargeRecoveryTime, aurEff->GetAmount());
                    int32 bp = CalculatePct((100 - plr->GetHealthPct()), cdmod);
                    plr->ModSpellChargeCooldown(GetSpellInfo()->Id, bp);
                }
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_frenzied_regeneration::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
        AfterEffectApply += AuraEffectApplyFn(spell_dru_frenzied_regeneration::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// Stampeding Roar - 106898
class spell_dru_stampeding_roar_speed : public SpellScriptLoader
{
    public:
        spell_dru_stampeding_roar_speed() : SpellScriptLoader("spell_dru_stampeding_roar_speed") { }

        class spell_dru_stampeding_roar_speed_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_stampeding_roar_speed_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (_player->HasAura(1850))
                        if (_player->HasAura(106898))
                            _player->RemoveAura(106898);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dru_stampeding_roar_speed_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_stampeding_roar_speed_SpellScript();
        }
};

// Teleport : Moonglade - 18960
class spell_dru_teleport_moonglade : public SpellScriptLoader
{
    public:
        spell_dru_teleport_moonglade() : SpellScriptLoader("spell_dru_teleport_moonglade") { }

        class spell_dru_teleport_moonglade_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_teleport_moonglade_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if(_player->GetDistance(7964.063f, -2491.099f, 487.83f) > 100.0f)
                    {
                        _player->SaveRecallPosition();
                        _player->TeleportTo(1, 7964.063f, -2491.099f, 487.83f, _player->GetOrientation());
                    }
                    else
                        _player->TeleportTo(_player->m_recallLoc);
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_dru_teleport_moonglade_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_teleport_moonglade_SpellScript();
        }
};

// 40121 - Swift Flight Form (Passive)
class spell_dru_swift_flight_passive : public SpellScriptLoader
{
    public:
        spell_dru_swift_flight_passive() : SpellScriptLoader("spell_dru_swift_flight_passive") { }

        class spell_dru_swift_flight_passive_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_swift_flight_passive_AuraScript);

            bool Load() override
            {
                return GetCaster()->GetTypeId() == TYPEID_PLAYER;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Player* caster = GetCaster()->ToPlayer())
                    if (caster->GetSkillValue(SKILL_RIDING) >= 375)
                        amount = 310;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_swift_flight_passive_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_swift_flight_passive_AuraScript();
        }
};

// Travel Form Dummy - 159456
class spell_dru_travel_form_speed_increase_dummy : public AuraScript
{
    PrepareAuraScript(spell_dru_travel_form_speed_increase_dummy);

    void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (AuraEffect* eff = caster->GetAuraEffect(165961, EFFECT_2))
            {
                eff->SetCanBeRecalculated(true);
                eff->RecalculateAmount();
            }
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_dru_travel_form_speed_increase_dummy::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_CHANGE_AMOUNT);
    }
};

// Travel Form - 165961
class spell_dru_travel_form_speed_increase : public AuraScript
{
    PrepareAuraScript(spell_dru_travel_form_speed_increase);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                if (!plr->HasPvpStatsScalingEnabled())
                {
                    if (AuraEffect const* eff = caster->GetAuraEffect(236012, EFFECT_0)) // Malorne's Swiftness (Feral)
                    {
                        amount = eff->GetAmount();
                    }
                    else if (AuraEffect const* eff = caster->GetAuraEffect(236147, EFFECT_0)) // Malorne's Swiftness (Guardian)
                    {
                        amount = eff->GetAmount();
                    }
                    else if (!caster->isInCombat())
                    {
                        if (AuraEffect const* eff = caster->GetAuraEffect(159456, EFFECT_0))
                            amount += eff->GetAmount();
                    }
                }
            }
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_travel_form_speed_increase::CalculateAmount, EFFECT_2, SPELL_AURA_MOD_SPEED_ALWAYS);
    }
};

// Rejuvenation - 774, 155777 
class spell_druid_rejuvenation : public SpellScriptLoader
{
    public:
        spell_druid_rejuvenation() : SpellScriptLoader("spell_druid_rejuvenation") { }

        class spell_druid_rejuvenation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_druid_rejuvenation_AuraScript);

            enum MyEnum
            {
                Abundance = 207640
            };

            void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(207383)) // Abundance
                    {
                        if (mode & AURA_EFFECT_HANDLE_REAPPLY)
                        {
                            if (Aura* aura = caster->GetAura(Abundance))
                                aura->RefreshDuration();

                            return;
                        }

                        caster->CastSpellDelay(caster, Abundance, true, 50);
                    }
                }
            }

            void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetCaster())
                {
                    if (Aura* aura = caster->GetAura(207640)) // Abundance
                        aura->ModStackAmount(-1);
                }
            }

            void HandleEffectPeriodic(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEf = caster->GetAuraEffect(208220, EFFECT_0)) // Aman'Thul's Wisdom
                    {
                        if (Unit* unitTarget = GetUnitOwner())
                        {
                            if (unitTarget->IsFullHealth())
                            {
                                if (Aura* aur = unitTarget->GetAura(GetSpellInfo()->Id, caster->GetGUID()))
                                {
                                    if (aurEff->GetTickNumber() == 2 || aurEff->GetTickNumber() == 3 || aurEff->GetTickNumber() == 4)
                                        aur->SetDuration(aur->GetDuration() + aurEf->GetAmount() * IN_MILLISECONDS);
                                }
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_druid_rejuvenation_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
                AfterEffectApply += AuraEffectApplyFn(spell_druid_rejuvenation_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_druid_rejuvenation_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_druid_rejuvenation_AuraScript();
        }
};

// 106785, 213771 - Swipe
class spell_dru_swipe : public SpellScript
{
    PrepareSpellScript(spell_dru_swipe);

    void HandleAfterCast()
    {
        if (GetSpell()->GetEffectTargets().empty())
            return;

        if (Unit* caster = GetCaster())
            if (GetId() == 106785)
                caster->ModifyPower(POWER_COMBO_POINTS, 1);
    }

    void Damage(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                int32 damage = GetHitDamage();
                GetSpell()->AddEffectTarget(unitTarget->GetGUID());
                if (caster->HasAura(231283) && unitTarget->HasAuraState(AURA_STATE_BLEEDING))
                    AddPct(damage, GetSpellInfo()->Effects[EFFECT_1]->BasePoints);

                if (AuraEffect const* aurEff = caster->GetAuraEffect(211142, EFFECT_0)) // Item - Druid T19 Feral 4P Bonus
                {
                    Unit::AuraList mechanicAuras;
                    unitTarget->GetAuraEffectsByMechanic(1 << MECHANIC_BLEED, mechanicAuras, caster->GetGUID());
                    if (!mechanicAuras.empty())
                        AddPct(damage, mechanicAuras.size() * aurEff->GetAmount());        
                }
                SetHitDamage(damage);
            }
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dru_swipe::HandleAfterCast);
        OnEffectHitTarget += SpellEffectFn(spell_dru_swipe::Damage, EFFECT_2, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// Healing Touch - 5185
class spell_dru_healing_ouch : public SpellScript
{
    PrepareSpellScript(spell_dru_healing_ouch);

    uint32 spellId = 0;

    uint32 CallSpecialFunction(uint32 /*Num*/) override
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(203374)) // Nourish (Honor Talent)
            {
                if (Unit* target = GetExplTargetUnit())
                {
                    std::vector<uint32> auraSet;
                    ObjectGuid const& casterGUID = caster->GetGUID();

                    for (uint32 auraId : {774, 8936, 33763, 48438})
                        if (!target->HasAura(auraId, casterGUID))
                            auraSet.push_back(auraId);

                    if (auraSet.empty())
                        return 1;

                    spellId = Trinity::Containers::SelectRandomContainerElement(auraSet);
                }
            }
        }
        return 0;
    }

    void HandleOnHit()
    {
        if (!spellId)
            return;

        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetExplTargetUnit())
                caster->AddAura(spellId, target);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_dru_healing_ouch::HandleOnHit);
    }
};

// Glyph of One With Nature - 147420
class spell_dru_one_with_nature : public SpellScriptLoader
{
    public:
        spell_dru_one_with_nature() : SpellScriptLoader("spell_dru_one_with_nature") { }

        class spell_dru_one_with_nature_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_one_with_nature_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                if (!caster || caster->isInCombat())
                    return;

                Player* player = caster->ToPlayer();
                if (!player)
                    return;

                float X, Y, Z;
                uint32 mapId;
                switch (irand(0,7))
                {
                    case 0:
                        mapId = 870;
                        X = 272.611755f; Y = 1965.844971f; Z = 164.396271f;
                        break;
                    case 1:
                        mapId = 571;
                        X = 6673.821777f; Y = 4872.219727f; Z = -10.785309f;
                        break;
                    case 2:
                        mapId = 1;
                        X = 4990.475098f; Y = 108.657242f; Z = 52.308815f;
                        break;
                    case 3:
                        mapId = 1;
                        X = 7831.314941f; Y = -2479.068604f; Z = 487.088989f;
                        break;
                    case 4:
                        mapId = 870;
                        X = 854.048828f; Y = -1548.728394f; Z = 66.733719f;
                        break;
                    case 5:
                        mapId = 1;
                        X = 3806.464111f; Y = 128.605453f; Z = 9.519085f;
                        break;
                    case 6:
                        mapId = 0;
                        X = 2367.002197f; Y = 1266.341553f; Z = 31.316511f;
                        break;
                    case 7:
                        mapId = 0;
                        X = -12454.017578f; Y = -2722.260742f; Z = 0.786638f;
                        break;
                    default:
                        mapId = 870;
                        X = 272.611755f; Y = 1965.844971f; Z = 164.396271f;
                        break;
                }
                player->TeleportTo(mapId, X, Y, Z, player->GetOrientation());
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dru_one_with_nature_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_one_with_nature_SpellScript();
        }
};

// 783 - Travel Form (Shapeshift)
class spell_dru_travel_form : public SpellScriptLoader
{
    public:
        spell_dru_travel_form() : SpellScriptLoader("spell_dru_travel_form") { }

        class spell_dru_travel_form_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_travel_form_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->GetMap()->IsOutdoors(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ()))
                        return SPELL_FAILED_ONLY_OUTDOORS;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dru_travel_form_SpellScript::CheckCast);
            }
        };

        class spell_dru_travel_form_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_travel_form_AuraScript);

            uint32 update = 0;
            uint32 checkOutdoor = 0;

            bool fly = false;
            bool water = false;
            bool walk = false;

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (!GetCaster())
                    return;

                if (Player* caster = GetCaster()->ToPlayer())
                {
                    if (caster->IsInWater())
                    {
                        caster->CastSpell(caster, 1066, true);
                        water = true;
                    }
                    else
                    {
                        bool canFly = !caster->isInCombat() && !caster->GetMap()->IsDungeon() && !caster->GetMap()->IsBattlegroundOrArena();
                        Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(caster->GetCurrentZoneID());
                        if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(caster->GetCurrentAreaID()))
                            if (area->Flags[0] & AREA_FLAG_NO_FLY_ZONE  || (Bf && !Bf->CanFlyIn()))
                                canFly = false;
                        switch (caster->GetMapId())
                        {
                            case 1116:
                                if (!caster->HasSpell(191645))
                                    canFly = false;
                                break;
                            case 1220:
                                if (!caster->HasSpell(233368))
                                    canFly = false;
                                break;
                            case 1502:
                            case 1179: // duel zone
                            case 1669: // argus
                            case 1779: // argus invasion points
                            case 1464:
                            case 1178:
                                canFly = false;
                                break;
                            default:
                                if (!caster->HasSpell(90267))
                                    canFly = false;
                                break;
                        }
                        if (caster->HasAura(145389))
                            canFly = false;

                        if (canFly)
                        {
                            caster->CastSpell(caster, 40120, true);
                            fly = true;
                        }
                        else
                        {
                            caster->CastSpell(caster, 165961, true);
                            walk = true;
                        }
                    }
                }
            }

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                update += diff;
                checkOutdoor += diff;

                if (checkOutdoor >= 2000)
                {
                    if (Unit* caster = GetCaster())
                    {
                        if (!caster->GetMap()->IsOutdoors(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ()))
                        {
                            GetAura()->Remove();
                            return;
                        }
                    }

                    checkOutdoor = 0;
                }
                if (update >= 500)
                {
                    if (!GetCaster())
                        return;
                    if (Player* caster = GetCaster()->ToPlayer())
                    {
                        if (caster->IsInWater())
                        {
                            if (!water)
                            {
                                if (fly)
                                {
                                    fly = false;
                                    caster->RemoveAurasDueToSpell(40120);
                                }
                                if (walk)
                                {
                                    walk = false;
                                    caster->RemoveAurasDueToSpell(165961);
                                }
                                water = true;
                                caster->CastSpell(caster, 1066, true);
                            }
                        }
                        else
                        {
                            bool canFly = !caster->GetMap()->IsDungeon() && !caster->GetMap()->IsBattlegroundOrArena();
                            if (!fly && canFly)
                                canFly = !caster->isInCombat();

                            Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(caster->GetCurrentZoneID());
                            if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(caster->GetCurrentAreaID()))
                                if (area->Flags[0] & AREA_FLAG_NO_FLY_ZONE  || (Bf && !Bf->CanFlyIn()))
                                    canFly = false;
                            switch (caster->GetMapId())
                            {
                                case 1116:
                                    if (!caster->HasSpell(191645))
                                        canFly = false;
                                    break;
                                case 1220:
                                    if (!caster->HasSpell(233368))
                                        canFly = false;
                                    break;
                                case 1502:
                                case 1179: // duel zone
                                case 1669: // argus
                                case 1779: // argus invasion points
                                case 1464:
                                    canFly = false;
                                    break;
                                default:
                                    if (!caster->HasSpell(90267))
                                        canFly = false;
                                    break;
                            }

                            if (caster->HasAura(145389))
                                canFly = false;

                            if (canFly)
                            {
                                if (!fly)
                                {
                                    if (water)
                                    {
                                        water = false;
                                        caster->RemoveAurasDueToSpell(1066);
                                    }
                                    if (walk)
                                    {
                                        walk = false;
                                        caster->RemoveAurasDueToSpell(165961);
                                    }
                                    caster->CastSpell(caster, 40120, true);
                                    fly = true;
                                }
                            }
                            else
                            {
                                if (!walk)
                                {
                                    if (water)
                                    {
                                        water = false;
                                        caster->RemoveAurasDueToSpell(1066);
                                    }
                                    if (fly)
                                    {
                                        fly = false;
                                        caster->RemoveAurasDueToSpell(40120);
                                    }
                                    caster->CastSpell(caster, 165961, true);
                                    walk = true;
                                }
                            }
                        }
                    }
                    update = 0;
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (water)
                        caster->RemoveAurasDueToSpell(1066);
                    if (walk)
                        caster->RemoveAurasDueToSpell(165961);
                    if (fly)
                        caster->RemoveAurasDueToSpell(40120);
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_dru_travel_form_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_travel_form_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectUpdate += AuraEffectUpdateFn(spell_dru_travel_form_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
            }

            int32 cooldown;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_travel_form_AuraScript();
        }
        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_travel_form_SpellScript();
        }
};

// 1066, 165961, 40120 - Remove Travel Form
class spell_dru_travel_form_remove : public SpellScriptLoader
{
    public:
        spell_dru_travel_form_remove() : SpellScriptLoader("spell_dru_travel_form_remove") { }

        class spell_dru_travel_form_remove_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_travel_form_remove_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_CANCEL)
                    return;

                if (Unit* caster = GetCaster())
                {
                    caster->AddDelayedEvent(250, [caster]() -> void
                    {
                        caster->RemoveAurasDueToSpell(783);
                    });
                }
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_dru_travel_form_remove_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
            }

            int32 cooldown;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_travel_form_remove_AuraScript();
        }
};

// 1822 - Rake
class spell_dru_rake : public SpellScriptLoader
{
    public:
        spell_dru_rake() : SpellScriptLoader("spell_dru_rake") { }

        class spell_dru_rake_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_rake_SpellScript);

            void Damage(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                if (Unit* caster = GetCaster())
                {
                    int32 bp = GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster);
                    if (caster->CanPvPScalar()) // hack, why doesn't it work PvPMultiplier?!
                        bp *= GetSpellInfo()->Effects[EFFECT_3]->PvPMultiplier;

                    if (caster->HasAura(231052) && (GetSpell()->GetCastedFromStealth() || caster->HasAura(102543)))
                        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), bp));

                    if (caster->HasAura(231052) && GetSpell()->GetCastedFromStealth())
                        caster->CastSpell(target, 163505, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_rake_SpellScript::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_rake_SpellScript();
        }
};

// Brambles - 203953
class spell_dru_brambles : public SpellScriptLoader
{
    public:
        spell_dru_brambles() : SpellScriptLoader("spell_dru_brambles") { }

        class spell_dru_brambles_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_brambles_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* caster = GetCaster())
                {
                    Unit* attacker = dmgInfo.GetAttacker();
                    float bp = CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), 24); // (Attack power * 0.24) on 7.0.3
                    if (bp > int32(dmgInfo.GetDamage()))
                    {
                        bp = dmgInfo.GetDamage();
                        absorbAmount = dmgInfo.GetDamage();
                    }
                    else
                        absorbAmount = bp;

                    if (bp)
                        caster->CastCustomSpell(attacker, 203958, &bp, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_brambles_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_brambles_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_brambles_AuraScript();
        }
};

// Earthwarden - 203975
class spell_dru_earthwarden : public SpellScriptLoader
{
    public:
        spell_dru_earthwarden() : SpellScriptLoader("spell_dru_earthwarden") { }

        class spell_dru_earthwarden_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_earthwarden_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                if (dmgInfo.GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL)
                {
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), 30);
                    ModStackAmount(-1);
                }
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_earthwarden_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_earthwarden_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_earthwarden_AuraScript();
        }
};

// Rend and Tear - 204053
class spell_dru_rend_and_tear : public SpellScriptLoader
{
    public:
        spell_dru_rend_and_tear() : SpellScriptLoader("spell_dru_rend_and_tear") { }

        class spell_dru_rend_and_tear_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rend_and_tear_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                Unit* caster = GetCaster();
                if (!caster || caster->GetShapeshiftForm() != FORM_BEAR)
                    return;

                if(Unit* target = dmgInfo.GetAttacker())
                {
                    if (Aura* aura = target->GetAura(192090))
                        absorbAmount = CalculatePct(dmgInfo.GetDamage(), 2 * GetStackAmount());
                }
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_rend_and_tear_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rend_and_tear_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_rend_and_tear_AuraScript();
        }
};

// Tranquility - 157982
class spell_dru_tranquility : public SpellScriptLoader
{
    public:
        spell_dru_tranquility() : SpellScriptLoader("spell_dru_tranquility") { }

        class spell_dru_tranquility_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_tranquility_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (Player* _player = caster->ToPlayer())
                        if (Group* group = _player->GetGroup())
                            if (group->isRaidGroup())
                                return;

                SetHitHeal(GetHitHeal() * 2);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dru_tranquility_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_tranquility_SpellScript();
        }
};

// Flourish - 197721
class spell_druid_flourish : public SpellScriptLoader
{
    public:
        spell_druid_flourish() : SpellScriptLoader("spell_druid_flourish") { }

        class spell_druid_flourish_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_druid_flourish_SpellScript)

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        int32 dur = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster) * 1000;
                        if (Unit::AuraEffectList const* mPeriodic = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL))
                        {
                            for (Unit::AuraEffectList::const_iterator i = mPeriodic->begin(); i != mPeriodic->end(); ++i)
                            {
                                if ((*i)->GetCasterGUID() == caster->GetGUID())
                                    if(Aura* aura = (*i)->GetBase())
                                        aura->SetDuration(aura->GetDuration() + dur);
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_druid_flourish_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_druid_flourish_SpellScript();
        }
};

// Lunar Strike - 194153
class spell_dru_lunar_strike : public SpellScriptLoader
{
    public:
        spell_dru_lunar_strike() : SpellScriptLoader("spell_dru_lunar_strike") { }

        class spell_dru_lunar_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_lunar_strike_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                    if (GetHitUnit() != GetExplTargetUnit())
                        SetHitDamage(int32(CalculatePct(GetHitDamage(), GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster))));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dru_lunar_strike_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_lunar_strike_SpellScript();
        }
};

// Lunar Strike - 197628
class spell_dru_lunar_strike2 : public SpellScriptLoader
{
public:
    spell_dru_lunar_strike2() : SpellScriptLoader("spell_dru_lunar_strike2") { }

    class spell_dru_lunar_strike2_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dru_lunar_strike2_SpellScript);

        void HandleOnHit()
        {
            if (Unit* caster = GetCaster())
                if (GetHitUnit() != GetExplTargetUnit())
                    SetHitDamage(int32(CalculatePct(GetHitDamage(), GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster))));
        }

        void Register() override
        {
            OnHit += SpellHitFn(spell_dru_lunar_strike2_SpellScript::HandleOnHit);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dru_lunar_strike2_SpellScript();
    }
};

//202770
class spell_dru_fury_of_elune : public SpellScriptLoader
{
    public:
        spell_dru_fury_of_elune() : SpellScriptLoader("spell_dru_fury_of_elune") { }

        class spell_dru_fury_of_elune_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_fury_of_elune_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (GetCaster())
                    GetCaster()->RemoveAreaObject(GetId());
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_dru_fury_of_elune_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_fury_of_elune_AuraScript();
        }
};

// Rage of the Sleeper (Artifact) - 200851
class spell_dru_rage_of_the_sleeper : public SpellScriptLoader
{
    public:
        spell_dru_rage_of_the_sleeper() : SpellScriptLoader("spell_dru_rage_of_the_sleeper") { }

        class spell_dru_rage_of_the_sleeper_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_rage_of_the_sleeper_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* caster = GetCaster())
                {
                    Unit* attacker = dmgInfo.GetAttacker();
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_4]->CalcValue(caster));
                    caster->CastSpell(attacker, 219432, true);
                }
            }

            void Register() override
            {
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dru_rage_of_the_sleeper_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_rage_of_the_sleeper_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_rage_of_the_sleeper_AuraScript();
        }
};

// Fury of Elune - 211547
class spell_dru_fury_of_elune_re : public SpellScriptLoader
{
    public:
        spell_dru_fury_of_elune_re() : SpellScriptLoader("spell_dru_fury_of_elune_re") { }

        class spell_dru_fury_of_elune_re_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_fury_of_elune_re_SpellScript);

            void HandleOnCast()
            {
                if(Unit* caster = GetCaster())
                    if (AreaTrigger* areaObj = caster->GetAreaObject(202770))
                        areaObj->SendReShape(GetExplTargetDest());
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_dru_fury_of_elune_re_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_fury_of_elune_re_SpellScript();
        }
};

// 5221 - Shred
class spell_dru_shred : public SpellScriptLoader
{
    public:
        spell_dru_shred() : SpellScriptLoader("spell_dru_shred") { }

        class spell_dru_shred_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_shred_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                if (Unit* caster = GetCaster())
                {
                    int32 damage = GetHitDamage();
                    int32 bp = GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster);
                    if (caster->CanPvPScalar()) // hack, why doesn't it work PvPMultiplier?!
                        bp *= GetSpellInfo()->Effects[EFFECT_3]->PvPMultiplier;

                    if (caster->HasAura(231057) && (GetSpell()->GetCastedFromStealth() || caster->HasAura(102543)))
                        AddPct(damage, bp);

                    if (caster->HasAura(231063) && target->HasAuraWithMechanic((1 << MECHANIC_BLEED)))
                        AddPct(damage, GetSpellInfo()->Effects[EFFECT_4]->BasePoints);

                    if (AuraEffect const* aurEff = caster->GetAuraEffect(211142, EFFECT_0)) // Item - Druid T19 Feral 4P Bonus
                    {
                        Unit::AuraList mechanicAuras;
                        target->GetAuraEffectsByMechanic(1<<MECHANIC_BLEED, mechanicAuras, caster->GetGUID());
                        if (!mechanicAuras.empty())
                            damage += CalculatePct(damage, mechanicAuras.size() * aurEff->GetAmount());
                    }
                    SetHitDamage(damage);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dru_shred_SpellScript::HandleDamage, EFFECT_2, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_shred_SpellScript();
        }
};

// Thrash (Feral, Guardian) - 106830
class spell_dru_thrash : public SpellScriptLoader
{
    public:
        spell_dru_thrash() : SpellScriptLoader("spell_dru_thrash") { }

        class spell_dru_thrash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_thrash_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(210663, EFFECT_0)) // Scent of Blood
                    {
                        float bp0 = GetSpell()->GetTargetCount() * -aurEff->GetAmount();
                        caster->CastCustomSpell(caster, 210664, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_dru_thrash_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_thrash_SpellScript();
        }
};

// Ekowraith, Creator of Worlds - 210667
class spell_dru_ekowraith_creator_of_worlds : public SpellScriptLoader
{
    public:
        spell_dru_ekowraith_creator_of_worlds() : SpellScriptLoader("spell_dru_ekowraith_creator_of_worlds") { }

        class spell_dru_ekowraith_creator_of_worlds_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dru_ekowraith_creator_of_worlds_AuraScript);

            uint32 addPct;

            bool Load() override
            {
                addPct = GetSpellInfo()->Effects[EFFECT_7]->CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount0(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* aurSpell = caster->GetAuraEffect(197524, EFFECT_0)) // Astral Influence
                        amount += CalculatePct(aurSpell->GetAmount(), addPct);
                }
            }

            void CalculateAmount1(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* aurSpell = caster->GetAuraEffect(16931, EFFECT_0)) // Thick Hide
                        amount -= CalculatePct(-aurSpell->GetAmount(), addPct);
                }
            }

            void CalculateAmount3(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* aurSpell = caster->GetAuraEffect(131768, EFFECT_0)) // Feline Swiftness
                        amount += CalculatePct(aurSpell->GetAmount(), addPct);
                }
            }

            void CalculateAmount5(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* aurSpell = caster->GetAuraEffect(197524, EFFECT_1)) // Astral Influence
                        amount += CalculatePct(aurSpell->GetAmount(), addPct);
                }
            }

            void CalculateAmount6(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if(AuraEffect* aurSpell = caster->GetAuraEffect(197524, EFFECT_2)) // Astral Influence
                        amount += CalculatePct(aurSpell->GetAmount(), addPct);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_ekowraith_creator_of_worlds_AuraScript::CalculateAmount0, EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_ekowraith_creator_of_worlds_AuraScript::CalculateAmount1, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_ekowraith_creator_of_worlds_AuraScript::CalculateAmount3, EFFECT_3, SPELL_AURA_MOD_SPEED_ALWAYS);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_ekowraith_creator_of_worlds_AuraScript::CalculateAmount5, EFFECT_5, SPELL_AURA_MOD_AUTO_ATTACK_RANGE);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dru_ekowraith_creator_of_worlds_AuraScript::CalculateAmount6, EFFECT_5, SPELL_AURA_ADD_FLAT_MODIFIER);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dru_ekowraith_creator_of_worlds_AuraScript();
        }
};

// 194153, 190984
class spell_dru_nature_balance : public SpellScript
{
    PrepareSpellScript(spell_dru_nature_balance);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (Aura* aur = caster->GetAura(202430))
                {
                    if (GetId() == 194153)
                        if (Aura* aura = target->GetAura(164812, caster->GetGUID()))
                            if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                                aura->SetDuration(aura->GetDuration() + eff->GetPeriod() * aur->GetSpellInfo()->Effects[EFFECT_0]->BasePoints);

                    if (GetId() == 190984)
                        if (Aura* aura = target->GetAura(164815, caster->GetGUID()))
                            if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                                aura->SetDuration(aura->GetDuration() + eff->GetPeriod() * aur->GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
                }
            }
        }
    }

    void Register()
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dru_nature_balance::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 203242 - Rip and Tear (PvP Talent)
class spell_dru_rip_and_tear : public SpellScriptLoader
{
    public:
        spell_dru_rip_and_tear() : SpellScriptLoader("spell_dru_rip_and_tear") { }

        class spell_dru_rip_and_tear_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dru_rip_and_tear_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                auto target = GetHitUnit();
                if (!target)
                    return;

                if (auto caster = GetCaster())
                {
                    caster->CastSpell(caster, 203259, true);
                    caster->CastSpell(target, 1079, true);
                    caster->CastSpell(target, 1822, true);
                }
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_dru_rip_and_tear_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dru_rip_and_tear_SpellScript();
        }
};

// New Moon - 202767
class spell_arti_dru_new_moon : public SpellScript
{
    PrepareSpellScript(spell_arti_dru_new_moon);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        caster->AddAura(202787, caster);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_arti_dru_new_moon::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// Half Moon - 202768
class spell_arti_dru_half_moon : public SpellScript
{
    PrepareSpellScript(spell_arti_dru_half_moon);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAurasDueToSpell(202787);
        caster->AddAura(202788, caster);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_arti_dru_half_moon::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

// Full Moon - 202771
class spell_arti_dru_full_moon : public SpellScript
{
    PrepareSpellScript(spell_arti_dru_full_moon);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        if (!caster)
            return;

        caster->RemoveAurasDueToSpell(202788);
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (GetHitUnit() != GetExplTargetUnit())
        {
            uint8 count = GetSpell()->GetTargetCount() - 1;
            int32 damage = GetHitDamage();
            if (count)
                damage /= count;

            SetHitDamage(damage);
        }
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_arti_dru_full_moon::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
        OnEffectHitTarget += SpellEffectFn(spell_arti_dru_full_moon::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 774 - Germination(155675)
class spell_dru_germination : public SpellScript
{
    PrepareSpellScript(spell_dru_germination);

    bool castGermination = false;

    void HandleTarget(WorldObject*& target)
    {
        if (!target)
            return;

        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(155675))
                return;

            if (Unit* unit = target->ToUnit())
            {
                int32 RejuvenationDur = 0;
                int32 GerminationDur = 0;
                ObjectGuid const& casterGUID = caster->GetGUID();

                if (Aura* Rejuven = unit->GetAura(774, casterGUID))
                    RejuvenationDur = Rejuven->GetDuration();

                if (Aura* Germin = unit->GetAura(GERMINATION, casterGUID))
                    GerminationDur = Germin->GetDuration();

                if (GerminationDur < RejuvenationDur)
                {
                    castGermination = true;
                    caster->CastSpell(unit, GERMINATION, true);
                    target = nullptr;
                }
            }
        }
    }

    void ClearTarget(WorldObject*& target)
    {
        if (castGermination)
            target = nullptr;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_dru_germination::HandleTarget, EFFECT_0, TARGET_UNIT_TARGET_ALLY);
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_dru_germination::ClearTarget, EFFECT_1, TARGET_UNIT_TARGET_ALLY);
    }
};

// 18562 - Swiftmend
class spell_dru_swiftmend : public SpellScript
{
    PrepareSpellScript(spell_dru_swiftmend);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        auto plr = GetCaster()->ToPlayer();
        auto target = GetHitUnit();
        if (!plr || !target)
            return;

        if (AuraEffect* const aurEff = plr->GetAuraEffect(242238, EFFECT_0)) // Item - Druid T20 Restoration 2P Bonus
        {
            int32 cdmod = CalculatePct(GetSpellInfo()->Category.ChargeRecoveryTime, aurEff->GetAmount());
            int32 bp = CalculatePct((100 - target->GetHealthPct()), cdmod);
            plr->ModSpellChargeCooldown(GetSpellInfo()->Id, bp);
        }

		// Extends lifebloom by 10 sec when having T18 Resto 4P bonus with Edraith, Bonds of Aglaya legion legendary
		if (plr->HasAura(185397) && plr->HasAura(207943))
			if (const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(188550))
				if (Aura* aur = target->GetAura(spellInfo->Id, plr->GetGUID()))
					if (AuraEffect const* aurEf = plr->GetAuraEffect(207943, EFFECT_0))
						aur->SetDuration(aur->GetDuration() + aurEf->GetAmount() * IN_MILLISECONDS);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dru_swiftmend::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 8936 - Regrowth
class spell_dru_regrowth : public SpellScript
{
    PrepareSpellScript(spell_dru_regrowth);

    bool haspvptal = false;

    uint32 CallSpecialFunction(uint32 /*Num*/) override
    {
        if (Unit* caster = GetCaster())
        {
            if (haspvptal)
            {
                if (Unit* target = GetExplTargetUnit())
                    if (caster != target)
                        return 1;
            }
        }
        return 0;
    }

    void BeforeHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(209730)) // Protector of the Grove (PvP Talent)
                haspvptal = true;
        }
    }
    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (Unit* caster = GetCaster())
            if (haspvptal)
                if (caster != target)
                    caster->CastSpell(caster, 209731, true);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dru_regrowth::BeforeHit, EFFECT_0, SPELL_EFFECT_HEAL);
        OnEffectHitTarget += SpellEffectFn(spell_dru_regrowth::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// Ysera's Gift - 145108
class spell_dru_ysera_gift : public AuraScript
{
    PrepareAuraScript(spell_dru_ysera_gift);

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& /*isPeriodic*/, int32& amplitude)
    {
        if (Unit* caster = GetCaster())
            if (AuraEffect const* aurEff = caster->GetAuraEffect(253434, EFFECT_0)) // for Item - Druid T21 Restoration 4P Bonus
                AddPct(amplitude, aurEff->GetAmount());
    }

    void HandleEffectPeriodic(AuraEffect const* aurEff)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetUnitOwner())
            {
                float percHeal = aurEff->GetAmount();
                if (AuraEffect* aurEf = caster->GetAuraEffect(210667, EFFECT_4)) // Ekowraith, Creator of Worlds
                    AddPct(percHeal, aurEf->GetAmount());

                uint32 triggerSpell = caster->IsFullHealth() ? 145110 : 145109;
                float heal = CalculatePct(caster->GetMaxHealth(), percHeal);
                if (caster->HasAura(137010)) // for spec Guardian is bp/2
                    heal /= 2.f;

                caster->CastCustomSpell(target, triggerSpell, &heal, nullptr, nullptr, true);
            }
        }
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_dru_ysera_gift::CalcPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_dru_ysera_gift::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Item - Druid T21 Restoration 4P Bonus - 253434
class spell_dru_t21_4p_rest : public AuraScript
{
    PrepareAuraScript(spell_dru_t21_4p_rest);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (auto caster = GetCaster())
            if (AuraEffect* aurEff = caster->GetAuraEffect(145108, EFFECT_0))
                aurEff->RecalculateTickPeriod(caster);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_dru_t21_4p_rest::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Mastery: Nature's Guardian - 155783
class spell_dru_natures_guardian : public AuraScript
{
    PrepareAuraScript(spell_dru_natures_guardian);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (DamageInfo* dmgInfo = eventInfo.GetDamageInfo())
        {
            if (dmgInfo->GetSpellInfo()->Id == 227034)
                return;

            if (uint32 heal = dmgInfo->GetDamage())
            {
                if (Unit* dru = GetUnitOwner())
                {
                    float finalHeal = CalculatePct(heal, aurEff->GetAmount());
                    dru->CastCustomSpell(dru, 227034, &finalHeal, nullptr, nullptr, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dru_natures_guardian::OnProc, EFFECT_1, SPELL_AURA_MOD_ABSORBTION_PERCENT);
    }
};

// Sunfire - 164815
class spell_dru_sunfire_dot : public SpellScript
{
    PrepareSpellScript(spell_dru_sunfire_dot);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(231050)) // Sunfire (lvl 2)
                targets.clear();

            if (Spell* spell = GetSpell())
            {
                for (auto& itr : *spell->GetUniqueTargetInfo())
                {
                    if (Unit* target = ObjectAccessor::GetUnit(*caster, itr->targetGUID))
                        targets.push_back(target);
                }
            }
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dru_sunfire_dot::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// Ashamane's Frenzy - 214843
class spell_dru_ashamane_frenzy_proc : public AuraScript
{
    PrepareAuraScript(spell_dru_ashamane_frenzy_proc);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetProcTarget())
            {
                if (target->GetCreatureType() == CREATURE_TYPE_BEAST)
                    caster->CastSpell(caster, 223971, true);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dru_ashamane_frenzy_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

void AddSC_druid_spell_scripts()
{
    new spell_dru_ferocious_bite();
    new spell_dru_lifebloom();
    new spell_dru_cat_form();
    RegisterAuraScript(spell_dru_frenzied_regeneration);
    new spell_dru_stampeding_roar_speed();
    new spell_dru_teleport_moonglade();
    new spell_dru_swift_flight_passive();
    new spell_druid_rejuvenation();
    new spell_dru_incarnation();
    new spell_dru_incarnation_tree_of_life();
    RegisterSpellScript(spell_dru_swipe);
    new spell_dru_one_with_nature();
    new spell_dru_travel_form();
    new spell_dru_rake();
    new spell_dru_brambles();
    new spell_dru_earthwarden();
    new spell_dru_rend_and_tear();
    new spell_dru_tranquility();
    new spell_druid_flourish();
    new spell_dru_lunar_strike();
    new spell_dru_lunar_strike2();
    new spell_dru_travel_form_remove();
    new spell_dru_fury_of_elune();
    new spell_dru_rage_of_the_sleeper();
    new spell_dru_fury_of_elune_re();
    new spell_dru_shred();
    new spell_dru_thrash();
    new spell_dru_ekowraith_creator_of_worlds();
    new spell_dru_ashamanes_frenzy();
    RegisterSpellScript(spell_dru_nature_balance);
    new spell_dru_rip_and_tear();
    RegisterSpellScript(spell_arti_dru_new_moon);
    RegisterSpellScript(spell_arti_dru_half_moon);
    RegisterSpellScript(spell_arti_dru_full_moon);
    RegisterSpellScript(spell_dru_swiftmend);
    RegisterSpellScript(spell_dru_germination);
    RegisterSpellScript(spell_dru_healing_ouch);
    RegisterSpellScript(spell_dru_ferocious_bite);
    RegisterAuraScript(spell_dru_travel_form_speed_increase);
    RegisterAuraScript(spell_dru_travel_form_speed_increase_dummy);
    RegisterAuraScript(spell_dru_rip);
    RegisterSpellScript(spell_dru_regrowth);
    RegisterAuraScript(spell_dru_ysera_gift);
    RegisterAuraScript(spell_dru_t21_4p_rest);
    RegisterAuraScript(spell_dru_natures_guardian);
    RegisterSpellScript(spell_dru_sunfire_dot);
    RegisterAuraScript(spell_dru_ashamane_frenzy_proc);
}
