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
 * Scripts for spells with SPELLFAMILY_MONK and SPELLFAMILY_GENERIC spells used by monk players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_monk_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "Group.h"
#include "GridNotifiers.h"
#include "AreaTriggerAI.h"
#include "AreaTrigger.h"


enum MonkVariables
{
    WindwalkingAuraOfSpeed      = 166646,
    WindwalkingAuraDuration     = 10000,
    Permanent                   = -1,
    RingOfPeaceLongKnockBack    = 237371,
    RingOfPeaceShortKnockBack   = 142895
};

enum StormEarthAndFireSpells
{
	SPELL_MONK_SEF = 137639,

	NPC_FIRE_SPIRIT = 69791,
	NPC_EARTH_SPIRIT = 69792
};

// 137639
class spell_monk_storm_earth_and_fire : public SpellScriptLoader
{
    public:
        spell_monk_storm_earth_and_fire() : SpellScriptLoader("spell_monk_storm_earth_and_fire") { }

        class spell_monk_storm_earth_and_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_storm_earth_and_fire_AuraScript);

            void RemoveEff(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    for (uint8 i = 3; i < 5; ++i)
                        if (caster->m_SummonSlot[i])
                            if (Creature* crt = caster->GetMap()->GetCreature(caster->m_SummonSlot[i]))
                                if (!crt->IsDespawn())
                                    if (CreatureAI* crtAI = crt->AI())
                                        crtAI->ComonOnHome();
                }
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_monk_storm_earth_and_fire_AuraScript::RemoveEff, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_storm_earth_and_fire_AuraScript();
        }
};

class spell_monk_storm_earth_and_fire_clone_visual : public SpellScriptLoader
{
public:
    spell_monk_storm_earth_and_fire_clone_visual() : SpellScriptLoader("spell_monk_storm_earth_and_fire_clone_visual") { }

    class spell_monk_storm_earth_and_fire_clone_visualAuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_storm_earth_and_fire_clone_visualAuraScript);

        void ApplyEff(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit* caster = GetCaster())
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    if (!caster->m_SummonSlot[13] && !caster->m_SummonSlot[14])
                        caster->RemoveAura(137639);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_monk_storm_earth_and_fire_clone_visualAuraScript::ApplyEff, EFFECT_2, SPELL_AURA_OVERRIDE_SPELL_VISUAL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_storm_earth_and_fire_clone_visualAuraScript();
    }
};

class spell_monk_clone_cast : public SpellScriptLoader
{
public:
    spell_monk_clone_cast() : SpellScriptLoader("spell_monk_clone_cast") { }

    class spell_monk_clone_cast_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_clone_cast_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if(Unit* caster = GetCaster())
                if (caster->HasSpell(139598))
                    caster->CastSpell(caster, 139597, true);
        }

        void Register() override
        {
            OnEffectHitTarget += SpellEffectFn(spell_monk_clone_cast_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_monk_clone_cast_SpellScript();
    }
};

// Windwalking - 157411
class areatrigger_at_windwalking : public AreaTriggerScript
{
    public:
    areatrigger_at_windwalking() : AreaTriggerScript("areatrigger_at_windwalking") {}

    struct areatrigger_at_windwalkingAI : AreaTriggerAI
    {
        areatrigger_at_windwalkingAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) {}

        bool IsValidTarget(Unit* caster, Unit* target, AreaTriggerActionMoment actionM) override
        {
            if (caster && !caster->isAlive())
                return false;

            if (target->GetGUID() == at->GetCasterGUID())
                return true;

            if (Aura* aura = target->GetAura(WindwalkingAuraOfSpeed))
            {
                if (aura->GetDuration() != Permanent)
                {
                    if (aura->GetCasterGUID() == at->GetCasterGUID())
                    {
                        if (aura->GetDuration() != Permanent)
                            aura->SetAuraTimer(Permanent);
                    }
                    else
                        return true;
                }
                return false;
            }
            return true;
        }

        void OnUnitExit(Unit* unit) override
        {
            if (unit->GetGUID() == at->GetCasterGUID())
            {
                GuidList* affectedList = at->GetAffectedPlayers();
                for (GuidList::const_iterator iter = affectedList->begin(); iter != affectedList->end(); ++iter)
                {
                    if (Unit* unit = ObjectAccessor::GetUnit(*at, *iter))
                        BeforeRemove(unit);
                }
            }
            else
            {
                BeforeRemove(unit);
            }
        }

        void BeforeRemove(Unit* unit) override
        {
            if (Aura* aura = unit->GetAura(WindwalkingAuraOfSpeed, at->GetCasterGUID()))
                aura->SetAuraTimer(WindwalkingAuraDuration);
        }

        void ActionOnUpdate(GuidList& affectedPlayers) override
        {
            if (Unit* caster = at->GetCaster())
            {
                if (!caster->isAlive())
                    return;

                for (GuidList::iterator itr = affectedPlayers.begin(), next; itr != affectedPlayers.end(); itr = next)
                {
                    next = itr;
                    ++next;

                    if (*itr == at->GetCasterGUID())
                        continue;

                    if (Unit* unit = ObjectAccessor::GetUnit(*at, *itr))
                    {
                        if (Aura* aura = unit->GetAura(WindwalkingAuraOfSpeed))
                        {
                            if (aura->GetDuration() != Permanent)
                            {
                                if (!unit->IsInRaidWith(caster))
                                {
                                    affectedPlayers.erase(itr);
                                    continue;
                                }
                                else
                                    caster->CastSpell(unit, WindwalkingAuraOfSpeed, true);
                            }
                            else
                            {
                                if (aura->GetCasterGUID() != at->GetCasterGUID())
                                    continue;

                                if (!unit->IsInRaidWith(caster))
                                {
                                    aura->SetAuraTimer(WindwalkingAuraDuration);
                                    affectedPlayers.erase(itr);
                                    continue;
                                }
                            }
                        }
                        else
                            caster->CastSpell(unit, WindwalkingAuraOfSpeed, true);
                    }
                }
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_windwalkingAI(areatrigger);
    }
};

// Ring of Peace - 116844
class areatrigger_at_ring_of_peace : public AreaTriggerScript
{
    public:
    areatrigger_at_ring_of_peace() : AreaTriggerScript("areatrigger_at_ring_of_peace") {}

    struct areatrigger_at_ring_of_peaceAI : AreaTriggerAI
    {
        areatrigger_at_ring_of_peaceAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        void OnCreate() override
        {
            if (Unit* caster = at->GetCaster())
            {
                caster->CastSpell(at->GetPositionX(), at->GetPositionY(), at->GetPositionZH(), RingOfPeaceLongKnockBack, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
            }
        }

        void ActionOnUpdate(GuidList& affectedPlayers) override
        {
            if (affectedPlayers.empty())
                return;

            if (Unit* caster = at->GetCaster())
            {
                for (GuidList::const_iterator itr = affectedPlayers.begin(); itr != affectedPlayers.end(); ++itr)
                {
                    if (Unit* unit = ObjectAccessor::GetUnit(*at, *itr))
                    {
                        if (Player* plr = unit->ToPlayer())
                        {
                            if (!plr->GetKnockBackTime())
                            {
                                caster->CastSpell(at->GetPositionX(), at->GetPositionY(), at->GetPositionZH(), RingOfPeaceShortKnockBack, TriggerCastFlags(TRIGGERED_FULL_MASK | TRIGGERED_CASTED_BY_AREATRIGGER));
                                return;
                            }
                        }
                    }
                }
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_ring_of_peaceAI(areatrigger);
    }
};

// Diffuse Magic - 122783
class spell_monk_diffuse_magic : public SpellScriptLoader
{
    public:
        spell_monk_diffuse_magic() : SpellScriptLoader("spell_monk_diffuse_magic") { }

        class spell_monk_diffuse_magic_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_diffuse_magic_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    Unit::AuraApplicationMap AuraList = caster->GetAppliedAuras();
                    for (Unit::AuraApplicationMap::iterator iter = AuraList.begin(); iter != AuraList.end(); ++iter)
                    {
                        Aura* aura = iter->second->GetBase();
                        if (!aura)
                            continue;

                        Unit* target = aura->GetCaster();
                        if (!target || target->GetGUID() == caster->GetGUID())
                            continue;

                        if (!target->IsWithinDist(caster, 40.0f))
                            continue;

                        if (aura->GetSpellInfo()->IsPositive())
                            continue;

                        if (aura->GetSpellInfo()->Id == 235213 || aura->GetSpellInfo()->Id == 235240)
                            continue;

                        if (!(aura->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC))
                            continue;

                        bool badTarget = false;

                        if (auto crt = target->ToCreature())
                            if (crt->isWorldBoss() || crt->IsDungeonBoss() || crt->isTrigger())
                                badTarget = true;

                        if (caster->_IsValidAttackTarget(target, aura->GetSpellInfo()))
                            badTarget = true;

                        if (!badTarget)
                        {
                            caster->AddAura(aura->GetSpellInfo()->Id, target);

                            if (Aura* targetAura = target->GetAura(aura->GetSpellInfo()->Id, caster->GetGUID()))
                            {
                                targetAura->SetMaxDuration(aura->GetDuration());
                                targetAura->SetDuration(aura->GetDuration());
                                targetAura->SetStackAmount(aura->GetStackAmount());

                                for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                                    if (AuraEffect* targetEff = targetAura->GetEffect(i))
                                        if (AuraEffect* Eff = aura->GetEffect(i))
                                        {
                                            targetEff->SetAmount(Eff->GetAmount());
                                            targetEff->SetPeriodicTimer(Eff->GetPeriodicTimer());
                                        }
                            }
                        }

                        caster->RemoveAura(aura->GetSpellInfo()->Id, target->GetGUID());
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_monk_diffuse_magic_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_diffuse_magic_SpellScript();
        }
};

// Zen Flight - 125883
class spell_monk_zen_flight_check : public SpellScriptLoader
{
    public:
        spell_monk_zen_flight_check() : SpellScriptLoader("spell_monk_zen_flight_check") { }

        class spell_monk_zen_flight_check_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_flight_check_SpellScript);

            SpellCastResult CheckTarget()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (_player->GetMap()->IsBattlegroundOrArena())
                        return SPELL_FAILED_NOT_IN_BATTLEGROUND;

                    bool canFly = !_player->GetMap()->IsDungeon() && !_player->GetMap()->IsBattlegroundOrArena();
                    switch (_player->GetMapId())
                    {
                        case 1116:
                            if (!_player->HasSpell(191645))
                                canFly = false;
                            break;
                        case 1220:
                            if (!_player->HasSpell(233368))
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
                            if (!_player->HasSpell(90267))
                                canFly = false;
                            break;
                    }
                    if (!canFly)
                        return SPELL_FAILED_NOT_HERE;
                }

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_zen_flight_check_SpellScript::CheckTarget);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_zen_flight_check_SpellScript();
        }
};

// Tiger Palm - 100780
class spell_monk_power_strikes : public SpellScriptLoader
{
    public:
        spell_monk_power_strikes() : SpellScriptLoader("spell_monk_power_strikes") { }

        class spell_monk_power_strikes_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_power_strikes_SpellScript)

            void HandleAfterCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(129914)) // Power Strikes
                        caster->CastSpell(caster, 121283, true);

                    caster->RemoveAura(129914);
                }
            }

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                int32 cd = GetSpellInfo()->Effects[EFFECT_2]->CalcValue(caster) * -1000;

                if (Player* plr = caster->ToPlayer())
                {
                    plr->ModifySpellCooldown(115203, cd);
                    plr->ModifySpellCooldown(115399, cd);
                    plr->ModSpellChargeCooldown(119582, -cd);
                }

                AuraEffect const* aurEff = caster->GetAuraEffect(213116, EFFECT_0); // Face Palm
                if (!aurEff || !roll_chance_i(aurEff->GetAmount()))
                    return;

                if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(227679))
                {
                    cd += spellInfo->Effects[EFFECT_1]->CalcValue(caster) * -1000;
                    SetHitDamage(CalculatePct(GetHitDamage(), spellInfo->Effects[EFFECT_0]->CalcValue(caster)));
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_power_strikes_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
                AfterCast += SpellCastFn(spell_monk_power_strikes_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_power_strikes_SpellScript();
        }
};

// Touch of Karma - 122470
class spell_monk_touch_of_karma : public SpellScriptLoader
{
    public:
        spell_monk_touch_of_karma() : SpellScriptLoader("spell_monk_touch_of_karma") { }

        class spell_monk_touch_of_karma_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_touch_of_karma_AuraScript);

            ObjectGuid eff1Target;

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                    caster->m_SpecialTarget = GetUnitOwner()->GetGUID();
            }

            void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    caster->RemoveAura(122470);
                    caster->m_SpecialTarget.Clear();
                }
            }

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                if (GetCaster())
                    amount = CalculatePct(GetCaster()->GetMaxHealth(), GetSpellInfo()->Effects[EFFECT_2]->BasePoints);
            }

            void AfterAbsorb(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, float& absorbAmount)
            {
                if (Unit* caster = dmgInfo.GetVictim())
                {
                    if (Player* plr = caster->ToPlayer())
                    {
                        if (!caster->m_SpecialTarget)
                            return;

                        if (Unit* target = ObjectAccessor::GetUnit(*caster, caster->m_SpecialTarget))
                        {
                            float bp = absorbAmount;

                            AddPct(bp, plr->GetFloatValue(PLAYER_FIELD_VERSATILITY) + plr->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));

                            if (AuraEffect* aurEff = caster->GetAuraEffect(208842, EFFECT_0)) // Cenedril, Reflector of Hatred
                                AddPct(bp, aurEff->GetAmount());

                            if (caster->HasAura(195295)) // Good Karma
                                caster->CastCustomSpell(caster, 195318, &bp, NULL, NULL, true);

                            bp /= 6.f;

                            if (bp)
                                caster->CastCustomSpell(target, 124280, &bp, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_monk_touch_of_karma_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_monk_touch_of_karma_AuraScript::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_touch_of_karma_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                AfterEffectAbsorb += AuraEffectAbsorbFn(spell_monk_touch_of_karma_AuraScript::AfterAbsorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_touch_of_karma_AuraScript();
        }
};

// Flying Serpent Kick - 115057
class spell_monk_flying_serpent_kick : public SpellScriptLoader
{
    public:
        spell_monk_flying_serpent_kick() : SpellScriptLoader("spell_monk_flying_serpent_kick") { }

        class spell_monk_flying_serpent_kick_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_flying_serpent_kick_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(115057))
                    return false;
                return true;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (caster->HasAura(101545))
                        caster->RemoveAura(101545);

                    caster->CastSpell(caster, 123586, true);
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_monk_flying_serpent_kick_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_flying_serpent_kick_SpellScript();
        }
};

// Purifying Brew - 119582
class spell_monk_purifying_brew: public SpellScript
{
    PrepareSpellScript(spell_monk_purifying_brew);

    SpellCastResult CheckCast()
    {
        if (Unit* caster = GetCaster())
            if (!caster->HasAura(124255)) // Check Stagger DoT
                return SPELL_FAILED_CASTER_AURASTATE;

        return SPELL_CAST_OK;
    }

    void HandleOnHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(145055))  // Item - Monk T16 Brewmaster 4P Bonus
                caster->CastSpell(caster, 145056, true);

            if (caster->HasAura(196738))  // Elusive Dance
            {
                float basepoints = 0.f;
                if (caster->HasAura(124275))
                    basepoints = 5;
                if (caster->HasAura(124274))
                    basepoints = 10;
                if (caster->HasAura(124273))
                    basepoints = 15;
                if (basepoints)
                    caster->CastCustomSpell(caster, 196739, &basepoints, &basepoints, nullptr, true);
            }

            if (AuraEffect const* aurEff = caster->GetAuraEffect(124255, EFFECT_1))
            {
                if (AuraEffect const* gaiPlins = caster->GetAuraEffect(208837, EFFECT_0)) // Gai Plin's Imperial Brew
                {
                    float bp0 = CalculatePct(aurEff->GetAmount(), gaiPlins->GetAmount());
                    caster->CastCustomSpell(caster, 208840, &bp0, nullptr, nullptr, true);
                }

                int32 staggerNerf = CalculatePct(aurEff->GetAmount(), GetSpellInfo()->Effects[EFFECT_0]->CalcValue(caster));
                if (staggerNerf)
                    caster->CalcStaggerDamage(staggerNerf, SPELL_SCHOOL_MASK_NORMAL, aurEff->GetSpellInfo());

                if (AuraEffect const* hotTrub = caster->GetAuraEffect(202126, EFFECT_0)) // Hot Trub (Honor Talent)
                {
                    float bp0 = CalculatePct(staggerNerf, hotTrub->GetAmount());
                    caster->CastCustomSpell(caster, 202127, &bp0, nullptr, nullptr, true);
                }
            }

            if (AuraEffect* aurEff2 = caster->GetAuraEffect(238129, EFFECT_2)) // Quick Sip
            {
                uint32 dur = aurEff2->GetAmount() * IN_MILLISECONDS;
                if (Aura* aura = caster->GetAura(215479))
                    aura->SetDuration(aura->GetDuration() + dur);
                else
                    caster->AddAura(215479, caster, nullptr, NULL, dur);
            }
        }
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_monk_purifying_brew::CheckCast);
        OnHit += SpellHitFn(spell_monk_purifying_brew::HandleOnHit);
    }
};

// Zen Pilgrimage - 126892, Zen Pilgrimage - 194011
class spell_monk_zen_pilgrimage : public SpellScriptLoader
{
    public:
        spell_monk_zen_pilgrimage() : SpellScriptLoader("spell_monk_zen_pilgrimage") { }

        class spell_monk_zen_pilgrimage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_pilgrimage_SpellScript);

            SpellCastResult CheckDist()
            {
                if (GetSpellInfo()->Id == 194011)
                    return SPELL_CAST_OK;

                if (Unit* caster = GetCaster())
                    if (Player* _player = caster->ToPlayer())
                        if(_player->IsQuestRewarded(40236)) // Check quest for port to oplot
                        {
                            caster->CastSpell(caster, 194011, false);
                            return SPELL_FAILED_DONT_REPORT;
                        }

                return SPELL_CAST_OK;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (Player* _player = caster->ToPlayer())
                    {
                        _player->SaveRecallPosition();
                        _player->CastSpell(_player, 126896, true);
                    }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_monk_zen_pilgrimage_SpellScript::HandleOnCast);
                OnCheckCast += SpellCheckCastFn(spell_monk_zen_pilgrimage_SpellScript::CheckDist);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_zen_pilgrimage_SpellScript();
        }
};

// Zen Pilgrimage : Return - 126895
class spell_monk_zen_pilgrimage_return : public SpellScriptLoader
{
    public:
        spell_monk_zen_pilgrimage_return() : SpellScriptLoader("spell_monk_zen_pilgrimage_return") { }

        class spell_monk_zen_pilgrimage_return_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_pilgrimage_return_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* _player = caster->ToPlayer())
                    {
                        _player->TeleportTo(_player->m_recallLoc);
                        _player->RemoveAura(126896);
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pilgrimage_return_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_zen_pilgrimage_return_SpellScript();
        }
};

// Provoke - 115546
class spell_monk_provoke : public SpellScriptLoader
{
    public:
        spell_monk_provoke() : SpellScriptLoader("spell_monk_provoke") { }

        class spell_monk_provoke_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_provoke_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* target = GetExplTargetUnit();
                if (!target)
                    return SPELL_FAILED_NO_VALID_TARGETS;
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_TARGETS;
                else if (!target->IsWithinLOSInMap(GetCaster()))
                    return SPELL_FAILED_LINE_OF_SIGHT;
                return SPELL_CAST_OK;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (target && caster)
                {
                    if (caster->getClass() == CLASS_MONK && caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (target->GetEntry() == 61146)
                            caster->CastSpell(target, 118635, true, NULL, NULL, target->GetGUID());
                        else
                            caster->CastSpell(target, 116189, true);
                    }
                }
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_monk_provoke_SpellScript::CheckCast);
                OnEffectHitTarget += SpellEffectFn(spell_monk_provoke_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_provoke_SpellScript();
        }
};

// Roll - 109132
class spell_monk_roll : public SpellScriptLoader
{
    public:
        spell_monk_roll() : SpellScriptLoader("spell_monk_roll") { }

        class spell_monk_roll_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_roll_SpellScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(109132))
                    return false;
                return true;
            }

            void HandleBeforeCast()
            {
                Aura* aur = GetCaster()->AddAura(107427, GetCaster());
                if (!aur)
                    return;

                AuraApplication* app =  aur->GetApplicationOfTarget(GetCaster()->GetGUID());
                if (!app)
                    return;

                app->ClientUpdate();
            }

            void HandleAfterCast()
            {
                Unit* caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                caster->CastSpell(caster, 107427, true);
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_monk_roll_SpellScript::HandleBeforeCast);
                AfterCast += SpellCastFn(spell_monk_roll_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_roll_SpellScript();
        }
};

// Zen Flight - 125883 (remove aura)
class spell_monk_remove_zen_flight : public SpellScriptLoader
{
    public:
        spell_monk_remove_zen_flight() : SpellScriptLoader("spell_monk_remove_zen_flight") { }

        class spell_monk_remove_zen_flight_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_remove_zen_flight_AuraScript);

            uint32 checkTimer = 1000;

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                Player* player = target->ToPlayer();
                if (!player)
                    return;

                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();

                if (removeMode != AURA_REMOVE_BY_CANCEL)
                {
                    player->CastSpell(player, 54649, true);
                }
            }

            void OnUpdate(uint32 diff)
            {
                if (checkTimer <= diff)
                {
                    checkTimer = 2000;
        
                    if (Player* _player = GetCaster()->ToPlayer())
                    {
                        bool canFly = !_player->GetMap()->IsDungeon() && !_player->GetMap()->IsBattlegroundOrArena();

                        switch (_player->GetMapId())
                        {
                            case 1116:
                                if (!_player->HasSpell(191645))
                                    canFly = false;
                                break;
                            case 1220:
                                if (!_player->HasSpell(233368))
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
                                if (!_player->HasSpell(90267))
                                    canFly = false;
                                break;
                        }
                        if (!canFly)
                            GetAura()->Remove();
                    }
                }
                else
                    checkTimer -= diff;
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_monk_remove_zen_flight_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_MOD_INCREASE_VEHICLE_FLIGHT_SPEED, AURA_EFFECT_HANDLE_REAL);
                OnAuraUpdate += AuraUpdateFn(spell_monk_remove_zen_flight_AuraScript::OnUpdate);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_remove_zen_flight_AuraScript();
        }
};

// Transcendence - 101643
class spell_monk_transcendence : public SpellScriptLoader
{
    public:
        spell_monk_transcendence() : SpellScriptLoader("spell_monk_transcendence") { }

        class spell_monk_transcendence_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_transcendence_SpellScript);

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

            void HandleBeforeCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if(caster->m_SummonSlot[17])
                    {
                        if(Creature* summon = caster->GetMap()->GetCreature(caster->m_SummonSlot[17]))
                            summon->DespawnOrUnsummon(500);
                        caster->RemoveAreaObject(199391); // Spirit Tether
                    }
                }
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_monk_transcendence_SpellScript::HandleBeforeCast);
                OnCheckCast += SpellCheckCastFn(spell_monk_transcendence_SpellScript::CheckElevation);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_transcendence_SpellScript();
        }
};

// Transcendence: Transfer - 119996
class spell_monk_transcendence_transfer : public SpellScriptLoader
{
    public:
        spell_monk_transcendence_transfer() : SpellScriptLoader("spell_monk_transcendence_transfer") { }

        class spell_monk_transcendence_transfer_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_transcendence_transfer_SpellScript);

            SpellCastResult CheckDist()
            {
                if (Unit* caster = GetCaster())
                    if (caster->m_SummonSlot[17])
                        if (Creature* summon = caster->GetMap()->GetCreature(caster->m_SummonSlot[17]))
                            if (summon->IsWithinDistInMap(caster, 40.0f))
                                return SPELL_CAST_OK;

                return SPELL_FAILED_OUT_OF_RANGE;
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if(caster->m_SummonSlot[17])
                    {
                        if(Creature* summon = caster->GetMap()->GetCreature(caster->m_SummonSlot[17]))
                        {
                            float x, y, z, o;
                            summon->GetPosition(x, y, z, o);
                            summon->NearTeleportTo(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), caster->GetOrientation());
                            caster->NearTeleportTo(x, y, z, o);
                            if (AreaTrigger* areaObj = caster->GetAreaObject(199391))
                                areaObj->Relocate(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), caster->GetOrientation()); // Relocate AT Spirit Tether
                        }
                    }
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_monk_transcendence_transfer_SpellScript::HandleOnCast);
                OnCheckCast += SpellCheckCastFn(spell_monk_transcendence_transfer_SpellScript::CheckDist);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_transcendence_transfer_SpellScript();
        }
};

// Touch of Death - 115080
class spell_monk_touch_of_death : public SpellScriptLoader
{
    public:
        spell_monk_touch_of_death() : SpellScriptLoader("spell_monk_touch_of_death") { }

        class spell_monk_touch_of_death_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_touch_of_death_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* target = GetUnitOwner())
                    {
                        if (target->GetTypeId() == TYPEID_PLAYER)
                            amount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->BasePoints / 4);
                        else
                            amount = caster->CountPctFromMaxHealth(GetSpellInfo()->Effects[EFFECT_1]->BasePoints);

                        if (AuraEffect* aurEff = caster->GetAuraEffect(115636, EFFECT_0))
                            if (aurEff->GetOldBaseAmount())
                                AddPct(amount, aurEff->GetAmount());

                        if (AuraEffect* aurEff = caster->GetAuraEffect(213112, EFFECT_1))
                            AddPct(amount, aurEff->GetAmount());

                        if (Player* plr = caster->ToPlayer())
                            AddPct(amount, plr->GetFloatValue(PLAYER_FIELD_VERSATILITY) + plr->GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));
                    }
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Player* player = caster->ToPlayer();
                if (!player)
                    return;

                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode == AURA_REMOVE_BY_DEATH)
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(195266, EFFECT_0)) // Death Art
                    {
                        int32 cooldownMod = CalculatePct(GetSpellInfo()->Cooldowns.RecoveryTime, aurEff->GetAmount()) * -1;
                        player->ModifySpellCooldown(115080, cooldownMod);
                    }
                }
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_monk_touch_of_death_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_touch_of_death_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_touch_of_death_AuraScript();
        }
};

// Dampen Harm - 122278
class spell_monk_dampen_harm : public SpellScriptLoader
{
    public:
        spell_monk_dampen_harm() : SpellScriptLoader("spell_monk_dampen_harm") { }

        class spell_monk_dampen_harm_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_dampen_harm_AuraScript);

            uint32 pctMin = 0;
            uint32 pctMax = 0;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
                pctMin = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
                pctMax = GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
            {
                absorbAmount = 0;
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (dmgInfo.GetDamage() >= caster->CountPctFromMaxHealth(pctMax))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), pctMax);
                else if (dmgInfo.GetDamage() <= caster->CountPctFromMaxHealth(pctMin))
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), pctMin);
                else
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), dmgInfo.GetDamage() * 100 / caster->GetMaxHealth());
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_dampen_harm_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_dampen_harm_AuraScript::CalculateAmount, EFFECT_3, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_dampen_harm_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_dampen_harm_AuraScript::Absorb, EFFECT_3, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_dampen_harm_AuraScript();
        }
};

//Chi Wave - 132466
class spell_monk_chi_wave_filter : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave_filter() : SpellScriptLoader("spell_monk_chi_wave_filter") { }

        class spell_monk_chi_wave_filter_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_wave_filter_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove(GetCaster());
                targets.remove_if(OptionCheck(GetCaster()));
                if (!GetCaster()->IsFriendlyTo(GetOriginalCaster()))
                {
                    targets.remove_if(FriendlyToOriginalCaster(GetOriginalCaster(), GetSpellInfo()));
                    targets.sort(CheckHealthState());
                    if (targets.size() > 1)
                        targets.resize(1);
                }
                else
                {
                    targets.remove_if(OptionCheck(GetOriginalCaster()));
                    targets.sort(CheckNearbyVictim(GetCaster()));
                    if (targets.size() > 1)
                        targets.resize(1);
                }
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                float bp1 = GetEffectValue();

                if(bp1 >= 7)
                    return;

                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                Unit* origCaster = GetOriginalCaster();
                if (target && caster && origCaster)
                {
                    bp1++;
                    if (target->IsFriendlyTo(origCaster))
                        caster->CastCustomSpell(target, 132464, NULL, &bp1, NULL, true, NULL, NULL, origCaster->GetGUID());
                    else
                        caster->CastCustomSpell(target, 132467, NULL, &bp1, NULL, true, NULL, NULL, origCaster->GetGUID());
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_monk_chi_wave_filter_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_monk_chi_wave_filter_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
            }
        private:
            class OptionCheck
            {
                public:
                    OptionCheck(Unit* caster) : _caster(caster) {}

                    Unit* _caster;

                    bool operator()(WorldObject* unit)
                    {
                        if(!unit->ToUnit())
                            return true;
                        if(!_caster->IsValidAttackTarget(unit->ToUnit()))
                            return true;
                        return false;
                    }
            };
            class FriendlyToOriginalCaster
            {
            public:
                FriendlyToOriginalCaster(Unit* caster, SpellInfo const* spellInfo) : _caster(caster), _spellInfo(spellInfo){}

                Unit* _caster;
                SpellInfo const* _spellInfo;

                bool operator()(WorldObject* unit)
                {
                    Unit* victim = unit->ToUnit();
                    if (!victim)
                        return true;
                    if (!_caster->IsFriendlyTo(victim) || !_caster->_IsValidAssistTarget(victim, _spellInfo))
                        return true;
                    return false;
                }
            };
            class CheckHealthState
            {
                public:
                    CheckHealthState() { }

                    bool operator() (WorldObject* a, WorldObject* b) const
                    {
                        Unit* unita = a->ToUnit();
                        Unit* unitb = b->ToUnit();
                        if(!unita)
                            return true;
                        if(!unitb)
                            return false;
                        return unita->GetHealthPct() < unitb->GetHealthPct();
                    }
            };
            class CheckNearbyVictim
            {
            public:
                CheckNearbyVictim(Unit* caster) : _caster(caster) { }

                Unit* _caster;

                bool operator() (WorldObject* a, WorldObject* b) const
                {
                    Unit* unita = a->ToUnit();
                    Unit* unitb = b->ToUnit();
                    if (!unita)
                        return true;
                    if (!unitb)
                        return false;

                    Position posA;
                    Position posB;
                    unita->GetPosition(&posA);
                    unitb->GetPosition(&posB);
                    float distA = _caster->GetDistance(posA);
                    float distB = _caster->GetDistance(posB);

                    return distA < distB;
                }
            };
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_chi_wave_filter_SpellScript();
        }
};

// Chi Wave - 132464, 132467
class spell_monk_chi_wave_dummy : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave_dummy() : SpellScriptLoader("spell_monk_chi_wave_dummy") { }

        class spell_monk_chi_wave_dummy_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_chi_wave_dummy_AuraScript);

            void ApplyDummy(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetTarget();
                if (target && caster)
                {
                    float bp1 = aurEff->GetAmount();
                    target->CastCustomSpell(target, 132466, NULL, &bp1, NULL, true, NULL, NULL, GetCasterGUID());
                    if (target->IsFriendlyTo(caster))
                        caster->CastCustomSpell(target, 132463, NULL, &bp1, NULL, true, NULL, NULL, GetCasterGUID());
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_monk_chi_wave_dummy_AuraScript::ApplyDummy, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_chi_wave_dummy_AuraScript();
        }
};

// Chi Wave - 115098
class spell_monk_chi_wave : public SpellScriptLoader
{
    public:
        spell_monk_chi_wave() : SpellScriptLoader("spell_monk_chi_wave") { }

        class spell_monk_chi_wave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_chi_wave_SpellScript);

            void HandleBeforeCast()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetExplTargetUnit())
                    {
                        uint32 spellId = 0;
                        float bp = 1.f;

                        if (target->IsFriendlyTo(caster))
                        {
                            spellId = 132464;

                            if (!caster->_IsValidAssistTarget(target, GetSpellInfo()))
                                target = caster;
                        }
                        else
                        {
                            if (!caster->IsValidAttackTarget(target))
                            {
                                spellId = 132464;
                                target = caster;
                            }
                            else
                                spellId = 132467;
                        }

                        caster->CastCustomSpell(target, spellId, NULL, &bp, NULL, true, NULL, NULL, caster->GetGUID());
                    }
                }
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_monk_chi_wave_SpellScript::HandleBeforeCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_chi_wave_SpellScript();
        }
};

// Disable - 116095
class spell_monk_disable : public SpellScriptLoader
{
    public:
        spell_monk_disable() : SpellScriptLoader("spell_monk_disable") { }

        class spell_monk_disable_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_disable_AuraScript);

            void CalculateMaxDuration(int32& duration)
            {
                if (Unit* caster = GetCaster())
                    if(Unit* target = GetUnitOwner())
                    {
                        if(target->ToPlayer())
                            duration = 8000;
                        else
                            duration = 15000;
                    }
            }

            void Register() override
            {
                DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_monk_disable_AuraScript::CalculateMaxDuration);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_disable_AuraScript();
        }
};

// Purified Healing - 145056
class spell_monk_purified_healing : public SpellScriptLoader
{
    public:
        spell_monk_purified_healing() : SpellScriptLoader("spell_monk_purified_healing") { }

        class spell_monk_purified_healing_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_purified_healing_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Aura* bonusAura = caster->GetAura(145055)) // Item - Monk T16 Brewmaster 4P Bonus
                    {
                        int32 perc = bonusAura->GetEffect(EFFECT_0)->GetAmount();
                        int32 _amount = 0;
                        if (Aura* staggerAura = caster->GetAura(124275))
                            if(AuraEffect* eff = staggerAura->GetEffect(EFFECT_1))
                                _amount += eff->GetAmount();
                        if (Aura* staggerAura = caster->GetAura(124274))
                            if(AuraEffect* eff = staggerAura->GetEffect(EFFECT_1))
                                _amount += eff->GetAmount();
                        if (Aura* staggerAura = caster->GetAura(124273))
                            if(AuraEffect* eff = staggerAura->GetEffect(EFFECT_1))
                                _amount += eff->GetAmount();
                        if(_amount)
                            SetHitHeal(CalculatePct(_amount, perc));
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_purified_healing_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_purified_healing_SpellScript();
        }
};

// Hurricane Strike - 152175
class spell_monk_hurricane_strike : public SpellScriptLoader
{
    public:
        spell_monk_hurricane_strike() : SpellScriptLoader("spell_monk_hurricane_strike") { }

        class spell_monk_hurricane_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_hurricane_strike_AuraScript);

            uint32 update = 0;

            void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), 158221, true);
            }

            void OnUpdate(uint32 diff, AuraEffect* aurEff)
            {
                update += diff;

                if (update >= 140)
                {
                    if (Unit* caster = GetCaster())
                        caster->CastSpell(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), 158221, true);
                    update = 0;
                }
            }

            void Register() override
            {
                OnEffectApply += AuraEffectApplyFn(spell_monk_hurricane_strike_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_ALLOW_ONLY_ABILITY, AURA_EFFECT_HANDLE_REAL);
                OnEffectUpdate += AuraEffectUpdateFn(spell_monk_hurricane_strike_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_ALLOW_ONLY_ABILITY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_hurricane_strike_AuraScript();
        }
};

// Whirling Dragon Punch - 152175, active by 107428, 113656
class spell_monk_whirling_dragon_punch_activater : public SpellScriptLoader
{
    public:
        spell_monk_whirling_dragon_punch_activater() : SpellScriptLoader("spell_monk_whirling_dragon_punch_activater") { }

        class spell_monk_whirling_dragon_punch_activater_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_whirling_dragon_punch_activater_SpellScript)

            void HandleAfterCast()
            {
                if (Player* _plr = GetCaster()->ToPlayer())
                {
                    uint32 cooldown1 = _plr->GetSpellCooldownDelay(113656);
                    uint32 cooldown2 = _plr->GetChargesCooldown(107428);
                    if (cooldown1 && cooldown2)
                        _plr->CastSpell(_plr, 196742, true);
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_monk_whirling_dragon_punch_activater_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_whirling_dragon_punch_activater_SpellScript();
        }
};

// Whirling Dragon Punch - 196742
class spell_monk_whirling_dragon_punch_activated : public SpellScriptLoader
{
public:
    spell_monk_whirling_dragon_punch_activated() : SpellScriptLoader("spell_monk_whirling_dragon_punch_activated") { }

    class spell_monk_whirling_dragon_punch_activated_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_whirling_dragon_punch_activated_AuraScript);

        void CalculateMaxDuration(int32 & duration)
        {
            duration = 0; // If bugs
            Unit* caster = GetCaster();
            if(!caster)
                return;

            if (Player* _player = caster->ToPlayer())
            {
                duration = _player->GetSpellCooldownDelay(113656) * 1000;
                int32 cooldown = _player->GetChargesCooldown(107428);
                if (duration > cooldown)
                    duration = cooldown;
            }
        }

        void Register() override
        {
            DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_monk_whirling_dragon_punch_activated_AuraScript::CalculateMaxDuration);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_whirling_dragon_punch_activated_AuraScript();
    }
};

// Expel Harm - 115072
class spell_monk_expel_harm : public SpellScriptLoader
{
    public:
        spell_monk_expel_harm() : SpellScriptLoader("spell_monk_expel_harm") { }

        class spell_monk_expel_harm_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_expel_harm_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    int32 countSphere = 0;
                    std::list<AreaTrigger*> list;
                    std::vector<uint32> spellIdList = {124503, 124506, 213458, 213460};
                    caster->GetAreaObjectList(list, spellIdList); // Gift of the Ox

                    if (!list.empty())
                    {
                        for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
                        {
                            if (AreaTrigger* areaObj = (*itr))
                                if (caster->GetDistance(areaObj) <= 30.0f)
                                {
                                    areaObj->CastAction();
                                    countSphere++;
                                }
                        }
                        if (countSphere)
                        {
                            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(124507)) // TODO: need take different spellinfo healing spells (example: for 213458/213460 - get spellinfo (213464) , for 124503/124506 - 124507) 
                            {
                                float heal_ = spellInfo->Effects[EFFECT_0]->CalcValue(caster);
                                heal_ = caster->SpellHealingBonusDone(caster, spellInfo, heal_, HEAL, EFFECT_0);
                                heal_ = caster->SpellHealingBonusTaken(caster, spellInfo, heal_, HEAL, EFFECT_0);
                                float bp = CalculatePct(heal_ * countSphere, 10);
                                caster->CastCustomSpell(caster, 115129, &bp, NULL, NULL, true);
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_monk_expel_harm_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_expel_harm_SpellScript();
        }
};

// Zen Pulse - 198487
class spell_monk_zen_pulse : public SpellScriptLoader
{
    public:
        spell_monk_zen_pulse() : SpellScriptLoader("spell_monk_zen_pulse") { }

        class spell_monk_zen_pulse_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_zen_pulse_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (GetSpell()->GetTargetParentCount() > 2)
                    SetHitHeal(GetHitHeal() * (GetSpell()->GetTargetParentCount() - 1));
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_zen_pulse_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_zen_pulse_SpellScript();
        }
};

// Sheilun's Gift - 205406
class spell_monk_sheiluns_gift  : public SpellScriptLoader
{
    public:
        spell_monk_sheiluns_gift() : SpellScriptLoader("spell_monk_sheiluns_gift") { }

        class spell_monk_sheiluns_gift_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_sheiluns_gift_SpellScript);

            void HandleHeal(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<AreaTrigger*> list;
                    caster->GetAreaObjectList(list, 214501);
                    SetHitHeal(GetHitHeal() * list.size());
                    if(!list.empty())
                    {
                        for (std::list<AreaTrigger*>::iterator itr = list.begin(); itr != list.end(); ++itr)
                        {
                            if(AreaTrigger* areaObj = (*itr))
                                areaObj->Despawn();
                        }

                        if (caster->HasAura(238130)) // Whispers of Shaohao
                        {
                            float bp0 = list.size();
                            caster->CastCustomSpell(caster, 242400, &bp0, NULL, NULL, true);
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_monk_sheiluns_gift_SpellScript::HandleHeal, EFFECT_0, SPELL_EFFECT_HEAL);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_sheiluns_gift_SpellScript();
        }
};

// Ironskin Brew - 115308
class spell_monk_ironskin_brew_aura : public SpellScript
{
    PrepareSpellScript(spell_monk_ironskin_brew_aura);

    void HandleOnHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (AuraEffect const* aurEff = caster->GetAuraEffect(238129, EFFECT_1)) // Quick Sip
            {
                if (AuraEffect const* stagger = caster->GetAuraEffect(124255, EFFECT_1))
                {
                    int32 staggerNerf = CalculatePct(stagger->GetAmount(), aurEff->GetAmount());
                    if (staggerNerf)
                        caster->CalcStaggerDamage(staggerNerf, SPELL_SCHOOL_MASK_NORMAL, stagger->GetSpellInfo());
                }
            }

            if (AuraEffect const* aurEff = caster->GetAuraEffect(228563, EFFECT_3)) // Blackout Combo
            {
                if (Aura* auraBleed = caster->GetAura(124255))
                {
                    auraBleed->ModCustomData(aurEff->GetAmount() * 2); // one tick 500ms

                    if (Aura* aura = caster->GetAura(124275))
                        aura->SetDuration(aura->GetDuration() + aurEff->GetAmount() * 1000);
                    if (Aura* aura = caster->GetAura(124274))
                        aura->SetDuration(aura->GetDuration() + aurEff->GetAmount() * 1000);
                    if (Aura* aura = caster->GetAura(124273))
                        aura->SetDuration(aura->GetDuration() + aurEff->GetAmount() * 1000);
                }
            }

            caster->CastSpell(caster, 215479, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_monk_ironskin_brew_aura::HandleOnHit);
    }
};

// Ironskin Brew - 215479 (aura)
class spell_monk_ib : public AuraScript
{
    PrepareAuraScript(spell_monk_ib);

    void CalculateMaxDuration(int32& duration)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aura = caster->GetAura(GetSpellInfo()->Id))
            {
                uint32 dur = aura->GetDuration() + duration;
                uint32 limitdur = duration * 3; // base aura duration
                if (caster->HasAura(213047)) // custom duration cap shift
                    limitdur += duration;

                if (dur > limitdur)
                    dur = limitdur;

                duration = dur;
            }
        }
    }

    void Register() override
    {
        DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_monk_ib::CalculateMaxDuration);
    }
};

// Gift of the Ox - 124502
class spell_monk_gift_of_the_ox : public SpellScriptLoader
{
public:
    spell_monk_gift_of_the_ox() : SpellScriptLoader("spell_monk_gift_of_the_ox") { }

    class spell_monk_gift_of_the_ox_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_gift_of_the_ox_AuraScript);

        float chance = 0.0f;

        void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
        {
            if (Unit* caster = GetCaster())
            {
                float talentMod = 1.0f;
                float damage = float(eventInfo.GetDamageInfo()->GetDamage()/* - eventInfo.GetDamageInfo()->GetAbsorb()*/);

                if (AuraEffect const* aurEff = caster->GetAuraEffect(196719, EFFECT_0)) // Gift of the Mists
                    talentMod = 1.0f + ((aurEff->GetAmount() / 100.0f) * float((100.0f - caster->GetHealthPct()) / 100.0f));

                chance += (2 - ((caster->GetHealth() - damage) / caster->GetMaxHealth())) * talentMod;

                if (chance >= 100.0f)
                {
                    chance = chance - 100.0f;
                    AuraEffect const* aurEff2 = caster->GetAuraEffect(213180, EFFECT_0); // Overflow
                    if (aurEff2 && roll_chance_i(aurEff2->GetAmount()))
                    {
                        caster->CastSpell(caster, 213458, true); // AT
                        caster->CastSpell(caster, 213460, true); // AT
                    }
                    else
                    {
                        caster->CastSpell(caster, 124503, true); // AT
                        caster->CastSpell(caster, 124506, true); // AT
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectProc += AuraEffectProcFn(spell_monk_gift_of_the_ox_AuraScript::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_gift_of_the_ox_AuraScript();
    }
};

// Guard (PvP talent) - 202162
class spell_monk_guard : public SpellScriptLoader
{
    public:
        spell_monk_guard() : SpellScriptLoader("spell_monk_guard") { }

        class spell_monk_guard_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_guard_AuraScript);

            uint32 pctAbsorb = 0;

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
                pctAbsorb = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
            }

            void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                if (Unit* caster = GetCaster())
                {
                    absorbAmount = CalculatePct(dmgInfo.GetDamage(), pctAbsorb);
                    caster->CalcStaggerDamage(absorbAmount, dmgInfo.GetSchoolMask(), dmgInfo.GetSpellInfo(), true);
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_guard_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_guard_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_guard_AuraScript();
        }
};

// Enveloping Mist - 124682, 227345
class spell_monk_enveloping_mist : public SpellScriptLoader
{
    public:
        spell_monk_enveloping_mist() : SpellScriptLoader("spell_monk_enveloping_mist") { }

        class spell_monk_enveloping_mist_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_enveloping_mist_AuraScript);

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target)
                    return;

                Unit* caster = GetCaster();
                if(!caster)
                    return;

                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode == AURA_REMOVE_BY_ENEMY_SPELL)
                {
                    if(AuraEffect const* dome = caster->GetAuraEffect(202577, EFFECT_0)) // Dome of Mist (Honor Talent)
                    {
                        int32 duration = aurEff->GetBase()->GetDuration();
                        int32 amplitude = aurEff->GetPeriod();
                        if(!duration || !amplitude)
                            return;
                        float bp0 = CalculatePct(aurEff->GetAmount() * uint32(duration / amplitude), dome->GetAmount());
                        caster->CastCustomSpell(target, 205655, &bp0, NULL, NULL, true);
                    }
                }
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_monk_enveloping_mist_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_enveloping_mist_AuraScript();
        }

        class spell_monk_enveloping_mist_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_enveloping_mist_SpellScript);

            void FilterTargets(WorldObject*& target)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Spell* spell = caster->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    {
                        if (spell->GetSpellInfo()->Id == 209525)
                        {
                            if (WorldObject* _target = spell->m_targets.GetObjectTarget())
                                target = _target;
                        }
                    }
                }
            }

            void Register() override
            {
                OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_monk_enveloping_mist_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ALLY);
                OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_monk_enveloping_mist_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_TARGET_ALLY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_enveloping_mist_SpellScript();
        }
};

// Dark Side of the Moon - 213062, 227688, 227689, 227690, 227691, 227692, 239306, 239307
class spell_monk_dark_side_of_the_moon : public SpellScriptLoader
{
    public:
        spell_monk_dark_side_of_the_moon() : SpellScriptLoader("spell_monk_dark_side_of_the_moon") { }

        class spell_monk_dark_side_of_the_moon_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_dark_side_of_the_moon_AuraScript);

            void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
            {
                amount = -1;
            }

            void Absorb(AuraEffect* /*aurEff*/, DamageInfo & dmgInfo, float & absorbAmount)
            {
                absorbAmount = 0;
                Unit* victim = dmgInfo.GetAttacker();
                Unit* caster = GetCaster();
                if (victim && caster)
                {
                    if (AuraEffect const* aurEff = victim->GetAuraEffect(213063, EFFECT_0, caster->GetGUID()))
                    {
                        absorbAmount = CalculatePct(dmgInfo.GetDamage(), aurEff->GetAmount());
                        aurEff->GetBase()->Remove();
                    }
                }
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_dark_side_of_the_moon_AuraScript::CalculateAmount, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_monk_dark_side_of_the_moon_AuraScript::Absorb, EFFECT_1, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_dark_side_of_the_moon_AuraScript();
        }
};

// Thunder Focus Tea - 116680
class spell_monk_thunder_focus_tea : public AuraScript
{
    PrepareAuraScript(spell_monk_thunder_focus_tea);

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Aura* aura = GetAura())
            aura->SetMaxStackAmount();

        if (Unit* caster = GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                for (auto itr : plr->GetSpellModList(SPELLMOD_DAMAGE))
                {
                    if (itr->spellId == 216992)
                    {
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(216992))
                            itr->value = 0.f;

                        return;
                    }
                }
            }
        }
    }

    void RemoveEff(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Player* plr = caster->ToPlayer())
            {
                for (auto itr : plr->GetSpellModList(SPELLMOD_DAMAGE))
                {
                    if (itr->spellId == 216992)
                    {
                        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(216992))
                            itr->value = spellInfo->Effects[EFFECT_0]->BasePoints;

                        return;
                    }
                }
            }
        }
    }

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        if (Spell* spell = eventInfo.GetSpell())
        {
            if (spell->GetTriggeredAuraEff())
                PreventDefaultAction();
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_thunder_focus_tea::HandleApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_monk_thunder_focus_tea::RemoveEff, EFFECT_1, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        OnEffectProc += AuraEffectProcFn(spell_monk_thunder_focus_tea::OnProc, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER);
    }
};

// Essence Font & Thunder Focus Tea - 191837
class spell_monk_essence_font : public SpellScriptLoader
{
    public:
    spell_monk_essence_font() : SpellScriptLoader("spell_monk_essence_font") {}

    class spell_monk_essence_font_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_essence_font_SpellScript);

        void HandleOnCast()
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasAura(217000))
                {
                    caster->RemoveAurasDueToSpell(217000);
                }
                else if (Aura* aura = caster->GetAura(116680))
                {
                    aura->DropCharge();
                }
            }
        }

        void Register() override
        {
            OnCast += SpellCastFn(spell_monk_essence_font_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_monk_essence_font_SpellScript();
    }

    class spell_monk_essence_font_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_essence_font_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (Unit* caster = GetCaster())
            {
                if (!caster->HasAura(116680))
                {
                    caster->RemoveAurasDueToSpell(197218);
                }
            }
        }

        void Register() override
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_monk_essence_font_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_essence_font_AuraScript();
    }
};

// Rising Sun Kick - 185099
class spell_monk_rising_sun_kick : public SpellScriptLoader
{
    public:
        spell_monk_rising_sun_kick() : SpellScriptLoader("spell_monk_rising_sun_kick") { }

        class spell_monk_rising_sun_kick_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_rising_sun_kick_SpellScript);

            void HandleOnHit()
            {
                if (AuraEffect const* trigerEff = GetTriggeredAuraEff())
                {
                    if (Unit* caster = GetCaster())
                    {
                        SetHitDamage(CalculatePct(GetHitDamage(), trigerEff->GetAmount()));
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_monk_rising_sun_kick_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_rising_sun_kick_SpellScript();
        }
};

// Effuse- 227344
class spell_monk_effuse : public SpellScriptLoader
{
public:
    spell_monk_effuse() : SpellScriptLoader("spell_monk_effuse") {}

    class spell_monk_effuse_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_effuse_SpellScript);

        void FilterTargets(WorldObject*& target)
        {
            if (Unit* caster = GetCaster())
            {
                if (Spell* spell = caster->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                {
                    if (spell->GetSpellInfo()->Id == 209525)
                    {
                        if (WorldObject* _target = spell->m_targets.GetObjectTarget())
                            target = _target;
                    }
                }
            }
        }

        void Register() override
        {
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_monk_effuse_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ALLY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_monk_effuse_SpellScript();
    }
};

// Renewing Mist - 115151
class spell_monk_renewing_mist_main : public SpellScriptLoader
{
    public:
    spell_monk_renewing_mist_main() : SpellScriptLoader("spell_monk_renewing_mist_main") {}

    class spell_monk_renewing_mist_main_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_renewing_mist_main_SpellScript);

        void HandleBeforeCast()
        {
            if (Unit* caster = GetOriginalCaster())
            {
                if (Unit* target = GetExplTargetUnit())
                {
                    if (AuraEffect const* aurEff2 = caster->GetAuraEffect(199573, EFFECT_0)) // Dancing Mists
                    {
                        if (roll_chance_f(aurEff2->GetAmount()))
                        {
                            if (Player* plr = caster->ToPlayer())
                            {
                                if (Group* group = plr->GetGroup())
                                {
                                    std::vector<Player*> tempList;
                                    bool findTarget = false;

                                    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                                    {
                                        if (Player* player = itr->getSource())
                                        {
                                            if (player->GetGUID() == target->GetGUID())
                                                continue;

                                            if (!player->IsInWorld() || !player->isAlive())
                                                continue;

                                            if (!player->IsWithinDistInMap(target, 50.f))
                                                continue;

                                            if (!caster->IsValidAssistTarget(player))
                                                continue;

                                            if (player->HasAura(119611, caster->GetGUID()))
                                            {
                                                tempList.push_back(player);
                                                continue;
                                            }

                                            caster->CastSpell(player, 119611, true);
                                            findTarget = true;
                                            break;
                                        }
                                    }

                                    if (!findTarget && !tempList.empty())
                                    {
                                        caster->CastSpell(tempList.front(), 119611, true);
                                    }
                                }
                            }
                        }
                    }

                    if (GetTriggeredAuraEff())
                        return;

                    caster->CastSpell(target, 119611, true);
                }
            }
        }

        void Register() override
        {
            BeforeCast += SpellCastFn(spell_monk_renewing_mist_main_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_monk_renewing_mist_main_SpellScript();
    }
};

// Renewing Mist - 119611
class spell_monk_renewing_mist : public SpellScriptLoader
{
    public:
        spell_monk_renewing_mist() : SpellScriptLoader("spell_monk_renewing_mist") { }

        class spell_monk_renewing_mist_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_renewing_mist_AuraScript);

            bool JumpOnTarget(Unit* caster, Player* target, Unit* monk, AuraEffect const* aurEff)
            {
                if (!target->HasAura(119611, monk->GetGUID()))
                {
                    if (Aura* aura = GetAura())
                    {
                        monk->CastSpellDuration(target, 119611, true, aura->GetDuration(), 0, 0, aurEff);
                        aura->Remove();
                    }

                    caster->CastSpell(target, 115151, true, NULL, aurEff, monk->GetGUID());
                    return true;
                }
                return false;
            }

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Player* plr = caster->ToPlayer())
                    {
                        if (Unit* target = GetUnitOwner())
                        {
                            if (!target->IsFullHealth())
                                return;

                            if (Group* group = plr->GetGroup())
                            {
                                for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
                                {
                                    if (Player* player = itr->getSource())
                                    {
                                        if (player->GetGUID() == target->GetGUID())
                                            continue;

                                        if (!player->IsInWorld() || !player->isAlive())
                                            continue;

                                        if (player->IsFullHealth())
                                            continue;

                                        if (!player->IsWithinDistInMap(target, 20.f))
                                            continue;

                                        if (!caster->IsValidAssistTarget(player))
                                            continue;

                                        if (JumpOnTarget(target, player, caster, aurEff))
                                            return;
                                    }
                                }
                            }
                            else if (!caster->IsFullHealth())
                            {
                                JumpOnTarget(target, plr, caster, aurEff);
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_renewing_mist_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_renewing_mist_AuraScript();
        }
};

// Soothing Mist - 115175, 209525, 198533
class spell_monk_soothing_mist : public SpellScriptLoader
{
public:
    spell_monk_soothing_mist() : SpellScriptLoader("spell_monk_soothing_mist") {}

    class spell_monk_soothing_mist_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_soothing_mist_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (caster->GetGUID() == target->GetGUID())
                        return;

                    caster->CastSpell(target, 117899, true);
                }
            }
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetId() == 198533)
                return;

            if (Unit* caster = GetCaster())
            {
                if (Unit* target = GetUnitOwner())
                {
                    if (GuidList* summonList = caster->GetSummonList(60849))
                    {
                        for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                        {
                            if (Unit* summon = ObjectAccessor::GetUnit(*caster, (*iter)))
                            {
                                if (summon->IsWithinLOSInMap(target))
                                    summon->CastSpell(target, 198533, true);
                            }
                        }
                    }
                }
            }
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (auto caster = GetCaster())
            {
                if (auto summons = caster->GetSummonList(60849))
                {
                    for (GuidList::const_iterator itr = summons->begin(); itr != summons->end(); ++itr)
                    {
                        if (Unit* statue = ObjectAccessor::GetUnit(*caster, (*itr)))
                            statue->InterruptSpell(CURRENT_CHANNELED_SPELL);
                    }
                }
            }
        }

        void Register() override
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_soothing_mist_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_HEAL);
            OnEffectApply += AuraEffectApplyFn(spell_monk_soothing_mist_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnEffectRemove += AuraEffectRemoveFn(spell_monk_soothing_mist_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_HEAL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_monk_soothing_mist_AuraScript();
    }
};

// Chi Orbit - 196743
class spell_monk_chi_orbit : public SpellScriptLoader
{
    public:
        spell_monk_chi_orbit() : SpellScriptLoader("spell_monk_chi_orbit") { }

        class spell_monk_chi_orbit_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_monk_chi_orbit_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if(Unit* caster = GetCaster())
                {
                    if (!caster->HasAura(196744))
                    {
                        caster->CastSpell(caster, 196744, true, NULL, aurEff);
						caster->RemoveAurasDueToSpell(196744);
                        return;
                    }
                    if (!caster->HasAura(196745))
                    {
                        caster->CastSpell(caster, 196745, true, NULL, aurEff);
						caster->RemoveAurasDueToSpell(196745);
                        return;
                    }
                    if (!caster->HasAura(196746))
                    {
                        caster->CastSpell(caster, 196746, true, NULL, aurEff);
						caster->RemoveAurasDueToSpell(196746);
                        return;
                    }
                    if (!caster->HasAura(196747))
                    {
                        caster->CastSpell(caster, 196747, true, NULL, aurEff);
						caster->RemoveAurasDueToSpell(196747);
                        return;
                    }
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_chi_orbit_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_monk_chi_orbit_AuraScript();
        }
};

// 121253 - Keg Smash
class spell_monk_keg_smash : public SpellScriptLoader
{
    public:
        spell_monk_keg_smash() : SpellScriptLoader("spell_monk_keg_smash") { }

        class spell_monk_keg_smash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_keg_smash_SpellScript)

            void HandleBeforeCast()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                int32 cd = GetSpellInfo()->Effects[EFFECT_3]->CalcValue(caster) * -1000;
                if (AuraEffect* aurEff = caster->GetAuraEffect(228563, EFFECT_2))
                    cd += aurEff->GetAmount() * -1000;

                if (Player* plr = caster->ToPlayer())
                {
                    plr->ModifySpellCooldown(115203, cd);
                    plr->ModifySpellCooldown(115399, cd);
                    plr->ModSpellChargeCooldown(119582, -cd);
                }
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_monk_keg_smash_SpellScript::HandleBeforeCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_keg_smash_SpellScript();
        }
};

// 124507, 213464
class spell_monk_gift_of_the_ox_heal : public SpellScriptLoader
{
    public:
        spell_monk_gift_of_the_ox_heal() : SpellScriptLoader("spell_monk_gift_of_the_ox_heal") { }

        class spell_monk_gift_of_the_ox_heal_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_gift_of_the_ox_heal_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (AuraEffect const* aurEff = caster->GetAuraEffect(124255, EFFECT_1))
                    {
                        if (AuraEffect const* t20 = caster->GetAuraEffect(242256, EFFECT_0)) // Item - Monk T20 Brewmaster 4P Bonus
                        {
                            int32 staggerNerf = CalculatePct(aurEff->GetAmount(), t20->GetAmount());
                            if (staggerNerf)
                                caster->CalcStaggerDamage(staggerNerf, SPELL_SCHOOL_MASK_NORMAL, aurEff->GetSpellInfo());
                        }
                    }
                }
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_monk_gift_of_the_ox_heal_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_gift_of_the_ox_heal_SpellScript();
        }
};

// Strike of the Windlord - 222029, 205414
class spell_monk_sotw : public SpellScript
{
    PrepareSpellScript(spell_monk_sotw);

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Player* plr = GetCaster()->ToPlayer())
        {
            if (plr->GetSelectedUnit() != GetHitUnit())
            {
                uint8 count = GetSpell()->GetTargetCount();
                int32 damage = GetHitDamage();
                if (count)
                    damage /= count;

                SetHitDamage(damage);
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_monk_sotw::HandleOnHit, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
    }
};

//115176
class spell_monk_zen_meditation : public AuraScript
{
    PrepareAuraScript(spell_monk_zen_meditation);

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        if (!GetTarget())
            return;

        if (GetTarget()->HasAura(208878) || GetTarget()->HasAura(202200))
            PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_zen_meditation::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Mastery: Gust of Mists - 117907
class spell_monk_gust_of_mists : public AuraScript
{
    PrepareAuraScript(spell_monk_gust_of_mists);

    enum MyEnum
    {
        GustOfMists     = 117907,
        GustOfMistsHeal = 191894
    };

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        if (DamageInfo* dmgInfo = eventInfo.GetDamageInfo())
        {
            if (Unit* caster = dmgInfo->GetAttacker())
            {
                if (caster->HasSpellCooldown(GustOfMists))
                    return;

                if (Spell* spell = eventInfo.GetSpell())
                {
                    if (spell->GetTriggeredAuraEff())
                    {
                        PreventDefaultAction();
                        return;
                    }
                }

                if (Unit* target = dmgInfo->GetVictim())
                {
                    if (target->HasAura(191840, caster->GetGUID()))
                        caster->CastSpell(target, GustOfMistsHeal, true);

                    caster->CastSpell(target, GustOfMistsHeal, true);

                    if (G3D::fuzzyGt(0.1, 0.0))
                        caster->AddSpellCooldown(GustOfMists, 0, getPreciseTime() + 0.1);
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_gust_of_mists::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Life Cocoon - 116849
class spell_monk_life_cocoon : public AuraScript
{
    PrepareAuraScript(spell_monk_life_cocoon);

    void HandleApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetUnitOwner())
            {
                if (!caster->HasAura(199563)) // Mists of Life
                    return;

                uint32 EnvelopingMist = caster->HasAura(209520) ? 227345 : 124682; // Ancient Mistweaver Arts
                caster->CastSpell(target, EnvelopingMist, true, nullptr, aurEff);
                caster->CastSpell(target, 119611, true, nullptr, aurEff);
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_monk_life_cocoon::HandleApply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// Celestial Fortune - 216519
class spell_monk_celestial_fortune : public AuraScript
{
    PrepareAuraScript(spell_monk_celestial_fortune);

    enum MyEnum
    {
        CelestialFortuneHeal = 216521
    };

    float critPct = 0.f;

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (DamageInfo* dmgInfo = eventInfo.GetDamageInfo())
        {
            if (dmgInfo->GetSpellInfo()->Id == CelestialFortuneHeal)
                return;

            if (uint32 heal = dmgInfo->GetDamage())
            {
                if (!roll_chance_f(critPct))
                    return;

                if (Unit* monk = GetUnitOwner())
                {
                    if (Player* plr = monk->ToPlayer())
                    {
                        float finalHeal = CalculatePct(heal, aurEff->GetAmount());
                        monk->CastCustomSpell(monk, CelestialFortuneHeal, &finalHeal, NULL, NULL, true);
                    }
                }
            }
        }
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & canBeRecalculated)
    {
        if (Unit* monk = GetUnitOwner())
        {
            if (Player* plr = monk->ToPlayer())
                amount = critPct = plr->GetFloatValue(PLAYER_FIELD_CRIT_PERCENTAGE);
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_monk_celestial_fortune::CalculateAmount, EFFECT_1, SPELL_AURA_MOD_ABSORBTION_PERCENT);
        OnEffectProc += AuraEffectProcFn(spell_monk_celestial_fortune::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

//115313
class spell_monk_summon_jade_serpent_statue : public SpellScriptLoader
{
    public:
        spell_monk_summon_jade_serpent_statue() : SpellScriptLoader("spell_monk_summon_jade_serpent_statue") { }

        class spell_monk_summon_jade_serpent_statue_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_monk_summon_jade_serpent_statue_SpellScript);

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                {
                    GuidList* summonList = caster->GetSummonList(60849);
                    for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                    {
                        if (Creature* summon = ObjectAccessor::GetCreature(*caster, (*iter)))
                            summon->DespawnOrUnsummon(500);
                    }
                }
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_monk_summon_jade_serpent_statue_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_monk_summon_jade_serpent_statue_SpellScript();
        }
};

// 206902 - Petrichor Lagniappe
class spell_monk_petrichor_lagniappe : public AuraScript
{
    PrepareAuraScript(spell_monk_petrichor_lagniappe);

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        if (DamageInfo* dmgInfo = eventInfo.GetDamageInfo())
        {
            if (Unit* caster = dmgInfo->GetAttacker())
            {
                if (Spell* spell = eventInfo.GetSpell())
                {
                    if (spell->GetTriggeredAuraEff())
                    {
                        PreventDefaultAction();
                        return;
                    }
                }

                if (Unit* target = dmgInfo->GetVictim())
                {
                    if (Player* plr = caster->ToPlayer())
                        plr->ModifySpellCooldown(115310, aurEff->GetAmount());
                }
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_petrichor_lagniappe::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 247483 - Tigereye Brew (Honor Talent)
class spell_monk_tigereye_brew : public AuraScript
{
    PrepareAuraScript(spell_monk_tigereye_brew);

    void CalculateMaxDuration(int32& duration)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aur = caster->GetAura(248646))
            {
                uint8 stackAmount = aur->GetStackAmount();
                int8 stackCap = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
                if (stackAmount > stackCap)
                    stackAmount = stackCap;
                duration *= stackAmount;
            }
        }
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Aura* aur = caster->GetAura(248646))
            {
                int8 stackCap = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
                aur->ModStackAmount(-stackCap);
            }
        }
    }

    void Register() override
    {
        DoCalcMaxDuration += AuraCalcMaxDurationFn(spell_monk_tigereye_brew::CalculateMaxDuration);
        AfterEffectApply += AuraEffectApplyFn(spell_monk_tigereye_brew::HandleApply, EFFECT_0, SPELL_AURA_220, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

// 115181 - Breath of Fire
class spell_monk_breath_of_fire : public SpellScript
{
    PrepareSpellScript(spell_monk_breath_of_fire);

    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(213183)) // Dragonfire Brew
            {
                caster->AddDelayedEvent(600, [caster]() -> void
                {
                    caster->CastSpell(caster, 227681, true);
                });
                caster->AddDelayedEvent(1200, [caster]() -> void
                {
                    caster->CastSpell(caster, 227681, true);
                });
            }
            if (caster->HasAura(202272)) // Incendiary Breath (Honor Talent)
                caster->CastSpellDelay(caster, 202274, true, 200);

            if (Player* plr = caster->ToPlayer())
            {
                if (AuraEffect const* aurEff = caster->GetAuraEffect(224489, EFFECT_0)) // Firestone Walker's Vintage Brew
                {
                    uint8 count = GetSpell()->GetTargetCount();
                    int32 cdmod = aurEff->GetAmount() * IN_MILLISECONDS;
                    if (count > 3)
                        count = 3;
                    plr->ModifySpellCooldown(115203, -cdmod * count);
                }
            }
        }
    }

    void HandleOnHit(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (caster->HasAura(117906)) // Mastery: Elusive Brawler
                    caster->CastSpell(caster, 195630, true);

                if (caster->HasAura(251829)) // Item - Monk T21 Brewmaster 2P Bonus
                    caster->CastSpell(caster, 195630, true);

                if (target->HasAura(121253, caster->GetGUID()) || target->HasAura(196733, caster->GetGUID()))
                    caster->CastSpell(target, 123725, true);
            }
        }
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_monk_breath_of_fire::HandleOnCast);
        OnEffectHitTarget += SpellEffectFn(spell_monk_breath_of_fire::HandleOnHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// 242255 - Item - Monk T20 Brewmaster 2P Bonus
class spell_monk_t20_brew_2p : public AuraScript
{
    PrepareAuraScript(spell_monk_t20_brew_2p);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& /*eventInfo*/)
    {
        if (Unit* caster = GetCaster())
        {
            AuraEffect const* aurEff2 = caster->GetAuraEffect(213180, EFFECT_0); // Overflow
            if (aurEff2 && roll_chance_i(aurEff2->GetAmount()))
            {
                caster->CastSpell(caster, 213458, true); // AT
                caster->CastSpell(caster, 213460, true); // AT
            }
            else
            {
                caster->CastSpell(caster, 124503, true); // AT
                caster->CastSpell(caster, 124506, true); // AT
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_monk_t20_brew_2p::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 221771 - Storm, earth, fire fixate
class spell_monk_fixate : public SpellScriptLoader
{
public:
	spell_monk_fixate() : SpellScriptLoader("spell_monk_fixate") { }

	class spell_monk_fixate_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_monk_fixate_SpellScript);

		void HandleOnCast()
		{
			if (Unit* caster = GetCaster())
				if (Player* player = caster->ToPlayer())
					if (player->HasAura(SPELL_MONK_SEF))
						if (Unit* target = player->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*player, *itr))
									if (creature->IsAIEnabled && !creature->IsDespawn() && (creature->GetEntry() == NPC_FIRE_SPIRIT || creature->GetEntry() == NPC_EARTH_SPIRIT))
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);
									}
		}

		void Register() override
		{
			OnCast += SpellCastFn(spell_monk_fixate_SpellScript::HandleOnCast);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new spell_monk_fixate_SpellScript();
	}
};

void AddSC_monk_spell_scripts()
{
    new spell_monk_clone_cast();
    new spell_monk_storm_earth_and_fire_clone_visual();
    new spell_monk_storm_earth_and_fire();
    new spell_monk_diffuse_magic();
    new spell_monk_zen_flight_check();
    new spell_monk_power_strikes();
    new spell_monk_touch_of_karma();
    new spell_monk_flying_serpent_kick();
    RegisterSpellScript(spell_monk_purifying_brew);
    new spell_monk_zen_pilgrimage();
    new spell_monk_zen_pilgrimage_return();
    new spell_monk_provoke();
    new spell_monk_roll();
    new spell_monk_remove_zen_flight();
    new spell_monk_transcendence();
    new spell_monk_transcendence_transfer();
    new spell_monk_touch_of_death();
    new spell_monk_dampen_harm();
    new spell_monk_chi_wave_filter();
    new spell_monk_chi_wave();
    new spell_monk_chi_wave_dummy();
    new spell_monk_disable();
    new spell_monk_purified_healing();
    new spell_monk_hurricane_strike();
    new spell_monk_whirling_dragon_punch_activater();
    new spell_monk_whirling_dragon_punch_activated();
    new spell_monk_expel_harm();
    new spell_monk_zen_pulse();
    new spell_monk_sheiluns_gift();
    RegisterSpellScript(spell_monk_ironskin_brew_aura);
    RegisterAuraScript(spell_monk_ib);
    new spell_monk_gift_of_the_ox();
    new spell_monk_guard();
    new spell_monk_enveloping_mist();
    new spell_monk_dark_side_of_the_moon();
    new spell_monk_rising_sun_kick();
    new spell_monk_renewing_mist();
    new spell_monk_chi_orbit();
    new spell_monk_soothing_mist();
    new spell_monk_effuse();
    new areatrigger_at_windwalking();
    new areatrigger_at_ring_of_peace();
    new spell_monk_keg_smash();
    new spell_monk_renewing_mist_main();
    new spell_monk_essence_font();
    new spell_monk_gift_of_the_ox_heal();
    RegisterSpellScript(spell_monk_sotw);
    new spell_monk_summon_jade_serpent_statue();
    RegisterAuraScript(spell_monk_zen_meditation);
    RegisterAuraScript(spell_monk_gust_of_mists);
    RegisterAuraScript(spell_monk_celestial_fortune);
    RegisterAuraScript(spell_monk_thunder_focus_tea);
    RegisterAuraScript(spell_monk_life_cocoon);
    RegisterAuraScript(spell_monk_petrichor_lagniappe);
    RegisterAuraScript(spell_monk_tigereye_brew);
    RegisterSpellScript(spell_monk_breath_of_fire);
    RegisterAuraScript(spell_monk_t20_brew_2p);
	new spell_monk_fixate();
}
