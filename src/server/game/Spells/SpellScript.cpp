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

#include "Spell.h"
#include "SpellAuras.h"
#include "SpellScript.h"

bool _SpellScript::_Validate(SpellInfo const* entry)
{
    if (!Validate(entry))
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` did not pass Validate() function of script `%s` - script will be not added to the spell", entry->Id, m_scriptName->c_str());
        return false;
    }
    return true;
}

_SpellScript::_SpellScript(): m_currentScriptState(SPELL_SCRIPT_STATE_NONE), m_scriptName(nullptr), m_scriptSpellId(0)
{
}

bool _SpellScript::ValidateSpellInfo(std::vector<uint32> spellIds)
{
    for (auto spellId : spellIds)
        if (!sSpellMgr->GetSpellInfo(spellId))
        {
            TC_LOG_ERROR(LOG_FILTER_TSCR, "%s: Spell %u does not exist.", __FUNCTION__, spellId);
            return false;
        }

    return true;
}

void _SpellScript::_Register()
{
    m_currentScriptState = SPELL_SCRIPT_STATE_REGISTRATION;
    Register();
    m_currentScriptState = SPELL_SCRIPT_STATE_NONE;
}

void _SpellScript::_Unload()
{
    m_currentScriptState = SPELL_SCRIPT_STATE_UNLOADING;
    Unload();
    m_currentScriptState = SPELL_SCRIPT_STATE_NONE;
}

void _SpellScript::_Init(std::string const* scriptname, uint32 spellId)
{
    m_currentScriptState = SPELL_SCRIPT_STATE_NONE;
    m_scriptName = scriptname;
    m_scriptSpellId = spellId;
}

std::string const* _SpellScript::_GetScriptName() const
{
    return m_scriptName;
}

_SpellScript::EffectHook::EffectHook(uint8 _effIndex)
{
    // effect index must be in range <0;2>, allow use of special effindexes
    ASSERT(_effIndex == EFFECT_ALL || _effIndex == EFFECT_FIRST_FOUND || _effIndex < MAX_SPELL_EFFECTS);
    effIndex = _effIndex;
}

uint32 _SpellScript::EffectHook::GetAffectedEffectsMask(SpellInfo const* spellEntry)
{
    uint32 mask = 0;
    if ((effIndex == EFFECT_ALL) || (effIndex == EFFECT_FIRST_FOUND))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellEntry->EffectMask < uint32(1 << i))
                break;

            if ((effIndex == EFFECT_FIRST_FOUND) && mask)
                return mask;
            if (CheckEffect(spellEntry, i))
                mask |= 1 << i;
        }
    }
    else
    {
        if (CheckEffect(spellEntry, effIndex))
            mask |= static_cast<uint8>(1) << effIndex;
    }
    return mask;
}

bool _SpellScript::EffectHook::IsEffectAffected(SpellInfo const* spellEntry, uint8 effIndex)
{
    return (GetAffectedEffectsMask(spellEntry) & 1 << effIndex) != 0;
}

std::string _SpellScript::EffectHook::EffIndexToString()
{
    switch (effIndex)
    {
        case EFFECT_ALL:
            return "EFFECT_ALL";
        case EFFECT_FIRST_FOUND:
            return "EFFECT_FIRST_FOUND";
        case EFFECT_0:
            return "EFFECT_0";
        case EFFECT_1:
            return "EFFECT_1";
        case EFFECT_2:
            return "EFFECT_2";
        case EFFECT_3:
            return "EFFECT_3";
        case EFFECT_4:
            return "EFFECT_4";
        case EFFECT_5:
            return "EFFECT_5";
        case EFFECT_6:
            return "EFFECT_6";
        case EFFECT_7:
            return "EFFECT_7";
        case EFFECT_8:
            return "EFFECT_8";
        case EFFECT_9:
            return "EFFECT_9";
        case EFFECT_10:
            return "EFFECT_10";
        case EFFECT_11:
            return "EFFECT_11";
        case EFFECT_12:
            return "EFFECT_12";
        case EFFECT_13:
            return "EFFECT_13";
            /*case EFFECT_14:
                return "EFFECT_14";
            case EFFECT_15:
                return "EFFECT_15";
            case EFFECT_16:
                return "EFFECT_16";
            case EFFECT_17:
                return "EFFECT_17";
            case EFFECT_18:
                return "EFFECT_18";
            case EFFECT_19:
                return "EFFECT_19";
            case EFFECT_20:
                return "EFFECT_20";
            case EFFECT_21:
                return "EFFECT_21";
            case EFFECT_22:
                return "EFFECT_22";
            case EFFECT_23:
                return "EFFECT_23";
            case EFFECT_24:
                return "EFFECT_24";
            case EFFECT_25:
                return "EFFECT_25";
            case EFFECT_26:
                return "EFFECT_26";
            case EFFECT_27:
                return "EFFECT_27";
            case EFFECT_28:
                return "EFFECT_28";
            case EFFECT_29:
                return "EFFECT_29";
            case EFFECT_30:
                return "EFFECT_30";
            case EFFECT_31:
                return "EFFECT_31";*/
    }
    return "Invalid Value";
}

bool _SpellScript::EffectNameCheck::Check(SpellInfo const* spellEntry, uint8 effIndex)
{
    if (!spellEntry->Effects[effIndex]->Effect && !effName)
        return true;
    if (!spellEntry->Effects[effIndex]->Effect)
        return false;
    return (effName == SPELL_EFFECT_ANY) || (spellEntry->Effects[effIndex]->Effect == effName);
}

std::string _SpellScript::EffectNameCheck::ToString()
{
    switch (effName)
    {
        case SPELL_EFFECT_ANY:
            return "SPELL_EFFECT_ANY";
        default:
            char num[10];
            sprintf(num, "%u", effName);
            return num;
    }
}

bool _SpellScript::EffectAuraNameCheck::Check(SpellInfo const* spellEntry, uint8 effIndex)
{
    if (!spellEntry->Effects[effIndex]->ApplyAuraName && !effAurName)
        return true;
    if (!spellEntry->Effects[effIndex]->ApplyAuraName)
        return false;
    return (effAurName == SPELL_EFFECT_ANY) || (spellEntry->Effects[effIndex]->ApplyAuraName == effAurName);
}

std::string _SpellScript::EffectAuraNameCheck::ToString()
{
    switch (effAurName)
    {
        case SPELL_AURA_ANY:
            return "SPELL_AURA_ANY";
        default:
            char num[10];
            sprintf(num, "%u", effAurName);
            return num;
    }
}

SpellScript::CastHandler::CastHandler(SpellCastFnType _pCastHandlerScript)
{
    pCastHandlerScript = _pCastHandlerScript;
}

void SpellScript::CastHandler::Call(SpellScript* spellScript)
{
    (spellScript->*pCastHandlerScript)();
}

SpellScript::CheckCastHandler::CheckCastHandler(SpellCheckCastFnType checkCastHandlerScript)
{
    _checkCastHandlerScript = checkCastHandlerScript;
}

SpellCastResult SpellScript::CheckCastHandler::Call(SpellScript* spellScript)
{
    return (spellScript->*_checkCastHandlerScript)();
}

SpellScript::TakePowerHandler::TakePowerHandler(TakePowerFnType TakePowerHandlerScript)
{
    _TakePowerHandlerScript = TakePowerHandlerScript;
}

void SpellScript::TakePowerHandler::Call(SpellScript* spellScript, Powers power, int32 &amount)
{
    return (spellScript->*_TakePowerHandlerScript)(power, amount);
}

SpellScript::CalcEffectMaskHandler::CalcEffectMaskHandler(CalcEffectMaskFnType CalcEffectMaskHandlerScript)
{
    _CalcEffectMaskHandlerScript = CalcEffectMaskHandlerScript;
}

void SpellScript::CalcEffectMaskHandler::Call(SpellScript* spellScript, uint32 &effMask)
{
    return (spellScript->*_CalcEffectMaskHandlerScript)(effMask);
}

SpellScript::EffectHandler::EffectHandler(SpellEffectFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectNameCheck(_effName), EffectHook(_effIndex)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

std::string SpellScript::EffectHandler::ToString()
{
    return "Index: " + EffIndexToString() + " Name: " + EffectNameCheck::ToString();
}

bool SpellScript::EffectHandler::CheckEffect(SpellInfo const* spellEntry, uint8 effIndex)
{
    return Check(spellEntry, effIndex);
}

void SpellScript::EffectHandler::Call(SpellScript* spellScript, SpellEffIndex effIndex)
{
    (spellScript->*pEffectHandlerScript)(effIndex);
}

SpellScript::HitHandler::HitHandler(SpellHitFnType _pHitHandlerScript)
{
    pHitHandlerScript = _pHitHandlerScript;
}

void SpellScript::HitHandler::Call(SpellScript* spellScript)
{
    (spellScript->*pHitHandlerScript)();
}

SpellScript::TargetHook::TargetHook(uint8 _effectIndex, uint16 _targetType, bool _area) : EffectHook(_effectIndex), targetType(_targetType), area(_area)
{
}

std::string SpellScript::TargetHook::ToString()
{
    std::ostringstream oss;
    oss << "Index: " << EffIndexToString() << " Target: " << targetType;
    return oss.str();
}

bool SpellScript::TargetHook::CheckEffect(SpellInfo const* spellEntry, uint8 effIndex)
{
    if (!targetType)
        return false;

    if (spellEntry->Effects[effIndex]->TargetA.GetTarget() != targetType && spellEntry->Effects[effIndex]->TargetB.GetTarget() != targetType)
        return false;

    SpellImplicitTargetInfo targetInfo(targetType);
    switch (targetInfo.GetSelectionCategory())
    {
        case TARGET_SELECT_CATEGORY_CHANNEL: // SINGLE
            return !area;
        case TARGET_SELECT_CATEGORY_NEARBY: // BOTH
            return true;
        case TARGET_SELECT_CATEGORY_CONE: // AREA
        case TARGET_SELECT_CATEGORY_AREA: // AREA
        case TARGET_SELECT_CATEGORY_BETWEEN: // AREA
        case TARGET_SELECT_CATEGORY_GOTOMOVE: // AREA
        case TARGET_SELECT_CATEGORY_THREAD: // AREA
            return area;
        case TARGET_SELECT_CATEGORY_DEFAULT:
            switch (targetInfo.GetObjectType())
            {
                case TARGET_OBJECT_TYPE_SRC: // EMPTY
                case TARGET_OBJECT_TYPE_DEST: // EMPTY
                case TARGET_OBJECT_TYPE_OBJ_AND_DEST: // EMPTY
                    return false;
                default:
                    switch (targetInfo.GetReferenceType())
                    {
                        case TARGET_REFERENCE_TYPE_CASTER: // SINGLE
                            return !area;
                        case TARGET_REFERENCE_TYPE_TARGET: // BOTH
                            return true;
                        default:
                            break;
                    }
                    break;
            }
            break;
        default:
            break;
    }

    return false;
}

SpellScript::ObjectAreaTargetSelectHandler::ObjectAreaTargetSelectHandler(SpellObjectAreaTargetSelectFnType _pObjectAreaTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType) : TargetHook(_effIndex, _targetType, true)
{
    pObjectAreaTargetSelectHandlerScript = _pObjectAreaTargetSelectHandlerScript;
}

void SpellScript::ObjectAreaTargetSelectHandler::Call(SpellScript* spellScript, std::list<WorldObject*>& targets)
{
    (spellScript->*pObjectAreaTargetSelectHandlerScript)(targets);
}

SpellScript::ObjectTargetSelectHandler::ObjectTargetSelectHandler(SpellObjectTargetSelectFnType _pObjectTargetSelectHandlerScript, uint8 _effIndex, uint16 _targetType) : TargetHook(_effIndex, _targetType, false)
{
    pObjectTargetSelectHandlerScript = _pObjectTargetSelectHandlerScript;
}

void SpellScript::ObjectTargetSelectHandler::Call(SpellScript* spellScript, WorldObject*& target)
{
    (spellScript->*pObjectTargetSelectHandlerScript)(target);
}

SpellScript::ObjectJumpTargetHandler::ObjectJumpTargetHandler(SpellObjectJumpTargetFnType _pObjectJumpTargetHandlerScript, uint8 _effIndex, uint16 _targetType) : TargetHook(_effIndex, _targetType, false)
{
    pObjectJumpTargetHandlerScript = _pObjectJumpTargetHandlerScript;
}

void SpellScript::ObjectJumpTargetHandler::Call(SpellScript* spellScript, int32 & AddJumpTargets)
{
    (spellScript->*pObjectJumpTargetHandlerScript)(AddJumpTargets);
}

bool SpellScript::_Validate(SpellInfo const* entry)
{
    for (auto& itr : OnEffectLaunch)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectLaunch` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectLaunchTarget)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectLaunchTarget` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectHit)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectHit` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectHitTarget)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectHitTarget` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnObjectAreaTargetSelect)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnObjectAreaTargetSelect` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnObjectTargetSelect)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnObjectTargetSelect` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectSuccessfulDispel)
    {
        if (!itr.GetAffectedEffectsMask(entry))
			TC_LOG_DEBUG(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectSuccessfulDispel` of SpellScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    return _SpellScript::_Validate(entry);
}

bool SpellScript::_Load(Spell* spell)
{
    m_spell = spell;
    _PrepareScriptCall(static_cast<SpellScriptHookType>(SPELL_SCRIPT_STATE_LOADING));
    bool load = Load();
    _FinishScriptCall();
    return load;
}

void SpellScript::_InitHit()
{
    m_hitPreventEffectMask = 0;
    m_hitPreventDefaultEffectMask = 0;
}

void SpellScript::_PrepareScriptCall(SpellScriptHookType hookType)
{
    m_currentScriptState = hookType;
}

void SpellScript::_FinishScriptCall()
{
    m_currentScriptState = SPELL_SCRIPT_STATE_NONE;
}

bool SpellScript::IsInCheckCastHook() const
{
    return m_currentScriptState == SPELL_SCRIPT_HOOK_CHECK_CAST;
}

bool SpellScript::IsInTargetHook() const
{
    switch (m_currentScriptState)
    {
        case SPELL_SCRIPT_HOOK_EFFECT_LAUNCH_TARGET:
        case SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET:
        case SPELL_SCRIPT_HOOK_BEFORE_HIT:
        case SPELL_SCRIPT_HOOK_HIT:
        case SPELL_SCRIPT_HOOK_AFTER_HIT:
        case SPELL_SCRIPT_HOOK_EFFECT_SUCCESSFUL_DISPEL:
            return true;
    }
    return false;
}

bool SpellScript::IsInHitPhase() const
{
    return (m_currentScriptState >= HOOK_SPELL_HIT_START && m_currentScriptState < HOOK_SPELL_HIT_END);
}

bool SpellScript::IsInEffectHook() const
{
    return (m_currentScriptState >= SPELL_SCRIPT_HOOK_EFFECT_LAUNCH && m_currentScriptState <= SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET) || m_currentScriptState == SPELL_SCRIPT_HOOK_EFFECT_SUCCESSFUL_DISPEL;
}

Unit* SpellScript::GetCaster()
{
    return m_spell->GetCaster();
}

Unit* SpellScript::GetOriginalCaster()
{
    return m_spell->GetOriginalCaster();
}

SpellInfo const* SpellScript::GetSpellInfo()
{
    return m_spell->GetSpellInfo();
}

uint32 SpellScript::GetId() const
{
    return m_spell->GetSpellInfo()->Id;
}

WorldLocation const* SpellScript::GetExplTargetDest()
{
    if (m_spell->m_targets.HasDst())
        return m_spell->m_targets.GetDstPos();
    return nullptr;
}

void SpellScript::SetExplTargetDest(WorldLocation& loc)
{
    m_spell->m_targets.SetDst(loc);
}

WorldObject* SpellScript::GetExplTargetWorldObject()
{
    return m_spell->m_targets.GetObjectTarget();
}

Unit* SpellScript::GetExplTargetUnit()
{
    return m_spell->m_targets.GetUnitTarget();
}

GameObject* SpellScript::GetExplTargetGObj()
{
    return m_spell->m_targets.GetGOTarget();
}

Item* SpellScript::GetExplTargetItem()
{
    return m_spell->m_targets.GetItemTarget();
}

Unit* SpellScript::GetHitUnit()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitUnit was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    return m_spell->unitTarget;
}

Unit* SpellScript::GetOriginalTarget()
{
    return m_spell->m_originalTarget;
}

Creature* SpellScript::GetHitCreature()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitCreature was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    if (m_spell->unitTarget)
        return m_spell->unitTarget->ToCreature();
    return nullptr;
}

Player* SpellScript::GetHitPlayer()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitPlayer was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    if (m_spell->unitTarget)
        return m_spell->unitTarget->ToPlayer();
    return nullptr;
}

Item* SpellScript::GetHitItem()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitItem was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    return m_spell->itemTarget;
}

GameObject* SpellScript::GetHitGObj()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitGObj was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    return m_spell->gameObjTarget;
}

WorldLocation* SpellScript::GetHitDest()
{
    if (!IsInEffectHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDest was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    return m_spell->destTarget;
}

int32 SpellScript::GetHitDamage()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_damage;
}

int32 SpellScript::GetHitAbsorb()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_absorb;
}

int32 SpellScript::GetHitResist()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_resist;
}

int32 SpellScript::GetHitBlocked()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_blocked;
}

int32 SpellScript::GetFinalHitDamage()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_final_damage;
}

int32 SpellScript::GetAbsorbDamage()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetAbsorbDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_absorb;
}

void SpellScript::SetHitDamage(int32 damage)
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::SetHitDamage was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    m_spell->m_damage = damage;
}

int32 SpellScript::GetHitHeal()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitHeal was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->m_healing;
}

void SpellScript::SetHitHeal(int32 heal)
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::SetHitHeal was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    m_spell->m_healing = heal;
}

Aura* SpellScript::GetHitAura()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::GetHitAura was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return nullptr;
    }
    if (!m_spell->m_spellAura)
        return nullptr;
    if (m_spell->m_spellAura->IsRemoved())
        return nullptr;
    return m_spell->m_spellAura;
}

void SpellScript::PreventHitAura()
{
    if (!IsInTargetHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::PreventHitAura was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    if (m_spell->m_spellAura)
        m_spell->m_spellAura->Remove();
}

void SpellScript::PreventHitEffect(SpellEffIndex effIndex)
{
    if (!IsInHitPhase() && !IsInEffectHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::PreventHitEffect was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    m_hitPreventEffectMask |= 1 << effIndex;
    PreventHitDefaultEffect(effIndex);
}

void SpellScript::PreventHitDefaultEffect(SpellEffIndex effIndex)
{
    if (!IsInHitPhase() && !IsInEffectHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::PreventHitDefaultEffect was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    m_hitPreventDefaultEffectMask |= 1 << effIndex;
}

int32 SpellScript::GetEffectValue()
{
    if (!IsInEffectHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::PreventHitDefaultEffect was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return 0;
    }
    return m_spell->damage;
}

void SpellScript::SetEffectValue(int32 val)
{
    if (!IsInEffectHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::SetEffectValue was called, but function has no effect in current hook!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }
    m_spell->damage = val;
}

Item* SpellScript::GetCastItem()
{
    return m_spell->m_CastItem;
}

void SpellScript::CreateItem(uint32 effIndex, uint32 itemId)
{
    m_spell->DoCreateItem(effIndex, itemId);
}

SpellInfo const* SpellScript::GetTriggeringSpell()
{
    return m_spell->m_triggeredByAuraSpell;
}

AuraEffect const* SpellScript::GetTriggeredAuraEff()
{
    return m_spell->m_triggeredByAura;
}

void SpellScript::FinishCast(SpellCastResult result)
{
    m_spell->SendCastResult(result);
    m_spell->finish(result == SPELL_CAST_OK);
}

void SpellScript::SetCustomCastResultMessage(SpellCustomErrors result)
{
    if (!IsInCheckCastHook())
    {
        TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u`: function SpellScript::SetCustomCastResultMessage was called while spell not in check cast phase!", m_scriptName->c_str(), m_scriptSpellId);
        return;
    }

    m_spell->m_customError = result;
}

SpellValue const* SpellScript::GetSpellValue()
{
    return m_spell->m_spellValue;
}

bool AuraScript::_Validate(SpellInfo const* entry)
{
    for (auto itr = DoCheckTargetsList.begin(); itr != DoCheckTargetsList.end(); ++itr)
    {
        if (!entry->HasDynAuraEffect())
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` of script `%s` does not have area aura effect - handler bound to hook `DoCheckTargetsList` of AuraScript won't be executed", entry->Id, m_scriptName->c_str());
    }

    for (auto itr = DoCheckAreaTarget.begin(); itr != DoCheckAreaTarget.end(); ++itr)
    {
        if (!entry->HasAreaAuraEffect())
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` of script `%s` does not have area aura effect - handler bound to hook `DoCheckAreaTarget` of AuraScript won't be executed", entry->Id, m_scriptName->c_str());
    }

    for (auto itr = OnDispel.begin(); itr != OnDispel.end(); ++itr)
    {
        if (!entry->HasEffect(SPELL_EFFECT_APPLY_AURA) && !entry->HasAreaAuraEffect())
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` of script `%s` does not have apply aura effect - handler bound to hook `OnDispel` of AuraScript won't be executed", entry->Id, m_scriptName->c_str());
    }

    for (auto itr = AfterDispel.begin(); itr != AfterDispel.end(); ++itr)
    {
        if (!entry->HasEffect(SPELL_EFFECT_APPLY_AURA) && !entry->HasAreaAuraEffect())
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` of script `%s` does not have apply aura effect - handler bound to hook `AfterDispel` of AuraScript won't be executed", entry->Id, m_scriptName->c_str());
    }

    for (auto& itr : OnEffectApply)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectApply` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectRemove)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectRemove` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : AfterEffectApply)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `AfterEffectApply` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : AfterEffectRemove)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `AfterEffectRemove` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectPeriodic)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectPeriodic` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectUpdatePeriodic)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectUpdatePeriodic` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : DoEffectCalcAmount)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `DoEffectCalcAmount` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : DoEffectBeforeCalcAmount)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `DoEffectBeforeCalcAmount` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : DoEffectCalcPeriodic)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `DoEffectCalcPeriodic` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : DoEffectCalcSpellMod)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `DoEffectCalcSpellMod` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectAbsorb)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectAbsorb` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : AfterEffectAbsorb)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `AfterEffectAbsorb` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectManaShield)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectManaShield` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : AfterEffectManaShield)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `AfterEffectManaShield` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    for (auto& itr : OnEffectProc)
    {
        if (!itr.GetAffectedEffectsMask(entry))
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Spell `%u` Effect `%s` of script `%s` did not match dbc effect data - handler bound to hook `OnEffectProc` of AuraScript won't be executed", entry->Id, itr.ToString().c_str(), m_scriptName->c_str());
    }

    return _SpellScript::_Validate(entry);
}

AuraScript::CheckTargetsListHandler::CheckTargetsListHandler(AuraCheckTargetsListFnType _pHandlerScript)
{
    pHandlerScript = _pHandlerScript;
}

void AuraScript::CheckTargetsListHandler::Call(AuraScript* auraScript, std::list<Unit*>& _unitTargets)
{
    (auraScript->*pHandlerScript)(_unitTargets);
}

AuraScript::CheckAreaTargetHandler::CheckAreaTargetHandler(AuraCheckAreaTargetFnType _pHandlerScript)
{
    pHandlerScript = _pHandlerScript;
}

bool AuraScript::CheckAreaTargetHandler::Call(AuraScript* auraScript, Unit* _target)
{
    return (auraScript->*pHandlerScript)(_target);
}

AuraScript::CalcMaxDurationHandler::CalcMaxDurationHandler(AuraCalcMaxDurationFnType _pHandlerScript)
{
    pHandlerScript = _pHandlerScript;
}

void AuraScript::CalcMaxDurationHandler::Call(AuraScript* auraScript, int32& maxDuration)
{
    (auraScript->*pHandlerScript)(maxDuration);
}

AuraScript::AuraDispelHandler::AuraDispelHandler(AuraDispelFnType _pHandlerScript)
{
    pHandlerScript = _pHandlerScript;
}

void AuraScript::AuraDispelHandler::Call(AuraScript* auraScript, DispelInfo* _dispelInfo)
{
    (auraScript->*pHandlerScript)(_dispelInfo);
}

AuraScript::EffectBase::EffectBase(uint8 _effIndex, uint16 _effName) : EffectAuraNameCheck(_effName), EffectHook(_effIndex)
{
}

bool AuraScript::EffectBase::CheckEffect(SpellInfo const* spellEntry, uint8 effIndex)
{
    return Check(spellEntry, effIndex);
}

std::string AuraScript::EffectBase::ToString()
{
    return "Index: " + EffIndexToString() + " AuraName: " + EffectAuraNameCheck::ToString();
}

AuraScript::EffectPeriodicHandler::EffectPeriodicHandler(AuraEffectPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectPeriodicHandler::Call(AuraScript* auraScript, AuraEffect const* _aurEff)
{
    (auraScript->*pEffectHandlerScript)(_aurEff);
}

AuraScript::EffectUpdateHandler::EffectUpdateHandler(AuraEffectUpdateFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectUpdateHandler::Call(AuraScript* auraScript, uint32 diff, AuraEffect* aurEff)
{
    (auraScript->*pEffectHandlerScript)(diff, aurEff);
}

AuraScript::EffectUpdatePeriodicHandler::EffectUpdatePeriodicHandler(AuraEffectUpdatePeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectUpdatePeriodicHandler::Call(AuraScript* auraScript, AuraEffect* aurEff)
{
    (auraScript->*pEffectHandlerScript)(aurEff);
}

AuraScript::EffectCalcAmountHandler::EffectCalcAmountHandler(AuraEffectCalcAmountFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectCalcAmountHandler::Call(AuraScript* auraScript, AuraEffect const* aurEff, float& amount, bool& canBeRecalculated)
{
    (auraScript->*pEffectHandlerScript)(aurEff, amount, canBeRecalculated);
}

AuraScript::EffectChangeTickDamageHandler::EffectChangeTickDamageHandler(AuraEffectChangeTickDamageFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectChangeTickDamageHandler::Call(AuraScript* auraScript, AuraEffect const* aurEff, float& amount, Unit* target)
{
    (auraScript->*pEffectHandlerScript)(aurEff, amount, target);
}

AuraScript::EffectCalcPeriodicHandler::EffectCalcPeriodicHandler(AuraEffectCalcPeriodicFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectCalcPeriodicHandler::Call(AuraScript* auraScript, AuraEffect const* aurEff, bool& isPeriodic, int32& periodicTimer)
{
    (auraScript->*pEffectHandlerScript)(aurEff, isPeriodic, periodicTimer);
}

AuraScript::EffectCalcSpellModHandler::EffectCalcSpellModHandler(AuraEffectCalcSpellModFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectCalcSpellModHandler::Call(AuraScript* auraScript, AuraEffect const* aurEff, SpellModifier*& spellMod)
{
    (auraScript->*pEffectHandlerScript)(aurEff, spellMod);
}

AuraScript::EffectApplyHandler::EffectApplyHandler(AuraEffectApplicationModeFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName, AuraEffectHandleModes _mode) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
    mode = _mode;
}

AuraScript::AuraUpdateHandler::AuraUpdateHandler(AuraUpdateFnType _pEffectHandlerScript)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::AuraUpdateHandler::Call(AuraScript* auraScript, uint32 diff)
{
    (auraScript->*pEffectHandlerScript)(diff);
}

void AuraScript::EffectApplyHandler::Call(AuraScript* auraScript, AuraEffect const* _aurEff, AuraEffectHandleModes _mode)
{
    if (_mode & mode)
        (auraScript->*pEffectHandlerScript)(_aurEff, _mode);
}

AuraScript::EffectAbsorbHandler::EffectAbsorbHandler(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex, uint16 _effName) : EffectBase(_effIndex, _effName)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectAbsorbHandler::Call(AuraScript* auraScript, AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
{
    (auraScript->*pEffectHandlerScript)(aurEff, dmgInfo, absorbAmount);
}

AuraScript::EffectManaShieldHandler::EffectManaShieldHandler(AuraEffectAbsorbFnType _pEffectHandlerScript, uint8 _effIndex) : EffectBase(_effIndex, SPELL_AURA_MANA_SHIELD)
{
    pEffectHandlerScript = _pEffectHandlerScript;
}

void AuraScript::EffectManaShieldHandler::Call(AuraScript* auraScript, AuraEffect* aurEff, DamageInfo& dmgInfo, float& absorbAmount)
{
    (auraScript->*pEffectHandlerScript)(aurEff, dmgInfo, absorbAmount);
}

AuraScript::EffectProcHandler::EffectProcHandler(AuraEffectProcFnType effectHandlerScript, uint8 effIndex, uint16 effName) : EffectBase(effIndex, effName)
{
    _EffectHandlerScript = effectHandlerScript;
}

void AuraScript::EffectProcHandler::Call(AuraScript* auraScript, AuraEffect const* aurEff, ProcEventInfo& eventInfo)
{
    (auraScript->*_EffectHandlerScript)(aurEff, eventInfo);
}

bool AuraScript::_Load(Aura* aura)
{
    m_aura = aura;
    _PrepareScriptCall(static_cast<AuraScriptHookType>(SPELL_SCRIPT_STATE_LOADING), nullptr);
    bool load = Load();
    _FinishScriptCall();
    return load;
}

void AuraScript::_PrepareScriptCall(AuraScriptHookType hookType, AuraApplication const* aurApp)
{
    i_scriptStatesLock.lock();
    m_scriptStates.push(ScriptStateStore(m_currentScriptState, m_auraApplication, m_defaultActionPrevented));
    i_scriptStatesLock.unlock();
    m_currentScriptState = hookType;
    m_defaultActionPrevented = false;
    m_auraApplication = aurApp;
}

void AuraScript::_FinishScriptCall()
{
    i_scriptStatesLock.lock();
    ScriptStateStore stateStore = m_scriptStates.top();
    m_currentScriptState = stateStore._currentScriptState;
    m_auraApplication = stateStore._auraApplication;
    m_defaultActionPrevented = stateStore._defaultActionPrevented;
    m_scriptStates.pop();
    i_scriptStatesLock.unlock();
}

bool AuraScript::_IsDefaultActionPrevented()
{
    switch (m_currentScriptState)
    {
        case AURA_SCRIPT_HOOK_EFFECT_APPLY:
        case AURA_SCRIPT_HOOK_EFFECT_REMOVE:
        case AURA_SCRIPT_HOOK_EFFECT_PERIODIC:
        case AURA_SCRIPT_HOOK_EFFECT_ABSORB:
        case AURA_SCRIPT_HOOK_PREPARE_PROC:
        case AURA_SCRIPT_HOOK_EFFECT_PROC:
            return m_defaultActionPrevented;
        default:
            ASSERT(false && "AuraScript::_IsDefaultActionPrevented is called in a wrong place");
            return false;
    }
}

void AuraScript::PreventDefaultAction()
{
    switch (m_currentScriptState)
    {
        case AURA_SCRIPT_HOOK_EFFECT_APPLY:
        case AURA_SCRIPT_HOOK_EFFECT_REMOVE:
        case AURA_SCRIPT_HOOK_EFFECT_PERIODIC:
        case AURA_SCRIPT_HOOK_EFFECT_ABSORB:
        case AURA_SCRIPT_HOOK_PREPARE_PROC:
        case AURA_SCRIPT_HOOK_EFFECT_PROC:
            m_defaultActionPrevented = true;
            break;
        default:
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u` AuraScript::PreventDefaultAction called in a hook in which the call won't have effect!", m_scriptName->c_str(), m_scriptSpellId);
            break;
    }
}

SpellInfo const* AuraScript::GetSpellInfo() const
{
    return m_aura->GetSpellInfo();
}

uint32 AuraScript::GetId() const
{
    return m_aura->GetId();
}

ObjectGuid AuraScript::GetCasterGUID() const
{
    return m_aura->GetCasterGUID();
}

Unit* AuraScript::GetCaster() const
{
    return m_aura->GetCaster();
}

WorldObject* AuraScript::GetOwner() const
{
    return m_aura->GetOwner();
}

Unit* AuraScript::GetUnitOwner() const
{
    return m_aura->GetUnitOwner();
}

DynamicObject* AuraScript::GetDynobjOwner() const
{
    return m_aura->GetDynobjOwner();
}

Position const* AuraScript::GetExplTargetDest()
{
    return m_aura->GetDstPos();
}

void AuraScript::Remove(uint32 removeMode)
{
    m_aura->Remove(static_cast<AuraRemoveMode>(removeMode));
}

Aura* AuraScript::GetAura() const
{
    return m_aura;
}

AuraObjectType AuraScript::GetType() const
{
    return m_aura->GetType();
}

int32 AuraScript::GetDuration() const
{
    return m_aura->GetDuration();
}

void AuraScript::SetDuration(int32 duration, bool withMods)
{
    m_aura->SetDuration(duration, withMods);
}

void AuraScript::SetAuraTimer(int32 time, ObjectGuid guid)
{
    m_aura->SetAuraTimer(time, guid);
}

void AuraScript::RefreshDuration()
{
    m_aura->RefreshDuration();
}

time_t AuraScript::GetApplyTime() const
{
    return m_aura->GetApplyTime();
}

int32 AuraScript::GetMaxDuration() const
{
    return m_aura->GetMaxDuration();
}

void AuraScript::SetMaxDuration(int32 duration)
{
    m_aura->SetMaxDuration(duration);
}

int32 AuraScript::CalcMaxDuration() const
{
    return m_aura->CalcMaxDuration();
}

bool AuraScript::IsExpired() const
{
    return m_aura->IsExpired();
}

bool AuraScript::IsPermanent() const
{
    return m_aura->IsPermanent();
}

uint8 AuraScript::GetCharges() const
{
    return m_aura->GetCharges();
}

void AuraScript::SetCharges(uint8 charges)
{
    m_aura->SetCharges(charges);
}

uint8 AuraScript::CalcMaxCharges() const
{
    return m_aura->CalcMaxCharges();
}

bool AuraScript::ModCharges(int8 num, AuraRemoveMode removeMode /*= AURA_REMOVE_BY_DEFAULT*/)
{
    return m_aura->ModCharges(num, removeMode);
}

bool AuraScript::DropCharge(AuraRemoveMode removeMode)
{
    return m_aura->DropCharge(removeMode);
}

uint16 AuraScript::GetStackAmount() const
{
    return m_aura->GetStackAmount();
}

void AuraScript::SetStackAmount(uint16 num)
{
    m_aura->SetStackAmount(num);
}

bool AuraScript::ModStackAmount(int16 num, AuraRemoveMode removeMode)
{
    return m_aura->ModStackAmount(num, removeMode);
}

bool AuraScript::IsPassive() const
{
    return m_aura->IsPassive();
}

bool AuraScript::IsDeathPersistent() const
{
    return m_aura->IsDeathPersistent();
}

bool AuraScript::HasEffect(uint8 effIndex) const
{
    return m_aura->HasEffect(effIndex);
}

AuraEffect* AuraScript::GetEffect(uint8 effIndex) const
{
    return m_aura->GetEffect(effIndex);
}

bool AuraScript::HasEffectType(AuraType type) const
{
    return m_aura->HasEffectType(type);
}

Unit* AuraScript::GetTarget() const
{
    switch (m_currentScriptState)
    {
        case AURA_SCRIPT_HOOK_EFFECT_APPLY:
        case AURA_SCRIPT_HOOK_EFFECT_REMOVE:
        case AURA_SCRIPT_HOOK_EFFECT_AFTER_APPLY:
        case AURA_SCRIPT_HOOK_EFFECT_AFTER_REMOVE:
        case AURA_SCRIPT_HOOK_EFFECT_PERIODIC:
        case AURA_SCRIPT_HOOK_EFFECT_ABSORB:
        case AURA_SCRIPT_HOOK_EFFECT_AFTER_ABSORB:
        case AURA_SCRIPT_HOOK_EFFECT_MANASHIELD:
        case AURA_SCRIPT_HOOK_EFFECT_AFTER_MANASHIELD:
        case AURA_SCRIPT_HOOK_EFFECT_SPLIT_DAMAGE:
        case AURA_SCRIPT_HOOK_CHECK_PROC:
        case AURA_SCRIPT_HOOK_PREPARE_PROC:
        case AURA_SCRIPT_HOOK_PROC:
        case AURA_SCRIPT_HOOK_AFTER_PROC:
        case AURA_SCRIPT_HOOK_EFFECT_PROC:
        case AURA_SCRIPT_HOOK_EFFECT_AFTER_PROC:
            return m_auraApplication->GetTarget();
        default:
            TC_LOG_ERROR(LOG_FILTER_TSCR, "Script: `%s` Spell: `%u` AuraScript::GetTarget called in a hook in which the call won't have effect!", m_scriptName->c_str(), m_scriptSpellId);
    }

    return nullptr;
}

AuraApplication const* AuraScript::GetTargetApplication() const
{
    return m_auraApplication;
}
