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
 * Scripts for spells with SPELLFAMILY_HUNTER, SPELLFAMILY_PET and SPELLFAMILY_GENERIC spells used by hunter players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_hun_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "GridNotifiers.h"
#include "AreaTriggerAI.h"
#include "AreaTrigger.h"

enum HunterSpells
{
    DIRE_BEAST_JADE_FOREST                       = 121118,
    DIRE_BEAST_KALIMDOR                          = 122802,
    DIRE_BEAST_EASTERN_KINGDOMS                  = 122804,
    DIRE_BEAST_OUTLAND                           = 122806,
    DIRE_BEAST_NORTHREND                         = 122807,
    DIRE_BEAST_KRASARANG_WILDS                   = 122809,
    DIRE_BEAST_VALLEY_OF_THE_FOUR_WINDS          = 122811,
    DIRE_BEAST_VALE_OF_THE_ETERNAL_BLOSSOM       = 126213,
    DIRE_BEAST_KUN_LAI_SUMMIT                    = 126214,
    DIRE_BEAST_TOWNLONG_STEPPES                  = 126215,
    DIRE_BEAST_DREAD_WASTES                      = 126216,
    DIRE_BEAST_DUNGEONS                          = 132764,
    DIRE_FRENZY                                  = 217200
};

// Dire Beast - 120679
class spell_hun_dire_beast : public SpellScriptLoader
{
    public:
        spell_hun_dire_beast() : SpellScriptLoader("spell_hun_dire_beast") { }

        class spell_hun_dire_beast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_dire_beast_SpellScript);

            void HandleOnHit()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        // 213466 - charges to target
                        _player->CastSpell(_player, 120694, true); // Energize

                        // Summon's skin is different function of Map or Zone ID
                        switch (_player->GetCurrentZoneID())
                        {
                            case 5785: // The Jade Forest
                                _player->CastSpell(target, DIRE_BEAST_JADE_FOREST, true);
                                break;
                            case 5805: // Valley of the Four Winds
                                _player->CastSpell(target, DIRE_BEAST_VALLEY_OF_THE_FOUR_WINDS, true);
                                break;
                            case 5840: // Vale of Eternal Blossoms
                                _player->CastSpell(target, DIRE_BEAST_VALE_OF_THE_ETERNAL_BLOSSOM, true);
                                break;
                            case 5841: // Kun-Lai Summit
                                _player->CastSpell(target, DIRE_BEAST_KUN_LAI_SUMMIT, true);
                                break;
                            case 5842: // Townlong Steppes
                                _player->CastSpell(target, DIRE_BEAST_TOWNLONG_STEPPES, true);
                                break;
                            case 6134: // Krasarang Wilds
                                _player->CastSpell(target, DIRE_BEAST_KRASARANG_WILDS, true);
                                break;
                            case 6138: // Dread Wastes
                                _player->CastSpell(target, DIRE_BEAST_DREAD_WASTES, true);
                                break;
                            default:
                            {
                                switch (_player->GetMapId())
                                {
                                    case 0: // Eastern Kingdoms
                                        _player->CastSpell(target, DIRE_BEAST_EASTERN_KINGDOMS, true);
                                        break;
                                    case 1: // Kalimdor
                                        _player->CastSpell(target, DIRE_BEAST_KALIMDOR, true);
                                        break;
                                    case 8: // Outland
                                    case 530:
                                        _player->CastSpell(target, DIRE_BEAST_OUTLAND, true);
                                        break;
                                    case 10: // Northrend
                                        _player->CastSpell(target, DIRE_BEAST_NORTHREND, true);
                                        break;
                                    default:
                                        _player->CastSpell(target, DIRE_BEAST_DUNGEONS, true);
                                        break;
                                }
                                break;
                            }
                        }
                    }
                }
            }

            void Register() override
            {
               OnHit += SpellHitFn(spell_hun_dire_beast_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_dire_beast_SpellScript();
        }
};

// A Murder of Crows - 131894, 206505
class spell_hun_a_murder_of_crows : public SpellScriptLoader
{
    public:
        spell_hun_a_murder_of_crows() : SpellScriptLoader("spell_hun_a_murder_of_crows") { }

        class spell_hun_a_murder_of_crows_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_a_murder_of_crows_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* target = GetTarget())
                {
                    if (Unit* caster = GetCaster())
                    {
                        caster->CastSpell(target, 131900, true);
                        target->CastSpell(target, 131637, true);
                        target->CastSpell(target, 131951, true);
                        target->CastSpell(target, 131952, true);
                    }
                }
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode == AURA_REMOVE_BY_DEATH)
                    if (Unit* caster = GetCaster())
                        if (Player* player = caster->ToPlayer())
                            player->RemoveSpellCooldown(GetSpellInfo()->Id, true);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_hun_a_murder_of_crows_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_a_murder_of_crows_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_a_murder_of_crows_AuraScript();
        }
};

// Called by Multi Shot - 2643
// Beast Cleave - 115939
class spell_hun_beast_cleave : public SpellScriptLoader
{
    public:
        spell_hun_beast_cleave() : SpellScriptLoader("spell_hun_beast_cleave") { }

        class spell_hun_beast_cleave_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_beast_cleave_SpellScript);

            void HandleAfterCast()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                    if (AuraEffect* auraEffect = _player->GetAuraEffect(115939, EFFECT_0))
                    {
                        if (Pet* pet = _player->GetPet())
                        {
                            float bp = auraEffect->GetAmount();
                            if (AuraEffect* auraEff = _player->GetAuraEffect(197047, EFFECT_0)) // Furious Swipes
                                bp += auraEff->GetAmount();

                            _player->CastCustomSpell(pet, 118455, &bp, NULL, NULL, true);
                        }
                    }
            }

            void Register() override
            {
               AfterCast += SpellCastFn(spell_hun_beast_cleave_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_beast_cleave_SpellScript();
        }
};

// Ancient Hysteria - 90355
class spell_hun_ancient_hysteria : public SpellScriptLoader
{
    public:
        spell_hun_ancient_hysteria() : SpellScriptLoader("spell_hun_ancient_hysteria") { }

        class spell_hun_ancient_hysteria_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_ancient_hysteria_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                std::list<WorldObject*> removeList;

                for (auto itr : targets)
                {
                    if (Unit* unit = itr->ToUnit())
                    {
                        if (unit->HasAura(95809) || unit->HasAura(57723) || unit->HasAura(57724) || unit->HasAura(80354) || unit->HasAura(160455) || unit->HasAura(146555) || unit->HasAura(178207))
                        {
                            removeList.push_back(itr);
                            continue;
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
                    target->CastSpell(target, 95809, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_ancient_hysteria_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_ancient_hysteria_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_hun_ancient_hysteria_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_ancient_hysteria_SpellScript();
        }
};

// Kill Command - 34026
class spell_hun_kill_command : public SpellScriptLoader
{
    public:
        spell_hun_kill_command() : SpellScriptLoader("spell_hun_kill_command") { }

        class spell_hun_kill_command_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_kill_command_SpellScript);

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(34026))
                    return false;
                return true;
            }

            void FilterTargets(WorldObject*& target)
            {
                if (Unit* caster = GetCaster())
                    if (Unit* pet = caster->GetGuardianPet())
                        if (pet->getVictim())
                            target = pet->getVictim();
            }

            SpellCastResult CheckCastMeet()
            {
                Unit* caster = GetCaster();
                if (!caster || caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_NO_PET;

                Player* player = caster->ToPlayer();
                Unit* pet = caster->GetGuardianPet();
                Unit* target = player->GetSelectedUnit();

                if (!pet || pet->isDead())
                    return SPELL_FAILED_NO_PET;

                // pet has a target and target is within 5 yards
                if (!target || !pet->IsWithinDist(target, GetSpellInfo()->Effects[EFFECT_2]->BasePoints, true))
                {
                    SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TARGET_TOO_FAR);
                    return SPELL_FAILED_CUSTOM_ERROR;
                }
                
                if (pet->HasUnitState(UNIT_STATE_CONTROLLED))
                {
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                }
                
                return SPELL_CAST_OK;
            }
            
            SpellCastResult CheckIfPetInLOS()
            {
                Unit* caster = GetCaster();
                if (Player* player = GetCaster()->ToPlayer())
                {
                    if (Unit* pet = GetCaster()->GetGuardianPet())
                    {
                        float x, y, z;
                        pet->GetPosition(x, y, z);
                        
                        if(Unit* target = player->GetSelectedUnit())
                           if (target->IsWithinLOS(x, y, z))
                               return SPELL_CAST_OK;
                    }
                }
                return SPELL_FAILED_LINE_OF_SIGHT;
            }
            
            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* pet = caster->GetGuardianPet())
                    {
                        if (Unit* target = GetHitUnit())
                        {
                            float damage = caster->GetTotalAttackPowerValue(WeaponAttackType(RANGED_ATTACK)) * 3.6f;
                            pet->CastCustomSpell(target, 83381, &damage, nullptr, nullptr, false);

                            if (!pet->IsWithinMeleeRange(target))
                                pet->CastSpell(target, 118171, false);

                            if (caster->HasAura(197248)) // Master of Beasts
                                if (Unit* hati = caster->GetHati())
                                {
                                    hati->CastCustomSpell(target, 83381, &damage, nullptr, nullptr, false);

                                    if (!hati->IsWithinMeleeRange(target))
                                        hati->CastSpell(target, 118171, false);
                                }

                            if (caster->HasAura(191384)) // Aspect of the Beast
                            {
                                switch (((Pet*)pet)->GetSpecializationId())
                                {
                                    case SPEC_PET_ADAPTATION_FEROCITY:
                                    case SPEC_PET_FEROCITY:
                                        pet->CastSpell(target, 191413, true);
                                        break;
                                    case SPEC_PET_ADAPTATION_TENACITY:
                                    case SPEC_PET_TENACITY:
                                        pet->CastSpell(pet, 191414, true);
                                        break;
                                    case SPEC_PET_ADAPTATION_CUNNING:
                                    case SPEC_PET_CUNNING:
                                        pet->CastSpell(target, 191397, true);
                                        break;
                                }
                            }
                        }
                    }
                }
            }

            void Register() override
            {
                OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_kill_command_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_TARGET_ENEMY);
                OnCheckCast += SpellCheckCastFn(spell_hun_kill_command_SpellScript::CheckCastMeet);
                OnCheckCast += SpellCheckCastFn(spell_hun_kill_command_SpellScript::CheckIfPetInLOS);
                OnEffectHitTarget += SpellEffectFn(spell_hun_kill_command_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_kill_command_SpellScript();
        }
};

// Master's Call - 53271
class spell_hun_masters_call : public SpellScriptLoader
{
    public:
        spell_hun_masters_call() : SpellScriptLoader("spell_hun_masters_call") { }

        class spell_hun_masters_call_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_masters_call_SpellScript);

            bool Validate(SpellInfo const* SpellInfo) override
            {
                if (!sSpellMgr->GetSpellInfo(62305))
                    return false;
                return true;
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* ally = GetHitUnit())
                {
                    if (Player* caster = GetCaster()->ToPlayer())
                        if (Pet* target = caster->GetPet())
                        {
                            TriggerCastFlags castMask = TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_IGNORE_CASTER_AURASTATE);
                            target->CastSpell(target, 62305, castMask);
                            target->CastSpell(ally, GetEffectValue(), castMask);
                        }
                }
            }
            
            SpellCastResult CheckIfPetInLOS()
            {
                if (Player* _player = GetCaster()->ToPlayer())
                {
                    if (Pet* pet = _player->GetPet())
                    {
                        if (pet->isDead())
                            return SPELL_FAILED_NO_PET;

                        float x, y, z;
                        pet->GetPosition(x, y, z);

                        if (_player->IsWithinLOS(x, y, z))
                            return SPELL_CAST_OK;
                    }
                }
                return SPELL_FAILED_LINE_OF_SIGHT;
            }
            
            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_masters_call_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
                OnCheckCast += SpellCheckCastFn(spell_hun_masters_call_SpellScript::CheckIfPetInLOS);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_masters_call_SpellScript();
        }
};

// Heart of the Phoenix - 55709
class spell_hun_pet_heart_of_the_phoenix : public SpellScriptLoader
{
    public:
        spell_hun_pet_heart_of_the_phoenix() : SpellScriptLoader("spell_hun_pet_heart_of_the_phoenix") { }

        class spell_hun_pet_heart_of_the_phoenix_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_pet_heart_of_the_phoenix_SpellScript);

            bool Load() override
            {
                if (!GetCaster()->isPet())
                    return false;
                return true;
            }

            bool Validate(SpellInfo const* /*SpellInfo*/) override
            {
                if (!sSpellMgr->GetSpellInfo(54114) || !sSpellMgr->GetSpellInfo(55711))
                    return false;
                return true;
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (Unit* owner = caster->GetOwner())
                    if (!caster->HasAura(55711))
                    {
                        owner->CastCustomSpell(54114, SPELLVALUE_BASE_POINT0, 100, caster, true);
                        caster->CastSpell(caster, 55711, true);
                    }
            }

            void Register() override
            {
                // add dummy effect spell handler to pet's Last Stand
                OnEffectHitTarget += SpellEffectFn(spell_hun_pet_heart_of_the_phoenix_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_pet_heart_of_the_phoenix_SpellScript();
        }
};

// Tame Beast - 1515 
class spell_hun_tame_beast : public SpellScriptLoader
{
    public:
        spell_hun_tame_beast() : SpellScriptLoader("spell_hun_tame_beast") { }

        class spell_hun_tame_beast_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_tame_beast_SpellScript);

            SpellCastResult CheckCast()
            {
                Unit* caster = GetCaster();
                if (caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_DONT_REPORT;

                if (!GetExplTargetUnit())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (Creature* target = GetExplTargetUnit()->ToCreature())
                {
                    if (target->getLevel() > caster->getLevel())
                        return SPELL_FAILED_HIGHLEVEL;

                    if (!target->GetCreatureTemplate()->isTameable(caster->ToPlayer()))
                        return SPELL_FAILED_BAD_TARGETS;

                    if (caster->GetPetGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if (caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }
                else
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                return SPELL_CAST_OK;
            }

            void Register() override
            {
                OnCheckCast += SpellCheckCastFn(spell_hun_tame_beast_SpellScript::CheckCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_tame_beast_SpellScript();
        }
};

// Fetch - 125050
class spell_hun_fetch : public SpellScriptLoader
{
    public:
        spell_hun_fetch() : SpellScriptLoader("spell_hun_fetch") { }

        class spell_hun_fetch_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_fetch_SpellScript);

            void HandleScriptEffect(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Player* _player = caster->ToPlayer();
                if (!_player)
                    return;

                Unit* target = _player->GetSelectedUnit();
                if (!target)
                    return;

                Creature* corps = target->ToCreature();
                if (!corps)
                    return;

                if (!_player->isAllowedToLoot(corps))
                {
                    if (const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(125050))
                        GetSpell()->SendCastResult(_player, spellInfo, SPELL_FAILED_BAD_TARGETS);
                    return;
                }

                if (!target->IsWithinDistInMap(_player, 40.0f))
                {
                    if (const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(125050))
                        GetSpell()->SendCastResult(_player, spellInfo, SPELL_FAILED_OUT_OF_RANGE);
                    return;
                }

                Pet* pet = _player->GetPet();
                if (!pet)
                    return;

                pet->StopMoving();
                pet->GetMotionMaster()->Clear(false);
                pet->GetMotionMaster()->MoveFetch(target, PET_FOLLOW_DIST, pet->GetFollowAngle());
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_hun_fetch_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_fetch_SpellScript();
        }
};

// Fireworks - 127933
class spell_hun_fireworks : public SpellScriptLoader
{
    public:
        spell_hun_fireworks() : SpellScriptLoader("spell_hun_fireworks") { }

        class spell_hun_fireworks_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_fireworks_SpellScript);

            void HandleEffect(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    switch(urand(0, 3))
                    {
                        case 0:
                            caster->CastSpell(caster, 127936, true);
                            break;
                        case 1:
                            caster->CastSpell(caster, 127937, true);
                            break;
                        case 2:
                            caster->CastSpell(caster, 127951, true);
                            break;
                        case 3:
                            caster->CastSpell(caster, 127961, true);
                            break;
                    }
                }
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_hun_fireworks_SpellScript::HandleEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_fireworks_SpellScript();
        }
};

// Flanking Strike - 202800
class spell_hun_flanking_strike : public SpellScriptLoader
{
    public:
        spell_hun_flanking_strike() : SpellScriptLoader("spell_hun_flanking_strike") {}

        class spell_hun_flanking_strike_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_flanking_strike_SpellScript);

            bool targetOnOwner = false;

            void HandleOnHit(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                Unit* caster = GetCaster();
                Pet* pet = caster->ToPlayer()->GetPet();
                if (!caster || !target || !caster->ToPlayer() || !caster->ToPlayer()->GetPet())
                    return;
                
                float _damage = caster->GetTotalAttackPowerValue(WeaponAttackType(BASE_ATTACK)) * 3.75f;
                float _threat = GetSpellInfo()->Effects[EFFECT_3]->BasePoints;

                if (!pet->IsWithinMeleeRange(target))
                    pet->CastSpell(target, 118171, false);

                if (targetOnOwner)
                {
                    _damage += CalculatePct(_damage, GetSpellInfo()->Effects[EFFECT_2]->BasePoints);
                    pet->CastCustomSpell(target, 206933, &_damage, nullptr, nullptr, true);
                    pet->AddThreat(target, _threat);
                }
                else
                {
                    SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), GetSpellInfo()->Effects[EFFECT_2]->BasePoints));
                    pet->CastCustomSpell(target, 206933, &_damage, nullptr, nullptr, true);
                }

                if (caster->HasAura(191384)) // Aspect of the Beast
                {
                    switch (pet->GetSpecializationId())
                    {
                        case SPEC_PET_ADAPTATION_FEROCITY:
                        case SPEC_PET_FEROCITY:
                            pet->CastSpell(target, 191413, true);
                            break;
                        case SPEC_PET_ADAPTATION_TENACITY:
                        case SPEC_PET_TENACITY:
                            pet->CastSpell(pet, 191414, true);
                            break;
                        case SPEC_PET_ADAPTATION_CUNNING:
                        case SPEC_PET_CUNNING:
                            pet->CastSpell(target, 191397, true);
                            break;
                    }
                }
            }

            void HandleOnCast()
            {
                Unit* caster = GetCaster();
                if (Unit* target = GetExplTargetUnit())
                    if (target->getVictim() == caster)
                        targetOnOwner = true;
                if (caster && caster->HasSpell(204315))//Animal Instincts
                {
                    switch (urand(0,2))
                    {
                        case 0:
                            caster->CastSpell(caster, 204333, true);
                            break;
                        case 1:
                            caster->CastSpell(caster, 204321, true);
                            break;
                        case 2:
                            caster->CastSpell(caster, 204324, true);
                            break;
                    }
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_flanking_strike_SpellScript::HandleOnHit, EFFECT_0, SPELL_EFFECT_WEAPON_PERCENT_DAMAGE);
                OnCast += SpellCastFn(spell_hun_flanking_strike_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_flanking_strike_SpellScript();
        }
};

// Explosive Shot: Detonate! - 212679
class spell_hun_explosive_shot_detonate : public SpellScriptLoader
{
    public:
        spell_hun_explosive_shot_detonate() : SpellScriptLoader("spell_hun_explosive_shot_detonate") { }

        class spell_hun_explosive_shot_detonate_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_explosive_shot_detonate_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (Unit* caster = GetCaster())
                {
                    if (AreaTrigger* areaObj = caster->GetAreaObject(212431))
                        areaObj->CastAction();
                }
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_hun_explosive_shot_detonate_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_explosive_shot_detonate_SpellScript();
        }
};

// Explosive Shot - 212680
class spell_hun_explosive_shot : public SpellScriptLoader
{
    public:
        spell_hun_explosive_shot() : SpellScriptLoader("spell_hun_explosive_shot") { }

        class spell_hun_explosive_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_explosive_shot_SpellScript);

            void HandleDamage(SpellEffIndex /*effIndex*/)
            {
                Position const* pos = GetExplTargetDest();
                if (Unit* target = GetHitUnit())
                {
                    int32 pct = 100 - (70.0f / 8.0f * target->GetDistance(*pos));
                    SetHitDamage(CalculatePct(GetHitDamage(), pct));
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_explosive_shot_SpellScript::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_explosive_shot_SpellScript();
        }
};

// Cobra Shot - 193455
class spell_hun_cobra_shot : public SpellScriptLoader
{
    public:
        spell_hun_cobra_shot() : SpellScriptLoader("spell_hun_cobra_shot") { }

        class spell_hun_cobra_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_cobra_shot_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    if (Unit* target = GetHitUnit())
                    {
                        if (AuraEffect const* aurEff = caster->GetAuraEffect(194397, EFFECT_0)) // Way of the Cobra
                        {
                            uint32 count = 0;
                            for (auto itr : caster->m_Controlled)
                                if(Unit* unit = ObjectAccessor::GetUnit(*caster, itr))
                                    if (unit->HasUnitTypeMask(UNIT_MASK_CREATED_BY_CLASS_SPELL) || unit->isPet())
                                        count++;

                            SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), aurEff->GetAmount() * count));
                        }
                    }
                }
            }

            void Register() override
            {
               OnHit += SpellHitFn(spell_hun_cobra_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_cobra_shot_SpellScript();
        }
};

// 13813 - Explosive Trap
class spell_hun_explosive_trap : public SpellScript
{
    PrepareSpellScript(spell_hun_explosive_trap);

    void HandleOnHit()
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
            {
                if (AuraEffect const* eff = caster->GetAuraEffect(199543, EFFECT_0)) // Expert Trapper
                {
                    Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();

                    if (target->GetGUID() == cont.GetValue<ObjectGuid>("13813Expert", ObjectGuid::Empty))
                    {
                        int32 damage = GetHitDamage();
                        SetHitDamage(AddPct(damage, eff->GetAmount()));
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_hun_explosive_trap::HandleOnHit);
    }
};

// 162496 - Steel Trap
class areatrigger_at_steel_trap : public AreaTriggerScript
{
    public:
    areatrigger_at_steel_trap() : AreaTriggerScript("areatrigger_at_steel_trap") {}

    struct areatrigger_at_steel_trapAI : AreaTriggerAI
    {
        areatrigger_at_steel_trapAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        uint32 triggerTimer;
        uint32 despawnTimer;

        void OnCreate() override
        {
            triggerTimer = 0;
            despawnTimer = 0;
        }

        void OnUpdate(uint32 diff) override
        {
            triggerTimer += diff;

            if (despawnTimer)
            {
                despawnTimer += diff;

                if (despawnTimer > 1000)
                    at->Despawn();
            }
        }

        uint32 CallSpecialFunction(uint32 /*Num*/)
        {
            return triggerTimer;
        }

        void OnUnitEnter(Unit* unit) override
        {
            if (Unit* hunt = at->GetCaster())
            {
                if (hunt->HasAura(212574)) // Nesingwary's Trapping Treads
                    hunt->CastSpell(hunt, 212575, true);

                if (hunt->HasAura(199543)) // Expert Trapper
                    hunt->CastSpell(unit, 201199, true);

                hunt->CastSpell(unit, 162480, true);
                hunt->CastSpell(unit, 162487, true);

                despawnTimer = 1;
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_steel_trapAI(areatrigger);
    }
};

// 187699 - Tar Trap
class areatrigger_at_tar_trap : public AreaTriggerScript
{
    public:
    areatrigger_at_tar_trap() : AreaTriggerScript("areatrigger_at_tar_trap") {}

    struct areatrigger_at_tar_trapAI : AreaTriggerAI
    {
        areatrigger_at_tar_trapAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        uint32 triggerTimer;

        void OnCreate() override
        {
            triggerTimer = 0;
        }

        void OnUpdate(uint32 diff) override
        {
            triggerTimer += diff;
        }

        void OnUnitEnter(Unit* /*unit*/) override
        {
            if (Unit* hunt = at->GetCaster())
            {
                if (hunt->HasAura(212574)) // Nesingwary's Trapping Treads
                    hunt->CastSpell(hunt, 212575, true);

                hunt->CastSpell(at, 187700, true);

                if (AuraEffect const* aurEff = hunt->GetAuraEffect(234955, EFFECT_0)) // Waylay
                {
                    if (triggerTimer > (aurEff->GetAmount() * 1000))
                        hunt->CastSpell(at, 236699, true);
                }

                at->Despawn();
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_tar_trapAI(areatrigger);
    }
};

// 3355 - Freezing Trap
class areatrigger_at_freezing_trap : public AreaTriggerScript
{
    public:
    areatrigger_at_freezing_trap() : AreaTriggerScript("areatrigger_at_freezing_trap") {}

    struct areatrigger_at_freezing_trapAI : AreaTriggerAI
    {
        areatrigger_at_freezing_trapAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
        {
        }

        uint32 triggerTimer;

        void OnCreate() override
        {
            triggerTimer = 0;
        }

        void OnUpdate(uint32 diff) override
        {
            triggerTimer += diff;
        }

        void Triggered(Unit* caster)
        {
            at->Despawn();

            if (caster->HasAura(212574)) // Nesingwary's Trapping Treads
                caster->CastSpell(caster, 212575, true);
        }

        bool CreateUnitList(std::list<Unit*>& unitList) override
        {
            at->GetAttackableUnitListInRange(unitList, at->GetRadius(), true);

            if (unitList.size() > 1)
                unitList.sort(Trinity::UnitSortDistance(true, at));

            return true;
        }

        void OnUnitEnter(Unit* unit) override
        {
            if (Unit* hunt = at->GetCaster())
            {
                if (hunt->HasAura(203340)) // Diamond Ice
                {
                    hunt->CastSpell(unit, 203337, true);
                    Triggered(hunt);
                    return;
                }

                if (AuraEffect const* aurEff = hunt->GetAuraEffect(234955, EFFECT_0)) // Waylay
                {
                    if (triggerTimer > (aurEff->GetAmount() * 1000))
                    {
                        float bp = 1.f;
                        hunt->CastCustomSpell(unit, 3355, &bp, NULL, NULL, true);
                        Triggered(hunt);
                        return;
                    }
                }
                hunt->CastSpell(unit, 3355, true);
                Triggered(hunt);
                return;
            }
        }
    };

    AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
    {
        return new areatrigger_at_freezing_trapAI(areatrigger);
    }
};

// 13813 - Explosive Trap
class areatrigger_at_explosive_trap : public AreaTriggerScript
{
    public:
        areatrigger_at_explosive_trap() : AreaTriggerScript("areatrigger_at_explosive_trap") {}

        struct areatrigger_at_explosive_trapAI : AreaTriggerAI
        {
            areatrigger_at_explosive_trapAI(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
            {
            }

            uint32 triggerTimer;

            void OnCreate() override
            {
                triggerTimer = 0;
            }

            void OnUpdate(uint32 diff) override
            {
                triggerTimer += diff;
            }

            bool CreateUnitList(std::list<Unit*>& unitList) override
            {
                at->GetAttackableUnitListInRange(unitList, at->GetRadius(), true);

                if (unitList.size() > 1)
                    unitList.sort(Trinity::UnitSortDistance(true, at));

                return true;
            }

            void OnUnitEnter(Unit* unit) override
            {
                if (Unit* hunt = at->GetCaster())
                {
                    Trinity::AnyDataContainer& cont = hunt->GetAnyDataContainer();
                    bool removeData = false;

                    if (unit && hunt->HasAura(199543)) // Expert Trapper
                    {
                        removeData = true;
                        cont.Set("13813Expert", unit->GetGUID());
                    }

                    if (hunt->HasAura(212574)) // Nesingwary's Trapping Treads
                        hunt->CastSpell(hunt, 212575, true);

                    hunt->CastSpell(at, 13812, true);

                    if (AuraEffect const* aurEff = hunt->GetAuraEffect(234955, EFFECT_0)) // Waylay
                    {
                        if (triggerTimer > (aurEff->GetAmount() * 1000))
                            hunt->CastSpell(at, 237338, true);
                    }

                    if (removeData)
                        cont.Remove("13813Expert");

                    at->Despawn();
                }
            }
        };

        AreaTriggerAI* GetAI(AreaTrigger* areatrigger) const override
        {
            return new areatrigger_at_explosive_trapAI(areatrigger);
        }
};

// Claw, Bite, Smack - 16827, 49966, 17253
class spell_hun_blink_strikes : public SpellScriptLoader
{
    public:
        spell_hun_blink_strikes() : SpellScriptLoader("spell_hun_blink_strikes") { }

        class spell_hun_blink_strikes_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_blink_strikes_SpellScript);

            void HandleOnCast()
            {
                Creature* caster = GetCaster()->ToCreature();
                Unit* target = GetExplTargetUnit();
                Unit* owner = caster->GetAnyOwner();
                if (!caster || !target || caster->HasCreatureSpellCooldown(130393) || !owner->HasAura(130392))
                    return;

                if (!caster->IsWithinMeleeRange(target))
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(130392)) // Blink Strikes
                    {
                        caster->CastSpell(target, 130393, true);
                        caster->_AddCreatureSpellCooldown(130393, time(NULL) + time_t(spellInfo->Effects[EFFECT_3]->BasePoints));
                    }
                }
            }
            void Register() override
            {
                OnCast += SpellCastFn(spell_hun_blink_strikes_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_blink_strikes_SpellScript();
        }
};

// Beast Cleave - 118455
class spell_hun_beast_cleave_tgr : public SpellScript
{
    PrepareSpellScript(spell_hun_beast_cleave_tgr);

    void FilterTargets(WorldObject*& /*target*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->HasAura(197248)) // Master of Beasts
            {
                if (Unit* hati = caster->GetHati())
                {
                    if (Spell* spell = GetSpell())
                        spell->AddUnitTarget(hati, 1 << EFFECT_0, false);
                }
            }
        }
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_beast_cleave_tgr::FilterTargets, EFFECT_0, TARGET_UNIT_PET);
    }
};

// Piercing Shot - 193455
class spell_hun_piercing_shot : public SpellScriptLoader
{
    public:
        spell_hun_piercing_shot() : SpellScriptLoader("spell_hun_piercing_shot") { }

        class spell_hun_piercing_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_piercing_shot_SpellScript);

            void HandleOnHit()
            {
                SetHitDamage(CalculatePct(GetHitDamage(), GetSpell()->GetPowerCost(POWER_FOCUS)));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_hun_piercing_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_piercing_shot_SpellScript();
        }
};

// Netherwinds - 160452
class spell_hun_netherwinds : public SpellScriptLoader
{
    public:
        spell_hun_netherwinds() : SpellScriptLoader("spell_hun_netherwinds") { }

        class spell_hun_netherwinds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_netherwinds_SpellScript);

            void RemoveInvalidTargets(std::list<WorldObject*>& targets)
            {
                std::list<WorldObject*> removeList;

                for (auto itr : targets)
                {
                    if (Unit* unit = itr->ToUnit())
                    {
                        if (unit->HasAura(95809) || unit->HasAura(57723) || unit->HasAura(57724) || unit->HasAura(80354) || unit->HasAura(160455))
                        {
                            removeList.push_back(itr);
                            continue;
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
                    target->CastSpell(target, 160455, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_netherwinds_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_RAID);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_netherwinds_SpellScript::RemoveInvalidTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_RAID);
                AfterHit += SpellHitFn(spell_hun_netherwinds_SpellScript::ApplyDebuff);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_netherwinds_SpellScript();
        }
};

// Dragonscale Armor (PvP talent) 202589 
class spell_hun_dragonscale_armor : public SpellScriptLoader
{
    public:
        spell_hun_dragonscale_armor() : SpellScriptLoader("spell_hun_dragonscale_armor") { }

        class spell_hun_dragonscale_armor_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_dragonscale_armor_AuraScript);

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
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_dragonscale_armor_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
                OnEffectAbsorb += AuraEffectAbsorbFn(spell_hun_dragonscale_armor_AuraScript::Absorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_dragonscale_armor_AuraScript();
        }
};

// Item - Hunter T19 Beast Mastery 4P Bonus - 211183
class spell_hun_t19_bm : public SpellScriptLoader
{
    public:
    spell_hun_t19_bm() : SpellScriptLoader("spell_hun_t19_bm") {}

    class spell_hun_t19_bm_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_hun_t19_bm_AuraScript);

        void CalculateAmount(AuraEffect const* /*aurEff*/, float & amount, bool & /*canBeRecalculated*/)
        {
            if (Unit* caster = GetCaster())
            {
                if (caster->HasSpell(DIRE_FRENZY))
                    amount = GetSpellInfo()->Effects[EFFECT_1]->BasePoints;
            }
        }

        void Register() override
        {
            DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_t19_bm_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_hun_t19_bm_AuraScript();
    }

    class spell_hun_t19_bm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hun_t19_bm_SpellScript);

        void RemoveInvalidTargets(std::list<WorldObject*>& targets)
        {
            targets.clear();

            if (Unit* caster = GetCaster())
            {
                bool hasDireFrenzy = caster->HasSpell(DIRE_FRENZY);

                for (auto itr : caster->m_Controlled)
                {
                    Unit* unit = ObjectAccessor::GetUnit(*caster, itr);
                    if (!unit)
                        continue;

                    if (hasDireFrenzy)
                    {
                        if (unit->isPet())
                        {
                            targets.push_back(unit);
                            break;
                        }
                    }
                    else
                    {
                        if (uint32 createdSpellId = unit->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL))
                        {
                            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(createdSpellId))
                            {
                                if (spellInfo->ClassOptions.SpellClassMask.HasFlag(0, 0, 0, 16))
                                    targets.push_back(unit);
                            }
                        }
                    }
                }
            }
        }

        void RemoveHunter(WorldObject*& target)
        {
            target = NULL;
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hun_t19_bm_SpellScript::RemoveInvalidTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_SUMMON);
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_t19_bm_SpellScript::RemoveHunter, EFFECT_0, TARGET_UNIT_CASTER);
            OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_hun_t19_bm_SpellScript::RemoveHunter, EFFECT_1, TARGET_UNIT_CASTER);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_hun_t19_bm_SpellScript();
    }
};

// Hati's Bond - 197344
class spell_hun_hatis_bond : public SpellScriptLoader
{
    public:
        spell_hun_hatis_bond() : SpellScriptLoader("spell_hun_hatis_bond") { }

        class spell_hun_hatis_bond_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_hatis_bond_AuraScript);

            uint32 update = 0;

            void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Player* _player = caster->ToPlayer();
                if (!_player)
                    return;

                if (Unit* hati = caster->GetHati())
                    if (Creature* creature = hati->ToCreature())
                        creature->DespawnOrUnsummon(100);

                Item* item = _player->GetItemByGuid(GetAura()->GetCastItemGUID());
                if (!item || !item->IsEquipped())
                    return;

                ArtifactAppearanceEntry const* artifactAppearance = sArtifactAppearanceStore.LookupEntry(item->GetModifier(ITEM_MODIFIER_ARTIFACT_APPEARANCE_ID));
                if (!artifactAppearance)
                    return;

                uint32 hatiSpellID = 197388;
                switch(artifactAppearance->ArtifactAppearanceSetID)
                {
                    case 117:
                        hatiSpellID = 211145;
                        break;
                    case 118:
                        hatiSpellID = 211146;
                        break;
                    case 119:
                        hatiSpellID = 211147;
                        break;
                    case 120:
                    case 221:
                        hatiSpellID = 211148;
                        break;
                }

                GetAura()->SetCustomData(hatiSpellID);
                if (_player->GetSession()->PlayerLogout() || _player->GetSession()->PlayerLoading())
                    return;

                if (_player->GetPet())
                    caster->CastSpell(caster, hatiSpellID, true);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                caster->RemoveAurasDueToSpell(211117);

                for (Unit::ControlList::iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
                {
                    if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*caster, *itr))
                    {
                        switch (creature->GetEntry())
                        {
                            case 100324:
                            case 106548:
                            case 106549:
                            case 106550:
                            case 106551:
                                creature->DespawnOrUnsummon(1000);
                                break;
                        }
                    }
                }
            }

            void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
            {
                update += diff;

                if (update >= 2000)
                {
                    update = 0;
                    Unit* caster = GetCaster();
                    if (!caster || !caster->isAlive())
                        return;

                    bool findHati = false;
                    bool findPet = false;
                    Creature* hati = NULL;

                    for (Unit::ControlList::iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
                    {
                        if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*caster, *itr))
                        {
                            switch (creature->GetEntry())
                            {
                                case 100324:
                                case 106548:
                                case 106549:
                                case 106550:
                                case 106551:
                                    findHati = true;
									if (creature->isAlive())
									{
										hati = creature;
										if (Player* player = caster->ToPlayer())
											if (Pet* pet = player->GetPet())
												if (ReactStates reactState = pet->GetReactState())
													hati->SetReactState(reactState);
									}
                                    break;
                                default:
                                    findPet = true;
                                    break;
                            }
                        }
                    }

                    if (!findHati && findPet && caster->ToPlayer()->GetPet())
                        caster->CastSpell(caster, GetAura()->GetCustomData(), true);

                    if (!findPet && hati)
                        hati->DespawnOrUnsummon(1000);
                    else if (hati && caster->GetDistance(hati) > 100.0f)
                        hati->DespawnOrUnsummon(1000);
                }
            }

            void Register() override
            {
                AfterEffectApply += AuraEffectApplyFn(spell_hun_hatis_bond_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_hatis_bond_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
                OnEffectUpdate += AuraEffectUpdateFn(spell_hun_hatis_bond_AuraScript::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_hatis_bond_AuraScript();
        }
};

// Broken Bond - 211117
class spell_hun_broken_bond : public SpellScriptLoader
{
    public:
        spell_hun_broken_bond() : SpellScriptLoader("spell_hun_broken_bond") { }

        class spell_hun_broken_bond_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_broken_bond_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
                if (removeMode != AURA_REMOVE_BY_EXPIRE)
                    return;

                Unit* caster = GetCaster();
                if (!caster || !caster->isAlive())
                    return;

                for (Unit::ControlList::iterator itr = caster->m_Controlled.begin(); itr != caster->m_Controlled.end(); ++itr)
                {
                    if (Creature* creature = ObjectAccessor::GetCreature(*caster, *itr))
                    {
                        switch (creature->GetEntry())
                        {
                            case 100324:
                            case 106548:
                            case 106549:
                            case 106550:
                            case 106551:
                                if (creature->isAlive())
                                    return;
                                else
                                    creature->DespawnOrUnsummon(1000);
                                break;
                        }
                    }
                }
                if (Aura* aura = caster->GetAura(197344))
                    caster->CastSpell(caster, aura->GetCustomData(), true);
            }

            void Register() override
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_hun_broken_bond_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_broken_bond_AuraScript();
        }
};

// Windburst (Artifact) - 204147
class spell_hun_windburst : public SpellScriptLoader
{
    public:
        spell_hun_windburst() : SpellScriptLoader("spell_hun_windburst") { }

        class spell_hun_windburst_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_windburst_SpellScript);

            enum MyEnum
            {
                WindburstVisual1 = 223114,
                WindburstVisual2 = 226872
            };

            void HandleAfterCast()
            {
                Unit* caster = GetCaster();
                Unit* target = GetExplTargetUnit();
                if(caster && target)
                {
                    uint32 count = 0;
                    uint32 spellId = WindburstVisual1;
                    float dist = caster->GetExactDist(target);
                    caster->CastSpell(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), spellId, true);

                    Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();
                    cont.Set("WindburstRange", dist);
                    caster->CastSpell(caster, 204475, true);

                    float angle = caster->GetAngle(target);
                    cont.Set("WindburstAngle", angle);
                    int32 maxCount = uint32(dist / 5.0f) + 1;

                    for (int i = 1; i < maxCount; i++)
                    {
                        spellId = spellId == WindburstVisual1 ? WindburstVisual2 : WindburstVisual1;
                        count++;
                        float x = caster->GetPositionX() + 5.0f * std::cos(angle) * i;
                        float y = caster->GetPositionY() + 5.0f * std::sin(angle) * i;
                        float z = caster->GetPositionZ();
                        Trinity::NormalizeMapCoord(x);
                        Trinity::NormalizeMapCoord(y);
                        caster->UpdateGroundPositionZ(x, y, z);
                        caster->CastSpell(x, y, z, spellId, true);
                    }

                    caster->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spellId, true);
                }
            }

            void Register() override
            {
                AfterCast += SpellCastFn(spell_hun_windburst_SpellScript::HandleAfterCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_windburst_SpellScript();
        }
};

// Aimed Shot - 19434
class spell_hun_aimed_shot : public SpellScriptLoader
{
    public:
        spell_hun_aimed_shot() : SpellScriptLoader("spell_hun_aimed_shot") { }

        class spell_hun_aimed_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_aimed_shot_SpellScript);

            void HandleOnHit()
            {
                if (Unit* target = GetHitUnit())
                if (Unit* caster = GetCaster())
                {
                    int32 damage = GetHitDamage();
                    if (!target->HasAura(236641, caster->GetGUID()))
                    {
                        damage += CalculatePct(damage, GetSpellInfo()->Effects[EFFECT_2]->BasePoints);
                    }
                    if (AuraEffect* auraEff = caster->GetAuraEffect(199522, EFFECT_0))
                    {
                        float bp = CalculatePct(damage, auraEff->GetAmount());
                        caster->CastCustomSpell(target, 199885, &bp, NULL, NULL, true);
                    }
                    SetHitDamage(damage);
                }
            }

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (!caster->isInCombat())
                        caster->ClearSpellTargets(GetSpellInfo()->Id);
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_hun_aimed_shot_SpellScript::HandleOnCast);
                OnHit += SpellHitFn(spell_hun_aimed_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_aimed_shot_SpellScript();
        }
};

// Trick Shot - 199885
class spell_hun_trick_shot : public SpellScriptLoader
{
    public:
        spell_hun_trick_shot() : SpellScriptLoader("spell_hun_trick_shot") { }

        class spell_hun_trick_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_trick_shot_SpellScript);

            void HandleOnHit()
            {
                if (Unit* caster = GetCaster())
                {
                    SpellInfo const* spellinfo =  sSpellMgr->GetSpellInfo(19434);
                    if (Unit* target = GetHitUnit())
                    {
                        caster->DealDamage(target, GetSpellValue()->EffectBasePoints[EFFECT_0], NULL, SPELL_DIRECT_DAMAGE, spellinfo->GetSchoolMask(), spellinfo, false);
                        caster->SendSpellNonMeleeDamageLog(target, 19434, GetSpellValue()->EffectBasePoints[EFFECT_0], spellinfo->GetSchoolMask(), 0, 0, false, 0, spellinfo->GetSpellXSpellVisualId(caster));
                    }
                }
            }

            void Register() override
            {
               OnHit += SpellHitFn(spell_hun_trick_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_trick_shot_SpellScript();
        }
};

// Sidewinders - 214579
class spell_hun_sidewinders : public SpellScriptLoader
{
    public:
        spell_hun_sidewinders() : SpellScriptLoader("spell_hun_sidewinders") { }

        class spell_hun_sidewinders_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_sidewinders_SpellScript)

            bool check = false;

            void HandleOnCast()
            {
                if (Unit* caster = GetCaster())
                    if (caster->HasAura(223138) || caster->HasAura(193526))
                        check = true;
            }

            void HandleEffectHit(SpellEffIndex /*effIndex*/)
            {
                if (check)
                    if (Unit* caster = GetCaster())
                        if (Unit* target = GetHitUnit())
                            caster->CastSpell(target, 185365, true);
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_hun_sidewinders_SpellScript::HandleOnCast);
                OnEffectHitTarget += SpellEffectFn(spell_hun_sidewinders_SpellScript::HandleEffectHit, EFFECT_2, SPELL_EFFECT_DUMMY);
                OnEffectHitTarget += SpellEffectFn(spell_hun_sidewinders_SpellScript::HandleEffectHit, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_sidewinders_SpellScript();
        }
};

// Trailblazer - 199921
class spell_hun_trailblazer : public AuraScript
{
    PrepareAuraScript(spell_hun_trailblazer);

    enum MyEnum
    {
        TrailblazerSpeed = 231390
    };

    int32 timer = 3000;

    void OnProc(AuraEffect const* aurEff, ProcEventInfo& /*eventInfo*/)
    {
        timer = aurEff->GetAmount();

        if (Unit* hunt = GetUnitOwner())
            hunt->RemoveAurasDueToSpell(TrailblazerSpeed);
    }

    void OnUpdate(uint32 diff, AuraEffect* /*aurEff*/)
    {
        if (!timer)
            return;

        timer -= diff;

        if (timer <= 0)
        {
            timer = 0;

            if (Unit* hunt = GetUnitOwner())
                hunt->CastSpell(hunt, TrailblazerSpeed, true);
        }
    }

    void Register() override
    {
        OnEffectUpdate += AuraEffectUpdateFn(spell_hun_trailblazer::OnUpdate, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectProc += AuraEffectProcFn(spell_hun_trailblazer::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// Eagle's Bite - 204081
class spell_hun_eagles_bite : public AuraScript
{
    PrepareAuraScript(spell_hun_eagles_bite);

    void OnProc(AuraEffect const* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        if (Unit* attacker = eventInfo.GetActor())
        {
            if (GetCasterGUID() != attacker->GetGUID())
            {
                PreventDefaultAction();
                return;
            }

            GetAura()->RefreshTimers();
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_hun_eagles_bite::OnProc, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

// Vulnerable - 187131
class spell_hun_vulnerable : public SpellScriptLoader
{
    public:
        spell_hun_vulnerable() : SpellScriptLoader("spell_hun_vulnerable") { }

        class spell_hun_vulnerable_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_vulnerable_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if(AuraEffect* aurEff1 = const_cast<AuraEffect*>(aurEff->GetBase()->GetEffect(EFFECT_1)))
                    aurEff1->ChangeAmount(aurEff->GetAmount() + aurEff1->GetAmount());
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_vulnerable_AuraScript::OnTick, EFFECT_3, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_vulnerable_AuraScript();
        }
};

// Kill command damage - 83381
class spell_hun_kill_command_damage : public SpellScriptLoader
{
    public:
        spell_hun_kill_command_damage() : SpellScriptLoader("spell_hun_kill_command_damage") { }

        class spell_hun_kill_command_damage_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_kill_command_damage_SpellScript)

            void HandleEffectHit(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* owner = caster->GetAnyOwner();
                Unit* target = GetHitUnit();
                if (!caster || !owner || !target)
                    return;

                if (AuraEffect* auraEff = owner->GetAuraEffect(197199, EFFECT_0)) // Spirit Bond
                {
                    float bp0 = CalculatePct(GetHitDamage(), auraEff->GetAmount());
                    owner->CastCustomSpell(owner, 197205, &bp0, nullptr, nullptr, true);
                }

                if (AuraEffect* auraEff = owner->GetAuraEffect(197162, EFFECT_0)) // Jaws of Thunder
                {
                    if (!roll_chance_i(auraEff->GetAmount()))
                        return;

                    AuraEffect* auraEff1 = auraEff->GetBase()->GetEffect(EFFECT_1);
                    if (!auraEff1)
                        return;

                    float bp0 = CalculatePct(GetHitDamage(), auraEff1->GetAmount());
                    caster->CastCustomSpell(target, 197163, &bp0, nullptr, nullptr, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_hun_kill_command_damage_SpellScript::HandleEffectHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_kill_command_damage_SpellScript();
        }
};

// Mark of Helbrine 213156 
class spell_hun_mark_of_helbrine : public SpellScriptLoader
{
    public:
        spell_hun_mark_of_helbrine() : SpellScriptLoader("spell_hun_mark_of_helbrine") { }

        class spell_hun_mark_of_helbrine_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_mark_of_helbrine_AuraScript);

            void CalculateAmount(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                Unit* target = GetUnitOwner();
                if (!target)
                    return;

                if (AuraEffect* auraEff = caster->GetAuraEffect(213154, EFFECT_0))
                    amount = CalculatePct(auraEff->GetAmount(), caster->GetDistance(target) / 35.0f * 100.0f);
            }

            void Register() override
            {
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hun_mark_of_helbrine_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_INCREASE_SCHOOL_DAMAGE_TAKEN);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_mark_of_helbrine_AuraScript();
        }
};

// Marked Shot - 212621
class spell_hun_marked_shot : public SpellScriptLoader
{
    public:
        spell_hun_marked_shot() : SpellScriptLoader("spell_hun_marked_shot") { }

        class spell_hun_marked_shot_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_marked_shot_SpellScript);

            bool canRemove = true;

            void HandleOnCast()
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (AuraEffect* auraEff = caster->GetAuraEffect(224550, EFFECT_0)) // Zevrim's Hunger
                    if (roll_chance_i(auraEff->GetAmount()))
                        canRemove = false;

                if (canRemove)
                    caster->RemoveAurasDueToSpell(185743); // Hunter's Mark
            }

            void HandleOnHit()
            {
                if (!canRemove)
                    return;

                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* target = GetHitUnit())
                    target->RemoveAurasDueToSpell(185365, caster->GetGUID()); // Hunter's Mark
            }

            void Register() override
            {
                OnCast += SpellCastFn(spell_hun_marked_shot_SpellScript::HandleOnCast);
                OnHit += SpellHitFn(spell_hun_marked_shot_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_marked_shot_SpellScript();
        }
};

// Titan's Thunder - 207094
class spell_hun_titans_thunder : public SpellScriptLoader
{
    public:
        spell_hun_titans_thunder() : SpellScriptLoader("spell_hun_titans_thunder") { }

        class spell_hun_titans_thunder_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hun_titans_thunder_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit* caster = GetUnitOwner())
                    if (Unit* target = caster->getVictim())
                        caster->CastSpell(target, 207097, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_hun_titans_thunder_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hun_titans_thunder_AuraScript();
        }
};

// Misdirection - 34477
class spell_hun_misdirection : public AuraScript
{
    PrepareAuraScript(spell_hun_misdirection);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->SetReducedThreatPercent(0, ObjectGuid::Empty);
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, 35079, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_hun_misdirection::OnRemove, EFFECT_1, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectApply += AuraEffectApplyFn(spell_hun_misdirection::OnApply, EFFECT_1, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 120679, 217200 - (->reduction cd for 19574)
class spell_hun_bw : public SpellScriptLoader
{
    public:
        spell_hun_bw() : SpellScriptLoader("spell_hun_bw") { }

        class spell_hun_bw_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hun_bw_SpellScript)

            void HandleBeforeCast()
            {
                auto caster = GetCaster();
                if (!caster)
                    return;

                if (caster->HasAura(231548))
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(19574))
                    {
                        int32 cd = spellInfo->Effects[EFFECT_2]->CalcValue(caster) * -1000;
                        if (Player* plr = caster->ToPlayer())
                            plr->ModifySpellCooldown(19574, cd);
                    }
                }
            }

            void Register() override
            {
                BeforeCast += SpellCastFn(spell_hun_bw_SpellScript::HandleBeforeCast);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hun_bw_SpellScript();
        }
};

// 204475 - Windburst
struct areatrigger_hun_windburst : public AreaTriggerAI
{
    areatrigger_hun_windburst(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)
    {}

    enum MyEnum
    {
        Windburst     = 204477,
        CyclonicBurst = 242712
    };

    bool bHasCyclonicBurst = false;

    bool IsValidTarget(Unit* caster, Unit* target, AreaTriggerActionMoment actionM) override
    {
        if (!target->isTotem() && (caster->IsFriendlyTo(target) || caster->IsValidAttackTarget(target)))
            return true;

        return false;
    }

    void OnCreate() override
    {
        if (Unit* caster = at->GetCaster())
        {
            Trinity::AnyDataContainer& cont = caster->GetAnyDataContainer();
            float range = cont.GetValue<float>("WindburstRange", 0.f);
            at->SetOrientation(cont.GetValue<float>("WindburstAngle", 0.f));

            cont.Remove("WindburstRange");
            cont.Remove("WindburstAngle");

            at->SetRadius(range);

            at->SetPolygonVertices(0, true, range);
            at->SetPolygonVertices(3, true, range);

            if (caster->HasAura(238124))
                bHasCyclonicBurst = true;
        }
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (int32 dur = at->GetDuration())
            {
                if (caster->IsFriendlyTo(unit))
                {
                    caster->AddAura(Windburst, unit, nullptr, 0, dur, dur);
                }
                else if (bHasCyclonicBurst)
                {
                    if (Aura* aura = caster->AddAura(CyclonicBurst, unit, nullptr, 0, dur, dur))
                    {
                        if (AuraEffect* eff = aura->GetEffect(EFFECT_1))
                        {
                            int32 effPeriod = eff->GetPeriod();
                            eff->SetPeriodicTimer(effPeriod - (dur - ((dur / effPeriod) * effPeriod)));
                        }
                    }
                }
            }
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (ObjectGuid const& casterGUID = at->GetCasterGUID())
            RemoveAuras(unit, casterGUID);
    }

    void RemoveAuras(Unit* target, ObjectGuid const& casterGUID)
    {
        target->RemoveAurasDueToSpell(Windburst, casterGUID);
        target->RemoveAurasDueToSpell(CyclonicBurst, casterGUID);
    }
};

// 19574 - Bestial Wrath
class spell_hun_bestial_wrath : public SpellScriptLoader
{
public:
	spell_hun_bestial_wrath() : SpellScriptLoader("spell_hun_bestial_wrath") { }

	class spell_hun_bestial_wrath_SpellScript : public SpellScript
	{
		PrepareSpellScript(spell_hun_bestial_wrath_SpellScript)

			void HandleAfterCast()
		{
			if (Unit* caster = GetCaster())
				if (caster->HasAura(197248)) // Master of Beasts
					if (Unit* hati = caster->GetHati())
						caster->CastSpell(hati, 207033, true);
		}

		void Register() override
		{
			AfterCast += SpellCastFn(spell_hun_bestial_wrath_SpellScript::HandleAfterCast);
		}
	};

	SpellScript* GetSpellScript() const override
	{
		return new spell_hun_bestial_wrath_SpellScript();
	}
};

void AddSC_hunter_spell_scripts()
{
    new spell_hun_dire_beast();
    new spell_hun_a_murder_of_crows();
    new spell_hun_beast_cleave();
    new spell_hun_ancient_hysteria();
    new spell_hun_kill_command();
    new spell_hun_masters_call();
    new spell_hun_pet_heart_of_the_phoenix();
    new spell_hun_tame_beast();
    new spell_hun_fetch();
    new spell_hun_fireworks();
    new spell_hun_flanking_strike();
    new spell_hun_explosive_shot_detonate();
    new spell_hun_explosive_shot();
    new spell_hun_cobra_shot();
    new spell_hun_blink_strikes();
    new spell_hun_piercing_shot();
    new spell_hun_netherwinds();
    new spell_hun_dragonscale_armor();
    new spell_hun_hatis_bond();
    new spell_hun_broken_bond();
    new spell_hun_windburst();
    new spell_hun_aimed_shot();
    new spell_hun_trick_shot();
    new spell_hun_sidewinders();
    new spell_hun_vulnerable();
    new spell_hun_kill_command_damage();
    new spell_hun_mark_of_helbrine();
    new spell_hun_marked_shot();
    new spell_hun_titans_thunder();
    new spell_hun_t19_bm();
    new spell_hun_bw();
    new areatrigger_at_freezing_trap();
    new areatrigger_at_tar_trap();
    new areatrigger_at_steel_trap();
    new areatrigger_at_explosive_trap();
    RegisterAuraScript(spell_hun_misdirection);
    RegisterAuraScript(spell_hun_trailblazer);
    RegisterSpellScript(spell_hun_beast_cleave_tgr);
    RegisterAuraScript(spell_hun_eagles_bite);
    RegisterSpellScript(spell_hun_explosive_trap);
    RegisterAreaTriggerAI(areatrigger_hun_windburst);
	new spell_hun_bestial_wrath();
}
