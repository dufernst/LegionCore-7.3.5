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
 * Scripts for spells with SPELLFAMILY_PRIEST and SPELLFAMILY_GENERIC spells used by priest players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_pri_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Group.h"

// Called by Heal - 2050, Flash Heal - 2061, Greater Heal - 2060 and Prayer of Healing - 596
// Spirit Shell - 109964
class spell_pri_spirit_shell : public SpellScriptLoader
{
    public:
        spell_pri_spirit_shell() : SpellScriptLoader("spell_pri_spirit_shell") { }

        class spell_pri_spirit_shell_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_spirit_shell_SpellScript);

            void HandleOnHit()
            {
                Unit* target = GetHitUnit();
                Unit* caster = GetCaster();
                if (!target || !caster)
                    return;

                float bp = GetHitHeal();
                if (GetSpellInfo()->Id == 2061)
                {
                    if (AuraEffect* aurEff = caster->GetAuraEffect(200128, 0)) // Trail of Light
                    {
                        if (Aura* auraTrail = aurEff->GetBase())
                        {
                            GuidList saveTargets = auraTrail->GetEffectTargets();
                            if (!saveTargets.empty())
                            {
                                ObjectGuid const& targetFirst = saveTargets.front();
                                ObjectGuid const& targetLast = saveTargets.back();

                                if (targetFirst != target->GetGUID())
                                {
                                    if(Unit* targetTrail = ObjectAccessor::GetPlayer(*caster, targetFirst))
                                    {
                                        if(caster->GetDistance(targetTrail) <= 40.0f)
                                        {
                                            float heal_ = CalculatePct(bp, aurEff->GetAmount());
                                            caster->CastCustomSpell(targetTrail, 234946, &heal_, NULL, NULL,  true);
                                        }
                                        else
                                            auraTrail->RemoveEffectTarget(targetFirst);
                                    }
                                    else
                                        auraTrail->RemoveEffectTarget(targetFirst);

                                    if (targetFirst == targetLast)
                                        auraTrail->AddEffectTarget(target->GetGUID());
                                    else if (targetLast != target->GetGUID())
                                    {
                                        auraTrail->RemoveEffectTarget(targetFirst);
                                        auraTrail->AddEffectTarget(target->GetGUID());
                                    }
                                }
                            }
                            else
                                auraTrail->AddEffectTarget(target->GetGUID());
                        }
                    }
                }
                if (caster->HasAura(109964))
                {
                    if (AuraEffect* aurEff = target->GetAuraEffect(114908, 0))
                        bp += aurEff->GetAmount();
                    if (bp > int32(CalculatePct(target->GetMaxHealth(), 60)))
                        bp = int32(CalculatePct(target->GetMaxHealth(), 60));
                    caster->CastCustomSpell(target, 114908, &bp, NULL, NULL,  true);
                    SetHitHeal(0);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pri_spirit_shell_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_spirit_shell_SpellScript();
        }
};

// Halo - 120692
class spell_pri_halo_heal : public SpellScriptLoader
{
    public:
        spell_pri_halo_heal() : SpellScriptLoader("spell_pri_halo_heal") { }

        class spell_pri_halo_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_halo_heal_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        int32 _heal = GetHitHeal();
                        float Distance = caster->GetDistance(target);
                        float pct = (0.5f * pow((1.01f),(-1 * pow(((Distance - 25.0f) / 2), 4))) + 0.1f + 0.015f*Distance);
                        _heal = int32(_heal * pct);
                        SetHitHeal(_heal);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_halo_heal_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_halo_heal_SpellScript;
        }
};

// Void Shift - 108968, 142723
class spell_pri_void_shift : public SpellScriptLoader
{
    public:
        spell_pri_void_shift() : SpellScriptLoader("spell_pri_void_shift") { }

        class spell_pri_void_shift_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_void_shift_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (GetExplTargetUnit())
                    if (GetExplTargetUnit()->GetTypeId() != TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_TARGETS;

                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        float casterPct;
                        float targetPct;
                        float basePct = GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                        float casterHeal = 0.f, targetHeal = 0.f;
                        float casterDamage = 0.f, targetDamage = 0.f;

                        casterPct = caster->GetHealthPct();
                        targetPct = target->GetHealthPct();

                        if (casterPct < basePct)
                            targetHeal = target->CountPctFromMaxHealth(int32(basePct));
                        else
                            targetHeal = target->CountPctFromMaxHealth(int32(casterPct));

                        if (targetPct < basePct)
                            casterHeal = int32(caster->CountPctFromMaxHealth(basePct));
                        else
                            casterHeal = caster->CountPctFromMaxHealth(int32(targetPct));

                        if (target->GetHealth() > uint32(targetHeal))
                        {
                            targetDamage = target->GetHealth() - targetHeal;
                            targetHeal = 0;
                        }
                        else
                            targetHeal -= target->GetHealth();

                        if (caster->GetHealth() > uint32(casterHeal))
                        {
                            casterDamage = caster->GetHealth() - casterHeal;
                            casterHeal = 0;
                        }
                        else
                            casterHeal -= caster->GetHealth();

                        caster->CastCustomSpell(caster, 118594, &casterDamage, &casterHeal, nullptr, true);
                        caster->CastCustomSpell(target, 118594, &targetDamage, &targetHeal, nullptr, true);
                    }
                }
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_pri_void_shift_SpellScript::CheckTarget);
                OnEffectLaunch += SpellEffectFn(spell_pri_void_shift_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_void_shift_SpellScript;
        }
};

// Guardian Spirit - 47788
class spell_pri_guardian_spirit : public SpellScriptLoader
{
    public:
        spell_pri_guardian_spirit() : SpellScriptLoader("spell_pri_guardian_spirit") { }

        class spell_pri_guardian_spirit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_guardian_spirit_AuraScript);

            uint32 healPct;

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(48153))
                    return false;
                return true;
            }

            bool Load() override
            {
                healPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue();
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                Unit* target = GetTarget();
                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                float healAmount = int32(target->CountPctFromMaxHealth(healPct));
                // remove the aura now, we don't want 40% healing bonus
                Remove(AURA_REMOVE_BY_ENEMY_SPELL);
                target->CastCustomSpell(target, 48153, &healAmount, NULL, NULL, true);
                if (target->HasAura(234689)) // Phyrix's Embrace
                    target->CastSpell(target, 234693, true);
                absorbAmount = dmgInfo.GetDamage();
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                if (Unit* caster = GetCaster())
                    if (caster->HasAura(196437)) // Guardians of the Light
                        if (target->GetGUID() != caster->GetGUID())
                            caster->CastSpell(caster, 47788, true);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                if (Unit* caster = GetCaster())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_EXPIRE)
                    {
                        if (Player* plr = caster->ToPlayer())
                        {
                            if (plr->HasAura(200209) && target->GetGUID() == caster->GetGUID()) // Guardian Angel
                            {
                                if (plr->HasAura(196602)) // Divine Attendant (PvP Talent)
                                    plr->ModifySpellCooldown(47788, -20000);
                                else
                                    plr->ModifySpellCooldown(47788, -140000);
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_guardian_spirit_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_guardian_spirit_AuraScript::Absorb, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                AfterEffectApply += AuraEffectApplyFn(spell_pri_guardian_spirit_AuraScript::OnApply, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectApplyFn(spell_pri_guardian_spirit_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_guardian_spirit_AuraScript();
        }
};

// Voidform - 194249
class spell_pri_voidform : public AuraScript
{
    PrepareAuraScript(spell_pri_voidform);

    enum MyEnum
    {
        Unlock          = 0x0,
        VoidTorrent     = 0x1,
        Dispersion      = 0x2,
        SustainedSanity = 0x4,

        LockAll         = (Dispersion | SustainedSanity),
        LockInsanity    = (VoidTorrent | LockAll),

        T21Shadow4PAura = 251846,
        T21Shadow4PCrit = 252909,
        VoidformVisual2 = 210198,
        VoidformVisual3 = 210197,
        VoidformVisual4 = 210196,

    };

    uint16 curStack = 0;
    int32 saveAmount = 0;
    uint32 status = Unlock;

    uint32 CallSpecialFunction(uint32 Num) override
    {
        status = Unlock;

        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(47585))
                status |= Dispersion;
            if (caster->HasAura(219772))
                status |= SustainedSanity;
            if (caster->HasAura(205065))
                status |= VoidTorrent;

            if (!Num)
            {
                if (AuraEffect* aurEff = GetAura()->GetEffect(EFFECT_1))
                {
                    aurEff->SetCanBeRecalculated(true);
                    aurEff->RecalculateAmount(caster);
                }
            }
        }
        return 1;
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (status & LockAll)
            return;

        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = GetAura())
            {
                if (!(status & LockInsanity))
                    saveAmount += 300;

                curStack = aura->GetStackAmount() + 1;
                
                if (curStack < 101)
                    aura->SetStackAmount(curStack);

                if (caster->HasAura(T21Shadow4PAura))
                {
                    float bp1 = 0.5f * curStack;
                    float bp2 = curStack;
                    caster->CastCustomSpell(caster, T21Shadow4PCrit, &bp1, &bp2, NULL, true);
                }

                switch (curStack)
                {
                    case 15:
                        caster->CastSpell(caster, VoidformVisual2, true);
                        break;
                    case 30:
                        caster->CastSpell(caster, VoidformVisual3, true);
                        break;
                    case 45:
                        caster->CastSpell(caster, VoidformVisual4, true);
                        break;
                }
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (caster->HasAura(194179)) // Sphere of Insanity
            caster->RemoveAllMinionsByFilter(98680);

        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEATH)
            return;

        int8 _count = GetAura()->GetStackAmount();
        if (Unit* caster = GetCaster())
            if (caster->HasAura(199849)) // Lingering Insanity
                for (int i = 0; i < _count; i++)
                    caster->CastSpell(caster, 197937, true);

        caster->RemoveAurasDueToSpell(VoidformVisual2);
        caster->RemoveAurasDueToSpell(VoidformVisual3);
        caster->RemoveAurasDueToSpell(VoidformVisual4);
        caster->RemoveAurasDueToSpell(T21Shadow4PCrit);
        caster->CastSpell(caster, 232698, true);
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            CallSpecialFunction(1);

            if (caster->HasAura(T21Shadow4PAura))
            {
                float bp = 0.5f;
                caster->CastCustomSpell(caster, T21Shadow4PCrit, &bp, &bp, NULL, true);
            }
            caster->RemoveAurasDueToSpell(232698);
        }
    }

    void CalculateAmount1(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        if (status & LockInsanity)
        {
            amount = 0;
            return;
        }

        if (Unit* caster = GetCaster())
        {
            float mod = caster->HasAura(242273) ? 0.9f : 1.f;
            amount = (amount - saveAmount) * mod;
        }
    }

    void CalculateAmount2(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        amount = curStack;
    }

    void CalculateAmount3(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (AuraEffect const* aurEff0 = caster->GetAuraEffect(194378, EFFECT_0)) // Mass Hysteria
                amount = aurEff0->GetAmount() * curStack;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_voidform::CalculateAmount3, EFFECT_9, SPELL_AURA_ADD_PCT_MODIFIER);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_voidform::CalculateAmount2, EFFECT_2, SPELL_AURA_MELEE_SLOW);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_voidform::CalculateAmount1, EFFECT_1, SPELL_AURA_MOD_POWER_REGEN);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_voidform::OnTick, EFFECT_4, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_voidform::OnRemove, EFFECT_4, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_pri_voidform::OnApply, EFFECT_4, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Divine Hymn - 64844
class spell_pri_divine_hymn : public SpellScriptLoader
{
    public:
        spell_pri_divine_hymn() : SpellScriptLoader("spell_pri_divine_hymn") { }

        class spell_pri_divine_hymn_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_divine_hymn_SpellScript);

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
                OnHit += SpellHitFn(spell_pri_divine_hymn_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_divine_hymn_SpellScript();
        }
};

// Light's Wrath - 207946
class spell_pri_lights_wrath : public SpellScriptLoader
{
    public:
        spell_pri_lights_wrath() : SpellScriptLoader("spell_pri_lights_wrath") { }

        class spell_pri_lights_wrath_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_lights_wrath_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetHitUnit())
                    {
                        uint32 count = caster->GetCountMyAura(caster->HasAura(214205) ? 214206 : 194384);
                        int32 perc = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster) * count;
                        int32 damage = GetHitDamage();
                        damage += CalculatePct(damage, perc);
                        SetHitDamage(damage);
                        if(AuraEffect const* aurEff = caster->GetAuraEffect(233978, EFFECT_0)) // Kam Xi'raff
                        {
                            int32 duration = 1000 + aurEff->GetAmount() * count * 1000;
                            caster->CastSpellDuration(caster, 233997, true, duration);
                        }
                    }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_lights_wrath_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_lights_wrath_SpellScript();
        }
};

// Shadow Word: Death - 32379
class spell_pri_shadow_word_death : public SpellScriptLoader
{
    public:
        spell_pri_shadow_word_death() : SpellScriptLoader("spell_pri_shadow_word_death") { }

        class spell_pri_shadow_word_death_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_shadow_word_death_SpellScript);

            float bp = 0.f;
            Unit* target = NULL;

            void HandleCast()
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetExplTargetUnit())
                    {
                        float bp0 = 1500.f;
                        if (target->isAlive())
                            caster->CastCustomSpell(caster, 190714, &bp0, NULL, NULL, true);
                        else
                            caster->CastSpell(caster, 190714, true);
                    }
            }

            void Register() override
            {
                OnFinishCast += SpellCastFn(spell_pri_shadow_word_death_SpellScript::HandleCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_shadow_word_death_SpellScript();
        }
};

// Plea - 200829
class spell_pri_plea : public SpellScript
{
    PrepareSpellScript(spell_pri_plea);

    void ApplyAtonement(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        if (caster->HasAura(214205)) // Trinity (Honor Talent), source aura Atonement 
            caster->CastSpell(target, 214206, true);
        else
            caster->CastSpell(target, 194384, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_plea::ApplyAtonement, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// Shadow Mend - 186263
class spell_pri_shadow_mend: public SpellScript
{
    PrepareSpellScript(spell_pri_shadow_mend);

    void HandleHeal(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                float bp0 = GetHitHeal() / 20.f;
                float bp1 = GetHitHeal() / 2.f;

                if (caster->HasAura(81749) && !caster->HasAura(214205))
                    caster->CastSpell(target, 194384, true);

                if (caster->GetGUID() == target->GetGUID() && caster->HasAura(193063)) // Masochism
                {
                    caster->CastCustomSpell(caster, 193065, &bp0, nullptr, nullptr, true);
                    return;
                }

                if (target->isInCombat())
                    caster->CastCustomSpell(target, 187464, &bp1, nullptr, nullptr, true); // send penalty proc dmg
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_shadow_mend::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// Shadow Mend proc - 187464
class spell_pri_shadow_mend_aura : public AuraScript
{
    PrepareAuraScript(spell_pri_shadow_mend_aura);

    int32 tempdamageSM = 0;

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* target = eventInfo.GetActionTarget();
        if (!target)
            return;

        if (!eventInfo.GetDamageInfo())
            return;

        tempdamageSM -= eventInfo.GetDamageInfo()->GetDamage(); // penalty damage

        if (Aura* shadowMendAura = GetAura())
            if (tempdamageSM <= 0)
                shadowMendAura->Remove();
        return;
    }

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !target)
            return;

        if (!target->isInCombat())
            GetAura()->Remove(); // if leave combat, aura need remove
        else
            target->CastCustomSpell(186439, SPELLVALUE_BASE_POINT0, aurEff->GetAmount(), target, true, nullptr, nullptr, caster->GetGUID());
    }

    void CalcDamage(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
    {
        tempdamageSM += amount; // Calculate penalty damage
        amount = tempdamageSM / 10;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_shadow_mend_aura::CalcDamage, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_pri_shadow_mend_aura::OnProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_shadow_mend_aura::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Atonement - 194384 
// Atonement - 214206 , by talent Trinity - 214205 (PvP Talent)
class spell_pri_atonement_aura : public AuraScript
{
    PrepareAuraScript(spell_pri_atonement_aura);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(198074)) // Sins of the Many
                caster->CastSpell(caster, 198076, true);
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aur = caster->GetAura(198076)) // Sins of the Many
                aur->ModStackAmount(-1);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_pri_atonement_aura::OnApply, EFFECT_1, SPELL_AURA_MOD_HEALING_RECEIVED, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_atonement_aura::OnRemove, EFFECT_1, SPELL_AURA_MOD_HEALING_RECEIVED, AURA_EFFECT_HANDLE_REAL);
    }
};

// Clarity of Will - 152118
class spell_pri_clarity_of_will : public SpellScriptLoader
{
    public:
        spell_pri_clarity_of_will() : SpellScriptLoader("spell_pri_clarity_of_will") { }

        class spell_pri_clarity_of_will_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_clarity_of_will_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                int32 pctHP = amount * 2;
                if (Unit* target = GetUnitOwner())
                {
                    if(AuraEffect const* aurEff = target->GetAuraEffect(target->HasAura(214205) ? 214206 : 194384, EFFECT_1))
                        amount += CalculatePct(amount, aurEff->GetAmount());
                }
                amount += aurEff->GetOldBaseAmount();
                if (amount > pctHP)
                    amount = pctHP;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_clarity_of_will_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_clarity_of_will_AuraScript();
        }
};

// Power Word: Shield - 17
class spell_pri_power_word_shield : public SpellScriptLoader
{
    public:
        spell_pri_power_word_shield() : SpellScriptLoader("spell_pri_power_word_shield") { }

        class spell_pri_power_word_shield_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_power_word_shield_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (caster->HasAura(81749))
                            if (!caster->HasAura(214205))
                                caster->CastSpell(target, 194384, true);

                        if (AuraEffect const* aurEff = caster->GetAuraEffect(211563, EFFECT_0)) // Item - Priest T19 Discipline 4P Bonus
                            if (Aura* aur = target->GetAura(194384, caster->GetGUID()))
                                if (caster->HasAura(47536))
                                    aur->SetDuration(aur->GetDuration() + aurEff->GetAmount() * IN_MILLISECONDS);
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pri_power_word_shield_SpellScript::HandleOnHit);
            }
        };

        class spell_pri_power_word_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_power_word_shield_AuraScript);

            enum MyEnum
            {
                StrengthOfSoul = 197548
            };

            uint32 absorb = 0;

            void CalculateAmount0(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* eff = caster->GetAuraEffect(caster->HasAura(214205) ? 214206 : 194384, EFFECT_1))
                        amount += CalculatePct(amount, eff->GetAmount());
                    if (AuraEffect const* eff = caster->GetAuraEffect(214576, EFFECT_1)) // legendary item bonus
                        amount += CalculatePct(amount, eff->GetAmount());
                    if (AuraEffect const* eff = caster->GetAuraEffect(197729, EFFECT_0)) // artifact perk
                        amount += CalculatePct(amount, eff->GetAmount());
                    if (AuraEffect const* eff = caster->GetAuraEffect(238135, EFFECT_0)) // artifact perk
                        amount += CalculatePct(amount, eff->GetAmount());

                    if (Player* plr = caster->ToPlayer())
                    {
                        float critChance = plr->GetFloatValue(PLAYER_FIELD_CRIT_PERCENTAGE);
                        if (roll_chance_f(critChance))
                            amount *= plr->CanPvPScalar() ? 1.5f : 2.f;
                    }

                    if (Unit* target = GetAura()->GetUnitOwner())
                    {
                        if (caster != target)
                        {
                            if (AuraEffect const* eff = caster->GetAuraEffect(197781, EFFECT_0)) // Share in the Light
                            {
                                float bp = CalculatePct(amount, eff->GetAmount());
                                caster->CastCustomSpell(caster, 210027, &bp, NULL, NULL, true);
                            }
                        }
                    }
                }
            }

            void CalculateAmount1(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                if (!amount)
                    return;

                if (AuraEffect* eff0 = GetAura()->GetEffect(EFFECT_0))
                    absorb = CalculatePct(eff0->GetAmount(), amount);
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (!absorb)
                    return;

                if (Aura* aura = GetAura())
                {
                    if (AuraEffect* eff0 = aura->GetEffect(EFFECT_0))
                    {
                        uint32 auraAbsorb = eff0->GetAmount();

                        if (auraAbsorb <= absorb)
                        {
                            aura->Remove();
                            return;
                        }
                        eff0->SetAmount(auraAbsorb - absorb);
                    }
                }
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(197535))
                    {
                        if (Unit* target = GetUnitOwner())
                            caster->CastSpell(target, StrengthOfSoul, true);
                    }
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetUnitOwner())
                    target->RemoveAurasDueToSpell(StrengthOfSoul, GetCasterGUID());
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount0, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_power_word_shield_AuraScript::CalculateAmount1, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_power_word_shield_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectApply += AuraEffectApplyFn(spell_pri_power_word_shield_AuraScript::OnApply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_power_word_shield_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_power_word_shield_SpellScript();
        }

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_power_word_shield_AuraScript();
        }
};

// Power Word: Radiance - 194509
class spell_pri_pw_radiance : public SpellScript
{
    PrepareSpellScript(spell_pri_pw_radiance);

    void HandleOnHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (!caster->HasAura(214205))
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(194384))
                    {
                        int32 dur = CalculatePct(GetSpellInfo()->Effects[EFFECT_3]->BasePoints, spellInfo->GetDuration());
                        Aura* aur = target->GetAura(194384, caster->GetGUID());
                        if (!aur || aur->GetDuration() < dur)
                            caster->AddAura(194384, target, nullptr, 0, dur, dur);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_pri_pw_radiance::HandleOnHit);
    }
};

// Leap of Faith - 73325
class spell_pri_leap_of_faith : public SpellScriptLoader
{
    public:
        spell_pri_leap_of_faith() : SpellScriptLoader("spell_pri_leap_of_faith") { }

        class spell_pri_leap_of_faith_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_leap_of_faith_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if(_player->HasAura(196611)) // Delivered from Evil(Honor Talent)
                            target->RemoveAurasWithMechanic((1<<MECHANIC_SNARE)|(1<<MECHANIC_ROOT));
                        target->CastSpell(_player, 92832, true);
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_pri_leap_of_faith_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_leap_of_faith_SpellScript;
        }
};

// Desperate Prayer - 19236
class spell_pri_desperate_prayer : public SpellScriptLoader
{
    public:
        spell_pri_desperate_prayer() : SpellScriptLoader("spell_pri_desperate_prayer") { }

        class spell_pri_desperate_prayer_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_desperate_prayer_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (AuraEffect* aurEff0 = const_cast<AuraEffect*>(aurEff->GetBase()->GetEffect(EFFECT_0)))
                    //if (aurEff0->GetAmount() >= aurEff->GetAmount())
                        aurEff0->ChangeAmount(aurEff0->GetAmount() - aurEff->GetAmount());
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_desperate_prayer_AuraScript::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_desperate_prayer_AuraScript();
        }
};

// Lingering Insanity - 197937
class spell_pri_lingering_insanity : public SpellScriptLoader
{
    public:
        spell_pri_lingering_insanity() : SpellScriptLoader("spell_pri_lingering_insanity") { }

        class spell_pri_lingering_insanity_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_lingering_insanity_AuraScript);

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* caster = GetCaster())
                    if(AuraEffect const* aurEff = caster->GetAuraEffect(199849, EFFECT_0))
                        GetAura()->ModStackAmount(-aurEff->GetAmount());
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_lingering_insanity_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_lingering_insanity_AuraScript();
        }
};

// Purify - 527
class spell_pri_purify : public SpellScriptLoader
{
    public:
        spell_pri_purify() : SpellScriptLoader("spell_pri_purify") { }

        class spell_pri_purify_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_purify_SpellScript);

            void OnSuccessfulDispel(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                if (AuraEffect* aurEff = caster->GetAuraEffect(196439, 0)) // Purified Resolve (Honor Talent)
                {
                    float absort = target->CountPctFromMaxHealth(aurEff->GetAmount());
                    caster->CastCustomSpell(target, 196440, &absort, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectSuccessfulDispel += SpellEffectFn(spell_pri_purify_SpellScript::OnSuccessfulDispel, EFFECT_0, SPELL_EFFECT_DISPEL);
                OnEffectSuccessfulDispel += SpellEffectFn(spell_pri_purify_SpellScript::OnSuccessfulDispel, EFFECT_1, SPELL_EFFECT_DISPEL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_purify_SpellScript();
        }
};

// Ray of Hope (Honor Talent) - 197268
class spell_pri_ray_of_hopet : public SpellScriptLoader
{
    public:
        spell_pri_ray_of_hopet() : SpellScriptLoader("spell_pri_ray_of_hopet") { }

        class spell_pri_ray_of_hopet_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_ray_of_hopet_AuraScript);

            float absorbDamage = 0.f;
            float absorbHeals = 0.f;

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void AbsorbDam(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = dmgInfo.GetDamage();
                absorbDamage += dmgInfo.GetDamage();
            }

            void AbsorbHeal(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = dmgInfo.GetDamage();
                absorbHeals += dmgInfo.GetDamage() + CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_4]->CalcValue(GetCaster()));
            }

            void OnRemove0(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetTarget())
                        caster->CastCustomSpell(target, 197341, &absorbDamage, NULL, NULL, true);
            }

            void OnRemove1(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetTarget())
                        caster->CastCustomSpell(target, 197336, &absorbHeals, NULL, NULL, true);
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_ray_of_hopet_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_ray_of_hopet_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_ray_of_hopet_AuraScript::AbsorbDam, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_ray_of_hopet_AuraScript::AbsorbHeal, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
                AfterEffectRemove += AuraEffectRemoveFn(spell_pri_ray_of_hopet_AuraScript::OnRemove0, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_pri_ray_of_hopet_AuraScript::OnRemove1, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_ray_of_hopet_AuraScript();
        }
};

// Shadow Mania - 199572
class spell_pri_shadow_mania : public SpellScriptLoader
{
    public:
        spell_pri_shadow_mania() : SpellScriptLoader("spell_pri_shadow_mania") { }

        class spell_pri_shadow_mania_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_shadow_mania_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->GetCountMyAura(34914) >= aurEff->GetAmount())
                    {
                        if (!caster->HasAura(199579))
                            caster->CastSpell(caster, 199579, true);
                    }
                    else
                        caster->RemoveAurasDueToSpell(199579);
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pri_shadow_mania_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_shadow_mania_AuraScript();
        }
};

// Premonition - 209780
class spell_pri_premonition : public SpellScriptLoader
{
    public:
        spell_pri_premonition() : SpellScriptLoader("spell_pri_premonition") { }

        class spell_pri_premonition_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_premonition_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    caster->CastSpell(caster, 194384, true);
                    if (caster->GetCountMyAura(194384) > 0)
                        caster->CastSpellDelay(caster, 209885, true, IN_MILLISECONDS);
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_pri_premonition_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_premonition_SpellScript();
        }
};

// Premonition (Honor Talent) - 209885
class spell_pri_premonition_pvp : public SpellScriptLoader
{
    public:
        spell_pri_premonition_pvp() : SpellScriptLoader("spell_pri_premonition_pvp") { }

        class spell_pri_premonition_pvp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_premonition_pvp_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(209780);
                if(!spellInfo)
                    return;

                if (Unit* caster = GetCaster())
                {
                    std::vector<TargetInfoPtr>* memberList = GetSpell()->GetUniqueTargetInfo();
                    if(memberList->empty())
                        return;

                    int32 duration = spellInfo->Effects[EFFECT_0]->BasePoints;
                    uint32 count = 0;
                    for (std::vector<TargetInfoPtr>::iterator ihit = memberList->begin(); ihit != memberList->end(); ++ihit)
                    {
                        if ((*ihit)->effectMask & (1 << EFFECT_0))
                        {
                            if(Unit* member = ObjectAccessor::GetUnit(*caster, (*ihit)->targetGUID))
                                if(Aura* aura = member->GetAura(194384))
                                    aura->SetDuration(aura->GetDuration() + duration);
                            ++count;
                        }
                    }
                    if (!count)
                        return;

                    SetHitDamage(caster->GetSpellPowerDamage(spellInfo->GetSchoolMask()) * spellInfo->Effects[EFFECT_0]->BonusCoefficient * count);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_premonition_pvp_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_premonition_pvp_SpellScript();
        }
};

// Mental Fortitude - 194022
class spell_pri_mental_fortitude : public SpellScriptLoader
{
    public:
        spell_pri_mental_fortitude() : SpellScriptLoader("spell_pri_mental_fortitude") { }

        class spell_pri_mental_fortitude_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_mental_fortitude_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* target = GetUnitOwner())
                {
                    amount += aurEff->GetOldBaseAmount();
                    int32 pctHP = 0;
                    if(AuraEffect const* aurEff0 = target->GetAuraEffect(194018, EFFECT_0)) // Mental Fortitude
                        pctHP = target->CountPctFromMaxHealth(aurEff0->GetAmount());

                    if (amount > pctHP)
                        amount = pctHP;
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_mental_fortitude_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_mental_fortitude_AuraScript();
        }
};

// Penance - 47666
class spell_pri_penance : public SpellScriptLoader
{
    public:
        spell_pri_penance() : SpellScriptLoader("spell_pri_penance") { }

        class spell_pri_penance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HasAura(225683)) // Promises of N'ero
                        return;

                    uint32 heal_ = GetHitDamage();
                    if (Unit::AuraList* _auraList = caster->GetMyAura(81782))
                        for (Unit::AuraList::iterator itr = _auraList->begin(); itr != _auraList->end(); ++itr)
                            if (Aura* _aura = (*itr))
                                if (Unit* unitTarget = _aura->GetUnitOwner())
                                    caster->HealBySpell(unitTarget, GetSpellInfo(), heal_);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_pri_penance_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_penance_SpellScript();
        }
};

// Vampiric Touch - 34914
class spell_pri_vampiric_touch : public SpellScriptLoader
{
    public:
        spell_pri_vampiric_touch() : SpellScriptLoader("spell_pri_vampiric_touch") { }

        class spell_pri_vampiric_touch_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pri_vampiric_touch_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                Unit* caster = GetCaster();
                Unit* dispeller = dispelInfo->GetDispeller();
                if (!caster || !dispeller)
                    return;

                dispeller->CastSpellDelay(dispeller, 87204, true, 50, nullptr, nullptr, caster->GetGUID());
            }

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                if (Unit* caster = GetCaster())
                    if (caster->HasAura(238558))
                        caster->CastSpell(target, 589, true);
            }

            void Register() override
            {
                OnDispel += AuraDispelFn(spell_pri_vampiric_touch_AuraScript::HandleDispel);
                AfterEffectApply += AuraEffectApplyFn(spell_pri_vampiric_touch_AuraScript::OnApply, EFFECT_1, SPELL_AURA_PERIODIC_LEECH, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_pri_vampiric_touch_AuraScript();
        }
};

// Penance - 47540
class spell_pri_penance_cast : public SpellScriptLoader
{
    public:
        spell_pri_penance_cast() : SpellScriptLoader("spell_pri_penance_cast") { }
 
        class spell_pri_penance_cast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_pri_penance_cast_SpellScript);
  
            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetExplTargetUnit())
                        if (caster->IsFriendlyTo(target) && !caster->HasAura(200347))
                            return SPELL_FAILED_BAD_TARGETS;
                return SPELL_CAST_OK;
            }
			
            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* unitTarget = GetHitUnit())
                    {
                        if (!unitTarget->isAlive())
                            return;
 
                        if (_player->IsFriendlyTo(unitTarget))
                            _player->CastSpell(unitTarget, 47757, true);
                        else
                            _player->CastSpell(unitTarget, 47758, true);
                    }
                }
            }
 
            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_pri_penance_cast_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_pri_penance_cast_SpellScript::CheckCast);
            }
        };
 
        SpellScript* GetSpellScript() const override
        {
            return new spell_pri_penance_cast_SpellScript;
        }
};

// Smite - 585
class spell_pri_smite : public SpellScript
{
    PrepareSpellScript(spell_pri_smite);

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Player* plr = caster->ToPlayer();
        if (!caster || !target || !plr)
            return;

        if (AuraEffect const* aurEff = caster->GetAuraEffect(231682, EFFECT_0)) // Smite (Discipline)
        {
            float bp = CalculatePct(caster->GetSpellPowerDamage(SPELL_SCHOOL_MASK_HOLY), aurEff->GetAmount());
            caster->CastCustomSpell(target, 208772, &bp, nullptr, nullptr, true);
            caster->CastSpell(caster, 208771, true);
        }

        if (AuraEffect const* aurEff = caster->GetAuraEffect(231687, EFFECT_0)) // Smite (Holy)
        {
            if (roll_chance_f(aurEff->GetAmount()))
                plr->RemoveSpellCooldown(14914, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_smite::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Smite - 208771 
class spell_pri_smite_absorb : public AuraScript
{
    PrepareAuraScript(spell_pri_smite_absorb);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* attacker = dmgInfo.GetAttacker())
            {
                if (uint32 damage = dmgInfo.GetDamage())
                {
                    if (Aura* aur = attacker->GetAura(208772, caster->GetGUID()))
                    {
                        if (AuraEffect* eff = aur->GetEffect(EFFECT_0))
                        {
                            float amount = eff->GetAmount();

                            if (amount > damage)
                            {
                                eff->SetAmount(amount - damage);
                                absorbAmount = damage;
                                return;
                            }

                            absorbAmount = amount;
                            aur->Remove();
                            return;
                        }
                    }
                }
            }
        }
        absorbAmount = 0;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_smite_absorb::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_pri_smite_absorb::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// 232698 - Shadowform
class spell_pri_shadowform : public AuraScript
{
    PrepareAuraScript(spell_pri_shadowform);

    int32 update = 1000;

    void OnUpdate(uint32 diff)
    {
        if (!update)
            return;

        update -= diff;

        if (update <= 0)
        {
            if (Unit* caster = GetCaster())
            {
                // check power
                int32 insanity = caster->GetPower(POWER_INSANITY);
                if (insanity >= 2500)
                    caster->CastSpell(caster, 185909, true); // visual insanity 2
                if (insanity >= 5000)
                    caster->CastSpell(caster, 185910, true); // visual insanity 3
                if (insanity >= 7500)
                    caster->CastSpell(caster, 185911, true); // visual insanity 4

                caster->CastSpell(caster, 185908, true); // visual insanity 1
            }
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
        {
            caster->RemoveAurasDueToSpell(185908);
            caster->RemoveAurasDueToSpell(185909);
            caster->RemoveAurasDueToSpell(185910);
            caster->RemoveAurasDueToSpell(185911);
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_pri_shadowform::OnUpdate);
        AfterEffectRemove += AuraEffectRemoveFn(spell_pri_shadowform::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

// Renew - 139
class spell_pri_renew : public AuraScript
{
    PrepareAuraScript(spell_pri_renew);

    void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* owner = GetUnitOwner())
            {
                if (caster->GetGUID() == owner->GetGUID() && caster->HasAura(235189))
                    return;
            }
        }
        amount = 0;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_pri_renew::CalculateAmount, EFFECT_2, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }
};

// 81751 - Atonement (Heal)
class spell_pri_atonement_heal : public SpellScript
{
    PrepareSpellScript(spell_pri_atonement_heal);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        auto target = GetHitUnit();
        if (!caster || !target)
            return;

        if (GetSpell()->IsCritForTarget(target))
        {
            float mod = caster->CanPvPScalar() ? 0.75f : 0.5f;
            SetHitHeal(GetHitHeal() * mod);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_atonement_heal::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// Prayer of Mending - 41635
class spell_pri_prayer_of_mending_proc_aura : public AuraScript
{
    PrepareAuraScript(spell_pri_prayer_of_mending_proc_aura);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* owner = GetUnitOwner())
            {
                if (Aura* aura = GetAura())
                {
                    caster->CastSpell(owner, 33110, true);
                    owner->CastSpell(owner, 155793, true, nullptr, aurEff);
                    aura->Remove();
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_pri_prayer_of_mending_proc_aura::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 33110 - Prayer of Mending (Heal)
class spell_pri_prayer_of_mending_heal : public SpellScript
{
    PrepareSpellScript(spell_pri_prayer_of_mending_heal);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* priest = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (AuraEffect* const aurEff2 = priest->GetAuraEffect(193157, EFFECT_0)) // Benediction
                {
                    if (roll_chance_f(aurEff2->GetAmount()))
                        priest->CastSpell(target, 139, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_prayer_of_mending_heal::HandleOnHit, EFFECT_0, SPELL_EFFECT_HEAL);
    }
};

// 155793 - Prayer of Mending (Target Spell)
class spell_pri_prayer_of_mending_target_spell : public SpellScript
{
    PrepareSpellScript(spell_pri_prayer_of_mending_target_spell);

    void HandleOnLaunchTarget(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (AuraEffect const* triggeredByAuraEff = GetTriggeredAuraEff())
            {
                if (Aura* aura = triggeredByAuraEff->GetBase())
                {
                    if (Unit* priest = aura->GetCaster())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            uint32 bp = aura->GetStackAmount();

                            if (!priest->HasAura(210994))
                            {
                                bp--;

                                if (AuraEffect* aurEff = priest->GetAuraEffect(196358, EFFECT_0)) // Say Your Prayers
                                    if (roll_chance_i(aurEff->GetAmount()))
                                        bp++;
                            }

                            if (bp)
                            {
                                priest->CastSpellDuration(target, 41635, true, 0, bp);
                                caster->CastSpell(target, 225275, true);
                            }
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_pri_prayer_of_mending_target_spell::HandleOnLaunchTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 199845 - Psyflay (PvP Talent)
class spell_pri_psyflay_pvp : public SpellScript
{
    PrepareSpellScript(spell_pri_psyflay_pvp);

    void FilterTarget(std::list<WorldObject*>& targetList)
    {
        targetList.clear();

        if (Unit* caster = GetCaster())
        {
            if (Unit* owner = caster->GetOwner())
            {
                for (auto aur : owner->m_whoHasMyAuras)
                {
                    for (auto spell : aur.second)
                    {
                        if (spell == 211522)
                        {
                            if (Unit* target = ObjectAccessor::GetUnit(*caster, aur.first))
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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psyflay_pvp::FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psyflay_pvp::FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_pri_psyflay_pvp::FilterTarget, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// Evangelism - 246287
class spell_pri_evangelism : public SpellScript
{
    PrepareSpellScript(spell_pri_evangelism);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                int32 dur = GetSpellInfo()->Effects[EFFECT_0]->BasePoints * IN_MILLISECONDS;
                if (Aura* aura = unitTarget->GetAura(caster->HasAura(214205) ? 214206 : 194384, caster->GetGUID()))
                    aura->SetDuration(aura->GetDuration() + dur);
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_pri_evangelism::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// Archangel - 197862
class spell_pri_archangel : public SpellScript
{
    PrepareSpellScript(spell_pri_archangel);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                if (Aura* aura = unitTarget->GetAura(caster->HasAura(214205) ? 214206 : 194384, caster->GetGUID()))
                    aura->RefreshTimers();
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_pri_archangel::HandleScript, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// Mind Flay - 15407
class spell_pri_mind_flay : public AuraScript
{
    PrepareAuraScript(spell_pri_mind_flay);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetUnitOwner();
        if (!target)
            return;

        if (Unit* caster = GetCaster())
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode == AURA_REMOVE_BY_EXPIRE)
            {
                if (caster->HasAura(199445)) // Mind Trauma	(Honor Talent)
                {
                    caster->CastSpell(target, 247777, true);
                    if (Aura* aura = target->GetAura(247777, caster->GetGUID()))
                    {
                        if (Aura* aur = caster->GetAura(247776))
                        {
                            aur->SetStackAmount(aura->GetStackAmount());
                            aur->RefreshDuration();
                        }
                        else
                            caster->AddAura(247776, caster);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_pri_mind_flay::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 32375
class spell_pri_mass_dispel : public SpellScript
{
    PrepareSpellScript(spell_pri_mass_dispel);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto target = GetHitUnit())
        {
            target->RemoveAura(33786);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_pri_mass_dispel::HandleOnHit, EFFECT_0, SPELL_EFFECT_DISPEL);
    }
};

void AddSC_priest_spell_scripts()
{
    new spell_pri_spirit_shell();
    new spell_pri_halo_heal();
    new spell_pri_void_shift();
    new spell_pri_guardian_spirit();
    new spell_pri_divine_hymn();
    new spell_pri_lights_wrath();
    new spell_pri_shadow_word_death();
    RegisterSpellScript(spell_pri_plea);
    RegisterSpellScript(spell_pri_shadow_mend);
    RegisterAuraScript(spell_pri_shadow_mend_aura);
    RegisterAuraScript(spell_pri_atonement_aura);
    new spell_pri_clarity_of_will();
    new spell_pri_power_word_shield();
    RegisterSpellScript(spell_pri_pw_radiance);
    new spell_pri_leap_of_faith();
    new spell_pri_desperate_prayer();
    new spell_pri_lingering_insanity();
    new spell_pri_purify();
    new spell_pri_ray_of_hopet();
    new spell_pri_shadow_mania();
    new spell_pri_premonition();
    new spell_pri_premonition_pvp();
    new spell_pri_mental_fortitude();
    new spell_pri_penance();
    new spell_pri_vampiric_touch();
    new spell_pri_penance_cast();
    RegisterSpellScript(spell_pri_smite);
    RegisterAuraScript(spell_pri_smite_absorb);
    RegisterAuraScript(spell_pri_shadowform);
    RegisterSpellScript(spell_pri_atonement_heal);
    RegisterSpellScript(spell_pri_prayer_of_mending_heal);
    RegisterSpellScript(spell_pri_prayer_of_mending_target_spell);
    RegisterAuraScript(spell_pri_renew); 
    RegisterAuraScript(spell_pri_prayer_of_mending_proc_aura);
    RegisterAuraScript(spell_pri_voidform);
    RegisterSpellScript(spell_pri_psyflay_pvp);
    RegisterSpellScript(spell_pri_evangelism);
    RegisterSpellScript(spell_pri_archangel);
    RegisterAuraScript(spell_pri_mind_flay);
    RegisterSpellScript(spell_pri_mass_dispel);
}