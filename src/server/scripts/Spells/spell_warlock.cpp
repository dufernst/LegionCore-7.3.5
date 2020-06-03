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
 * Scripts for spells with SPELLFAMILY_WARLOCK and SPELLFAMILY_GENERIC spells used by warlock players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_warl_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "ObjectVisitors.hpp"
#include "AreaTrigger.h"

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h" 
#include "Cell.h"
#include "CellImpl.h"
#include "Packets/SpellPackets.h"

// Burning Rush - 111400
class spell_warl_burning_rush : public SpellScriptLoader
{
    public:
        spell_warl_burning_rush() : SpellScriptLoader("spell_warl_burning_rush") { }

        class spell_warl_burning_rush_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_burning_rush_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (!caster->HealthAbovePct(9))
                        GetAura()->Remove();
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_burning_rush_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_burning_rush_AuraScript();
        }
};

// Banish - 710
class spell_warl_banish : public SpellScriptLoader
{
public:
    spell_warl_banish() : SpellScriptLoader("spell_warl_banish") { }

    class spell_warl_banish_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_warl_banish_SpellScript);

        bool Load() override
        {
            _removed = false;
            return true;
        }

        void HandleBanish()
        {
            // Casting Banish on a banished target will cancel the effect
            // Check if the target already has Banish, if so, do nothing.

            if (Unit* target = GetHitUnit())
            {
                if (target->GetAuraEffect(SPELL_AURA_SCHOOL_IMMUNITY, SPELLFAMILY_WARLOCK, 0, 0x08000000, 0))
                {
                    // No need to remove old aura since its removed due to not stack by current Banish aura
                    PreventHitDefaultEffect(EFFECT_0);
                    PreventHitDefaultEffect(EFFECT_1);
                    PreventHitDefaultEffect(EFFECT_2);
                    _removed = true;
                }
            }
        }

        void RemoveAura()
        {
            if (_removed)
                PreventHitAura();
        }

        void Register() override
        {
            BeforeHit += SpellHitFn(spell_warl_banish_SpellScript::HandleBanish);
            AfterHit += SpellHitFn(spell_warl_banish_SpellScript::RemoveAura);
        }

        bool _removed;
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_warl_banish_SpellScript();
    }
};

// Demonic Circle (summon) - 48018
class spell_warl_demonic_circle_summon : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_summon() : SpellScriptLoader("spell_warl_demonic_circle_summon") { }

        class spell_warl_demonic_circle_summon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_circle_summon_SpellScript);

            SpellCastResult CheckElevation()
            {
                auto caster = GetCaster();
                if (!caster)
                    return SPELL_FAILED_DONT_REPORT;

                switch (caster->GetMapId())
                {
                case 617:
                    if (caster->GetPositionZ() >= 11.40f)
                        return SPELL_FAILED_NOT_HERE;
                    break;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_demonic_circle_summon_SpellScript::CheckElevation);
            }
        };

        class spell_warl_demonic_circle_summon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_summon_AuraScript);

            void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (Unit* target = GetTarget())
                {
                    if (!(mode & AURA_EFFECT_HANDLE_REAPPLY))
                        target->RemoveGameObject(GetId(), true);

                    target->RemoveAurasDueToSpell(62388);
                }
            }

            void HandleDummyTick(AuraEffect const* /*aurEff*/)
            {
                if (Unit* target = GetTarget())
                {
                    if (GameObject* circle = target->GetGameObject(GetId()))
                    {
                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(48020);

                        if (target->IsWithinDist(circle, spellInfo->GetMaxRange(true)))
                        {
                            target->AddAura(62388, target);
                        }
                        else
                        {
                            target->RemoveAurasDueToSpell(62388);
                        }
                    }
                }
            }

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = GetTarget())
                    target->AddAura(62388, target);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectApplyFn(spell_warl_demonic_circle_summon_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demonic_circle_summon_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
                OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_summon_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_demonic_circle_summon_AuraScript();
        }
};

// Demonic Circle (teleport) - 48020
class spell_warl_demonic_circle_teleport : public SpellScriptLoader
{
    public:
        spell_warl_demonic_circle_teleport() : SpellScriptLoader("spell_warl_demonic_circle_teleport") { }

        class spell_warl_demonic_circle_teleport_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_circle_teleport_AuraScript);

            void HandleTeleport(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* player = GetTarget()->ToPlayer())
                {
                    if (GameObject* circle = player->GetGameObjectbyId(191083))
                    {
                        player->NearTeleportTo(circle->GetPositionX(), circle->GetPositionY(), circle->GetPositionZ(), circle->GetOrientation());
                        player->CastSpell(player, 104163, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_circle_teleport_AuraScript::HandleTeleport, EFFECT_0, SPELL_AURA_MECHANIC_IMMUNITY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_demonic_circle_teleport_AuraScript();
        }
};

// Unstable Affliction - 233490, 233496, 233497, 233498, 233499
class spell_warl_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_warl_unstable_affliction() : SpellScriptLoader("spell_warl_unstable_affliction") {}

        class spell_warl_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_unstable_affliction_AuraScript);

            void HandleDispel(DispelInfo* dispelInfo)
            {
                Unit* caster = GetCaster();
                Unit* dispeller = dispelInfo->GetDispeller();
                if (!caster || !dispeller)
                    return;

                if (AuraEffect const* aurEff = GetEffect(EFFECT_0))
                {
                    int32 perc = 400;
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(30108);
                    if (spellInfo)
                        perc = spellInfo->Effects[EFFECT_0]->BasePoints;

                    float damage = CalculatePct(aurEff->GetAmount(), perc);

                    ObjectGuid targetGUID = dispeller->GetGUID();
                    caster->AddDelayedEvent(50, [caster, targetGUID, damage]() -> void
                    {
                        Unit* target = ObjectAccessor::GetUnit(*caster, targetGUID);
                        if (!target)
                            return;

                        caster->CastCustomSpell(target, 196364, &damage, NULL, NULL, true);
                    });
                }
            }

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (!caster || !target)
                    return;

                if (AuraEffect* eff = caster->GetAuraEffect(208821, EFFECT_0))
                    eff->SetAmount(1);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (!caster || !target)
                    return;

                if (AuraEffect* eff = caster->GetAuraEffect(208821, EFFECT_0))
                    eff->SetAmount(1);

                if (Aura* aura = caster->GetAura(231791))
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_DEATH)
                    {
                        for (auto itr : aura->m_loadedScripts)
                        {
                            if (itr->CallSpecialFunction(target->GetGUID()))
                                break;
                        }
                    }
                }

                AuraEffect* aurEff = caster->GetAuraEffect(199257, EFFECT_0); // Fatal Echoes
                if (!aurEff || !roll_chance_i(aurEff->GetAmount()))
                    return;

                caster->CastSpell(target, GetSpellInfo()->Id, true);
            }

            void Register() override
            {
                OnDispel += AuraDispelFn(spell_warl_unstable_affliction_AuraScript::HandleDispel);
                AfterEffectApply += AuraEffectApplyFn(spell_warl_unstable_affliction_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_CHANGE_AMOUNT_MASK);
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_unstable_affliction_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_unstable_affliction_AuraScript();
        }
};

// Unstable Affliction (Rank 2) - 231791
class spell_warl_unstable_affliction_R2 : public AuraScript
{
    PrepareAuraScript(spell_warl_unstable_affliction_R2);

    std::set<ObjectGuid> targetList;
    int32 timer = 0;

    uint32 CallSpecialFunction(ObjectGuid const& GUID) override
    {
        timer = 200;
        targetList.insert(GUID);
        return 1;
    }

    void OnUpdate(uint32 diff)
    {
        if (!timer)
            return;

        timer -= diff;

        if (timer <= 0)
        {
            timer = 0;

            if (Unit* caster = GetUnitOwner())
            {
                for (size_t i = 0; i < targetList.size(); i++)
                    caster->CastSpell(caster, 31117, true);

                targetList.clear();
            }
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_warl_unstable_affliction_R2::OnUpdate);
    }
};

// 243046 - Searing Bolts
class spell_warl_searing_bolts : public AuraScript
{
    PrepareAuraScript(spell_warl_searing_bolts);

    void OnTick(AuraEffect const* /*aurEff*/)
    {
        if (Unit* sum = GetUnitOwner())
        {
            if (Unit* warlock = sum->GetOwner())
            {
                if (Unit* target = sum->GetTargetUnit())
                    sum->CastSpell(target, 243050, true, nullptr, nullptr, warlock->GetGUID());
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_searing_bolts::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Create Healthstone - 23517
class spell_warl_create_healthstone : public SpellScript
{
    PrepareSpellScript(spell_warl_create_healthstone);

    void FilterTargets(WorldObject*& target)
    {
        if (!target)
            return;

        if (Player* plr = target->ToPlayer())
        {
            if (Item* item = plr->GetItemByEntry(5512))
            {
                item->SetSpellCharges(1, -3);
                target = nullptr;
            }
        }
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_warl_create_healthstone::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER);
    }
};

// Streten's Insanity - 208821
class spell_warl_streten_insanity : public SpellScriptLoader
{
    public:
    spell_warl_streten_insanity() : SpellScriptLoader("spell_warl_streten_insanity") {}

    class spell_warl_streten_insanity_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_streten_insanity_AuraScript);

        void OnUpdate(uint32 diff, AuraEffect* aurEff)
        {
            if (aurEff->GetAmount())
            {
                if (Unit* owner = GetUnitOwner())
                {
                    std::vector<uint32> auraId = {233490, 233496, 233497, 233498, 233499};
                    uint8 targetCount = owner->TargetsWhoHasMyAuras(auraId);

                    if (Aura* aura = owner->GetAura(208822))
                    {
                        if (targetCount)
                        {
                            aura->SetStackAmount(targetCount);
                        }
                        else
                            owner->RemoveAurasDueToSpell(208822);
                    }
                    else
                    {
                        if (targetCount)
                        {
                            owner->AddAura(208822, owner, nullptr, targetCount);
                        }
                    }
                }
                aurEff->SetAmount(0);
            }
        }

        void Register() override
        {
            OnEffectUpdate += AuraEffectUpdateFn(spell_warl_streten_insanity_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_streten_insanity_AuraScript();
    }
};

// Seed of Corruption - 27243
class spell_warl_seed_of_corruption_dota : public SpellScriptLoader
{
    public:
        spell_warl_seed_of_corruption_dota() : SpellScriptLoader("spell_warl_seed_of_corruption_dota") { }

        class spell_warl_seed_of_corruption_dota_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_seed_of_corruption_dota_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 perc = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster);
                    amount = CalculatePct(caster->GetSpellPowerDamage(GetSpellInfo()->GetSchoolMask()), perc);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_seed_of_corruption_dota_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_seed_of_corruption_dota_AuraScript();
        }
};

// 113942 - Demonic Gateway Debuf
class spell_warl_demonic_gateway : public SpellScriptLoader
{
    public:
        spell_warl_demonic_gateway() : SpellScriptLoader("spell_warl_demonic_gateway") { }

        class spell_warl_demonic_gateway_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_gateway_AuraScript);

            void CalculateMaxDuration(int32 & duration)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* owner = ObjectAccessor::GetUnit(*caster, GetCasterGUID()))
                        if (owner->HasAura(248855)) // Gateway Mastery (Honor Talent)
                            duration -= 15000;

                    if (caster->HasAura(217519)) // Pillars of the Dark Portal
                    {
                        if (!caster->HasAura(217551))
                        {
                            caster->CastSpell(caster, 217551, true);
                            duration = 0;
                        }
                        else
                            caster->RemoveAurasDueToSpell(217551);
                    }
                }
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_warl_demonic_gateway_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_demonic_gateway_AuraScript();
        }
};

// Demonic Gateway - 111771
class spell_warl_demonic_gateway_cast : public SpellScriptLoader
{
    public:
        spell_warl_demonic_gateway_cast() : SpellScriptLoader("spell_warl_demonic_gateway_cast") { }

        class spell_warl_demonic_gateway_cast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_gateway_cast_SpellScript);

            SpellCastResult CheckCast()
            {
                if(Unit* caster = GetCaster())
                {
                    Position const* pos = GetExplTargetDest();
                    float delta_z = fabs(pos->GetPositionZ()) - fabs(caster->GetPositionZ());
                    if(delta_z > 2.7f || delta_z < -2.7f)
                        return SPELL_FAILED_NOT_HERE;

                    if (auto caster = GetCaster())
                    {
                        if (caster->GetMapId() == 529)
                        {
                            switch (caster->GetCurrentAreaID())
                            {
                            case 3424:
                                if ((pos->GetPositionX() <= 1199.0f && pos->GetPositionY() <= 1199.0f && pos->GetPositionZ() >= -51.0f) || (pos->GetPositionZ() >= 13.9f))
                                    return SPELL_FAILED_NOT_HERE;
                                break;
                            case 3422:
                                if ((pos->GetPositionZ() >= 18.8f))
                                    return SPELL_FAILED_NOT_HERE;
                                break;
                            }
                        } 
                    }
                }
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_demonic_gateway_cast_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_demonic_gateway_cast_SpellScript();
        }
};

// 113896, 120729 - Demonic Gateway
class spell_warl_demonic_gateway_duration : public SpellScriptLoader
{
    public:
        spell_warl_demonic_gateway_duration() : SpellScriptLoader("spell_warl_demonic_gateway_duration") { }

        class spell_warl_demonic_gateway_duration_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demonic_gateway_duration_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetCaster())
                {
                    if(Position const* pos = GetExplTargetDest())
                    {
                        float dist = caster->GetExactDist2d(pos->GetPositionX(), pos->GetPositionY());
                        int32 duration = int32((dist / 20.0f) * 1000.0f);
                        SetDuration(duration);
                    }
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_warl_demonic_gateway_duration_AuraScript::OnApply, EFFECT_0, SPELL_AURA_SCREEN_EFFECT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_demonic_gateway_duration_AuraScript();
        }
};

// Demonic Gateway - 113902
class spell_warl_demonic_gateway_at : public SpellScriptLoader
{
    public:
        spell_warl_demonic_gateway_at() : SpellScriptLoader("spell_warl_demonic_gateway_at") { }

        class spell_warl_demonic_gateway_at_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonic_gateway_at_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    float distanceG = 0.0f;
                    float distanceP = 0.0f;
                    ObjectGuid uGateG;
                    ObjectGuid uGateP;
                    for (int32 i = 0; i < MAX_SUMMON_SLOT; ++i)
                    {
                        if(!caster->m_SummonSlot[i])
                            continue;

                        Unit* uGate = ObjectAccessor::GetUnit(*caster, caster->m_SummonSlot[i]);
                        if(uGate && uGate->GetEntry() == 59271)
                        {
                            distanceG = caster->GetDistance(uGate);
                            uGateG = caster->m_SummonSlot[i];
                        }
                        if(uGate && uGate->GetEntry() == 59262)
                        {
                            distanceP = caster->GetDistance(uGate);
                            uGateP = caster->m_SummonSlot[i];
                        }
                    }

                    if(uGateG && uGateP && (distanceG != 0.0f || distanceP != 0.0f))
                    {
                        if(distanceG > distanceP)
                        {
                            if(Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*caster, uGateP))
                                creature->AI()->OnSpellClick(caster);
                        }
                        else
                        {
                            if(Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*caster, uGateG))
                                creature->AI()->OnSpellClick(caster);
                        }
                    }
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_warl_demonic_gateway_at_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_demonic_gateway_at_SpellScript();
        }
};

// 115236 - Void Shield (Special Ability)
class spell_warl_void_shield : public SpellScriptLoader
{
    public:
        spell_warl_void_shield() : SpellScriptLoader("spell_warl_void_shield") { }

        class spell_warl_void_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_void_shield_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetCaster())
                {
                    caster->CastSpell(caster, 115241, true); // Void Shield Charge 1
                    caster->CastSpell(caster, 115242, true); // Void Shield Charge 2
                    caster->CastSpell(caster, 115243, true); // Void Shield Charge 3
                }
                SetStackAmount(3);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetCaster())
                {
                    caster->RemoveAurasDueToSpell(115241); // Void Shield Charge 1
                    caster->RemoveAurasDueToSpell(115242); // Void Shield Charge 2
                    caster->RemoveAurasDueToSpell(115243); // Void Shield Charge 3
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_warl_void_shield_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_warl_void_shield_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_void_shield_AuraScript();
        }
};

// 115240 - Void Shield
class spell_warl_void_shield_damage : public SpellScriptLoader
{
    public:
        spell_warl_void_shield_damage() : SpellScriptLoader("spell_warl_void_shield_damage") { }

        class spell_warl_void_shield_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_void_shield_damage_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* shield = caster->GetAura(115236))
                    {
                        uint8 stack = shield->GetStackAmount();
                        switch(stack)
                        {
                            case 1:
                            shield->ModStackAmount(-1);
                            return;
                            case 2:
                            caster->RemoveAurasDueToSpell(115242); // Void Shield Charge 2
                            break;
                            case 3:
                            caster->RemoveAurasDueToSpell(115243); // Void Shield Charge 3
                            break;
                        }
                        shield->SetStackAmount(stack - 1);
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_warl_void_shield_damage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_void_shield_damage_SpellScript();
        }
};

// 115232 - Shadow Shield (Special Ability)
class spell_warl_shadow_shield : public SpellScriptLoader
{
    public:
        spell_warl_shadow_shield() : SpellScriptLoader("spell_warl_shadow_shield") { }

        class spell_warl_shadow_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_shadow_shield_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                SetStackAmount(3);
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_warl_shadow_shield_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_shadow_shield_AuraScript();
        }
};

// 115234 - Shadow Shield
class spell_warl_shadow_shield_damage : public SpellScriptLoader
{
    public:
        spell_warl_shadow_shield_damage() : SpellScriptLoader("spell_warl_shadow_shield_damage") { }

        class spell_warl_shadow_shield_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_shadow_shield_damage_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* shield = caster->GetAura(115232))
                        shield->ModStackAmount(-1);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_warl_shadow_shield_damage_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_shadow_shield_damage_SpellScript();
        }
};

// 85692 - Doom Bolt
class spell_warl_doom_bolt : public SpellScriptLoader
{
    public:
        spell_warl_doom_bolt() : SpellScriptLoader("spell_warl_doom_bolt") { }

        class spell_warl_doom_bolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_doom_bolt_SpellScript);

            void HandleOnHit()
            {
                if (Unit* target = GetHitUnit())
                {
                    int32 perc = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
                    if (target->HealthBelowPct(perc))
                        SetHitDamage(int32(GetHitDamage() * 1.2f));
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_warl_doom_bolt_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_doom_bolt_SpellScript();
        }
};

// Mastery: Chaotic Energies - 77220
class spell_warl_mastery_chaotic_energies : public SpellScriptLoader
{
    public:
        spell_warl_mastery_chaotic_energies() : SpellScriptLoader("spell_warl_mastery_chaotic_energies") { }

        class spell_warl_mastery_chaotic_energies_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_mastery_chaotic_energies_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* caster = GetCaster())
                {
                    Unit* attacker = dmgInfo.GetAttacker();
                    if (attacker && attacker != caster)
                    {
                        if (AuraEffect* aurEff = GetAura()->GetEffect(EFFECT_1))
                            absorbAmount = CalculatePct(dmgInfo.GetDamage(), urand(0, aurEff->GetAmount()));
                    }
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_warl_mastery_chaotic_energies_AuraScript::CalculateAmount, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_warl_mastery_chaotic_energies_AuraScript::Absorb, EFFECT_2, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_mastery_chaotic_energies_AuraScript();
        }
};

// Health Funnel - 217979
class spell_warl_health_funnel : public SpellScriptLoader
{
    public:
        spell_warl_health_funnel() : SpellScriptLoader("spell_warl_health_funnel") { }

        class spell_warl_health_funnel_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_health_funnel_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_0]->BasePoints, target));
            }

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* target = GetHitUnit())
                    SetHitHeal(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->BasePoints, target));
            }

            void HandleCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->ToPlayer())
                        if (Pet* pet = caster->ToPlayer()->GetPet())
                            if (pet->IsFullHealth())
                                caster->InterruptSpell(CURRENT_CHANNELED_SPELL, false);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_health_funnel_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnEffectHitTarget += SpellEffectFn(spell_warl_health_funnel_SpellScript::HandleHeal, EFFECT_1, SPELL_EFFECT_HEAL);
                AfterCast += SpellCastFn(spell_warl_health_funnel_SpellScript::HandleCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_health_funnel_SpellScript();
        }
};

// Immolate - 157736
class spell_warl_immolate : public SpellScriptLoader
{
    public:
        spell_warl_immolate() : SpellScriptLoader("spell_warl_immolate") { }

        class spell_warl_immolate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_immolate_AuraScript);

            void OnApplyOrRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetUnitOwner())
                        target->RemoveAurasDueToSpell(205690, caster->GetGUID());
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_warl_immolate_AuraScript::OnApplyOrRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectRemove += AuraEffectRemoveFn(spell_warl_immolate_AuraScript::OnApplyOrRemove, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_immolate_AuraScript();
        }
};

// Soul Harvest - 196098
class spell_warl_soul_harvest : public SpellScriptLoader
{
    public:
        spell_warl_soul_harvest() : SpellScriptLoader("spell_warl_soul_harvest") { }

        class spell_warl_soul_harvest_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_harvest_AuraScript);

            void CalculateMaxDuration(int32 & duration)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* plr = caster->ToPlayer())
                    {
                        int32 pirTarget = GetSpellInfo()->Effects[EFFECT_1]->BasePoints * 1000;
                        int32 maxDur = GetSpellInfo()->Effects[EFFECT_2]->BasePoints * 1000;
                        uint32 spellID = 980;
                        switch (plr->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID))
                        {
                            case SPEC_WARLOCK_DEMONOLOGY:
                                spellID = 603;
                                break;
                            case SPEC_WARLOCK_DESTRUCTION:
                                spellID = 157736;
                                break;
                        }
                        uint32 count = plr->GetCountMyAura(spellID);
                        int32 newDur = duration + pirTarget * count;
                        if (newDur > maxDur)
                            duration = maxDur;
                        else
                            duration = newDur;
                    }
                }
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_warl_soul_harvest_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_soul_harvest_AuraScript();
        }
};

// Demon Skin - 219272
class spell_warl_demon_skin : public SpellScriptLoader
{
    public:
        spell_warl_demon_skin() : SpellScriptLoader("spell_warl_demon_skin") { }

        class spell_warl_demon_skin_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_demon_skin_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    int32 triggered_spell_id = 108366;
                    float perc = aurEff->GetAmount();
                    float addAbsorb = CalculatePct(caster->GetMaxHealth(), perc);
                    float maxPerc = 20.f;

                    if (Unit::AuraEffectList const* mAbsorbtionPercent = caster->GetAuraEffectsByType(SPELL_AURA_MOD_ABSORB_AMOUNT))
                        for (Unit::AuraEffectList::const_iterator i = mAbsorbtionPercent->begin(); i != mAbsorbtionPercent->end(); ++i)
                            AddPct(addAbsorb, (*i)->GetAmount());

                    if (Aura* aura = GetAura())
                    {
                        if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                            maxPerc = eff->GetAmount();
                    }

                    if (Player* plr = caster->ToPlayer())
                    {
                        if (Unit* pet = plr->GetPet())
                        {
                            if (AuraEffect* auraEff = pet->GetAuraEffect(triggered_spell_id, EFFECT_0))
                            {
                                float maxAbsorb = pet->CountPctFromMaxHealth(maxPerc);
                                float allAbsorb = auraEff->GetAmount() + addAbsorb;

                                if (allAbsorb > maxAbsorb)
                                    allAbsorb = maxAbsorb;

                                auraEff->SetAmount(allAbsorb);
                            }
                            else
                            {
                                pet->CastCustomSpell(pet, triggered_spell_id, &addAbsorb, NULL, NULL, true);
                            }
                        }
                    }

                    if (AuraEffect* auraEff = caster->GetAuraEffect(triggered_spell_id, EFFECT_0))
                    {
                        float maxAbsorb = caster->CountPctFromMaxHealth(maxPerc);
                        float allAbsorb = auraEff->GetAmount() + addAbsorb;

                        if (allAbsorb > maxAbsorb)
                            allAbsorb = maxAbsorb;

                        auraEff->SetAmount(allAbsorb);
                    }
                    else
                    {
                        caster->CastCustomSpell(caster, triggered_spell_id, &addAbsorb, NULL, NULL, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_demon_skin_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_demon_skin_AuraScript();
        }
};

// Corruption - 146739
class spell_warl_corruption : public SpellScriptLoader
{
    public:
        spell_warl_corruption() : SpellScriptLoader("spell_warl_corruption") { }

        class spell_warl_corruption_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_corruption_AuraScript);

            void CalculateMaxDuration(int32 & duration)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(196103, EFFECT_0)) // Absolute Corruption
                    {
                        if (GetUnitOwner() && GetUnitOwner()->ToPlayer())
                            duration = aurEff->GetAmount() * IN_MILLISECONDS;
                        else
                            duration = -1;
                    }
                }
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_warl_corruption_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_corruption_AuraScript();
        }
};

// Thal'kiel's Consumption - 211714
class spell_warl_thalkiels_consumption : public SpellScriptLoader
{
    public:
        spell_warl_thalkiels_consumption() : SpellScriptLoader("spell_warl_thalkiels_consumption") { }

        class spell_warl_thalkiels_consumption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_thalkiels_consumption_SpellScript);

            float bp = 0.f;
            Unit* target = NULL;

            void HandleCast()
            {
                if (Unit* caster = GetCaster())
                    if (target && target->IsInWorld())
                        caster->CastCustomSpell(target, 211715, &bp, nullptr, nullptr, true);
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target_ = GetHitUnit())
                        bp += target_->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->BasePoints, caster);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                target = GetHitUnit();
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_thalkiels_consumption_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnEffectHitTarget += SpellEffectFn(spell_warl_thalkiels_consumption_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT);
                OnFinishCast += SpellCastFn(spell_warl_thalkiels_consumption_SpellScript::HandleCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_thalkiels_consumption_SpellScript();
        }
};

// Demonwrath - 193440
class spell_warl_demonwrath : public SpellScriptLoader
{
    public:
        spell_warl_demonwrath() : SpellScriptLoader("spell_warl_demonwrath") { }

        class spell_warl_demonwrath_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonwrath_SpellScript);

            SpellCastResult CheckCast()
            {
                if(Unit* caster = GetCaster())
                    if (caster->ToPlayer())
                        if (Pet* pet = caster->ToPlayer()->GetPet())
                            return SPELL_CAST_OK;

                return SPELL_FAILED_NO_PET;
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                GuidList saveTargets;
                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    if (GetCaster() != (*itr))
                        saveTargets.push_back((*itr)->GetGUID());
                GetSpell()->SetEffectTargets(saveTargets);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_demonwrath_SpellScript::CheckCast);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_demonwrath_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_demonwrath_SpellScript();
        }
};

// Call Dreadstalkers - 104316
class spell_warl_call_dreadstalker : public SpellScript
{
    PrepareSpellScript(spell_warl_call_dreadstalker);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                caster->CastSpell(target, 193331, true);
                caster->CastSpell(target, 193332, true);

                if (AuraEffect* eff = caster->GetAuraEffect(257926, EFFECT_0)) // Item - Warlock T21 Demonology 2P Bonus
                {
                    GuidList* summonList = caster->GetSummonList(98035);
                    for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    {
                        if (Creature* summon = ObjectAccessor::GetCreature(*caster, (*iter)))
                        {
                            float bp0 = eff->GetAmount();
                            summon->CastCustomSpell(summon, 253014, &bp0, nullptr, nullptr, true);
                            eff->GetBase()->Remove();
                        }
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_warl_call_dreadstalker::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Hand of Gul'dan - 105174
class spell_warl_hand_of_guldan : public SpellScript
{
    PrepareSpellScript(spell_warl_hand_of_guldan);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        float bp = GetSpell()->GetPowerCost(POWER_SOUL_SHARDS) / 10.f;
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (caster->HasAura(251851)) // Item - Warlock T21 Demonology 2P Bonus
                {
                    for (int i = 0; i < bp; i++)
                    caster->CastSpell(caster, 257926, true);
                }
                caster->CastCustomSpell(target, 86040, &bp, nullptr, nullptr, true);
                caster->CastCustomSpell(target, 104317, &bp, nullptr, nullptr, true);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_hand_of_guldan::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Hand of Gul'dan - 86040
class spell_warl_hand_of_guldan_damage : public SpellScriptLoader
{
    public:
        spell_warl_hand_of_guldan_damage() : SpellScriptLoader("spell_warl_hand_of_guldan_damage") { }

        class spell_warl_hand_of_guldan_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_hand_of_guldan_damage_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                SetHitDamage(int32(GetHitDamage() * GetSpellValue()->EffectBasePoints[EFFECT_0]));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_hand_of_guldan_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_hand_of_guldan_damage_SpellScript();
        }
};

// Soul Link - 108446
class spell_warl_soul_link : public SpellScriptLoader
{
    public:
        spell_warl_soul_link() : SpellScriptLoader("spell_warl_soul_link") { }

        class spell_warl_soul_link_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_soul_link_AuraScript);

            void SplitDamage(AuraEffect* aurEff, DamageInfo& /*dmgInfo*/, float& absorbAmount)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* owner = caster->GetOwner())
                    {
                        int32 ownerPerc = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
                        int32 petPerc = GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                        float bp0 = CalculatePct(absorbAmount, ownerPerc);
                        float bp1 = CalculatePct(absorbAmount, petPerc);
                        owner->CastCustomSpell(owner, 108447, &bp0, &bp1, NULL, true);
                    }
                }
            }

            void Register() override
            {
                OnEffectSplitDamage += AuraEffectAbsorbFn(spell_warl_soul_link_AuraScript::SplitDamage, EFFECT_0, SPELL_AURA_SPLIT_DAMAGE_PCT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_soul_link_AuraScript();
        }
};

// Demonwrath - 193439
class spell_warl_demonwrath_trigger : public SpellScriptLoader
{
    public:
        spell_warl_demonwrath_trigger() : SpellScriptLoader("spell_warl_demonwrath_trigger") { }

        class spell_warl_demonwrath_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonwrath_trigger_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (Aura* aura = GetCaster()->GetAura(193440))
                {
                    GuidList saveTargets = aura->GetEffectTargets();
                    if (saveTargets.empty())
                        targets.clear();
                    else
                    {
                        std::list<WorldObject*> targetList;
                        for (GuidList::const_iterator iter = saveTargets.begin(); iter != saveTargets.end(); ++iter)
                        {
                            if (Unit* pet = ObjectAccessor::GetUnit(*GetCaster(), *iter))
                            {
                                for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end();)
                                {
                                    if ((*itr)->GetDistance(pet) <= 10.0f && pet->GetCreatureType() == CREATURE_TYPE_DEMON)
                                        targetList.push_back(*itr);
                                    ++itr;
                                }
                            }
                        }
                        targets.clear();
                        targets = targetList;
                    }
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_demonwrath_trigger_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_demonwrath_trigger_SpellScript();
        }
};

// Eye Laser - 205231
class spell_warl_eye_laser : public SpellScriptLoader
{
    public:
        spell_warl_eye_laser() : SpellScriptLoader("spell_warl_eye_laser") { }

        class spell_warl_eye_laser_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_eye_laser_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                        if (target->HasAura(603))
                            return SPELL_CAST_OK;

                    if (Unit* owner = caster->GetAnyOwner())
                    {
                        if (owner->GetCountMyAura(603))
                        {
                            UnitList targets;
                            Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(caster, owner, 40.0f, 603, false);
                            Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(caster, targets, u_check);
                            Trinity::VisitNearbyObject(caster, 80, searcher);

                            targets.remove_if(Trinity::UnitAuraCheck(false, 603, owner->GetGUID()));
                            targets.sort(Trinity::UnitSortDistance(true, caster));

                            if (targets.size() > 1)
                                targets.resize(1);

                            if (!targets.empty())
                                GetSpell()->m_targets.SetUnitTarget(Trinity::Containers::SelectRandomContainerElement(targets));
                        }
                    }
                }

                return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_eye_laser_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_eye_laser_SpellScript();
        }
};

// Demonbolt - 157695
class spell_warl_demonbolt : public SpellScriptLoader
{
    public:
        spell_warl_demonbolt() : SpellScriptLoader("spell_warl_demonbolt") { }

        class spell_warl_demonbolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_demonbolt_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                int32 perc = 100 + GetSpellInfo()->Effects[EFFECT_2]->BasePoints * GetSpell()->GetEffectTargets().size();
                int32 damage = CalculatePct(GetHitDamage(), perc);
                SetHitDamage(damage);
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                GuidList saveTargets;
                for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                    if (Unit* unit = (*itr)->ToUnit())
                        if (GetCaster() != unit && unit->GetCreatureType() == CREATURE_TYPE_DEMON)
                            saveTargets.push_back((*itr)->GetGUID());
                GetSpell()->SetEffectTargets(saveTargets);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_demonbolt_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_CASTER_AREA_SUMMON);
                OnEffectHitTarget += SpellEffectFn(spell_warl_demonbolt_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_demonbolt_SpellScript();
        }
};

// Imp Firebolt - 3110, 115746 - Felbolt
class spell_warl_imp_firebolt : public SpellScriptLoader
{
    public:
        spell_warl_imp_firebolt() : SpellScriptLoader("spell_warl_imp_firebolt") { }

        class spell_warl_imp_firebolt_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_imp_firebolt_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                Unit* owner = caster->GetAnyOwner();
                if (!owner)
                    return;

                AuraEffect* aurEff = owner->GetAuraEffect(231795, EFFECT_0); // Firebolt
                if (!aurEff)
                    return;

                if (target->HasAura(157736, owner->GetGUID()))
                {
                    int32 damage = GetHitDamage();
                    damage += CalculatePct(damage, aurEff->GetAmount());
                    SetHitDamage(damage);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_imp_firebolt_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_imp_firebolt_SpellScript();
        }
};

// Reap Souls (Artifact) - 216698
class spell_warl_reap_souls : public SpellScriptLoader
{
    public:
        spell_warl_reap_souls() : SpellScriptLoader("spell_warl_reap_souls") {}

        class spell_warl_reap_souls_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_reap_souls_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<AreaTrigger*> list;
                    std::vector<uint32> spellIdList = {216721, 216463};
                    caster->GetAreaObjectList(list, spellIdList);

                    if(!list.empty())
                    {
                        int32 duration = 5000;
                        if(AuraEffect* aurEff = caster->GetAuraEffect(236114, EFFECT_0)) // Reap and Sow
                            duration += aurEff->GetAmount();

                        duration *= list.size();

                        if (Aura* aura = caster->GetAura(216708))
                        {
                            duration += aura->GetDuration();
                            if (duration > 60000)
                                duration = 60000;
                            aura->SetDuration(duration);
                        }
                        else
                            caster->CastSpellDuration(caster, 216708, true, duration);

                        for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
                        {
                            if(AreaTrigger* areaObj = (*itr))
                            {
                                caster->CastSpell(areaObj->GetPositionX(), areaObj->GetPositionY(), areaObj->GetPositionZ(), 216714, true);
                                areaObj->Despawn();
                            }
                        }
                        return SPELL_CAST_OK;
                    }
                }
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_ULTHALESH_HAS_NO_POWER_WITHOUT_SOULS);
                return SPELL_FAILED_CUSTOM_ERROR;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_reap_souls_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_reap_souls_SpellScript();
        }
};

// Call Dreadstalkers - 196273, 196274
class spell_warl_call_dreadstalkers : public SpellScriptLoader
{
    public:
        spell_warl_call_dreadstalkers() : SpellScriptLoader("spell_warl_call_dreadstalkers") { }

        class spell_warl_call_dreadstalkers_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_call_dreadstalkers_SpellScript);

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    Creature* imp = nullptr;
                    Creature* stalker = nullptr;

                    if (GuidList* summonList = caster->GetSummonList(98035))
                        for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                            if (Creature* summonPet = ObjectAccessor::GetCreature(*caster, (*iter)))
                                if (!summonPet->HasAura(46598))
                                    stalker = summonPet;

                    if (GuidList* summonList = caster->GetSummonList(99737))
                        for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                            if (Creature* summonPet = ObjectAccessor::GetCreature(*caster, (*iter)))
                                if (summonPet->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL) == GetSpellInfo()->Id)
                                    imp = summonPet;

                    if (imp && stalker)
                    {
                        imp->CastSpell(stalker, 46598, true);
                        if (Unit* _target = stalker->getVictim())
                            imp->Attack(_target, true);
                    }

                    if (imp)
                        imp->SetReactState(REACT_AGGRESSIVE);
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_warl_call_dreadstalkers_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_call_dreadstalkers_SpellScript();
        }
};

// Shadow Bite (Basic Attack) - 54049 , 115778 - Tongue Lash (Basic Attack)
class spell_warl_shadow_bite : public SpellScriptLoader
{
    public:
        spell_warl_shadow_bite() : SpellScriptLoader("spell_warl_shadow_bite") { }

        class spell_warl_shadow_bite_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_shadow_bite_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                Unit* owner = caster->GetAnyOwner();
                if (!owner)
                    return;

                AuraEffect* aurEff = owner->GetAuraEffect(231799, EFFECT_0);
                if (!aurEff)
                    return;

                uint32 count = target->GetDoTsByCaster(owner->GetGUID());
                if (count)
                {
                    int32 perc = aurEff->GetAmount() * count;
                    int32 damage = GetHitDamage();
                    damage += CalculatePct(damage, perc);
                    SetHitDamage(damage);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_shadow_bite_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_shadow_bite_SpellScript();
        }
};

// Fel Cleave - 213688
class spell_warl_fel_cleave : public SpellScriptLoader
{
    public:
        spell_warl_fel_cleave() : SpellScriptLoader("spell_warl_fel_cleave") { }

        class spell_warl_fel_cleave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_fel_cleave_SpellScript);

            void HandleDamage(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[effIndex]->CalcValue(GetCaster())));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_fel_cleave_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_fel_cleave_SpellScript();
        }
};

// Singe Magic (Honor Talent) - 212620
class spell_warl_singe_magic : public SpellScriptLoader
{
    public:
        spell_warl_singe_magic() : SpellScriptLoader("spell_warl_singe_magic") { }

        class spell_warl_singe_magic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_singe_magic_SpellScript);

            void HandleDamage(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[effIndex]->CalcValue(GetCaster())));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_singe_magic_SpellScript::HandleDamage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_singe_magic_SpellScript();
        }
};

// Singe Magic (Honor Talent) - 212623
class spell_warl_singe_magic_cast : public SpellScriptLoader
{
    public:
        spell_warl_singe_magic_cast() : SpellScriptLoader("spell_warl_singe_magic_cast") { }

        class spell_warl_singe_magic_cast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_singe_magic_cast_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                    if (Player* player = caster->ToPlayer())
                        if (Pet* pet = player->GetPet())
                            if (pet->GetEntry() == 416 || pet->GetEntry() == 58959)
                                return SPELL_CAST_OK;

                return SPELL_FAILED_NO_PET;
            }

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                if (Player* player = caster->ToPlayer())
                    if (Pet* pet = player->GetPet())
                        pet->CastSpell(target, 212620, true);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_warl_singe_magic_cast_SpellScript::HandleOnHit);
                OnCheckCast += SpellCheckCastFn(spell_warl_singe_magic_cast_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_singe_magic_cast_SpellScript();
        }
};

// Call Felhunter (Honor Talent) - 212619
class spell_warl_call_felhunter : public SpellScriptLoader
{
    public:
        spell_warl_call_felhunter() : SpellScriptLoader("spell_warl_call_felhunter") { }

        class spell_warl_call_felhunter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_call_felhunter_SpellScript);

            SpellCastResult CheckCast()
            {
                if (Unit* caster = GetCaster())
                    if (Player* player = caster->ToPlayer())
                        if (Pet* pet = player->GetPet())
                            if (pet->GetEntry() == 417 || pet->GetEntry() == 58964)
                                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                return SPELL_CAST_OK; 
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_warl_call_felhunter_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_call_felhunter_SpellScript();
        }
};

// DStolen Power - 211529
class spell_warl_stolen_power : public SpellScriptLoader
{
    public:
        spell_warl_stolen_power() : SpellScriptLoader("spell_warl_stolen_power") { }

        class spell_warl_stolen_power_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_warl_stolen_power_AuraScript);

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                if (GetStackAmount() < 100)
                    return;

                if (Unit* target = GetTarget())
                    target->CastSpell(target, 211583, true);

                GetAura()->Remove();
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_stolen_power_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_warl_stolen_power_AuraScript();
        }
};

// Agony - 980
class spell_warl_agony : public SpellScriptLoader
{
    public:
    spell_warl_agony() : SpellScriptLoader("spell_warl_agony") {}

    class spell_warl_agony_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_agony_AuraScript);

        // https://www.askmrrobot.com/wow/theory/mechanic/spell/agonysoulsharddriver?spec=WarlockAffliction&version=7_2_0_23835
        // This is the hidden tracker in-game that gives you soul shards.
        // Every time Agony ticks, the counter is incremented an average of 0.16. Once it is greater than 1, you get a shard.
        // The value is reduced when you have Agony on 3 or more targets.

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (Aura* aura = caster->GetAura(17940))
                {
                    if (AuraEffect* eff0 = aura->GetEffect(EFFECT_0))
                    {
                        if (AuraEffect* eff1 = aura->GetEffect(EFFECT_1))
                        {
                            uint32 dotCount = RoundingFloatValue(eff0->GetAmount());
                            float dequeueSavedValue = eff1->GetAmount();
                            float value = 0.32f;
                            float RandomFloat = frand(1.f, 100.f);

                            if (AuraEffect* eff = caster->GetAuraEffect(212003, EFFECT_0))
                                AddPct(value, eff->GetAmount());

                            value = dequeueSavedValue + (RandomFloat * value / sqrt(dotCount));

                            if (value >= 100.f)
                            {
                                value -= 100.f;
                                caster->CastSpell(caster, 17941, true);
                            }
                            eff1->SetAmount(value);
                        }
                    }
                }
            }
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (AuraEffect* eff = caster->GetAuraEffect(17940, EFFECT_0))
                {
                    eff->SetAmount(eff->GetAmount() + 1.f);
                }
                else if (Aura* aura = caster->AddAura(17940, caster))
                {
                    if (AuraEffect* eff = aura->GetEffect(EFFECT_0))
                        eff->SetAmount(1.f);
                }
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (AuraEffect* eff = caster->GetAuraEffect(17940, EFFECT_0))
                {
                    float targets = eff->GetAmount() - 1.f;

                    if (targets < 0.f)
                        targets = 0.f;

                    eff->SetAmount(targets);
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_agony_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            AfterEffectApply += AuraEffectApplyFn(spell_warl_agony_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_warl_agony_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_agony_AuraScript();
    }
};

// Drain Soul - 198590
class spell_warl_drain_soul : public SpellScriptLoader
{
    public:
    spell_warl_drain_soul() : SpellScriptLoader("spell_warl_drain_soul") {}

    class spell_warl_drain_soul_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_warl_drain_soul_AuraScript);

        void OnTick(AuraEffect const* /*aurEff*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (AuraEffect* RotAndDecay = caster->GetAuraEffect(212371, EFFECT_0)) // Rot and Decay
                {
                    if (Unit* victim = GetUnitOwner())
                    {
                        Aura* unstable = NULL;
                        int32 minDur = 0;
                        float durBonus = RotAndDecay->GetAmount();

                        Unit::AuraApplicationMap const& appliedAuras = victim->GetAppliedAuras();
                        for (Unit::AuraApplicationMap::const_iterator itr = appliedAuras.begin(); itr != appliedAuras.end(); ++itr)
                        {
                            if (Aura* aura = itr->second->GetBase())
                            {
                                if (aura->GetCasterGUID() == caster->GetGUID())
                                {
                                    if (SpellInfo const* spellInfo = aura->GetSpellInfo())
                                    {
                                        if (spellInfo->ClassOptions.SpellClassMask.HasFlag(1026))
                                        {
                                            int32 finalDur = aura->GetDuration() + durBonus;

                                            if (finalDur > aura->GetMaxDuration())
                                                finalDur = aura->GetMaxDuration();

                                            aura->SetDuration(finalDur);
                                            aura->SetMaxDuration(finalDur);
                                            aura->SetAuraAttribute(AURA_ATTR_REAPPLIED_AURA);
                                        }
                                        else if (spellInfo->ClassOptions.SpellClassMask.HasFlag(0, 256))
                                        {
                                            if (!minDur || minDur > aura->GetDuration())
                                            {
                                                unstable = aura;
                                                minDur = aura->GetDuration() + durBonus;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (unstable)
                        {
                            int32 finalDur = minDur + durBonus;

                            if (finalDur > unstable->GetMaxDuration())
                                finalDur = unstable->GetMaxDuration();

                            unstable->SetDuration(finalDur);
                            unstable->SetMaxDuration(finalDur);
                            unstable->SetAuraAttribute(AURA_ATTR_REAPPLIED_AURA);
                        }
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_warl_drain_soul_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_LEECH);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_warl_drain_soul_AuraScript();
    }
};

// 215111 - Sharpened Dreadfangs
class spell_warl_sharpened_dreadfangs : public AuraScript
{
    PrepareAuraScript(spell_warl_sharpened_dreadfangs);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
    {
        if (Unit* dreadstalker = GetCaster())
        {
            if (Unit* warlock = dreadstalker->GetOwner())
            {
                if (AuraEffect* aurEff = warlock->GetAuraEffect(211123, EFFECT_0)) // Sharpened Dreadfangs
                    amount = aurEff->GetAmount();
            }
        }
    }

    void Register() override
    {
        DoEffectBeforeCalcAmount += AuraEffectCalcAmountFn(spell_warl_sharpened_dreadfangs::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_CRIT_PCT);
    }
};

// 205196 - Dreadbite
class spell_warl_dreadbite : public SpellScript
{
    PrepareSpellScript(spell_warl_dreadbite);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* dreadstalker = GetCaster())
        {
            if (Unit* warlock = dreadstalker->GetOwner())
            {
                if (AuraEffect const* aurEff = warlock->GetAuraEffect(238109, EFFECT_0)) // Jaws of Shadow
                {
                    if (Unit* target = dreadstalker->getVictim())
                    {
                        float bp = aurEff->GetAmount();
                        dreadstalker->CastCustomSpell(target, 242922, &bp, nullptr, nullptr, true, nullptr, nullptr, warlock->GetGUID());
                    }
                }
                if (AuraEffect const* aurEf = warlock->GetAuraEffect(251852, EFFECT_0)) // Item - Warlock T21 Demonology 4P Bonus
                    if (dreadstalker->HasAura(193396))
                        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), aurEf->GetAmount()));
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_dreadbite::HandleOnHit, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// Fel Firebolt (Basic Attack) - 104318
class spell_warl_imp_firebolt_basic : public SpellScript
{
    PrepareSpellScript(spell_warl_imp_firebolt_basic);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target)
            return;

        Unit* owner = caster->GetAnyOwner();
        if (!owner)
            return;

        AuraEffect* aurEff = target->GetAuraEffect(242922, EFFECT_0, owner->GetGUID()); // Jaws of Shadow
        if (!aurEff)
            return;

        int32 damage = GetHitDamage();
        damage += CalculatePct(damage, aurEff->GetAmount());
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warl_imp_firebolt_basic::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Demonic Empowerment - 193396
class spell_warl_demonic_empowerment : public AuraScript
{
    PrepareAuraScript(spell_warl_demonic_empowerment);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
    {
        Unit* caster = GetCaster();
        Unit* target = GetUnitOwner();
        if (!caster || !target)
            return;
        
        if (AuraEffect* aurEff = caster->GetAuraEffect(238145, EFFECT_0)) // Thal'kiel's Ascendance
        {
            if (roll_chance_i(aurEff->GetAmount()))
                if (Unit* victim = target->getVictim())
                    target->CastSpell(victim, 242832, true);
        }

        if (mode & AURA_EFFECT_HANDLE_REAL)
        {
            if (caster->HasAura(251852)) // Item - Warlock T21 Demonology 4P Bonus
            {
                GuidList* summonList = caster->GetSummonList(98035);
                for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                {
                    if (Creature* summon = ObjectAccessor::GetCreature(*caster, (*iter)))
                        summon->CastSpell(target, 194247, true);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_warl_demonic_empowerment::OnApply, EFFECT_1, SPELL_AURA_MELEE_SLOW, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// Compounding Horror - 231489
class spell_warl_compounding_horror : public SpellScriptLoader
{
    public:
        spell_warl_compounding_horror() : SpellScriptLoader("spell_warl_compounding_horror") { }

        class spell_warl_compounding_horror_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_compounding_horror_SpellScript);

            uint32 stackCount = 0;
            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* aura = caster->GetAura(199281))
                    {
                        stackCount = aura->GetStackAmount();
                        aura->Remove();
                    }
                }
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                SetHitDamage(GetHitDamage() * stackCount);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_compounding_horror_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                OnCast += SpellCastFn(spell_warl_compounding_horror_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_compounding_horror_SpellScript();
        }
};

// Cremation (Honor Talent) - 212327
class spell_warl_cremation : public SpellScriptLoader
{
    public:
        spell_warl_cremation() : SpellScriptLoader("spell_warl_cremation") { }

        class spell_warl_cremation_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_cremation_SpellScript);

            void HandleDamage(SpellEffIndex effIndex)
            {
                if (Unit* target = GetHitUnit())
                    SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[effIndex]->CalcValue(GetCaster())));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_warl_cremation_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_cremation_SpellScript();
        }
};

// Thal'kiel's Consumption - 211715
class spell_warl_thalkiels_consumption_damage : public SpellScriptLoader
{
    public:
        spell_warl_thalkiels_consumption_damage() : SpellScriptLoader("spell_warl_thalkiels_consumption_damage") { }

        class spell_warl_thalkiels_consumption_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_warl_thalkiels_consumption_damage_SpellScript);

            void HandleDamage(SpellEffIndex effIndex)
            {
                if (Unit* caster = GetCaster())
                    if (AuraEffect* aurEff = caster->GetAuraEffect(236200, EFFECT_0)) // Thal'kiel's Consumption
                        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), aurEff->GetAmount()));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_warl_thalkiels_consumption_damage_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_warl_thalkiels_consumption_damage_SpellScript();
        }
};

// Soulshatter (Honor Talent) - 212356
class spell_warl_soulshatter : public SpellScript
{
    PrepareSpellScript(spell_warl_soulshatter);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
        {
            if (auto target = GetHitUnit())
            {
                SetHitDamage(target->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster)));
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE, caster->GetGUID());
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT, caster->GetGUID());
                target->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH, caster->GetGUID());
                caster->CastSpell(caster, 212921, true);
                caster->CastSpell(caster, 236471, true);
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_warl_soulshatter::HandleOnHit, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Incinerate - 29722
class spell_warl_incinerate : public SpellScript
{
    PrepareSpellScript(spell_warl_incinerate);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
        {
            caster->CastSpell(caster, 244670, true);
            if (caster->HasAura(242295)) // Item - Warlock T20 Destruction 2P Bonus
                caster->CastSpell(caster, 193540, true);
        }
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        auto caster = GetCaster();
        auto target = GetHitUnit();
        if (caster && target)
            if (GetSpell()->IsCritForTarget(target))
                caster->CastSpell(caster, 193540, true);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(196408))
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
        OnEffectLaunchTarget += SpellEffectFn(spell_warl_incinerate::HandleOnHit, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnEffectHitTarget += SpellEffectFn(spell_warl_incinerate::HandleHit, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warl_incinerate::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

void AddSC_warlock_spell_scripts()
{
    new spell_warl_burning_rush();
    new spell_warl_banish();
    new spell_warl_demonic_circle_summon();
    new spell_warl_demonic_circle_teleport();
    new spell_warl_unstable_affliction();
    new spell_warl_seed_of_corruption_dota();
    new spell_warl_demonic_gateway();
    new spell_warl_demonic_gateway_cast();
    new spell_warl_demonic_gateway_duration();
    new spell_warl_demonic_gateway_at();
    new spell_warl_void_shield();
    new spell_warl_void_shield_damage();
    new spell_warl_shadow_shield();
    new spell_warl_shadow_shield_damage();
    new spell_warl_doom_bolt();
    new spell_warl_mastery_chaotic_energies();
    new spell_warl_health_funnel();
    new spell_warl_immolate();
    new spell_warl_demon_skin();
    new spell_warl_corruption();
    new spell_warl_thalkiels_consumption();
    new spell_warl_soul_harvest();
    new spell_warl_demonwrath();
    RegisterSpellScript(spell_warl_call_dreadstalker);
    RegisterSpellScript(spell_warl_hand_of_guldan);
    new spell_warl_hand_of_guldan_damage();
    new spell_warl_soul_link();
    new spell_warl_demonwrath_trigger();
    new spell_warl_eye_laser();
    new spell_warl_demonbolt();
    new spell_warl_imp_firebolt();
    new spell_warl_reap_souls();
    new spell_warl_call_dreadstalkers();
    new spell_warl_shadow_bite();
    new spell_warl_fel_cleave();
    new spell_warl_singe_magic();
    new spell_warl_singe_magic_cast();
    new spell_warl_call_felhunter();
    new spell_warl_stolen_power();
    RegisterAuraScript(spell_warl_sharpened_dreadfangs);
    RegisterSpellScript(spell_warl_dreadbite);
    RegisterSpellScript(spell_warl_imp_firebolt_basic);
    RegisterAuraScript(spell_warl_demonic_empowerment);
    new spell_warl_compounding_horror();
    new spell_warl_cremation();
    new spell_warl_thalkiels_consumption_damage();
    new spell_warl_drain_soul();
    new spell_warl_agony();
    RegisterSpellScript(spell_warl_soulshatter);
    new spell_warl_streten_insanity();
    RegisterAuraScript(spell_warl_unstable_affliction_R2);
    RegisterSpellScript(spell_warl_create_healthstone);
    RegisterSpellScript(spell_warl_incinerate);
    RegisterAuraScript(spell_warl_searing_bolts);
}