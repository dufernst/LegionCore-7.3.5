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
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "AreaTrigger.h"

// Desecrated ground - 118009
class spell_dk_desecrated_ground : public SpellScriptLoader
{
    public:
        spell_dk_desecrated_ground() : SpellScriptLoader("spell_dk_desecrated_ground") { }

        class spell_dk_desecrated_ground_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_desecrated_ground_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if(Unit* caster = GetCaster())
                    if (DynamicObject* dynObj = caster->GetDynObject(118009))
                        if (caster->GetDistance(dynObj) <= 8.0f)
                            caster->CastSpell(caster, 115018, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_desecrated_ground_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_desecrated_ground_AuraScript();
        }
};

// Festering Strike - 85948
class spell_dk_festering_strike : public SpellScriptLoader
{
    public:
        spell_dk_festering_strike() : SpellScriptLoader("spell_dk_festering_strike") { }

        class spell_dk_festering_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_festering_strike_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if(caster && target)
                {
                    int32 count = GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster);
                    if (AuraEffect* aurEff = caster->GetAuraEffect(207305, EFFECT_0)) // Castigator
                        if (GetSpell()->IsCritForTarget(target))
                            count += aurEff->GetAmount();

                    for (int i = 0; i < count; i++)
                        caster->CastSpell(target, 194310, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_festering_strike_SpellScript::HandleDummy, EFFECT_2, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_festering_strike_SpellScript();
        }
};

// Pillar of Frost - 51271
class spell_dk_pillar_of_frost : public SpellScriptLoader
{
    public:
        spell_dk_pillar_of_frost() : SpellScriptLoader("spell_dk_pillar_of_frost") { }

        class spell_dk_pillar_of_frost_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_pillar_of_frost_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->ApplySpellImmune(51271, IMMUNITY_MECHANIC, MECHANIC_INCAPACITATE, false);
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                    _player->ApplySpellImmune(51271, IMMUNITY_MECHANIC, MECHANIC_INCAPACITATE, true);
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_dk_pillar_of_frost_AuraScript::OnApply, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_pillar_of_frost_AuraScript::OnRemove, EFFECT_2, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_pillar_of_frost_AuraScript();
        }
};

// Purgatory - 116888
class spell_dk_purgatory : public SpellScriptLoader
{
    public:
        spell_dk_purgatory() : SpellScriptLoader("spell_dk_purgatory") { }

        class spell_dk_purgatory_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_purgatory_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Player* _player = GetTarget()->ToPlayer())
                {
                    AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                    if (removeMode == AURA_REMOVE_BY_EXPIRE)
                        _player->CastSpell(_player, 123982, true);
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_purgatory_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_purgatory_AuraScript();
        }
};

// Purgatory - 114556
class spell_dk_purgatory_absorb : public SpellScriptLoader
{
    public:
        spell_dk_purgatory_absorb() : SpellScriptLoader("spell_dk_purgatory_absorb") { }

        class spell_dk_purgatory_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_purgatory_absorb_AuraScript);

            void CalculateAmount(AuraEffect const* /*AuraEffect**/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                Unit* target = GetTarget();

                if (dmgInfo.GetDamage() < target->GetHealth())
                    return;

                // No damage received under Shroud of Purgatory
                if (target->ToPlayer()->HasAura(116888))
                {
                    absorbAmount = dmgInfo.GetDamage();
                    return;
                }

                if (target->ToPlayer()->HasAura(123981))
                    return;

                float bp = dmgInfo.GetDamage();

                target->CastCustomSpell(target, 116888, &bp, NULL, NULL, true);
                target->CastSpell(target, 123981, true);
                target->SetHealth(1);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_purgatory_absorb_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_purgatory_absorb_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_purgatory_absorb_AuraScript();
        }
};

// 50462 - Anti-Magic Shell (on raid member)
class spell_dk_anti_magic_shell_raid : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_raid() : SpellScriptLoader("spell_dk_anti_magic_shell_raid") { }

        class spell_dk_anti_magic_shell_raid_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_raid_AuraScript);

            uint32 absorbPct;

            bool Load() override
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster());
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                // TODO: this should absorb limited amount of damage, but no info on calculation formula
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                 absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_raid_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_raid_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_anti_magic_shell_raid_AuraScript();
        }
};

// 48707 - Anti-Magic Shell (on self)
class spell_dk_anti_magic_shell_self : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_shell_self() : SpellScriptLoader("spell_dk_anti_magic_shell_self") { }

        class spell_dk_anti_magic_shell_self_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_shell_self_AuraScript);

            uint32 absorbPct, container;
            int32 _damage;

            bool Load() override
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster());
                container = 0;
                _damage = 0;
                return true;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(49088))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                if(Unit* caster = GetCaster())
                    amount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster));
                else
                    amount = 0;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = std::min(CalculatePct(dmgInfo.GetDamage(), absorbPct), uint32(aurEff->GetAmount()));
                _damage += absorbAmount;
            }

            void Trigger(AuraEffect* aurEff, DamageInfo & /*dmgInfo*/, float & absorbAmount)
            {
                if (Unit* target = GetTarget()) // AMS generates 2 Runic Power for every percent of maximum health absorbed
                {
                    if (target->HasAura(210852)) // Acherus Drapes
                    {
                        float bp = absorbAmount;
                        target->CastCustomSpell(target, 210862, &bp, NULL, NULL, true);
                    }

                    uint32 RPCap = target->CountPctFromMaxHealth(1);
                    if (RPCap > 0)
                    {
                        float bp = (container + absorbAmount) / RPCap;
                        container = (container + absorbAmount) - (bp * RPCap);
                        bp *= 10;
                        if (target->HasAura(207188)) // Volatile Shielding
                            bp *= 2;

                        if (bp >= 10)
                            target->CastCustomSpell(target, 49088, &bp, NULL, NULL, true, NULL, aurEff);
                    }
                }
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_shell_self_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 AfterEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_shell_self_AuraScript::Trigger, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_anti_magic_shell_self_AuraScript();
        }
};

// 50461 - Anti-Magic Zone (Honor talent)
class spell_dk_anti_magic_zone : public SpellScriptLoader
{
    public:
        spell_dk_anti_magic_zone() : SpellScriptLoader("spell_dk_anti_magic_zone") { }

        class spell_dk_anti_magic_zone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_anti_magic_zone_AuraScript);

            uint32 absorbPct;

            bool Load() override
            {
                absorbPct = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(GetCaster());
                return true;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(51052))
                    return false;
                return true;
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                SpellInfo const* talentSpell = sSpellMgr->GetSpellInfo(51052);
                amount = 136800;
                if (Player* player = GetCaster()->ToPlayer())
                     amount += int32(player->GetStat(STAT_STRENGTH) * 4);
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                 absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPct);
            }

            void Register() override
            {
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_anti_magic_zone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_anti_magic_zone_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_anti_magic_zone_AuraScript();
        }
};

// 47496 - Explode, Ghoul spell for Corpse Explosion
class spell_dk_ghoul_explode : public SpellScriptLoader
{
    public:
        spell_dk_ghoul_explode() : SpellScriptLoader("spell_dk_ghoul_explode") { }

        class spell_dk_ghoul_explode_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_ghoul_explode_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(43999))
                    return false;
                return true;
            }

            void Suicide(SpellEffIndex /*effIndex*/)
            {
                if (Unit* unitTarget = GetHitUnit())
                {
                    // Corpse Explosion (Suicide)
                    unitTarget->CastSpell(unitTarget, 43999, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_ghoul_explode_SpellScript::Suicide, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_ghoul_explode_SpellScript();
        }
};

// Death Gate - 53822, Death Gate - 187753
class spell_dk_death_gate_teleport : public SpellScriptLoader
{
    public:
        spell_dk_death_gate_teleport() : SpellScriptLoader("spell_dk_death_gate_teleport") {}

        class spell_dk_death_gate_teleport_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_teleport_SpellScript);

            SpellCastResult CheckClass()
            {
                if (GetCaster()->getClass() != CLASS_DEATH_KNIGHT)
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                if (GetSpellInfo()->Id == 187753)
                    return SPELL_CAST_OK;

                if (Unit* caster = GetCaster())
                    if (Player* _player = caster->ToPlayer())
                        if(_player->IsQuestRewarded(40715)) // Check quest for port to oplot
                        {
                            caster->CastSpell(caster, 187753, true);
                            return SPELL_FAILED_DONT_REPORT;
                        }

                return SPELL_CAST_OK;
            }

            void HandleTeleport(SpellEffIndex effIndex)
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if(_player->GetCurrentZoneID() == (GetSpellInfo()->Id == 187753 ? 7679 : 139))
                    {
                        PreventHitEffect(effIndex);
                        _player->TeleportTo(_player->m_homebindMapId, _player->m_homebindX, _player->m_homebindY, _player->m_homebindZ, 0.0f);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_teleport_SpellScript::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_L);
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_teleport_SpellScript::CheckClass);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_death_gate_teleport_SpellScript();
        }
};

// Death Gate - 52751
class spell_dk_death_gate : public SpellScriptLoader
{
    public:
        spell_dk_death_gate() : SpellScriptLoader("spell_dk_death_gate") {}

        class spell_dk_death_gate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_death_gate_SpellScript);

            SpellCastResult CheckClass()
            {
                if (auto caster = GetCaster())
                {
                    if (caster->getClass() != CLASS_DEATH_KNIGHT)
                    {
                        SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_MUST_BE_DEATH_KNIGHT);
                        return SPELL_FAILED_CUSTOM_ERROR;
                    }
                    if (auto plr = caster->ToPlayer())
                    {
                        if (!plr->IsQuestRewarded(12801) && !plr->IsQuestRewarded(40715))
                            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                    }
                }

                return SPELL_CAST_OK;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                if (Unit* target = GetHitUnit())
                    target->CastSpell(target, GetEffectValue(), false);
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_death_gate_SpellScript::CheckClass);
                OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_death_gate_SpellScript();
        }
};

// Death Pact - 48743
class spell_dk_death_pact : public SpellScriptLoader
{
    public:
        spell_dk_death_pact() : SpellScriptLoader("spell_dk_death_pact") { }

        class spell_dk_death_pact_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_death_pact_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                if(Unit* caster = GetCaster())
                {
                    amount = CalculatePct(caster->GetMaxHealth(), GetSpellInfo()->Effects[EFFECT_1]->BasePoints);
                    amount += aurEff->GetOldBaseAmount();
                }
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                Unit* target = GetTarget();

                if (SpellInfo const* spellInfo = dmgInfo.GetSpellInfo())
                    if(spellInfo->Id == GetSpellInfo()->Id)
                        absorbAmount = 0;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_death_pact_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_death_pact_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_HEAL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_death_pact_AuraScript();
        }
};

// Death Grip - 49576
class spell_dk_death_grip_dummy : public SpellScript
{
    PrepareSpellScript(spell_dk_death_grip_dummy);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (caster->HasAura(137008)) // DK blood
                    caster->CastSpell(target, 51399, true);

                target->CastSpell(caster, 49575, true);
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dk_death_grip_dummy::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// Gorefiend's Grasp - 108199
class spell_dk_gorefiends_grasp : public SpellScriptLoader
{
    public:
        spell_dk_gorefiends_grasp() : SpellScriptLoader("spell_dk_gorefiends_grasp") { }

        class spell_dk_gorefiends_grasp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_gorefiends_grasp_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetExplTargetUnit();
                Unit* target = GetHitUnit();
                if(caster && target && caster != target)
                {
                    Position const* pos = GetExplTargetDest();
                    caster->CastSpell(target, 114869, true);
                    target->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), 146599, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_gorefiends_grasp_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_gorefiends_grasp_SpellScript();
        }
};

// Corpse Explosion - 127344
class spell_dk_corpse_explosion : public SpellScriptLoader
{
    public:
        spell_dk_corpse_explosion() : SpellScriptLoader("spell_dk_corpse_explosion") {}

        class spell_dk_corpse_explosion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_corpse_explosion_SpellScript);

            SpellCastResult CheckClass()
            {
                if (Unit* target = GetExplTargetUnit())
                {
                    if(!target->isDead() || target->GetDisplayId() != target->GetNativeDisplayId())
                        return SPELL_FAILED_TARGET_NOT_DEAD;
                }
                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_dk_corpse_explosion_SpellScript::CheckClass);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_corpse_explosion_SpellScript();
        }
};

// 152280 - Defile
class spell_dk_defile : public SpellScriptLoader
{
    public:
        spell_dk_defile() : SpellScriptLoader("spell_dk_defile") { }

        class spell_dk_defile_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_defile_AuraScript);

            uint32 tickCount = 0;

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                amount = 0;
            }

            /*void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    GuidList* summonList = caster->GetSummonList(82521);
                    for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    {
                        if(Creature* summon = ObjectAccessor::GetCreature(*caster, (*iter)))
                        {
                            switch (tickCount)
                            {
                                case 0:
                                    summon->SendPlaySpellVisualKit(46617, 0);
                                    break;
                                case 1:
                                    summon->SendPlaySpellVisualKit(46618, 0);
                                    break;
                                case 2:
                                    summon->SendPlaySpellVisualKit(46619, 0);
                                    break;
                                case 3:
                                    summon->SendPlaySpellVisualKit(46629, 0);
                                    break;
                                case 4:
                                    summon->SendPlaySpellVisualKit(46630, 0);
                                    break;
                                case 5:
                                    summon->SendPlaySpellVisualKit(46631, 0);
                                    break;
                                case 6:
                                    summon->SendPlaySpellVisualKit(46632, 0);
                                    break;
                                case 7:
                                    summon->SendPlaySpellVisualKit(46633, 0);
                                    break;
                                case 8:
                                    summon->SendPlaySpellVisualKit(46634, 0);
                                    break;
                                case 9:
                                    summon->SendPlaySpellVisualKit(46635, 0);
                                    break;
                                case 10:
                                    summon->SendPlaySpellVisualKit(46643, 0);
                                    break;
                            }
                        }
                    }
                    tickCount++;
                }
            }*/

            void Register() override
            {
                //OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_defile_AuraScript::OnTick, EFFECT_2, SPELL_AURA_PERIODIC_DUMMY);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_defile_AuraScript::CalculateAmount, EFFECT_3, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_defile_AuraScript();
        }
};

// Breath of Sindragosa - 155166
class spell_dk_breath_of_sindragosa : public SpellScriptLoader
{
    public:
        spell_dk_breath_of_sindragosa() : SpellScriptLoader("spell_dk_breath_of_sindragosa") { }

        class spell_dk_breath_of_sindragosa_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_breath_of_sindragosa_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;
                Aura* aura = caster->GetAura(152279);
                if (!aura)
                    return;
                int32 dam = GetHitDamage();
                ObjectGuid mainTargetGuid = aura->GetEffectTargets().empty() ? ObjectGuid::Empty : aura->GetRndEffectTarget();
                Unit* mainTarget = NULL;
                if (mainTargetGuid)
                    mainTarget = ObjectAccessor::GetUnit(*caster, mainTargetGuid);
                if(!mainTarget)
                {
                    mainTarget = caster->getAttackerForHelper();
                    if (!mainTarget && caster->ToPlayer())
                    {
                        mainTarget = caster->ToPlayer()->GetSelectedUnit();
                        if(mainTarget && !caster->_IsValidAttackTarget(mainTarget, GetSpellInfo()))
                            mainTarget = NULL;
                    }
                    if (!mainTarget)
                        mainTarget = caster->SelectNearbyTarget(caster);
                }
                if(mainTarget && !mainTargetGuid)
                    aura->AddEffectTarget(mainTarget->GetGUID());
                if(target != mainTarget)
                    SetHitDamage(GetHitDamage()/2);
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dk_breath_of_sindragosa_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_breath_of_sindragosa_SpellScript();
        }
};

// Unholy Frenzy - 207290, Runic Corruption - 51460
class spell_dk_change_duration : public SpellScriptLoader
{
    public:
        spell_dk_change_duration() : SpellScriptLoader("spell_dk_change_duration") { }

        class spell_dk_change_duration_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_change_duration_SpellScript);

            int32 duration = 0;

            void HandleBeforeCast()
            {
                if(Unit* caster = GetCaster())
                    if(Aura* aura = caster->GetAura(GetSpellInfo()->Id))
                        duration = aura->GetDuration();
            }

            void HandleAfterCast()
            {
                if(Unit* caster = GetCaster())
                    if(Aura* aura = caster->GetAura(GetSpellInfo()->Id))
                        aura->SetDuration(aura->GetDuration() + duration);
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_dk_change_duration_SpellScript::HandleBeforeCast);
                AfterCast += SpellCastFn(spell_dk_change_duration_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_change_duration_SpellScript();
        }
};

// Will of the Necropolis - 206967
class spell_dk_will_of_the_necropolis : public SpellScriptLoader
{
    public:
        spell_dk_will_of_the_necropolis() : SpellScriptLoader("spell_dk_will_of_the_necropolis") { }

        class spell_dk_will_of_the_necropolis_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_will_of_the_necropolis_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                Unit* caster = GetCaster();
                if(!caster)
                    return;

                if (caster->GetHealthPct() < GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), GetSpellInfo()->Effects[EFFECT_1]->CalcValue(caster));
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_will_of_the_necropolis_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_will_of_the_necropolis_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_will_of_the_necropolis_AuraScript();
        }
};

// Bone Shield - 195181
class spell_dk_bone_shield : public SpellScriptLoader
{
    public:
        spell_dk_bone_shield() : SpellScriptLoader("spell_dk_bone_shield") { }

        class spell_dk_bone_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_bone_shield_AuraScript);

            int32 procDelay = 0;

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                Unit* target = GetTarget();
                if (!target)
                    return;

                int32 absorbPerc = GetSpellInfo()->Effects[EFFECT_4]->CalcValue(target);
                int32 absorbStack = 1;
                if (AuraEffect* aurEff = target->GetAuraEffect(211078, EFFECT_0)) // Spectral Deflection
                {
                    if (target->CountPctFromMaxHealth(aurEff->GetAmount()) < dmgInfo.GetDamage())
                    {
                        absorbPerc *= 2;
                        absorbStack *= 2;
                        ModStackAmount(-1);
                    }
                }
                if (AuraEffect* aurEff = target->GetAuraEffect(192558, EFFECT_0)) // Skeletal Shattering
                {
                    if (roll_chance_f(target->GetFloatValue(PLAYER_FIELD_SPELL_CRIT_PERCENTAGE)))
                        absorbPerc += aurEff->GetAmount();
                }
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPerc);

                if (Player* _player = target->ToPlayer())
                {
                    if ((dmgInfo.GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL) && !procDelay)
                    {
                        if (AuraEffect const* aurEff = _player->GetAuraEffect(251876, EFFECT_0)) // Item - Death Knight T21 Blood 2P Bonus
                            _player->ModifySpellCooldown(49028, aurEff->GetAmount() * absorbStack);

                        if (_player->HasSpell(221699)) // Blood Tap
                            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(221699))
                                _player->ModSpellChargeCooldown(221699, 1000 * spellInfo->Effects[EFFECT_1]->CalcValue(target) * absorbStack);

                        ModStackAmount(-1);
                        procDelay = 2000;
                    }
                }
            }

            void OnStackChange(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if(!target)
                    return;

                if (AuraEffect* aurEff = target->GetAuraEffect(219786, EFFECT_0)) // Ossuary
                {
                    if (GetStackAmount() >= aurEff->GetAmount())
                    {
                        if (!target->HasAura(219788))
                            target->CastSpell(target, 219788, true);
                    }
                    else
                        target->RemoveAurasDueToSpell(219788);
                }
            }

            void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
            {
                if (!procDelay)
                    return;

                procDelay -= diff;

                if (procDelay <= 0)
                    procDelay = 0;
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_bone_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_bone_shield_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectApply += AuraEffectApplyFn(spell_dk_bone_shield_AuraScript::OnStackChange, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
                OnEffectUpdate += AuraEffectUpdateFn(spell_dk_bone_shield_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_bone_shield_AuraScript();
        }
};

// Tombstone - 219809
class spell_dk_tombstone : public SpellScriptLoader
{
    public:
        spell_dk_tombstone() : SpellScriptLoader("spell_dk_tombstone") { }

        class spell_dk_tombstone_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_tombstone_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = 0;
                if (Unit* caster = GetCaster())
                { 
                    if (Aura* aura = caster->GetAura(195181))
                    {
                        uint32 stack = aura->GetStackAmount();
                        uint32 maxStack = GetSpellInfo()->Effects[EFFECT_4]->CalcValue(caster);
                        if (stack > maxStack)
                            stack = maxStack;

                        amount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster)) * stack;
                        if (Player* _player = caster->ToPlayer())
                        {
                            if (_player->HasSpell(221699)) // Blood Tap
                                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(221699))
                                    _player->ModSpellChargeCooldown(221699, 1000 * spellInfo->Effects[EFFECT_1]->CalcValue(caster) * stack);

                            if (AuraEffect const* aurEff = caster->GetAuraEffect(251876, EFFECT_0)) // Item - Death Knight T21 Blood 2P Bonus
                                _player->ModifySpellCooldown(49028, aurEff->GetAmount() * stack);

                            aura->ModStackAmount(-1 * stack);
                        } 
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_tombstone_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_tombstone_AuraScript();
        }
};

// Bonestorm - 194844
class spell_dk_bonestorm : public SpellScriptLoader
{
    public:
        spell_dk_bonestorm() : SpellScriptLoader("spell_dk_bonestorm") { }

        class spell_dk_bonestorm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_bonestorm_AuraScript);

            void CalculateMaxDuration(int32& duration)
            {
                duration = 1000 * GetAura()->GetPowerCost(POWER_RUNIC_POWER) / 100;
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_dk_bonestorm_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_bonestorm_AuraScript();
        }
};

// Blood Mirror - 206977
class spell_dk_blood_mirror : public SpellScriptLoader
{
    public:
        spell_dk_blood_mirror() : SpellScriptLoader("spell_dk_blood_mirror") { }

        class spell_dk_blood_mirror_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_blood_mirror_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                Unit* target = GetTarget();
                if (!target)
                    return;

                int32 absorbPerc = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(target);
                absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPerc);
                float bp = absorbAmount;
                if (ObjectGuid TargetGuid = GetAura()->GetRndEffectTarget())
                    if (Unit* damageTarget = ObjectAccessor::GetUnit(*target, TargetGuid))
                        target->CastCustomSpell(damageTarget, 221847, &bp, NULL, NULL, true);
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if(Unit* caster = GetCaster())
                    if (Unit* target = caster->ToPlayer()->GetSelectedUnit())
                        GetAura()->AddEffectTarget(target->GetGUID());
            }

            void Register() override
            {
                 OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_blood_mirror_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                 DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_blood_mirror_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                 AfterEffectApply += AuraEffectApplyFn(spell_dk_blood_mirror_AuraScript::OnApply, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_blood_mirror_AuraScript();
        }
};

// Frost Strike - 66196, 222026
class spell_dk_frost_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_frost_strike);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            if (Aura* auraInfo = caster->GetAura(207057)) // Shattering Strikes
                if (Unit* target = GetHitUnit())
                    if (Aura* aura = target->GetAura(51714, caster->GetGUID()))
                        if (aura->GetStackAmount() == 5)
                        {
                            SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), auraInfo->GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster)));
                            if (GetId() == 66196)
                                aura->Remove();
                        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_frost_strike::HandleOnHit, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

// Glacial Advance - 194913
class spell_dk_glacial_advance : public SpellScriptLoader
{
    public:
        spell_dk_glacial_advance() : SpellScriptLoader("spell_dk_glacial_advance") { }

        class spell_dk_glacial_advance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_glacial_advance_SpellScript);

            void HandleAfterCast()
            {
                if(Unit* caster = GetCaster())
                {
                    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(195975);
                    if (!spellInfo)
                        return;

                    caster->ClearSpellTargets(195975);

                    CustomSpellValues values;
                    SpellCastTargets targets;
                    targets.SetCaster(caster);

                    uint32 delay = 50;
                    uint32 castTime =  GetSpell()->GetCastTime();
                    ObjectGuid spellGuid = (GetSpell()->m_castGuid[1].GetSubType() == SPELL_CAST_TYPE_NORMAL) ? GetSpell()->m_castGuid[1] : GetSpell()->m_castGuid[0];

                    Position const* pos = GetExplTargetDest();
                    float angle = caster->GetAngle(pos);
                    for (int i = 1; i < 9; i++)
                    {
                        float x = caster->GetPositionX() + 2.5f * std::cos(angle) * i;
                        float y = caster->GetPositionY() + 2.5f * std::sin(angle) * i;
                        float z = caster->GetPositionZ();
                        Trinity::NormalizeMapCoord(x);
                        Trinity::NormalizeMapCoord(y);
                        caster->UpdateGroundPositionZ(x, y, z);
                        Position _position;
                        _position.Relocate(x, y, z, 0.0f);
                        targets.SetDst(_position);
                        caster->AddDelayedEvent(delay*(i - 1), [caster, spellInfo, values, castTime, targets, spellGuid]() -> void
                        {
                            TriggerCastData triggerData;
                            triggerData.triggerFlags = TRIGGERED_FULL_MASK;
                            triggerData.spellGuid = spellGuid;
                            triggerData.originalCaster = caster->GetGUID();
                            triggerData.casttime = castTime;
                            triggerData.SubType = SPELL_CAST_TYPE_MISSILE;

                            caster->CastSpell(targets, spellInfo, &values, triggerData);
                        });
                    }
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_dk_glacial_advance_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_glacial_advance_SpellScript();
        }
};

// Glacial Advance - 195975
class spell_dk_glacial_advance_damage : public SpellScriptLoader
{
    public:
        spell_dk_glacial_advance_damage() : SpellScriptLoader("spell_dk_glacial_advance_damage") { }

        class spell_dk_glacial_advance_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_glacial_advance_damage_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                std::list<WorldObject*> targetTemp = targets;
                for (std::list<WorldObject*>::iterator itr = targetTemp.begin(); itr != targetTemp.end(); ++itr)
                {
                    if(WorldObject* object = (*itr))
                    {
                        if (caster->ExistSpellTarget(195975, object->GetGUID()))
                            targets.remove(object);
                        else
                            caster->AddSpellTargets(195975, object->GetGUID());
                    }
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_glacial_advance_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_glacial_advance_damage_SpellScript();
        }
};

// Scourge Strike - 55090, Clawing Shadows - 207311
class spell_dk_scourge_strike : public SpellScript
{
    PrepareSpellScript(spell_dk_scourge_strike);

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster && target)
        {
            if (Aura* aura = target->GetAura(194310, caster->GetGUID()))
            {
                int32 count = 1;
                if (caster->HasAura(207305)) // Castigator
                    if (GetSpell()->IsCritForTarget(target))
                        count++;

                if (AuraEffect* aurEff = caster->GetAuraEffect(208713, EFFECT_0)) // Lesson of Razuvious
                    if (roll_chance_i(25))
                        count += urand(1, aurEff->GetAmount());

                if (aura->GetStackAmount() < count)
                    count = aura->GetStackAmount();

                for (int i = 0; i < count; i++)
                    caster->CastSpell(target, 194311, true);
                    caster->CastSpell(target, 195757, true);

                aura->ModStackAmount(-count);
            }
        }
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(188290)) // Death and Decay
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
        OnHit += SpellHitFn(spell_dk_scourge_strike::HandleOnHit);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_scourge_strike::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_scourge_strike::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// Scourge Strike (force trigger) - 70890
class spell_dk_scourge_strike_trigger : public SpellScript
{
    PrepareSpellScript(spell_dk_scourge_strike_trigger);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (Unit* caster = GetCaster())
        {
            if (!caster->HasAura(188290)) // Death and Decay
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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_scourge_strike_trigger::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// Festering Wound - 194311
class spell_dk_festering_wound : public SpellScriptLoader
{
    public:
        spell_dk_festering_wound() : SpellScriptLoader("spell_dk_festering_wound") { }

        class spell_dk_festering_wound_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_festering_wound_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* aura = caster->GetAura(194917)) // Pestilent Pustules
                    {
                        aura->ModCustomData(1);
                        if (aura->GetCustomData() >= 8)
                        {
                            caster->CastSpell(caster, 220211, true);
                            aura->SetCustomData(0);
                        }
                    }
                    if (Unit* target = GetExplTargetUnit())
                        if (target->HasAura(130736, caster->GetGUID())) // Soul Reaper
                            caster->CastSpell(caster, 215711, true);
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_dk_festering_wound_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_festering_wound_SpellScript();
        }
};

// Festering Wound Aura - 194310
class spell_dk_festering_wound_dummy : public AuraScript
{
    PrepareAuraScript(spell_dk_festering_wound_dummy);

    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (auto caster = GetCaster())
        {
            if (GetTarget() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            {
                for (int i = 0; i < aurEff->GetBase()->GetStackAmount(); i++)
                    caster->CastSpell(caster, 195757, true);
            }
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_dk_festering_wound_dummy::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Corpse Shield - 207319
class spell_dk_corpse_shield : public AuraScript
{
    PrepareAuraScript(spell_dk_corpse_shield);

    void CalculateAmount(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
    {
        amount = -1;
    }

    void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
    {
        absorbAmount = 0;
        Player* target = GetTarget()->ToPlayer();
        if (!target)
            return;

        if (Pet* pet = target->GetPet())
        {
            if (!pet->isAlive())
                return;

            int32 absorbPerc = GetSpellInfo()->Effects[EFFECT_0]->CalcValue(target);
            absorbAmount = CalculatePct(dmgInfo.GetDamage(), absorbPerc);
            float bp = absorbAmount;
            if (pet->GetHealth() <= absorbAmount)
            {
                absorbAmount = pet->GetHealth();
                target->CastSpell(target, 212756, true);
            }
            target->CastCustomSpell(pet, 212753, &bp, NULL, NULL, true);
        }
    }

    void Register() override
    {
            OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_corpse_shield::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_corpse_shield::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
    }
};

// Shadow Empowerment - 211947
class spell_dk_shadow_empowerment : public AuraScript
{
    PrepareAuraScript(spell_dk_shadow_empowerment);

    void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
    {
        amount += aurEff->GetOldBaseAmount();
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_shadow_empowerment::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    }
};

// Frost Shield - 207203
class spell_dk_frost_shield : public SpellScriptLoader
{
    public:
        spell_dk_frost_shield() : SpellScriptLoader("spell_dk_frost_shield") { }

        class spell_dk_frost_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_frost_shield_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float& amount, bool& /*canBeRecalculated*/)
            {
                if(Unit* caster = GetCaster())
                {
                    amount += aurEff->GetOldBaseAmount();
                    int32 health20 = caster->CountPctFromMaxHealth(20); // 20% from max HP
                    if (health20 < amount)
                        amount = health20;
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_frost_shield_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_frost_shield_AuraScript();
        }
};

// Necrotic Wound - 223929
class spell_dk_necrotic_strike : public SpellScriptLoader
{
    public:
        spell_dk_necrotic_strike() : SpellScriptLoader("spell_dk_necrotic_strike") { }

        class spell_dk_necrotic_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_necrotic_strike_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(223829))
                    {
                        amount = CalculatePct(caster->GetTotalAttackPowerValue(BASE_ATTACK), spellInfo->Effects[EFFECT_0]->BasePoints);
                        amount += aurEff->GetOldBaseAmount();
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_necrotic_strike_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_HEAL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_necrotic_strike_AuraScript();
        }
};

// Consumption (Artifact) - 205223
class spell_dk_consumption : public SpellScriptLoader
{
    public:
        spell_dk_consumption() : SpellScriptLoader("spell_dk_consumption") { }

        class spell_dk_consumption_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_consumption_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if(Unit* caster = GetCaster())
                {
                    float bp = CalculatePct(GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster), GetHitDamage());
                    caster->CastCustomSpell(caster, 205224, &bp, NULL, NULL, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_consumption_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_consumption_SpellScript();
        }
};

// Apocalypse (Artifact) - 220143
class spell_dk_apocalypse : public SpellScriptLoader
{
    public:
        spell_dk_apocalypse() : SpellScriptLoader("spell_dk_apocalypse") { }

        class spell_dk_apocalypse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_apocalypse_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if(caster && target)
                {
                    if (Aura* aura = target->GetAura(194310, caster->GetGUID()))
                    {
                        int32 count = aura->GetStackAmount();
                        int8 cap = caster->CanPvPScalar() ? 4 : GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                        if (count > cap)
                            count = cap;

                        for (int i = 0; i < count; i++)
                        {
                            caster->CastSpell(target, 194311, true);
                            caster->CastSpell(caster, 195757, true);
                            caster->CastSpell(target, 205491, true);
                        }
                        aura->ModStackAmount(-count);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_apocalypse_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_apocalypse_SpellScript();
        }
};

// Hook - 212472, 212528
class spell_dk_hook : public SpellScriptLoader
{
    public:
        spell_dk_hook() : SpellScriptLoader("spell_dk_hook") { }

        class spell_dk_hook_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_hook_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                if (Unit* target = GetHitUnit())
                {
                    if ((target->GetMechanicImmunityMask() & (1 << MECHANIC_GRIP)) || target->HasAuraWithMechanic(1 << MECHANIC_ROOT))
                        caster->CastSpell(target, 49575, true);
                    else
                        target->CastSpell(caster, 49575, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_hook_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_hook_SpellScript();
        }
};

// Marrowrend - 195182
class spell_dk_marrowrend : public SpellScriptLoader
{
    public:
        spell_dk_marrowrend() : SpellScriptLoader("spell_dk_marrowrend") { }

        class spell_dk_marrowrend_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_marrowrend_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* owner = caster->GetAnyOwner())
                        caster = owner;

                    int32 count = GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                    for (int i = 0; i < count; i++)
                        caster->CastSpell(caster, 195181, true);
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_dk_marrowrend_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_marrowrend_SpellScript();
        }
};

// Cadaverous Pallor (Honor Talent) - 201995
class spell_dk_cadaverous_pallor : public SpellScriptLoader
{
    public:
        spell_dk_cadaverous_pallor() : SpellScriptLoader("spell_dk_cadaverous_pallor") { }

        class spell_dk_cadaverous_pallor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_cadaverous_pallor_AuraScript);

            int32 chance = 0;
            void CalculateAmount(AuraEffect const* /*AuraEffect**/, float& amount, bool& /*canBeRecalculated*/)
            {
                amount = -1;
                chance = GetSpellInfo()->Effects[EFFECT_1]->CalcValue(GetCaster());
            }

            void Absorb(AuraEffect* /*AuraEffect**/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;

                if (!roll_chance_i(chance))
                    return;

                float bp0 = dmgInfo.GetDamage() / 5;
                float bp1 = dmgInfo.GetDamage();

                Unit* target = GetTarget();
                target->CastCustomSpell(target, 213726, &bp0, &bp1, NULL, true);
                absorbAmount = dmgInfo.GetDamage();
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_dk_cadaverous_pallor_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_dk_cadaverous_pallor_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_cadaverous_pallor_AuraScript();
        }
};

// Decomposing Aura (Honor Talent) - 199721
class spell_dk_decomposing_aura : public SpellScriptLoader
{
    public:
        spell_dk_decomposing_aura() : SpellScriptLoader("spell_dk_decomposing_aura") { }

        class spell_dk_decomposing_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_decomposing_aura_AuraScript);

            void OnTick(AuraEffect const* /*aurEff*/)
            {
                Unit* target = GetTarget();
                Unit* caster = GetCaster();
                if (target && caster)
                {
                    if (caster->GetDistance(target) > 10.0f)
                        GetAura()->ModStackAmount(-1);
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_dk_decomposing_aura_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_decomposing_aura_AuraScript();
        }
};

// Chill Streak (Honor Talent) - 204199
class spell_dk_chill_streak : public SpellScriptLoader
{
    public:
        spell_dk_chill_streak() : SpellScriptLoader("spell_dk_chill_streak") { }

        class spell_dk_chill_streak_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_chill_streak_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                float bp = GetEffectValue() - 1;
                if (bp > 0)
                    caster->CastCustomSpell(target, 204165, &bp, NULL, NULL, true, NULL, NULL, GetOriginalCaster() ? GetOriginalCaster()->GetGUID() : caster->GetGUID());
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_chill_streak_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_chill_streak_SpellScript();
        }
};

// Chill Streak (Honor Talent) - 204167
class spell_dk_chill_streak_damage : public SpellScriptLoader
{
    public:
        spell_dk_chill_streak_damage() : SpellScriptLoader("spell_dk_chill_streak_damage") { }

        class spell_dk_chill_streak_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_chill_streak_damage_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                SetHitDamage(target->CountPctFromMaxHealth(GetEffectValue()));
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_dk_chill_streak_damage_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_chill_streak_damage_SpellScript();
        }
};

// Zombie Explosion (Honor Talent)- 210141
class spell_dk_zombie_explosion : public SpellScriptLoader
{
    public:
        spell_dk_zombie_explosion() : SpellScriptLoader("spell_dk_zombie_explosion") { }

        class spell_dk_zombie_explosion_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_zombie_explosion_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                SetHitDamage(target->CountPctFromMaxHealth(GetEffectValue()));

                if (Unit* caster = GetCaster())
                    if (Creature* creature = caster->ToCreature())
                        creature->DespawnOrUnsummon(100);
            }

            void Register() override
            {
                OnEffectLaunchTarget += SpellEffectFn(spell_dk_zombie_explosion_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_zombie_explosion_SpellScript();
        }
};

// Umbilicus Eternus - 193249
class spell_dk_umbilicus_eternus : public SpellScriptLoader
{
    public:
        spell_dk_umbilicus_eternus() : SpellScriptLoader("spell_dk_umbilicus_eternus") { }

        class spell_dk_umbilicus_eternus_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_dk_umbilicus_eternus_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                float bp = aurEff->GetAmount() * 5.f;
                if (bp > 0)
                    caster->CastCustomSpell(caster, 193320, &bp, nullptr, nullptr, true);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_dk_umbilicus_eternus_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_dk_umbilicus_eternus_AuraScript();
        }
};

// Black Claws - 212333, 47468
class spell_dk_black_claws : public SpellScriptLoader
{
    public:
        spell_dk_black_claws() : SpellScriptLoader("spell_dk_black_claws") { }

        class spell_dk_black_claws_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_black_claws_SpellScript);

            void HandleOnHit()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                Unit* owner = caster->GetAnyOwner();
                if (!owner || !caster->HasAura(63560))
                    return;

                AuraEffect* aurEff = owner->GetAuraEffect(238116, EFFECT_0); // Black Claws
                if (!aurEff || !roll_chance_i(aurEff->GetAmount()))
                    return;

                if (Aura* aura = target->GetAura(194310, owner->GetGUID()))
                {
                    owner->CastSpell(target, 194311, true);
                    owner->CastSpell(owner, 195757, true);
                    aura->ModStackAmount(-1);
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_dk_black_claws_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_black_claws_SpellScript();
        }
};

// Frozen Soul - 204959
class spell_dk_frozen_soul : public SpellScriptLoader
{
    public:
        spell_dk_frozen_soul() : SpellScriptLoader("spell_dk_frozen_soul") { }

        class spell_dk_frozen_soul_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_frozen_soul_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Aura* aura = caster->GetAura(204957))
                {
                    SetHitDamage(GetHitDamage() * aura->GetStackAmount());
                    aura->SetDuration(100);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_frozen_soul_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_frozen_soul_SpellScript();
        }
};

// 248397 - Cold Heart
class spell_dk_cold_heart : public SpellScriptLoader
{
    public:
        spell_dk_cold_heart() : SpellScriptLoader("spell_dk_cold_heart") { }

        class spell_dk_cold_heart_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_cold_heart_SpellScript);

            uint32 stackCount = 0;

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* aura = caster->GetAura(235599))
                    {
                        stackCount = aura->GetStackAmount();
                        if (stackCount == 20)
                            caster->CastSpell(GetHitUnit(), 248406, true);

                        SetHitDamage(GetHitDamage() * stackCount);
                        aura->Remove();
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_cold_heart_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_cold_heart_SpellScript();
        }
};

// 127517 - Army Transform: Call for Glyph - 58642 
class spell_dk_army_transform : public SpellScriptLoader
{
    public:
        spell_dk_army_transform() : SpellScriptLoader("spell_dk_army_transform") { }

        class spell_dk_army_transform_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_dk_army_transform_SpellScript);

            uint32 ArmyTransforms[6] = {127525, 127526, 127527, 127528, 127533, 127534};

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (auto caster = GetCaster())
                    caster->CastSpell(caster, ArmyTransforms[urand(0, 5)], true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_dk_army_transform_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_dk_army_transform_SpellScript();
        }
};

// 42651, 205491 - Army of the Dead
class spell_dk_army_of_the_dead : public SpellScript
{
    PrepareSpellScript(spell_dk_army_of_the_dead);

    void HandleBeforeCast()
    {
        if (auto caster = GetCaster())
            if (caster->HasAura(242064)) // Item - Death Knight T20 Unholy 2P Bonus
            {
                if (Aura* aur = caster->GetAura(246995)) 
                    aur->SetDuration(aur->GetDuration() + 3000);
                else
                    caster->CastSpellDuration(caster, 246995, true, 3000);
            }
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_dk_army_of_the_dead::HandleBeforeCast);
    }
};

//190778
class spell_dk_sindragosas_fury : public SpellScriptLoader
{
public:
    spell_dk_sindragosas_fury() : SpellScriptLoader("spell_dk_sindragosas_fury") { }

    class spell_dk_sindragosas_fury_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_dk_sindragosas_fury_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (auto caster = GetCaster())
            {
                float x = caster->GetPositionX();
                float y = caster->GetPositionY();
                float z = caster->GetPositionZ() + 4;
                float o = caster->GetOrientation();
                if (auto sindra = caster->SummonCreature(92870, x, y, z, o, TEMPSUMMON_TIMED_DESPAWN, 1800))
                {
                    Position pos = sindra->GetPosition();
                    sindra->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    sindra->SetDisableGravity(true);
                    sindra->CastSpell(sindra, 171686, true);
                    sindra->SetSpeed(MOVE_FLIGHT, 4.5f);
                    sindra->GetNearPoint2D(pos, 45.0f, 0.0f);
                    sindra->GetMotionMaster()->MovePoint(0, pos, false);
                }
            }
        }

        void Register() override
        {
            OnEffectLaunch += SpellEffectFn(spell_dk_sindragosas_fury_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_CREATE_AREATRIGGER);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_dk_sindragosas_fury_SpellScript();
    }
};

// 196780 - Outbreak dummy
class spell_dk_outbreak_aoe_dummy : public SpellScript
{
    PrepareSpellScript(spell_dk_outbreak_aoe_dummy);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (auto caster = GetCaster())
        {
            if (auto target = GetHitUnit())
            {
                if (caster->HasAura(207316))
                    caster->CastSpell(target, 208278, true);

                if (Aura* aur = target->GetAura(191587, caster->GetGUID()))
                    aur->RefreshTimers();
                else
                    caster->CastSpell(target, 191587, true);
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dk_outbreak_aoe_dummy::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 197531 - Bloodworms
class spell_dk_bloodworms : public SpellScript
{
    PrepareSpellScript(spell_dk_bloodworms);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.clear();

        if (Unit* caster = GetCaster())
        {
            for (auto itr : caster->m_Controlled)
                if(Unit* unit = ObjectAccessor::GetUnit(*caster, itr))
                    if (unit->GetEntry() == 99773)
                        targets.push_back(unit);
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_dk_bloodworms::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_SUMMON);
    }
};

// 196771 - Remorseless Winter
class spell_dk_remorseless_winter : public SpellScript
{
    PrepareSpellScript(spell_dk_remorseless_winter);

    void HandleAfterHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (caster->HasAura(207170) && caster->HasAura(51271)) // Winter is Coming
                {
                    if (Aura* aur = target->GetAura(211794, caster->GetGUID()))
                        if (aur->GetStackAmount() == 4)
                            caster->CastSpell(target, 207171, true);

                    caster->CastSpell(target, 211794, true);
                }
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_remorseless_winter::HandleAfterHit);
    }
};

// Consumption - 214837
class spell_dk_consumption_proc : public AuraScript
{
    PrepareAuraScript(spell_dk_consumption_proc);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = eventInfo.GetProcTarget())
            {
                if (Creature* cre = target->ToCreature())
                {
                    if (CreatureTemplate const* cinfo = cre->GetCreatureTemplate())
                    {
                        if (cinfo->Classification == CREATURE_CLASSIFICATION_NORMAL && (target->GetCreatureType() == CREATURE_TYPE_HUMANOID || target->GetCreatureType() == CREATURE_TYPE_DEMON))
                            caster->CastSpell(caster, 215377, true);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_dk_consumption_proc::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Pandemic - 199799 (Honor Talent)
class spell_dk_pandemic_pvp : public SpellScript
{
    PrepareSpellScript(spell_dk_pandemic_pvp);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* unitTarget = GetHitUnit())
            {
                if (Aura* aura = unitTarget->GetAura(191587, caster->GetGUID()))
                    aura->RefreshTimers();
            }
        }
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_dk_pandemic_pvp::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 199373 - Claw
class spell_dk_claw_owner : public SpellScript
{
    PrepareSpellScript(spell_dk_claw_owner);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* summon = GetCaster())
        {
            if (Unit* dk = summon->GetOwner())
            {
                if (dk->HasAura(191731)) // Armies of the Damned
                {
                    if (Unit* target = GetHitUnit())
                    {
                        uint32 spellId[4] = {191727, 191728, 191729, 191730};
                        summon->CastSpell(target, spellId[urand(0, 3)], true, nullptr, nullptr, dk->GetGUID());
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_claw_owner::HandleOnHit, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

void AddSC_deathknight_spell_scripts()
{
    new spell_dk_desecrated_ground();
    new spell_dk_festering_strike();
    new spell_dk_pillar_of_frost();
    new spell_dk_purgatory();
    new spell_dk_purgatory_absorb();
    new spell_dk_anti_magic_shell_raid();
    new spell_dk_anti_magic_shell_self();
    new spell_dk_anti_magic_zone();
    new spell_dk_ghoul_explode();
    new spell_dk_death_gate_teleport();
    new spell_dk_death_gate();
    RegisterSpellScript(spell_dk_death_grip_dummy);
    new spell_dk_death_pact();
    new spell_dk_gorefiends_grasp();
    new spell_dk_corpse_explosion();
    new spell_dk_defile();
    new spell_dk_breath_of_sindragosa();
    new spell_dk_change_duration();
    new spell_dk_will_of_the_necropolis();
    new spell_dk_bone_shield();
    new spell_dk_tombstone();
    new spell_dk_bonestorm();
    new spell_dk_blood_mirror();
    RegisterSpellScript(spell_dk_frost_strike);
    new spell_dk_glacial_advance();
    new spell_dk_glacial_advance_damage();
    RegisterSpellScript(spell_dk_scourge_strike);
    RegisterSpellScript(spell_dk_scourge_strike_trigger);
    RegisterAuraScript(spell_dk_corpse_shield);
    RegisterAuraScript(spell_dk_shadow_empowerment);
    new spell_dk_frost_shield();
    new spell_dk_necrotic_strike();
    new spell_dk_festering_wound();
    RegisterAuraScript(spell_dk_festering_wound_dummy);
    new spell_dk_consumption();
    new spell_dk_apocalypse();
    new spell_dk_hook();
    new spell_dk_marrowrend();
    new spell_dk_cadaverous_pallor();
    new spell_dk_decomposing_aura();
    new spell_dk_chill_streak();
    new spell_dk_chill_streak_damage();
    new spell_dk_zombie_explosion();
    new spell_dk_umbilicus_eternus();
    new spell_dk_black_claws();
    new spell_dk_frozen_soul();
    new spell_dk_cold_heart();
    new spell_dk_army_transform();
    new spell_dk_sindragosas_fury();
    RegisterSpellScript(spell_dk_army_of_the_dead);
    RegisterSpellScript(spell_dk_outbreak_aoe_dummy);
    RegisterSpellScript(spell_dk_bloodworms);
    RegisterSpellScript(spell_dk_remorseless_winter);
    RegisterAuraScript(spell_dk_consumption_proc);
    RegisterSpellScript(spell_dk_pandemic_pvp);
    RegisterSpellScript(spell_dk_claw_owner);
}