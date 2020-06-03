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
 * Scripts for spells with SPELLFAMILY_MAGE and SPELLFAMILY_GENERIC spells used by mage players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_mage_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ScriptedCreature.h"
#include "GridNotifiers.h"
//#include "AreaTriggerAI.h"
//#include "AreaTrigger.h"

// Cauterize - 86949
class spell_mage_cauterize : public SpellScriptLoader
{
    public:
        spell_mage_cauterize() : SpellScriptLoader("spell_mage_cauterize") { }

        class spell_mage_cauterize_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_cauterize_AuraScript);

            uint32 healtPct;

            bool Load() override
            {
                healtPct = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                return GetUnitOwner()->ToPlayer();
            }

            void CalculateAmount(AuraEffect const* /*AuraEffect**/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                Unit* target = GetCaster();

                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                if (target->ToPlayer()->HasAura(87024))
                    return;

                float bp1 = target->CountPctFromMaxHealth(healtPct);
                target->CastCustomSpell(target, 87023, NULL, &bp1, NULL, true);
                target->CastCustomSpell(target, 87024, NULL, &bp1, NULL, true);

                absorbAmount = dmgInfo.GetDamage();
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_cauterize_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_cauterize_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_cauterize_AuraScript();
        }
};

class CheckArcaneBarrageImpactPredicate
{
    public:
        CheckArcaneBarrageImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

// Arcane Barrage - 44425
class spell_mage_arcane_barrage : public SpellScript
{
    PrepareSpellScript(spell_mage_arcane_barrage);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        if (Player* _player = GetCaster()->ToPlayer())
        {
            if (Unit* target = GetHitUnit())
            {
                if (_player->GetSelectedUnit() != target)
                {
                    int32 perc = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
                    SetHitDamage(CalculatePct(GetHitDamage(), perc));
                }
                if (AuraEffect const* aurEff = _player->GetAuraEffect(205028, EFFECT_0))
                    SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), aurEff->GetAmount() * GetSpell()->GetTargetCount()));
            }
        }
    }

    void TakePower(Powers /*power*/, int32 &/*amount*/)
    {
        if (Unit* caster = GetCaster())
            caster->ModifyPower(POWER_ARCANE_CHARGES, -caster->GetPower(POWER_ARCANE_CHARGES));
    }

    void Register() override
    {
        OnTakePower += TakePowertFn(spell_mage_arcane_barrage::TakePower);
        OnEffectHitTarget += SpellEffectFn(spell_mage_arcane_barrage::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Flurry - 44614
class spell_mage_flurry : public SpellScriptLoader
{
public:
    spell_mage_flurry() : SpellScriptLoader("spell_mage_flurry") {}

    class spell_mage_flurry_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mage_flurry_SpellScript);

        void HandleLaunchTarget(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetHitUnit())
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(228354))
                    {
                        float damage = 1.f;
                        std::vector<uint32> ExcludeAuraList;
                        damage = caster->SpellDamageBonusDone(target, spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE, ExcludeAuraList, EFFECT_1);

                        if (Aura* aura = caster->GetAura(231584))
                        {
                            if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                            {
                                if (caster->HasAura(190446))
                                {
                                    eff->SetAmount(1.f);
                                }
                                else
                                {
                                    eff->SetAmount(0.f);
                                }
                            }
                        }

                        float dmgMod = 0.f;
                        float finalDamage = damage;

                        if (AuraEffect const* aurEff = caster->GetAuraEffect(251859, EFFECT_0)) // Item - Mage T21 Frost 2P Bonus
                            dmgMod = aurEff->GetAmount();

                        if (dmgMod)
                            finalDamage += CalculatePct(damage, dmgMod);
                        
                        caster->CastCustomSpell(target, 228596, &finalDamage, NULL, NULL, true);

                        if (dmgMod)
                            finalDamage += CalculatePct(damage, dmgMod);

                        ObjectGuid targetGUID = target->GetGUID();
                        caster->AddDelayedEvent(300, [caster, targetGUID, finalDamage]() -> void
                        {
                            Unit* target = ObjectAccessor::GetUnit(*caster, targetGUID);
                            if (!target)
                                return;

                            caster->CastCustomSpell(target, 228596, &finalDamage, NULL, NULL, true);
                        });

                        if (dmgMod)
                            finalDamage += CalculatePct(damage, dmgMod);

                        caster->AddDelayedEvent(600, [caster, targetGUID, finalDamage]() -> void
                        {
                            Unit* target = ObjectAccessor::GetUnit(*caster, targetGUID);
                            if (!target)
                                return;

                            caster->CastCustomSpell(target, 228596, &finalDamage, NULL, NULL, true);
                        });
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectLaunchTarget += SpellEffectFn(spell_mage_flurry_SpellScript::HandleLaunchTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_mage_flurry_SpellScript();
    }
};

class CheckNetherImpactPredicate
{
    public:
        CheckNetherImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

class CheckInfernoBlastImpactPredicate
{
    public:
        CheckInfernoBlastImpactPredicate(Unit* caster, Unit* mainTarget) : _caster(caster), _mainTarget(mainTarget) {}

        bool operator()(Unit* target)
        {
            if (!_caster || !_mainTarget)
                return true;

            if (!_caster->IsValidAttackTarget(target))
                return true;

            if (!target->IsWithinLOSInMap(_caster))
                return true;

            if (!_caster->isInFront(target))
                return true;

            if (target->GetGUID() == _caster->GetGUID())
                return true;

            if (target->GetGUID() == _mainTarget->GetGUID())
                return true;

            return false;
        }

    private:
        Unit* _caster;
        Unit* _mainTarget;
};

// Conjure Refreshment - 42955, 190336
class spell_mage_conjure_refreshment : public SpellScriptLoader
{
    public:
        spell_mage_conjure_refreshment() : SpellScriptLoader("spell_mage_conjure_refreshment") { }

        class spell_mage_conjure_refreshment_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_conjure_refreshment_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    Group* group = _player->GetGroup();
                    if (group)
                    {
                        if (_player->getLevel() < 80)
                            _player->CastSpell(_player, 120056, true);  // Conjure Refreshment Table
                        else if (_player->getLevel() < 85)
                            _player->CastSpell(_player, 120055, true);  // Conjure Refreshment Table
                        else if (_player->getLevel() < 90)
                            _player->CastSpell(_player, 120054, true);  // Conjure Refreshment Table
                        else
                            _player->CastSpell(_player, 120053, true);  // Conjure Refreshment Table
                    }
                    else
                    {
                        if (_player->getLevel() < 44)
                            _player->CastSpell(_player, 92739, true);
                        else if (_player->getLevel() < 54)
                            _player->CastSpell(_player, 92799, true);
                        else if (_player->getLevel() < 64)
                            _player->CastSpell(_player, 92802, true);
                        else if (_player->getLevel() < 74)
                            _player->CastSpell(_player, 92805, true);
                        else if (_player->getLevel() < 80)
                            _player->CastSpell(_player, 74625, true);
                        else if (_player->getLevel() < 85)
                            _player->CastSpell(_player, 42956, true);
                        else if (_player->getLevel() < 90)
                            _player->CastSpell(_player, 92727, true);
                        else
                            _player->CastSpell(_player, 116130, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_conjure_refreshment_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_conjure_refreshment_SpellScript();
        }
};

// Time Warp - 80353
class spell_mage_time_warp : public SpellScriptLoader
{
    public:
        spell_mage_time_warp() : SpellScriptLoader("spell_mage_time_warp") { }

        class spell_mage_time_warp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_time_warp_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                std::list<WorldObject*> removeList;

                if (Unit* caster = GetCaster())
                {
                    for (auto itr : targets)
                    {
                        if (Unit* unit = itr->ToUnit())
                        {
                            if (unit->GetGUID() == caster->GetGUID() && unit->HasAura(207970)) // Shard of the Exodar
                                continue;

                            if (unit->HasAura(95809) || unit->HasAura(57723) || unit->HasAura(57724) || unit->HasAura(80354) || unit->HasAura(160455))
                            {
                                removeList.push_back(itr);
                                continue;
                            }
                        }
                    }
                }

                if (!removeList.empty())
                {
                    for (auto itr : removeList)
                        targets.remove(itr);
                }
            }

            void ApplyDebuff()
            {
                if (Unit* target = GetHitUnit())
                    if (!target->HasAura(207970)) // Shard of the Exodar
                        target->CastSpell(target, 80354, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_time_warp_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_time_warp_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_mage_time_warp_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_time_warp_SpellScript();
        }
};

struct auraData
{
    auraData(uint32 id, int32 duration) : m_id(id), m_duration(duration) {}
    uint32 m_id;
    int32 m_duration;
};

const uint32 icicles[5][3]
{
    {148012, 148017, 148013},
    {148013, 148018, 148014},
    {148014, 148019, 148015},
    {148015, 148020, 148016},
    {148016, 148021, 148012}
};

enum MageSpells
{
    SPELL_MAGE_SQUIRREL_FORM                     = 32813,
    SPELL_MAGE_GIRAFFE_FORM                      = 32816,
    SPELL_MAGE_SERPENT_FORM                      = 32817,
    SPELL_MAGE_DRAGONHAWK_FORM                   = 32818,
    SPELL_MAGE_WORGEN_FORM                       = 32819,
    SPELL_MAGE_SHEEP_FORM                        = 32820,

    SplittingIce                                 = 56377,
    IciclesStack                                 = 205473,
    IciclesDamage                                = 148022,
    MasteryIcicles                               = 76613
};

enum SilvermoonPolymorph
{
    NPC_AUROSALIA   = 18744,
};

// TODO: move out of here and rename - not a mage spell
class spell_mage_polymorph_cast_visual : public SpellScriptLoader
{
    public:
        spell_mage_polymorph_cast_visual() : SpellScriptLoader("spell_mage_polymorph_visual") { }

        class spell_mage_polymorph_cast_visual_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_polymorph_cast_visual_SpellScript);

            static const uint32 PolymorhForms[6];

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                // check if spell ids exist in dbc
                for (uint32 i = 0; i < 6; i++)
                    if (!sSpellMgr->GetSpellInfo(PolymorhForms[i]))
                        return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetCaster()->FindNearestCreature(NPC_AUROSALIA, 30.0f))
                    if (target->GetTypeId() == TYPEID_UNIT)
                        target->CastSpell(target, PolymorhForms[urand(0, 5)], true);
            }

            void Register() override
            {
                // add dummy effect spell handler to Polymorph visual
                OnEffectHitTarget += SpellEffectFn(spell_mage_polymorph_cast_visual_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_polymorph_cast_visual_SpellScript();
        }
};

const uint32 spell_mage_polymorph_cast_visual::spell_mage_polymorph_cast_visual_SpellScript::PolymorhForms[6] =
{
    SPELL_MAGE_SQUIRREL_FORM,
    SPELL_MAGE_GIRAFFE_FORM,
    SPELL_MAGE_SERPENT_FORM,
    SPELL_MAGE_DRAGONHAWK_FORM,
    SPELL_MAGE_WORGEN_FORM,
    SPELL_MAGE_SHEEP_FORM
};

// Greater Invisibility - 110960
class spell_mage_greater_invisibility : public SpellScriptLoader
{
    public:
        spell_mage_greater_invisibility() : SpellScriptLoader("spell_mage_greater_invisibility") { }

        class spell_mage_greater_invisibility_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_greater_invisibility_AuraScript);

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 count = 0;
                    if (Unit::AuraEffectList const* mPeriodic = caster->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE))
                    {
                        for (Unit::AuraEffectList::const_iterator iter = mPeriodic->begin(); iter != mPeriodic->end(); ++iter)
                        {
                            if (!(*iter)) // prevent crash
                                continue;
                            Aura* aura = (*iter)->GetBase();
                            aura->Remove();
                            count++;
                            if(count > 1)
                                return;
                        }
                    }
                    caster->CastSpell(GetTarget(), 113862, true, NULL, aurEff);
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Aura* aura = GetTarget()->GetAura(113862, caster->GetGUID()))
                    {
                        aura->SetDuration(3000);
                        aura->SetMaxDuration(3000);
                    }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_mage_greater_invisibility_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_greater_invisibility_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_greater_invisibility_AuraScript();
        }
};

// Invisibility - 32612
// Greater Invisibility - 110960
class spell_elem_invisibility : public SpellScriptLoader
{
    public:
        spell_elem_invisibility() : SpellScriptLoader("spell_elem_invisibility") { }

        class spell_elem_invisibility_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_elem_invisibility_AuraScript);

        public:
 
            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (target->GetOwnerGUID())
                    return;

                if (Guardian* pet = target->GetGuardianPet())
                    pet->CastSpell(pet, 96243, true);
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (target->GetOwnerGUID())
                    return;

                if (Guardian* pet = target->GetGuardianPet())
                    pet->RemoveAurasDueToSpell(96243);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_elem_invisibility_AuraScript::OnApply, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_elem_invisibility_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_INVISIBILITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_elem_invisibility_AuraScript();
        }
};

// Arcane Blast - 30451
class spell_mage_arcane_blast : public SpellScriptLoader
{
    public:
        spell_mage_arcane_blast() : SpellScriptLoader("spell_mage_arcane_blast") { }

        class spell_mage_arcane_blast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_arcane_blast_SpellScript);

            void HandleAfterHit()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                caster->CastSpell(caster, GetSpellInfo()->Effects[1]->TriggerSpell, true);
            }

            void Register() override
            {
                AfterHit += SpellHitFn(spell_mage_arcane_blast_SpellScript::HandleAfterHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_arcane_blast_SpellScript();
        }
};

// Icicle - 148023 (periodic dummy)
class spell_mage_icicle : public AuraScript
{
    PrepareAuraScript(spell_mage_icicle);

    ObjectGuid iceLanceTarget;

    bool Load()
    {
        if (Unit* caster = GetCaster())
        {
            if (iceLanceTarget = caster->GetAnyDataContainer().GetValue<ObjectGuid>("IceLanceTarget", ObjectGuid::Empty))
                return true;
        }
        return false;
    }

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = caster->GetAura(MasteryIcicles))
            {
                for (auto itr : aura->m_loadedScripts)
                {
                    if (uint32 spellId = itr->CallSpecialFunction())
                    {
                        if (AuraEffect const* icicle = caster->GetAuraEffect(spellId, EFFECT_0))
                        {
                            if (Unit* target = ObjectAccessor::GetUnit(*caster, iceLanceTarget))
                            {
                                float tickamount = icicle->GetAmount();

                                for (uint8 i = 0; i < 5; i++)
                                {
                                    if (icicles[i][0] == spellId)
                                    {
                                        caster->CastSpell(target, icicles[i][1], true);
                                        caster->CastCustomSpell(target, IciclesDamage, &tickamount, NULL, NULL, true);
                                        caster->RemoveAurasDueToSpell(spellId);
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
                GetAura()->Remove();
                return;
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_icicle::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

//Illusion glyph - 131784
class spell_mage_illusion : public SpellScriptLoader
{
    public:
        spell_mage_illusion() : SpellScriptLoader("spell_mage_illusion") { }

        class spell_mage_illusion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_illusion_SpellScript)

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    /*if(Unit* target = _player->GetSelectedUnit())
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER && target != GetCaster())
                        {
                            target->CastSpell(_player, 80396, true);
                            return;
                        }
                    }*/
                    _player->CastSpell(_player, 94632, true);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_mage_illusion_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_illusion_SpellScript();
        }
};

// Flameglow - 140468
class spell_mage_flameglow : public SpellScriptLoader
{
    public:
        spell_mage_flameglow() : SpellScriptLoader("spell_mage_flameglow") { }

        class spell_mage_flameglow_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_flameglow_AuraScript);

            uint32 absorb;
            uint32 LimitAbsorb;
            uint32 TakenDamage;

            bool Load() override
            {
                absorb = 0;
                LimitAbsorb = 0;
                TakenDamage = 0;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = -1;                
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorb = GetCaster()->GetSpellPowerDamage() * GetSpellInfo()->Effects[EFFECT_1]->BasePoints / 100;
                LimitAbsorb = GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), LimitAbsorb);
                if (absorbAmount > absorb)
                    absorbAmount = absorb;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_flameglow_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_flameglow_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_flameglow_AuraScript();
        }
};

// Glyph of Conjure Familiar - 126748
enum CreateFamiliarStone
{
    ITEM_ICY_FAMILIAR_STONE_1 = 87259,
    ITEM_FIERY_FAMILIAR_STONE_2 = 87258,
    ITEM_ARCANE_FAMILIAR_STONE_3 = 87257,
};

class spell_mage_glyph_of_conjure_familiar : public SpellScriptLoader
{
    public:
        spell_mage_glyph_of_conjure_familiar() : SpellScriptLoader("spell_mage_glyph_of_conjure_familiar") { }

        class spell_mage_glyph_of_conjure_familiar_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_glyph_of_conjure_familiar_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Player* target = GetHitPlayer())
                {
                    static const uint32 items[] = {ITEM_ICY_FAMILIAR_STONE_1, ITEM_FIERY_FAMILIAR_STONE_2, ITEM_ARCANE_FAMILIAR_STONE_3};
                    target->AddItem(items[urand(0, 2)], 1);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_glyph_of_conjure_familiar_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_CREATE_ITEM_2);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_glyph_of_conjure_familiar_SpellScript();
        }
};

// Supernova - 157980, Blast Wave - 157981, Ice Nova - 157997
class spell_mage_supernova : public SpellScriptLoader
{
    public:
        spell_mage_supernova() : SpellScriptLoader("spell_mage_supernova") { }

        class spell_mage_supernova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_supernova_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (m_scriptSpellId == 157980)
                {
                    targets.remove_if([](WorldObject* object) -> bool
                    {
                        if (auto plr = object->ToPlayer())
                            if (plr->IsMounted() && plr->IsFlying())
                                return true;

                        return false;
                    });
                }
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* target = GetHitUnit())
                {
                    if (target == GetExplTargetUnit())
                        SetHitDamage(GetHitDamage() * 2);
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mage_supernova_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_mage_supernova_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_supernova_SpellScript();
        }
};

// Incanter's Flow - 1463
class spell_mage_incanters_flow : public AuraScript
{
    PrepareAuraScript(spell_mage_incanters_flow);

    bool upd = true;
    bool direction = false;

    void OnTick(AuraEffect const* /*auraEff*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->isInCombat())
                return;

            if (caster->HasAura(116267))
            {
                if (Aura* aur = caster->GetAura(116267))
                {
                    direction = false;

                    if (aur->GetStackAmount() == 5 && upd)
                    {
                        upd = false;
                        direction = true;
                    }
                    else if (aur->GetStackAmount() == 1 && !upd)
                    {
                        upd = true;
                        direction = true;
                    }

                    if (!direction)
                        aur->ModStackAmount(upd ? 1 : -1);
                }
            }
            else if (caster->isInCombat())
            {
                caster->CastSpell(caster, 116267, true);
                upd = true;
                direction = false;
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_incanters_flow::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Erosion - 210154
class spell_mage_erosion : public SpellScriptLoader
{
    public:
        spell_mage_erosion() : SpellScriptLoader("spell_mage_erosion") { }

        class spell_mage_erosion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_erosion_AuraScript);

            bool wait = false;

            void OnTick(AuraEffect const* aurEff)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* target = GetUnitOwner())
                {
                    if (!wait && aurEff->GetTickNumber() == 3)
                    {
                        GetAura()->SetMaxDuration(11000);
                        GetAura()->SetDuration(11000);
                        wait = true;
                    }
                    else if (wait)
                    {
                        if (Aura* aura = target->GetAura(210134, caster->GetGUID()))
                            aura->ModStackAmount(-1);
                    }
                }
            }

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                wait = false;
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_erosion_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectApply += AuraEffectApplyFn(spell_mage_erosion_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_erosion_AuraScript();
        }
};

// Displacement - 195676, 212801
class spell_mage_displacement : public SpellScriptLoader
{
    public:
        spell_mage_displacement() : SpellScriptLoader("spell_mage_displacement") { }

        class spell_mage_displacement_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_displacement_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<AreaTrigger*> list;
                    caster->GetAreaObjectList(list, 212799);
                    if (!list.empty())
                    {
                        for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
                        {
                            if (AreaTrigger* areaObj = (*itr))
                            {
                                if (areaObj->GetDistance(caster) <= 60.f)
                                {
                                    caster->NearTeleportTo(areaObj->GetPositionX(), areaObj->GetPositionY(), areaObj->GetPositionZ(), areaObj->GetOrientation(), true);
                                    areaObj->Despawn();
                                    caster->RemoveAurasDueToSpell(212799);
                                    return SPELL_CAST_OK;
                                }
                            }
                        }
                    }
                }
                return SPELL_FAILED_OUT_OF_RANGE;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_mage_displacement_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_displacement_SpellScript();
        }
};

// Combustion - 190319
class spell_mage_combustion : public SpellScriptLoader
{
    public:
        spell_mage_combustion() : SpellScriptLoader("spell_mage_combustion") { }

        class spell_mage_combustion_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_combustion_AuraScript);

            int32 lastData = 0;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (Player* _player = caster->ToPlayer())
                    lastData = amount = CalculatePct(_player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_CRIT_SPELL), 50);
            }

            void OnTick(AuraEffect const* aurEff)
            {
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (Player* _player = caster->ToPlayer())
                {
                    uint32 crit = _player->GetUInt32Value(PLAYER_FIELD_COMBAT_RATINGS + CR_CRIT_SPELL);
                    int32 critData = CalculatePct(crit, 50); // 50%
                    if (lastData != critData)
                    {
                        lastData = critData;
                        if (Aura* aura = const_cast<Aura*>(aurEff->GetBase()))
                            if (AuraEffect* aurEff1 = aura->GetEffect(EFFECT_1))
                                aurEff1->ChangeAmount(lastData);
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_combustion_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_RATING);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_combustion_AuraScript::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_combustion_AuraScript();
        }
};

// Living Bomb - 217694, 244813
class spell_mage_living_bomb : public AuraScript
{
    PrepareAuraScript(spell_mage_living_bomb);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetUnitOwner();
        if (!caster || !target)
            return;

        float bp0 = aurEff->GetAmount();
        caster->CastCustomSpell(target, 44461, &bp0, nullptr, nullptr, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_mage_living_bomb::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Living Bomb - 44461
class spell_mage_living_bomb_damage : public SpellScript
{
    PrepareSpellScript(spell_mage_living_bomb_damage);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        SpellValue const* spellValue = GetSpellValue();
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        Unit* explunit = GetExplTargetUnit();
        if (!caster || !spellValue || !target || !explunit)
            return;

        if (spellValue->EffectBasePoints[0] && target != explunit)
        {
            float bp0 = 0;
            caster->CastCustomSpell(target, 244813, nullptr, &bp0, nullptr, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_living_bomb_damage::HandleEffectHit, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Shimmer - 212653
class spell_mage_shimmer : public SpellScriptLoader
{
    public:
        spell_mage_shimmer() : SpellScriptLoader("spell_mage_shimmer") { }

        class spell_mage_shimmer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_shimmer_SpellScript);

            SpellCastResult CheckCast()
            {
                if(Unit* caster = GetCaster())
                {
                    if (caster->HasUnitState(UNIT_STATE_CONFUSED))
                        return SPELL_FAILED_CONFUSED;
                    if (caster->HasUnitState(UNIT_STATE_FLEEING))
                        return SPELL_FAILED_FLEEING;
                    if (caster->HasUnitState(UNIT_STATE_STUNNED))
                        return SPELL_FAILED_STUNNED;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_mage_shimmer_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_shimmer_SpellScript();
        }
};

// Ice Lance - 30455
class spell_mage_ice_lance_main : public SpellScript
{
    PrepareSpellScript(spell_mage_ice_lance_main);

    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
        {
            Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();

            if (caster->HasAura(MasteryIcicles) && !caster->HasSpell(199786)) // Mastery: Icicles, Glacial Spike
            {
                if (Unit* target = GetOriginalTarget())
                {
                    cont.Set("IceLanceTarget", target->GetGUID());
                    caster->CastSpell(caster, 148023, true);
                }
            }

            if (Aura* aura = caster->GetAura(44544))
            {
                if (aura->ModStackAmount(-1))
                    caster->RemoveAura(aura);

                if (Aura* Alert = caster->GetAura(126084))
                    Alert->ModStackAmount(-1);

                cont.Set("isFrozenTarget", 0);
            }

            if (caster->HasAura(SplittingIce))
            {
                if (Spell* spell = GetSpell())
                {
                    for (auto itr : *spell->GetUniqueTargetInfo())
                    {
                        if (itr->HasMask(TARGET_INFO_IS_JUMP_TARGET))
                        {
                            cont.Set("SplittingIceTarget", itr->targetGUID);
                            break;
                        }
                    }
                }
            }
        }
    }

    void HandleFinishCast()
    {
        if (Unit* caster = GetCaster())
        {
            Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();

            cont.Remove("isFrozenTarget");
            cont.Remove("SplittingIceTarget");
            cont.Remove("IceLanceTarget");
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_mage_ice_lance_main::HandleOnCast);
        OnFinishCast += SpellCastFn(spell_mage_ice_lance_main::HandleFinishCast);
    }
};


// Ice Lance - 228598
class spell_mage_ice_lance : public SpellScript
{
    PrepareSpellScript(spell_mage_ice_lance);

    void HandleEffectHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();
                bool isFrozen = (cont.Exist("isFrozenTarget") || target->HasAuraState(AURA_STATE_FROZEN, GetSpellInfo(), caster) || target->HasAura(228358, caster->GetGUID()));
                float dmgMod = isFrozen ? 3.f : 1.f;

                if (isFrozen)
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(238056, EFFECT_0)) // Obsidian Lance
                        AddPct(dmgMod, aurEff->GetAmount());

                    if (caster->HasSpell(155149)) // Thermal Void
                    {
                        if (Aura* aura = caster->GetAura(12472))
                            aura->SetDuration(aura->GetDuration() + 1000);
                    }
                }

                if (ObjectGuid const& jumpTargetGUID = cont.GetValue<ObjectGuid>("SplittingIceTarget", ObjectGuid::Empty))
                {
                    if (target->GetGUID() == jumpTargetGUID)
                    {
                        if (SpellInfo const* spellinfo = sSpellMgr->GetSpellInfo(SplittingIce))
                            dmgMod = CalculatePct(dmgMod, spellinfo->Effects[EFFECT_1]->BasePoints);
                    }
                }

                SetHitDamage(GetHitDamage() * dmgMod);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_mage_ice_lance::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Glacial Spike - 199786
class spell_mage_glacial_spike : public SpellScriptLoader
{
    public:
        spell_mage_glacial_spike() : SpellScriptLoader("spell_mage_glacial_spike") { }

        class spell_mage_glacial_spike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_glacial_spike_SpellScript)

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    float damage = 0.f;
                    for (auto const spellID : {148012, 148013, 148014, 148015, 148016})
                    {
                        if (Aura* icicle = caster->GetAura(spellID))
                        {
                            if (icicle->GetEffect(EFFECT_0))
                                damage += icicle->GetEffect(EFFECT_0)->GetAmount();
                            icicle->Remove();
                        }
                    }
                    for (auto const visual : {148017, 148018, 148019, 148020, 148021})
                        if (Aura* icicle = caster->GetAura(visual))
                            icicle->Remove();

                    caster->CastCustomSpell(caster, 214325, &damage, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_mage_glacial_spike_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_glacial_spike_SpellScript();
        }
};

// Glacial Spike - 228600
class spell_mage_glacial_spike_damage : public SpellScriptLoader
{
    public:
        spell_mage_glacial_spike_damage() : SpellScriptLoader("spell_mage_glacial_spike_damage") { }

        class spell_mage_glacial_spike_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_mage_glacial_spike_damage_SpellScript)

            void HandleEffectHit(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 damage = GetHitDamage();
                    if (Aura* aura = caster->GetAura(214325))
                    {
                        if (aura->GetEffect(EFFECT_0))
                            damage += aura->GetEffect(EFFECT_0)->GetAmount();
                        aura->Remove();
                    }
                    SetHitDamage(damage);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_mage_glacial_spike_damage_SpellScript::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_mage_glacial_spike_damage_SpellScript();
        }
};

// Ice barrier - 11426 , Prismatic barrier - 235450
class spell_mage_barriers : public SpellScriptLoader
{
    public:
        spell_mage_barriers() : SpellScriptLoader("spell_mage_barriers") { }

        class spell_mage_barriers_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_barriers_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(195354, EFFECT_0))
                        amount += CalculatePct(amount, aurEff->GetAmount());
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(210716, EFFECT_0))
                        amount += CalculatePct(amount, aurEff->GetAmount());
                }
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect* aurEff = caster->GetAuraEffect(235463, EFFECT_0)) // Mana Shield
                    {
                        float bp0 = CalculatePct(absorbAmount, aurEff->GetAmount());
                        caster->CastCustomSpell(caster, 235470, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_barriers_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_mage_barriers_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_barriers_AuraScript();
        }
};

// Dampened Magic (PvP talent) - 236788
class spell_mage_dampened_magic : public SpellScriptLoader
{
    public:
        spell_mage_dampened_magic() : SpellScriptLoader("spell_mage_dampened_magic") { }

        class spell_mage_dampened_magic_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_dampened_magic_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                // Set absorbtion amount to unlimited
                amount = -1;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                if (dmgInfo.GetDamageType() == DOT)
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_dampened_magic_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_mage_dampened_magic_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_dampened_magic_AuraScript();
        }
};

// Temporal Shield (Honor Talent) - 198111
class spell_mage_temporal_shield : public SpellScriptLoader
{
    public:
        spell_mage_temporal_shield() : SpellScriptLoader("spell_mage_temporal_shield") { }

        class spell_mage_temporal_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_temporal_shield_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_EXPIRE && removeMode != AURA_REMOVE_BY_CANCEL)
                    return;

                if (Unit* owner = GetUnitOwner())
                {
                    if (float bp0 = aurEff->GetAmount())
                        owner->CastCustomSpell(owner, 198116, &bp0, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_temporal_shield_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_temporal_shield_AuraScript();
        }
};

// Touch of the Magi - 210824
class spell_mage_touch_of_the_magi : public SpellScriptLoader
{
    public:
        spell_mage_touch_of_the_magi() : SpellScriptLoader("spell_mage_touch_of_the_magi") { }

        class spell_mage_touch_of_the_magi_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_touch_of_the_magi_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    float bp0 = aurEff->GetAmount();
                    if (Unit* target = GetUnitOwner())
                        caster->CastCustomSpell(target, 210833, &bp0, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_touch_of_the_magi_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_touch_of_the_magi_AuraScript();
        }
};

// Aegwynn's Ascendance - 210847
class spell_mage_aegwynns_ascendance : public SpellScriptLoader
{
    public:
        spell_mage_aegwynns_ascendance() : SpellScriptLoader("spell_mage_aegwynns_ascendance") { }

        class spell_mage_aegwynns_ascendance_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_aegwynns_ascendance_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 perc = 25;
                    if (AuraEffect* aurEff2 = caster->GetAuraEffect(187680, EFFECT_0)) // Aegwynn's Ascendance
                        perc = aurEff2->GetAmount();
                    float bp0 = CalculatePct(aurEff->GetAmount(), perc);
                    caster->CastCustomSpell(caster, 187677, &bp0, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_mage_aegwynns_ascendance_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_aegwynns_ascendance_AuraScript();
        }
};

// Belo'vir's Final Stand - 207283
class spell_mage_belovirs_final_stand : public SpellScriptLoader
{
    public:
        spell_mage_belovirs_final_stand() : SpellScriptLoader("spell_mage_belovirs_final_stand") { }

        class spell_mage_belovirs_final_stand_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_belovirs_final_stand_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff2 = caster->GetAuraEffect(207277, EFFECT_0)) // Belo'vir's Final Stand
                        amount = caster->CountPctFromMaxHealth(aurEff2->GetAmount());
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_mage_belovirs_final_stand_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_belovirs_final_stand_AuraScript();
        }
};

// 208141 - Ray of Frost
class spell_mage_ray_of_frost : public SpellScriptLoader
{
    public:
        spell_mage_ray_of_frost() : SpellScriptLoader("spell_mage_ray_of_frost") { }

        class spell_mage_ray_of_frostAuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_ray_of_frostAuraScript);

            void OnStackChange(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (!caster->HasAura(208166))
                        caster->InterruptSpell(CURRENT_CHANNELED_SPELL, true, true);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_mage_ray_of_frostAuraScript::OnStackChange, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_ray_of_frostAuraScript();
        }
};

// Arcane Familiar - 210126
class spell_mage_arcane_familiar : public AuraScript
{
    PrepareAuraScript(spell_mage_arcane_familiar);

    enum MyEnum
    {
        Timer = 1000
    };

    uint8 castCount = 0;
    uint32 castDelay = 0;
    ObjectGuid targetGUID;

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* target = eventInfo.GetProcTarget())
        {
            if (target->GetGUID() == GetCaster()->GetGUID())
                return;

            if (Aura* aura = GetAura())
            {
                targetGUID = target->GetGUID();
                aura->SetAuraAttribute(AURA_ATTR_IS_NOT_ACTIVE);
                castCount = 2;
            }
        }
    }

    void OnUpdate(uint32 diff)
    {
        if (!castCount)
            return;

        castDelay += diff;

        if (castDelay >= Timer)
        {
            if (Unit* caster = GetUnitOwner())
            {
                if (Unit* target = ObjectAccessor::GetUnit(*caster, targetGUID))
                    caster->CastSpell(target, 225119, TriggerCastFlags(TRIGGERED_FULL_MASK &~ TRIGGERED_IGNORE_LOS));
            }

            castCount--;

            if (!castCount)
            {
                castDelay = 0;
                targetGUID = ObjectGuid::Empty;

                if (Aura* aura = GetAura())
                    aura->SetAuraAttribute(AURA_ATTR_IS_NOT_ACTIVE, false);

                return;
            }

            castDelay -= Timer;
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_arcane_familiar::OnProc, EFFECT_0, SPELL_AURA_MOD_MAX_MANA);
        OnAuraUpdate += AuraUpdateFn(spell_mage_arcane_familiar::OnUpdate);
    }
};

// Mastery: Icicles - 76613
class spell_mage_mastery_icicles: public AuraScript
{
    PrepareAuraScript(spell_mage_mastery_icicles);

    std::vector<uint32> getIcicles;

    uint32 CallSpecialFunction(uint32 Num) override
    {
        if (!getIcicles.empty())
        {
            if (!Num)
                return getIcicles.front();
            
            getIcicles.erase(getIcicles.begin());
        }
        return 0;
    }

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (DamageInfo* dmgInfo = eventInfo.GetDamageInfo())
            {
                if (float dmg = CalculatePct(dmgInfo->GetDamage() + dmgInfo->GetAbsorb(), aurEff->GetAmount()))
                {
                    float addPctDamage = 0.f;
                    uint8 procAmount = 1;

                    if (caster->HasAura(214664) && roll_chance_i(10)) // Ice Nine
                        procAmount++;

                    if (AuraEffect const* aurEffPct = caster->GetAuraEffect(195615, EFFECT_0)) // Black Ice
                        addPctDamage = aurEffPct->GetAmount();

                    for (uint8 i = 0; i < procAmount; i++)
                    {
                        if (addPctDamage && roll_chance_i(20))
                            AddPct(dmg, addPctDamage);

                        if (getIcicles.empty())
                        {
                            Cast(caster, icicles[0][0], dmg);
                        }
                        else if (getIcicles.size() == 5)
                        {
                            if (Unit* target = eventInfo.GetProcTarget())
                            {
                                uint32 spellId = getIcicles.front();

                                if (AuraEffect const* eff = caster->GetAuraEffect(spellId, EFFECT_0))
                                {
                                    float bp = eff->GetAmount();
                                    caster->CastCustomSpell(target, IciclesDamage, &bp, NULL, NULL, true);

                                    for (uint8 i = 0; i < 5; i++)
                                    {
                                        if (icicles[i][0] == spellId)
                                        {
                                            caster->CastSpell(target, icicles[i][1], true);
                                            break;
                                        }
                                    }
                                    caster->RemoveAurasDueToSpell(spellId);
                                }
                                Cast(caster, spellId, dmg);
                            }
                        }
                        else
                        {
                            uint32 spellId = getIcicles.back();

                            for (uint8 i = 0; i < 5; i++)
                            {
                                if (icicles[i][0] == spellId)
                                {
                                    Cast(caster, icicles[i][2], dmg);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void Cast(Unit* caster, uint32 const& spellId, float& damage)
    {
        caster->CastSpell(caster, IciclesStack, true);
        caster->CastCustomSpell(caster, spellId, &damage, NULL, NULL, true);
        getIcicles.push_back(spellId);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_mastery_icicles::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Arcane Missiles - 79683
class spell_mage_arcane_missiles : public SpellScriptLoader
{
    public:
        spell_mage_arcane_missiles() : SpellScriptLoader("spell_mage_arcane_missiles") { }

        class spell_mage_arcane_missiles_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_mage_arcane_missiles_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                if (Aura* aur = caster->GetAura(GetSpellInfo()->Id))
                {
                    switch (aur->GetStackAmount())
                    {
                        case 1:
                            caster->RemoveAurasDueToSpell(79808);
                            caster->RemoveAurasDueToSpell(170572);
                            caster->CastSpell(caster, 170571, true);
                            break;
                        case 2:
                            caster->RemoveAurasDueToSpell(79808);
                            caster->RemoveAurasDueToSpell(170571);
                            caster->RemoveAurasDueToSpell(170572);
                            caster->CastSpell(caster, 79808, true);
                            break;
                        case 3:
                            caster->RemoveAurasDueToSpell(79808);
                            caster->CastSpell(caster, 170572, true);
                            break;
                    }
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                Aura* aur = caster->GetAura(GetSpellInfo()->Id);
                if (!aur)
                {
                    caster->RemoveAurasDueToSpell(79808);
                    caster->RemoveAurasDueToSpell(170571);
                    caster->RemoveAurasDueToSpell(170572);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectApplyFn(spell_mage_arcane_missiles_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectApply += AuraEffectApplyFn(spell_mage_arcane_missiles_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_mage_arcane_missiles_AuraScript();
        }
};

// 238126
class spell_mage_time_and_space : public AuraScript
{
    PrepareAuraScript(spell_mage_time_and_space);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            std::list<AreaTrigger*> list;
            caster->GetAreaObjectList(list, 240692);
            if (!list.empty())
            {
                for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
                {
                    if (AreaTrigger* areaObj = (*itr))
                    {
                        caster->CastSpell(areaObj->GetPositionX(), areaObj->GetPositionY(), areaObj->GetPositionZ(), 240689, true);
                        areaObj->Despawn();
                    }
                }
            }
            caster->CastSpell(caster, 240692, true); // AT
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_mage_time_and_space::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 211918 - Immolation
class spell_mage_immolation : public AuraScript
{
    PrepareAuraScript(spell_mage_immolation);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, 194462, true);      
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            caster->RemoveAurasDueToSpell(194462);
            caster->RemoveAurasDueToSpell(194461);
            caster->RemoveAurasDueToSpell(226754);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectApplyFn(spell_mage_immolation::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_mage_immolation::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 194462 - Highblade's Will
class spell_mage_highblades_will : public AuraScript
{
    PrepareAuraScript(spell_mage_highblades_will);

    void OnTick(AuraEffect const* aurEff)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(194461) || caster->HasAura(226754))
            {
                caster->CastSpell(caster, 194461, true);
            }
            else if (caster->HasAura(194461) && !caster->HasAura(226754))
            {
                caster->RemoveAurasDueToSpell(194461);
                caster->CastSpell(caster, 226754, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_mage_highblades_will::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 194466 - Phoenix's Flames
class spell_mage_phoenixs_flames : public SpellScript
{
    PrepareSpellScript(spell_mage_phoenixs_flames);

    void HandleCast()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(194461) && !caster->HasAura(226754))
            {
                caster->RemoveAurasDueToSpell(194461);
            }
            else if (!caster->HasAura(194461) && caster->HasAura(226754))
            {
                caster->RemoveAurasDueToSpell(226754);
                caster->CastSpell(caster, 194461, true);
            }
            else if (caster->HasAura(194461) && caster->HasAura(226754))
            {
                caster->RemoveAurasDueToSpell(194461);
            }
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_mage_phoenixs_flames::HandleCast);
    }
};

// 84714 - Frozen orb
/*class areatrigger_at_mage_frozen_orb : public AreaTriggerScript
{
public:
	areatrigger_at_mage_frozen_orb() : AreaTriggerScript("areatrigger_at_mage_frozen_orb") {}

	struct areatrigger_at_mage_frozen_orbAI : AreaTriggerAI
	{
		areatrigger_at_mage_frozen_orbAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

		void OnRemove() override
		{
			float posX = at->GetPositionX();
			float posY = at->GetPositionY();
			float posZ = at->GetPositionZ();

			if (Unit* caster = at->GetCaster())
			{
				if (caster->HasAura(235227)) // Ice Time Legion legendary shoulders
				{
					caster->CastSpell(posX, posY, posZ, 235235, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
				}
			}
		}
	};

	AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
	{
		return new areatrigger_at_mage_frozen_orbAI(areatrigger);
	}
};*/

void AddSC_mage_spell_scripts()
{
    new spell_mage_cauterize();
    new spell_mage_conjure_refreshment();
    new spell_mage_time_warp();
    new spell_mage_polymorph_cast_visual();
    new spell_mage_greater_invisibility();
    new spell_elem_invisibility();
    new spell_mage_arcane_blast();
    new spell_mage_illusion();
    new spell_mage_flameglow();
    new spell_mage_glyph_of_conjure_familiar();
    new spell_mage_supernova();
    RegisterAuraScript(spell_mage_incanters_flow);
    new spell_mage_erosion();
    new spell_mage_displacement();
    new spell_mage_combustion();
    RegisterAuraScript(spell_mage_living_bomb);
    RegisterSpellScript(spell_mage_living_bomb_damage);
    new spell_mage_shimmer();
    new spell_mage_glacial_spike();
    new spell_mage_glacial_spike_damage();
    new spell_mage_barriers();
    new spell_mage_dampened_magic();
    new spell_mage_temporal_shield();
    new spell_mage_touch_of_the_magi();
    new spell_mage_aegwynns_ascendance();
    new spell_mage_belovirs_final_stand();
    new spell_mage_ray_of_frost();
    new spell_mage_flurry();
    new spell_mage_arcane_missiles();
    RegisterSpellScript(spell_mage_arcane_barrage);
    RegisterSpellScript(spell_mage_ice_lance);
    RegisterSpellScript(spell_mage_ice_lance_main);
    RegisterAuraScript(spell_mage_arcane_familiar);
    RegisterAuraScript(spell_mage_mastery_icicles); 
    RegisterAuraScript(spell_mage_icicle);
    RegisterAuraScript(spell_mage_time_and_space);
    RegisterAuraScript(spell_mage_immolation);
    RegisterAuraScript(spell_mage_highblades_will);
    RegisterSpellScript(spell_mage_phoenixs_flames);
	//new areatrigger_at_mage_frozen_orb();
}
