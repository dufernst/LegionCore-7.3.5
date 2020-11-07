/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "Battleground.h"
#include "CellImpl.h"
#include "CharmInfo.h"
#include "CombatLogPackets.h"
#include "Common.h"
#include "ConditionMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "DynamicObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GuildMgr.h"
#include "InstanceScript.h"
#include "Log.h"
#include "LootMgr.h"
#include "MapInstanced.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "PathGenerator.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerDefines.h"
#include "ScenarioMgr.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpectatorAddon.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "TradeData.h"
#include "Unit.h"
#include "UpdateData.h"
#include "Util.h"
#include "Vehicle.h"
#include "VMapFactory.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#define SPELL_SEARCHER_COMPENSATION 30.0f

extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];



SpellValue::SpellValue(SpellInfo const* proto, uint8 diff)
{
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (proto->EffectMask < uint32(1 << i))
            break;

        EffectBasePoints[i] = proto->GetEffect(i, diff)->BasePoints;
        LockBasePoints[i] = false;
    }
    MaxAffectedTargets = proto->GetMaxAffectedTargets(diff);
    RadiusMod = 1.0f;
    AuraStackAmount = 1;
}

Spell::Spell(Unit* caster, SpellInfo const* info, TriggerCastData& triggerData) :
m_spellInfo(info),
m_CastItem(triggerData.castItem),
m_preCastSpell(0), m_SpellVisual(0), m_customError(SPELL_CUSTOM_ERROR_NONE), m_count_dispeling(0),
m_interupted(false), m_replaced(triggerData.replaced), mCriticalDamageBonus(0.f),
m_canLostTarget(true), m_caster((info->HasAttribute(SPELL_ATTR6_CAST_BY_CHARMER) && caster->GetCharmerOrOwner()) ? caster->GetCharmerOrOwner() : caster), m_spellValue(nullptr), m_originalTarget(nullptr), m_selfContainer(nullptr),
m_casttime(0), m_castedTime(triggerData.casttime), m_runesState(0), m_triggerData(triggerData), m_delayAtDamageCount(0), m_delayStart(0),
m_referencedFromCurrentSpell(false), m_executedCurrently(false), hasPredictedDispel(0), m_applyMultiplierMask(0),
unitTarget(nullptr), itemTarget(nullptr), gameObjTarget(nullptr), m_spellAura(nullptr), focusObject(nullptr), m_absorb(0), m_resist(0),
m_blocked(0), m_channelTargetEffectMask(0), m_spellState(SPELL_STATE_NULL), m_timer(0), _triggeredCastFlags(triggerData.triggerFlags), m_dispelResetCD(false), m_triggeredByAuraSpell(nullptr), m_triggeredByAura(nullptr),
m_skipCheck(triggerData.skipCheck), m_spellMissMask(0), m_auraScaleMask(0), m_currentExecutedEffect(SPELL_EFFECT_NONE)
{
    if (!m_castedTime)
        m_castedTime = getMSTime();

    m_delayMoment = triggerData.delay;
    m_spellGuid = triggerData.spellGuid;
    m_parentTargetCount = triggerData.parentTargetCount;

    m_castPos = *m_caster;
    m_diffMode = m_caster->GetMap() ? m_caster->GetMap()->GetSpawnMode() : 0;
    m_spellValue = new SpellValue(m_spellInfo, m_diffMode);

    if (!triggerData.powerCost.empty())
        m_powerCost = triggerData.powerCost;
    else
        m_powerCost.assign(MAX_POWERS + 1, 0);

    // Get data for type of attack
    switch (m_spellInfo->Categories.DefenseType)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
            if (m_spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND) && !m_spellInfo->HasAttribute(SPELL_ATTR3_MAIN_HAND))
                m_attackType = OFF_ATTACK;
            else
                m_attackType = BASE_ATTACK;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            m_attackType = m_spellInfo->IsRangedWeaponSpell() ? RANGED_ATTACK : BASE_ATTACK;
            break;
        default:
                                                            // Wands
            if (m_spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
                m_attackType = RANGED_ATTACK;
            else
                m_attackType = BASE_ATTACK;
            break;
    }

    m_spellSchoolMask = info->GetSchoolMask();           // Can be override for some spell (wand shoot for example)

    if (!triggerData.originalCaster.IsEmpty())
        m_originalCasterGUID = triggerData.originalCaster;
    else
        m_originalCasterGUID = m_caster->GetGUID();

    if (m_originalCasterGUID == m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
    {
        m_originalCaster = ObjectAccessor::GetUnit(*m_caster, m_originalCasterGUID);
        if (m_originalCaster && !m_originalCaster->IsInWorld())
            m_originalCaster = nullptr;
    }

    if (triggerData.triggeredByAura)
    {
        m_triggeredByAuraSpell  = triggerData.triggeredByAura->GetSpellInfo();
        m_triggeredByAura = triggerData.triggeredByAura;
    }

    if (info->HasAttribute(SPELL_ATTR4_TRIGGERED) && !m_CastItem)
    {
        bool fullTrigger = (_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD);
        _triggeredCastFlags |= TRIGGERED_FULL_MASK;
        _triggeredCastFlags &= ~TRIGGERED_IGNORE_POWER_AND_REAGENT_COST; // if attr full trigger cost use example 202968
        if (!fullTrigger)
        {
            _triggeredCastFlags &= ~TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD;
            _triggeredCastFlags &= ~TRIGGERED_IGNORE_LOS;
        }
    }

    if (m_replaced || m_spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG)) //If spell casted as replaced, enable proc from him
        _triggeredCastFlags &= ~TRIGGERED_DISALLOW_PROC_EVENTS;

    if (m_replaced)
        _triggeredCastFlags &= ~TRIGGERED_CAST_DIRECTLY;

    //Auto Shot & Shoot (wand)
    m_autoRepeat = m_spellInfo->IsAutoRepeatRangedSpell();

    m_castedFromStealth = m_caster->HasStealthAura();

    canHitTargetInLOS = m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) || (_triggeredCastFlags & TRIGGERED_IGNORE_LOS);

    // Special ignore check. Example creature entry: 99801 (Maw of Souls)
    if (m_caster->IsCreature() && m_caster->ToCreature()->IsIgnoreLos())
        canHitTargetInLOS = true;

    // Determine if spell can be reflected back to the caster
    // Patch 1.2 notes: Spell Reflection no longer reflects abilities
    m_canReflect = m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MAGIC && !m_spellInfo->HasAttribute(SPELL_ATTR0_ABILITY)
        && !m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_BE_REFLECTED) && !m_spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)
        && !m_spellInfo->HasAttribute(SPELL_ATTR0_CU_IGNORE_AVOID_MECHANIC) && !m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS)
        && !m_spellInfo->IsPassive() && !m_spellInfo->IsPositive();

    CleanupTargetList();

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        damageCalculate[i] = false;
        saveDamageCalculate[i] = 0.f;
        m_destTargets[i] = SpellDestination(*m_caster);
    }

    variance = 0.0f;
    m_damage = 0;
    m_healing = 0;
    m_final_damage = 0;
    m_absorb = 0;
    m_resist = 0;
    m_blocked = 0;
    m_addpower = 0;
    m_addptype = -1;
    m_castItemEntry = 0;
    m_combatItemEntry = 0;
    m_failedArg[0] = -1;
    m_failedArg[1] = -1;
    m_isDamageSpell = false;

    m_caster->GetPosition(&visualPos);

    memset(m_miscData, 0, sizeof(m_miscData));
    memset(m_castFlags, 0, sizeof(m_castFlags));

    m_miscData[0] = triggerData.miscData0;
    m_miscData[1] = triggerData.miscData1;

    m_castGuid[0] = ObjectGuid::Create<HighGuid::Cast>(m_caster->GetMapId(), m_spellInfo->Id, sObjectMgr->GetGenerator<HighGuid::Cast>()->Generate(), triggerData.SubType);
    m_castGuid[1] = m_spellGuid;

    objectCountInWorld[uint8(HighGuid::Spell)]++;
    spellCountInWorld[m_spellInfo->Id]++;
    m_caster->_castCount++;
}

Spell::~Spell()
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        _powerDrainTargets[i].clear();
        _extraAttacksTargets[i].clear();
        _durabilityDamageTargets[i].clear();
        _genericVictimTargets[i].clear();
        _tradeSkillTargets[i].clear();
        _feedPetTargets[i].clear();
    }

    // volatile uint32 SpellID = m_spellInfo->Id;

    // unload scripts
    for (auto itr = m_loadedScripts.begin(); itr != m_loadedScripts.end(); ++itr)
    {
        (*itr)->_Unload();
        delete *itr;
    }

    if (m_referencedFromCurrentSpell && m_selfContainer && *m_selfContainer == this)
    {
        // Clean the reference to avoid later crash.
        // If this error is repeating, we may have to add an ASSERT to better track down how we get into this case.
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "SPELL: deleting spell for spell ID %u. However, spell still referenced.", m_spellInfo->Id);
        *m_selfContainer = nullptr;
    }

    if (m_caster && m_caster->IsPlayer())
        ASSERT(m_caster->ToPlayer()->m_spellModTakingSpell != this);
    delete m_spellValue;

    objectCountInWorld[uint8(HighGuid::Spell)]--;
    spellCountInWorld[m_spellInfo->Id]--;
}

void Spell::InitExplicitTargets(SpellCastTargets const& targets)
{
    m_targets = targets;
    m_originalTarget = targets.GetUnitTarget();
    m_originalTargetGUID = m_originalTarget ? m_originalTarget->GetGUID() : ObjectGuid::Empty;
    // this function tries to correct spell explicit targets for spell
    // client doesn't send explicit targets correctly sometimes - we need to fix such spells serverside
    // this also makes sure that we correctly send explicit targets to client (removes redundant data)
    uint32 neededTargets = m_spellInfo->GetExplicitTargetMask();

    if (WorldObject* target = m_targets.GetObjectTarget())
    {
        // check if object target is valid with needed target flags
        // for unit case allow corpse target mask because player with not released corpse is a unit target
        if ((target->IsUnit() && !(neededTargets & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK))) || (target->ToGameObject() && !(neededTargets & TARGET_FLAG_GAMEOBJECT_MASK)) || (target->ToCorpse() && !(neededTargets & TARGET_FLAG_CORPSE_MASK)))
            m_targets.RemoveObjectTarget();
    }
    else
    {
        // try to select correct unit target if not provided by client or by serverside cast
        if (neededTargets & (TARGET_FLAG_UNIT_MASK))
        {
            Unit* unit = nullptr;
            // try to use player selection as a target
            if (Player* playerCaster = m_caster->ToPlayer())
            {
                // selection has to be found and to be valid target for the spell
                if (Unit* selectedUnit = ObjectAccessor::GetUnit(*m_caster, playerCaster->GetSelection()))
                    if (m_spellInfo->CheckExplicitTarget(m_caster, selectedUnit) == SPELL_CAST_OK)
                        unit = selectedUnit;
            }
            // try to use attacked unit as a target
            else if ((m_caster->IsCreature()) && neededTargets & (TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT))
                unit = m_caster->getVictim();

            // didn't find anything - let's use self as target
            if (!unit && neededTargets & (TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY | TARGET_FLAG_UNIT_ALLY))
                unit = m_caster;

            if(unit)
                m_targets.SetUnitTarget(unit);
        }
    }

    // check if spell needs dst target
    if (neededTargets & TARGET_FLAG_DEST_LOCATION)
    {
        // and target isn't set
        if (!m_targets.HasDst())
        {
            // try to use unit target if provided
            if (WorldObject* target = targets.GetObjectTarget())
                m_targets.SetDst(*target);
            // or use self if not available
            else
                m_targets.SetDst(*m_caster);
        }
    }
    else
        m_targets.RemoveDst();

    if (neededTargets & TARGET_FLAG_SOURCE_LOCATION)
    {
        if (!targets.HasSrc())
            m_targets.SetSrc(*m_caster);
    }
    else
        m_targets.RemoveSrc();
}

void Spell::SelectExplicitTargets()
{
    // here go all explicit target changes made to explicit targets after spell prepare phase is finished
    if (Unit* target = m_targets.GetUnitTarget())
    {
        // check for explicit target redirection, for Grounding Totem for example
        if (m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT_ENEMY || (m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT && !m_spellInfo->IsPositive()))
        {
            Unit* redirect;
            switch (m_spellInfo->Categories.DefenseType)
            {
                case SPELL_DAMAGE_CLASS_MAGIC:
                    redirect = m_caster->GetMagicHitRedirectTarget(target, m_spellInfo);
                    break;
                case SPELL_DAMAGE_CLASS_MELEE:
                case SPELL_DAMAGE_CLASS_RANGED:
                    redirect = m_caster->GetMeleeHitRedirectTarget(target, m_spellInfo);
                    break;
                default:
                    redirect = nullptr;
                    break;
            }
            if (redirect && (redirect != target))
            {
                canHitTargetInLOS = true;
                m_magnetGuid = target->GetGUID();
                m_targets.SetUnitTarget(redirect);
            }
        }
    }
}

void Spell::SelectSpellTargets()
{
    // select targets for cast phase
    SelectExplicitTargets();

    uint32 processedAreaEffectsMask = 0;
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if ((m_spellInfo->EffectMask & (1 << i)) == 0)
            continue;
        // not call for empty effect.
        // Also some spells use not used effect targets for store targets for dummy effect in triggered spells
        if (!m_spellInfo->GetEffect(i, m_diffMode)->IsEffect())
            continue;

        if (m_spellMissMask & ((1 << SPELL_MISS_MISS) | (1 << SPELL_MISS_IMMUNE) | (1 << SPELL_MISS_DODGE) | (1 << SPELL_MISS_PARRY)))
            if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_CASTER)
                continue;

        // set expected type of implicit targets to be sent to client
        uint32 implicitTargetMask = GetTargetFlagMask(m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetObjectType()) | GetTargetFlagMask(m_spellInfo->GetEffect(i, m_diffMode)->TargetB.GetObjectType());
        if (implicitTargetMask & TARGET_FLAG_UNIT)
            m_targets.SetTargetFlag(TARGET_FLAG_UNIT);
        if (implicitTargetMask & (TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_GAMEOBJECT_ITEM))
            m_targets.SetTargetFlag(TARGET_FLAG_GAMEOBJECT);

        SelectEffectImplicitTargets(SpellEffIndex(i), m_spellInfo->GetEffect(i, m_diffMode)->TargetA, processedAreaEffectsMask);
        SelectEffectImplicitTargets(SpellEffIndex(i), m_spellInfo->GetEffect(i, m_diffMode)->TargetB, processedAreaEffectsMask);

        // Select targets of effect based on effect type
        // those are used when no valid target could be added for spell effect based on spell target type
        // some spell effects use explicit target as a default target added to target map (like SPELL_EFFECT_LEARN_SPELL)
        // some spell effects add target to target map only when target type specified (like SPELL_EFFECT_WEAPON)
        SelectEffectTypeImplicitTargets(i);

        if (m_targets.HasDst())
            AddDestTarget(*m_targets.GetDst(), i);
        
        for (auto& ihit : m_UniqueTargetInfo)
            m_spellMissMask |= (1 << ihit->missCondition);

        if (m_spellInfo->IsChanneled())
        {
            uint8 mask = (1 << i);
            for (auto& ihit : m_UniqueTargetInfo)
            {
                if (ihit->effectMask & mask)
                {
                    m_channelTargetEffectMask |= mask;
                    break;
                }
            }
        }
        else if (m_auraScaleMask)
        {
            bool checkLvl = !m_UniqueTargetInfo.empty();
            std::vector<TargetInfoPtr> tempTarget;
            std::swap(tempTarget, m_UniqueTargetInfo);
            for (auto ihit = tempTarget.begin(); ihit != tempTarget.end();++ihit)
            {
                // remove targets which did not pass min level check
                if (m_auraScaleMask && (*ihit)->effectMask == m_auraScaleMask)
                {
                    // Do not check for selfcast
                    if (!(*ihit)->scaleAura && (*ihit)->targetGUID != m_caster->GetGUID())
                    {
                        continue;
                    }
                }
                m_UniqueTargetInfo.push_back(*ihit);
            }
            if (checkLvl && m_UniqueTargetInfo.empty())
            {
                SendCastResult(SPELL_FAILED_LOWLEVEL);
                finish(false);
            }
        }
    }

    if (m_targets.HasDst())
    {
        auto const& miscData = m_spellInfo->GetMisc(m_diffMode)->MiscData;

        if (m_targets.HasTraj())
        {
            float speed = m_targets.GetSpeedXY();
            if (speed > 0.0f)
                m_delayMoment = static_cast<uint64>(floor(m_targets.GetDist2d() / speed * 1000.0f) + (miscData.LaunchDelay * 1000.0f));
        }
        else if (miscData.Speed > 0.0f)
        {
            auto pos = static_cast<Position>(*m_targets.GetDstPos());
            auto dist = m_caster->GetExactDist(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

            if (!m_spellInfo->HasAttribute(SPELL_ATTR9_SPECIAL_DELAY_CALCULATION))
                m_delayMoment = uint64(floor((dist / miscData.Speed * 1000.0f) + (miscData.LaunchDelay * 1000.0f)));
            else
                m_delayMoment = uint64((miscData.Speed + miscData.LaunchDelay) * 1000.0f);
        }
        m_delayMoment *= m_caster->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE);
    }
}

void Spell::SelectEffectImplicitTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32& processedEffectMask)
{
    if (!targetType.GetTarget())
        return;

    uint32 effectMask = 1 << effIndex;
    // set the same target list for all effects
    // some spells appear to need this, however this requires more research
    switch (targetType.GetSelectionCategory())
    {
        case TARGET_SELECT_CATEGORY_NEARBY:
        case TARGET_SELECT_CATEGORY_CONE:
        case TARGET_SELECT_CATEGORY_AREA:
        {
            // targets for effect already selected
            if (effectMask & processedEffectMask)
                return;

            bool hasAreaTargetScript = false;

            for (auto itr : m_loadedScripts)
            {
                if (itr->OnObjectAreaTargetSelect.begin() != itr->OnObjectAreaTargetSelect.end())
                {
                    hasAreaTargetScript = true;
                    break;
                }
            }

            if (!hasAreaTargetScript)
            {
                // choose which targets we can select at once
                for (uint32 j = effIndex + 1; j < MAX_SPELL_EFFECTS; ++j)
                {
                    if (m_spellInfo->EffectMask < uint32(1 << j))
                        break;

                    if (GetSpellInfo()->GetEffect(effIndex, m_diffMode)->TargetA.GetTarget() == GetSpellInfo()->Effects[j]->TargetA.GetTarget() &&
                        GetSpellInfo()->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == GetSpellInfo()->Effects[j]->TargetB.GetTarget() &&
                        GetSpellInfo()->Effects[effIndex]->ImplicitTargetConditions == GetSpellInfo()->Effects[j]->ImplicitTargetConditions &&
                        GetSpellInfo()->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster) == GetSpellInfo()->Effects[j]->CalcRadius(m_caster))
                        effectMask |= 1 << j;
                }
            }
            processedEffectMask |= effectMask;
            break;
        }
        default:
            break;
    }

    switch (targetType.GetSelectionCategory())
    {
        case TARGET_SELECT_CATEGORY_GOTOMOVE:
            SelectImplicitGotoMoveTargets(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_BETWEEN:
            SelectImplicitBetweenTargets(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_CHANNEL:
            SelectImplicitChannelTargets(effIndex, targetType);
            break;
        case TARGET_SELECT_CATEGORY_NEARBY:
            SelectImplicitNearbyTargets(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_CONE:
            SelectImplicitConeTargets(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_AREA:
            SelectImplicitAreaTargets(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_THREAD:
            SelectImplicitTargetsFromThreadList(effIndex, targetType, effectMask);
            break;
        case TARGET_SELECT_CATEGORY_DEFAULT:
            switch (targetType.GetObjectType())
            {
                case TARGET_OBJECT_TYPE_SRC:
                    switch (targetType.GetReferenceType())
                    {
                        case TARGET_REFERENCE_TYPE_CASTER:
                            m_targets.SetSrc(*m_caster);
                            break;
                        default:
                            ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT_SRC");
                            break;
                    }
                    break;
                case TARGET_OBJECT_TYPE_DEST:
                case TARGET_OBJECT_TYPE_OBJ_AND_DEST:
                    switch (targetType.GetReferenceType())
                    {
                        case TARGET_REFERENCE_TYPE_CASTER:
                            SelectImplicitCasterDestTargets(effIndex, targetType);
                            break;
                        case TARGET_REFERENCE_TYPE_TARGET:
                            SelectImplicitTargetDestTargets(effIndex, targetType);
                            break;
                        case TARGET_REFERENCE_TYPE_DEST:
                            SelectImplicitDestDestTargets(effIndex, targetType);
                            break;
                        default:
                            ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT_DEST");
                            break;
                    }
                    break;
                default:
                    switch (targetType.GetReferenceType())
                    {
                        case TARGET_REFERENCE_TYPE_CASTER:
                            SelectImplicitCasterObjectTargets(effIndex, targetType);
                            break;
                        case TARGET_REFERENCE_TYPE_TARGET:
                            SelectImplicitTargetObjectTargets(effIndex, targetType);
                            break;
                        default:
                            ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT");
                            break;
                    }
                    break;
            }
            break;
        case TARGET_SELECT_CATEGORY_NYI:
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SPELL: target type %u, found in spellID %u, effect %u is not implemented yet!", m_spellInfo->Id, effIndex, targetType.GetTarget());
            break;
        default:
            printf("Spell::SelectEffectImplicitTargets: received not implemented select target category / Spell ID = %u and Effect = %u and target type = %u \n", m_spellInfo->Id, effIndex, targetType.GetTarget());
            //ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target category");
            break;
    }
}

void Spell::SelectImplicitChannelTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitChannelTargets: received not implemented target reference type");
        return;
    }

    Spell* channeledSpell = m_originalCaster->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!channeledSpell)
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitChannelTargets: cannot find channel spell for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
        return;
    }
    switch (targetType.GetTarget())
    {
        case TARGET_UNIT_CHANNEL_TARGET:
        {
            for (ObjectGuid const& channelTarget : m_originalCaster->GetChannelObjects())
            {
                WorldObject* target = ObjectAccessor::GetUnit(*m_caster, channelTarget);
                CallScriptObjectTargetSelectHandlers(target, effIndex);
                // unit target may be no longer avalible - teleported out of map for example
                if (target && target->ToUnit())
                    AddUnitTarget(target->ToUnit(), 1 << effIndex);
                else
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SPELL: cannot find channel spell target for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
                break;
            }
        }
        case TARGET_DEST_CHANNEL_TARGET:
            if (channeledSpell->m_targets.HasDst())
                m_targets.SetDst(channeledSpell->m_targets);
            else
            {
                DynamicFieldStructuredView<ObjectGuid> channelObjects = m_originalCaster->GetChannelObjects();
                WorldObject* target = channelObjects.size() > 0 ? ObjectAccessor::GetWorldObject(*m_caster, *channelObjects.begin()) : nullptr;
                CallScriptObjectTargetSelectHandlers(target, effIndex);
                if (target)
                    m_targets.SetDst(*target);
                else
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SPELL: cannot find channel spell destination for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
            }
            break;
        case TARGET_DEST_CHANNEL_CASTER:
            m_targets.SetDst(*channeledSpell->GetCaster());
            break;
        default:
            ASSERT(false && "Spell::SelectImplicitChannelTargets: received not implemented target type");
            break;
    }
}

void Spell::SelectImplicitNearbyTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER && targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_DEST)
    {
        ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented target reference type");
        return;
    }

    float range = 0.0f;
    switch (targetType.GetCheckType())
    {
        case TARGET_CHECK_ENEMY:
            range = m_spellInfo->GetMaxRange(false, m_caster, this);
            break;
        case TARGET_CHECK_ALLY:
        case TARGET_CHECK_PARTY:
        case TARGET_CHECK_RAID:
        case TARGET_CHECK_RAID_CLASS:
        case TARGET_CHECK_SUMMON:
            range = m_spellInfo->GetMaxRange(true, m_caster, this);
            break;
        case TARGET_CHECK_ENTRY:
        case TARGET_CHECK_DEFAULT:
            range = m_spellInfo->GetMaxRange(m_spellInfo->IsPositive(), m_caster, this);
            break;
        default:
            ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented selection check type");
            break;
    }

    ConditionList* condList = m_spellInfo->Effects[effIndex]->ImplicitTargetConditions;

    // handle emergency case - try to use other provided targets if no conditions provided
    if (targetType.GetCheckType() == TARGET_CHECK_ENTRY && (!condList || condList->empty()))
    {
        switch (targetType.GetObjectType())
        {
            case TARGET_OBJECT_TYPE_GOBJ:
                if (m_spellInfo->CastingReq.RequiresSpellFocus)
                {
                    if (focusObject)
                        AddGOTarget(focusObject, effMask);
                    return;
                }
                break;
            case TARGET_OBJECT_TYPE_DEST:
            case TARGET_OBJECT_TYPE_OBJ_AND_DEST:
                if (m_spellInfo->CastingReq.RequiresSpellFocus)
                {
                    if (focusObject)
                        m_targets.SetDst(*focusObject);
                    else if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
                        m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, 0.0f, st->target_mapId);
                    return;
                }
                // A lot off new spells for dungeons not have target entry. Just use spell target position system.
                // it's right way and no need all time write hacks.
                if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
                {
                    // TODO: fix this check
                    if (m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_L) || m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT))
                        m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation, (int32)st->target_mapId);
                    else if (st->target_mapId == m_caster->GetMapId())
                        m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation);
                    return;
                }
                if (Unit* referer = m_targets.GetUnitTarget())
                {
                    m_targets.SetDst(*referer);
                    return;
                }
#ifdef WIN32
                TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets: no target destination on db: spell_target_position of spell ID %u, effect %u - selecting default targets", m_spellInfo->Id, effIndex);
#endif
                break;
            default:
#ifdef WIN32
                TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets: no conditions entry for target with TARGET_CHECK_ENTRY of spell ID %u, effect %u - selecting default targets", m_spellInfo->Id, effIndex);
#endif
                break;
        }
    }

    WorldObject* target = SearchNearbyTarget(range, targetType.GetObjectType(), targetType.GetCheckType(), condList);
    if (!target)
    {
        #ifdef WIN32
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets: cannot find nearby target for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
        #endif
        return;
    }

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets spell id %u caster %u target %u", m_spellInfo->Id, m_caster->GetGUIDLow(), target->GetGUIDLow());
    #endif

    CallScriptObjectTargetSelectHandlers(target, effIndex);

    switch (targetType.GetObjectType())
    {
        case TARGET_OBJECT_TYPE_UNIT:
            if (Unit* unitTarget = target->ToUnit())
            {
                AddUnitTarget(unitTarget, effMask, condList == nullptr, false);
                m_targets.SetUnitTarget(unitTarget);
            }
            break;
        case TARGET_OBJECT_TYPE_GOBJ:
            if (GameObject* gobjTarget = target->ToGameObject())
                AddGOTarget(gobjTarget, effMask);
            break;
        case TARGET_OBJECT_TYPE_DEST:
        case TARGET_OBJECT_TYPE_OBJ_AND_DEST:
            m_targets.SetDst(*target);
            break;
        case TARGET_OBJECT_TYPE_CORPSE:
            if (Unit* unitTarget = target->ToUnit())
                if (unitTarget->isDead())
                    AddUnitTarget(unitTarget, effMask, true, false);
            break;
        default:
            ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented target object type");
            break;
    }

    SelectImplicitChainTargets(effIndex, targetType, target, effMask);
}

void Spell::SelectImplicitTargetsFromThreadList(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitTargetsFromThreadList: received not implemented target reference type");
        return;
    }

    GuidSet* savethreatlist = m_caster->GetSaveThreatList();
    for (GuidSet::const_iterator itr = savethreatlist->begin(); itr != savethreatlist->end(); ++itr)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*m_caster, (*itr)))
        {
            if (target->IsPlayerLootCooldown(m_spellInfo->Id, TYPE_SPELL, target->GetMap()->GetDifficultyID())) //Don`t add player if exist CD
                continue;

            if(m_caster->GetCurrentZoneID() == target->GetCurrentZoneID()) //Check target if this zone
            {
                AddUnitTarget(target->ToUnit(), effMask, false, false);
                m_targets.SetUnitTarget(target->ToUnit());
            }
        }
    }
}

void Spell::SelectImplicitBetweenTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    std::list<WorldObject*> targets;
    SpellTargetObjectTypes objectType = targetType.GetObjectType();
    SpellTargetCheckTypes selectionType = targetType.GetCheckType();
    ConditionList* condList = m_spellInfo->Effects[effIndex]->ImplicitTargetConditions;
    float width = m_spellInfo->TargetRestrictions.Width / 2.f;
    float angle = m_spellInfo->TargetRestrictions.ConeAngle ? m_spellInfo->TargetRestrictions.ConeAngle : targetType.CalcDirectionAngle();
    float dist = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster) * m_spellValue->RadiusMod;

    if (targetType.GetDirectionType() == TARGET_DIR_TARGET)
    {
        if (m_targets.HasDst())
            angle = m_caster->GetRelativeAngle(m_targets.GetDstPos());
        else if (Unit* target = m_targets.GetUnitTarget())
            angle = m_caster->GetRelativeAngle(target);
    }

    if (uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList))
    {
        Unit* referer = nullptr;
        switch (targetType.GetReferenceType())
        {
            case TARGET_REFERENCE_TYPE_TARGET:
                referer = m_targets.GetUnitTarget();
                break;
            case TARGET_REFERENCE_TYPE_LAST:
            {
                // find last added target for this effect
                for (auto ihit = m_UniqueTargetInfo.rbegin(); ihit != m_UniqueTargetInfo.rend(); ++ihit)
                {
                    if ((*ihit)->effectMask & (1<<effIndex))
                    {
                        referer = ObjectAccessor::GetUnit(*m_caster, (*ihit)->targetGUID);
                        break;
                    }
                }
                break;
            }
            default:
                referer = m_caster;
                break;
        }

        Position const* center = nullptr;
        switch (targetType.GetReferenceType())
        {
            case TARGET_REFERENCE_TYPE_NEAR_DEST:
            {
                Position position;
                m_caster->GetNearPoint2D(position, dist, angle);
                center = &position;
                break;
            }
            case TARGET_REFERENCE_TYPE_SRC:
                center = m_targets.GetSrcPos();
                break;
            case TARGET_REFERENCE_TYPE_DEST:
                center = m_targets.GetDstPos();
                break;
             default:
                center = referer;
                break;
        }

        Trinity::WorldObjectSpellBetweenTargetCheck check(width, dist, m_caster, center, referer, m_spellInfo, selectionType, condList);
        Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellBetweenTargetCheck> searcher(m_caster, targets, check, containerTypeMask);
        SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellBetweenTargetCheck> >(searcher, containerTypeMask, m_caster, m_caster, dist);

        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitBetweenTargets angle %f, dist %f, x %f, y %f, Id %u, targets.size %u", angle, dist, center->GetPositionX(), center->GetPositionY(), m_spellInfo->Id, targets.size());

        CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType.GetTarget());

        if (!targets.empty())
        {
            // Other special target selection goes here
            if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
                Trinity::Containers::RandomResizeList(targets, maxTargets);

            // for compability with older code - add only unit and go targets
            // TODO: remove this
            std::list<Unit*> unitTargets;
            std::list<GameObject*> gObjTargets;

            for (auto& target : targets)
            {
                if (Unit* unitTarget = target->ToUnit())
                    unitTargets.push_back(unitTarget);
                else if (GameObject* gObjTarget = target->ToGameObject())
                    gObjTargets.push_back(gObjTarget);
            }

            for (auto& unitTarget : unitTargets)
                AddUnitTarget(unitTarget, effMask, false);

            for (auto& gObjTarget : gObjTargets)
                AddGOTarget(gObjTarget, effMask);
        }
    }
}

void Spell::SelectImplicitConeTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitConeTargets: received not implemented target reference type");
        return;
    }
    std::list<WorldObject*> targets;
    SpellTargetObjectTypes objectType = targetType.GetObjectType();
    SpellTargetCheckTypes selectionType = targetType.GetCheckType();
    ConditionList* condList = m_spellInfo->Effects[effIndex]->ImplicitTargetConditions;
    Unit* caster = m_originalCaster ? m_originalCaster : m_caster;
    float coneAngle = m_spellInfo->TargetRestrictions.ConeAngle ? m_spellInfo->TargetRestrictions.ConeAngle : 180.0f;
    float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(caster) * m_spellValue->RadiusMod;

    if (uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList))
    {
        Trinity::WorldObjectSpellConeTargetCheck check(coneAngle, radius, caster, m_spellInfo, selectionType, condList);
        Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellConeTargetCheck> searcher(caster, targets, check, containerTypeMask);
        SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellConeTargetCheck> >(searcher, containerTypeMask, caster, caster, radius);

        CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType.GetTarget());

        if (!targets.empty())
        {
            // Other special target selection goes here
            if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
                Trinity::Containers::RandomResizeList(targets, maxTargets);

            // for compability with older code - add only unit and go targets
            // TODO: remove this
            std::list<Unit*> unitTargets;
            std::list<GameObject*> gObjTargets;

            for (auto& target : targets)
            {
                if (Unit* unitTarget = target->ToUnit())
                    unitTargets.push_back(unitTarget);
                else if (GameObject* gObjTarget = target->ToGameObject())
                    gObjTargets.push_back(gObjTarget);
            }

            for (auto& unitTarget : unitTargets)
            {
                if (Unit* target = unitTarget)
                {
                    if (canHitTargetInLOS || target->IsWithinLOSInMap(caster))
                        AddUnitTarget(unitTarget, effMask, false);
                }
            }

            for (auto& gObjTarget : gObjTargets)
                AddGOTarget(gObjTarget, effMask);
        }
    }
}

void Spell::SelectImplicitAreaTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    Unit* referer = nullptr;
    switch (targetType.GetReferenceType())
    {
        case TARGET_REFERENCE_TYPE_SRC:
        case TARGET_REFERENCE_TYPE_DEST:
        case TARGET_REFERENCE_TYPE_CASTER:
            referer = m_caster;
            break;
        case TARGET_REFERENCE_TYPE_TARGET:
            referer = m_targets.GetUnitTarget();
            break;
        case TARGET_REFERENCE_TYPE_LAST:
        {
            // find last added target for this effect
            for (auto ihit = m_UniqueTargetInfo.rbegin(); ihit != m_UniqueTargetInfo.rend(); ++ihit)
            {
                if ((*ihit)->effectMask & (1<<effIndex))
                {
                    referer = ObjectAccessor::GetUnit(*m_caster, (*ihit)->targetGUID);
                    break;
                }
            }
            break;
        }
        default:
            ASSERT(false && "Spell::SelectImplicitAreaTargets: received not implemented target reference type");
            return;
    }

    if (!referer)
        return;

    Position const* center = nullptr;
    switch (targetType.GetReferenceType())
    {
        case TARGET_REFERENCE_TYPE_SRC:
            center = m_targets.GetSrcPos();
            break;
        case TARGET_REFERENCE_TYPE_DEST:
            center = m_targets.GetDstPos();
            break;
        case TARGET_REFERENCE_TYPE_CASTER:
        case TARGET_REFERENCE_TYPE_TARGET:
        case TARGET_REFERENCE_TYPE_LAST:
            center = referer;
            break;
         default:
            ASSERT(false && "Spell::SelectImplicitAreaTargets: received not implemented target reference type");
            return;
    }
    std::list<WorldObject*> targets;
    float radius = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster) * m_spellValue->RadiusMod;
    if (radius <= 0)
    {
        if (m_caster->InInstance())
            radius = 5000.0f;
        else
            radius = m_caster->GetVisibilityRange();
    }

    //Hack
    if (targetType.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY && m_caster->IsCreature() && m_caster->GetEntry() == 121975)
        radius += m_caster->GetObjectSize();

    bool allowTargetObjSize = targetType.GetTarget() != TARGET_UNIT_SRC_AREA_ALLY;

    SearchAreaTargets(targets, radius, center, referer, targetType.GetObjectType(), targetType.GetCheckType(), m_spellInfo->Effects[effIndex]->ImplicitTargetConditions, allowTargetObjSize);

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitAreaTargets %u, radius %f, GetObjectType %u, targets count %ull, effIndex %i",
        m_spellInfo->Id, radius, targetType.GetObjectType(), targets.size(), effIndex);

    CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType.GetTarget());

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitAreaTargets after filter %u, radius %f, GetObjectType %u, targets count %u, GetCheckType %i, X %f, Y %f",
    m_spellInfo->Id, radius, targetType.GetObjectType(), targets.size(), targetType.GetCheckType(), center->GetPositionX(), center->GetPositionY());

    std::list<Unit*> unitTargets;
    std::list<GameObject*> gObjTargets;
    // for compability with older code - add only unit and go targets
    // TODO: remove this
    if (!targets.empty())
    {
        for (auto& target : targets)
        {
            if (!target)
                continue;
            if (Unit* unitTarget = target->ToUnit())
            {
                if (targetType.GetTarget() == TARGET_MASS_RESSURECTION && unitTarget->isAlive())
                    continue;

                unitTargets.push_back(unitTarget);
            }
            else if (GameObject* gObjTarget = target->ToGameObject())
                gObjTargets.push_back(gObjTarget);
        }
    }

    if (!unitTargets.empty())
    {
        // Special target selection for smart heals and energizes
        uint32 maxSize = 0;
        int32 power = -1;
        bool checkOnFullHealth = false;

        switch (m_spellInfo->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_GENERIC:
                switch (m_spellInfo->Id)
                {
                    case 148009: // Spirit of Chi-Ji
                    {
                        maxSize = 5;
                        power = POWER_HEALTH;
                        break;
                    }
                    case 71610: // Echoes of Light (Althor's Abacus normal version)
                    case 71641: // Echoes of Light (Althor's Abacus heroic version)
                        maxSize = 1;
                        power = POWER_HEALTH;
                        break;
                    default:
                        break;
                }
                break;
            case SPELLFAMILY_DRUID:
            {
                bool shouldCheck = false;
                switch (m_spellInfo->Id)
                {
                    case 145110: // Ysera's Gift
                    {
                        checkOnFullHealth = true;
                        maxSize = 1;
                        power = POWER_HEALTH;
                        break;
                    }
                    default:
                        break;
                }

                // Remove targets outside caster's raid
                // WTF?????????? May be better to change targets?
                if (shouldCheck)
                {
                    for (auto itr = unitTargets.begin(); itr != unitTargets.end();)
                        if (!(*itr)->IsInRaidWith(m_caster))
                            itr = unitTargets.erase(itr);
                        else
                            ++itr;
                }
                break;
            }
            default:
                break;
        }

        if (targetType.GetTarget() == TARGET_UNIT_FRIEND_OR_RAID)
        {
            if (m_originalTarget)
            {
                if (!m_caster->IsInRaidWith(m_originalTarget))
                {
                    unitTargets.clear();
                    unitTargets.push_back(m_originalTarget);
                }
            }
        }

        if (maxSize && power != -1)
        {
            if (Powers(power) == POWER_HEALTH)
            {
                if (unitTargets.size() > maxSize)
                {
                    unitTargets.sort(Trinity::HealthPctOrderPred());
                    unitTargets.resize(maxSize);
                }
            }
            else
            {
                for (auto itr = unitTargets.begin(); itr != unitTargets.end();)
                    if ((*itr)->getPowerType() != static_cast<Powers>(power))
                        itr = unitTargets.erase(itr);
                    else
                        ++itr;

                if (unitTargets.size() > maxSize)
                {
                    unitTargets.sort(Trinity::PowerPctOrderPred(static_cast<Powers>(power)));
                    unitTargets.resize(maxSize);
                }
            }
        }

        if (checkOnFullHealth)
        {
            for (auto itr = unitTargets.begin(); itr != unitTargets.end();)
                if ((*itr)->IsFullHealth())
                    itr = unitTargets.erase(itr);
                else
                    ++itr;
        }

        // todo: move to scripts, but we must call it before resize list by MaxAffectedTargets
        // Intimidating Shout
        if (m_spellInfo->Id == 5246 && effIndex != EFFECT_0)
            unitTargets.remove(m_targets.GetUnitTarget());

        // Other special target selection goes here
        if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
        {
            if (targetType.GetTarget() == TARGET_UNIT_SRC_CLOSEST_AREA_ENEMY)
            {
                if (unitTargets.size() > maxTargets)
                {
                    unitTargets.sort(Trinity::UnitSortDistance(true, m_caster));
                    unitTargets.resize(maxTargets);
                }
            }
            else
                Trinity::Containers::RandomResizeList(unitTargets, maxTargets);
        }

        for (auto& unitTarget : unitTargets)
        {
            if (Unit* target = unitTarget)
            {
                if (canHitTargetInLOS || target->IsWithinLOS(center->GetPositionX(), center->GetPositionY(), center->GetPositionZ()))
                    AddUnitTarget(unitTarget, effMask);
            }
        }
    }

    if (!gObjTargets.empty())
    {
        if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
            Trinity::Containers::RandomResizeList(gObjTargets, maxTargets);

        for (auto& gObjTarget : gObjTargets)
            AddGOTarget(gObjTarget, effMask);
    }
}

void Spell::SelectImplicitCasterDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    m_targets.SetDst(*m_caster);

    switch (targetType.GetTarget())
    {
        case TARGET_DEST_HOME:
            if (Player* playerCaster = m_caster->ToPlayer())
                m_targets.SetDst(playerCaster->m_homebindX, playerCaster->m_homebindY, playerCaster->m_homebindZ, playerCaster->GetOrientation(), playerCaster->m_homebindMapId);
            return;
        case TARGET_DEST_DB:
            if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
            {
                // TODO: fix this check
                if (m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_L) || m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT))
                    m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation, (int32)st->target_mapId);
                else if (st->target_mapId == m_caster->GetMapId())
                    m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation);
            }
            else
            {
                TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SPELL: unknown target coordinates for spell ID %u", m_spellInfo->Id);
                WorldObject* target = m_targets.GetObjectTarget();
                m_targets.SetDst(target ? *target : *m_caster);
            }
            return;
        case TARGET_DEST_CASTER_FISHING:
        {
            float minDist = m_spellInfo->GetMinRange(true);
            float maxDist = m_spellInfo->GetMaxRange(true);
            float dist = frand(minDist, maxDist);
            float x, y, z;
            float angle = float(rand_norm()) * static_cast<float>(M_PI * 35.0f / 180.0f) - static_cast<float>(M_PI * 17.5f / 180.0f);
            m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE, dist, angle);

            float ground = m_caster->GetHeight(x, y, z, true, 50.0f);
            float liquidLevel = VMAP_INVALID_HEIGHT_VALUE;
            LiquidData liquidData;
            if (m_caster->GetMap()->getLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &liquidData))
                liquidLevel = liquidData.level;

            if (liquidLevel <= ground) // When there is no liquid Map::GetWaterOrGroundLevel returns ground level
            {
                SendCastResult(SPELL_FAILED_NOT_HERE);
                SendChannelUpdate(0);
                finish(false);
                return;
            }

            if (ground + 0.75 > liquidLevel)
            {
                SendCastResult(SPELL_FAILED_TOO_SHALLOW);
                SendChannelUpdate(0);
                finish(false);
                return;
            }

            Position pos{x, y, liquidLevel, m_caster->GetOrientation()};
            m_targets.ModDst(pos);
            return;
        }
        default:
            break;
    }

    if (targetType.GetDirectionType() == TARGET_DIR_NONE)
        return;

    float dist = 0.0f;
    float angle = targetType.CalcDirectionAngle();
    float objSize = m_caster->GetObjectSize();
    if (targetType.GetTarget() == TARGET_DEST_CASTER_SUMMON)
        dist = PET_FOLLOW_DIST;
    //else if (targetType.GetTarget() == TARGET_UNK_125)
    //    dist = m_spellInfo->GetMaxRange(false, m_caster);
    else
        dist = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster);

    if (dist < objSize)
        dist = objSize;
    else if (targetType.GetTarget() == TARGET_DEST_CASTER_RANDOM)
        dist = objSize + (dist - objSize) * static_cast<float>(rand_norm());

    Position pos;
    switch (targetType.GetTarget())
    {
        case TARGET_DEST_CASTER_FRONT_LEAP:
        {
            if (canHitTargetInLOS && m_caster->IsCreature() && dist < 200.0f)
                m_caster->GetNearPoint2D(pos, dist, angle);
            else
            {
                bool needRecalcPatch = false;
                m_caster->GetFirstCollisionPosition(pos, dist, angle);

                if (m_caster->IsInWater()) // don`t check in water
                    break;

                float destx = pos.m_positionX;
                float desty = pos.m_positionY;
                float destz = pos.m_positionZ;

                float ground = m_caster->GetHeight(destx, desty, MAX_HEIGHT, true);
                float floor = m_caster->GetHeight(destx, desty, pos.m_positionZ, true);

                if (m_caster->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR)) // don`t check if falling
                {
                    float z_now = m_caster->m_movementInfo.fall.lastTimeUpdate ? (pos.m_positionZ - Movement::computeFallElevation(Movement::MSToSec(getMSTime() - m_caster->m_movementInfo.fall.lastTimeUpdate), false) - 5.0f) : pos.m_positionZ;
                    if ((z_now - ground) > 10.0f)
                        break;
                }

                PathGenerator* m_path = new PathGenerator(m_caster);
                m_path->SetShortPatch(false);
                if (!needRecalcPatch)
                {
                    bool result = m_path->CalculatePath(pos.m_positionX, pos.m_positionY, pos.m_positionZ, false);
                    PathType _pathType = m_path->GetPathType();
                    float _totalLength = m_path->GetTotalLength();
                    if (_pathType & PATHFIND_SHORT && _triggeredCastFlags != TRIGGERED_FULL_MASK)
                        needRecalcPatch = true;
                    if (!result || _pathType & PATHFIND_NOPATH)
                        needRecalcPatch = true;
                }

                if (needRecalcPatch)
                {
                    float _dist = m_caster->GetDistance(destx, desty, destz);
                    float realAngle = angle + m_caster->GetOrientation();
                    float step = _dist / 10.0f;
                    for (uint8 j = 0; j < 10; ++j)
                    {
                        bool _correct = true;
                        if (!m_caster->IsWithinLOS(destx, desty, destz))
                            _correct = false;

                        if (_correct)
                        {
                            m_path->Clear();
                            bool result = m_path->CalculatePath(destx, desty, destz, false);
                            if (m_path->GetPathType() & PATHFIND_SHORT && _triggeredCastFlags != TRIGGERED_FULL_MASK)
                                _correct = false;
                            if (!result || m_path->GetPathType() & PATHFIND_NOPATH)
                                _correct = false;
                        }
                        if (!_correct)
                        {
                            destx -= step * std::cos(realAngle);
                            desty -= step * std::sin(realAngle);
                            ground = m_caster->GetHeight(destx, desty, MAX_HEIGHT, true);
                            floor = m_caster->GetHeight(destx, desty, pos.m_positionZ, true);
                            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
                            _correct = false;
                        }
                        else
                        {
                            pos.Relocate(destx, desty, destz);
                            break;
                        }
                    }
                }
                delete m_path;
            }
            break;
        }
        case TARGET_DEST_CASTER_RADIUS:
        case TARGET_DEST_CASTER_FRONT_RIGHT:
        case TARGET_DEST_CASTER_BACK_RIGHT:
        case TARGET_DEST_CASTER_BACK_LEFT:
        case TARGET_DEST_CASTER_FRONT_LEFT:
        case TARGET_DEST_CASTER_FRONT:
        case TARGET_DEST_CASTER_BACK:
        case TARGET_DEST_CASTER_RIGHT:
        case TARGET_DEST_CASTER_LEFT:
        case TARGET_UNK_125:
            if (canHitTargetInLOS && m_caster->IsCreature() && dist < 200.0f)
                m_caster->GetNearPoint2D(pos, dist, angle, false);
            else
                m_caster->GetFirstCollisionPosition(pos, dist, angle);
            break;
        default:
            m_caster->GetNearPoint2D(pos, dist, angle);
            break;
    }
    m_targets.ModDst(pos);
}

void Spell::SelectImplicitGotoMoveTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& /*targetType*/, uint32 /*effMask*/)
{
    auto angle = 0.0f;
    if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_FORWARD) && m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_LEFT))
        angle = static_cast<float>(M_PI / 4);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_FORWARD) && m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_RIGHT))
        angle = static_cast<float>(-M_PI / 4);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD) && m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_LEFT))
        angle = static_cast<float>(3 * M_PI / 4);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD) && m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_RIGHT))
        angle = static_cast<float>(-3 * M_PI / 4);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD))
        angle = static_cast<float>(M_PI);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_LEFT))
        angle = static_cast<float>(M_PI / 2);
    else if (m_caster->HasUnitMovementFlag(MOVEMENTFLAG_STRAFE_RIGHT))
        angle = static_cast<float>(-M_PI / 2);

    Position pos;
    m_caster->GetFirstCollisionPosition(pos, std::max(m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster), m_caster->GetObjectSize()), angle);
    m_targets.SetDst(*m_caster);
    m_targets.ModDst(pos);
}

void Spell::SelectImplicitTargetDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    WorldObject* target = m_targets.GetObjectTarget();
    switch (targetType.GetTarget())
    {
        case TARGET_DEST_TARGET_ANY:
            m_targets.SetDst(*target);
            return;
        default:
            break;
    }

    if (targetType.GetDirectionType() == TARGET_DIR_NONE)
    {
        m_targets.SetDst(*target);
        return;
    }

    float angle = targetType.CalcDirectionAngle();
    float objSize = target->GetObjectSize();
    float dist = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster);
    if (dist < objSize)
        dist = objSize;
    else if (targetType.GetTarget() == TARGET_DEST_TARGET_RANDOM)
        dist = objSize + (dist - objSize) * static_cast<float>(rand_norm());

    Position pos;
    if (canHitTargetInLOS && m_caster->IsCreature() && dist < 200.0f)
        target->GetNearPoint2D(pos, dist, angle);
    else
    {
        target->GetNearPosition(pos, dist, angle);

        if ((m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetA.GetTarget() == TARGET_DEST_TARGET_BACK || m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_DEST_TARGET_BACK) &&
            /* m_spellInfo->HasAttribute(SPELL_ATTR0_ABILITY) && */ (fabs(pos.GetPositionZ() - target->GetPositionZ()) > 15.0f))
        {
            m_caster->GetFirstCollisionPosition(pos, m_caster->GetExactDist2d(target) - dist, m_caster->GetRelativeAngle(target));
        }
    }
    
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitTargetDestTargets SpellId %u canHitTargetInLOS %u", m_spellInfo->Id, canHitTargetInLOS);
    m_targets.SetDst(*target);
    m_targets.ModDst(pos);
}

void Spell::SelectImplicitDestDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    // set destination to caster if no dest provided
    // can only happen if previous destination target could not be set for some reason
    // (not found nearby target, or channel target for example
    // maybe we should abort the spell in such case?
    if (targetType.GetDirectionType() != TARGET_DIR_RANDOM)
    {
        if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
        {
            // TODO: fix this check
            if (m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_L) || m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT))
                m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation, (int32)st->target_mapId);
            else if (st->target_mapId == m_caster->GetMapId())
                m_targets.SetDst(st->target_X, st->target_Y, st->target_Z, st->target_Orientation);
            return;
        }
    }

    bool exit = false;

    switch (targetType.GetTarget())
    {
        case TARGET_DEST_TARGET_ENEMY:
            if (Unit* target = m_targets.GetUnitTarget())
            {
                AddUnitTarget(target, 1 << effIndex);
                m_targets.SetDst(*target);
            }
            exit = true;
            break;
        case TARGET_DEST_TARGET_SELECT:
        case TARGET_DEST_CHANNEL_CASTER:
            if (Unit* target = m_targets.GetUnitTarget())
                m_targets.SetDst(*target);
            exit = true;
            break;
        case TARGET_DEST_NEARBY_ENTRY:
            if (WorldObject* target = m_targets.GetObjectTarget())
                m_targets.SetDst(*target);
            exit = true;
            break;
        case TARGET_DEST_DYNOBJ_ENEMY:
        case TARGET_DEST_DYNOBJ_ALLY:
        case TARGET_DEST_DYNOBJ_NONE:
        case TARGET_DEST_DEST:
        case TARGET_UNK_128:
            exit = true;
            break;
        case TARGET_DEST_TRAJ:
            SelectImplicitTrajTargets();
            exit = true;
            break;
        default:
            break;
    }

    if (!m_targets.HasDst())
        m_targets.SetDst(*m_caster);

    if (exit)
        return;

    if (targetType.GetDirectionType() == TARGET_DIR_NONE)
        return;

    float angle = targetType.CalcDirectionAngle();
    float dist = m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcRadius(m_caster);
    if (targetType.GetTarget() == TARGET_DEST_DEST_RANDOM)
        dist *= static_cast<float>(rand_norm());

    switch (m_spellInfo->Id)
    {
        case 203537: //Nythendra: Infested Breath
            angle = frand(-2.5f, 2.5f);
            break;
        case 219231: //Botanist: Toxic Spores
            dist = 5.0f;
            break;
        default:
            break;
    }

    Position pos = static_cast<Position>(*m_targets.GetDstPos());

    if (canHitTargetInLOS && m_caster->IsCreature() && dist < 200.0f)
    {
        Position tempPos;
        pos.SimplePosXYRelocationByAngle(tempPos, dist, angle);
        pos = tempPos;
    }
    else
        m_caster->MovePosition(pos, dist, angle);
    
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitDestDestTargets SpellId %u canHitTargetInLOS %u position: %f %f %f", m_spellInfo->Id, canHitTargetInLOS, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());

    m_targets.ModDst(pos);
}

void Spell::SelectImplicitCasterObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    WorldObject* target = nullptr;
    bool checkIfValid = true;

    switch (targetType.GetTarget())
    {
        case TARGET_UNIT_CASTER:
            target = m_caster;
            checkIfValid = false;
            break;
        case TARGET_UNIT_MASTER:
            target = m_caster->GetAnyOwner();
            checkIfValid = false;
            break;
        case TARGET_UNIT_PET:
            if(m_caster->isPet())
                target = m_caster;
            else
                target = m_caster->GetGuardianPet();
            break;
        case TARGET_UNIT_VEHICLE:
            target = m_caster->GetVehicleBase();
            break;
        case TARGET_UNIT_PASSENGER_0:
        case TARGET_UNIT_PASSENGER_1:
        case TARGET_UNIT_PASSENGER_2:
        case TARGET_UNIT_PASSENGER_3:
        case TARGET_UNIT_PASSENGER_4:
        case TARGET_UNIT_PASSENGER_5:
        case TARGET_UNIT_PASSENGER_6:
        case TARGET_UNIT_PASSENGER_7:
            if (m_caster->IsCreature() && m_caster->ToCreature()->IsVehicle())
                target = m_caster->GetVehicleKit()->GetPassenger(targetType.GetTarget() - TARGET_UNIT_PASSENGER_0);
            break;
        case TARGET_UNIT_TARGET_PASSENGER:
            if (m_caster->IsCreature() && m_caster->ToCreature()->IsVehicle())
                target = m_caster->GetVehicleKit()->GetPassenger(0);
            break;
        default:
            break;
    }

    CallScriptObjectTargetSelectHandlers(target, effIndex);

    if (target && target->ToUnit())
        AddUnitTarget(target->ToUnit(), 1 << effIndex, checkIfValid);
}

void Spell::SelectImplicitTargetObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitTargetObjectTargets Id %u effIndex %u", m_spellInfo->Id, effIndex);

    //ASSERT((m_targets.GetObjectTarget() || m_targets.GetItemTarget()) && "Spell::SelectImplicitTargetObjectTargets - no explicit object or item target available!");
    if(!m_targets.GetObjectTarget() && !m_targets.GetItemTarget())
        return;

    WorldObject* target = m_targets.GetObjectTarget();

    CallScriptObjectTargetSelectHandlers(target, effIndex);

    if (target)
    {
        if (Unit* unit = target->ToUnit())
            AddUnitTarget(unit, 1 << effIndex, true, false);
        else if (GameObject* gobj = target->ToGameObject())
            AddGOTarget(gobj, 1 << effIndex);

        SelectImplicitChainTargets(effIndex, targetType, target, 1 << effIndex);
    }
    // Script hook can remove object target and we would wrongly land here
    else if (Item* item = m_targets.GetItemTarget())
        AddItemTarget(item, 1 << effIndex);
}

void Spell::SelectImplicitChainTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, WorldObject* target, uint32 effMask)
{
    int32 maxTargets = m_spellInfo->GetEffect(effIndex, m_diffMode)->ChainTargets;
    int32 additionaltarget = 0;

    if(maxTargets < 0)
        maxTargets = m_spellInfo->GetEffect(EFFECT_0, m_diffMode)->ChainTargets;

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, additionaltarget, this);

    CallScriptObjectJumpTargetHandlers(additionaltarget, effIndex);

    bool canHavoc = m_spellInfo->IsDamageSpell();
    if (m_spellInfo->Id == 116858 && m_caster->HasAura(233577)) // Chaos Bolt
        canHavoc = false;

    // Havoc and Bane of Havoc (Honor Talent)
    if (canHavoc && m_caster->getClass() == CLASS_WARLOCK)
    {
        float max_range = m_spellInfo->GetMaxRange(false, m_caster, this);

        std::list<Unit*> targetList;
        std::vector<uint32> auraList = {80240, 200548};
        m_caster->TargetsWhoHasMyAuras(targetList, auraList);

        for (auto unitTarget : targetList)
        {
            if (m_caster->IsWithinCombatRange(unitTarget, max_range) && m_caster->IsWithinLOSInMap(unitTarget))
                AddUnitTarget(unitTarget, effMask, false);
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitChainTargets maxTargets %i additionaltarget %u Id %i", maxTargets, additionaltarget, m_spellInfo->Id);

    if (additionaltarget || maxTargets > 1)
    {
        additionaltarget += maxTargets ? maxTargets - 1 : maxTargets;

        m_applyMultiplierMask |= effMask;

        std::list<WorldObject*> targets;
        SearchChainTargets(targets, additionaltarget, target, targetType.GetObjectType(), targetType.GetCheckType() , m_spellInfo->Effects[effIndex]->ImplicitTargetConditions, targetType.GetTarget() == TARGET_UNIT_TARGET_CHAINHEAL_ALLY);

        // Chain primary target is added earlier
        CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType.GetTarget());

        // for backward compability
        std::list<Unit*> unitTargets;
        for (auto& itr : targets)
        {
            if (Unit* unitTarget = itr->ToUnit())
            {
                unitTargets.push_back(unitTarget);
            }
        }

        for (auto& unitTarget : unitTargets)
            AddUnitTarget(unitTarget, effMask, false, true, true);
    }
    switch (m_spellInfo->Id)
    {
        case 120755: // Glave Toss for visual back(off-like)
        case 120756: // Glave Toss for visual back(off-like)
        case 31935: // Avenger's Shield
        case 162638: // Avenger's Shield
        case 187219: // Avenger's Shield
        case 221704: // Avenger's Shield
            AddTargetVisualHit(m_caster);
            break;
        default:
            break;
    }
}

float tangent(float x)
{
    x = tan(x);
    //if (x < std::numeric_limits<float>::max() && x > -std::numeric_limits<float>::max()) return x;
    //if (x >= std::numeric_limits<float>::max()) return std::numeric_limits<float>::max();
    //if (x <= -std::numeric_limits<float>::max()) return -std::numeric_limits<float>::max();
    if (x < 100000.0f && x > -100000.0f)
        return x;

    if (x >= 100000.0f)
        return 100000.0f;

    if (x <= 100000.0f)
        return -100000.0f;

    return 0.0f;
}

#define DEBUG_TRAJ(a) //a

void Spell::SelectImplicitTrajTargets()
{
    if (!m_targets.HasTraj())
        return;

    float dist2d = m_targets.GetDist2d();
    if (!dist2d)
        return;

    float srcToDestDelta = m_targets.GetDstPos()->m_positionZ - m_targets.GetSrcPos()->m_positionZ;

    std::list<WorldObject*> targets;
    Trinity::WorldObjectSpellTrajTargetCheck check(dist2d, m_targets.GetSrcPos(), m_caster, m_spellInfo);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> searcher(m_caster, targets, check, GRID_MAP_TYPE_MASK_ALL);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> > (searcher, GRID_MAP_TYPE_MASK_ALL, m_caster, m_targets.GetSrcPos(), dist2d);
    if (targets.empty())
        return;

    targets.sort(Trinity::ObjectDistanceOrderPred(m_caster));

    float b = tangent(m_targets.GetPitch());
    float a = (srcToDestDelta - dist2d * b) / (dist2d * dist2d);
    if (a > -0.0001f)
        a = 0;
    DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::SelectTrajTargets: a %f b %f", a, b);)

    float bestDist = m_spellInfo->GetMaxRange(false, m_caster);

    std::list<WorldObject*>::const_iterator itr = targets.begin();
    for (; itr != targets.end(); ++itr)
    {
        if (Unit* unitTarget = (*itr)->ToUnit())
            if (m_caster == *itr || m_caster->IsOnVehicle(unitTarget) || (unitTarget)->GetVehicle())//(*itr)->IsOnVehicle(m_caster))
                continue;

        const float size = std::max((*itr)->GetObjectSize() * 0.7f, 1.0f); // 1/sqrt(3)
        // TODO: all calculation should be based on src instead of m_caster
        const float objDist2d = m_targets.GetSrcPos()->GetExactDist2d(*itr) * std::cos(m_targets.GetSrcPos()->GetRelativeAngle(*itr));
        const float dz = (*itr)->GetPositionZ() - m_targets.GetSrcPos()->m_positionZ;

        DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::SelectTrajTargets: check %u, dist between %f %f, height between %f %f.", (*itr)->GetEntry(), objDist2d - size, objDist2d + size, dz - size, dz + size);)

        float dist = objDist2d - size;
        float height = dist * (a * dist + b);
        DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::SelectTrajTargets: dist %f, height %f.", dist, height);)
        if (dist < bestDist && height < dz + size && height > dz - size)
        {
            bestDist = dist > 0 ? dist : 0;
            break;
        }

#define CHECK_DIST {\
            DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::SelectTrajTargets: dist %f, height %f.", dist, height);)\
            if (dist > bestDist)\
                continue;\
            if (dist < objDist2d + size && dist > objDist2d - size)\
            {\
                bestDist = dist;\
                break;\
            }\
        }

        if (!a)
        {
            height = dz - size;
            dist = height / b;
            CHECK_DIST;

            height = dz + size;
            dist = height / b;
            CHECK_DIST;

            continue;
        }

        height = dz - size;
        float sqrt1 = b * b + 4 * a * height;
        if (sqrt1 > 0)
        {
            sqrt1 = sqrt(sqrt1);
            dist = (sqrt1 - b) / (2 * a);
            CHECK_DIST;
        }

        height = dz + size;
        float sqrt2 = b * b + 4 * a * height;
        if (sqrt2 > 0)
        {
            sqrt2 = sqrt(sqrt2);
            dist = (sqrt2 - b) / (2 * a);
            CHECK_DIST;

            dist = (-sqrt2 - b) / (2 * a);
            CHECK_DIST;
        }

        if (sqrt1 > 0)
        {
            dist = (-sqrt1 - b) / (2 * a);
            CHECK_DIST;
        }
    }

    if (m_targets.GetSrcPos()->GetExactDist2d(m_targets.GetDstPos()) > bestDist)
    {
        float x = m_targets.GetSrcPos()->m_positionX + std::cos(m_caster->GetOrientation()) * bestDist;
        float y = m_targets.GetSrcPos()->m_positionY + std::sin(m_caster->GetOrientation()) * bestDist;
        float z = m_targets.GetSrcPos()->m_positionZ + bestDist * (a * bestDist + b);

        if (itr != targets.end())
        {
            float distSq = (*itr)->GetExactDistSq(x, y, z);
            float sizeSq = (*itr)->GetObjectSize();
            sizeSq *= sizeSq;
            DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Initial %f %f %f %f %f", x, y, z, distSq, sizeSq);)
            if (distSq > sizeSq)
            {
                float factor = 1 - sqrt(sizeSq / distSq);
                x += factor * ((*itr)->GetPositionX() - x);
                y += factor * ((*itr)->GetPositionY() - y);
                z += factor * ((*itr)->GetPositionZ() - z);

                distSq = (*itr)->GetExactDistSq(x, y, z);
                DEBUG_TRAJ(TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Initial %f %f %f %f %f", x, y, z, distSq, sizeSq);)
            }
        }

        Position trajDst;
        trajDst.Relocate(x, y, z, m_caster->GetOrientation());
        m_targets.ModDst(trajDst);
    }
}

void Spell::SelectEffectTypeImplicitTargets(uint8 effIndex)
{
    // special case for SPELL_EFFECT_SUMMON_RAF_FRIEND and SPELL_EFFECT_SUMMON_PLAYER
    // TODO: this is a workaround - target shouldn't be stored in target map for those spells
    switch (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect)
    {
        case SPELL_EFFECT_SUMMON_RAF_FRIEND:
        case SPELL_EFFECT_SUMMON_PLAYER:
            if (m_caster->IsPlayer() && m_caster->ToPlayer()->GetSelection())
            {
                WorldObject* target = ObjectAccessor::FindPlayer(m_caster->ToPlayer()->GetSelection());

                CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex));

                if (target && target->ToPlayer())
                    AddUnitTarget(target->ToUnit(), 1 << effIndex, false);
            }
            return;
        default:
            break;
    }

    // select spell implicit targets based on effect type
    if (!m_spellInfo->GetEffect(effIndex, m_diffMode)->GetImplicitTargetType())
        return;

    uint32 targetMask = m_spellInfo->GetEffect(effIndex, m_diffMode)->GetMissingTargetMask();

    if (!targetMask)
        return;

    WorldObject* target = nullptr;

    switch (m_spellInfo->GetEffect(effIndex, m_diffMode)->GetImplicitTargetType())
    {
        // add explicit object target or self to the target map
        case EFFECT_IMPLICIT_TARGET_EXPLICIT:
            // player which not released his spirit is Unit, but target flag for it is TARGET_FLAG_CORPSE_MASK
            if (targetMask & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK))
            {
                if (Unit* unitTarget = m_targets.GetUnitTarget())
                    target = unitTarget;
                else if (targetMask & TARGET_FLAG_CORPSE_MASK)
                {
                    if (Corpse* corpseTarget = m_targets.GetCorpseTarget())
                    {
                        // TODO: this is a workaround - corpses should be added to spell target map too, but we can't do that so we add owner instead
                        if (Player* owner = ObjectAccessor::FindPlayer(corpseTarget->GetOwnerGUID()))
                            target = owner;
                    }
                }
                else //if (targetMask & TARGET_FLAG_UNIT_MASK)
                    target = m_caster;
            }
            if (targetMask & TARGET_FLAG_ITEM_MASK)
            {
                if (Item* itemTarget = m_targets.GetItemTarget())
                    AddItemTarget(itemTarget, 1 << effIndex);
                return;
            }
            if (targetMask & TARGET_FLAG_GAMEOBJECT_MASK)
                target = m_targets.GetGOTarget();
            break;
        // add self to the target map
        case EFFECT_IMPLICIT_TARGET_CASTER:
            if (targetMask & TARGET_FLAG_UNIT_MASK)
                target = m_caster;
            break;
        default:
            break;
    }

    CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex));

    if (target)
    {
        if (target->ToUnit())
            AddUnitTarget(target->ToUnit(), 1 << effIndex, false);
        else if (target->ToGameObject())
            AddGOTarget(target->ToGameObject(), 1 << effIndex);
    }
}

std::vector<SpellScript*> const & Spell::GetSpellScripts()
{
    return m_loadedScripts;
}

uint32 Spell::GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionList* condList)
{
    // this function selects which containers need to be searched for spell target
    uint32 retMask = GRID_MAP_TYPE_MASK_ALL;

    // filter searchers based on searched object type
    switch (objType)
    {
        case TARGET_OBJECT_TYPE_UNIT:
        case TARGET_OBJECT_TYPE_UNIT_AND_DEST:
        case TARGET_OBJECT_TYPE_CORPSE:
        case TARGET_OBJECT_TYPE_CORPSE_ENEMY:
        case TARGET_OBJECT_TYPE_CORPSE_ALLY:
            retMask &= GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CORPSE | GRID_MAP_TYPE_MASK_CREATURE;
            break;
        case TARGET_OBJECT_TYPE_PLAYER:
        {
            retMask &= GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CORPSE;
            break;
        }
        case TARGET_OBJECT_TYPE_GOBJ:
        case TARGET_OBJECT_TYPE_GOBJ_ITEM:
            retMask &= GRID_MAP_TYPE_MASK_GAMEOBJECT;
            break;
        default:
            break;
    }
    if (!m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_DEAD))
        retMask &= ~GRID_MAP_TYPE_MASK_CORPSE;
    if (m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS))
        retMask &= GRID_MAP_TYPE_MASK_CORPSE | GRID_MAP_TYPE_MASK_PLAYER;
    if (m_spellInfo->IsRequiringDeadTarget())
        retMask &= GRID_MAP_TYPE_MASK_PLAYER;

    if (condList)
        retMask &= sConditionMgr->GetSearcherTypeMaskForConditionList(*condList);
    return retMask;
}

template <typename Searcher>
void Spell::SearchTargets(Searcher& searcher, uint32 containerMask, Unit* referer, Position const* pos, float radius)
{
    if (!containerMask)
        return;

    // search world and grid for possible targets
    bool searchInGrid = (containerMask & (GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_GAMEOBJECT)) != 0;
    bool searchInWorld = (containerMask & (GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CORPSE)) != 0;
    if (searchInGrid || searchInWorld)
    {
        float x = pos->GetPositionX();
        float y = pos->GetPositionY();

        CellCoord p(Trinity::ComputeCellCoord(x, y));
        Cell cell(p);
        cell.SetNoCreate();

        Map& map = *(referer->GetMap());

        if (searchInWorld)
            cell.Visit(p, Trinity::makeWorldVisitor(searcher), map, radius + SPELL_SEARCHER_COMPENSATION, x, y);

        if (searchInGrid)
            cell.Visit(p, Trinity::makeGridVisitor(searcher), map, radius + SPELL_SEARCHER_COMPENSATION, x , y);
    }
}

WorldObject* Spell::SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList)
{
    WorldObject* target = nullptr;
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return nullptr;
    Trinity::WorldObjectSpellNearbyTargetCheck check(range, m_caster, m_spellInfo, selectionType, condList);
    Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> searcher(m_caster, target, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> > (searcher, containerTypeMask, m_caster, m_caster, range);
    return target;
}

void Spell::SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList, bool allowObjectSize /*=true*/)
{
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return;
    Unit* caster = m_originalCaster ? m_originalCaster : m_caster;
    Trinity::WorldObjectSpellAreaTargetCheck check(range, position, caster, referer, m_spellInfo, selectionType, condList, allowObjectSize);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> searcher(caster, targets, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> > (searcher, containerTypeMask, caster, position, range);
}

void Spell::SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, ConditionList* condList, bool isChainHeal)
{
    // max dist for jump target selection
    float jumpRadius = 0.0f;
    switch (m_spellInfo->Categories.DefenseType)
    {
        case SPELL_DAMAGE_CLASS_RANGED:
            // 7.5y for multi shot
            jumpRadius = 7.5f;
            break;
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            switch (m_spellInfo->Id)
            {
                case 20271:  // Judgement
                {
                    jumpRadius = 8.0f;
                    break;
                }
                case 185123: // Throw Glaive
                case 204157:
                {
                    jumpRadius = 10.0f;
                    break;
                }
                default:
                {
                    // 5y for swipe, cleave and similar
                    jumpRadius = 5.0f;
                    break;
                }
            }
            break;
        }
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:
            // 12.5y for chain heal spell since 3.2 patch
            if (isChainHeal)
                jumpRadius = 12.5f;
            // 10y as default for magic chain spells
            else
                jumpRadius = 10.0f;
            break;
    }

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_DISTANCE, jumpRadius);

    // chain lightning/heal spells and similar - allow to jump at larger distance and go out of los
    bool isBouncingFar = (m_spellInfo->HasAttribute(SPELL_ATTR4_AREA_TARGET_CHAIN) || m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_NONE || 
                          m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MAGIC || m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MELEE);

    // max dist which spell can reach
    GuidList tempGUIDs;
    WorldObject* nextTarget = target;
    while (chainTargets)
    {
        std::list<WorldObject*> tempTargets;
        SearchAreaTargets(tempTargets, jumpRadius, nextTarget, m_caster, objectType, selectType, condList);

        tempGUIDs.push_back(nextTarget->GetGUID());

        std::list<WorldObject*> removeTargets;

        for (auto& tempGUID : tempGUIDs)
            for (auto& tempTarget : tempTargets)
                if (Unit* unitTarget = tempTarget->ToUnit())
                    if (tempGUID == unitTarget->GetGUID())
                        removeTargets.push_back(tempTarget);

        for (auto& removeTarget : removeTargets)
            tempTargets.remove(removeTarget);

        if (!isBouncingFar)
        {
            for (auto itr = tempTargets.begin(); itr != tempTargets.end();)
            {
                auto checkItr = itr++;
                if (!m_caster->HasInArc(static_cast<float>(M_PI), *checkItr))
                    tempTargets.erase(checkItr);
            }
        }

        WorldObject* foundItr = nullptr;

        if (isChainHeal)
        {
            uint32 maxHPDeficit = 0;
            for (auto& tempTarget : tempTargets)
            {
                if (Unit* unitTarget = tempTarget->ToUnit())
                {
                    uint32 deficit = unitTarget->GetMaxHealth(m_caster) - unitTarget->GetHealth(m_caster);
                    if ((deficit > maxHPDeficit || !foundItr) && nextTarget->IsWithinDist(unitTarget, jumpRadius) && nextTarget->IsWithinLOSInMap(unitTarget))
                    {
                        foundItr = unitTarget;
                        maxHPDeficit = deficit;
                    }
                }
            }
        }
        else
        {
            for (auto& tempTarget : tempTargets)
            {
                if (!foundItr)
                {
                    // isBouncingFar allow hit not in los target & IsWithinDist already checked at SearchAreaTargets
                    if (isBouncingFar || target->IsWithinLOSInMap(tempTarget))
                        foundItr = tempTarget;
                }
                else if (target->GetDistanceOrder(tempTarget, foundItr) && target->IsWithinLOSInMap(tempTarget))
                    foundItr = tempTarget;
            }
        }

        if (!foundItr)
            break;

        nextTarget = foundItr;
        targets.push_back(nextTarget);
        --chainTargets;
    }
}

void Spell::UpdateSpellCastDataTargets(WorldPackets::Spells::SpellCastData& data)
{
    for (auto& targetInfo : m_UniqueTargetInfo)
    {
        if (!targetInfo->effectMask)
            targetInfo->missCondition = SPELL_MISS_IMMUNE2;

        if (targetInfo->missCondition == SPELL_MISS_NONE)
        {
            data.HitTargets.push_back(targetInfo->targetGUID);
            m_channelTargetEffectMask |= targetInfo->effectMask;
        }
        else
        {
            data.MissTargets.push_back(targetInfo->targetGUID);
            data.MissStatus.push_back(WorldPackets::Spells::SpellMissStatus(targetInfo->missCondition, targetInfo->missCondition == SPELL_MISS_REFLECT ? targetInfo->reflectResult : 0));
        }
    }

    for (auto const& targetInfo : m_VisualHitTargetInfo)
        data.HitTargets.push_back(targetInfo->targetGUID);

    for (GOTargetInfo const& targetInfo : m_UniqueGOTargetInfo)
        data.HitTargets.push_back(targetInfo.targetGUID);

    for (ItemTargetInfo const& targetInfo : m_UniqueItemInfo)
        data.HitTargets.push_back(targetInfo.item->GetGUID());

    if (!m_spellInfo->IsChanneled())
        m_channelTargetEffectMask = 0;
}

void Spell::UpdateSpellCastDataAmmo(WorldPackets::Spells::SpellAmmo& ammo)
{
    uint32 ammoDisplayID = 0;

    if (m_caster->IsPlayer())
    {
        if (Item* item = m_caster->ToPlayer()->GetWeaponForAttack(RANGED_ATTACK))
            ammoDisplayID = item->GetDisplayId(m_caster->ToPlayer());
    }
    else
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (uint32 item_id = m_caster->GetVirtualItemId(i))
            {
                if (ItemEntry const* itemEntry = sItemStore.LookupEntry(item_id))
                {
                    if (itemEntry->ClassID == ITEM_CLASS_WEAPON)
                    {
                        switch (itemEntry->SubclassID)
                        {
                            case ITEM_SUBCLASS_WEAPON_THROWN:
                                ammoDisplayID = sDB2Manager.GetItemDisplayId(item_id, m_caster->GetVirtualItemAppearanceMod(i));
                                break;
                            case ITEM_SUBCLASS_WEAPON_BOW:
                            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                                ammoDisplayID = 5996;       // is this need fixing?
                                break;
                            case ITEM_SUBCLASS_WEAPON_GUN:
                                ammoDisplayID = 5998;       // is this need fixing?
                                break;
                            default:
                                break;
                        }

                        if (ammoDisplayID)
                            break;
                    }
                }
            }
        }
    }

    ammo.DisplayID = ammoDisplayID;
}

void Spell::prepareDataForTriggerSystem(AuraEffect const* /*triggeredByAura**/)
{
    //==========================================================================================
    // Now fill data for trigger system, need know:
    // can spell trigger another or not (m_canTrigger)
    // Create base triggers flags for Attacker and Victim (m_procAttacker, m_procVictim and m_procEx)
    //==========================================================================================

    m_procVictim = m_procAttacker = 0;
    m_procEx = PROC_EX_NONE;

    /* Effects which are result of aura proc from triggered spell cannot proc
        to prevent chain proc of these spells */

    // Hellfire Effect - trigger as DOT
    if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_WARLOCK && m_spellInfo->ClassOptions.SpellClassMask[0] & 0x00000040)
    {
        m_procAttacker = PROC_FLAG_DONE_PERIODIC;
        m_procVictim   = PROC_FLAG_TAKEN_PERIODIC;
    }

    // Ranged autorepeat attack is set as triggered spell - ignore it
    if (!(m_procAttacker & PROC_FLAG_DONE_RANGED_AUTO_ATTACK))
    {
        if (_triggeredCastFlags & TRIGGERED_DISALLOW_PROC_EVENTS && (m_spellInfo->HasAttribute(SPELL_ATTR2_TRIGGERED_CAN_TRIGGER_PROC) || m_spellInfo->HasAttribute(SPELL_ATTR3_TRIGGERED_CAN_TRIGGER_PROC_2)))
            m_procEx |= PROC_EX_INTERNAL_CANT_PROC;
        else if ((_triggeredCastFlags & TRIGGERED_DISALLOW_PROC_EVENTS) && !(_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER))
            m_procEx |= PROC_EX_INTERNAL_TRIGGERED;
    }
}

void Spell::CleanupTargetList()
{
    m_UniqueTargetInfo.clear();
    m_UniqueGOTargetInfo.clear();
    m_UniqueItemInfo.clear();
    m_delayMoment = 0;
}

void Spell::AddUnitTarget(Unit* target, uint32 effectMask, bool checkIfValid /*= true*/, bool implicit /*= true*/, bool jump/* = false*/)
{
    if(!target || !m_spellInfo)
        return;

    LinkedSpell(target, target, SPELL_LINK_ON_ADD_TARGET, effectMask);

    volatile uint32 spellid = m_spellInfo->Id;

    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effIndex))
            break;

        if ((m_spellInfo->EffectMask & (1 << effIndex)) != 0)
            if (!m_spellInfo->GetEffect(effIndex, m_diffMode)->IsEffect() || !CheckEffectTarget(target, effIndex) || CheckEffFromDummy(target, effIndex))
                effectMask &= ~(1 << effIndex);
    }

    // no effects left
    if (!effectMask)
        return;

    if (checkIfValid)
        if (m_spellInfo->CheckTarget(m_caster, target, implicit) != SPELL_CAST_OK)
            return;

    if (m_spellInfo->HasAttribute(SPELL_ATTR7_CONSOLIDATED_RAID_BUFF))
        if (!target->IsPlayer() && target != m_caster)
            if (target->GetOwner() && target->GetOwner()->IsPlayer())
                return;

//     // Check for effect immune skip if immuned
//     for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
//     {
//         if (m_spellInfo->EffectMask < uint32(1 << effIndex))
//             break;
// 
//         if (target != m_caster && target->IsImmunedToSpellEffect(m_spellInfo, effIndex))
//             effectMask &= ~(1 << effIndex);
//     }

    ObjectGuid targetGUID = target->GetGUID();

    // Lookup target in already in list
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (targetGUID == ihit->targetGUID)             // Found in list
        {
            ihit->effectMask |= effectMask;             // Immune effects removed from mask
            ihit->scaleAura = false;
            if (m_auraScaleMask && ihit->effectMask == m_auraScaleMask && m_caster != target)
            {
                SpellInfo const* auraSpell = sSpellMgr->GetSpellInfo(sSpellMgr->GetFirstSpellInChain(m_spellInfo->Id));
                if (uint32(target->getLevelForTarget(m_caster) + 10) >= auraSpell->SpellLevel)
                    ihit->scaleAura = true;
            }
            return;
        }
    }

    // This is new target calculate data for him

    // Get spell hit result on target
    TargetInfoPtr targetInfo = std::make_shared<TargetInfo>(targetGUID, effectMask);

    if (target->isAlive())
        targetInfo->AddMask(TARGET_INFO_ALIVE);

    if (jump)
        targetInfo->AddMask(TARGET_INFO_IS_JUMP_TARGET);

    if (m_auraScaleMask && targetInfo->effectMask == m_auraScaleMask && m_caster != target)
    {
        SpellInfo const* auraSpell = sSpellMgr->GetSpellInfo(sSpellMgr->GetFirstSpellInChain(m_spellInfo->Id));
        if (uint32(target->getLevelForTarget(m_caster) + 10) >= auraSpell->SpellLevel)
            targetInfo->scaleAura = true;
    }

    // Calculate hit result
    if (m_originalCaster)
    {
        targetInfo->missCondition = m_originalCaster->SpellHitResult(target, m_spellInfo, m_canReflect, effectMask);
        if (m_skipCheck && targetInfo->missCondition != SPELL_MISS_IMMUNE)
            targetInfo->missCondition = SPELL_MISS_NONE;
    }
    else
        targetInfo->missCondition = SPELL_MISS_EVADE; //SPELL_MISS_NONE;

    auto const& spellMiscData = m_spellInfo->GetMisc(m_diffMode)->MiscData;

    // Spell have speed - need calculate incoming time
    // Incoming time is zero for self casts. At least I think so.
    if (spellMiscData.Speed > 0.0f && m_caster != target)
    {
        float mindist = 5.0f;
        switch (m_spellInfo->Id)
        {
            case 132467: // Chi Wave Neg
            case 132464: // Chi Wave Pos
            case 228596: // Flurry
                mindist = 0.0f;
                break;
            default:
                break;
        }
        // calculate spell incoming interval
        // TODO: this is a hack
        float dist = m_caster->GetExactDist(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

        if (dist < mindist)
            dist = mindist;

        if (!m_spellInfo->HasAttribute(SPELL_ATTR9_SPECIAL_DELAY_CALCULATION))
            targetInfo->timeDelay = uint64(floor((dist / spellMiscData.Speed * 1000.0f) + (spellMiscData.LaunchDelay * 1000.0f)));
        else
            targetInfo->timeDelay = uint64((spellMiscData.Speed + spellMiscData.LaunchDelay) * 1000.0f);

        // Calculate minimum incoming time
        if (m_delayMoment == 0 || m_delayMoment > targetInfo->timeDelay)
            m_delayMoment = targetInfo->timeDelay;

        m_delayMoment *= m_caster->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE);
    }
    else
        targetInfo->timeDelay = 0LL;

    // If target reflect spell back to caster
    if (targetInfo->missCondition == SPELL_MISS_REFLECT)
    {
        // process reflect removal (not delayed)
        if (!targetInfo->timeDelay)
        {
            DamageInfo dmgInfoProc = DamageInfo(m_caster, target, 1, m_spellInfo, m_spellInfo ? SpellSchoolMask(spellMiscData.SchoolMask) : SPELL_SCHOOL_MASK_NORMAL, SPELL_DIRECT_DAMAGE, targetInfo->damageBeforeHit);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            m_caster->ProcDamageAndSpell(target, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_EX_REFLECT, &dmgInfoProc, BASE_ATTACK, m_spellInfo);
        }

        // Calculate reflected spell result on caster
        targetInfo->reflectResult = m_caster->SpellHitResult(m_caster, m_spellInfo, m_canReflect, effectMask);

        if (targetInfo->reflectResult == SPELL_MISS_REFLECT)     // Impossible reflect again, so simply deflect spell
            targetInfo->reflectResult = SPELL_MISS_PARRY;

        // Increase time interval for reflected spells by 1.5
        targetInfo->timeDelay += targetInfo->timeDelay >> 1;
    }
    else
        targetInfo->reflectResult = SPELL_MISS_NONE;

    // Add target to list
    m_UniqueTargetInfo.push_back(targetInfo);
}

void Spell::AddTargetVisualHit(Unit* target)
{
    if(!m_spellInfo)
        return;

    // Get spell hit result on target
    TargetInfoPtr targetInfo = std::make_shared<TargetInfo>(target->GetGUID(), m_spellInfo->EffectMask);

    if (target->isAlive())
        targetInfo->AddMask(TARGET_INFO_ALIVE);

    // Add target to list
    m_VisualHitTargetInfo.push_back(targetInfo);
}

void Spell::AddGOTarget(GameObject* go, uint32 effectMask)
{
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effIndex))
            break;

        if ((m_spellInfo->EffectMask & (1 << effIndex)) == 0)
            continue;

        if (!m_spellInfo->GetEffect(effIndex, m_diffMode)->IsEffect())
            effectMask &= ~(1 << effIndex);
        else
        {
            switch (m_spellInfo->GetEffect(effIndex, m_diffMode)->Effect)
            {
            case SPELL_EFFECT_GAMEOBJECT_DAMAGE:
            case SPELL_EFFECT_GAMEOBJECT_REPAIR:
            case SPELL_EFFECT_GAMEOBJECT_SET_DESTRUCTION_STATE:
                if (go->GetGoType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
                    effectMask &= ~(1 << effIndex);
                break;
            default:
                break;
            }
        }
    }

    if (!effectMask)
        return;

    ObjectGuid targetGUID = go->GetGUID();

    // Lookup target in already in list
    for (auto& ihit : m_UniqueGOTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)                 // Found in list
        {
            ihit.effectMask |= effectMask;                 // Add only effect mask
            return;
        }
    }

    // This is new target calculate data for him

    GOTargetInfo target;
    target.targetGUID = targetGUID;
    target.effectMask = effectMask;
    target.processed  = false;                              // Effects not apply on target

    // Spell have speed - need calculate incoming time
    auto const& spellMiscData = m_spellInfo->GetMisc(m_diffMode)->MiscData;

    if (spellMiscData.Speed > 0.0f)
    {
        // calculate spell incoming interval
        float dist = m_caster->GetExactDist(go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        if (dist < 5.0f)
            dist = 5.0f;

       if (!m_spellInfo->HasAttribute(SPELL_ATTR9_SPECIAL_DELAY_CALCULATION))
           target.timeDelay = uint64(floor((dist / spellMiscData.Speed * 1000.0f) + (spellMiscData.LaunchDelay * 1000.0f)));
       else
           target.timeDelay = uint64((spellMiscData.Speed + spellMiscData.LaunchDelay) * 1000.0f);

        if (m_delayMoment == 0 || m_delayMoment > target.timeDelay)
            m_delayMoment = target.timeDelay;

        m_delayMoment *= m_caster->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE);
    }
    else
        target.timeDelay = 0LL;

    // Add target to list
    m_UniqueGOTargetInfo.push_back(target);
}

void Spell::AddItemTarget(Item* item, uint32 effectMask)
{
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effIndex))
            break;

        if (!m_spellInfo->GetEffect(effIndex, m_diffMode)->IsEffect())
            effectMask &= ~(1 << effIndex);
    }

    // no effects left
    if (!effectMask)
        return;

    // Lookup target in already in list
    for (auto& ihit : m_UniqueItemInfo)
    {
        if (item == ihit.item)                            // Found in list
        {
            ihit.effectMask |= effectMask;                 // Add only effect mask
            return;
        }
    }

    // This is new target add data

    ItemTargetInfo target;
    target.item       = item;
    target.effectMask = effectMask;

    m_UniqueItemInfo.push_back(target);
}

void Spell::AddDestTarget(SpellDestination const& dest, uint32 effIndex)
{
    m_destTargets[effIndex] = dest;
}

void Spell::DoAllEffectOnTarget(TargetInfoPtr target)
{
    if (!target || target->processed)
        return;

    target->processed = true;                               // Target checked in apply effects procedure

    // Get mask of effects for target
    uint32 mask = target->effectMask;

    // Reset damage/healing counter
    m_damage = target->damage;
    m_healing = -target->damage;

    Unit* unit = m_caster->GetGUID() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target->targetGUID);
    if (!unit)
    {
        uint32 farMask = 0;
        // create far target mask
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (m_spellInfo->HasFarUnitTargetEffect())
                if ((1 << i) & mask)
                    farMask |= (1 << i);
        }

        if (!farMask)
            return;
        // find unit in world
        unit = ObjectAccessor::FindUnit(target->targetGUID);
        if (!unit)
            return;

        // do far effects on the unit
        // can't use default call because of threading, do stuff as fast as possible
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (farMask & (1 << i))
                HandleEffects(unit, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_HIT_TARGET);
        }
        return;
    }

    if (unit->isAlive() != target->HasMask(TARGET_INFO_ALIVE))
        return;

    //if (m_spellInfo)
        //if (getState() == SPELL_STATE_DELAYED && !m_spellInfo->IsPositive() && (getMSTime() - target->timeDelay) <= unit->m_lastSanctuaryTime)
            //return;                                             // No missinfo in that case

    // Get original caster (if exist) and calculate damage/healing from him data
    Unit* caster = m_originalCaster ? m_originalCaster : m_caster;

    if (m_canLostTarget)
        if (unit->HasAura(58984)) // I found only one spell, but maybe exist more
            return;

    // Skip if m_originalCaster not avaiable
    if (!caster)
        return;

    SpellMissInfo missInfo = target->missCondition;

    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo != SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;
    if (!unitTarget)
        return;

    m_caster->_targetCount++;

    // Reset damage/healing counter
    m_absorb = 0;

    // Fill base trigger info
    uint32 procAttacker = m_procAttacker;
    uint32 procVictim   = m_procVictim;
    uint32 procEx = m_procEx;

    m_spellAura = nullptr; // Set aura to null for every target-make sure that pointer is not used for unit without aura applied

    //Check can or not triggered
    bool canEffectTrigger = CanSpellProc(unitTarget, mask);
    Unit* spellHitTarget = nullptr;

    if (missInfo == SPELL_MISS_NONE)                          // In case spell hit target, do all effect on that target
    {
        spellHitTarget = unit;

        if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_REMOVE_PERIODIC_DAMAGE))
        {
            spellHitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
            spellHitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            spellHitTarget->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
            spellHitTarget->RemoveAura(192432); // hack, it's periodic dummy for tick damage may be removed
        }
    }
    else if (missInfo == SPELL_MISS_REFLECT)                // In case spell reflect from target, do all effect on caster (if hit)
    {
        if (target->reflectResult == SPELL_MISS_NONE)       // If reflected spell hit caster -> do all effect on him
        {
            spellHitTarget = m_caster;
            if (m_caster->IsCreature())
                m_caster->ToCreature()->LowerPlayerDamageReq(target->damage);
        }
    }

    if (missInfo != SPELL_MISS_NONE)
    {
        if (m_caster->IsCreature() && m_caster->ToCreature()->IsAIEnabled)
            m_caster->ToCreature()->AI()->SpellMissTarget(unit, m_spellInfo, missInfo);
    }

    if (spellHitTarget)
    {
        uint32 _ss = getMSTime();
        SpellMissInfo missInfo2 = DoSpellHitOnUnit(spellHitTarget, mask, target->scaleAura);
        if (missInfo2 != SPELL_MISS_NONE)
        {
            if (missInfo2 != SPELL_MISS_MISS)
                m_caster->SendSpellMiss(unit, m_spellInfo->Id, missInfo2);
            m_damage = 0;
            spellHitTarget = nullptr;
        }
        uint32 _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget spellHitTarget Caster %u entry %u SpellId %u wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, _mss);
    }

    // Trigger info was not filled in spell::preparedatafortriggersystem - we do it now
    bool positive = true;
    if (canEffectTrigger && !procAttacker && !procVictim)
    {
        // Hunter trap spells - activation proc for Lock and Load, Entrapment and Misdirection
        if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_HUNTER &&
            (m_spellInfo->ClassOptions.SpellClassMask[0] & 0x18 ||     // Freezing and Frost Trap, Freezing Arrow
            m_spellInfo->Id == 57879 || m_spellInfo->Id == 13810 ||                     // Snake Trap - done this way to avoid double proc
            m_spellInfo->ClassOptions.SpellClassMask[2] & 0x00024000)) // Explosive and Immolation Trap
            procAttacker |= PROC_FLAG_DONE_TRAP_ACTIVATION;

        bool dmgSpell = m_damage || m_healing;

        if (m_damage > 0)
        {
            positive = false;
            m_isDamageSpell = true;
        }
        else if (!m_healing)
        {
            for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                // If at least one effect negative spell is negative hit
                if (mask & (1<<i))
                {
                    if (!m_spellInfo->IsPositiveEffect(i))
                    {
                        if(m_spellInfo->Effects[i]->ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE)
                            dmgSpell = true;
                        positive = false;
                    }
                    else if (!dmgSpell)
                    {
                        dmgSpell = m_spellInfo->Effects[i]->ApplyAuraName == SPELL_AURA_SCHOOL_ABSORB;
                    }
                    break;
                }
            }
        }
        switch (m_spellInfo->Categories.DefenseType)
        {
            case SPELL_DAMAGE_CLASS_MAGIC:
                if (positive)
                {
                    procAttacker |= PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG;
                    procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                    procVictim   |= dmgSpell ? PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS;
                }
                else
                {
                    procAttacker |= PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG;
                    procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    procVictim   |= dmgSpell ? PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                }
            break;
            case SPELL_DAMAGE_CLASS_MELEE:
            {
                bool isAutoAttack = false;

                if (Player* plr = m_caster->ToPlayer())
                {
                    if (m_spellInfo->Id == plr->GetAutoattackSpellId(m_attackType))
                    {
                        isAutoAttack = true;
                        procAttacker |= PROC_FLAG_DONE_MELEE_AUTO_ATTACK;
                        procVictim |= PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK;
                    }
                }

                if (!isAutoAttack)
                {
                    procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;

                    if (dmgSpell)
                        procVictim |= m_caster->IsWithinMeleeRange(spellHitTarget) ? PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS : PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS;
                    else
                        procVictim |= PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                }

                if (m_attackType == BASE_ATTACK)
                    procAttacker |= PROC_FLAG_DONE_MAINHAND_ATTACK;
                else
                    procAttacker |= PROC_FLAG_DONE_OFFHAND_ATTACK;

                if (target->targetGUID == m_caster->GetGUID())
                    canEffectTrigger = false;

                break;
            }
            case SPELL_DAMAGE_CLASS_NONE:
            {
                if (m_spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC)
                {
                    if (positive)
                    {
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                        procVictim   |= dmgSpell ? PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS;
                    }
                    else
                    {
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                        procVictim   |= dmgSpell ? PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                    }
                }
                else
                {
                    if (positive)
                    {
                        procAttacker |= PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                        procVictim   |= PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_POS;
                    }
                    else
                    {
                        procAttacker |= PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                        procVictim   |= PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                    }
                }
                break;
            }
            case SPELL_DAMAGE_CLASS_RANGED:
            {
                // Auto attack
                if (m_spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
                {
                    procAttacker |= PROC_FLAG_DONE_RANGED_AUTO_ATTACK;
                    procVictim   |= PROC_FLAG_TAKEN_RANGED_AUTO_ATTACK;
                }
                else // Ranged spell attack
                {
                    procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    procVictim   |= dmgSpell ? PROC_FLAG_TAKEN_SPELL_RANGED_DMG_CLASS : PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG;
                }

                if (target->targetGUID == m_caster->GetGUID())
                    canEffectTrigger = false;

                break;
            }
        }
    }
    CallScriptOnHitHandlers();

    if (missInfo != SPELL_MISS_EVADE && m_caster->IsValidAttackTarget(unit) && (m_spellInfo->CanStartCombat() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)))
    {
        bool initCombat = !m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO) && !m_spellInfo->HasAttribute(SPELL_ATTR1_NO_THREAT) && !(m_caster->IsCreature() && !m_caster->IsVisible());

        m_caster->CombatStart(unit, initCombat);

        if (initCombat && m_caster->isInCombat())
        {
            if (m_caster->IsPlayer() || m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER))
            {
                if (!m_caster->getVictim())
                    m_caster->UpdateVictim(unit);
            }
            else
            {
                m_caster->UpdateVictim(unit);
            }
        }

        if (m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_AURA_CC)
            if (!unit->IsStandState())
                unit->SetStandState(UNIT_STAND_STATE_STAND);
    }

    if (m_count_dispeling)
        procEx |= PROC_EX_DISPEL;

    // All calculated do it!
    // Do healing and triggers
    if (m_healing > 0)
    {
        uint32 addhealth = m_healing;
        if (target->HasMask(TARGET_INFO_CRIT))
        {
            procEx |= PROC_EX_CRITICAL_HIT;
            addhealth = caster->SpellCriticalHealingBonus(m_spellInfo, addhealth);
        }
        else
            procEx |= PROC_EX_NORMAL_HIT;

        int32 gain = caster->HealBySpell(unitTarget, m_spellInfo, addhealth, target->HasMask(TARGET_INFO_CRIT));
        unitTarget->getHostileRefManager().threatAssist(caster, float(gain) * 0.5f, m_spellInfo);
        uint32 overHeal = addhealth - gain;
        m_healing = gain;

        //TC_LOG_DEBUG(LOG_FILTER_PROC, "DoAllEffectOnTarget: m_spellInfo->Id %i, mask %i, addhealth %i, m_healing %i, canEffectTrigger %i, m_damage %i, m_addpower %i, procEx %i",
        //m_spellInfo->Id, mask, addhealth, m_healing, canEffectTrigger, m_damage, m_addpower, procEx);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (canEffectTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            DamageInfo dmgInfoProc = DamageInfo(m_caster, unitTarget, addhealth, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, target->damageBeforeHit);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetOverHeal(overHeal);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            dmgInfoProc.SetTargetInfoMask(target->GetMask());
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, nullptr, this);
        }
    }
    // Do damage and triggers
    else if (m_damage > 0)
    {
        uint32 _ss = getMSTime();
        m_isDamageSpell = true;
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, m_SpellVisual, m_spellSchoolMask, m_castGuid[0]);

        damageInfo.damageBeforeHit = target->damageBeforeHit;
        damageInfo.isAoe = m_spellInfo->IsAffectingArea() || (_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER);

        uint32 _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget 0 m_damage Caster %u entry %u SpellId %u Pos %s wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, m_caster->GetPosition().ToString().c_str(), _mss);

        // Add bonuses and fill damageInfo struct
        caster->CalculateSpellDamageTaken(&damageInfo, m_damage, m_spellInfo, mask, m_attackType, mCriticalDamageBonus, target->HasMask(TARGET_INFO_CRIT));

        _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget 1 m_damage Caster %u entry %u SpellId %u Pos %s wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, m_caster->GetPosition().ToString().c_str(), _mss);

        caster->DealDamageMods(damageInfo.target, damageInfo.damage, &damageInfo.absorb, m_spellInfo, GetTriggeredCastFlags() & TRIGGERED_CASTED_BY_AREATRIGGER);

        m_absorb = damageInfo.absorb;

        procEx |= createProcExtendMask(&damageInfo, missInfo);
        procVictim |= PROC_FLAG_TAKEN_DAMAGE;

        _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget 2 m_damage Caster %u entry %u SpellId %u Pos %s wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, m_caster->GetPosition().ToString().c_str(), _mss);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (canEffectTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            DamageInfo dmgInfoProc = DamageInfo(damageInfo, m_spellInfo);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            dmgInfoProc.SetTargetInfoMask(target->GetMask());

            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, nullptr, this);
            if (caster->IsPlayer() && !m_spellInfo->HasAttribute(SPELL_ATTR0_STOP_ATTACK_TARGET) &&
               (m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MELEE || m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_RANGED))
                caster->ToPlayer()->CastItemCombatSpell(unitTarget, m_attackType, procVictim, procEx);
        }

        m_damage = damageInfo.damage;

        _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget 3 m_damage Caster %u entry %u SpellId %u Pos %s wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, m_caster->GetPosition().ToString().c_str(), _mss);

        // Send log damage message to client
        caster->SendSpellNonMeleeDamageLog(&damageInfo);

        caster->DealSpellDamage(&damageInfo, true);
        m_final_damage = damageInfo.damage;
        m_absorb = damageInfo.absorb;
        m_resist = damageInfo.resist;
        m_blocked = damageInfo.blocked;

        _mss = GetMSTimeDiffToNow(_ss);
        if (_mss > 250)
            sLog->outDiff("Spell::DoAllEffectOnTarget 4 m_damage Caster %u entry %u SpellId %u Pos %s wait %ums", m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, m_caster->GetPosition().ToString().c_str(), _mss);

        // Hunter's pet special attacks
        if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_HUNTER && m_spellInfo->ClassOptions.SpellClassMask[0] & 0x00080000)
            if (Unit * owner = caster->GetOwner())
            {
                // Cobra Strikes
                if (Aura* pAura = owner->GetAura(53257))
                    pAura->ModStackAmount(-1);
            }
    }
    else if (m_addpower != 0)
    {
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (canEffectTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            procEx |= PROC_EX_NORMAL_HIT;
            DamageInfo dmgInfoProc = DamageInfo(m_caster, m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_caster, 0, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, target->damageBeforeHit);
            dmgInfoProc.SetAddPower(m_addpower);
            dmgInfoProc.SetAddPType(m_addptype);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, nullptr, this);
        }
    }
    // Passive spell hits/misses or active spells only misses (only triggers)
    else if (unitTarget)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, m_SpellVisual, m_spellSchoolMask, m_castGuid[0]);
        damageInfo.damageBeforeHit = target->damageBeforeHit;
        procEx |= createProcExtendMask(&damageInfo, missInfo);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (canEffectTrigger && missInfo != SPELL_MISS_REFLECT)
        {
            DamageInfo dmgInfoProc = DamageInfo(m_caster, unit, 0, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, damageInfo.damageBeforeHit);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            caster->ProcDamageAndSpell(unit, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, nullptr, this);
        }

        // Failed Pickpocket, reveal rogue
        if (missInfo == SPELL_MISS_RESIST && m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_PICKPOCKET && unitTarget->IsCreature())
        {
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK, m_spellInfo->Id);
            if (unitTarget->ToCreature()->IsAIEnabled)
                unitTarget->ToCreature()->AI()->AttackStart(m_caster);
        }
        m_absorb = damageInfo.absorb;
        m_resist = damageInfo.resist;
        m_blocked = damageInfo.blocked;
    }

    // process reflect removal (delayed)
    if (missInfo == SPELL_MISS_REFLECT && target->timeDelay)
    {
        DamageInfo dmgInfoProc = DamageInfo(m_caster, unit, 1, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, target->damageBeforeHit);
        dmgInfoProc.SetStartCast(m_castedTime);
        dmgInfoProc.SetCastTime(m_casttime);
        dmgInfoProc.SetMagnet(m_magnetGuid);
        caster->ProcDamageAndSpell(unit, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_EX_REFLECT, &dmgInfoProc, BASE_ATTACK, m_spellInfo, nullptr, nullptr, this);
    }

    if (missInfo == SPELL_MISS_NONE && m_spellInfo->HasAttribute(SPELL_ATTR7_INTERRUPT_ONLY_NONPLAYER) && !unit->IsPlayer())
        caster->CastSpell(unit, 32747, true);

    if (spellHitTarget)
    {
        //AI functions
        if (spellHitTarget->IsCreature())
        {
            if (spellHitTarget->ToCreature()->IsAIEnabled)
                spellHitTarget->ToCreature()->AI()->SpellHit(m_caster, m_spellInfo);
        }

        if (m_caster->IsCreature() && m_caster->ToCreature()->IsAIEnabled)
            m_caster->ToCreature()->AI()->SpellHitTarget(spellHitTarget, m_spellInfo);

        // Needs to be called after dealing damage/healing to not remove breaking on damage auras
        DoTriggersOnSpellHit(spellHitTarget, mask);

        // if target is fallged for pvp also flag caster if a player
        if (unit->IsPvP() && m_caster->IsPlayer())
            m_caster->ToPlayer()->UpdatePvP(true);

        CallScriptAfterHitHandlers();
    }
}

TargetInfoPtr Spell::GetTargetInfo(ObjectGuid const& targetGUID)
{
    TargetInfoPtr infoTarget;
    if (m_UniqueTargetInfo.empty())
        return infoTarget;

    for (auto& ihit : m_UniqueTargetInfo)
        if (ihit->targetGUID == targetGUID)
            infoTarget = ihit;

    return infoTarget;
}

SpellMissInfo Spell::DoSpellHitOnUnit(Unit* unit, uint32 effectMask, bool scaleAura)
{
    if (!unit || !effectMask)
        return SPELL_MISS_EVADE;

    // For delayed spells immunity may be applied between missile launch and hit - check immunity for that case
    if (m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed && (/*unit->IsImmunedToDamage(m_spellInfo) ||*/unit->IsImmunedToSpell(m_spellInfo)))
        return SPELL_MISS_IMMUNE;

    // disable effects to which unit is immune
    SpellMissInfo returnVal = SPELL_MISS_IMMUNE;

    if (m_caster != unit)
    {
        for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
        {
            if (m_spellInfo->EffectMask < uint32(1 << effectNumber))
                break;

            if (effectMask & (1 << effectNumber))
                if (unit->IsImmunedToSpellEffect(m_spellInfo, effectNumber))
                    effectMask &= ~(1 << effectNumber);
        }
    }

    if (!effectMask)
        return returnVal;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

    LinkedSpell(unit, unit, SPELL_LINK_BEFORE_HIT, effectMask);

    bool calcDiminishingReturns = true;
    Map* map = unit->GetMap();

    if (auto const& player = unit->ToPlayer())
    {
        player->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET, m_spellInfo->Id);
        player->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, m_spellInfo->Id);
        player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, m_spellInfo->Id, 0, 0, m_caster);
        player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, m_spellInfo->Id, 0, 0, m_caster);

        if (map && map->IsDungeon())
            calcDiminishingReturns = false;

        // Update scenario/challenge criterias
        if (Scenario* progress = sScenarioMgr->GetScenario(map && unit->InInstance() ? map->GetInstanceId() : 0))
        {
            progress->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, m_spellInfo->Id, 0, 0, m_caster, player);
            progress->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET2, m_spellInfo->Id, 0, 0, nullptr, player);
        }
    }

    if (m_caster->IsPlayer())
    {
        m_caster->ToPlayer()->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_CASTER, m_spellInfo->Id);
        m_caster->ToPlayer()->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_CASTER2, m_spellInfo->Id);
        m_caster->ToPlayer()->UpdateAchievementCriteria(CRITERIA_TYPE_CAST_SPELL2, m_spellInfo->Id, 0, 0, unit);
    }

    if (m_caster != unit)
    {
        // Recheck  UNIT_FLAG_NON_ATTACKABLE for delayed spells
        if (m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed > 0.0f && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE) && unit->GetCharmerOrOwnerGUID() != m_caster->GetGUID())
            return SPELL_MISS_EVADE;

        if (m_caster->_IsValidAttackTarget(unit, m_spellInfo) && !unit->IsPlayer())
            unit->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_HITBYSPELL, m_spellInfo->Id);
        else if (m_caster->IsFriendlyTo(unit))
        {
            bool positive = false;
            for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
            {
                if (m_spellInfo->EffectMask < uint32(1 << effIndex))
                    break;

                if (effectMask & (1 << effIndex))
                    positive = m_spellInfo->IsPositiveEffect(effIndex);
            }
            // for delayed spells ignore negative spells (after duel end) for friendly targets
            // TODO: this cause soul transfer bugged
            /// Handle custom flag SPELL_ATTR0_CU_CAN_BE_CASTED_ON_ALLIES, some spells are negative but can be casted on allies
            if (m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed > 0.0f && unit->IsPlayer() && !positive && !(m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_CAN_BE_CASTED_ON_ALLIES))
                return SPELL_MISS_EVADE;

            // assisting case, healing and resurrection
            if (unit->HasUnitState(UNIT_STATE_ATTACK_PLAYER))
            {
                m_caster->SetContestedPvP();
                if (m_caster->IsPlayer())
                    m_caster->ToPlayer()->UpdatePvP(true);
            }
            if (unit->isInCombat() && !m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO) && !(m_spellInfo->HasAttribute(SPELL_ATTR1_NOT_BREAK_STEALTH) && !m_damage))
            {
                bool isPvP = false;

                if (map && !map->IsDungeon())
                {
                    if (Player* plr = unit->ToPlayer())
                    {
                        if (plr->HasPvpRulesEnabled())
                            isPvP = true;
                    }
                    else if (Unit* owner = unit->GetOwner())
                    {
                        if (plr = owner->ToPlayer())
                            if (plr->HasPvpRulesEnabled())
                                isPvP = true;
                    }
                }
                m_caster->SetInCombatState(unit, isPvP);
                unit->getHostileRefManager().threatAssist(m_caster, 0.0f);
            }
        }
    }

    // Get Data Needed for Diminishing Returns, some effects may have multiple auras, so this must be done on spell hit, not aura add
    if (calcDiminishingReturns)
        m_diminishGroup = GetDiminishingReturnsGroupForSpell(m_spellInfo, m_triggeredByAuraSpell != nullptr);

    if (m_diminishGroup)
    {
        m_diminishLevel = unit->GetDiminishing(m_diminishGroup);
        DiminishingReturnsType type = GetDiminishingReturnsGroupType(m_diminishGroup);
        //Increase Diminishing on unit, current informations for actually casts will use values above
        if ((type == DRTYPE_PLAYER && unit->GetCharmerOrOwnerPlayerOrPlayerItself()) || type == DRTYPE_ALL)
            unit->IncrDiminishing(m_diminishGroup);
    }

    uint32 aura_effmask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (effectMask & (1 << i) && m_spellInfo->GetEffect(i, m_diffMode)->IsUnitOwnedAuraEffect())
            aura_effmask |= 1 << i;
    }

    if (aura_effmask)
    {
        // Select rank for aura with level requirements only in specific cases
        // Unit has to be target only of aura effect, both caster and target have to be players, target has to be other than unit target
        SpellInfo const* aurSpellInfo = m_spellInfo;
        float basePoints[MAX_SPELL_EFFECTS];
        if (scaleAura)
        {
            aurSpellInfo = m_spellInfo->GetAuraRankForLevel(unitTarget->getLevelForTarget(m_caster));
            ASSERT(aurSpellInfo);
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                basePoints[i] = aurSpellInfo->GetEffect(i, m_diffMode)->BasePoints;
                if (m_spellInfo->GetEffect(i, m_diffMode)->Effect != aurSpellInfo->GetEffect(i, m_diffMode)->Effect)
                {
                    aurSpellInfo = m_spellInfo;
                    break;
                }
            }
        }

        if (m_originalCaster)
        {
            bool refresh = false;
            m_spellAura = Aura::TryRefreshStackOrCreate(aurSpellInfo, effectMask, unit,
                m_originalCaster, (aurSpellInfo == m_spellInfo) ? &m_spellValue->EffectBasePoints[0] : &basePoints[0], m_CastItem, ObjectGuid::Empty, &refresh, 0, this);
            if (m_spellAura)
            {
                // Set aura stack amount to desired value
                if (m_spellValue->AuraStackAmount > 1)
                {
                    if (!refresh)
                        m_spellAura->SetStackAmount(m_spellValue->AuraStackAmount);
                    else
                        m_spellAura->ModStackAmount(m_spellValue->AuraStackAmount);
                }

                if (m_triggerData.setSTack)
                    m_spellAura->SetStackAmount(m_triggerData.setSTack);

                // Now Reduce spell duration using data received at spell hit
                int32 duration = m_spellAura->GetMaxDuration();
                if (m_triggerData.auraDuration)
                    duration = m_triggerData.auraDuration;

                int32 limitduration = GetDiminishingReturnsLimitDuration(m_diminishGroup, aurSpellInfo);
                float diminishMod = unit->ApplyDiminishingToDuration(m_diminishGroup, duration, m_originalCaster, m_diminishLevel, limitduration);

                // unit is immune to aura if it was diminished to 0 duration
                if (diminishMod == 0.0f)
                {
                    m_spellAura->Remove();
                    bool found = false;
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (m_spellInfo->EffectMask < uint32(1 << i))
                            break;

                        if (effectMask & (1 << i) && m_spellInfo->GetEffect(i, m_diffMode)->Effect != SPELL_EFFECT_APPLY_AURA)
                            found = true;
                    }
                    if (!found)
                        return SPELL_MISS_IMMUNE;
                }
                else
                {
                    ((UnitAura*)m_spellAura)->SetDiminishGroup(m_diminishGroup);

                    bool positive = m_spellAura->GetSpellInfo()->IsPositive();
                    if (AuraApplication* aurApp = m_spellAura->GetApplicationOfTarget(m_originalCaster->GetGUID()))
                        positive = aurApp->IsPositive();

                    duration = m_originalCaster->ModSpellDuration(aurSpellInfo, unit, duration, positive, effectMask);

                    if (duration > 0)
                    {
                        if (!m_spellInfo->HasAttribute(SPELL_ATTR5_HIDE_DURATION))
                        {
                            if (float timeRate = m_originalCaster->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE))
                            {
                                if (timeRate != 1.f && !m_castItemGUID)
                                {
                                    m_spellAura->TimeMod = timeRate;
                                    duration *= timeRate;
                                }
                            }
                        }

                        // Haste modifies duration of channeled spells
                        if (m_spellInfo->IsChanneled())
                        {
                            if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY))
                                m_originalCaster->ModSpellCastTime(aurSpellInfo, duration, this);
                            //if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION))
                                //duration = int32(duration * m_originalCaster->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE));
                        }
//                         else if (m_spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_TICK_AND_CASTTIME))
//                         {
//                             int32 origDuration = duration;
//                             for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
//                             {
//                                 if (m_spellInfo->EffectMask < uint32(1 << i))
//                                     break;
// 
//                                 if (AuraEffect const* eff = m_spellAura->GetEffect(i))
//                                     if (int32 amplitude = eff->GetPeriod())  // amplitude is hastened by UNIT_FIELD_MOD_CASTING_SPEED
//                                     {
//                                         int32 gettotalticks = origDuration / amplitude;
//                                         int32 rest = origDuration - (gettotalticks * amplitude);
//                                         int32 timer = eff->GetPeriodicTimer() == amplitude ? 0: eff->GetPeriodicTimer();
// 
//                                         if (rest > (amplitude / 2))
//                                             gettotalticks++;
// 
//                                         duration = gettotalticks * amplitude + timer;
//                                     }
//                             }
//                         }
                    }

                    if (duration != m_spellAura->GetMaxDuration())
                    {
                        bool periodicDamage = false;
                        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        {
                            if (m_spellInfo->EffectMask < uint32(1 << i))
                                break;

                            if (m_spellAura->GetEffect(i))
                                if (m_spellAura->GetEffect(i)->GetAuraType() == SPELL_AURA_PERIODIC_DAMAGE)
                                    periodicDamage = true;
                        }

                        // Fix Pandemic
                        if (periodicDamage && refresh && m_originalCaster->HasAura(131973))
                        {
                            int32 newDuration = (duration + m_spellAura->GetDuration()) <= (int32(m_spellInfo->GetMaxDuration() * 1.5f)) ?
                                duration + m_spellAura->GetDuration() : int32(m_spellInfo->GetMaxDuration() * 1.5f);
                            int32 newMaxDuration = (duration + m_spellAura->GetMaxDuration()) <= (int32(m_spellInfo->GetMaxDuration() * 1.5f)) ?
                                duration + m_spellAura->GetMaxDuration() : int32(m_spellInfo->GetMaxDuration() * 1.5f);

                            m_spellAura->SetMaxDuration(newMaxDuration);
                            m_spellAura->SetDuration(newDuration);
                        }
                        else
                        {
                            m_spellAura->SetMaxDuration(duration);
                            m_spellAura->SetDuration(duration);
                        }
                    }

                    m_spellAura->SetSpellVisual(m_SpellVisual);

                    m_spellAura->_RegisterForTargets();
                    GuidList list_efftarget = GetEffectTargets();
                    if (!list_efftarget.empty())
                        m_spellAura->SetEffectTargets(list_efftarget);
                    if (m_triggeredByAura)
                        m_spellAura->SetTriggeredAuraEff(m_triggeredByAura);
                    if (_triggeredCastFlags)
                        m_spellAura->SetTriggeredCastFlags(_triggeredCastFlags);
                    ObjectGuid dynObjGuid = GetSpellDynamicObject();
                    if (!dynObjGuid.IsEmpty())
                        m_spellAura->SetSpellDynamicObject(dynObjGuid);
                    if (m_targets.HasDst())
                        AddDst(m_targets.GetDstPos());
                    if (!_positions.empty())
                        m_spellAura->SetDstVector(&_positions);
                }
            }
        }
    }

    for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effectNumber))
            break;

        if (effectMask & (1 << effectNumber))
            HandleEffects(unit, nullptr, nullptr, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);
    }

    return SPELL_MISS_NONE;
}

void Spell::DoTriggersOnSpellHit(Unit* unit, uint32 effMask)
{
    // Apply additional spell effects to target
    // TODO: move this code to scripts
    if (m_preCastSpell)
    {
        if (sSpellMgr->GetSpellInfo(m_preCastSpell))
        {
            if(m_preCastSpell == 160029 && unit->isAlive()) // aura only for death target
            {}
            else
                // Blizz seems to just apply aura without bothering to cast
                m_caster->AddAura(m_preCastSpell, unit);
        }
    }

    // handle SPELL_AURA_ADD_TARGET_TRIGGER auras
    // this is executed after spell proc spells on target hit
    // spells are triggered for each hit spell target
    // info confirmed with retail sniffs of permafrost and shadow weaving

    if (!m_hitTriggerSpells.empty())
    {
        int _duration = 0;
        for (HitTriggerSpellList::const_iterator i = m_hitTriggerSpells.begin(); i != m_hitTriggerSpells.end(); ++i)
        {
            if (CanExecuteTriggersOnHit(effMask, i->triggeredByAura) && roll_chance_i(i->chance) && !m_spellInfo->IsPassive())
            {
                m_caster->CastSpell(unit, i->triggeredSpell, true);
                TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell %d triggered spell %d by SPELL_AURA_ADD_TARGET_TRIGGER aura", m_spellInfo->Id, i->triggeredSpell->Id);

                // SPELL_AURA_ADD_TARGET_TRIGGER auras shouldn't trigger auras without duration
                // set duration of current aura to the triggered spell
                if (i->triggeredSpell->GetDuration() == -1)
                {
                    Aura* triggeredAur = unit->GetAura(i->triggeredSpell->Id, m_caster->GetGUID());
                    if (triggeredAur != nullptr)
                    {
                        // get duration from aura-only once
                        if (!_duration)
                        {
                            Aura* aur = unit->GetAura(m_spellInfo->Id, m_caster->GetGUID());
                            _duration = aur ? aur->GetDuration() : -1;
                        }
                        triggeredAur->SetDuration(_duration);
                    }
                }
            }
        }
        m_hitTriggerSpells.clear();
    }

    // trigger linked auras remove/apply
    // TODO: remove/cleanup this, as this table is not documented and people are doing stupid things with it
    LinkedSpell(unit, unit, SPELL_LINK_ON_HIT, effMask);
}

void Spell::DoAllEffectOnTarget(GOTargetInfo* target)
{
    if (target->processed)                                  // Check target
        return;
    target->processed = true;                               // Target checked in apply effects procedure

    uint32 effectMask = target->effectMask;
    if (!effectMask)
        return;

    GameObject* go = m_caster->GetMap()->GetGameObject(target->targetGUID);
    if (!go)
        return;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

    for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effectNumber))
            break;

        if (effectMask & (1 << effectNumber))
            HandleEffects(nullptr, nullptr, go, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);
    }

    CallScriptOnHitHandlers();
    CallScriptAfterHitHandlers();
}

void Spell::DoAllEffectOnTarget(ItemTargetInfo* target)
{
    uint32 effectMask = target->effectMask;
    if (!target->item || !effectMask)
        return;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

    for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effectNumber))
            break;

        if (effectMask & (1 << effectNumber))
            HandleEffects(nullptr, target->item, nullptr, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);
    }

    CallScriptOnHitHandlers();

    CallScriptAfterHitHandlers();
}

bool Spell::UpdateChanneledTargetList()
{
    // Automatically forces caster to face target
    if (m_spellInfo->HasAttribute(SPELL_ATTR1_CHANNEL_TRACK_TARGET) && !m_caster->HasInArc(0.1f, m_targets.GetUnitTarget()))
        if (m_targets.GetUnitTarget())
            m_caster->SetInFront(m_targets.GetUnitTarget());

    // Not need check return true
    if (m_channelTargetEffectMask == 0)
        return true;

    uint32 channelTargetEffectMask = m_channelTargetEffectMask;
    uint32 channelAuraMask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_APPLY_AURA)
            channelAuraMask |= 1<<i;
        if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA)
            return true;
    }

    channelAuraMask &= channelTargetEffectMask;

    float range = 0.f;
    if (channelAuraMask)
    {
        range = m_spellInfo->GetMaxRange(m_spellInfo->IsPositive(), m_caster);

        if (Player* modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, range, this);
    }

    if (!range)
        range = 60.0f;

    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit->missCondition == SPELL_MISS_NONE && (channelTargetEffectMask & ihit->effectMask))
        {
            Unit* unit = m_caster->GetGUID() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
            if (!unit)
                continue;

            if (IsValidDeadOrAliveTarget(unit))
            {
                if (channelAuraMask & ihit->effectMask)
                {
                    if (AuraApplication * aurApp = unit->GetAuraApplication(m_spellInfo->Id, m_originalCasterGUID))
                    {
                        if (m_caster != unit && !m_caster->IsWithinDistInMap(unit, range + 5.f))
                        {
                            ihit->effectMask &= ~aurApp->GetEffectMask();
                            unit->RemoveAura(aurApp);
                            continue;
                        }
                    }
                    else // aura is dispelled
                        continue;
                }

                channelTargetEffectMask &= ~ihit->effectMask;   // remove from need alive mask effect that have alive target
            }
        }
    }

    // is all effects from m_needAliveTargetMask have alive targets
    return channelTargetEffectMask == 0;
}

void Spell::preparePetCast(SpellCastTargets const* targets, Unit* target, Unit* pet, ObjectGuid petGuid, Player* player)
{
    m_castItemGUID.Clear();
    InitExplicitTargets(*targets);

    // Fill aura scaling information
    if (m_caster->IsControlledByPlayer() && !m_spellInfo->IsPassive() && m_spellInfo->SpellLevel && !m_spellInfo->IsChanneled() && !(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_SCALING))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_APPLY_AURA)
            {
                // Change aura with ranks only if basepoints are taken from spellInfo and aura is positive
                if (m_spellInfo->IsPositiveEffect(i))
                {
                    m_auraScaleMask |= (1 << i);
                    if (m_spellValue->EffectBasePoints[i] != m_spellInfo->GetEffect(i, m_diffMode)->BasePoints)
                    {
                        m_auraScaleMask = 0;
                        break;
                    }
                }
            }
        }
    }

    m_spellState = SPELL_STATE_PREPARING;

    // create and add update event for this spell
    SpellEvent* Event = new SpellEvent(this);
    m_caster->m_Events.AddEvent(Event, m_caster->m_Events.CalculateTime(1));

    //Prevent casting at cast another spell (ServerSide check)
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS) && m_caster->IsNonMeleeSpellCast(false, true, true))
    {
        SendCastResult(SPELL_FAILED_SPELL_IN_PROGRESS);
        finish(false);
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, m_caster))
    {
        SendCastResult(SPELL_FAILED_SPELL_UNAVAILABLE);
        finish(false);
        return;
    }

    LoadScripts();

    if (!m_spellInfo->NoPower())
    {
        m_spellInfo->CalcPowerCost(m_caster, m_spellSchoolMask, m_powerCost);
        m_caster->SetPowerCost(m_powerCost);
    }

    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_BEFORE_CHECK);

    SpellCastResult result = CheckPetCast(target);

    //auto turn to target unless possessed
    if (result == SPELL_FAILED_UNIT_NOT_INFRONT && !pet->isPossessed() && !pet->IsVehicle())
    {
        if (target)
        {
            pet->SetInFront(target);
            if (target->IsPlayer())
                pet->SendUpdateToPlayer((Player*) target);
        }
        else if (Unit* unit_target2 = m_targets.GetUnitTarget())
        {
            pet->SetInFront(unit_target2);
            if (unit_target2->IsPlayer())
                pet->SendUpdateToPlayer((Player*) unit_target2);
        }

        if (Unit* powner = pet->GetCharmerOrOwner())
            if (powner->IsPlayer())
                pet->SendUpdateToPlayer(powner->ToPlayer());

        result = SPELL_CAST_OK;
    }

    if (result == SPELL_CAST_OK)
    {
        pet->ToCreature()->AddCreatureSpellCooldown(m_spellInfo->Id);

        target = m_targets.GetUnitTarget();

        //10% chance to play special pet attack talk, else growl
        //actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
        if (pet->ToCreature()->isPet() && (((Pet*) pet)->getPetType() == SUMMON_PET) && (pet != target) && (urand(0, 100) < 10))
            pet->SendPetTalk((uint32) PET_TALK_SPECIAL_SPELL);
        else
            pet->SendPetAIReaction(petGuid);

        if (target && !player->IsFriendlyTo(target) && !pet->isPossessed() && !pet->IsVehicle())
        {
            // This is true if pet has no target or has target but targets differs.
            if (pet->getVictim() != target)
            {
                if (pet->getVictim())
                    pet->AttackStop();
                pet->GetMotionMaster()->Clear();
                if (pet->ToCreature()->IsAIEnabled)
                    pet->ToCreature()->AI()->AttackStart(target);
            }
        }
    }
    else
    {
        SendCastResult(player, m_spellInfo, result, SPELL_CUSTOM_ERROR_NONE, nullptr, !(pet->isPossessed() || pet->IsVehicle()));
        if (!pet->ToCreature()->HasCreatureSpellCooldown(m_spellInfo->Id))
            player->SendClearCooldown(m_spellInfo->Id, pet);

        finish(false);

        // reset specific flags in case of spell fail. AI will reset other flags
        if (pet->GetCharmInfo())
            pet->GetCharmInfo()->SetIsCommandAttack(false);

        return;
    }

    // Prepare data for triggers
    prepareDataForTriggerSystem(m_triggeredByAura);

    // calculate cast time (calculated after first CheckCast check to prevent charge counting for first CheckCast fail)
    if (!m_casttime)
    {
        if (m_triggerData.casttime)
            m_casttime = m_triggerData.casttime;
        else
            m_casttime = m_spellInfo->CalcCastTime(m_caster, this);
    }

    // set timer base at cast time
    ReSetTimer();

    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_PREPARE_CAST);

    //Containers for channeled spells have to be set
    //TODO:Apply this to all casted spells if needed
    // Why check duration? 29350: channeled triggers channeled
    if ((_triggeredCastFlags & TRIGGERED_CAST_DIRECTLY) && (!m_spellInfo->IsChanneled() || !m_spellInfo->GetMaxDuration(m_diffMode)))
        cast(true);
    else
    {
        // stealth must be removed at cast starting (at show channel bar)
        // skip triggered spell (item equip spell casting and other not explicit character casts/item uses)
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS) && m_spellInfo->IsBreakingStealth())
        {
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST, m_spellInfo->Id);
            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                if (m_spellInfo->GetEffect(i, m_diffMode)->GetUsedTargetObjectType() == TARGET_OBJECT_TYPE_UNIT)
                {
                    m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SPELL_ATTACK, m_spellInfo->Id);
                    break;
                }
            }
        }

        CallScriptBeforeStartCastHandlers();

        m_caster->SetCurrentCastedSpell(this);
        SendSpellPendingCast(); //Send activation spell
        SendSpellCastGuids();
        SendSpellStart();

        // set target for proper facing
        if ((m_casttime || m_spellInfo->IsChanneled()) && !(_triggeredCastFlags & TRIGGERED_IGNORE_SET_FACING))
            if (m_targets.GetObjectTargetGUID() && m_caster->GetGUID() != m_targets.GetObjectTargetGUID() && m_caster->IsCreature())
                m_caster->FocusTarget(this, m_targets.GetObjectTargetGUID());

        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_GCD))
            TriggerGlobalCooldown();

        //item: first cast may destroy item and second cast causes crash
        if (!m_casttime && !m_spellInfo->Cooldowns.StartRecoveryTime && !m_castItemGUID && GetCurrentContainer() == CURRENT_GENERIC_SPELL)
            cast(true);
    }
}

void Spell::prepare(SpellCastTargets const* targets)
{
    if (m_CastItem)
    {
        m_castItemGUID = m_CastItem->GetGUID();
        m_castItemEntry = m_CastItem->GetEntry();
        m_combatItemEntry = m_CastItem->GetEntry();
    }
    else
        m_castItemGUID.Clear();

    InitExplicitTargets(*targets);

    if (!m_SpellVisual)
        m_SpellVisual = m_spellInfo->GetSpellXSpellVisualId(m_caster, m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_caster);

    // Fill aura scaling information
    if (m_caster->IsControlledByPlayer() && !m_spellInfo->IsPassive() && m_spellInfo->SpellLevel && !m_spellInfo->IsChanneled() && !(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_SCALING))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_APPLY_AURA)
            {
                // Change aura with ranks only if basepoints are taken from spellInfo and aura is positive
                if (m_spellInfo->IsPositiveEffect(i))
                {
                    m_auraScaleMask |= (1 << i);
                    if (m_spellValue->EffectBasePoints[i] != m_spellInfo->GetEffect(i, m_diffMode)->BasePoints)
                    {
                        m_auraScaleMask = 0;
                        break;
                    }
                }
            }
        }
    }

    m_spellState = SPELL_STATE_PREPARING;

    // create and add update event for this spell
    SpellEvent* Event = new SpellEvent(this);
    m_caster->m_Events.AddEvent(Event, m_caster->m_Events.CalculateTime(1));

    if (m_CastItem && m_caster->IsPlayer())
    {
        if (m_caster->ToPlayer()->GetLastPotionId() && ((m_CastItem->IsPotion() && !(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_ITEM)) || m_spellInfo->IsCooldownStartedOnEvent() || IsItemCategoryCombat()))
        {
            SendCastResult(SPELL_FAILED_ERROR);
            finish(false);
            return;
        }
    }

    //Prevent casting at cast another spell (ServerSide check)
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS) && m_caster->IsNonMeleeSpellCast(false, true, true, m_spellInfo->Id == 75))
    {
        SendCastResult(SPELL_FAILED_SPELL_IN_PROGRESS);
        finish(false);
        return;
    }

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, m_caster))
    {
        SendCastResult(SPELL_FAILED_SPELL_UNAVAILABLE);
        finish(false);
        return;
    }

    LoadScripts();

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, true);

    // Fill cost data (not use power for item casts
    if (!m_CastItem && !m_spellInfo->NoPower())
    {
        m_spellInfo->CalcPowerCost(m_caster, m_spellSchoolMask, m_powerCost);
        switch (m_spellInfo->Id)
        {
            case 2098: // Run Through
            case 32645: // Envenom
            case 196819: // Eviscerate
                if (!m_caster->m_powerCostSave.empty())
                {
                    m_powerCost = m_caster->m_powerCostSave;
                    m_caster->m_powerCostSave.clear();
                }
                break;
            case 152150: // Death from Above
                m_caster->m_powerCostSave = m_powerCost;
                break;
        }
        m_caster->SetPowerCost(m_powerCost);
    }

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);

    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_BEFORE_CHECK);

    // Special ignore check. Example creature entry: 99801 (Maw of Souls)
    if (Unit* target = m_targets.GetUnitTarget())
        if (!canHitTargetInLOS && target->IsCreature() && target->ToCreature()->IsIgnoreLos())
            canHitTargetInLOS = true;

    SpellCastResult result = CheckCast(true);

    if (result != SPELL_CAST_OK && result != SPELL_FAILED_DONT_REPORT && (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR))
        result = SPELL_FAILED_DONT_REPORT;

    if (result != SPELL_CAST_OK && !IsAutoRepeat())          //always cast autorepeat dummy for triggering
    {
        // Periodic auras should be interrupted when aura triggers a spell which can't be cast
        // for example bladestorm aura should be removed on disarm as of patch 3.3.5
        // channeled periodic spells should be affected by this (arcane missiles, penance, etc)
        // a possible alternative sollution for those would be validating aura target on unit state change
        if (m_triggeredByAura && m_triggeredByAura->IsPeriodic() && !m_triggeredByAura->GetBase()->IsPassive())
        {
            SendChannelUpdate(0);
            m_triggeredByAura->GetBase()->SetDuration(0);
        }
        #ifdef WIN32
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::prepare::CheckCast fail. spell id %u res %u m_caster %u m_originalCaster %u customCastFlags %u mask %u TargetGUID %u",
            m_spellInfo->Id, result, m_caster->GetGUIDLow(), m_originalCaster ? m_originalCaster->GetGUIDLow() : 0,
            _triggeredCastFlags, m_targets.GetTargetMask(), m_targets.GetObjectTargetGUID().GetGUIDLow());
        #endif
        SendCastResult(result);

        finish(false);
        return;
    }

    // Prepare data for triggers
    prepareDataForTriggerSystem(m_triggeredByAura);

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, true);

    // calculate cast time (calculated after first CheckCast check to prevent charge counting for first CheckCast fail)
    if (!m_casttime)
    {
        if (m_triggerData.casttime)
            m_casttime = m_triggerData.casttime;
        else
            m_casttime = m_spellInfo->CalcCastTime(m_caster, this);
    }

    if (m_caster->IsPlayer())
    { 
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);

        // Set cast time to 0 if .cheat cast time is enabled.
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_CASTTIME))
             m_casttime = 0;
    }

    // don't allow channeled spells / spells with cast time to be casted while moving
    // (even if they are interrupted on moving, spells with almost immediate effect get to have their effect processed before movement interrupter kicks in)
    // don't cancel spells which are affected by a SPELL_AURA_CAST_WHILE_WALKING effect
    if (((m_spellInfo->IsChanneled() || m_casttime) && m_caster->IsPlayer() && m_caster->isMoving() && 
        m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT) && !m_caster->HasAuraCastWhileWalking(m_spellInfo) && !(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS))
    {
        //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::prepare::checkcast fail. spell id %u res %u source %u customCastFlags %u mask %u, InterruptFlags %i", m_spellInfo->Id, SPELL_FAILED_MOVING, m_caster->GetEntry(), _triggeredCastFlags, m_targets.GetTargetMask(), m_spellInfo->InterruptFlags);
        SendCastResult(SPELL_FAILED_MOVING);
        finish(false);
        return;
    }

    // set timer base at cast time
    ReSetTimer();

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::prepare: spell id %u m_caster %u m_originalCaster %u customCastFlags %u mask %u target %s", 
        m_spellInfo->Id, m_caster->GetGUIDLow(), m_originalCaster ? m_originalCaster->GetGUIDLow() : 0,
        _triggeredCastFlags, m_targets.GetTargetMask(), m_targets.GetUnitTarget() ? m_targets.GetUnitTarget()->ToString().c_str() : "" );
    #endif

    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_PREPARE_CAST);

    //Containers for channeled spells have to be set
    //TODO:Apply this to all casted spells if needed
    // Why check duration? 29350: channeled triggers channeled
    if ((_triggeredCastFlags & TRIGGERED_CAST_DIRECTLY) && (!m_spellInfo->IsChanneled() || !m_spellInfo->GetMaxDuration(m_diffMode)))
        cast(true);
    else
    {
        // stealth must be removed at cast starting (at show channel bar)
        // skip triggered spell (item equip spell casting and other not explicit character casts/item uses)
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS) && m_spellInfo->IsBreakingStealth())
        {
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST, m_spellInfo->Id);
            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                if (m_spellInfo->GetEffect(i, m_diffMode)->GetUsedTargetObjectType() == TARGET_OBJECT_TYPE_UNIT)
                {
                    m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SPELL_ATTACK, m_spellInfo->Id);
                    break;
                }
            }
        }

        CallScriptBeforeStartCastHandlers();

        m_caster->SetCurrentCastedSpell(this);
        SendSpellPendingCast(); //Send activation spell
        SendSpellCastGuids();
        SendSpellStart();

        // set target for proper facing
        if ((m_casttime || m_spellInfo->IsChanneled()) && !(_triggeredCastFlags & TRIGGERED_IGNORE_SET_FACING))
            if (m_targets.GetObjectTargetGUID() && m_caster->GetGUID() != m_targets.GetObjectTargetGUID() && m_caster->IsCreature())
                m_caster->FocusTarget(this, m_targets.GetObjectTargetGUID());

        if (GetCaster() && GetSpellInfo() && !IsTriggered())
            if (auto tmpPlayer = GetCaster()->ToPlayer())
                if (!tmpPlayer->GetSession()->PlayerLoading() && tmpPlayer->HaveSpectators())
                {
                    SpectatorAddonMsg msg;
                    msg.SetPlayer(tmpPlayer->GetGUID());
                    msg.CastSpell(m_spellInfo->Id, m_casttime);
                    tmpPlayer->SendSpectatorAddonMsgToBG(msg);
                }

        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_GCD) && result == SPELL_CAST_OK)
            TriggerGlobalCooldown();

        if (!m_casttime && m_spellInfo->HasAttribute(SPELL_ATTR0_CU_CAST_DIRECTLY))
        {
            cast(true);
            return;
        }
        //item: first cast may destroy item and second cast causes crash
        if (!m_casttime && !m_spellInfo->Cooldowns.StartRecoveryTime && !m_castItemGUID && GetCurrentContainer() == CURRENT_GENERIC_SPELL)
            cast(true);
    }
}

void Spell::cancel()
{
    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    uint32 oldState = m_spellState;
    m_spellState = SPELL_STATE_FINISHED;

    m_autoRepeat = false;
    switch (oldState)
    {
        case SPELL_STATE_PREPARING:
            CancelGlobalCooldown();
            if (m_caster->IsPlayer())
                m_caster->ToPlayer()->RestoreSpellMods(this);
        case SPELL_STATE_DELAYED:
            SendInterrupted(SPELL_FAILED_INTERRUPTED);
            SendCastResult(SPELL_FAILED_INTERRUPTED);
            break;

        case SPELL_STATE_CASTING:
            for (std::vector<TargetInfoPtr>::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                if ((*ihit)->missCondition == SPELL_MISS_NONE)
                    if (Unit* unit = m_caster->GetGUID() == (*ihit)->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, (*ihit)->targetGUID))
                        unit->RemoveOwnedAura(m_spellInfo->Id, m_originalCasterGUID, 0, AURA_REMOVE_BY_CANCEL);

            SendChannelUpdate(0);
            if (!m_spellInfo->HasAttribute(SPELL_ATTR1_IS_FISHING))
            {
                SendInterrupted(SPELL_FAILED_INTERRUPTED);
                SendCastResult(SPELL_FAILED_INTERRUPTED);
            }

            if (m_spellInfo->IsAllowsCastWhileMove(m_caster))
                m_caster->ClearUnitState(UNIT_STATE_MOVE_IN_CASTING);

            // spell is canceled-take mods and clear list
            if (m_caster->IsPlayer())
                m_caster->ToPlayer()->RemoveSpellMods(this);

            m_appliedMods.clear();
            break;

        default:
            break;
    }

    SetReferencedFromCurrent(false);
    if (m_selfContainer && *m_selfContainer == this)
        *m_selfContainer = nullptr;

    m_caster->RemoveDynObject(m_spellInfo->Id);
    if (m_spellInfo->IsChanneled()) // if not channeled then the object for the current cast wasn't summoned yet
        m_caster->RemoveGameObject(m_spellInfo->Id, true);

    if (auto tmpPlayer = m_caster->ToPlayer())
        if (GetSpellInfo() && GetSpellInfo()->Misc.CastTimes.Base && !GetSpellInfo()->IsPassive() && !IsTriggered())
        {
            if (tmpPlayer->HaveSpectators())
            {
                SpectatorAddonMsg msg;
                msg.SetPlayer(tmpPlayer->GetGUID());
                msg.CastSpell(GetSpellInfo()->Id, 99998);
                tmpPlayer->SendSpectatorAddonMsgToBG(msg);
            }
        }

    //set state back so finish will be processed
    m_spellState = oldState;

    finish(false);
}

void Spell::cast(bool skipCheck)
{
    if (!m_caster->CheckAndIncreaseCastCounter())
    {
        if (m_triggeredByAuraSpell)
            TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Spell %u triggered by aura spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id);
        else
            TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Spell %u too deep in cast chain for cast. Cast not allowed for prevent overflow stack crash.", m_spellInfo->Id);

        SendCastResult(SPELL_FAILED_ERROR);
        finish(false);
        return;
    }

    volatile uint32 spellid = m_spellInfo->Id;

    // update pointers base at GUIDs to prevent access to non-existed already object
    UpdatePointers();

    // cancel at lost explicit target during cast
    if (m_targets.GetObjectTargetGUID() && !m_targets.GetObjectTarget())
    {
        cancel();
        m_caster->DecreaseCastCounter();
        return;
    }

	if (Unit* caster = m_caster)
		if (caster->getClass() == CLASS_DRUID)
			if (SpellInfo const* info = this->GetSpellInfo())
				if (info->Categories.DefenseType != SPELL_DAMAGE_CLASS_NONE)
					if (info->Id == 155722)
						if (Player* player = caster->ToPlayer())
							if (player->GetSpecializationId() == SPEC_DRUID_CAT)
								if (m_targets.GetTargetMask() & TARGET_FLAG_UNIT)
									if (m_targets.GetObjectTarget())
										if (Unit* target = m_targets.GetObjectTarget()->ToUnit())
											if (Aura* aura = player->GetAura(203224))
												if (!target->HasAura(155722))
													aura->SetCustomData(1);

    if (m_replaced && m_caster->getClass() == CLASS_MONK && m_spellInfo->IsDamageSpell())
    {
        // Mastery: Combo Strikes
        if (AuraEffect* aurEff = m_caster->GetAuraEffect(115636, EFFECT_0))
        {
            if (aurEff->GetBase()->GetCustomData() != m_spellInfo->Id)
            {
                aurEff->SetOldBaseAmount(1);
                if (m_caster->HasAura(196740)) // Hit Combo
                    m_caster->CastSpell(m_caster, 196741, true);

                if (m_caster->HasAura(238095) && roll_chance_f(aurEff->GetAmount())) // Master of Combinations
                    m_caster->CastSpell(m_caster, 240672, true);

                if (AuraEffect* zenMoment = m_caster->GetAuraEffect(201325, EFFECT_0)) // Zen Moment (Honor Talent)
                {
                    int32 _percDecr = zenMoment->GetSpellInfo()->Effects[EFFECT_2]->BasePoints;
                    if (zenMoment->GetAmount() < 0)
                        zenMoment->ChangeAmount(zenMoment->GetAmount() + _percDecr);
                    if (AuraEffect* zenMoment2 = m_caster->GetAuraEffect(201325, EFFECT_1)) // Zen Moment (Honor Talent)
                        if (zenMoment2->GetAmount() > 0)
                            zenMoment2->ChangeAmount(zenMoment2->GetAmount() - _percDecr);
                }
            }
            else
            {
                aurEff->SetOldBaseAmount(0);
                if (m_caster->HasAura(196740)) // Hit Combo
                    m_caster->RemoveAurasDueToSpell(196741);
            }
            aurEff->GetBase()->SetCustomData(m_spellInfo->Id);
        }
        // Item - Monk T19 Windwalker 4P Bonus
        if (AuraEffect* aurEff = m_caster->GetAuraEffect(211430, EFFECT_0))
        {
            if (aurEff->GetBase()->GetCustomData() && aurEff->GetBase()->GetCustomData() != m_spellInfo->Id)
            {
                if (aurEff->GetOldBaseAmount() && aurEff->GetOldBaseAmount() != m_spellInfo->Id)
                {
                    aurEff->SetOldBaseAmount(0);
                    aurEff->GetBase()->SetCustomData(0);
                    m_caster->CastSpell(m_caster, 211432, true);
                }
                else
                    aurEff->SetOldBaseAmount(m_spellInfo->Id);
            }
            else
                aurEff->GetBase()->SetCustomData(m_spellInfo->Id);
        }
    }

    if (Player* playerCaster = m_caster->ToPlayer())
    {
        // now that we've done the basic check, now run the scripts
        // should be done before the spell is actually executed
        sScriptMgr->OnPlayerSpellCast(playerCaster, this, skipCheck);

        // As of 3.0.2 pets begin attacking their owner's target immediately
        // Let any pets know we've attacked something. Check Categories.DefenseType for harmful spells only
        // This prevents spells such as Hunter's Mark from triggering pet attack
		if (this->GetSpellInfo()->Categories.DefenseType != SPELL_DAMAGE_CLASS_NONE)
		{
			if (Pet* playerPet = playerCaster->GetPet())
				if (playerPet->isAlive() && playerPet->isControlled() && (m_targets.GetTargetMask() & TARGET_FLAG_UNIT))
					if (m_targets.GetObjectTarget())
						if (Unit* target_ = m_targets.GetObjectTarget()->ToUnit())
							playerPet->AI()->OwnerAttacked(target_);

			if (sWorld->getBoolConfig(CONFIG_PLAYER_CONTROL_GUARDIAN_PETS))
			{
				if (m_targets.GetTargetMask() & TARGET_FLAG_UNIT)
				{
					if (m_targets.GetObjectTarget())
						if (Unit* target = playerCaster->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = playerCaster->m_Controlled.begin(); itr != playerCaster->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*playerCaster, *itr))
									if (creature->IsAIEnabled && (((creature->isPet() || creature->m_isHati) && creature->HasReactState(REACT_HELPER)) ||
										(creature->isGuardian() && !creature->isPet() && !creature->m_isHati && creature->GetEntry() != 100868)))
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);

										if (creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
										{
											if (SpellInfo const* spellInfo = this->GetSpellInfo())
												if (spellInfo->Id == 100780 ||
													spellInfo->Id == 107428 ||
													spellInfo->Id == 100784 ||
													spellInfo->Id == 113656 ||
													spellInfo->Id == 117952 ||
													spellInfo->Id == 115098)
													creature->CastSpell(target, spellInfo->Id, true);
										}
									}
				}
				else if(SpellInfo const* spellInfo = this->GetSpellInfo())
				{
					if (playerCaster->HasAura(137639) && spellInfo->Id == 123986)
						if (Unit* target = playerCaster->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = playerCaster->m_Controlled.begin(); itr != playerCaster->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*playerCaster, *itr))
									if (creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);
										creature->CastSpell(target, spellInfo->Id, true);
									}
								
				}
			}
			else
			{
				if (m_targets.GetTargetMask() & TARGET_FLAG_UNIT)
				{
					if (m_targets.GetObjectTarget())
						if (Unit* target = playerCaster->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = playerCaster->m_Controlled.begin(); itr != playerCaster->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*playerCaster, *itr))
									if (creature->IsAIEnabled && ((creature->isPet() || creature->m_isHati) && creature->HasReactState(REACT_HELPER)) || creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);

										if (creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
										{
											if (SpellInfo const* spellInfo = this->GetSpellInfo())
												if (spellInfo->Id == 100780 ||
													spellInfo->Id == 107428 ||
													spellInfo->Id == 100784 ||
													spellInfo->Id == 113656 ||
													spellInfo->Id == 117952 ||
													spellInfo->Id == 115098)
													creature->CastSpell(target, spellInfo->Id, true);
										}
									}
				}
				else if (SpellInfo const* spellInfo = this->GetSpellInfo())
				{
					if (playerCaster->HasAura(137639) && spellInfo->Id == 123986)
						if (Unit* target = playerCaster->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = playerCaster->m_Controlled.begin(); itr != playerCaster->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*playerCaster, *itr))
									if (creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);
										creature->CastSpell(target, spellInfo->Id, true);
									}

				}
			}
		}
		else if (SpellInfo const* spellInfo = this->GetSpellInfo())
		{
			if (playerCaster->HasAura(137639) && (spellInfo->Id == 116847 || spellInfo->Id == 101546 || spellInfo->Id == 152175 || spellInfo->Id == 123986))
						if (Unit* target = playerCaster->GetSelectedUnit())
							for (Unit::ControlList::iterator itr = playerCaster->m_Controlled.begin(); itr != playerCaster->m_Controlled.end(); ++itr)
								if (Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*playerCaster, *itr))
									if (creature->GetEntry() == 69791 || creature->GetEntry() == 69792)
									{
										creature->ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
										creature->AI()->AttackStart(target);
										creature->CastSpell(target, spellInfo->Id, true);
									}

		}
           
        if(GetSpellInfo()->HasEffect(SPELL_EFFECT_TALENT_SPEC_SELECT))
            if (Item* pItem = playerCaster->GetArtifactWeapon())
            {
                ItemPosCountVec dest;
                InventoryResult msg = playerCaster->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
                if (msg != EQUIP_ERR_OK)
                {
                    playerCaster->SendEquipError(EQUIP_ERR_BAG_FULL, pItem);
                    SendCastResult(SPELL_FAILED_ERROR);
                    finish(false);
                    SetExecutedCurrently(false);
                    m_caster->DecreaseCastCounter();
                    return;
                }
            }
            
        if(GetSpellInfo()->HasAura(SPELL_AURA_MOUNTED))
        {
            // before added new mount, we need remove old mount
            playerCaster->Dismount();
            playerCaster->RemoveAurasByType(SPELL_AURA_MOUNTED);
            if (GetSpellInfo()->IsNotMount())
                playerCaster->RemoveFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT);
        }
    }
    
    SetExecutedCurrently(true);

    if (!m_caster->IsPlayer() && m_targets.GetUnitTarget() && m_targets.GetUnitTarget() != m_caster)
        if (!m_spellInfo->HasAttribute(SPELL_ATTR5_DONT_TURN_DURING_CAST))
            m_caster->SetInFront(m_targets.GetUnitTarget());

    // Should this be done for original caster?
    if (m_caster->IsPlayer())
    {
        // Set spell which will drop charges for triggered cast spells
        // if not successfully casted, will be remove in finish(false)
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, true);
    }

    CallScriptBeforeCastHandlers();
    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_BEFORE_CAST);

    // skip check if done already (for instant cast spells for example)
    if (!skipCheck)
    {
        SpellCastResult castResult = CheckCast(false);
        if (castResult != SPELL_CAST_OK)
        {
            #ifdef WIN32
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::cast::checkcast fail. spell id %u res %u source %u caster %d customCastFlags %u mask %u", m_spellInfo->Id, castResult, m_caster->GetEntry(), m_originalCaster ? m_originalCaster->GetEntry() : -1, _triggeredCastFlags, m_targets.GetTargetMask());
            #endif
            SendCastResult(castResult);
            SendInterrupted(0);
            //restore spell mods
            if (m_caster->IsPlayer())
            {
                m_caster->ToPlayer()->RestoreSpellMods(this);
                // cleanup after mod system
                // triggered spell pointer can be not removed in some cases
                m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);
            }
            finish(false);
            SetExecutedCurrently(false);
            m_caster->DecreaseCastCounter();
            return;
        }

        // additional check after cast bar completes (must not be in CheckCast)
        // if trade not complete then remember it in trade data
        if (m_targets.GetTargetMask() & TARGET_FLAG_TRADE_ITEM)
        {
            if (m_caster->IsPlayer())
            {
                if (TradeData* my_trade = m_caster->ToPlayer()->GetTradeData())
                {
                    if (!my_trade->IsInAcceptProcess())
                    {
                        // Spell will be casted at completing the trade. Silently ignore at this place
                        my_trade->SetSpell(m_spellInfo->Id, m_CastItem);
                        SendCastResult(SPELL_FAILED_DONT_REPORT);
                        SendInterrupted(0);
                        m_caster->ToPlayer()->RestoreSpellMods(this);
                        // cleanup after mod system
                        // triggered spell pointer can be not removed in some cases
                        m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);
                        finish(false);
                        SetExecutedCurrently(false);
                        m_caster->DecreaseCastCounter();
                        return;
                    }
                }
            }
        }
    }

    SelectSpellTargets();

    // Spell may be finished after target map check
    if (m_spellState == SPELL_STATE_FINISHED)
    {
        SendInterrupted(0);
        //restore spell mods
        if (m_caster->IsPlayer())
        {
            m_caster->ToPlayer()->RestoreSpellMods(this);
            // cleanup after mod system
            // triggered spell pointer can be not removed in some cases
            m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);
        }
        finish(false);
        SetExecutedCurrently(false);
        m_caster->DecreaseCastCounter();
        return;
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET))
        if (Creature* pet = ObjectAccessor::GetCreature(*m_caster, m_caster->GetPetGUID()))
            pet->DespawnOrUnsummon();

    PrepareTriggersExecutedOnHit();

    CallScriptOnCastHandlers();

    if (m_caster->IsCreature() && m_caster->ToCreature()->IsAIEnabled)
        m_caster->ToCreature()->AI()->SpellFinishCast(m_spellInfo);

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    m_targets.UpdateTradeSlotItem();

    if (Player* plrCaster = m_caster->ToPlayer())
    {
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_ITEM))
        {
            uint32 entry = m_CastItem ? m_CastItem->GetEntry() : 0;

            if (m_castItemEntry && (m_castFlags[1] & CAST_FLAG_EX_USE_TOY_SPELL))
                entry = m_castItemEntry;

            if (entry)
            {
                plrCaster->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_ITEM2, entry);
                plrCaster->UpdateAchievementCriteria(CRITERIA_TYPE_USE_ITEM, entry);
            }
        }

        plrCaster->UpdateAchievementCriteria(CRITERIA_TYPE_CAST_SPELL, m_spellInfo->Id);

        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD))
            plrCaster->TakeSpellCharge(m_spellInfo);
    }

    // CAST SPELL
    SendSpellCooldown();

    PrepareScriptHitHandlers();

    HandleLaunchPhase();

    // Powers have to be taken before SendSpellGo
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
        TakePower();

    m_caster->SendSpellVisualKit(m_spellInfo);
    m_caster->SendSpellCreateVisual(m_spellInfo, &visualPos, m_targets.GetUnitTarget());
    if (m_targets.HasDst())
        m_caster->SendSpellPlayOrphanVisual(m_spellInfo, true, m_targets.GetDstPos(), m_targets.GetUnitTarget());
    else
        m_caster->SendSpellPlayOrphanVisual(m_spellInfo, true, &*m_caster, m_targets.GetUnitTarget());

    // we must send smsg_spell_go packet before m_castItem delete in TakeCastItem()...
    SendSpellGo();

    //test fix for take some charges from aura mods
    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->RemoveSpellMods(this, true);

    bool hasDeley = true;
    // Okay, everything is prepared. Now we need to distinguish between immediate and evented delayed spells
    if (((m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed > 0.0f || m_delayMoment) && !m_spellInfo->IsChanneled() && !m_spellInfo->IsNonNeedDelay()) || m_spellInfo->HasAttribute(SPELL_ATTR4_HAS_DELAY))
    {
        // Remove used for cast item if need (it can be already NULL after TakeReagents call
        // in case delayed spell remove item at cast delay start
        TakeCastItem();

        // Okay, maps created, now prepare flags
        m_immediateHandled = false;
        m_spellState = SPELL_STATE_DELAYED;
        SetDelayStart(0);

        if (m_caster->HasUnitState(UNIT_STATE_CASTING) && !m_caster->IsNonMeleeSpellCast(false, false, true))
            m_caster->ClearUnitState(UNIT_STATE_CASTING);
    }
    else
    {
        hasDeley = false;
    }

    Unit* procTarget = m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_caster;
    uint32 procAttacker = PROC_EX_NONE;
    uint32 procVictim   = PROC_EX_NONE;
    TargetInfoPtr infoTarget = GetTargetInfo(procTarget ? procTarget->GetGUID() : ObjectGuid::Empty);
    // TC_LOG_DEBUG(LOG_FILTER_PROC, "Spell::cast Id %i, m_UniqueTargetInfo %i, procAttacker %i, target %u, infoTarget %u",
    // m_spellInfo->Id, m_UniqueTargetInfo.size(), procAttacker, procTarget ? procTarget->GetGUID() : 0, infoTarget ? infoTarget->targetGUID : 0);

    if (!procAttacker)
    {
        Unit* caster = m_originalCaster ? m_originalCaster : m_caster;
        uint32 mask = 7;
        bool canEffectTrigger = CanSpellProc(procTarget, mask);
        // TC_LOG_DEBUG(LOG_FILTER_PROC, "Spell::cast Id %i, mask %i, canEffectTrigger %i", m_spellInfo->Id, mask, canEffectTrigger);

        if(canEffectTrigger)
        {
            int32 procDamage = m_damage;
            uint32 procEx = m_procEx;

            if(infoTarget)
            {
                if (infoTarget->HasMask(TARGET_INFO_CRIT))
                {
                    procEx |= PROC_EX_CRITICAL_HIT;

                    if (!mCriticalDamageBonus)
                        mCriticalDamageBonus = caster->SpellCriticalDamageBonus(m_spellInfo, procTarget);

                    procDamage = infoTarget->damage * mCriticalDamageBonus;
                }
                else
                {
                    procEx |= PROC_EX_NORMAL_HIT;
                    procDamage = infoTarget->damage;
                }
            }
            else
                procEx |= PROC_EX_NORMAL_HIT;

            bool positive = true;
            bool dmgSpell = procDamage;
            if (procDamage > 0)
                positive = false;
            else if (procDamage < 0)
                procDamage = -procDamage;
            else if (!procDamage)
            {
                for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
                {
                    if (m_spellInfo->EffectMask < uint32(1 << i))
                        break;

                    // If at least one effect negative spell is negative hit
                    if (mask & (1 << i))
                    {
                        if (!m_spellInfo->IsPositiveEffect(i))
                        {
                            if(m_spellInfo->Effects[i]->ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE || m_spellInfo->Effects[i]->Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                                dmgSpell = true;
                            positive = false;
                        }
                        else if (!dmgSpell)
                            dmgSpell = m_spellInfo->Effects[i]->ApplyAuraName == SPELL_AURA_SCHOOL_ABSORB;
                        break;
                    }
                }
            }

            // Hunter trap spells - activation proc for Lock and Load, Entrapment and Misdirection
            if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_HUNTER &&
                (m_spellInfo->ClassOptions.SpellClassMask[0] & 0x18 ||     // Freezing and Frost Trap, Freezing Arrow
                m_spellInfo->Id == 57879 || m_spellInfo->Id == 13810 ||                     // Snake Trap - done this way to avoid double proc
                m_spellInfo->ClassOptions.SpellClassMask[2] & 0x00024000)) // Explosive and Immolation Trap
                procAttacker |= PROC_FLAG_DONE_TRAP_ACTIVATION;

            switch (m_spellInfo->Categories.DefenseType)
            {
                case SPELL_DAMAGE_CLASS_MELEE:
                {
                    bool isAutoattack = false;

                    if (Player* plr = m_caster->ToPlayer())
                    {
                        if (m_spellInfo->Id == plr->GetAutoattackSpellId(m_attackType))
                            isAutoattack = true;
                    }

                    if (!isAutoattack)
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;

                    if (m_attackType == BASE_ATTACK)
                        procAttacker |= PROC_FLAG_DONE_MAINHAND_ATTACK;
                    else
                        procAttacker |= PROC_FLAG_DONE_OFFHAND_ATTACK;
                    break;
                }
                case SPELL_DAMAGE_CLASS_MAGIC:
                {
                    if (!positive)
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    else
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                    break;
                }
                case SPELL_DAMAGE_CLASS_NONE:
                {
                    if (m_spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC)
                    {
                        if (!positive)
                            procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                        else
                            procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                    }
                    else
                    {
                        if (positive)
                            procAttacker |= PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
                        else
                            procAttacker |= PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    }
                    break;
                }
                case SPELL_DAMAGE_CLASS_RANGED:
                {
                    // Auto attack
                    if (!m_spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
                        procAttacker |= dmgSpell ? PROC_FLAG_DONE_SPELL_RANGED_DMG_CLASS : PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
                    break;
                }
            }

            //if (!(_triggeredCastFlags & TRIGGERED_DISALLOW_PROC_EVENTS))
                procEx |= PROC_EX_ON_CAST;

            TC_LOG_DEBUG(LOG_FILTER_PROC, "Cast m_spellInfo->Id %i, m_damage %i, procDamage %i, procEx %i procAttacker %u positive %u", m_spellInfo->Id, m_damage, procDamage, procEx, procAttacker, positive);

            if(procAttacker)
            {
                if(infoTarget)
                {
                    SpellNonMeleeDamage damageInfo(caster, procTarget, m_spellInfo->Id, m_SpellVisual, m_spellSchoolMask, m_castGuid[0]);
                    procEx |= createProcExtendMask(&damageInfo, infoTarget->missCondition);
                    DamageInfo dmgInfoProc = DamageInfo(caster, procTarget, procDamage, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, procDamage);
                    dmgInfoProc.SetStartCast(m_castedTime);
                    dmgInfoProc.SetCastTime(m_casttime);
                    dmgInfoProc.SetMagnet(m_magnetGuid);
                    caster->ProcDamageAndSpell(procTarget, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, &m_appliedProcMods, this);
                }
                else
                {
                    DamageInfo dmgInfoProc = DamageInfo(caster, procTarget, procDamage, m_spellInfo, SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask), SPELL_DIRECT_DAMAGE, procDamage);
                    dmgInfoProc.SetStartCast(m_castedTime);
                    dmgInfoProc.SetCastTime(m_casttime);
                    dmgInfoProc.SetMagnet(m_magnetGuid);
                    caster->ProcDamageAndSpell(procTarget, procAttacker, procVictim, procEx, &dmgInfoProc, m_attackType, m_spellInfo, m_triggeredByAuraSpell, &m_appliedProcMods, this);
                }
            }
        }
    }

    if (!hasDeley) // Immediate spell, no big deal
    {
        m_canLostTarget = false;
        handle_immediate();
    }

    CallScriptAfterCastHandlers();

    if (m_spellInfo->Cooldowns.RecoveryTime)
        if (Player* plr = m_caster->ToPlayer())
            if (plr->HasInstantCastModForSpell(m_spellInfo))
                plr->RemoveSpellCooldown(spellid, true);

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
        TakeReagents();
    else if (Item* targetItem = m_targets.GetItemTarget())
    {
        /// Not own traded item (in trader trade slot) req. reagents including triggered spell case
        if (targetItem->GetOwnerGUID() != m_caster->GetGUID())
            TakeReagents();
    }

    LinkedSpell(m_caster, m_targets.GetUnitTarget());

    if (hasDeley)
        if (Unit* target = m_targets.GetUnitTarget())
            if (target->HasAura(58984)) // I found only one spell, but maybe exist more
                m_canLostTarget = false;

    if (Player* plr = m_caster->ToPlayer())
    {
        plr->SetSpellModTakingSpell(this, false);
        //Clear spell cooldowns after every spell is cast if .cheat cooldown is enabled.
        if (plr->GetCommandStatus(CHEAT_COOLDOWN) || m_dispelResetCD)
            plr->RemoveSpellCooldown(m_spellInfo->Id, true);
    }

    SetExecutedCurrently(false);
    m_caster->DecreaseCastCounter();

    if (m_CastItem)
        m_CastItem->SetInUse(false);
}

void Spell::handle_immediate()
{
    PrepareTargetProcessing();

    // process immediate effects (items, ground, etc.) also initialize some variables
    _handle_immediate_phase();

    for (auto& ihit : m_UniqueTargetInfo)
        DoAllEffectOnTarget(ihit);

    for (auto& ihit : m_UniqueGOTargetInfo)
        DoAllEffectOnTarget(&ihit);

    FinishTargetProcessing();

    // spell is finished, perform some last features of the spell here
    _handle_finish_phase();

    // Remove used for cast item if need (it can be already NULL after TakeReagents call
    TakeCastItem();

    if (m_spellState != SPELL_STATE_CASTING)
        finish(true);                                       // successfully finish spell cast (not last in case autorepeat or channel spell)
}

uint64 Spell::handle_delayed(uint64 t_offset)
{
    UpdatePointers();

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, true);

    uint64 next_time = 0;

    PrepareTargetProcessing();

    if (!m_immediateHandled)
    {
        _handle_immediate_phase();
        m_immediateHandled = true;
    }

    bool single_missile = (m_targets.HasDst());

    uint32 _ss = getMSTime();
    // now recheck units targeting correctness (need before any effects apply to prevent adding immunity at first effect not allow apply second spell effect and similar cases)
    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit->processed == false)
        {
            if (single_missile || ihit->timeDelay <= t_offset)
            {
                ihit->timeDelay = t_offset;
                DoAllEffectOnTarget(ihit);
            }
            else if (next_time == 0 || ihit->timeDelay < next_time)
                next_time = ihit->timeDelay;
        }
    }

    // now recheck gameobject targeting correctness
    for (auto& ighit : m_UniqueGOTargetInfo)
    {
        if (ighit.processed == false)
        {
            if (single_missile || ighit.timeDelay <= t_offset)
                DoAllEffectOnTarget(&ighit);
            else if (next_time == 0 || ighit.timeDelay < next_time)
                next_time = ighit.timeDelay;
        }
    }

    FinishTargetProcessing();

    if (m_caster->IsPlayer())
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);

    uint32 _mss = GetMSTimeDiffToNow(_ss);
    if (_mss > 250)
        sLog->outDiff("Spell::handle_delayed Caster %u entry %u SpellId %u wait %ums GOTargetInfo %u TargetInfo %u",
            m_caster->GetGUIDLow(), m_caster->GetEntry(), m_spellInfo->Id, _mss, m_UniqueGOTargetInfo.size(), m_UniqueTargetInfo.size());

    // All targets passed - need finish phase
    if (next_time == 0)
    {
        // spell is finished, perform some last features of the spell here
        _handle_finish_phase();

        finish(true);                                       // successfully finish spell cast

        // return zero, spell is finished now
        return 0;
    }
    // spell is unfinished, return next execution time
    return next_time;
}

void Spell::_handle_immediate_phase()
{
    m_spellAura = nullptr;
    // initialize Diminishing Returns Data
    m_diminishLevel = DIMINISHING_LEVEL_1;
    m_diminishGroup = DIMINISHING_NONE;

    // handle some immediate features of the spell here
    HandleThreatSpells();

    PrepareScriptHitHandlers();

    // handle effects with SPELL_EFFECT_HANDLE_HIT mode
    for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->EffectMask < uint32(1 << j))
            break;

        // don't do anything for empty effect
        if (!m_spellInfo->Effects[j]->IsEffect())
            continue;

        // call effect handlers to handle destination hit
        HandleEffects(nullptr, nullptr, nullptr, j, SPELL_EFFECT_HANDLE_HIT);
    }

    // start channeling if applicable
    if (m_spellInfo->IsChanneled())
    {
        int32 duration = m_spellInfo->GetDuration(m_diffMode);
        if (duration)
        {
            // First mod_duration then haste - see Missile Barrage
            // Apply duration mod
            if (Player* modOwner = m_caster->GetSpellModOwner())
                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);

            // Apply haste mods
            if (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION_RECOVERY) || (m_spellInfo->HasAttribute(SPELL_ATTR8_HASTE_AFFECT_DURATION)))
                m_caster->ModSpellCastTime(m_spellInfo, duration, this);

            m_spellState = SPELL_STATE_CASTING;
            m_caster->AddInterruptMask(m_spellInfo->ChannelInterruptFlags);
            SendChannelStart(duration);
        }
        else if (duration == -1)
        {
            m_spellState = SPELL_STATE_CASTING;
            m_caster->AddInterruptMask(m_spellInfo->ChannelInterruptFlags);
            SendChannelStart(duration);
        }
    }

    // process items
    for (auto& ihit : m_UniqueItemInfo)
        DoAllEffectOnTarget(&ihit);

    if (!m_originalCaster)
        return;
    // Handle procs on cast
    // TODO: finish new proc system:P

    uint32 procAttacker = m_procAttacker;
    if (!procAttacker)
    {
        // Hunter trap spells - activation proc for Lock and Load, Entrapment and Misdirection
        if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_HUNTER &&
            (m_spellInfo->ClassOptions.SpellClassMask[0] & 0x18 ||     // Freezing and Frost Trap, Freezing Arrow
            m_spellInfo->Id == 57879 || m_spellInfo->Id == 13810 ||                     // Snake Trap - done this way to avoid double proc
            m_spellInfo->ClassOptions.SpellClassMask[2] & 0x00024000)) // Explosive and Immolation Trap
            procAttacker |= PROC_FLAG_DONE_TRAP_ACTIVATION;

        //procAttacker |= PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS;
        procAttacker |= PROC_FLAG_DONE_SPELL_MAGIC_DMG_POS_NEG;
    }

    if (m_UniqueTargetInfo.empty())
    {
        if (m_targets.HasDst())
        {
            // Proc the spells that have DEST target
            DamageInfo dmgInfoProc = DamageInfo(m_caster, nullptr, 0, m_spellInfo, m_spellInfo ? SpellSchoolMask(m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask) : SPELL_SCHOOL_MASK_NORMAL, SPELL_DIRECT_DAMAGE, 0);
            dmgInfoProc.SetStartCast(m_castedTime);
            dmgInfoProc.SetCastTime(m_casttime);
            dmgInfoProc.SetMagnet(m_magnetGuid);
            m_originalCaster->ProcDamageAndSpell(nullptr, procAttacker, 0, m_procEx | PROC_EX_NORMAL_HIT, &dmgInfoProc, BASE_ATTACK, m_spellInfo, m_triggeredByAuraSpell, nullptr, this);
        }
    }
    else
    {
        if (m_spellInfo->Cooldowns.StartRecoveryTime && m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MAGIC)
        {
            std::vector<uint32> spellId;
            std::list<AuraType> auralist;

            auralist.push_back(SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK);
            auralist.push_back(SPELL_AURA_HASTE_SPELLS);
            auralist.push_back(SPELL_AURA_MELEE_SLOW);

            for (auto& auratype : auralist)
            {
                if (Unit::AuraEffectList const* mModCastingSpeedNotStack = m_caster->GetAuraEffectsByType(auratype))
                    for (Unit::AuraEffectList::const_iterator i = mModCastingSpeedNotStack->begin(); i != mModCastingSpeedNotStack->end(); ++i)
                        if (SpellInfo const* sinfo = (*i)->GetSpellInfo())
                            if (sinfo->GetAuraOptions(m_diffMode)->ProcCharges && !(*i)->HasSpellClassMask())
                                spellId.push_back(sinfo->Id);
            }

            for (std::vector<uint32>::iterator itr = spellId.begin(); itr != spellId.end(); ++itr)
                m_caster->RemoveAura(*itr);
        }
    }
}

void Spell::_handle_finish_phase()
{
    if (!m_spellInfo->NoPower())
    {
        if (Player* _player = m_caster->ToPlayer())
        {
            // Take for real after all targets are processed
            if (m_powerCost[POWER_COMBO_POINTS])
            {
                if (AuraEffect const* soulOfTheForest = m_caster->GetAuraEffect(158476, EFFECT_0)) // Soul of the Forest
                {
                    float bp = soulOfTheForest->GetAmount() * GetComboPoints();
                    m_caster->CastCustomSpell(m_caster, 114113, &bp, nullptr, nullptr, true);
                }

                if(uint8 count = m_caster->GetSaveComboPoints())
                {
                    m_caster->AddComboPoints(count);
                    m_caster->SaveAddComboPoints(-count);
                }
            }
        }

        m_powerCost.assign(MAX_POWERS + 1, 0);
        m_caster->ResetPowerCost();
    }

    if (m_caster->m_extraAttacks && GetSpellInfo()->HasEffect(SPELL_EFFECT_ADD_EXTRA_ATTACKS))
        m_caster->HandleProcExtraAttackFor(m_caster->getVictim());

    // TODO: trigger proc phase finish here
}

void Spell::SendSpellCooldown()
{
    if (!m_caster->IsPlayer())
        return;

    Player* _player = (Player*)m_caster;

    uint32 itemEntry = 0;

    if (m_CastItem)
    {
        itemEntry = m_CastItem->GetEntry();

        if (m_CastItem->IsPotion() || m_spellInfo->IsCooldownStartedOnEvent() || IsItemCategoryCombat())
        {
            _player->SetLastPotionId(m_CastItem->GetEntry());
            return;
        }
    }
    else if (m_castItemEntry && (m_castFlags[1] & CAST_FLAG_EX_USE_TOY_SPELL))
    {
        itemEntry = m_castItemEntry;
    }

    if ((m_spellInfo->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE) || m_spellInfo->IsPassive()) || (_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD) || m_spellInfo->Categories.ChargeCategory)
        return;

    _player->AddSpellAndCategoryCooldowns(m_spellInfo, itemEntry, this);
}

void Spell::update(uint32 difftime)
{
    // update pointers based at it's GUIDs
    UpdatePointers();

    if (m_targets.GetUnitTargetGUID() && !m_targets.GetUnitTarget())
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell %u is cancelled due to removal of target.", m_spellInfo->Id);
        cancel();
        return;
    }

    // check if the player caster has moved before the spell finished
    // with the exception of spells affected with SPELL_AURA_CAST_WHILE_WALKING effect
    if ((m_caster->IsPlayer() && m_timer != 0) &&
        m_caster->isMoving() && (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT) &&
        (m_spellInfo->Effects[0]->Effect != SPELL_EFFECT_STUCK || !m_caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR)) &&
        !m_caster->HasAuraCastWhileWalking(m_spellInfo))
    {
        // don't cancel for melee, autorepeat, triggered and instant spells
        if (!IsNextMeleeSwingSpell() && !IsAutoRepeat() && !IsTriggered())
        {
            cancel();
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell %u is cancelled SPELL_INTERRUPT_FLAG_MOVEMENT", m_spellInfo->Id);
        }
    }

    switch (m_spellState)
    {
        case SPELL_STATE_PREPARING:
        {
            if (m_timer > 0)
            {
                if (difftime >= (uint32)m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if (m_timer == 0 && !IsNextMeleeSwingSpell() && !IsAutoRepeat())
                // don't CheckCast for instant spells - done in spell::prepare, skip duplicate checks, needed for range checks for example
                cast(!m_casttime);
            break;
        }
        case SPELL_STATE_CASTING:
        {
            if (m_timer)
            {
                // check if there are alive targets left
                if (!UpdateChanneledTargetList())
                {
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Channeled spell %d is removed due to lack of targets", m_spellInfo->Id);
                    SendChannelUpdate(0);
                    finish();
                }

                if (m_timer > 0)
                {
                    if (difftime >= (uint32)m_timer)
                        m_timer = 0;
                    else
                        m_timer -= difftime;
                }
            }

            if (m_timer == 0)
            {
                if (Aura* aura = m_caster->GetAura(m_spellInfo->Id))
                    if (!aura->IsExpired())
                        break;

                SendChannelUpdate(0);
                finish();
            }
            break;
        }
        default:
            break;
    }
}

void Spell::finish(bool ok)
{
    if (!m_caster)
        return;

    if (m_CastItem)
        m_CastItem->SetInUse(false);

    if (m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;

    if (m_spellInfo->IsChanneled())
        m_caster->UpdateInterruptMask();

    if (m_caster->HasUnitState(UNIT_STATE_CASTING) && !m_caster->IsNonMeleeSpellCast(false, false, true))
        m_caster->ClearUnitState(UNIT_STATE_CASTING);

    if (m_spellInfo->IsAllowsCastWhileMove(m_caster))
        m_caster->ClearUnitState(UNIT_STATE_MOVE_IN_CASTING);

    // Unsummon summon as possessed creatures on spell cancel
    if (m_spellInfo->IsChanneled() && m_caster->IsPlayer())
    {
        if (Unit* charm = m_caster->GetCharm())
            if (charm->IsCreature() && charm->ToCreature()->HasUnitTypeMask(UNIT_MASK_PUPPET) && charm->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL) == m_spellInfo->Id)
                ((Puppet*)charm)->UnSummon();
    }

    if (m_caster->IsCreature())
        m_caster->ReleaseFocus(this);

    if (!ok)
    {
        LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_FAILED_CAST);
        CallScriptOnFinishCastHandlers();
        return;
    }

    if (m_caster->IsCreature() && m_caster->ToCreature()->isSummon())
    {
        // Unsummon statue
        uint32 spell = m_caster->GetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL);
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell);
        if (spellInfo && spellInfo->GetMisc(m_diffMode)->MiscData.IconFileDataID == 134230)
        {
            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Statue %d is unsummoned in spell %d finish", m_caster->GetGUIDLow(), m_spellInfo->Id);
            m_caster->setDeathState(JUST_DIED);
            return;
        }
    }

    if (IsAutoActionResetSpell())
    {
        if (!m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_RESET_AUTO_ACTIONS))
        {
            m_caster->resetAttackTimer(BASE_ATTACK);
            if (m_caster->haveOffhandWeapon())
                m_caster->resetAttackTimer(OFF_ATTACK);
            m_caster->resetAttackTimer(RANGED_ATTACK);
        }
    }

    // potions disabled by client, send event "not in combat" if need
    if (m_caster->IsPlayer())
    {
        if (!m_triggeredByAuraSpell)
            m_caster->ToPlayer()->UpdatePotionCooldown(this);

        // triggered spell pointer can be not set in some cases
        // this is needed for proper apply of triggered spell mods
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, true);

        // Take mods after trigger spell (needed for 14177 to affect 48664)
        // mods are taken only on succesfull cast and independantly from targets of the spell
        m_caster->ToPlayer()->RemoveSpellMods(this);
        m_caster->ToPlayer()->SetSpellModTakingSpell(this, false);
    }

    // Stop Attack for some spells
    if (m_spellInfo->HasAttribute(SPELL_ATTR0_STOP_ATTACK_TARGET))
        m_caster->AttackStop();

    switch (m_spellInfo->Id)
    {
        // Hack for some stealth spells
        case 32612:
        case 110960: 
        case 58984: 
            m_caster->CombatStop();
            break;
    }

    if (m_castItemGUID && m_caster->IsPlayer())
        if (Item* item = m_caster->ToPlayer()->GetItemByGuid(m_castItemGUID))
            if (item->IsEquipable() && !item->IsEquipped())
                m_caster->ToPlayer()->ApplyItemEquipSpell(item, false);

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::finish Spell: %u m_spellMissMask %u", m_spellInfo->Id, m_spellMissMask);
    #endif

    LinkedSpell(m_caster, m_targets.GetUnitTarget(), SPELL_LINK_FINISH_CAST);
    CallScriptOnFinishCastHandlers();
}

void Spell::SendCastResult(SpellCastResult result)
{
    if (result == SPELL_CAST_OK)
        return;

    if (!m_caster->IsPlayer())
        return;

    if (m_caster->ToPlayer()->GetSession()->PlayerLoading())  // don't send cast results at loading time
        return;

    SendCastResult(m_caster->ToPlayer(), m_spellInfo, result, m_customError, m_miscData);
}

void Spell::SendCastResult(Player* caster, SpellInfo const* spellInfo, SpellCastResult result, SpellCustomErrors customError /*= SPELL_CUSTOM_ERROR_NONE*/, uint32* misc /*= nullptr*/, bool pet /*=false*/)
{
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "SendCastResult  Spell: %u result %u.", spellInfo->Id, result);

    if (result == SPELL_CAST_OK)
        return;

    WorldPackets::Spells::CastFailed packet(pet ? SMSG_PET_CAST_FAILED : SMSG_CAST_FAILED);
    if (m_castGuid[1].IsEmpty())
        packet.SpellGuid = m_castGuid[0];
    else
        packet.SpellGuid = m_castGuid[1];

    packet.SpellXSpellVisualID = m_SpellVisual;
    packet.SpellID = spellInfo->Id;
    packet.Reason = result;

    switch (result)
    {
        case SPELL_FAILED_NO_POWER:
            packet.FailedArg1 = m_failedArg[0];
            break;
        case SPELL_FAILED_NOT_READY:
            packet.FailedArg1 = 0;                                    // unknown (value 1 update cooldowns on client flag)                       
            break;
        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
            packet.FailedArg1 = m_failedArg[0];
            break;
        case SPELL_FAILED_REQUIRES_AREA:                    // AreaTable.dbc id
        {
            // hardcode areas limitation case
            switch (spellInfo->Id)
            {
                case 41617:                                 // Cenarion Mana Salve
                case 41619:                                 // Cenarion Healing Salve
                    packet.FailedArg1 = 3905;
                    break;
                case 41618:                                 // Bottled Nethergon Energy
                case 41620:                                 // Bottled Nethergon Vapor
                    packet.FailedArg1 = 3842;
                    break;
                case 45373:                                 // Bloodberry Elixir
                    packet.FailedArg1 = 4075;
                    break;
                default:                                    // default case (don't must be)
                    break;
            }
            break;
        }
        case SPELL_FAILED_TOTEMS:
            packet.FailedArg1 = m_failedArg[0];
            packet.FailedArg2 = m_failedArg[1];
            break;
        case SPELL_FAILED_TOTEM_CATEGORY:
            packet.FailedArg1 = spellInfo->Totems.TotemCategory[0];
            packet.FailedArg2 = spellInfo->Totems.TotemCategory[1];

            //if (hasBit1 && hasBit2 && (!spellInfo->TotemCategory[0] || !spellInfo->TotemCategory[1]))
            //    sLog->OutCS(">>> SPELL_FAILED_TOTEM_CATEGORY ");

            break;
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_MAINHAND:
        case SPELL_FAILED_EQUIPPED_ITEM_CLASS_OFFHAND:
            packet.FailedArg1 = spellInfo->EquippedItemClass;
            packet.FailedArg2 = spellInfo->EquippedItemSubClassMask;
            break;
        case SPELL_FAILED_TOO_MANY_OF_ITEM:
        {
            for (uint32 eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
            {
                if (m_spellInfo->EffectMask < uint32(1 << eff))
                    break;

                if (uint32 item = spellInfo->Effects[eff]->ItemType)
                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item))
                    {
                        packet.FailedArg1 = proto->GetLimitCategory();
                        break;
                    }
            }
            break;
        }
        case SPELL_FAILED_PREVENTED_BY_MECHANIC:
            packet.FailedArg1 = spellInfo->GetAllEffectsMechanicMask();   // SpellMechanic.dbc id
            break;
        case SPELL_FAILED_NEED_EXOTIC_AMMO:
            packet.FailedArg1 = spellInfo->EquippedItemSubClassMask;          // seems correct...
            break;
        case SPELL_FAILED_NEED_MORE_ITEMS:
            //param1 = 0;                              // Item count?
            //param2 = 0;                              // Item id
            break;
        case SPELL_FAILED_MIN_SKILL:
            //param1 = 0;                              // SkillLine.dbc id
            //param2 = 0;                              // required skill value
            break;
        case SPELL_FAILED_FISHING_TOO_LOW:
            //param1 = 0;                              // required fishing skill
            break;
        case SPELL_FAILED_CUSTOM_ERROR:
            packet.FailedArg1 = customError;
            break;
        case SPELL_FAILED_SILENCED:
            //param1 = 0;                              // Unknown
            break;
        case  SPELL_FAILED_CANT_UNTALENT:
            if (TalentEntry const* talent = sTalentStore.LookupEntry(misc[0]))
                packet.FailedArg1 = talent->SpellID;
            break;
        case SPELL_FAILED_REAGENTS:
            packet.FailedArg1 = m_failedArg[0];
            packet.FailedArg2 = m_failedArg[1];
            break;
        // TODO: SPELL_FAILED_NOT_STANDING
        default:
            break;
    }

    caster->SendDirectMessage(packet.Write());
}

void Spell::SendSpellCastGuids()
{
    if (m_spellGuid.IsEmpty())
        return;

    WorldPackets::Spells::SpellPrepare packet;
    packet.ClientCastID = m_spellGuid;
    packet.ServerCastID = m_castGuid[0];
    m_caster->SendMessageToSet(packet.Write(), true);
}

void Spell::SendSpellStart()
{
    if (!IsNeedSendToClient())
        return;

    uint32 castFlags = CAST_FLAG_HAS_TRAJECTORY;
    uint32 castFlagsEx = m_castFlags[1];
    uint32 schoolImmunityMask = m_caster->GetSchoolImmunityMask();
    uint32 mechanicImmunityMask = m_caster->GetMechanicImmunityMask();

    if (auto creature = m_caster->ToCreature())
    {
        if (creature->GetCreatureTemplate()->MechanicImmuneMask & (1 << (MECHANIC_INTERRUPT - 1)))
            mechanicImmunityMask = 3;
    }

    if (schoolImmunityMask || mechanicImmunityMask)
        castFlags |= CAST_FLAG_IMMUNITY;

    if ((IsTriggered() && !m_spellInfo->IsAutoRepeatRangedSpell() && !m_spellInfo->HasAttribute(SPELL_ATTR4_TRIGGERED)) || m_triggeredByAuraSpell)
        castFlags |= CAST_FLAG_PENDING;
        
    if (m_spellInfo->HasAttribute(SPELL_ATTR0_REQ_AMMO))
        castFlags |= CAST_FLAG_PROJECTILE;

    if (((m_caster->IsCreature() && (m_caster->ToCreature()->isPet() || m_caster->ToCreature()->IsVehicle())))
        && !GetSpellInfo()->HasPower(POWER_HEALTH))
        castFlags |= CAST_FLAG_POWER_LEFT_SELF;

    if (GetSpellInfo()->HasPower(POWER_RUNES))
        castFlags |= CAST_FLAG_NO_GCD;

    if (m_spellInfo->Cooldowns.StartRecoveryTime)
        castFlags |= CAST_FLAG_NO_GCD;

    if (_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD && !m_CastItem)
        castFlagsEx |= CAST_FLAG_EX_NO_CD;

    WorldPackets::Spells::SpellStart packet;
    WorldPackets::Spells::SpellCastData& castData = packet.Cast;

    castData.CastGuid = m_castGuid[0];
    if (_triggeredCastFlags)
        castData.CastGuid2 = m_castGuid[1];
    castData.CasterGUID = m_CastItem ? m_CastItem->GetGUID() : m_caster->GetGUID();
    castData.CasterUnit = m_caster->GetGUID();
    castData.SpellID = m_spellInfo->Id;
    castData.SpellXSpellVisualID = m_SpellVisual;
    castData.CastFlags = castFlags;
    castData.CastFlagsEx = castFlagsEx;
    castData.CastTime = m_casttime;

    m_targets.Write(castData.Target);

    if (castFlags & CAST_FLAG_POWER_LEFT_SELF)
    {
        if (m_spellInfo->NoPower())
            castData.CastFlags &= ~CAST_FLAG_POWER_LEFT_SELF;
        else
        {
            SpellPowerData powerData;
            if (GetSpellInfo()->GetSpellPowerByCasterPower(m_caster, powerData))
            {
                for (SpellPowerEntry const* power : powerData)
                    castData.RemainingPower.emplace_back(WorldPackets::Spells::SpellPowerData(m_caster->GetPower(Powers(power->PowerType)), power->PowerType));
            }
            else
                castData.CastFlags &= ~CAST_FLAG_POWER_LEFT_SELF;
        }
    }

    if (castFlags & CAST_FLAG_RUNE_LIST) // rune cooldowns list
    {
        castData.RemainingRunes = boost::in_place();

        //TODO: There is a crash caused by a spell with CAST_FLAG_RUNE_LIST casted by a creature
        //The creature is the mover of a player, so HandleCastSpellOpcode uses it as the caster
        if (Player* player = m_caster->ToPlayer())
        {
            castData.RemainingRunes->Start = m_runesState; // runes state before
            castData.RemainingRunes->Count = player->GetRunesState(); // runes state after
            for (uint8 i = 0; i < player->GetMaxPower(POWER_RUNES); ++i)
            {
                // float casts ensure the division is performed on floats as we need float result
                float baseCd = player->GetRuneBaseCooldown();
                castData.RemainingRunes->Cooldowns.push_back((baseCd - float(player->GetRuneCooldown(i))) / baseCd * 255); // rune cooldown passed
            }
        }
        else
        {
            castData.RemainingRunes->Start = 0;
            castData.RemainingRunes->Count = 0;
            for (uint8 i = 0; i < MAX_RUNES; ++i)
                castData.RemainingRunes->Cooldowns.push_back(0);
        }
    }
    
    if (castFlags & CAST_FLAG_PROJECTILE)
        UpdateSpellCastDataAmmo(castData.Ammo);

    if (castFlags & CAST_FLAG_IMMUNITY)
    {
        castData.Immunities.School = schoolImmunityMask;
        castData.Immunities.Value = mechanicImmunityMask;
    }

    m_caster->SendMessageToSet(packet.Write(), true);
}

void Spell::SendSpellPendingCast()
{
    uint32 spellID = 0;
    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (auto const* spellPending = sSpellMgr->GetSpellPendingCast(m_spellInfo->Id))
    {
        bool check = false;
        for (auto i : *spellPending)
        {
            switch (i.option)
            {
                case 0: // Check Spec
                {
                    if (player->GetLootSpecID() == i.check)
                    {
                        spellID = i.pending_id;
                        check = true;
                    }
                    break;
                }
            }

            if (check)
                break;
        }
    }

    if (spellID)
        player->SendDirectMessage(WorldPackets::Spells::ScriptCast(spellID).Write());
}

void Spell::SendSpellGo()
{
    // not send invisible spell casting
    if (!IsNeedSendToClient())
        return;

    uint32 castFlags = CAST_FLAG_UNKNOWN_9;
    uint32 castFlagsEx = m_castFlags[1];
    // triggered spells with spell visual != 0
    if ((IsTriggered() && !m_spellInfo->IsAutoRepeatRangedSpell() && !m_spellInfo->HasAttribute(SPELL_ATTR4_TRIGGERED)) || m_triggeredByAuraSpell)
        castFlags |= CAST_FLAG_PENDING;

    if (m_spellInfo->HasAttribute(SPELL_ATTR0_REQ_AMMO))
        castFlags |= CAST_FLAG_PROJECTILE; 

    if (m_caster->IsPlayer() || (m_caster->IsCreature() && (m_caster->ToCreature()->isAnySummons() || m_caster->ToCreature()->IsVehicle())))
        castFlags |= CAST_FLAG_POWER_LEFT_SELF; // should only be sent to self, but the current messaging doesn't make that possible

    if ((m_caster->IsPlayer()) && (m_caster->getClass() == CLASS_DEATH_KNIGHT) && GetSpellInfo()->HasPower(POWER_RUNES))
    {
        castFlags |= CAST_FLAG_NO_GCD;                   // same as in SMSG_SPELL_START
        castFlags |= CAST_FLAG_RUNE_LIST;                    // rune cooldowns list
    }

    if (m_targets.HasTraj())
        castFlags |= CAST_FLAG_ADJUST_MISSILE;

    if (m_spellInfo->Cooldowns.StartRecoveryTime)
        castFlags |= CAST_FLAG_NO_GCD;

    if (_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD && !m_CastItem)
    {
        castFlagsEx |= CAST_FLAG_EX_NO_COOLDOWN;
        castFlagsEx |= CAST_FLAG_EX_NO_CD;
    }

    WorldPackets::Spells::SpellGo packet;
    WorldPackets::Spells::SpellCastData& castData = packet.Cast;

    castData.CasterGUID = /*(m_CastItem && !m_CastItem->IsEquipped()) ? m_CastItem->GetGUID() : */m_caster->GetGUID();
    castData.CasterUnit = m_caster->GetGUID();
    castData.CastGuid = m_castGuid[0];
    if (_triggeredCastFlags)
        castData.CastGuid2 = m_castGuid[1];
    castData.SpellXSpellVisualID = m_SpellVisual;
    castData.SpellID = m_spellInfo->Id;
    castData.CastFlags = castFlags;
    castData.CastFlagsEx = castFlagsEx;
    castData.CastTime = m_castedTime;

    UpdateSpellCastDataTargets(castData);

    m_targets.Write(castData.Target);

    if (castFlags & CAST_FLAG_POWER_LEFT_SELF)
    {
        if (m_spellInfo->NoPower())
            castData.CastFlags &= ~CAST_FLAG_POWER_LEFT_SELF;
        else
        {
            SpellPowerData powerData;
            if (m_spellInfo->GetSpellPowerByCasterPower(m_caster, powerData))
            {
                for (SpellPowerEntry const* power : powerData)
                    castData.RemainingPower.emplace_back(WorldPackets::Spells::SpellPowerData(m_caster->GetPower(Powers(power->PowerType)), power->PowerType));
            }
            else
                castData.CastFlags &= ~CAST_FLAG_POWER_LEFT_SELF;
        }
    }

    if (castFlags & CAST_FLAG_RUNE_LIST) // rune cooldowns list
    {
        castData.RemainingRunes = boost::in_place();
        //TODO: There is a crash caused by a spell with CAST_FLAG_RUNE_LIST casted by a creature
        //The creature is the mover of a player, so HandleCastSpellOpcode uses it as the caster
        if (Player* player = m_caster->ToPlayer())
        {
            castData.RemainingRunes->Start = m_runesState; // runes state before
            castData.RemainingRunes->Count = player->GetRunesState(); // runes state after
            for (uint8 i = 0; i < player->GetMaxPower(POWER_RUNES); ++i)
            {
                // float casts ensure the division is performed on floats as we need float result
                float baseCd = player->GetRuneBaseCooldown();
                castData.RemainingRunes->Cooldowns.push_back((baseCd - float(player->GetRuneCooldown(i))) / baseCd * 255); // rune cooldown passed
            }
        }
        else
        {
            castData.RemainingRunes->Start = 0;
            castData.RemainingRunes->Count = 0;
            for (uint8 i = 0; i < MAX_RUNES; ++i)
                castData.RemainingRunes->Cooldowns.push_back(0);
        }
    }

    if (castFlags & CAST_FLAG_ADJUST_MISSILE)
    {
        castData.MissileTrajectory.TravelTime = m_delayMoment;
        castData.MissileTrajectory.Pitch = m_targets.GetPitch();
    }

    packet.LogData.Initialize(this);
    m_caster->SendCombatLogMessage(&packet);
}

void Spell::SendSpellExecuteLog()
{
    WorldPackets::CombatLog::SpellExecuteLog spellExecuteLog;
    spellExecuteLog.Caster = m_caster->GetGUID();
    spellExecuteLog.SpellID = m_spellInfo->Id;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if ((m_spellInfo->EffectMask & (1 << i)) == 0)
            continue;

        if (_powerDrainTargets[i].empty() && _extraAttacksTargets[i].empty() && _durabilityDamageTargets[i].empty() && _genericVictimTargets[i].empty() && _tradeSkillTargets[i].empty() && _feedPetTargets[i].empty())
            continue;

        WorldPackets::CombatLog::SpellExecuteLog::SpellLogEffect spellLogEffect;
        spellLogEffect.Effect = m_spellInfo->GetEffect(i)->Effect;
        spellLogEffect.PowerDrainTargets = std::move(_powerDrainTargets[i]);
        spellLogEffect.ExtraAttacksTargets = std::move(_extraAttacksTargets[i]);
        spellLogEffect.DurabilityDamageTargets = std::move(_durabilityDamageTargets[i]);
        spellLogEffect.GenericVictimTargets = std::move(_genericVictimTargets[i]);
        spellLogEffect.TradeSkillTargets = std::move(_tradeSkillTargets[i]);
        spellLogEffect.FeedPetTargets = std::move(_feedPetTargets[i]);
        spellExecuteLog.Effects.push_back(spellLogEffect);
    }

    m_caster->SendCombatLogMessage(&spellExecuteLog);
}

void Spell::ExecuteLogEffectTakeTargetPower(uint8 effIndex, Unit* target, uint32 powerType, uint32 points, float amplitude)
{
    SpellLogEffectPowerDrainParams spellLogEffectPowerDrainParams;
    spellLogEffectPowerDrainParams.Victim = target->GetGUID();
    spellLogEffectPowerDrainParams.Points = points;
    spellLogEffectPowerDrainParams.PowerType = powerType;
    spellLogEffectPowerDrainParams.Amplitude = amplitude;
    _powerDrainTargets[effIndex].push_back(spellLogEffectPowerDrainParams);
}

void Spell::ExecuteLogEffectExtraAttacks(uint8 effIndex, Unit* victim, uint32 numAttacks)
{
    SpellLogEffectExtraAttacksParams spellLogEffectExtraAttacksParams;
    spellLogEffectExtraAttacksParams.Victim = victim->GetGUID();
    spellLogEffectExtraAttacksParams.NumAttacks = numAttacks;
    _extraAttacksTargets[effIndex].push_back(spellLogEffectExtraAttacksParams);
}

void Spell::ExecuteLogEffectDurabilityDamage(uint8 effIndex, Unit* victim, int32 itemId, int32 amount)
{
    SpellLogEffectDurabilityDamageParams spellLogEffectDurabilityDamageParams;
    spellLogEffectDurabilityDamageParams.Victim = victim->GetGUID();
    spellLogEffectDurabilityDamageParams.ItemID = itemId;
    spellLogEffectDurabilityDamageParams.Amount = amount;
    _durabilityDamageTargets[effIndex].push_back(spellLogEffectDurabilityDamageParams);
}

void Spell::ExecuteLogEffectOpenLock(uint8 effIndex, Object* obj)
{
    SpellLogEffectGenericVictimParams spellLogEffectGenericVictimParams;
    spellLogEffectGenericVictimParams.Victim = obj->GetGUID();
    _genericVictimTargets[effIndex].push_back(spellLogEffectGenericVictimParams);
}

void Spell::ExecuteLogEffectCreateItem(uint8 effIndex, uint32 entry)
{
    SpellLogEffectTradeSkillItemParams spellLogEffectTradeSkillItemParams;
    spellLogEffectTradeSkillItemParams.ItemID = entry;
    _tradeSkillTargets[effIndex].push_back(spellLogEffectTradeSkillItemParams);
}

void Spell::ExecuteLogEffectDestroyItem(uint8 effIndex, uint32 entry)
{
    SpellLogEffectFeedPetParams spellLogEffectFeedPetParams;
    spellLogEffectFeedPetParams.ItemID = entry;
    _feedPetTargets[effIndex].push_back(spellLogEffectFeedPetParams);
}

void Spell::ExecuteLogEffectSummonObject(uint8 effIndex, WorldObject* obj)
{
    SpellLogEffectGenericVictimParams spellLogEffectGenericVictimParams;
    spellLogEffectGenericVictimParams.Victim = obj->GetGUID();
    _genericVictimTargets[effIndex].push_back(spellLogEffectGenericVictimParams);
}

void Spell::ExecuteLogEffectUnsummonObject(uint8 effIndex, WorldObject* obj)
{
    SpellLogEffectGenericVictimParams spellLogEffectGenericVictimParams;
    spellLogEffectGenericVictimParams.Victim = obj->GetGUID();
    _genericVictimTargets[effIndex].push_back(spellLogEffectGenericVictimParams);
}

void Spell::ExecuteLogEffectResurrect(uint8 effect, Unit* target)
{
    SpellLogEffectGenericVictimParams spellLogEffectGenericVictimParams;
    spellLogEffectGenericVictimParams.Victim = target->GetGUID();
    _genericVictimTargets[effect].push_back(spellLogEffectGenericVictimParams);
}

void Spell::SendInterrupted(uint8 result)
{
    WorldPackets::Spells::SpellFailure failurePacket;
    failurePacket.CasterUnit = m_caster->GetGUID();
    failurePacket.CastGUID = m_castGuid[0];
    failurePacket.SpellID = m_spellInfo->Id;
    failurePacket.SpellXSpellVisualID = m_SpellVisual;
    failurePacket.Reason = result;
    m_caster->SendMessageToSet(failurePacket.Write(), true);

    WorldPackets::Spells::SpellFailedOther failedPacket;
    failedPacket.CasterUnit = m_caster->GetGUID();
    failedPacket.CastGUID = m_castGuid[0];
    failedPacket.SpellID = m_spellInfo->Id;
    failedPacket.SpellXSpellVisualID = m_SpellVisual;
    failedPacket.Reason = result;
    m_caster->SendMessageToSet(failedPacket.Write(), true);
}

void Spell::ExecuteLogEffectInterruptCast(uint8 /*effIndex*/, Unit* victim, uint32 spellId)
{
    WorldPackets::CombatLog::SpellInterruptLog data;
    data.Caster = m_caster->GetGUID();
    data.Victim = victim->GetGUID();
    data.InterruptedSpellID = m_spellInfo->Id;
    data.SpellID = spellId;
    m_caster->SendMessageToSet(data.Write(), true);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if (time == 0)
    {
        m_caster->ClearDynamicValue(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS);
        m_caster->SetChannelSpellId(0);
        m_caster->SetChannelSpellXSpellVisualId(0);
    }

    WorldPackets::Spells::SpellChannelUpdate spellChannelUpdate;
    spellChannelUpdate.CasterGUID = m_caster->GetGUID();
    spellChannelUpdate.TimeRemaining = time;
    m_caster->SendMessageToSet(spellChannelUpdate.Write(), true);
}

void Spell::SendChannelStart(uint32 duration)
{
    WorldPackets::Spells::SpellChannelStart spellChannelStart;
    spellChannelStart.CasterGUID = m_caster->GetGUID();
    spellChannelStart.SpellID = m_spellInfo->Id;
    spellChannelStart.SpellXSpellVisualID = m_SpellVisual;
    spellChannelStart.ChannelDuration = duration;
    
    uint32 schoolImmunityMask = m_caster->GetSchoolImmunityMask();
    uint32 mechanicImmunityMask = m_caster->GetMechanicImmunityMask();

    if (schoolImmunityMask || mechanicImmunityMask)
    {
        spellChannelStart.InterruptImmunities = boost::in_place();
        spellChannelStart.InterruptImmunities->SchoolImmunities = schoolImmunityMask;
        spellChannelStart.InterruptImmunities->Immunities = mechanicImmunityMask;
    }
    m_caster->SendMessageToSet(spellChannelStart.Write(), true);

    m_timer = duration;

    if (m_spellInfo->HasAttribute(SPELL_ATTR1_IS_FISHING))
        return;

    for (auto const& target : m_UniqueTargetInfo)
        m_caster->AddChannelObject(target->targetGUID);

    for (GOTargetInfo const& target : m_UniqueGOTargetInfo)
        m_caster->AddChannelObject(target.targetGUID);

    if (m_spellInfo->Id != 101546)
        m_caster->SetChannelSpellId(m_spellInfo->Id);
    m_caster->SetChannelSpellXSpellVisualId(m_SpellVisual);
}

void Spell::SendResurrectRequest(Player* target)
{
    WorldPackets::Spells::ResurrectRequest resurrectRequest;
    resurrectRequest.ResurrectOffererGUID =  m_caster->GetGUID();
    resurrectRequest.ResurrectOffererVirtualRealmAddress = GetVirtualRealmAddress();
    resurrectRequest.SpellID = m_spellInfo->Id;
    resurrectRequest.Sickness = !m_caster->IsPlayer();
    resurrectRequest.Name = m_caster->IsPlayer() ? "" : m_caster->GetNameForLocaleIdx(target->GetSession()->GetSessionDbLocaleIndex());

    if (Pet* pet = target->GetPet())
        if (CharmInfo* charmInfo = pet->GetCharmInfo())
            resurrectRequest.PetNumber = charmInfo->GetPetNumber();

    target->SendDirectMessage(resurrectRequest.Write());
}

void Spell::TakeCastItem()
{
    if (!m_CastItem || !m_caster->IsPlayer() || !m_CastItem->IsInWorld())
        return;

    // TODO : research spell attributes or item flags
    bool alwaysDestroy = false;
    if (m_spellInfo->HasEffect(SPELL_EFFECT_GIVE_CURRENCY) || m_spellInfo->HasEffect(SPELL_EFFECT_GIVE_REPUTATION) || m_spellInfo->HasEffect(SPELL_EFFECT_ADD_GARRISON_FOLLOWER) || m_spellInfo->HasEffect(SPELL_EFFECT_LEARN_GARRISON_BUILDING))
        alwaysDestroy = true;

    if (m_spellInfo->HasEffect(SPELL_EFFECT_KILL_CREDIT) && !alwaysDestroy) // If item deleted by quest we have crash example http://pastebin.com/bvcaFJyA
        return;

    uint32 itemId = m_CastItem->GetEntry();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return;

    switch (itemId)
    {
        case 139461: // Rune of Darkness
        case 139459: // Blessing of the Light
            alwaysDestroy = true;
            break;
        default:
            break;
    }

    // not remove cast item at triggered spell (equipping, weapon damage, etc)
    if (_triggeredCastFlags & TRIGGERED_IGNORE_CAST_ITEM && !alwaysDestroy)
        return;

    bool expendable = false;
    bool withoutCharges = false;

    for (auto const& v : proto->Effects)
    {
        if (!v->SpellID)
            continue;

        if (v->SpellID != m_spellInfo->Id) //Example item=47030
            continue;

        if (v->Charges)
        {
            if (v->Charges < 0)
                expendable = true;

            int32 charges = m_CastItem->GetSpellCharges(v->LegacySlotIndex);

            if (charges)
            {
                (charges > 0) ? --charges : ++charges;
                if (proto->GetStackable() == 1)
                    m_CastItem->SetSpellCharges(v->LegacySlotIndex, charges);
                m_CastItem->SetState(ITEM_CHANGED, (Player*)m_caster);
            }

            withoutCharges = (charges == 0);
        }
    }

    if (expendable && withoutCharges)
    {
        uint32 count = 1;
        m_CastItem->SetInUse(false);
        m_caster->ToPlayer()->DestroyItemCount(m_CastItem, count, true);

        // prevent crash at access to deleted m_targets.GetItemTarget
        if (m_CastItem == m_targets.GetItemTarget())
            m_targets.SetItemTarget(nullptr);

        m_CastItem = nullptr;
        m_castItemEntry = 0;
        m_castItemGUID.Clear();
    }
}

bool Spell::CanDestoyCastItem()
{
    if (!m_CastItem || !m_caster->IsPlayer() || !m_CastItem->IsInWorld())
        return false;

    // TODO : research spell attributes or item flags
    bool alwaysDestroy = false;
    if (m_spellInfo->HasEffect(SPELL_EFFECT_GIVE_CURRENCY) || m_spellInfo->HasEffect(SPELL_EFFECT_GIVE_REPUTATION) || m_spellInfo->HasEffect(SPELL_EFFECT_ADD_GARRISON_FOLLOWER) || m_spellInfo->HasEffect(SPELL_EFFECT_LEARN_GARRISON_BUILDING))
        alwaysDestroy = true;

    if (m_spellInfo->HasEffect(SPELL_EFFECT_KILL_CREDIT) && !alwaysDestroy) // If item deleted by quest we have crash example http://pastebin.com/bvcaFJyA
        return false;

    uint32 itemId = m_CastItem->GetEntry();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    switch (itemId)
    {
        case 139461: // Rune of Darkness
        case 139459: // Blessing of the Light
            alwaysDestroy = true;
            break;
        default:
            break;
    }

    // not remove cast item at triggered spell (equipping, weapon damage, etc)
    if (_triggeredCastFlags & TRIGGERED_IGNORE_CAST_ITEM && !alwaysDestroy)
        return false;

    for (auto const& v : proto->Effects)
    {
        if (!v->SpellID)
            continue;

        if (v->SpellID != m_spellInfo->Id) //Example item=47030
            continue;

        if (v->Charges)
            return true;
    }

    return false;
}

void Spell::TakePower()
{
    if (m_CastItem || m_triggeredByAuraSpell || m_spellInfo->NoPower())
        return;
    //Don't take power if the spell is cast while .cheat power is enabled.
    if (m_caster->IsPlayer())
    {
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_POWER))
            return;
    }

    SpellPowerData powerData;
    if (!GetSpellInfo()->GetSpellPowerByCasterPower(m_caster, powerData))
        return;

    for (SpellPowerEntry const* power : powerData)
    {
        Powers powerType = Powers(power->PowerType);
        int32 _powerCost = GetPowerCost(powerType);
        int32 ifMissedPowerCost = GetPowerCost(powerType);
        bool hit = true;

        if (m_caster->IsPlayer())
        {
            if (ObjectGuid targetGUID = m_targets.GetUnitTargetGUID())
                for (auto& ihit : m_UniqueTargetInfo)
                    if (ihit->targetGUID == targetGUID)
                    {
                        if (ihit->missCondition != SPELL_MISS_NONE && ihit->missCondition != SPELL_MISS_IMMUNE)
                        {
                            switch (powerType)
                            {
                            case POWER_CHI:
                            case POWER_HOLY_POWER:
                            case POWER_COMBO_POINTS:
                            case POWER_RUNES:
                            case POWER_RUNIC_POWER:
                                ifMissedPowerCost = 0;
                                break;
                            case POWER_ENERGY:
                            case POWER_FOCUS:
                            case POWER_RAGE:
                            case POWER_FURY:
                            case POWER_MAELSTROM:
                                ifMissedPowerCost = CalculatePct(_powerCost, 20);
                                break;
                            default:
                                break;
                            }
                            hit = false;
                            //lower spell cost on fail (by talent aura)
                            if (Player* modOwner = m_caster->ToPlayer()->GetSpellModOwner())
                                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_SPELL_COST_REFUND_ON_FAIL, _powerCost);
                        }
                        break;
                    }
        }

        if (powerType == POWER_RUNES && hit)
            TakeRunePower();

        if (powerType == POWER_HEALTH)
            CallScriptTakePowerHandlers(powerType, m_powerCost[MAX_POWERS]);
        else
            CallScriptTakePowerHandlers(powerType, m_powerCost[powerType]);

        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::TakePower hit %i, powerType %i, m_powerCost %i Id %u", hit, powerType, GetPowerCost(powerType), m_spellInfo->Id);

        if (powerType == POWER_INSANITY) // Prist not use power on cast, is use when aura tick
            continue;

        if (powerType == POWER_RUNIC_POWER)
        {
            if (m_spellInfo->Power.PowerCost > 0)
            {
                if (Player* plr = m_caster->ToPlayer())
                {
                    if (plr->GetSpecializationId() == SPEC_DK_UNHOLY)
                    {
                        if (plr->HasSpell(207349)) // Dark Arbiter
                        {
                            GuidList* summonList = plr->GetSummonList(100876);
                            if (!summonList->empty())
                            {
                                float countMod = float(m_spellInfo->Power.PowerCost) / 10.f;
                                plr->CastCustomSpell(plr, 211947, &countMod, nullptr, nullptr, true);
                            }
                        }
                        if (Aura* auraInfo = plr->GetAura(51462)) // Runic Corruption
                        {
                            float chance = abs(m_spellInfo->Power.PowerCost * auraInfo->GetSpellInfo()->Effects[EFFECT_1]->BasePoints / 1000.0f);

                            if (roll_chance_f(chance))
                                plr->CastSpell(plr, 51460, true);
                        }
                    }
                }
            }
        }

        if (!GetPowerCost(powerType))
            continue;

        // health as power used
        if (powerType == POWER_HEALTH)
        {
            m_caster->ModifyHealth(-(int32)GetPowerCost(powerType));
            continue;
        }

        if (powerType >= MAX_POWERS)
        {
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::TakePower: Unknown power type '%d'", powerType);
            continue;
        }

        if (hit)
            m_caster->ModifyPower(powerType, -GetPowerCost(powerType), false, m_spellInfo);
        else
            m_caster->ModifyPower(powerType, -ifMissedPowerCost, false, m_spellInfo);
    }
}

void Spell::TakeRunePower()
{
    Player* player = m_caster->ToPlayer();

    if (!player || m_caster->getClass() != CLASS_DEATH_KNIGHT)
        return;

    uint8 _runesCost = m_powerCost[POWER_RUNES];
    m_runesState = player->GetRunesState();                 // store previous state

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "TakeRunePower _runesCost %u", _runesCost);

    for (uint8 i = 0; i < player->GetMaxPower(POWER_RUNES); ++i)
    {
        if (_runesCost && !player->GetRuneCooldown(i))
        {
            uint32 cooldown = uint32(player->GetRuneBaseCooldown());
            player->SetRuneCooldown(i, cooldown);
            _runesCost--;

            TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "TakeRunePower i %u cooldown %u", i, cooldown);
        }
    }
}

void Spell::LinkedSpell(Unit* _caster, Unit* _target, SpellLinkedType type, uint32 effectMask)
{
    if (const std::vector<SpellLinked>* spell_triggered = sSpellMgr->GetSpellLinked(type, m_spellInfo->Id))
    {
        std::set<uint32> spellBanList;
        std::set<uint32> groupLock;
        for (const auto& i : *spell_triggered)
        {
            if (i.hitmask)
                if (!(m_spellMissMask & i.hitmask))
                    continue;

            if (i.effectMask)
                if (!(effectMask & i.effectMask))
                    continue;

            if (!spellBanList.empty())
            {
                auto itr = spellBanList.find(abs(i.effect));
                if (itr != spellBanList.end())
                    continue;
            }

            if (i.group != 0 && !groupLock.empty())
            {
                auto itr = groupLock.find(i.group);
                if (itr != groupLock.end())
                    continue;
            }

            if (!_target)
                _target = m_targets.GetUnitTarget();
            _caster = m_caster;

            if (i.target)
                _target = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(_caster, _target,
                                                                                                  i.target, m_targets.GetUnitTarget());

            if (i.caster)
                _caster = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(_caster, _target,
                                                                                                  i.caster, m_targets.GetUnitTarget());

            if (!_caster)
                continue;

            if (i.targetCount != -1)
            {
                switch (i.targetCountType)
                {
                    case LINK_TARGET_DEFAULT: // 0
                        if (GetTargetCount() < uint32(i.targetCount))
                            continue;
                        break;
                    case LINK_TARGET_FROM_EFFECT: // 1
                        if (GetEffectTargets().size() < uint32(i.targetCount))
                            continue;
                        break;
                    case LINK_TARGET_NULL: // 2
                        if (GetTargetCount() != 0)
                            continue;
                        break;
                    case LINK_TARGET_NOT_NULL: // 3
                        if (GetTargetCount() == 0)
                            continue;
                        break;
                    default:
                        break;
                }
            }

            if (i.hastalent)
                if (m_caster->HasAuraLinkedSpell(m_caster, _target, i.hastype, i.hastalent, i.hasparam))
                    continue;

            if (i.hastalent2)
                if (m_caster->HasAuraLinkedSpell(m_caster, _target, i.hastype2, i.hastalent2, i.hasparam2))
                    continue;

            if (i.effect < 0)
            {
                switch (i.actiontype)
                {
                    case LINK_ACTION_DEFAULT: //0
                        _caster->RemoveAurasDueToSpell(abs(i.effect), i.param ? _caster->GetGUID() : ObjectGuid::Empty);
                        break;
                    case LINK_ACTION_LEARN: //1
                        if (Player* _lplayer = _caster->ToPlayer())
                            _lplayer->removeSpell(abs(i.effect));
                        break;
                    case LINK_ACTION_AURATYPE:
                        if (_target)
                            _target->RemoveAurasByType(AuraType(i.hastalent2));
                        break;
                    case LINK_ACTION_CHANGE_STACK: //7
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(abs(i.effect), _caster->GetGUID()))
                            aura->ModStackAmount(i.param ? -int32(i.param) : -1);
                        break;
                    case LINK_ACTION_CHANGE_CHARGES: //12
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(abs(i.effect), _caster->GetGUID()))
                            aura->ModCharges(i.param ? -int32(i.param) : -1);
                        break;
                    case LINK_ACTION_CAST_DURATION: // 21
                        _caster->CastSpellDuration(_target ? _target : _caster, abs(i.effect), true, i.duration);
                        break;
                    case LINK_ACTION_CAST_NO_TRIGGER: // 4
                        (_target ? _target : _caster)->RemoveAurasDueToSpell(abs(i.effect), _caster->GetGUID());
                        break;
                    default:
                        break;
                }
            }
            else
            {
                if (i.chance != 0)
                {
                    if (i.chance > 100)
                    {
                        if (_caster->IsPlayer())
                        {
                            int32 _chance = _caster->ToPlayer()->GetFloatValue(PLAYER_FIELD_MASTERY) * int32(i.param);
                            if (!roll_chance_i(_chance))
                                continue;
                        }
                        else
                            continue;
                    }
                    else if (!roll_chance_i(i.chance))
                        continue;
                }

                int32 spell_trigger = i.effect;
                if (!i.randList.empty())
                    spell_trigger = Trinity::Containers::SelectRandomContainerElement(i.randList);

                if (i.cooldown != 0 && _caster->HasSpellCooldown(spell_trigger))
                    continue;

                switch (i.actiontype)
                {
                    case LINK_ACTION_DEFAULT: //0
                    {
                        if (i.duration > 0)
                        {
                            _caster->CastSpellDelay(_target ? _target : _caster, spell_trigger, true, i.duration, m_CastItem);
                        }
                        else
                        {
                            _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                        }
                        spellBanList.insert(spell_trigger); // Triggered once for a cycle
                        break;
                    }
                    case LINK_ACTION_LEARN: //1
                        if (Player* _lplayer = _caster->ToPlayer())
                            _lplayer->learnSpell(spell_trigger, false);
                        break;
                    case LINK_ACTION_AURATYPE:
                        _caster->RemoveAurasByType(AuraType(i.hastalent2));
                        break;
                    case LINK_ACTION_SEND_COOLDOWN: // 3
                        _caster->SendSpellCooldown(spell_trigger, m_spellInfo->Id, i.duration);
                        break;
                    case LINK_ACTION_CAST_NO_TRIGGER: //4
                        _caster->CastSpell(_target ? _target : _caster, spell_trigger, false, m_CastItem);
                        break;
                    case LINK_ACTION_ADD_AURA: //5
                        _caster->AddAura(spell_trigger, _target ? _target : _caster);
                        break;
                    case LINK_ACTION_CHANGE_STACK: //7
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(spell_trigger))
                            aura->ModStackAmount(i.param ? int32(i.param) : 1);
                        else if (i.param)
                        {
                            for (uint8 count = 0; count < uint8(i.param); ++count)
                                _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                        }
                        else
                            _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                        break;
                    case LINK_ACTION_REMOVE_COOLDOWN: //8
                        if (Player* _lplayer = _caster->ToPlayer())
                            _lplayer->RemoveSpellCooldown(spell_trigger, true);
                        break;
                    case LINK_ACTION_REMOVE_MOVEMENT: //9
                        if (i.duration)
                            (_target ? _target : _caster)->RemoveAurasWithMechanic(i.duration);
                        else
                            (_target ? _target : _caster)->RemoveMovementImpairingEffects();
                        break;
                    case LINK_ACTION_CHANGE_DURATION: //10
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(spell_trigger, _caster->GetGUID()))
                        {
                            if (!int32(i.param))
                                aura->RefreshTimers();
                            else
                            {
                                int32 _duration = int32(aura->GetDuration() + int32(i.param));
                                if (i.duration)
                                {
                                    if (_duration < i.duration)
                                        aura->SetDuration(_duration);
                                    else
                                        aura->SetDuration(i.duration);
                                }
                                else
                                {
                                    if (aura->GetMaxDuration() < _duration)
                                        aura->SetMaxDuration(_duration);
                                    aura->SetDuration(_duration);
                                }
                            }
                        }
                        else
                            _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                        break;
                    case LINK_ACTION_CAST_DEST: //11
                        if (m_targets.HasDst())
                        {
                            CustomSpellValues values;
                            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_trigger);
                            TriggerCastData triggerData;
                            triggerData.replaced = true;
                            triggerData.originalCaster = m_originalCasterGUID;
                            triggerData.castItem = m_CastItem;
                            triggerData.triggeredByAura = m_triggeredByAura;
                            triggerData.spellGuid = m_castGuid[0];
                            triggerData.SubType = SPELL_CAST_TYPE_NORMAL;
                            triggerData.delay = i.duration;
                            triggerData.powerCost = m_powerCost;

                            _caster->CastSpell(m_targets, spellInfo, &values, triggerData);
                        }
                        break;
                    case LINK_ACTION_CHANGE_CHARGES: //12
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(spell_trigger))
                            aura->ModCharges(i.param ? int32(i.param) : 1);
                        else
                            _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                        break;
                    case LINK_ACTION_MODIFY_COOLDOWN: //13
                        if (_caster->IsPlayer())
                            _caster->ToPlayer()->ModifySpellCooldown(spell_trigger, int32(i.param));
                        break;
                    case LINK_ACTION_CATEGORY_COOLDOWN: //14
                        if (_caster->IsPlayer())
                            _caster->ToPlayer()->ModSpellChargeCooldown(spell_trigger, int32(i.param));
                        break;
                    case LINK_ACTION_CATEGORY_CHARGES: //15
                        if (_caster->IsPlayer())
                            _caster->ToPlayer()->ModSpellCharge(spell_trigger, int32(i.param));
                        break;
                    case LINK_ACTION_RECALCULATE_AMOUNT: //16
                        if(Aura* aura = _caster->GetAura(spell_trigger))
                            aura->RecalculateAmountOfEffects(true);
                        break;
                    case LINK_ACTION_CAST_COUNT: //17
                    {
                        if (i.duration)
                        {
                            for (uint8 count = 0; count < uint8(i.param); ++count)
                            {
                                if (uint32 _delay = i.duration * count)
                                    _caster->m_Events.AddEvent(new DelayCastEvent(*_caster, (_target ? _target : _caster)->GetGUID(), spell_trigger, true), _caster->m_Events.CalculateTime(_delay));
                                else
                                    _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);
                            }
                        }
                        else
                            for (uint8 count = 0; count < uint8(i.param); ++count)
                                _caster->CastSpell(_target ? _target : _caster, spell_trigger, true, m_CastItem);

                        spellBanList.insert(spell_trigger); // Triggered once for a cycle
                        break;
                    }
                    case LINK_ACTION_FULL_TRIGGER: //18
                    {
                        CustomSpellValues values;
                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_trigger);
                        TriggerCastData triggerData;
                        triggerData.triggerFlags = TRIGGERED_FULL_MASK;
                        triggerData.spellGuid = (m_castGuid[1].GetSubType() == SPELL_CAST_TYPE_NORMAL) ? m_castGuid[1] : m_castGuid[0];
                        triggerData.originalCaster = m_originalCasterGUID;
                        triggerData.skipCheck = true;
                        triggerData.casttime = m_casttime;
                        triggerData.castItem = m_CastItem;
                        triggerData.triggeredByAura = m_triggeredByAura;
                        triggerData.parentTargetCount = GetTargetCount();
                        // triggerData.parentTargets = m_UniqueTargetInfo; // Need convert to guid list
                        triggerData.delay = i.duration;
                        triggerData.powerCost = m_powerCost;

                        SpellCastTargets targets;
                        targets.SetCaster(_caster);
                        targets.SetUnitTarget(_target ? _target : _caster);
                        _caster->CastSpell(targets, spellInfo, &values, triggerData);
                        break;
                    }
                    case LINK_ACTION_REMOVE_CATEGORY_CD: // 19
                        if (Player* _lplayer = _caster->ToPlayer())
                            _lplayer->RemoveCategoryCooldownBySpell(spell_trigger, true);
                        break;
                    case LINK_ACTION_SEND_COOLDOWN_SELF: // 20
                        _caster->SendSpellCooldown(m_spellInfo->Id, spell_trigger);
                        break;
                    case LINK_ACTION_CAST_DURATION: // 21
                        _caster->CastSpellDuration(_target ? _target : _caster, spell_trigger, true, i.duration);
                        break;
                    case LINK_ACTION_CAST_ON_SUMMON: // 22
                    {
                        if (GuidList* summonList = _caster->GetSummonList(i.duration))
                            for (GuidList::const_iterator iter = summonList->begin(); iter != summonList->end(); ++iter)
                                if(Creature* summon = ObjectAccessor::GetCreature(*_caster, (*iter)))
                                    _caster->CastSpell(summon, spell_trigger, true, m_CastItem);
                        break;
                    }
                    case LINK_ACTION_MOD_DURATION: // 23
                        if (Aura* aura = (_target ? _target : _caster)->GetAura(spell_trigger, _caster->GetGUID()))
                        {
                            if (!int32(i.param))
                                aura->RefreshTimers();
                            else
                            {
                                int32 _duration = int32(aura->GetDuration() + int32(i.param));
                                if (i.duration)
                                {
                                    if (_duration < i.duration)
                                        aura->SetDuration(_duration);
                                    else
                                        aura->SetDuration(i.duration);
                                }
                                else
                                {
                                    if (aura->GetMaxDuration() < _duration)
                                        aura->SetMaxDuration(_duration);
                                    aura->SetDuration(_duration);
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }

                if (i.group != 0)
                    groupLock.insert(i.group);

                if (i.cooldown != 0)
                    _caster->AddSpellCooldown(spell_trigger, 0, getPreciseTime() + ((double)i.cooldown/1000.0f));
            }
        }
    }
}

void Spell::TakeReagents()
{
    if (!m_caster->IsPlayer())
        return;

    if (m_spellInfo->HasAura(SPELL_AURA_SHOW_CONFIRMATION_PROMPT_WITH_DIFFICULTY)) // Don`t take reagent
        return;

    ItemTemplate const* castItemTemplate = m_CastItem && m_CastItem->IsInWorld() ? m_CastItem->GetTemplate() : nullptr;

    // do not take reagents for these item casts
    if (castItemTemplate && castItemTemplate->GetFlags() & ITEM_FLAG_NO_REAGENT_COST)
        return;

    Player* p_caster = m_caster->ToPlayer();
    if (p_caster->CanNoReagentCast(m_spellInfo))
        return;

    for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
    {
        if (m_spellInfo->Reagents.Reagent[x] <= 0)
            continue;

        uint32 itemid = m_spellInfo->Reagents.Reagent[x];
        uint32 itemcount = m_spellInfo->Reagents.ReagentCount[x];

        // if CastItem is also spell reagent
        if (castItemTemplate && castItemTemplate->GetId() == itemid)
        {
            itemcount += std::count_if(std::begin(castItemTemplate->Effects), std::end(castItemTemplate->Effects), [=](ItemEffectEntry const* entry) -> bool
            {
                return entry->Charges < 0 && abs(m_CastItem->GetSpellCharges(entry->LegacySlotIndex)) < 2;
            });

            m_CastItem->SetInUse(false);
            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;
        }

        // if GetItemTarget is also spell reagent
        if (m_targets.GetItemTargetEntry() == itemid)
            m_targets.SetItemTarget(nullptr);

        p_caster->DestroyItemCount(itemid, itemcount, true);
    }

    if(m_spellInfo->Reagents.CurrencyID != 0)
        p_caster->ModifyCurrency(m_spellInfo->Reagents.CurrencyID, -int32(m_spellInfo->Reagents.CurrencyCount));
}

void Spell::HandleThreatSpells()
{
    if (m_UniqueTargetInfo.empty())
        return;

    if (m_spellInfo->HasAttribute(SPELL_ATTR1_NO_THREAT) ||
        m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO))
        return;

    float threat = 0.0f;
    if (SpellThreatEntry const* threatEntry = sSpellMgr->GetSpellThreatEntry(m_spellInfo->Id))
    {
        if (threatEntry->apPctMod != 0.0f)
            threat += threatEntry->apPctMod * m_caster->GetTotalAttackPowerValue(BASE_ATTACK);

        threat += threatEntry->flatMod;
    }
    else if ((m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_NO_INITIAL_THREAT) == 0)
        threat += m_spellInfo->SpellLevel;

    // past this point only multiplicative effects occur
    if (threat == 0.0f)
        return;

    // since 2.0.1 threat from positive effects also is distributed among all targets, so the overall caused threat is at most the defined bonus
    threat /= m_UniqueTargetInfo.size();

    for (auto& ihit : m_UniqueTargetInfo)
    {
        if (ihit->missCondition != SPELL_MISS_NONE)
            continue;

        Unit* target = ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
        if (!target)
            continue;

        // positive spells distribute threat among all units that are in combat with target, like healing
        if (m_spellInfo->_IsPositiveSpell())
            target->getHostileRefManager().threatAssist(m_caster, threat, m_spellInfo);
        // for negative spells threat gets distributed among affected targets
        else
        {
            if (!target->CanHaveThreatList())
                continue;

            target->AddThreat(m_caster, threat, m_spellInfo->GetSchoolMask(), m_spellInfo);
        }
    }
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell %u, added an additional %f threat for %s %u target(s)", m_spellInfo->Id, threat, m_spellInfo->_IsPositiveSpell() ? "assisting" : "harming", uint32(m_UniqueTargetInfo.size()));
}

void Spell::HandleEffects(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, uint32 i, SpellEffectHandleMode mode)
{
    effectHandleMode = mode;
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;
    destTarget = &m_destTargets[i]._position;

    uint16 eff = m_spellInfo->GetEffect(i, m_diffMode)->Effect;

    if (!damageCalculate[i])
    {
        damage = CalculateDamage(i, unitTarget, &variance);
        saveDamageCalculate[i] = damage;
        damageCalculate[i] = true;
    }
    else
        damage = saveDamageCalculate[i];

    bool preventDefault = CallScriptEffectHandlers((SpellEffIndex)i, mode);
    if (!preventDefault)
        preventDefault = CheckEffFromDummy(unitTarget, i);

    switch (eff)
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_HEALTH_LEECH:
        case SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT:
            if (unitTarget && unitTarget->IsImmunedToDamage(m_spellInfo))
                preventDefault = true;
            break;
        default:
            break;
    }

    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell: %u Effect : %u, damage %i, mode %i idx %i preventDefault %u", m_spellInfo->Id, eff, damage, mode, i, preventDefault);
    #endif

    if (!preventDefault && eff < TOTAL_SPELL_EFFECTS)
    {
        m_currentExecutedEffect = eff;
        (this->*SpellEffects[eff])((SpellEffIndex)i);
        m_currentExecutedEffect = SPELL_EFFECT_NONE;
    }
}

bool Spell::CheckEffFromDummy(Unit* target, uint32 eff)
{
    bool prevent = false;
    if (std::vector<SpellAuraDummy> const* spellAuraDummy = sSpellMgr->GetSpellAuraDummy(m_spellInfo->Id))
    {
        for (const auto& itr : *spellAuraDummy)
        {
            Unit* _caster = m_caster;
            Unit* _targetAura = m_caster;
            bool check = false;

            if(itr.targetaura == 1 && _caster->ToPlayer()) //get target pet
            {
                if (Pet* pet = _caster->ToPlayer()->GetPet())
                    _targetAura = pet->ToUnit();
            }
            if(itr.targetaura == 2) //get target owner
            {
                if (Unit* owner = _caster->GetOwner())
                    _targetAura = owner;
            }
            if(itr.targetaura == 3) //get target
                _targetAura = target;

            if(!_targetAura)
                _targetAura = _caster;

            if (itr.hastalent)
                if (m_caster->HasAuraLinkedSpell(_caster, target, itr.hastype, itr.hastalent, itr.hasparam))
                    continue;

            if (itr.hastalent2)
                if (m_caster->HasAuraLinkedSpell(_caster, target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                    continue;

            switch (itr.option)
            {
                case SPELL_DUMMY_MOD_EFFECT_MASK: //4
                {
                    if(itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                        continue;
                    if(itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                        continue;

                    if(itr.spellDummyId > 0 && !_targetAura->HasAura(itr.spellDummyId))
                    {
                        if(itr.effectmask & (1 << eff))
                            prevent = true;
                        check = true;
                    }
                    if(itr.spellDummyId < 0 && _targetAura->HasAura(abs(itr.spellDummyId)))
                    {
                        if(itr.effectmask & (1 << eff))
                            prevent = true;
                        check = true;
                    }
                    break;
                }
            }
            if(check && itr.removeAura)
                _caster->RemoveAurasDueToSpell(itr.removeAura);
        }
    }

    return prevent;
}

SpellCastResult Spell::CheckCast(bool strict)
{
    SpellCastResult castResult = CallScriptCheckCastHandlers();
    
    if (Player* plr = m_caster->ToPlayer())
    {
        // TODO: Disable items with clone effects on BG - bugged
        if (m_castFlags[1] & CAST_FLAG_EX_USE_TOY_SPELL)
        {
            if (m_spellInfo->Effects[EFFECT_0]->Effect == SPELL_EFFECT_APPLY_AURA && m_spellInfo->Effects[EFFECT_0]->ApplyAuraName == SPELL_AURA_CLONE_CASTER)
                if (plr->InBattleground() || plr->InArena() || plr->InRBG())
                    return SPELL_FAILED_YOU_CANNOT_USE_THAT_IN_PVP_INSTANCE;
        }

        // TODO: Disable some toys in instance - bugged
        if ((m_castFlags[1] & CAST_FLAG_EX_USE_TOY_SPELL) && (m_spellInfo->Effects[EFFECT_0]->Effect == SPELL_EFFECT_TRANS_DOOR || m_spellInfo->Effects[EFFECT_0]->Effect == SPELL_EFFECT_SUMMON))
            if (m_spellInfo->HasAttribute(SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG) || m_spellInfo->HasAttribute(SPELL_ATTR9_NOT_USABLE_IN_ARENA))
                if (plr->GetMap() && plr->GetMap()->IsDungeon())
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

        if (m_spellInfo->HasAttribute(SPELL_ATTR1_CU_USE_PATH_CHECK_FOR_CAST))
        {
            if (SpellDestination const* dest = m_targets.GetDst())
            {
                PathGenerator* m_path = new PathGenerator(m_caster);
                bool result = m_path->CalculatePath(dest->_position.m_positionX, dest->_position.m_positionY, dest->_position.m_positionZ, false, false, true);
                PathType _pathType = m_path->GetPathType();
                delete m_path;

                if (!(_pathType & PATHFIND_NORMAL))
                    return SPELL_FAILED_NOPATH;
            }
        }

        if (m_spellInfo->HasAura(SPELL_AURA_MOUNTED))
        {
            if (MountEntry const* mountEntry = sDB2Manager.GetMount(m_spellInfo->Id))
                if (!ConditionMgr::IsPlayerMeetingCondition(m_caster, mountEntry->PlayerConditionID))
                    return SPELL_FAILED_MOUNT_COLLECTED_ON_OTHER_CHAR;
        }
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR1_CU_CANT_USE_WHEN_ROOTED) && m_caster->HasUnitState(UNIT_STATE_ROOT))
        return SPELL_FAILED_ROOTED;

    if (m_spellInfo->HasAttribute(SPELL_ATTR11_NOT_USABLE_IN_CHALLENGE_MODE) && m_caster->GetMap() && m_caster->GetMap()->isChallenge())
    {
        m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
        return SPELL_FAILED_CUSTOM_ERROR;
    }

    if (castResult != SPELL_CAST_OK)
        return castResult;

    // check death state
    if (!m_caster->isAlive() && !m_spellInfo->IsPassive() && !(m_spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD) || (IsTriggered() && !m_triggeredByAuraSpell)))
        return SPELL_FAILED_CASTER_DEAD;

    if (!strict && m_spellInfo->talentId)
    {
        if (!m_caster->HasSpell(m_spellInfo->Id))
            return SPELL_FAILED_DONT_REPORT;
    }

    if (m_spellInfo->Effects[EFFECT_0]->Effect == SPELL_EFFECT_UNLEARN_TALENT)
        if (Player* plr = m_caster->ToPlayer())
        {
            PlayerTalentMap* Talents = plr->GetTalentMap(plr->GetActiveTalentGroup());
            for (auto& Talent : *Talents)
            {
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(Talent.first);
                if (!spell)
                    continue;

                if (spell->talentId != m_miscData[0])
                    continue;

                if (plr->HasSpellCooldown(spell->Id))
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                break;
            }
        }

    // check cooldowns to prevent cheating
    if (m_caster->IsPlayer() && !m_spellInfo->IsPassive())
    {
        //can cast triggered (by aura only?) spells while have this flag
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURASTATE) && m_caster->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY))
        {
            bool allow = false;
            if (Unit::AuraEffectList const* auras = m_caster->GetAuraEffectsByType(SPELL_AURA_ALLOW_ONLY_ABILITY))
            {
                for (Unit::AuraEffectList::const_iterator itr = auras->begin(); itr != auras->end(); ++itr)
                {
                    if ((*itr)->IsAffectingSpell(m_spellInfo))
                    {
                        allow = true;
                        break;
                    }
                }
            }

            if (!allow)
                return SPELL_FAILED_SPELL_IN_PROGRESS;
        }

        Player* playerCaster = m_caster->ToPlayer();
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD) && m_caster->HasSpellCooldown(m_spellInfo->Id))
        {
            if (m_triggeredByAuraSpell)
                return SPELL_FAILED_DONT_REPORT;
            if(!m_caster->HasAuraTypeWithAffectMask(SPELL_AURA_IGNORE_CD, m_spellInfo))
                return SPELL_FAILED_NOT_READY;
        }

        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD))
            if (!playerCaster->HasChargesForSpell(m_spellInfo))
                return m_triggeredByAuraSpell ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NO_CHARGES_REMAIN;
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR7_IS_CHEAT_SPELL) && !m_caster->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_ALLOW_CHEAT_SPELLS))
    {
        m_customError = SPELL_CUSTOM_ERROR_GM_ONLY;
        return SPELL_FAILED_CUSTOM_ERROR;
    }

    // Check global cooldown
    if (strict && !(_triggeredCastFlags & TRIGGERED_IGNORE_GCD) && HasGlobalCooldown())
        if(!m_caster->HasAuraTypeWithAffectMask(SPELL_AURA_IGNORE_CD, m_spellInfo))
            return SPELL_FAILED_NOT_READY;

    // only triggered spells can be processed an ended battleground
    if (!IsTriggered() && m_caster->IsPlayer())
        if (Battleground* bg = m_caster->ToPlayer()->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_LEAVE)
                return SPELL_FAILED_DONT_REPORT;

    if (m_caster->IsPlayer() && VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled())
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR0_OUTDOORS_ONLY) && !m_caster->GetMap()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if (m_spellInfo->HasAttribute(SPELL_ATTR0_INDOORS_ONLY) && m_caster->GetMap()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_INDOORS;
    }

    if (auto tmpPlayer = m_caster->ToPlayer())
        if (tmpPlayer->IsSpectator())
            return SPELL_FAILED_SPELL_UNAVAILABLE;

    //Dalaran Arena
    if (m_caster->GetMapId() == 617 && m_caster->GetPositionZ() > 12.0f)
    {
        switch (m_spellInfo->Id)
        {
            case 48018:  //Demonic Circle. Teleport
            case 101643: //Transcendence. Teleport
            case 212799: //Displacement Beacon. Teleport
                return SPELL_FAILED_DONT_REPORT;
            default:
                break;
        }
    }

    // only check at first call, Stealth auras are already removed at second call
    // for now, ignore triggered spells
    if (strict && !(_triggeredCastFlags & TRIGGERED_IGNORE_SHAPESHIFT))
    {
        bool checkForm = true;
        // Ignore form req aura
        if (Unit::AuraEffectList const* ignore = m_caster->GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_SHAPESHIFT))
        {
            for (Unit::AuraEffectList::const_iterator i = ignore->begin(); i != ignore->end(); ++i)
            {
                if (!(*i)->IsAffectingSpell(m_spellInfo))
                    continue;
                checkForm = false;
                break;
            }
        }

        if (checkForm)
        {
            // Cannot be used in this stance/form
            SpellCastResult shapeError = m_spellInfo->CheckShapeshift(m_caster->GetShapeshiftForm());
            if (shapeError != SPELL_CAST_OK)
                return shapeError;

            if (m_spellInfo->HasAttribute(SPELL_ATTR0_ONLY_STEALTHED) && !(m_caster->HasStealthAura()))
                return SPELL_FAILED_ONLY_STEALTHED;
        }
    }

    if (Unit::AuraEffectList const* blockSpells = m_caster->GetAuraEffectsByType(SPELL_AURA_BLOCK_SPELL_FAMILY))
        for (Unit::AuraEffectList::const_iterator blockItr = blockSpells->begin(); blockItr != blockSpells->end(); ++blockItr)
            if (uint32((*blockItr)->GetMiscValue()) == m_spellInfo->ClassOptions.SpellClassSet && (*blockItr)->GetSpellInfo()->Effects[(*blockItr)->GetEffIndex()]->TriggerSpell != m_spellInfo->Id)
                return SPELL_FAILED_SPELL_UNAVAILABLE;

    bool reqCombat = true;
    if (Unit::AuraEffectList const* stateAuras = m_caster->GetAuraEffectsByType(SPELL_AURA_ABILITY_IGNORE_AURASTATE))
    {
        for (Unit::AuraEffectList::const_iterator j = stateAuras->begin(); j != stateAuras->end(); ++j)
        {
            if ((*j)->IsAffectingSpell(m_spellInfo))
            {
                if ((*j)->GetMiscValue() == 1)
                {
                    reqCombat=false;
                    break;
                }
            }
        }
    }

    // caster state requirements
    // not for triggered spells (needed by execute)
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURASTATE))
    {
        if (m_spellInfo->AuraRestrictions.CasterAuraState && !m_caster->HasAuraState(AuraStateType(m_spellInfo->AuraRestrictions.CasterAuraState), m_spellInfo, m_caster))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (m_spellInfo->AuraRestrictions.ExcludeCasterAuraState && m_caster->HasAuraState(AuraStateType(m_spellInfo->AuraRestrictions.ExcludeCasterAuraState), m_spellInfo, m_caster))
            return SPELL_FAILED_CASTER_AURASTATE;

        if (m_spellInfo->AuraRestrictions.CasterAuraSpell && !m_caster->HasAura(m_spellInfo->AuraRestrictions.CasterAuraSpell))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (m_spellInfo->AuraRestrictions.ExcludeCasterAuraSpell && m_caster->HasAura(m_spellInfo->AuraRestrictions.ExcludeCasterAuraSpell))
            return SPELL_FAILED_CASTER_AURASTATE;

        if (reqCombat && m_caster->isInCombat() && !m_spellInfo->CanBeUsedInCombat())
            return SPELL_FAILED_AFFECTING_COMBAT;
    }

    // cancel autorepeat spells if cast start when moving
    // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
    // Do not cancel spells which are affected by a SPELL_AURA_CAST_WHILE_WALKING effect
    if (m_caster->IsPlayer() && m_caster->ToPlayer()->isMoving() && !m_caster->HasAuraCastWhileWalking(m_spellInfo))
    {
        // skip stuck spell to allow use it in falling case and apply spell limitations at movement
        if ((!m_caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR) || m_spellInfo->Effects[0]->Effect != SPELL_EFFECT_STUCK) &&
            (IsAutoRepeat() || m_spellInfo->HasAuraInterruptFlag(AURA_INTERRUPT_FLAG_NOT_SEATED)))
            return SPELL_FAILED_MOVING;
    }


    Vehicle* vehicle = m_caster->GetVehicle();
    if (vehicle && !(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE) && m_caster->GetVehicleBase() && m_caster->GetVehicleBase()->CanVehicleAI())
    {
        uint16 checkMask = 0;
        for (uint8 effIndex = EFFECT_0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        {
            if (m_spellInfo->EffectMask < uint32(1 << effIndex))
                break;

            if ((m_spellInfo->EffectMask & (1 << effIndex)) == 0)
                continue;
            SpellEffectInfo const* effInfo = m_spellInfo->GetEffect(effIndex, m_diffMode);
            if (effInfo && effInfo->ApplyAuraName == SPELL_AURA_MOD_SHAPESHIFT)
            {
                SpellShapeshiftFormEntry const* shapeShiftEntry = sSpellShapeshiftFormStore.LookupEntry(effInfo->MiscValue);
                if (shapeShiftEntry && (shapeShiftEntry->Flags & SHAPESHIFT_FORM_IS_NOT_A_SHAPESHIFT) == 0)
                    checkMask |= VEHICLE_SEAT_FLAG_UNCONTROLLED;
                break;
            }
        }

        if (m_spellInfo->HasAura(SPELL_AURA_MOUNTED))
            checkMask |= VEHICLE_SEAT_FLAG_CAN_CAST_MOUNT_SPELL;

        if (!checkMask)
            checkMask = VEHICLE_SEAT_FLAG_CAN_ATTACK;

        VehicleSeatEntry const* vehicleSeat = vehicle->GetSeatForPassenger(m_caster);
        if (!m_spellInfo->HasAttribute(SPELL_ATTR6_CASTABLE_WHILE_ON_VEHICLE) && !m_spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_MOUNTED) && (!vehicleSeat || (vehicleSeat->Flags & checkMask) != checkMask))
            return SPELL_FAILED_DONT_REPORT;
    }


    ConditionSourceInfo condInfo = ConditionSourceInfo(m_caster);
    condInfo.mConditionTargets[1] = m_targets.GetObjectTarget();
    ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_SPELL, m_spellInfo->Id);
    if (!conditions.empty() && !sConditionMgr->IsObjectMeetToConditions(condInfo, conditions))
    {
        // send error msg to player if condition failed and text message available
        // TODO: using WorldSession::SendNotification is not blizzlike
        if (Player* playerCaster = m_caster->ToPlayer())
        {
            // mLastFailedCondition can be NULL if there was an error processing the condition in Condition::Meets (i.e. wrong data for ConditionTarget or others)
            if (playerCaster->GetSession() && condInfo.mLastFailedCondition && condInfo.mLastFailedCondition->ErrorTextId)
            {
                playerCaster->GetSession()->SendNotification(condInfo.mLastFailedCondition->ErrorTextId);
                return SPELL_FAILED_DONT_REPORT;
            }
        }
        if (!condInfo.mLastFailedCondition || !condInfo.mLastFailedCondition->ConditionTarget)
            return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_FAILED_BAD_TARGETS;
    }

    // Don't check explicit target for passive spells (workaround) (check should be skipped only for learn case)
    // those spells may have incorrect target entries or not filled at all (for example 15332)
    // such spells when learned are not targeting anyone using targeting system, they should apply directly to caster instead
    // also, such casts shouldn't be sent to client
    if (!(m_spellInfo->IsPassive() && (!m_targets.GetUnitTarget() || m_targets.GetUnitTarget() == m_caster)))
    {
        bool targetCheck = false;
        // Check explicit target for m_originalCaster - todo: get rid of such workarounds
        if ((m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_CORPSE_MASK) && !m_targets.GetObjectTarget())
        {
            if (Unit* owner = m_caster->GetAnyOwner())
            {
                if (Player* playerCaster = owner->ToPlayer())
                    if (Unit* selectedUnit = ObjectAccessor::GetUnit(*m_caster, playerCaster->GetSelection()))
                    {
                        castResult = m_spellInfo->CheckExplicitTarget(m_originalCaster ? m_originalCaster : m_caster, selectedUnit, m_targets.GetItemTarget());
                        if (castResult != SPELL_CAST_OK)
                            return castResult;
                        m_targets.SetUnitTarget(selectedUnit);
                        targetCheck = true;
                    }
            }
        }
        if(!targetCheck)
        {
            castResult = m_spellInfo->CheckExplicitTarget(m_originalCaster ? m_originalCaster : m_caster, m_targets.GetObjectTarget(), m_targets.GetItemTarget());
            if (castResult != SPELL_CAST_OK)
                return castResult;
        }
    }

    if (Unit* target = m_targets.GetUnitTarget())
    {
        if (m_caster->IsRWVisibility() && m_caster->GetDistance(*target) > m_caster->GetRWVisibility())
            return SPELL_FAILED_LINE_OF_SIGHT;

        castResult = m_spellInfo->CheckTarget(m_caster, target, false);
        if (castResult != SPELL_CAST_OK)
            return castResult;

        if (target != m_caster)
        {
            // Must be behind the target
            if ((m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET) && target->HasInArc(static_cast<float>(M_PI), m_caster))
                return SPELL_FAILED_NOT_BEHIND;

            // Must be behind the target
            if ((m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_REQ_CASTER_NOT_FRONT_TARGET) && target->HasInArc(static_cast<float>(M_PI/2), m_caster))
                return SPELL_FAILED_NOT_BEHIND;

            // Target must be facing you
            if ((m_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_REQ_TARGET_FACING_CASTER) && !target->HasInArc(static_cast<float>(M_PI), m_caster))
                return SPELL_FAILED_NOT_INFRONT;

            if (target->IsPlayer() && target->ToPlayer()->isGameMaster())
                return SPELL_FAILED_DONT_REPORT;

            // Hackfix for Raigonn
            if (m_caster->GetEntry() != WORLD_TRIGGER && target->GetEntry() != 56895) // Ignore LOS for gameobjects casts (wrongly casted by a trigger)
                if ((!canHitTargetInLOS) && !DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, nullptr, SPELL_DISABLE_LOS) && !m_caster->IsWithinLOSInMap(target))
                    return SPELL_FAILED_LINE_OF_SIGHT;

            if (/*m_spellInfo->Categories.DefenseType != SPELL_DAMAGE_CLASS_MELEE &&*/ !IsTriggered() && m_caster->IsVisionObscured(target))
            {
                if (m_caster->IsCreature() && m_caster->GetEntry() == 71529) //fix exploit on Thok Bloodthirsty
                    m_caster->ToCreature()->AI()->EnterEvadeMode();
                return SPELL_FAILED_VISION_OBSCURED; // smoke bomb, camouflage...
            }

            // TOS: The Desolate Host
            if (!m_caster->IsValidDesolateHostTarget(target, m_spellInfo))
                return SPELL_FAILED_VISION_OBSCURED;
        }
    }

    // Check for line of sight for spells with dest
    if (m_targets.HasDst())
    {
        float x, y, z;
        m_targets.GetDstPos()->GetPosition(x, y, z);

        if ((!canHitTargetInLOS) && !DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, nullptr, SPELL_DISABLE_LOS) && !m_caster->IsWithinLOS(x, y, z))
            return SPELL_FAILED_LINE_OF_SIGHT;
    }

    // check pet presence
    for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->EffectMask < uint32(1 << j))
            break;

        if (m_spellInfo->Effects[j]->TargetA.GetTarget() == TARGET_UNIT_PET)
        {
            if (!m_caster->GetGuardianPet() && !m_caster->isPet())
            {
                if (m_triggeredByAuraSpell)              // not report pet not existence for triggered spells
                    return SPELL_FAILED_DONT_REPORT;
                return SPELL_FAILED_NO_PET;
            }
            break;
        }
        
        if (Unit* target = m_targets.GetUnitTarget())
        {
            if (m_spellInfo->Effects[j]->TargetA.GetTarget() == TARGET_UNIT_TARGET_MINIPET)
            {
                if (!target->isMinion())
                    return SPELL_FAILED_NO_PET;

                break;
            }
        }
    }

    // Spell casted only on battleground
    if (m_spellInfo->HasAttribute(SPELL_ATTR3_BATTLEGROUND) && m_caster->IsPlayer())
        if (!m_caster->ToPlayer()->InBattleground())
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    // do not allow spells to be cast in arenas or rated battlegrounds
    if (Player* player = m_caster->ToPlayer())
        if (player->InArena() || player->InRBG())
        {
            castResult = CheckArenaAndRatedBattlegroundCastRules(player);
            if (castResult != SPELL_CAST_OK)
                return castResult;
        }

    // zone check
    if (m_caster->IsCreature() || !m_caster->ToPlayer()->isGameMaster())
    {
        uint32 zone = m_caster->GetCurrentZoneID();
        uint32 area = m_caster->GetCurrentAreaID();

        SpellCastResult locRes = m_spellInfo->CheckLocation(m_caster->GetMapId(), zone, area, m_caster->IsPlayer() ? m_caster->ToPlayer() : nullptr);
        if (locRes != SPELL_CAST_OK)
            return locRes;
    }

    // not let players cast spells at mount (and let do it to creatures)
    if (m_caster->IsMounted() && m_caster->IsPlayer() && !(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE) &&
        !m_spellInfo->IsPassive() && !m_spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_MOUNTED) && !m_caster->HasFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT))
    {
        if (m_caster->isInFlight())
            return SPELL_FAILED_NOT_ON_TAXI;
        return SPELL_FAILED_NOT_MOUNTED;
    }

    // always (except passive spells) check items (focus object can be required for any type casts)
    if (!m_spellInfo->IsPassive())
    {
        castResult = CheckItems();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    // Triggered spells also have range check
    // TODO: determine if there is some flag to enable/disable the check
    castResult = CheckRange(strict);
    if (castResult != SPELL_CAST_OK)
        return castResult;

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
    {
        castResult = CheckPower();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURAS))
    {
        castResult = CheckCasterAuras();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    uint32 dispelMask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_DISPEL)
            dispelMask |= SpellInfo::GetDispelMask(DispelType(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue));
        if (m_spellInfo->GetEffect(i, m_diffMode)->IsEffect() && m_spellInfo->GetEffect(i, m_diffMode)->Effect != SPELL_EFFECT_DISPEL)
        {
            dispelMask = 0;
            break;
        }
    }

    if (dispelMask && !IsTriggered())
    {
        if (Unit* target = m_targets.GetUnitTarget())
        {
            DispelChargesList dispelList;
            target->GetDispellableAuraList(m_caster, dispelMask, dispelList);
            if (dispelList.empty())
                m_dispelResetCD = true;
        }
    }

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if ((m_spellInfo->EffectMask & (1 << i)) == 0)
            continue;
        // for effects of spells that have only one target
        switch (m_spellInfo->GetEffect(i, m_diffMode)->Effect)
        {
            case SPELL_EFFECT_TRIGGER_SPELL:
            {
                switch (m_spellInfo->Id)
                {
                    case 36554:     // Shadowstep
                    {
                        if (Unit * target = m_targets.GetUnitTarget())
                        {
                            if (target == m_caster)
                                return SPELL_FAILED_BAD_TARGETS;

                            if (Vehicle* vehicleTar = target->GetVehicle())
                                if (Unit* base = vehicleTar->GetBase())
                                    if (base->GetEntry() == 27894)
                                        return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                            if (target->GetEntry() == 27894)
                                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case SPELL_EFFECT_APPLY_AURA:
            {
                switch (m_spellInfo->Id)
                {
                    case 5171:  // Slice and Dice
                    {
                        if (Aura * aura = m_caster->GetAura(m_spellInfo->Id))
                        {
                            if (m_caster->ToPlayer())
                            {
                                if (aura->GetDuration() > GetPowerCost(POWER_COMBO_POINTS) * 6000 + 6000)
                                    return SPELL_FAILED_TRY_AGAIN;
                            }
                        }
                        break;
                    }
                    case 143333:    // Water Strider Water Walking
                    {
                        if (Map* map = m_caster->GetMap())
                            if (map->IsBattlegroundOrArena())
                                return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_INCORRECT_AREA;
                    }
                    default:
                        break;
                }
                break;
            }
            case SPELL_EFFECT_DUMMY:
            {
                // Death Coil
                if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_DEATHKNIGHT && m_spellInfo->ClassOptions.SpellClassMask[0] == 0x2000)
                {
                    Unit* target = m_targets.GetUnitTarget();
                    if (!target || (target == m_caster && !m_caster->HasAura(49039)) || (target->IsFriendlyTo(m_caster) && target->GetCreatureType() != CREATURE_TYPE_UNDEAD && !m_caster->HasAura(63333)))
                        return SPELL_FAILED_BAD_TARGETS;
                    if (!target->IsFriendlyTo(m_caster) && !m_caster->HasInArc(static_cast<float>(M_PI), target))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }
                else if (m_spellInfo->Id == 19938)          // Awaken Peon
                {
                    Unit* unit = m_targets.GetUnitTarget();
                    if (!unit || !unit->HasAura(17743))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 31789)          // Righteous Defense
                {
                    if (!m_caster->IsPlayer())
                        return SPELL_FAILED_DONT_REPORT;

                    Unit* target = m_targets.GetUnitTarget();
                    if (!target || !target->IsFriendlyTo(m_caster) || target->getAttackers()->empty())
                        return SPELL_FAILED_BAD_TARGETS;

                }
                
                switch (m_spellInfo->Id)
                {
                    case 86213:  // Soul Swap Exhale
                        if (Unit * target = m_targets.GetUnitTarget())
                            if (m_caster->m_SpecialTarget == target->GetGUID())
                                return SPELL_FAILED_BAD_TARGETS;
                        break;
                    case 51640: // Taunt Flag Targeting
                    {
                        if (Unit* target = m_targets.GetUnitTarget())
                            if (Player* plr = m_caster->ToPlayer())
                            {
                                if (Player* targetplr = target->ToPlayer())
                                {
                                    if (targetplr->isAlive())
                                        return SPELL_FAILED_BAD_TARGETS;

                                    if (targetplr->GetTeam() == HORDE && plr->GetTeam() == HORDE)
                                        return SPELL_FAILED_BAD_TARGETS;

                                    if (targetplr->GetTeam() == ALLIANCE && plr->GetTeam() == ALLIANCE)
                                        return SPELL_FAILED_BAD_TARGETS;
                                }
                                else
                                    return SPELL_FAILED_BAD_TARGETS;
                            }
                    }
                    default:
                        break;
                }
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() != TARGET_UNIT_PET)
                    break;

                Pet* pet = m_caster->ToPlayer()->GetPet();
                if (!pet)
                    return SPELL_FAILED_NO_PET;

                SpellInfo const* learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->GetEffect(i, m_diffMode)->TriggerSpell);

                if (!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if (m_spellInfo->SpellLevel > pet->getLevel())
                    return SPELL_FAILED_LOWLEVEL;

                break;
            }
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                // check target only for unit target case
                if (Unit* unitTarget = m_targets.GetUnitTarget())
                {
                    if (!m_caster->IsPlayer())
                        return SPELL_FAILED_BAD_TARGETS;

                    Pet* pet = unitTarget->ToPet();
                    if (!pet || pet->GetOwner() != m_caster)
                        return SPELL_FAILED_BAD_TARGETS;

                    SpellInfo const* learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->GetEffect(i, m_diffMode)->TriggerSpell);
                    if (!learn_spellproto)
                        return SPELL_FAILED_NOT_KNOWN;

                    if (m_spellInfo->SpellLevel > pet->getLevel())
                        return SPELL_FAILED_LOWLEVEL;
                }
                break;
            }
            case SPELL_EFFECT_APPLY_GLYPH:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_GLYPH_NO_SPEC;

                Player* caster = m_caster->ToPlayer();
                if (!caster->HasSpell(m_miscData[0]))
                    return SPELL_FAILED_NOT_KNOWN;

                if (m_caster->GetMap()->isChallenge())
                {
                    m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                if (uint32 glyphId = m_spellInfo->GetEffect(i, m_diffMode)->MiscValue)
                {
                    GlyphPropertiesEntry const* glyphProperties = sGlyphPropertiesStore.LookupEntry(glyphId);
                    if (!glyphProperties)
                        return SPELL_FAILED_INVALID_GLYPH;

                    std::vector<uint32> const* glyphBindableSpells = sDB2Manager.GetGlyphBindableSpells(glyphId);
                    if (!glyphBindableSpells)
                        return SPELL_FAILED_INVALID_GLYPH;

                    if (std::find(glyphBindableSpells->begin(), glyphBindableSpells->end(), m_miscData[0]) == glyphBindableSpells->end())
                        return SPELL_FAILED_INVALID_GLYPH;

                    if (std::vector<uint32> const* glyphRequiredSpecs = sDB2Manager.GetGlyphRequiredSpecs(glyphId))
                    {
                        if (!caster->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID))
                            return SPELL_FAILED_GLYPH_NO_SPEC;

                        if (std::find(glyphRequiredSpecs->begin(), glyphRequiredSpecs->end(), caster->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID)) == glyphRequiredSpecs->end())
                            return SPELL_FAILED_GLYPH_INVALID_SPEC;
                    }

                    uint32 replacedGlyph = 0;
                    for (uint32 activeGlyphId : caster->GetGlyphs(caster->GetActiveTalentGroup()))
                    {
                        if (std::vector<uint32> const* activeGlyphBindableSpells = sDB2Manager.GetGlyphBindableSpells(activeGlyphId))
                            if (std::find(activeGlyphBindableSpells->begin(), activeGlyphBindableSpells->end(), m_miscData[0]) != activeGlyphBindableSpells->end())
                            {
                                replacedGlyph = activeGlyphId;
                                break;
                            }
                    }

                    for (uint32 activeGlyphId : caster->GetGlyphs(caster->GetActiveTalentGroup()))
                    {
                        if (activeGlyphId == replacedGlyph)
                            continue;

                        if (activeGlyphId == glyphId)
                            return SPELL_FAILED_UNIQUE_GLYPH;

                        if (sGlyphPropertiesStore.AssertEntry(activeGlyphId)->GlyphExclusiveCategoryID == glyphProperties->GlyphExclusiveCategoryID)
                            return SPELL_FAILED_GLYPH_EXCLUSIVE_CATEGORY;
                    }
                }
                break;
            }
            case SPELL_EFFECT_FEED_PET:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                Item* foodItem = m_targets.GetItemTarget();
                if (!foodItem)
                    return SPELL_FAILED_BAD_TARGETS;

                Pet* pet = m_caster->ToPlayer()->GetPet();
                if (!pet)
                    return SPELL_FAILED_NO_PET;

                if (!pet->HaveInDiet(foodItem->GetTemplate()))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                if (!pet->GetCurrentFoodBenefitLevel(foodItem->GetTemplate()->ItemLevel))
                    return SPELL_FAILED_FOOD_LOWLEVEL;

                if (m_caster->isInCombat() || pet->isInCombat())
                    return SPELL_FAILED_AFFECTING_COMBAT;

                break;
            }
            case SPELL_EFFECT_CHARGE:
            {
                if (m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_WARRIOR)
                {
                    // Warbringer - can't be handled in proc system - should be done before checkcast root check and charge effect process
                    if (strict && m_caster->IsScriptOverriden(m_spellInfo, 6953))
                        m_caster->RemoveMovementImpairingEffects();
                }
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;
                if (m_caster->IsPlayer())
                {
                    if (Unit* target = m_targets.GetUnitTarget())
                    {
                        if (!target->isAlive())
                            return SPELL_FAILED_BAD_TARGETS;

                        if (Vehicle* vehicleTar = target->GetVehicle())
                            if (Unit* base = vehicleTar->GetBase())
                                if (base->GetEntry() == 27894)
                                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                        if (target->GetEntry() == 27894)
                            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                        float angle = target->GetRelativeAngle(m_caster);
                        Position pos;
                        target->GetFirstCollisionPosition(pos, target->GetObjectSize(), angle);

                        if (m_caster->IsInWater()) // don`t check in water
                            break;

                        float limit = m_spellInfo->GetMaxRange(true, m_caster, this) + 1.0f;
                        PathGenerator* m_path = new PathGenerator(m_caster);
                        bool result = m_path->CalculatePath(pos.m_positionX, pos.m_positionY, pos.m_positionZ, false);
                        PathType _pathType = m_path->GetPathType();
                        float _totalLength = m_path->GetTotalLength();
                        delete m_path;

                        if (_pathType & PATHFIND_SHORT && _triggeredCastFlags != TRIGGERED_FULL_MASK)
                            return SPELL_FAILED_OUT_OF_RANGE;
                        if (!result || _pathType & PATHFIND_NOPATH)
                            return SPELL_FAILED_NOPATH;
                        if (_totalLength > (limit * 1.5f))
                            return SPELL_FAILED_OUT_OF_RANGE;
                    }
                }
                break;
            }
            case SPELL_EFFECT_JUMP_DEST:
            {
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;

                if (m_caster->IsPlayer())
                {
                    if (Unit* target = m_targets.GetUnitTarget())
                    {
                        if (Vehicle* vehicleTar = target->GetVehicle())
                            if (Unit* base = vehicleTar->GetBase())
                                if (base->GetEntry() == 27894)
                                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                        if (target->GetEntry() == 27894)
                            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                    }
                }
                break;
            }
            case SPELL_EFFECT_SKINNING:
            {
                if (!m_caster->IsPlayer() || !m_targets.GetUnitTarget() || !m_targets.GetUnitTarget()->IsCreature())
                    return SPELL_FAILED_BAD_TARGETS;

                if (!(m_targets.GetUnitTarget()->GetUInt32Value(UNIT_FIELD_FLAGS) & UNIT_FLAG_SKINNABLE))
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                Creature* creature = m_targets.GetUnitTarget()->ToCreature();
                if (creature->GetCreatureType() != CREATURE_TYPE_CRITTER && !creature->loot.isLooted())
                    return SPELL_FAILED_TARGET_NOT_LOOTED;

                break;
            }
            case SPELL_EFFECT_OPEN_LOCK:
            {
                if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() != TARGET_GAMEOBJECT_TARGET &&
                    m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() != TARGET_GAMEOBJECT_ITEM_TARGET)
                    break;

                if (!m_caster->IsPlayer() || (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_GAMEOBJECT_TARGET && !m_targets.GetGOTarget()))
                    return SPELL_FAILED_BAD_TARGETS;

                Item* pTempItem = nullptr;
                if (m_targets.GetTargetMask() & TARGET_FLAG_TRADE_ITEM)
                {
                    if (TradeData* pTrade = m_caster->ToPlayer()->GetTradeData())
                        pTempItem = pTrade->GetTraderData()->GetItem(TradeSlots(m_targets.GetItemTargetGUID().GetRawValue()[0]));  // at this point item target guid contains the trade slot
                }
                else if (m_targets.GetTargetMask() & TARGET_FLAG_ITEM)
                    pTempItem = m_caster->ToPlayer()->GetItemByGuid(m_targets.GetItemTargetGUID());

                // we need a go target, or an openable item target in case of TARGET_GAMEOBJECT_ITEM_TARGET
                if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_GAMEOBJECT_ITEM_TARGET && !m_targets.GetGOTarget() && (!pTempItem || !pTempItem->GetTemplate()->GetLockID() || !pTempItem->IsLocked()))
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_spellInfo->Id != 1842 || (m_targets.GetGOTarget() && m_targets.GetGOTarget()->GetGOInfo()->type != GAMEOBJECT_TYPE_TRAP))
                    if (m_caster->ToPlayer()->InBattleground() && !m_caster->ToPlayer()->CanUseBattlegroundObject())
                        return SPELL_FAILED_TRY_AGAIN;

                if (m_targets.GetGOTarget())
                {
                    auto goInfo = m_targets.GetGOTarget()->GetGOInfo();
                    if (goInfo->type == GAMEOBJECT_TYPE_CHEST || goInfo->type == GAMEOBJECT_TYPE_GATHERING_NODE)
                    {
                        if (!sConditionMgr->IsPlayerMeetingCondition(m_caster, goInfo->type == GAMEOBJECT_TYPE_CHEST ? goInfo->chest.conditionID1 : goInfo->gatheringNode.conditionID1))
                            return SPELL_FAILED_NO_VALID_TARGETS;
                        if (m_caster->ToPlayer()->IsFlying() && m_caster->GetMapId() != 616)
                            return SPELL_FAILED_NOT_FLYING;
                        if (m_caster->IsMounted() && m_caster->NeedDismount())
                            m_caster->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    }
                }
                // get the lock entry
                uint32 lockId = 0;
                if (GameObject* go = m_targets.GetGOTarget())
                {
                    lockId = go->GetGOInfo()->GetLockId();
                    if (!lockId)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (Item* itm = m_targets.GetItemTarget())
                    lockId = itm->GetTemplate()->GetLockID();

                SkillType skillId = SKILL_NONE;
                int32 reqSkillValue = 0;
                int32 skillValue = 0;

                // check lock compatibility
                SpellCastResult res = CanOpenLock(i, lockId, skillId, reqSkillValue, skillValue);
                if (res != SPELL_CAST_OK)
                    return res;

                // chance for fail at orange mining/herb/LockPicking gathering attempt
                // second check prevent fail at rechecks
                if (skillId != SKILL_NONE && (!m_selfContainer || ((*m_selfContainer) != this)))
                {
                    bool canFailAtMax = skillId != SKILL_HERBALISM && skillId != SKILL_MINING;

                    // chance for failure in orange gather / lockpick (gathering skill can't fail at maxskill)
                    if ((canFailAtMax || skillValue < sWorld->GetConfigMaxSkillValue()) && reqSkillValue > irand(skillValue - 25, skillValue + 37))
                        return SPELL_FAILED_TRY_AGAIN;
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                Creature* pet = m_caster->GetGuardianPet();
                if (pet && pet->isAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                break;
            }
            // This is generic summon effect
            case SPELL_EFFECT_SUMMON:
            {
                SummonPropertiesEntry const* SummonProperties = sSummonPropertiesStore.LookupEntry(m_spellInfo->GetEffect(i, m_diffMode)->MiscValueB);
                if (!SummonProperties)
                    break;
                switch (SummonProperties->Control)
                {
                    case SUMMON_CATEGORY_PET:
                        if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET) && !m_caster->GetPetGUID().IsEmpty() && 
							m_spellInfo->Id != 118291 && m_spellInfo->Id != 118323 && m_spellInfo->Id != 157319) // Don't show error for shaman controlled elementals 
                            return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                    case SUMMON_CATEGORY_PUPPET:
                        if (m_caster->GetCharmGUID())
                            return SPELL_FAILED_ALREADY_HAVE_CHARM;
                        break;
                }
                break;
            }
            case SPELL_EFFECT_CREATE_TAMED_PET:
            {
                if (m_targets.GetUnitTarget())
                {
                    if (!m_targets.GetUnitTarget()->IsPlayer())
                        return SPELL_FAILED_BAD_TARGETS;
                    if (m_targets.GetUnitTarget()->GetPetGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_PET:
            {
                if (m_caster->GetPetGUID())                  //let warlock do a replacement summon
                {
                    if (m_caster->IsPlayer() && m_caster->getClass() == CLASS_WARLOCK)
                    {
                        if (strict)                         //starting cast, trigger pet stun (cast by pet so it doesn't attack player)
                            if (Pet* pet = m_caster->ToPlayer()->GetPet())
                            {
                                pet->CastSpell(pet, 32752, true, nullptr, nullptr, pet->GetGUID());
                            }
                    }
                    else if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET))
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                }

                if (!m_caster->GetCharmGUID().IsEmpty())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;
                break;
            }
            case SPELL_EFFECT_SUMMON_PLAYER:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                if (!m_caster->ToPlayer()->GetSelection())
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_caster->GetMap()->isChallenge())
                {
                    m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                Player* target = ObjectAccessor::FindPlayer(m_caster->ToPlayer()->GetSelection());
                if (!target || m_caster->ToPlayer() == target || (!target->IsInSameRaidWith(m_caster->ToPlayer()) && m_spellInfo->Id != 48955)) // refer-a-friend spell
                    return SPELL_FAILED_BAD_TARGETS;

                // check if our map is dungeon
                MapEntry const* map = sMapStore.LookupEntry(m_caster->GetMapId());
                if (map->IsDungeon())
                {
                    uint32 mapId = m_caster->GetMap()->GetId();
                    Difficulty difficulty = m_caster->GetMap()->GetDifficultyID();
                    if (map->IsRaid())
                    {
                        if (m_caster->GetMap()->GetEntry()->ExpansionID < EXPANSION_LEGION)
                        {
                            if (InstancePlayerBind* targetBind = target->GetBoundInstance(mapId, difficulty))
                                if (InstancePlayerBind* casterBind = m_caster->ToPlayer()->GetBoundInstance(mapId, difficulty))
                                {
                                    uint32 allMask = casterBind->save->GetCompletedEncounterMask() & targetBind->save->GetCompletedEncounterMask();
                                    if (allMask != targetBind->save->GetCompletedEncounterMask())
                                        return SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE;
                                }
                        }
                    }
                    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(mapId);
                    if (!instance)
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;
                    if (!target->Satisfy(sObjectMgr->GetAccessRequirement(mapId, difficulty), mapId))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            // RETURN HERE
            case SPELL_EFFECT_SUMMON_RAF_FRIEND:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_caster->GetMap()->isChallenge())
                {
                    m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                Player* playerCaster = m_caster->ToPlayer();
                if (!(playerCaster->GetSelection()))
                    return SPELL_FAILED_BAD_TARGETS;

                Player* target = ObjectAccessor::FindPlayer(playerCaster->GetSelection());
                if (!target || !(target->GetSession()->GetRecruiterId() == playerCaster->GetSession()->GetAccountId() || target->GetSession()->GetAccountId() == playerCaster->GetSession()->GetRecruiterId()))
                    return SPELL_FAILED_BAD_TARGETS;

                break;
            }
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
              //Do not allow to cast it before BG starts.
                if (m_caster->IsPlayer())
                    if (Battleground const* bg = m_caster->ToPlayer()->GetBattleground())
                        if (bg->GetStatus() != STATUS_IN_PROGRESS)
                            return SPELL_FAILED_TRY_AGAIN;
                break;
            }
            case SPELL_EFFECT_STEAL_BENEFICIAL_BUFF:
                if (m_targets.GetUnitTarget() == m_caster)
                    return SPELL_FAILED_BAD_TARGETS;
                break;
            case SPELL_EFFECT_LEAP_BACK:
                if (m_caster->HasUnitState(UNIT_STATE_ROOT) && !m_caster->HasAura(109215))
                {
                    if (m_caster->IsPlayer())
                        return SPELL_FAILED_ROOTED;
                    return SPELL_FAILED_DONT_REPORT;
                }
                break;
            case SPELL_EFFECT_UNLEARN_TALENT:

                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_BAD_TARGETS;

                if (m_caster->GetMap()->isChallenge())
                {
                    m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
                    return SPELL_FAILED_CUSTOM_ERROR;
                }

                if (TalentEntry const* talent = sTalentStore.LookupEntry(m_miscData[0]))
                {
                    if (m_caster->HasSpellCooldown(talent->SpellID))
                        return SPELL_FAILED_CANT_UNTALENT;
                }
                else
                    return SPELL_FAILED_DONT_REPORT;
                //no break
            case SPELL_EFFECT_TALENT_SPEC_SELECT:
                // can't change during already started arena/battleground
                if (Player* player = m_caster->ToPlayer())
                    if (Battleground* bg = player->GetBattleground())
                        if (bg->GetStatus() == STATUS_IN_PROGRESS)
                            return SPELL_FAILED_NOT_IN_BATTLEGROUND;

                if (m_caster->GetMap()->isChallenge())
                {
                    m_customError = SPELL_CUSTOM_ERROR_CANT_DO_THAT_WHILE_MYTHIC_KEYSTONE_IS_ACTIVE;
                    return SPELL_FAILED_CUSTOM_ERROR;
                }
                break;
            case SPELL_EFFECT_UNLOCK_GUILD_VAULT_TAB:
            {
                Player* player = m_caster->ToPlayer();
                if (!player)
                    return SPELL_FAILED_DONT_REPORT;

                Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId());
                
                if (!guild)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                if (guild->GetPurchasedTabsSize() >= GetSpellInfo()->Effects[0]->BasePoints)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                if (guild->GetPurchasedTabsSize() < GetSpellInfo()->Effects[0]->BasePoints - 1)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                break;
            }
            case SPELL_EFFECT_SCRIPT_EFFECT:
            {
                Player* player = m_caster->ToPlayer();
                if (!player)
                    break;

                switch (m_spellInfo->Id)
                {
                    case 143626:                                // Celestial Cloth and Its Uses
                        if (player->HasSpell(143011))
                            return SPELL_FAILED_SPELL_LEARNED;
                        break;
                    case 143644:                                // Hardened Magnificent Hide and Its Uses
                        if (player->HasSpell(142976))
                            return SPELL_FAILED_SPELL_LEARNED;
                        break;
                    case 143646:                                // Balanced Trillium Ingot and Its Uses
                        if (player->HasSpell(143255))
                            return SPELL_FAILED_SPELL_LEARNED;
                        break;
                    case 146428:                                // Timeless Essence of the Black Dragonflight
                    {
                        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BACK);
                        if (!item)
                            return SPELL_FAILED_EQUIPPED_ITEM;

                        switch (item->GetEntry())
                        {
                            case 98149:
                            case 98147:
                            case 98146:
                            case 98335:
                            case 98148:
                            case 98150:
                                break;
                            default:
                                return SPELL_FAILED_EQUIPPED_ITEM;
                        }
                    }
                }
                break;
            }
            case SPELL_EFFECT_SUMMON_RAID_MARKER:
            {
                Player* player = m_caster->ToPlayer();
                if (!player)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;

                Group* group = player->GetGroup();
                if (!group)
                    return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
                break;
            }
            case SPELL_EFFECT_RESURRECT:
                if (m_spellInfo->IsBattleResurrection())
                {
                    if (InstanceScript* instanceScript = m_caster->GetInstanceScript())
                        if (!instanceScript->CanUseCombatResurrection())
                            return SPELL_FAILED_IN_COMBAT_RES_LIMIT_REACHED;

                    if (Unit* uTarget = m_targets.GetUnitTarget())
                        if (Player* target = uTarget->ToPlayer())
                            if (target->IsRessurectRequested())
                                return SPELL_FAILED_TARGET_HAS_RESURRECT_PENDING;
                }
                break;
            case SPELL_EFFECT_GIVE_ARTIFACT_POWER:
            case SPELL_EFFECT_GIVE_ARTIFACT_POWER_NO_BONUS:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_TARGET_NOT_PLAYER;

                Item* artifact = m_caster->ToPlayer()->GetArtifactWeapon();
                if (!artifact)
                    return SPELL_FAILED_NO_ARTIFACT_EQUIPPED;

                ArtifactEntry const* artifactEntry = sArtifactStore.LookupEntry(artifact->GetTemplate()->GetArtifactID());
                if (!artifactEntry || artifactEntry->ArtifactCategoryID != m_spellInfo->GetEffect(i, m_diffMode)->MiscValue)
                    return SPELL_FAILED_WRONG_ARTIFACT_EQUIPPED;
                break;
            }
            case SPELL_EFFECT_DUEL:
            {
                // strong server-side checks
                Unit* target = m_targets.GetUnitTarget();

                if (!target)
                    return SPELL_FAILED_NO_VALID_TARGETS;

                if (!m_caster->IsPlayer() || !target->IsPlayer())
                    return SPELL_FAILED_TARGET_NOT_PLAYER;

                if (m_caster->isInCombat())
                    return SPELL_FAILED_AFFECTING_COMBAT;

                if (target->isInCombat())
                    return SPELL_FAILED_TARGET_AFFECTING_COMBAT;

                // Players can only fight a duel in zones with this flag
                AreaTableEntry const* casterAreaEntry = sAreaTableStore.LookupEntry(m_caster->GetCurrentAreaID());
                if (casterAreaEntry && !(casterAreaEntry->Flags[0] & AREA_FLAG_ALLOW_DUELS))
                    return SPELL_FAILED_NO_DUELING;

                AreaTableEntry const* targetAreaEntry = sAreaTableStore.LookupEntry(target->GetCurrentAreaID());
                if (targetAreaEntry && !(targetAreaEntry->Flags[0] & AREA_FLAG_ALLOW_DUELS))
                    return SPELL_FAILED_NO_DUELING;            // Dueling isn't allowed here

                break;
            }
            default:
                break;
        }
    }

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        auto const& auraName = m_spellInfo->GetEffect(i, m_diffMode)->ApplyAuraName;

        switch (auraName)
        {
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_AOE_CHARM:
            {
                if (m_caster->GetCharmerGUID())
                    return SPELL_FAILED_CHARMED;

                if (auraName == SPELL_AURA_MOD_CHARM || auraName == SPELL_AURA_MOD_POSSESS)
                {
                    if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET) && !m_caster->GetPetGUID().IsEmpty())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if (m_caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }

                if (Unit* target = m_targets.GetUnitTarget())
                {
                    if (target->IsCreature() && target->ToCreature()->IsVehicle())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    // TODO: disable in instance and raids on elite mobs - bugged, broke MovementGenerators and make abuses
                    if (target->IsCreature() && target->GetMap()->IsDungeon())
                    {
                        if (CreatureTemplate const* crTemplate = target->ToCreature()->GetCreatureTemplate())
                            if (crTemplate->Classification != CREATURE_CLASSIFICATION_NORMAL && crTemplate->Classification != CREATURE_CLASSIFICATION_TRIVIAL)
                                return SPELL_FAILED_CANT_BE_CHARMED;
                    }

                    if (target->IsMounted() && !target->HasFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT))
                        return SPELL_FAILED_CANT_BE_CHARMED;

                    if (target->GetCharmerGUID())
                        return SPELL_FAILED_CHARMED;

                    int32 damage = CalculateDamage(i, target);
                    if (damage && int32(target->getLevelForTarget(m_caster)) > damage)
                        return SPELL_FAILED_HIGHLEVEL;
                }

                break;
            }
            case SPELL_AURA_MOUNTED:
            {
                //if (m_caster->IsInWater())
                    //return SPELL_FAILED_ONLY_ABOVEWATER;

                // Ignore map check if spell have AreaId. AreaId already checked and this prevent special mount spells
                bool allowMount = !m_caster->GetMap()->IsDungeon() || m_caster->GetMap()->IsBattlegroundOrArena();
                InstanceTemplate const* it = sObjectMgr->GetInstanceTemplate(m_caster->GetMapId());
                if (it)
                    allowMount = it->AllowMount;
                if (m_caster->HasFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT))
                    allowMount = true;

                if ((m_caster->IsPlayer() && !allowMount && !m_spellInfo->CastingReq.RequiredAreasID) || (m_caster->GetMapId() == 530 && m_caster->ToPlayer()->GetCurrentZoneID() == 0))
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                if (m_caster->IsInDisallowedMountForm())
                    return SPELL_FAILED_NOT_SHAPESHIFT;

                break;
            }
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            {
                if (!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // can be casted at non-friendly unit or own pet/charm
                if (m_caster->IsFriendlyTo(m_targets.GetUnitTarget()))
                    return SPELL_FAILED_TARGET_FRIENDLY;

                break;
            }
            case SPELL_AURA_FLY:
            case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
            {
                // not allow cast fly spells if not have req. skills  (all spells is self target)
                // allow always ghost flight spells
                if (m_originalCaster && m_originalCaster->IsPlayer() && m_originalCaster->isAlive())
                {
                    Battlefield* Bf = sBattlefieldMgr->GetBattlefieldToZoneId(m_originalCaster->GetCurrentZoneID());
                    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(m_originalCaster->GetCurrentAreaID()))
                        if (area->Flags[0] & AREA_FLAG_NO_FLY_ZONE  || (Bf && !Bf->CanFlyIn()))
                            return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_NOT_HERE;
                }
                break;
            }
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            {
                if (m_spellInfo->GetEffect(i, m_diffMode)->IsTargetingArea())
                    break;

                if (!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (!m_caster->IsPlayer() || m_CastItem)
                    break;

                if (m_targets.GetUnitTarget()->getPowerType() != POWER_MANA)
                    return SPELL_FAILED_BAD_TARGETS;

                break;
            }
            case SPELL_AURA_CONTROL_VEHICLE:
            {
                if (!m_caster->IsPlayer())
                    break;

                if (Unit* target = m_targets.GetUnitTarget())
                    if (target->IsMounted() && !m_caster->IsWithinLOSInMap(target))
                        return SPELL_FAILED_LINE_OF_SIGHT;
            }
            default:
                break;
        }
    }

    // check trade slot case (last, for allow catch any another cast problems)
    if (m_targets.GetTargetMask() & TARGET_FLAG_TRADE_ITEM)
    {
        if (m_CastItem)
            return SPELL_FAILED_ITEM_ENCHANT_TRADE_WINDOW;

        if (!m_caster->IsPlayer())
            return SPELL_FAILED_NOT_TRADING;

        TradeData* my_trade = m_caster->ToPlayer()->GetTradeData();

        if (!my_trade)
            return SPELL_FAILED_NOT_TRADING;

        TradeSlots slot = TradeSlots(m_targets.GetItemTargetGUID().GetRawValue()[0]);
        if (slot != TRADE_SLOT_NONTRADED)
            return SPELL_FAILED_BAD_TARGETS;

        if (!IsTriggered())
            if (my_trade->GetSpell())
                return SPELL_FAILED_ITEM_ALREADY_ENCHANTED;
    }

    if (m_spellInfo->HasAttribute(SPELL_ATTR7_CONSOLIDATED_RAID_BUFF))
    {
        if (m_originalTarget)
            if (!m_originalTarget->IsPlayer())
                if (m_originalTarget->GetOwner() && m_originalTarget->GetOwner()->IsPlayer())
                    return SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED;
    }

    if (auto creature = m_caster->ToCreature())
        if (!creature->IsDungeonBoss() && creature->HasSchoolMaskCooldown(m_spellInfo->GetSchoolMask()))
            return SPELL_FAILED_NOT_READY;


    // all ok
    return SPELL_CAST_OK;
}

SpellCastResult Spell::CheckPetCast(Unit* target)
{
    if (m_caster->HasUnitState(UNIT_STATE_CASTING) && !(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS))              //prevent spellcast interruption by another spellcast
        return SPELL_FAILED_SPELL_IN_PROGRESS;

    // dead owner (pets still alive when owners ressed?)
    if (Unit* owner = m_caster->GetCharmerOrOwner())
    {
        if (!owner->isAlive())
            return SPELL_FAILED_CASTER_DEAD;

        if (Player* player = owner->ToPlayer())
            if (player->InArena() || player->InRBG())
            {
                SpellCastResult castResult = CheckArenaAndRatedBattlegroundCastRules(player);
                if (castResult != SPELL_CAST_OK)
                    return castResult;
            }
    }

    if (!target && m_targets.GetUnitTarget())
        target = m_targets.GetUnitTarget();

    if (m_spellInfo->NeedsExplicitUnitTarget())
    {
        if (!target)
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
        m_targets.SetUnitTarget(target);
    }

    // cooldown
    if (Creature const* creatureCaster = m_caster->ToCreature())
        if (creatureCaster->HasCreatureSpellCooldown(m_spellInfo->Id))
            if(!m_caster->HasAuraTypeWithAffectMask(SPELL_AURA_IGNORE_CD, m_spellInfo))
                return SPELL_FAILED_NOT_READY;

    return CheckCast(true);
}

SpellCastResult Spell::CheckCasterAuras() const
{
    // spells totally immuned to caster auras (wsg flag drop, give marks etc)
    if (m_spellInfo->HasAttribute(SPELL_ATTR6_IGNORE_CASTER_AURAS))
        return SPELL_CAST_OK;

    uint8 school_immune = 0;
    uint32 mechanic_immune = 0;
    uint32 dispel_immune = 0;

    // Check if the spell grants school or mechanic immunity.
    // We use bitmasks so the loop is done only once and not on every aura check below.
    if (m_spellInfo->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            auto const& auraName = m_spellInfo->GetEffect(i, m_diffMode)->ApplyAuraName;
            switch (auraName)
            {
            case SPELL_AURA_SCHOOL_IMMUNITY:
                school_immune |= uint32(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                break;
            case SPELL_AURA_MECHANIC_IMMUNITY:
                mechanic_immune |= 1 << uint32(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                break;
            case SPELL_AURA_DISPEL_IMMUNITY:
                dispel_immune |= SpellInfo::GetDispelMask(DispelType(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue));
                break;
            case SPELL_AURA_MECHANIC_IMMUNITY_MASK:
                mechanic_immune |= m_spellInfo->GetMechanicMask(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                break;
            default:
                break;
            }
        }
        // immune movement impairment and loss of control
        if (m_spellInfo->Id == 42292 || m_spellInfo->Id == 59752 || m_spellInfo->Id == 195710 || m_spellInfo->Id == 208683)
            mechanic_immune = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;
    }

    bool usableInStun = m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_STUNNED);

    // Check whether the cast should be prevented by any state you might have.
    SpellCastResult prevented_reason = SPELL_CAST_OK;
    // Have to check if there is a stun aura. Otherwise will have problems with ghost aura apply while logging out
    uint32 unitflag = m_caster->GetUInt32Value(UNIT_FIELD_FLAGS);     // Get unit state
    if (unitflag & UNIT_FLAG_STUNNED)
    {
        // spell is usable while stunned, check if caster has only mechanic stun auras, another stun types must prevent cast spell
        if (usableInStun)
        {
            static uint32 constexpr allowedStunMask = 1 << MECHANIC_STUN | 1 << MECHANIC_FREEZE | 1 << MECHANIC_SAPPED | 1 << MECHANIC_SLEEP;

            bool foundNotStun = false;
            if (Unit::AuraEffectList const* stunAuras = m_caster->GetAuraEffectsByType(SPELL_AURA_MOD_STUN))
            {
                for (Unit::AuraEffectList::const_iterator i = stunAuras->begin(); i != stunAuras->end(); ++i)
                {
                    uint32 mechanicMask = (*i)->GetSpellInfo()->GetAllEffectsMechanicMask();
                    if (mechanicMask && !(mechanicMask & allowedStunMask))
                    {
                        foundNotStun = true;
                        break;
                    }
                }
            }
            if (foundNotStun)
                prevented_reason = SPELL_FAILED_STUNNED;
        }
        else
            prevented_reason = SPELL_FAILED_STUNNED;
    }
    else if (unitflag & UNIT_FLAG_CONFUSED && !m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_CONFUSED))
        prevented_reason = SPELL_FAILED_CONFUSED;
    else if (unitflag & UNIT_FLAG_FLEEING && !m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_FEARED))
        prevented_reason = SPELL_FAILED_FLEEING;
    else if (unitflag & UNIT_FLAG_SILENCED && m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE && !m_spellInfo->HasAttribute(SPELL_ATTR8_USABLE_WHILE_SILENCED))
        prevented_reason = SPELL_FAILED_SILENCED;
    else if (unitflag & UNIT_FLAG_PACIFIED && m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_PACIFY)
        prevented_reason = SPELL_FAILED_PACIFIED;
    else if (m_caster->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_NO_ACTIONS) && m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_NO_ACTIONS)
        prevented_reason = SPELL_FAILED_NO_ACTIONS;

    if (unitflag & UNIT_FLAG_PACIFIED && m_spellInfo->Effects[EFFECT_0]->Effect == SPELL_EFFECT_OPEN_LOCK)
        return SPELL_FAILED_PACIFIED;

    // Attr must make flag drop spell totally immune from all effects
    if (prevented_reason != SPELL_CAST_OK)
    {
        if (school_immune || mechanic_immune || dispel_immune)
        {
            //Checking auras is needed now, because you are prevented by some state but the spell grants immunity.
            Unit::AuraApplicationMap const& auras = m_caster->GetAppliedAuras();
            for (const auto& itr : auras)
            {
                Aura const* aura = itr.second->GetBase();
                SpellInfo const* auraInfo = aura->GetSpellInfo();
                if (auraInfo->GetAllEffectsMechanicMask() & mechanic_immune)
                    continue;
                if (auraInfo->GetSchoolMask() & school_immune && !(auraInfo->HasAttribute(SPELL_ATTR1_UNAFFECTED_BY_SCHOOL_IMMUNE)))
                    continue;
                if (auraInfo->GetDispelMask() & dispel_immune)
                    continue;

                //Make a second check for spell failed so the right SPELL_FAILED message is returned.
                //That is needed when your casting is prevented by multiple states and you are only immune to some of them.
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (auraInfo->EffectMask < uint32(1 << i))
                        break;

                    if (AuraEffect* part = aura->GetEffect(i))
                    {
                        switch (part->GetAuraType())
                        {
                            case SPELL_AURA_MOD_STUN:
                                if (!usableInStun || !(auraInfo->GetAllEffectsMechanicMask() & (1<<MECHANIC_STUN)))
                                    return SPELL_FAILED_STUNNED;
                                break;
                            case SPELL_AURA_MOD_CONFUSE:
                                if (!m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_CONFUSED))
                                    return SPELL_FAILED_CONFUSED;
                                break;
                            case SPELL_AURA_MOD_FEAR:
                            case SPELL_AURA_MOD_FEAR_2:
                                if (!m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_FEARED))
                                    return SPELL_FAILED_FLEEING;
                                break;
                            case SPELL_AURA_MOD_SILENCE:
                            case SPELL_AURA_MOD_PACIFY:
                            case SPELL_AURA_MOD_PACIFY_SILENCE:
                            {
                                if (m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_PACIFY)
                                    return SPELL_FAILED_PACIFIED;
                                if (m_spellInfo->Categories.PreventionType & SPELL_PREVENTION_TYPE_SILENCE && !m_spellInfo->HasAttribute(SPELL_ATTR8_USABLE_WHILE_SILENCED))
                                    return SPELL_FAILED_SILENCED;
                                break;
                            }
                            default:
                                break;
                        }
                    }
                }
            }
        }
        // You are prevented from casting and the spell casted does not grant immunity. Return a failed error.
        else
            return prevented_reason;
    }
    return SPELL_CAST_OK;
}

SpellCastResult Spell::CheckArenaAndRatedBattlegroundCastRules(Player* player)
{
    if (!player)
        return SPELL_FAILED_NOT_IN_ARENA;

    bool isRatedBattleground = player->InRBG();
    bool isArena = player->InArena();

    if (isRatedBattleground && m_spellInfo->HasAttribute(SPELL_ATTR9_USABLE_IN_RATED_BATTLEGROUNDS))
        return SPELL_CAST_OK;

    if (isArena && m_spellInfo->HasAttribute(SPELL_ATTR4_USABLE_IN_ARENA))
        return SPELL_CAST_OK;

    if (m_spellInfo->HasAttribute(SPELL_ATTR4_NOT_USABLE_IN_ARENA_OR_RATED_BG))
        return isArena ? SPELL_FAILED_NOT_IN_ARENA : SPELL_FAILED_NOT_IN_RATED_BATTLEGROUND;

    if (isArena && m_spellInfo->HasAttribute(SPELL_ATTR9_NOT_USABLE_IN_ARENA))
        return SPELL_FAILED_NOT_IN_ARENA;

    return SPELL_CAST_OK;
}

float Spell::CalculateDamage(uint8 i, Unit const* target, float* var) const
{
    return m_caster->CalculateSpellDamage(target, m_spellInfo, i, &m_spellValue->EffectBasePoints[i], m_CastItem, m_spellValue->LockBasePoints[i], var);
}

bool Spell::CanAutoCast(Unit* target)
{
    if (!m_spellInfo->CanAutoCast(m_caster, target))
        return false;

    ObjectGuid targetguid = target->GetGUID();

    SpellCastResult result = CheckPetCast(target);
    if (result == SPELL_CAST_OK || result == SPELL_FAILED_UNIT_NOT_INFRONT)
    {
        SelectSpellTargets();

        //check if among target units, our WANTED target is as well (->only self cast spells return false)
        for (auto& ihit : m_UniqueTargetInfo)
            if (ihit->targetGUID == targetguid)
                return true;

        if (m_targets.HasDst())
            return true;
    }

    // If don't have target in spell
    if(m_spellInfo->IsNoTargets())
        return true;

    return false;                                           //target invalid
}

void Spell::CheckSrc()
{
    if (!m_targets.HasSrc())
        m_targets.SetSrc(*m_caster);
}

void Spell::CheckDst()
{
    if (!m_targets.HasDst())
        m_targets.SetDst(*m_caster);
}

SpellCastResult Spell::CheckRange(bool strict)
{
    // Don't check for instant cast spells
    if (!strict && m_casttime == 0)
        return SPELL_CAST_OK;

    uint32 range_type = 0;
    if (m_spellInfo->GetMisc(m_diffMode)->RangeEntry)
    {
        // check needed by 68766 51693 - both spells are cast on enemies and have 0 max range
        // these are triggered by other spells - possibly we should omit range check in that case?
        if (m_spellInfo->GetMisc(m_diffMode)->RangeEntry->ID == 1)
            return SPELL_CAST_OK;

        range_type = m_spellInfo->GetMisc(m_diffMode)->RangeEntry->Flags;
    }

    Unit* target = m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_originalTarget;
    float max_range = m_caster->GetSpellMaxRangeForTarget(target, m_spellInfo);
    float min_range = m_caster->GetSpellMinRangeForTarget(target, m_spellInfo);

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, max_range, this);

    if (target && target != m_caster && !m_spellInfo->DontCheckDistance())
    {
        if (range_type == SPELL_RANGE_MELEE)
        {
            if (_triggeredCastFlags != TRIGGERED_FULL_MASK)
            {
                // Because of lag, we can not check too strictly here.
                if (!m_caster->IsWithinMeleeRange(target, max_range)) //Fix range hack
                    return !(_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_OUT_OF_RANGE : SPELL_FAILED_DONT_REPORT;
            }
        }
        else if (!m_caster->IsWithinCombatRange(target, max_range))
            return !(_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_OUT_OF_RANGE : SPELL_FAILED_DONT_REPORT; //0x5A;

        if (range_type == SPELL_RANGE_RANGED)
        {
            if (m_caster->IsWithinMeleeRange(target))
                return !(_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_TOO_CLOSE : SPELL_FAILED_DONT_REPORT;
        }
        else if (min_range && m_caster->IsWithinCombatRange(target, min_range)) // skip this check if min_range = 0
            return !(_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_TOO_CLOSE : SPELL_FAILED_DONT_REPORT;

        if (_triggeredCastFlags != TRIGGERED_FULL_MASK)
        {
            if (m_caster->IsPlayer() && (m_spellInfo->CastingReq.FacingCasterFlags & SPELL_FACING_FLAG_INFRONT) && !m_caster->HasInArc(static_cast<float>(M_PI), target))
                return !(_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_UNIT_NOT_INFRONT : SPELL_FAILED_DONT_REPORT;
        }
    }

    if (!target && m_targets.HasDst() && !m_targets.HasTraj())
    {
        if (!m_caster->IsWithinDist3d(m_targets.GetDstPos(), max_range))
            return SPELL_FAILED_OUT_OF_RANGE;
        if (min_range && m_caster->IsWithinDist3d(m_targets.GetDstPos(), min_range))
            return SPELL_FAILED_TOO_CLOSE;
    }
    
    return SPELL_CAST_OK;
}

SpellCastResult Spell::CheckPower()
{
    // item cast not used power
    if (m_CastItem || m_spellInfo->NoPower())
        return SPELL_CAST_OK;

    for (uint8 i = 0; i < MAX_POWERS_FOR_SPELL; ++i)
    {
        if (!GetSpellInfo()->IsPowerActive(i))
            continue;

        SpellPowerEntry const* power = GetSpellInfo()->GetPowerInfo(i);
        if (!power)
            continue;

        // health as power used - need check health amount
        if (power->PowerType == POWER_HEALTH)
        {
            if (int32(m_caster->GetHealth()) <= GetPowerCost(power->PowerType))
                return SPELL_FAILED_CASTER_AURASTATE;
            return SPELL_CAST_OK;
        }
        // Check valid power type
        if (power->PowerType >= MAX_POWERS)
        {
            TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "Spell::CheckPower: Unknown power type '%d'", power->PowerType);
            return SPELL_FAILED_NO_POWER;
        }
    }

    if (m_spellInfo->GetSpellPowerByCasterPower(m_caster, m_powerData))
    {
        for (SpellPowerEntry const* power : m_powerData)
        {
            // Check power amount
            Powers powerType = Powers(power->PowerType);
            if (int32(m_caster->GetPower(powerType)) < GetPowerCost(power->PowerType))
            {
                m_failedArg[0] = powerType;
                return SPELL_FAILED_NO_POWER;
            }

            if (powerType == POWER_HOLY_POWER)
            {
                m_powerCost[power->PowerType] = m_caster->HandleHolyPowerCost(m_powerCost[power->PowerType], power);
                if(m_powerCost[power->PowerType])
                    if (Player* modOwner = m_caster->GetSpellModOwner())
                        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, m_powerCost[power->PowerType]);
            }
        }
    }
    else
    {
        for (uint32 i = 0; i < MAX_POWERS; ++i)
            if (m_powerCost[i])
            {
                m_failedArg[0] = i;
                return SPELL_FAILED_NO_POWER;
            }
    }

    return SPELL_CAST_OK;
}

SpellCastResult Spell::CheckItems()
{
    if (!m_caster->IsPlayer())
        return SPELL_CAST_OK;

    if (m_spellInfo->HasAttribute(SPELL_ATTR2_IGNORE_ITEM_CHECK))
        return SPELL_CAST_OK;

    auto caster = m_caster->ToPlayer();

    if (!m_CastItem)
    {
        if (m_castItemGUID)
            return SPELL_FAILED_ITEM_NOT_READY;
    }
    else
    {
        uint32 itemid = m_CastItem->GetEntry();
        if (!caster->HasItemCount(itemid))
            return SPELL_FAILED_ITEM_NOT_READY;

        ItemTemplate const* proto = m_CastItem->GetTemplate();
        if (!proto)
            return SPELL_FAILED_ITEM_NOT_READY;

        for (ItemEffectEntry const* effectData : proto->Effects)
            if (effectData->Charges > 0)
                if (m_CastItem->GetSpellCharges(effectData->LegacySlotIndex) == 0)
                    return SPELL_FAILED_NO_CHARGES_REMAIN;

        // consumable cast item checks
        if (proto->GetClass() == ITEM_CLASS_CONSUMABLE && m_targets.GetUnitTarget())
        {
            // such items should only fail if there is no suitable effect at all - see Rejuvenation Potions for example
            SpellCastResult failReason = SPELL_CAST_OK;
            for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                // skip check, pet not required like checks, and for TARGET_UNIT_PET m_targets.GetUnitTarget() is not the real target but the caster
                if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_PET)
                    continue;

                if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_HEAL)
                {
                    if (m_targets.GetUnitTarget()->IsFullHealth())
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_HEALTH;
                        continue;
                    }
                    failReason = SPELL_CAST_OK;
                    break;
                }

                // Mana Potion, Rage Potion, Thistle Tea(Rogue), ...
                if (m_spellInfo->GetEffect(i, m_diffMode)->Effect == SPELL_EFFECT_ENERGIZE)
                {
                    if (m_spellInfo->GetEffect(i, m_diffMode)->MiscValue < 0 || m_spellInfo->GetEffect(i, m_diffMode)->MiscValue >= int8(MAX_POWERS))
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        continue;
                    }

                    auto power = Powers(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                    if (m_targets.GetUnitTarget()->GetPower(power) == m_targets.GetUnitTarget()->GetMaxPower(power))
                    {
                        failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                        continue;
                    }
                    failReason = SPELL_CAST_OK;
                    break;
                }
            }
            if (failReason != SPELL_CAST_OK)
                return failReason;
        }

        //Check recipe rank
        if (proto->GetFlags() & ITEM_FLAG_HIDE_UNUSABLE_RECIPE)
        {
            if (proto->Effects.size() > 1 && !proto->Effects.empty())
                for (auto effect : proto->Effects)
                    if (uint32 prevSpellID = sSpellMgr->GetPrevSpellInChain(effect->SpellID))
                        if (prevSpellID && !m_caster->HasSpell(prevSpellID))
                            return SPELL_FAILED_MUST_KNOW_SUPERCEDING_SPELL;
        }
    }

    // check target item
    if (m_targets.GetItemTargetGUID())
    {
        if (!m_caster->IsPlayer())
            return SPELL_FAILED_BAD_TARGETS;

        if (!m_targets.GetItemTarget())
            return SPELL_FAILED_ITEM_GONE;

        if (!m_targets.GetItemTarget()->IsFitToSpellRequirements(m_spellInfo))
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }
    // if not item target then required item must be equipped
    else
    {
        if (m_caster->IsPlayer() && !m_caster->ToPlayer()->HasItemFitToSpellRequirements(m_spellInfo))
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }

    // check spell focus object
    if (m_spellInfo->CastingReq.RequiresSpellFocus)
    {
        m_failedArg[0] = m_spellInfo->CastingReq.RequiresSpellFocus;
        CellCoord p(Trinity::ComputeCellCoord(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell(p);

        GameObject* ok = nullptr;
        Trinity::GameObjectFocusCheck go_check(m_caster, m_spellInfo->CastingReq.RequiresSpellFocus);
        Trinity::GameObjectSearcher<Trinity::GameObjectFocusCheck> checker(m_caster, ok, go_check);

        Map& map = *m_caster->GetMap();
        cell.Visit(p, Trinity::makeGridVisitor(checker), map, *m_caster, m_caster->GetVisibilityRange());

        if (!ok)
        {
            if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
            {
                if (!m_caster->IsWithinDist3d(st->target_X, st->target_Y, st->target_Z, 20.0f) || m_caster->GetMapId() != st->target_mapId)
                    return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
            }
            else
                return SPELL_FAILED_REQUIRES_SPELL_FOCUS;
        }

        focusObject = ok;                                   // game object found in range
    }

    // do not take reagents for these item casts
    if (!(m_CastItem && m_CastItem->GetTemplate()->GetFlags() & ITEM_FLAG_NO_REAGENT_COST))
    {
        bool checkReagents = !(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST) && !caster->CanNoReagentCast(m_spellInfo);
        // Not own traded item (in trader trade slot) requires reagents even if triggered spell
        if (!checkReagents)
            if (Item* targetItem = m_targets.GetItemTarget())
                if (targetItem->GetOwnerGUID() != m_caster->GetGUID())
                    checkReagents = true;

        // check reagents (ignore triggered spells with reagents processed by original spell) and special reagent ignore case.
        if (checkReagents)
        {
            for (uint32 i = 0; i < MAX_SPELL_REAGENTS; i++)
            {
                if (m_spellInfo->Reagents.Reagent[i] <= 0)
                    continue;

                uint32 itemid = m_spellInfo->Reagents.Reagent[i];
                uint32 itemcount = m_spellInfo->Reagents.ReagentCount[i];
                m_failedArg[0] = itemid;
                m_failedArg[1] = itemcount;

                // if CastItem is also spell reagent
                if (m_CastItem && m_CastItem->GetEntry() == itemid)
                {
                    ItemTemplate const* proto = m_CastItem->GetTemplate();
                    if (!proto)
                        return SPELL_FAILED_ITEM_NOT_READY;

                    for (auto const& v : proto->Effects)
                    {
                        // CastItem will be used up and does not count as reagent
                        int32 charges = m_CastItem->GetSpellCharges(v->LegacySlotIndex);
                        if (v->Charges < 0 && abs(charges) < 2)
                        {
                            ++itemcount;
                            break;
                        }
                    }
                }
                if (!caster->HasItemCount(itemid, itemcount))
                    return SPELL_FAILED_REAGENTS;
            }
            if (m_spellInfo->Reagents.CurrencyID != 0)
            {
                uint32 currencyId = m_spellInfo->Reagents.CurrencyID;
                uint32 currencyCount = m_spellInfo->Reagents.CurrencyCount;
                m_failedArg[0] = currencyId;
                m_failedArg[1] = currencyCount;

                if (!caster->HasCurrency(currencyId, currencyCount))
                    return SPELL_FAILED_REAGENTS;
            }
        }

        // check totem-item requirements (items presence in inventory)
        uint32 totems = 2;
        for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
        {
            m_failedArg[i] = m_spellInfo->Totems.Totem[i];
            if (m_spellInfo->Totems.Totem[i] != 0)
            {
                if (caster->HasItemCount(m_spellInfo->Totems.Totem[i]))
                    totems -= 1;
            }
            else
                totems -= 1;
        }
        if (totems != 0)
            return SPELL_FAILED_TOTEMS;
    }

    // special checks for spell effects
    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        switch (m_spellInfo->GetEffect(i, m_diffMode)->Effect)
        {
            case SPELL_EFFECT_CREATE_ITEM:
            case SPELL_EFFECT_CREATE_ITEM_2:
            {
                if (!IsTriggered() && m_spellInfo->GetEffect(i, m_diffMode)->ItemType)
                {
                    ItemPosCountVec dest;
                    InventoryResult msg = caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->GetEffect(i, m_diffMode)->ItemType, 1);
                    if (msg != EQUIP_ERR_OK)
                    {
                        ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(m_spellInfo->GetEffect(i, m_diffMode)->ItemType);
                        // TODO: Needs review
                        if (pProto && !(pProto->GetLimitCategory()))
                        {
                            caster->SendEquipError(msg, nullptr, nullptr, m_spellInfo->GetEffect(i, m_diffMode)->ItemType);
                            return SPELL_FAILED_DONT_REPORT;
                        }
                        if (!(m_spellInfo->ClassOptions.SpellClassSet == SPELLFAMILY_MAGE && (m_spellInfo->ClassOptions.SpellClassMask[0] & 0x40000000)))
                            return SPELL_FAILED_TOO_MANY_OF_ITEM;
                        if (!(caster->HasItemCount(m_spellInfo->GetEffect(i, m_diffMode)->ItemType)))
                            return SPELL_FAILED_TOO_MANY_OF_ITEM;
                        // Conjure Mana Gem
                        if (m_spellInfo->Id == 759)
                            caster->CastSpell(m_caster, 54408, false);
                        // Conjure Mana Gem
                        else if (m_spellInfo->Id == 119316)
                            caster->CastSpell(m_caster, 119318, false);
                        // Conjure Healthstone
                        else if (m_spellInfo->Id == 23517)
                            caster->CastSpell(m_caster, 120038, false);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
                break;
            }
            case SPELL_EFFECT_CREATE_ITEM_3:
            {
                if (ItemTemplate const* itemProto = m_targets.GetItemTarget()->GetTemplate())
                {
                    if (caster->CanUseItem(itemProto) != EQUIP_ERR_OK)
                        return SPELL_FAILED_BAD_TARGETS;
                    if (!(itemProto->GetFlags() & ITEM_FLAG_USES_RESOURCES) || itemProto->ItemLevel != 496 || itemProto->GetId() == 104347) //Timeless Curio cannot be target
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_OBLITERATE_ITEM:
            {
                if (ItemTemplate const* itemProto = m_targets.GetItemTarget()->GetTemplate())
                {
                    if (!(itemProto->GetFlags3() & ITEM_FLAG3_OBLITERATABLE))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else
                    return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
                if (m_spellInfo->GetEffect(i, m_diffMode)->ItemType && m_targets.GetItemTarget() && (m_targets.GetItemTarget()->IsVellum()))
                {
                    // cannot enchant vellum for other player
                    if (m_targets.GetItemTarget()->GetOwner() != m_caster)
                        return SPELL_FAILED_NOT_TRADEABLE;
                    // do not allow to enchant vellum from scroll made by vellum-prevent exploit
                    if (m_CastItem && m_CastItem->GetTemplate()->GetFlags() & ITEM_FLAG_NO_REAGENT_COST)
                        return SPELL_FAILED_TOTEM_CATEGORY;
                    ItemPosCountVec dest;
                    InventoryResult msg = caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->GetEffect(i, m_diffMode)->ItemType, 1);
                    if (msg != EQUIP_ERR_OK)
                    {
                        caster->SendEquipError(msg, nullptr, nullptr, m_spellInfo->GetEffect(i, m_diffMode)->ItemType);
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
            case SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC:
            {
                Item* targetItem = m_targets.GetItemTarget();
                if (!targetItem)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                if (targetItem->GetTemplate()->ItemLevel < m_spellInfo->BaseLevel)
                    return SPELL_FAILED_LOWLEVEL;

                bool isItemUsable = false;

                ItemTemplate const* proto = targetItem->GetTemplate();
                for (auto const& v : proto->Effects)
                {
                    if (v->SpellID && (v->TriggerType == ITEM_SPELLTRIGGER_ON_USE || v->TriggerType == ITEM_SPELLTRIGGER_ON_NO_DELAY_USE))
                    {
                        isItemUsable = true;
                        break;
                    }
                }

                SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                // do not allow adding usable enchantments to items that have use effect already
                if (pEnchant && isItemUsable)
                    for (unsigned char s : pEnchant->Effect)
                        if (s == ITEM_ENCHANTMENT_TYPE_USE_SPELL)
                            return SPELL_FAILED_ON_USE_ENCHANT;

                // Not allow enchant in trade slot for some enchant type
                if (targetItem->GetOwner() != m_caster)
                {
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;

                    if (pEnchant->Flags & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            {
                Item* item = m_targets.GetItemTarget();
                if (!item)
                    return SPELL_FAILED_ITEM_NOT_FOUND;
                // Not allow enchant in trade slot for some enchant type
                if (item->GetOwner() != m_caster)
                {
                    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(m_spellInfo->GetEffect(i, m_diffMode)->MiscValue);
                    if (!pEnchant)
                        return SPELL_FAILED_ERROR;

                    if (pEnchant->Flags & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_DISENCHANT:
            {
                Item* item = m_targets.GetItemTarget();
                if (!item)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                Player* player = m_caster->ToPlayer();
                if (!player)
                    return SPELL_FAILED_ERROR;

                // prevent disenchanting in trade slot
                if (item->GetOwnerGUID() != m_caster->GetGUID())
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                ItemTemplate const* itemProto = item->GetTemplate();
                if (!itemProto)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                if (ItemDisenchantLootEntry const* itemDisenchantLoot = item->GetDisenchantLoot(player))
                {
                    if (itemDisenchantLoot->SkillRequired > player->GetSkillValue(SKILL_ENCHANTING))
                        return SPELL_FAILED_LOW_CASTLEVEL;
                }
                else
                {
                    // special disenchant
                    if (!(itemProto->GetFlags2() & ITEM_FLAG2_DISENCHANT_TO_LOOT_TABLE))
                        return SPELL_FAILED_CANT_BE_DISENCHANTED;
                }
                break;
            }
            case SPELL_EFFECT_PROSPECTING:
            {
                if (!m_targets.GetItemTarget())
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //ensure item is a prospectable ore
                if (!(m_targets.GetItemTarget()->GetTemplate()->GetFlags() & ITEM_FLAG_IS_PROSPECTABLE))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //prevent prospecting in trade slot
                if (m_targets.GetItemTarget()->GetOwnerGUID() != m_caster->GetGUID())
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //Check for enough skill in jewelcrafting
                uint32 item_prospectingskilllevel = m_targets.GetItemTarget()->GetTemplate()->GetRequiredSkillRank();
                if (item_prospectingskilllevel > caster->GetSkillValue(SKILL_JEWELCRAFTING))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                //make sure the player has the required ores in inventory
                if (m_targets.GetItemTarget()->GetCount() < 5)
                    return SPELL_FAILED_NEED_MORE_ITEMS;

                if (!LootTemplates_Prospecting.HaveLootFor(m_targets.GetItemTargetEntry()))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;

                break;
            }
            case SPELL_EFFECT_MILLING:
            {
                if (!m_targets.GetItemTarget())
                    return SPELL_FAILED_CANT_BE_MILLED;
                //ensure item is a millable herb
                if (!(m_targets.GetItemTarget()->GetTemplate()->GetFlags() & ITEM_FLAG_IS_MILLABLE))
                    return SPELL_FAILED_CANT_BE_MILLED;
                //prevent milling in trade slot
                if (m_targets.GetItemTarget()->GetOwnerGUID() != m_caster->GetGUID())
                    return SPELL_FAILED_CANT_BE_MILLED;
                //Check for enough skill in inscription
                uint32 item_millingskilllevel = m_targets.GetItemTarget()->GetTemplate()->GetRequiredSkillRank();
                if (item_millingskilllevel > caster->GetSkillValue(SKILL_INSCRIPTION))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                //make sure the player has the required herbs in inventory
                if (m_targets.GetItemTarget()->GetCount() < 5)
                    return SPELL_FAILED_NEED_MORE_ITEMS;

                if (!LootTemplates_Milling.HaveLootFor(m_targets.GetItemTargetEntry()))
                    return SPELL_FAILED_CANT_BE_MILLED;

                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if (!m_caster->IsPlayer())
                    return SPELL_FAILED_TARGET_NOT_PLAYER;

                if (m_attackType != RANGED_ATTACK)
                    break;

                Item* pItem = m_caster->ToPlayer()->GetWeaponForAttack(m_attackType);
                if (!pItem || pItem->CantBeUse())
                    return SPELL_FAILED_EQUIPPED_ITEM;

                switch (pItem->GetTemplate()->GetSubClass())
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    {
                        uint32 ammo = pItem->GetEntry();
                        if (!m_caster->ToPlayer()->HasItemCount(ammo))
                            return SPELL_FAILED_NO_AMMO;
                        break;
                    }
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    case ITEM_SUBCLASS_WEAPON_WAND:
                        break;
                    default:
                        break;
                }
                break;
            }
            case SPELL_EFFECT_CREATE_MANA_GEM:
            {
                uint32 item_id = m_spellInfo->GetEffect(i, m_diffMode)->ItemType;
                ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(item_id);

                if (!pProto)
                    return SPELL_FAILED_ITEM_AT_MAX_CHARGES;

                if (Item* pitem = caster->GetItemByEntry(item_id))
                {
                    for (auto const& v : pProto->Effects)
                        if (v->Charges != 0 && pitem->GetSpellCharges(v->LegacySlotIndex) == v->Charges)
                            return SPELL_FAILED_ITEM_AT_MAX_CHARGES;
                }
                break;
            }
            case SPELL_EFFECT_CHANGE_ITEM_BONUSES:
            {
                Item* item = m_targets.GetItemTarget();
                if (!item)
                    return SPELL_FAILED_NO_VALID_TARGETS;

                if (!item->IsSoulBound())
                    return SPELL_FAILED_NOT_SOULBOUND;

                if (m_spellInfo->Id == 183484 || m_spellInfo->Id == 249954) // Obliterum
                {
                    uint32 minItemLevel = m_spellInfo->GetEffect(EFFECT_3, m_diffMode)->BasePoints;
                    uint32 maxItemLevel = m_spellInfo->GetEffect(EFFECT_1, m_diffMode)->BasePoints;
                    uint32 itemLevel = item->GetItemLevel(m_caster->getLevel());
                    if (minItemLevel < 100)
                        minItemLevel = 885; // Check for 249954

                    if (sWorld->getBoolConfig(CONFIG_OBLITERUM_LEVEL_ENABLE))
                    {
                        minItemLevel = sWorld->getIntConfig(CONFIG_OBLITERUM_LEVEL_MIN);
                        maxItemLevel = sWorld->getIntConfig(CONFIG_OBLITERUM_LEVEL_MAX);
                    }

                    if (itemLevel < minItemLevel || itemLevel >= maxItemLevel)
                        return SPELL_FAILED_NO_VALID_TARGETS;
                }

                uint32 OldItemBonusTree = m_spellInfo->GetEffect(i, m_diffMode)->MiscValue;
                uint32 NewItemBonusTree = m_spellInfo->GetEffect(i, m_diffMode)->MiscValueB;

                if (OldItemBonusTree == NewItemBonusTree)
                    break;

                std::set<ItemBonusTreeNodeEntry const*> const* OldBonusTree = sDB2Manager.GetItemBonusSet(OldItemBonusTree);
                std::set<ItemBonusTreeNodeEntry const*> const* NewBonusTre = sDB2Manager.GetItemBonusSet(NewItemBonusTree);

                if (OldBonusTree == nullptr || NewBonusTre == nullptr)
                    return SPELL_FAILED_NO_VALID_TARGETS;

                std::vector<uint32> bonuses = item->GetDynamicValues(ITEM_DYNAMIC_FIELD_BONUS_LIST_IDS);

                bool _found = false;
                for (auto const bonus : bonuses)
                {
                    for (auto const oldBonus : *OldBonusTree)
                    {
                        if (bonus == oldBonus->ChildItemBonusListID)
                        {
                            _found = true;
                            break;
                        }
                    }
                }

                if (!_found)
                    return SPELL_FAILED_NO_VALID_TARGETS;
            }
            default:
                break;
        }
    }

    // check weapon presence in slots for main/offhand weapons
    if (m_spellInfo->EquippedItemClass >= 0)
    {
        // main hand weapon required
        if (m_spellInfo->HasAttribute(SPELL_ATTR3_MAIN_HAND))
        {
            Item* item = m_caster->ToPlayer()->GetWeaponForAttack(BASE_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->CantBeUse())
                return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(m_spellInfo))
                return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }

        // offhand hand weapon required
        if (m_spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND))
        {
            Item* item = m_caster->ToPlayer()->GetWeaponForAttack(OFF_ATTACK);

            // skip spell if no weapon in slot or broken
            if (!item || item->CantBeUse())
                return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(m_spellInfo))
                return (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) ? SPELL_FAILED_DONT_REPORT : SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }
    }

    return SPELL_CAST_OK;
}

void Spell::Delayed() // only called in DealDamage()
{
    if (!m_caster)// || !m_caster->IsPlayer())
        return;

    if (isDelayableNoMore())                                 // Spells may only be delayed twice
        return;

    //check pushback reduce
    int32 delaytime = 200;                                  // spellcasting delay is normally 500ms
    int32 delayReduce = 100;                                // must be initialized to 100 for percent modifiers
    m_caster->ToPlayer()->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, delayReduce, this);
    delayReduce += m_caster->GetTotalAuraModifier(SPELL_AURA_REDUCE_PUSHBACK) - 100;
    if (delayReduce >= 100)
        return;

    AddPct(delaytime, -delayReduce);

    if (m_timer + delaytime > m_casttime)
    {
        delaytime = m_casttime - m_timer;
        m_timer = m_casttime;
    }
    else
        m_timer += delaytime;

    TC_LOG_INFO(LOG_FILTER_SPELLS_AURAS, "Spell %u partially interrupted for (%d) ms at damage", m_spellInfo->Id, delaytime);

    WorldPackets::Spells::SpellDelayed delayed;
    delayed.Caster = m_caster->GetGUID();
    delayed.ActualDelay = delaytime;
    m_caster->SendMessageToSet(delayed.Write(), true);
}

void Spell::DelayedChannel()
{
    if (!m_caster || !m_caster->IsPlayer() || getState() != SPELL_STATE_CASTING)
        return;

    if (isDelayableNoMore())                                    // Spells may only be delayed twice
        return;

    //check pushback reduce
    int32 delaytime = CalculatePct(m_spellInfo->GetDuration(m_diffMode), 25); // channeling delay is normally 25% of its time per hit
    int32 delayReduce = 100;                                    // must be initialized to 100 for percent modifiers
    m_caster->ToPlayer()->ApplySpellMod(m_spellInfo->Id, SPELLMOD_NOT_LOSE_CASTING_TIME, delayReduce, this);
    delayReduce += m_caster->GetTotalAuraModifier(SPELL_AURA_REDUCE_PUSHBACK) - 100;
    if (delayReduce >= 100)
        return;

    AddPct(delaytime, -delayReduce);

    if (m_timer <= delaytime)
    {
        delaytime = m_timer;
        m_timer = 0;
    }
    else
        m_timer -= delaytime;

    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell %u partially interrupted for %i ms, new duration: %u ms", m_spellInfo->Id, delaytime, m_timer);

    for (std::vector<TargetInfoPtr>::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
        if ((*ihit)->missCondition == SPELL_MISS_NONE)
            if (Unit* unit = (m_caster->GetGUID() == (*ihit)->targetGUID) ? m_caster : ObjectAccessor::GetUnit(*m_caster, (*ihit)->targetGUID))
                unit->DelayOwnedAuras(m_spellInfo->Id, m_originalCasterGUID, delaytime);

    // partially interrupt persistent area auras
    if (DynamicObject* dynObj = m_caster->GetDynObject(m_spellInfo->Id))
        dynObj->Delay(delaytime);

    SendChannelUpdate(m_timer);
}

int32 Spell::GetPowerCost(int8 power)
{
    if (power > MAX_POWERS || power < 0)
        return m_powerCost[MAX_POWERS];
    return m_powerCost[power];
}

int8 Spell::GetComboPoints()
{
    return m_powerCost[POWER_COMBO_POINTS];
}

void Spell::UpdatePointers()
{
    if (m_originalCasterGUID == m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
    {
        m_originalCaster = ObjectAccessor::GetUnit(*m_caster, m_originalCasterGUID);
        if (m_originalCaster && !m_originalCaster->IsInWorld())
            m_originalCaster = nullptr;
    }

    if (!m_castItemGUID.IsEmpty() && m_caster->IsPlayer())
    {
        m_CastItem = m_caster->ToPlayer()->GetItemByGuid(m_castItemGUID);
        if (!m_CastItem)
            return;

        if (m_castItemEntry != m_CastItem->GetEntry())
            return;
    }

    m_targets.Update(m_caster);

    // further actions done only for dest targets
    if (!m_targets.HasDst())
        return;

    // cache last transport
    WorldObject* transport = nullptr;

    // update effect destinations (in case of moved transport dest target)
    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (m_spellInfo->EffectMask < uint32(1 << effIndex))
            break;

        SpellDestination& dest = m_destTargets[effIndex];
        if (!dest._transportGUID)
            continue;

        if (!transport || transport->GetGUID() != dest._transportGUID)
            transport = ObjectAccessor::GetWorldObject(*m_caster, dest._transportGUID);

        if (transport)
        {
            dest._position.Relocate(transport);
            dest._position.RelocateOffset(dest._transportOffset);
        }
    }
}

CurrentSpellTypes Spell::GetCurrentContainer() const
{
    if (IsNextMeleeSwingSpell())
        return CURRENT_MELEE_SPELL;
    if (IsAutoRepeat())
        return CURRENT_AUTOREPEAT_SPELL;
    if (m_spellInfo->IsChanneled())
        return CURRENT_CHANNELED_SPELL;
    return CURRENT_GENERIC_SPELL;
}

bool Spell::CheckEffectTarget(Unit const* target, uint32 eff) const
{
    switch (m_spellInfo->Effects[eff]->ApplyAuraName)
    {
        case SPELL_AURA_MOD_POSSESS:
        case SPELL_AURA_MOD_CHARM:
        case SPELL_AURA_AOE_CHARM:
            if (m_caster->IsPlayer() && target->IsCreature() && target->IsVehicle())
                return false;
            if (target->IsMounted() && !target->HasFlag(UNIT_FIELD_FLAGS_3, UNIT_FLAG3_NOT_CHECK_MOUNT))
                return false;
            if (target->GetCharmerGUID())
                return false;
            if (int32 damage = CalculateDamage(eff, target))
                if ((int32)target->getLevelForTarget(m_caster) > damage)
                    return false;
            break;
        default:
            break;
    }

    // check for ignore LOS on the effect itself
    if (canHitTargetInLOS || DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, nullptr, SPELL_DISABLE_LOS))
        return true;

    // if spell is triggered, need to check for LOS disable on the aura triggering it and inherit that behaviour
    if (IsTriggered() && m_triggeredByAuraSpell && (m_triggeredByAuraSpell->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) || DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_triggeredByAuraSpell->Id, nullptr, SPELL_DISABLE_LOS)))
        return true;

    // Special ignore check. Example creature entry: 99801 (Maw of Souls)
    if (target->IsCreature() && target->ToCreature()->IsIgnoreLos())
        return true;

    // todo: shit below shouldn't be here, but it's temporary
    //Check targets for LOS visibility (except spells without range limitations)
    switch (m_spellInfo->Effects[eff]->Effect)
    {
        case SPELL_EFFECT_RESURRECT_NEW:
            // player far away, maybe his corpse near?
            if (target != m_caster && !target->IsWithinLOSInMap(m_caster))
            {
                if (!m_targets.GetCorpseTargetGUID())
                    return false;

                Corpse* corpse = ObjectAccessor::GetCorpse(*m_caster, m_targets.GetCorpseTargetGUID());
                if (!corpse)
                    return false;

                if (target->GetGUID() != corpse->GetOwnerGUID())
                    return false;

                if (!corpse->IsWithinLOSInMap(m_caster))
                    return false;
            }

            // all ok by some way or another, skip normal check
            break;
        default:                                            // normal case
            // Get GO cast coordinates if original caster -> GO
            WorldObject* caster = nullptr;
            if (m_originalCasterGUID.IsGameObject())
                caster = m_caster->GetMap()->GetGameObject(m_originalCasterGUID);
            if (!caster)
                caster = m_caster;
//             if (target != m_caster && !target->IsWithinLOSInMap(caster))
//                 return false;
            break;
    }

    return true;
}

void Spell::SetStartCastTime(uint32 time)
{
    m_castedTime = time;
}

void Spell::SetTriggeredCastFlags(uint32 Flag)
{
    _triggeredCastFlags = Flag;
}

uint32 Spell::GetTriggeredCastFlags() const
{
    return _triggeredCastFlags;
}

bool Spell::IsNextMeleeSwingSpell() const
{
    return m_spellInfo->HasAttribute(SPELL_ATTR0_ON_NEXT_SWING) || m_spellInfo->HasAttribute(SPELL_ATTR0_ON_NEXT_SWING_2);
}

bool Spell::IsAutoActionResetSpell() const
{
    /// @todo changed SPELL_INTERRUPT_FLAG_AUTOATTACK -> SPELL_INTERRUPT_FLAG_INTERRUPT to fix compile - is this check correct at all?
    if (IsTriggered() || !(m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT))
        return false;

    if (!m_casttime && m_spellInfo->HasAttribute(SPELL_ATTR6_NOT_RESET_SWING_IF_INSTANT))
        return false;

    return true;
}

bool Spell::IsNeedSendToClient() const
{
    return m_SpellVisual || m_spellInfo->IsChanneled() || m_replaced ||
        m_spellInfo->HasAttribute(SPELL_ATTR8_AURA_SEND_AMOUNT) || m_spellInfo->GetMisc(m_diffMode)->MiscData.Speed > 0.0f || !m_triggeredByAuraSpell;
}

bool Spell::HaveTargetsForEffect(uint8 effect) const
{
    for (const auto& itr : m_UniqueTargetInfo)
        if (itr->effectMask & (1 << effect))
            return true;

    for (const auto& itr : m_UniqueGOTargetInfo)
        if (itr.effectMask & (1 << effect))
            return true;

    for (auto itr : m_UniqueItemInfo)
        if (itr.effectMask & (1 << effect))
            return true;

    return false;
}

SpellEvent::SpellEvent(Spell* spell) : BasicEvent()
{
    m_Spell = spell;
}

SpellEvent::~SpellEvent()
{
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();

    if (m_Spell->IsDeletable())
        delete m_Spell;
    else
    {
        TC_LOG_ERROR(LOG_FILTER_SPELLS_AURAS, "~SpellEvent: %s %u tried to delete non-deletable spell %u. Was not deleted, causes memory leak.", (m_Spell->GetCaster()->IsPlayer() ? "Player" : "Creature"), m_Spell->GetCaster()->GetGUIDLow(), m_Spell->m_spellInfo->Id);
        //ASSERT(false);
    }
}

bool SpellEvent::Execute(uint64 e_time, uint32 p_time)
{
    uint32 _ss = getMSTime();
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->update(p_time);

    switch (m_Spell->getState())
    {
        case SPELL_STATE_FINISHED:
            if (m_Spell->IsDeletable())
                return true;
            break;
        case SPELL_STATE_DELAYED:
            if (m_Spell->GetDelayStart() != 0)
            {
                if (uint64 n_offset = m_Spell->handle_delayed(e_time - m_Spell->GetDelayStart()))
                {
                    m_Spell->GetCaster()->m_Events.AddEvent(this, m_Spell->GetDelayStart() + n_offset, false);
                    return false;
                }
            }
            else
            {
                m_Spell->SetDelayStart(e_time);
                m_Spell->GetCaster()->m_Events.AddEvent(this, e_time + m_Spell->GetDelayMoment(), false);
                return false;
            }
            break;
        default:
            break;
    }

    uint32 _mss = GetMSTimeDiffToNow(_ss);
    if (_mss > 250)
        sLog->outDiff("SpellEvent::Execute: Caster %u entry %u SpellId %u wait %ums diff %u TargetCount %u", m_Spell->GetCaster()->GetGUIDLow(), m_Spell->GetCaster()->GetEntry(), m_Spell->m_spellInfo->Id, _mss, p_time, m_Spell->GetTargetCount());

    m_Spell->GetCaster()->m_Events.AddEvent(this, e_time + 1, false);
    return false;
}

void SpellEvent::Abort(uint64 /*e_time*/)
{
    // oops, the spell we try to do is aborted
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();
}

bool SpellEvent::IsDeletable() const
{
    return m_Spell->IsDeletable();
}

bool Spell::IsValidDeadOrAliveTarget(Unit const* target) const
{
    if (target->isAlive())
        return !m_spellInfo->IsRequiringDeadTarget();
    if (m_spellInfo->IsAllowingDeadTarget())
        return true;
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_DEAD))
        return true;
    return false;
}

void Spell::HandleLaunchPhase()
{
    // handle effects with SPELL_EFFECT_HANDLE_LAUNCH mode
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if ((m_spellInfo->EffectMask & (1 << i)) == 0)
            continue;
        // don't do anything for empty effect
        if (!m_spellInfo->GetEffect(i, m_diffMode)->IsEffect())
            continue;

        if (m_spellMissMask & ((1 << SPELL_MISS_MISS) | (1 << SPELL_MISS_IMMUNE)))
            if (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_CASTER)
                continue;

        HandleEffects(nullptr, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_LAUNCH);
    }

    float multiplier[MAX_SPELL_EFFECTS];
    for (float& i : multiplier)
        i = 0.0f;

    for (TargetInfoPtr info : m_UniqueTargetInfo)
    {
        if (!info->effectMask)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (m_applyMultiplierMask & (1 << i))
            {
                if (multiplier[i] == 0.0f)
                    multiplier[i] = 1.0f;
                else
                    multiplier[i] *= m_spellInfo->GetEffect(i, m_diffMode)->CalcDamageMultiplier(m_originalCaster, this);
            }
        }
        DoAllEffectOnLaunchTarget(info, multiplier);
    }
}

void Spell::DoAllEffectOnLaunchTarget(TargetInfoPtr targetInfo, float* multiplier)
{
    Unit* unit = nullptr;
    Unit* caster = m_caster;
    // In case spell hit target, do all effect on that target
    if (targetInfo->missCondition == SPELL_MISS_NONE)
        unit = m_caster->GetGUID() == targetInfo->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, targetInfo->targetGUID);
    // In case spell reflect from target, do all effect on caster (if hit)
    else if (targetInfo->missCondition == SPELL_MISS_REFLECT && targetInfo->reflectResult == SPELL_MISS_NONE)
        unit = m_caster;

    if (!unit)
        return;

    if (m_originalCaster && m_spellInfo->HasAttribute(SPELL_ATTR0_CU_BONUS_FROM_ORIGINAL_CASTER))
        caster = m_originalCaster;

    bool canCritTo = false;

    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (targetInfo->effectMask & (1<<i))
        {
            m_damage = 0;
            m_healing = 0;

            HandleEffects(unit, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_LAUNCH_TARGET);

            if (m_damage > 0)
            {
                m_isDamageSpell = true;
                if (m_spellInfo->GetEffect(i, m_diffMode)->IsTargetingArea() || (_triggeredCastFlags & TRIGGERED_CASTED_BY_AREATRIGGER))
                {
                    m_damage = int32(float(m_damage) * unit->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE, m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask));
                    if (caster->IsCreature())
                        m_damage = int32(float(m_damage) * unit->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE, m_spellInfo->GetMisc(m_diffMode)->MiscData.SchoolMask));

                    if (Player* player = unit->ToPlayer())
                        m_damage -= CalculatePct(m_damage, player->GetFloatValue(PLAYER_FIELD_AVOIDANCE));

                    if (caster->IsPlayer())
                    {
                        uint32 targetAmount = m_UniqueTargetInfo.size();
                        if (targetAmount > 20)
                            m_damage = m_damage * 20 / targetAmount;
                    }
                }

                if (m_spellInfo->Effects[i]->Effect == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
                {
                    targetInfo->damageBeforeHit += m_damage;
                    m_damage = unit->MeleeDamageBonusTaken(caster, m_damage, m_attackType, m_spellInfo);
                }
                else
                {
                    targetInfo->damageBeforeHit += m_damage;
                    m_damage = unit->SpellDamageBonusTaken(caster, m_spellInfo, m_damage);
                }
            }

            if (m_applyMultiplierMask & (1 << i))
                m_damage = int32(m_damage * multiplier[i]);

            targetInfo->damage += m_damage;

            if (m_spellInfo->Effects[i]->Effect == SPELL_EFFECT_HEALTH_LEECH) // Effect not have damage on launce phase
                canCritTo = true;
        }
    }

    float critChance = 0.0f;

    if (targetInfo->damage || canCritTo)
    {
        if ((m_originalCaster ? m_originalCaster : m_caster)->isSpellCrit(unit, m_spellInfo, m_spellSchoolMask, m_attackType, critChance, this))
            targetInfo->AddMask(TARGET_INFO_CRIT);
    }

    if (targetInfo->HasMask(TARGET_INFO_CRIT))
    {
        if (!mCriticalDamageBonus)
            mCriticalDamageBonus = caster->SpellCriticalDamageBonus(m_spellInfo, unit);

        targetInfo->damageBeforeHit *= mCriticalDamageBonus;
    }
}

SpellCastResult Spell::CanOpenLock(uint32 effIndex, uint32 lockId, SkillType& skillId, int32& reqSkillValue, int32& skillValue)
{
    if (!lockId) // possible case for GO and maybe for items.
        return SPELL_CAST_OK;

    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
    if (!lockInfo)
        return SPELL_FAILED_BAD_TARGETS;

    auto SkillByLockType = [](LockType locktype) -> SkillType
    {
        switch (locktype)
        {
        case LOCKTYPE_PICKLOCK:
            return SKILL_LOCKPICKING;
        case LOCKTYPE_HERBALISM:
            return SKILL_HERBALISM;
        case LOCKTYPE_MINING:
            return SKILL_MINING;
        case LOCKTYPE_FISHING:
            return SKILL_FISHING;
        case LOCKTYPE_INSCRIPTION:
            return SKILL_INSCRIPTION;
        case LOCKTYPE_ARCHAELOGY:
            return SKILL_ARCHAEOLOGY;
        default:
            return SKILL_NONE;
        }
    };

    bool reqKey = false;                                    // some locks not have reqs

    for (auto j = 0; j < MAX_LOCK_CASE; ++j)
    {
        switch (lockInfo->Type[j])
        {
            case LOCK_KEY_ITEM: // check key item (many fit cases can be)
                if (lockInfo->Index[j] && m_CastItem && m_CastItem->GetEntry() == lockInfo->Index[j])
                    return SPELL_CAST_OK;
                reqKey = true;
                break;
            case LOCK_KEY_SKILL: // check key skill (only single first fit case can be)
            {
                reqKey = true;

                // wrong locktype, skip
                if (uint32(m_spellInfo->GetEffect(effIndex, m_diffMode)->MiscValue) != lockInfo->Index[j])
                    continue;

                skillId = SkillByLockType(LockType(lockInfo->Index[j]));

                if (skillId != SKILL_NONE)
                {
                    reqSkillValue = lockInfo->Skill[j];

                    // castitem check: rogue using skeleton keys. the skill values should not be added in this case.
                    skillValue = m_CastItem || m_caster->GetTypeId()!= TYPEID_PLAYER ? 0 : m_caster->ToPlayer()->GetSkillValue(skillId);

                    // skill bonus provided by casting spell (mostly item spells)
                    // add the effect base points modifier from the spell casted (cheat lock / skeleton key etc.)
                    if (m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetA.GetTarget() == TARGET_GAMEOBJECT_ITEM_TARGET || m_spellInfo->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_GAMEOBJECT_ITEM_TARGET)
                        skillValue += m_spellInfo->GetEffect(effIndex, m_diffMode)->CalcValue(m_caster, nullptr);

                    if (skillValue < reqSkillValue)
                        return SPELL_FAILED_LOW_CASTLEVEL;
                }
                return SPELL_CAST_OK;
            }
            case LOCK_KEY_SPELL:
            {
                if (lockInfo->Index[j] == 143917 && m_caster->HasAura(146589))
                    return SPELL_CAST_OK;
                switch (m_spellInfo->Id)
                {
                case 218867:
                    if (Player* player = m_caster->ToPlayer())
                        if (player->IsInArmyTraining() && player->armyTrainingInfo.currentUnits[ARMY_UNIT_BERSERK].size() < 2)
                        {
                            m_customError = SPELL_CUSTOM_ERROR_REQUIRES_2_WITHERED_BERSERKER;
                            return SPELL_FAILED_CUSTOM_ERROR;
                        }
                    break;
                case 218846:
                    if (Player* player = m_caster->ToPlayer())
                        if (player->IsInArmyTraining() && player->GetAuraCount(227261) < 10)
                        {
                            m_customError = SPELL_CUSTOM_ERROR_REQUIRES_AT_LEAST_10_WITHERED;
                            return SPELL_FAILED_CUSTOM_ERROR;
                        }
                    break;
                default:
                    break;
                }
                if (lockInfo->Index[j] == m_spellInfo->Id) // May be bug? analyse this
                    return SPELL_CAST_OK;
                reqKey = true;
                break;
            }
            default:
                break;
        }
    }

    if (reqKey)
        return SPELL_FAILED_BAD_TARGETS;

    return SPELL_CAST_OK;
}

void Spell::SetSpellValue(SpellValueMod mod, float value, bool lockValue)
{
    switch (mod)
    {
        case SPELLVALUE_BASE_POINT0:
            m_spellValue->EffectBasePoints[0] = m_spellInfo->GetEffect(EFFECT_0, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[0] = lockValue;
            break;
        case SPELLVALUE_BASE_POINT1:
            m_spellValue->EffectBasePoints[1] = m_spellInfo->GetEffect(EFFECT_1, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[1] = lockValue;
            break;
        case SPELLVALUE_BASE_POINT2:
            m_spellValue->EffectBasePoints[2] = m_spellInfo->GetEffect(EFFECT_2, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[2] = lockValue;
            break;
        case SPELLVALUE_BASE_POINT3:
            m_spellValue->EffectBasePoints[3] = m_spellInfo->GetEffect(EFFECT_3, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[3] = lockValue;
            break;
        case SPELLVALUE_BASE_POINT4:
            m_spellValue->EffectBasePoints[4] = m_spellInfo->GetEffect(EFFECT_4, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[4] = lockValue;
            break;
        case SPELLVALUE_BASE_POINT5:
            m_spellValue->EffectBasePoints[5] = m_spellInfo->GetEffect(EFFECT_5, m_diffMode)->CalcBaseValue(value);
            m_spellValue->LockBasePoints[5] = lockValue;
            break;
        case SPELLVALUE_RADIUS_MOD:
            m_spellValue->RadiusMod = (float)value / 10000;
            break;
        case SPELLVALUE_MAX_TARGETS:
            m_spellValue->MaxAffectedTargets = (uint32)value;
            break;
        case SPELLVALUE_AURA_STACK:
            m_spellValue->AuraStackAmount = uint8(value);
            break;
    }
}

void Spell::PrepareTargetProcessing()
{
}

void Spell::FinishTargetProcessing()
{
    SendSpellExecuteLog();
}

void Spell::LoadScripts()
{
    sScriptMgr->CreateSpellScripts(m_spellInfo->Id, m_loadedScripts, this);
    for (auto itr = m_loadedScripts.begin(); itr != m_loadedScripts.end(); ++itr)
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::LoadScripts: Script `%s` for spell `%u` is loaded now", (*itr)->_GetScriptName()->c_str(), m_spellInfo->Id);
        (*itr)->Register();
    }
}

void Spell::CallScriptBeforeCastHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_CAST);
        auto hookItrEnd = m_loadedScript->BeforeCast.end(), hookItr = m_loadedScript->BeforeCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptBeforeStartCastHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_START_CAST);
        auto hookItrEnd = m_loadedScript->BeforeStartCast.end(), hookItr = m_loadedScript->BeforeStartCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptOnCastHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_ON_CAST);
        auto hookItrEnd = m_loadedScript->OnCast.end(), hookItr = m_loadedScript->OnCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptAfterCastHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_CAST);
        auto hookItrEnd = m_loadedScript->AfterCast.end(), hookItr = m_loadedScript->AfterCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptOnFinishCastHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_ON_FINISH_CAST);
        auto hookItrEnd = m_loadedScript->OnFinishCast.end(), hookItr = m_loadedScript->OnFinishCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

SpellCastResult Spell::CallScriptCheckCastHandlers()
{
    SpellCastResult retVal = SPELL_CAST_OK;
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_CHECK_CAST);
        auto hookItrEnd = m_loadedScript->OnCheckCast.end(), hookItr = m_loadedScript->OnCheckCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
        {
            SpellCastResult tempResult = (*hookItr).Call(m_loadedScript);
            if (retVal == SPELL_CAST_OK)
                retVal = tempResult;
        }

        m_loadedScript->_FinishScriptCall();
    }

    if(retVal == SPELL_CAST_OK)
        retVal = CustomCheckCast();

    return retVal;
}

SpellCastResult Spell::CustomCheckCast()
{
    SpellCastResult retVal = SPELL_CAST_OK;

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::CustomCheckCast spellId %u", m_spellInfo->Id);

    if (std::vector<SpellCheckCast> const* checkCast = sSpellMgr->GetSpellCheckCast(m_spellInfo->Id))
    {
        bool check = false;
        for (const auto& itr : *checkCast)
        {
            Unit* _caster = m_originalCaster ? m_originalCaster : m_caster;
            Unit* _target = m_targets.GetUnitTarget();

            if (itr.target)
                _target = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(_caster, _target, itr.target, m_targets.GetUnitTarget());

            if (itr.caster)
                _caster = (m_originalCaster ? m_originalCaster : m_caster)->GetUnitForLinkedSpell(_caster, _target, itr.caster, m_targets.GetUnitTarget());

            if(!_caster)
                check = true;

            if(!_target)
                _target = _caster;

            if(itr.dataType)
                if(m_caster->HasAuraLinkedSpell(m_caster, _target, itr.checkType, itr.dataType, itr.param2))
                    check = true;

            if(itr.dataType2)
                if(m_caster->HasAuraLinkedSpell(m_caster, _target, itr.checkType2, itr.dataType2, itr.param3))
                    check = true;

            switch (itr.type)
            {
                case SPELL_CHECK_CAST_DEFAULT: // 0
                    if(itr.param1 < 0 && !check)
                        check = true;
                    else if(itr.param1 < 0 && check)
                        check = false;
                    break;
                case SPELL_CHECK_CAST_HEALTH: // 1
                {
                    if(check)
                        break;
                    if (itr.param1 > 0 && _target->GetHealthPct() < itr.param1)
                        check = true;
                    else if (itr.param1 < 0 && _target->GetHealthPct() >= abs(itr.param1))
                        check = true;
                    break;
                }
            }

            //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::CustomCheckCast spellId %u check %i param1 %i type %i errorId %i customErrorId %i",
            //m_spellInfo->Id, check, itr->param1, itr->type, itr->errorId, itr->customErrorId);

            if(check)
            {
                if (itr.customErrorId)
                {
                    retVal = SPELL_FAILED_CUSTOM_ERROR;
                    m_customError = (SpellCustomErrors)itr.customErrorId;
                }
                else
                    retVal = (SpellCastResult)itr.errorId;
                break;
            }
        }
    }

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::CustomCheckCast spellId %u retVal %i m_customError %i", m_spellInfo->Id, retVal, m_customError);

    return retVal;
}

void Spell::CallScriptTakePowerHandlers(Powers p, int32 &amount)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_TAKE_POWER);
        auto hookItrEnd = m_loadedScript->OnTakePower.end(), hookItr = m_loadedScript->OnTakePower.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript, p, amount);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::PrepareScriptHitHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
        m_loadedScript->_InitHit();
}

bool Spell::CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode)
{
    // execute script effect handler hooks and check if effects was prevented
    bool preventDefault = false;
    for (auto& m_loadedScript : m_loadedScripts)
    {
        HookList<SpellScript::EffectHandler>::iterator effItr, effEndItr;
        SpellScriptHookType hookType;
        switch (mode)
        {
            case SPELL_EFFECT_HANDLE_LAUNCH:
                effItr = m_loadedScript->OnEffectLaunch.begin();
                effEndItr = m_loadedScript->OnEffectLaunch.end();
                hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH;
                break;
            case SPELL_EFFECT_HANDLE_LAUNCH_TARGET:
                effItr = m_loadedScript->OnEffectLaunchTarget.begin();
                effEndItr = m_loadedScript->OnEffectLaunchTarget.end();
                hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH_TARGET;
                break;
            case SPELL_EFFECT_HANDLE_HIT:
                effItr = m_loadedScript->OnEffectHit.begin();
                effEndItr = m_loadedScript->OnEffectHit.end();
                hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT;
                break;
            case SPELL_EFFECT_HANDLE_HIT_TARGET:
                effItr = m_loadedScript->OnEffectHitTarget.begin();
                effEndItr = m_loadedScript->OnEffectHitTarget.end();
                hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET;
                break;
            default:
                ASSERT(false);
                return false;
        }
        m_loadedScript->_PrepareScriptCall(hookType);
        for (; effItr != effEndItr; ++effItr)
            // effect execution can be prevented
            if (!m_loadedScript->_IsEffectPrevented(effIndex) && (*effItr).IsEffectAffected(m_spellInfo, effIndex))
                (*effItr).Call(m_loadedScript, effIndex);

        if (!preventDefault)
            preventDefault = m_loadedScript->_IsDefaultEffectPrevented(effIndex);

        m_loadedScript->_FinishScriptCall();
    }
    return preventDefault;
}

void Spell::CallScriptBeforeHitHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_HIT);
        auto hookItrEnd = m_loadedScript->BeforeHit.end(), hookItr = m_loadedScript->BeforeHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptSuccessfulDispel(SpellEffIndex effIndex)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_EFFECT_SUCCESSFUL_DISPEL);
        auto hookItrEnd = m_loadedScript->OnEffectSuccessfulDispel.end(), hookItr = m_loadedScript->OnEffectSuccessfulDispel.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            hookItr->Call(m_loadedScript, effIndex);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptOnHitHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_HIT);
        auto hookItrEnd = m_loadedScript->OnHit.end(), hookItr = m_loadedScript->OnHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptAfterHitHandlers()
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_HIT);
        auto hookItrEnd = m_loadedScript->AfterHit.end(), hookItr = m_loadedScript->AfterHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, Targets targetId)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_AREA_TARGET_SELECT);
        auto hookItrEnd = m_loadedScript->OnObjectAreaTargetSelect.end(), hookItr = m_loadedScript->OnObjectAreaTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if ((*hookItr).IsEffectAffected(m_spellInfo, effIndex))
                (*hookItr).Call(m_loadedScript, targets);

        m_loadedScript->_FinishScriptCall();
    }
    CustomTargetSelector(targets, effIndex, targetId);
}

void Spell::CallScriptCalculateEffMaskHandlers(uint32& effMask)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_CALC_EFFECT_MASK);
        auto hookItrEnd = m_loadedScript->DoCalcEffMask.end(), hookItr = m_loadedScript->DoCalcEffMask.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript, effMask);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CustomTargetSelector(std::list<WorldObject*>& targets, SpellEffIndex effIndex, Targets targetId)
{
    if (std::vector<SpellTargetFilter> const* spellTargetFilter = sSpellMgr->GetSpellTargetFilter(m_spellInfo->Id))
    {
        uint32 targetCount = 0;
        uint32 resizeType = 0;
        int32 addcaster = 0;
        Unit* _caster = m_originalCaster ? m_originalCaster : m_caster;
        Unit* _owner = _caster->GetAnyOwner();

        if(_caster->isTotem() && _owner)
            _caster = _owner;

        Unit* _target = m_targets.GetUnitTarget();
        if(!_target && m_caster->ToPlayer())
            _target = m_caster->ToPlayer()->GetSelectedUnit();
        else if(!_target)
            _target = m_caster->getVictim();

        for (const auto& itr : *spellTargetFilter)
        {
            if (!(itr.effectMask & (1<<effIndex)))
                continue;

            if (itr.targetId != targetId)
                continue;

            if(!addcaster)
                addcaster = itr.addcaster;

            if (targets.empty())
            {
                if(addcaster == 2)
                    targets.push_back(GetCaster());
                return;
            }

            if(itr.count && !targetCount && !itr.maxcount)
                targetCount = itr.count;
            else if(itr.maxcount && !targetCount)
            {
                if(m_caster->GetMap()->Is25ManRaid())
                    targetCount = itr.maxcount;
                else
                    targetCount = itr.count;
            }

            if(itr.resizeType && !resizeType)
                resizeType = itr.resizeType;

            if(itr.aura > 0 && _caster->HasAura(itr.aura))
                targetCount += itr.addcount;
            if(itr.aura < 0 && !_caster->HasAura(abs(itr.aura)))
                targetCount -= itr.addcount;

            if(addcaster < 0)
                targets.remove(GetCaster());
            if(addcaster == 2 || addcaster == 3)
                targets.remove(GetCaster());
            if(addcaster == 4)
                targets.clear();

            switch (itr.option)
            {
                case SPELL_FILTER_SORT_BY_HEALT: //0
                {
                    if (itr.param2 < 0.0f)
                        targets.remove_if(Trinity::UnitFullHPCheck(true));
                    else if (itr.param2 > 0.0f)
                        targets.remove_if(Trinity::UnitFullHPCheck(false));

                    if(itr.param1 < 0.0f)
                        targets.sort(Trinity::UnitHealthState(false));
                    else
                        targets.sort(Trinity::UnitHealthState(true));

                    break;
                }
                case SPELL_FILTER_BY_AURA: //1
                {
                    targets.remove_if(Trinity::UnitAuraAndCheck(itr.param1, itr.param2, itr.param3));
                    break;
                }
                case SPELL_FILTER_BY_DISTANCE: //2
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitDistanceCheck(false, m_caster, itr.param2));
                    else
                        targets.remove_if(Trinity::UnitDistanceCheck(true, m_caster, itr.param2));
                    break;
                }
                case SPELL_FILTER_TARGET_TYPE: //3
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitTypeCheck(true, uint32(itr.param2)));
                    else
                        targets.remove_if(Trinity::UnitTypeCheck(false, uint32(itr.param2)));
                    break;
                }
                case SPELL_FILTER_SORT_BY_DISTANCE: //4
                {
                    if(itr.param1 < 0.0f)
                        targets.sort(Trinity::UnitSortDistance(false, m_caster));
                    else
                        targets.sort(Trinity::UnitSortDistance(true, m_caster));
                    break;
                }
                case SPELL_FILTER_TARGET_FRIENDLY: //5
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitFriendlyCheck(true, _caster));
                    else
                        targets.remove_if(Trinity::UnitFriendlyCheck(false, _caster));
                    break;
                }
                case SPELL_FILTER_TARGET_IN_RAID: //6
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitRaidCheck(true, _caster));
                    else
                        targets.remove_if(Trinity::UnitRaidCheck(false, _caster));
                    break;
                }
                case SPELL_FILTER_TARGET_IN_PARTY: //7
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitPartyCheck(true, _caster));
                    else
                        targets.remove_if(Trinity::UnitPartyCheck(false, _caster));
                    break;
                }
                case SPELL_FILTER_TARGET_EXPL_TARGET: //8
                {
                    if (!itr.param1)
                        targets.clear();
                    if(_target)
                        targets.push_back(_target);
                    break;
                }
                case SPELL_FILTER_TARGET_EXPL_TARGET_REMOVE: //9
                {
                    if (targets.size() > uint32(itr.param1))
                        if(_target)
                            targets.remove(_target);
                    break;
                }
                case SPELL_FILTER_TARGET_IN_LOS: //10
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitCheckInLos(true, m_caster));
                    else
                        targets.remove_if(Trinity::UnitCheckInLos(false, m_caster));
                    break;
                }
                case SPELL_FILTER_TARGET_IS_IN_BETWEEN: //11
                {
                    if(_target)
                        targets.remove_if(Trinity::UnitCheckInBetween(false, m_caster, _target, itr.param1));
                    break;
                }
                case SPELL_FILTER_TARGET_IS_IN_BETWEEN_SHIFT: //12
                {
                    if(_target)
                        targets.remove_if(Trinity::UnitCheckInBetweenShift(false, m_caster, _target, itr.param1,
                                                                           itr.param2, itr.param3));
                    break;
                }
                case SPELL_FILTER_BY_AURA_OR: //13
                {
                    targets.remove_if(Trinity::UnitAuraOrCheck(itr.param1, itr.param2, itr.param3));
                    break;
                }
                case SPELL_FILTER_BY_ENTRY: //14
                {
                    targets.remove_if(Trinity::UnitEntryCheck(itr.param1, itr.param2, itr.param3));
                    break;
                }
                case SPELL_FILTER_TARGET_ATTACKABLE: // 15
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitAttackableCheck(true, _caster));
                    else
                        targets.remove_if(Trinity::UnitAttackableCheck(false, _caster));
                    break;
                }
                case SPELL_FILTER_BY_DISTANCE_TARGET: //16
                {
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector 0");
                    if (_target)
                    {
                        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector 1 guid %u", _target->GetGUIDLow());
                        if(itr.param1 < 0.0f)
                            targets.remove_if(Trinity::UnitDistanceCheck(false, _target, itr.param2));
                        else
                            targets.remove_if(Trinity::UnitDistanceCheck(true, _target, itr.param2));
                    }
                    break;
                }
                case SPELL_FILTER_OWNER_TARGET_REMOVE: //17
                {
                    if (Unit* owner = m_caster->GetAnyOwner())
                    {
                        targets.remove(owner);
                        if (targets.size() > uint32(itr.param1))
                        {
                            if (owner->getVictim())
                                targets.remove(owner->getVictim());
                            else if (owner->ToPlayer() && owner->ToPlayer()->GetSelectedUnit())
                                targets.remove(owner->ToPlayer()->GetSelectedUnit());
                        }
                    }
                    break;
                }
                case SPELL_FILTER_SORT_BY_DISTANCE_FROM_TARGET: // 18
                {
                    if(itr.param1 < 0.0f)
                        targets.sort(Trinity::UnitSortDistance(false, _target));
                    else
                        targets.sort(Trinity::UnitSortDistance(true, _target));
                    break;
                }
                case SPELL_FILTER_BY_DISTANCE_DEST: //19
                {
                    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector 0");
                    if (m_targets.HasDst())
                    {
                        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector HasDst 1");
                        if(itr.param1 < 0.0f)
                            targets.remove_if(Trinity::DestDistanceCheck(false, (Position*)m_targets.GetDstPos(), itr.param2));
                        else
                            targets.remove_if(Trinity::DestDistanceCheck(true, (Position*)m_targets.GetDstPos(), itr.param2));
                    }
                    else if (m_targets.HasSrc())
                    {
                        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector HasSrc 1");
                        if(itr.param1 < 0.0f)
                            targets.remove_if(Trinity::DestDistanceCheck(false, (Position*)m_targets.GetSrcPos(), itr.param2));
                        else
                            targets.remove_if(Trinity::DestDistanceCheck(true, (Position*)m_targets.GetSrcPos(), itr.param2));
                    }
                    break;
                }
                case SPELL_FILTER_BY_DISTANCE_PET: //20
                {
                    _target = nullptr;
                    if(m_caster->ToPlayer())
                        _target = m_caster->ToPlayer()->GetPet();
                    if (_target)
                    {
                        if(itr.param1 < 0.0f)
                            targets.remove_if(Trinity::UnitDistanceCheck(false, _target, itr.param2));
                        else
                            targets.remove_if(Trinity::UnitDistanceCheck(true, _target, itr.param2));
                    }
                    break;
                }
                case SPELL_FILTER_BY_OWNER: // 21
                {
                    if(itr.param1 < 0.0f)
                        targets.remove_if(Trinity::UnitOwnerCheck(true, _caster));
                    else
                        targets.remove_if(Trinity::UnitOwnerCheck(false, _caster));
                    break;
                }
                case SPELL_FILTER_ONLY_RANGED_SPEC: // 22
                {
                    std::list<WorldObject*> rangeList;

                    for (auto const& target : targets)
                    {
                        if (auto player = target->ToPlayer())
                        {
                            if (player->IsRangedDamageDealer(uint32(itr.param1))) //param1 == 1 - Also the Healer
                                rangeList.push_back(target);
                        }
                    }
                    targets = rangeList;
                    break;
                }
                case SPELL_FILTER_ONLY_MELEE_SPEC: // 23
                {
                    std::list<WorldObject*> meleeList;

                    for (auto const& target : targets)
                    {
                        if (auto player = target->ToPlayer())
                        {
                            if (player->IsMeleeDamageDealer(uint32(itr.param1))) //param1 == 1 - Also the Tank
                            {
                                meleeList.push_back(target);
                            }
                        }
                    }
                    targets = meleeList;
                    break;
                }
                case SPELL_FILTER_ONLY_TANK_SPEC_OR_NOT: // 24
                {
                    if (targets.size() <= uint32(itr.param2))
                        break;

                    std::list<WorldObject*> tempList;

                    for (auto const& target : targets)
                    {
                        if (auto player = target->ToPlayer())
                        {
                            if (itr.param1 < 0.0f) //Exclude tank players
                            {
                                if (!player->isInTankSpec())
                                    tempList.push_back(target);
                            }
                            else
                            {
                                if (player->isInTankSpec()) // Exclude non-tank players
                                    tempList.push_back(target);
                            }
                        }
                    }
                    targets = tempList;
                    break;
                }
                case SPELL_FILTER_BY_AURA_CASTER: // 25
                {
                    targets.remove_if(Trinity::UnitAuraAndCheck(itr.param1, itr.param2, itr.param3, (_owner ? _owner : _caster)->GetGUID()));
                    break;
                }
                case SPELL_FILTER_PLAYER_IS_HEALER_SPEC: // 26
                {
                    targets.remove_if([](WorldObject* object) -> bool
                    {
                        if (!object || !object->IsPlayer() || object->ToPlayer()->GetSpecializationRole() != ROLES_HEALER)
                            return true;

                        return false;
                    });
                    break;
                }
                case SPELL_FILTER_RANGED_SPEC_PRIORITY: // 27
                {
                    std::list<WorldObject*> rangeList;
                    std::list<WorldObject*> meleeList;
                    std::list<WorldObject*> tankList;

                    Trinity::Containers::RandomResizeList(targets, targets.size());

                    for (auto const& target : targets)
                    {
                        if (auto player = target->ToPlayer())
                        {
                            if (player->IsRangedDamageDealer(uint32(itr.param1))) //param1 == 1 - Also the Healer
                                rangeList.push_back(target);
                            else if (player->IsMeleeDamageDealer(uint32(itr.param2))) //param2 == 1 - Also the Tank
                            {
                                if (!player->isInTankSpec())
                                    meleeList.push_back(target);
                                else
                                    tankList.push_back(target);
                            }
                        }
                    }
                    targets.clear();
                    targets.splice(targets.end(), rangeList);
                    targets.splice(targets.end(), meleeList);
                    targets.splice(targets.end(), tankList);

                    if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
                    {
                        resizeType = 1;
                        targetCount = maxTargets;
                    }
                    break;
                }
                case SPELL_FILTER_MELEE_SPEC_PRIORITY: // 28
                {
                    std::list<WorldObject*> meleeList;
                    std::list<WorldObject*> rangeList;
                    std::list<WorldObject*> tankList;

                    Trinity::Containers::RandomResizeList(targets, targets.size());

                    for (auto const& target : targets)
                    {
                        if (auto player = target->ToPlayer())
                        {
                            if (player->IsMeleeDamageDealer(uint32(itr.param1))) //param1 == 1 - Also the Tank
                            {
                                if (!player->isInTankSpec())
                                    meleeList.push_back(target);
                                else if (uint32(itr.param1) == 1) // Tank
                                    tankList.push_back(target);
                            }
                            else if (player->IsRangedDamageDealer(uint32(itr.param2))) //param2 == 1 - Also the Healer
                                rangeList.push_back(target);
                        }
                    }
                    targets.clear();
                    targets.splice(targets.end(), meleeList);
                    targets.splice(targets.end(), rangeList);
                    targets.splice(targets.end(), tankList);

                    if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
                    {
                        resizeType = 1;
                        targetCount = maxTargets;
                    }
                    break;
                }
            }
            switch (addcaster)
            {
                case 1:
                    if (!targets.empty())
                        targets.remove(GetCaster());
                    targets.push_back(GetCaster());
                    break;
                case 2:
                    if (targets.empty())
                        targets.push_back(GetCaster());
                    break;
            }
        }
        switch (resizeType)
        {
            case 1:
                if (targets.size() > targetCount)
                    targets.resize(targetCount);
                break;
            case 2:
                if (targets.size() > targetCount)
                    Trinity::Containers::RandomResizeList(targets, targetCount);
                break;
        }
        switch (addcaster)
        {
            case 3:
            case 4:
                targets.push_back(GetCaster());
                break;
        }
        // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "CustomTargetSelector Id %u targetId %u effIndex %u targets %u caster %s", m_spellInfo->Id, targetId, effIndex, targets.size(), m_caster->GetGUID().ToString().c_str()) ;
    }
}

void Spell::CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_TARGET_SELECT);
        auto hookItrEnd = m_loadedScript->OnObjectTargetSelect.end(), hookItr = m_loadedScript->OnObjectTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if ((*hookItr).IsEffectAffected(m_spellInfo, effIndex))
                (*hookItr).Call(m_loadedScript, target);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptObjectJumpTargetHandlers(int32& AdditionalTarget, SpellEffIndex effIndex)
{
    for (auto& m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_JUMP_TARGET);
        auto hookItrEnd = m_loadedScript->OnObjectJumpTarget.end(), hookItr = m_loadedScript->OnObjectJumpTarget.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if ((*hookItr).IsEffectAffected(m_spellInfo, effIndex))
                (*hookItr).Call(m_loadedScript, AdditionalTarget);

        m_loadedScript->_FinishScriptCall();
    }
}

bool Spell::CanExecuteTriggersOnHit(uint32 effMask, SpellInfo const* triggeredByAura) const
{
    bool only_on_caster = (triggeredByAura && (triggeredByAura->HasAttribute(SPELL_ATTR4_PROC_ONLY_ON_CASTER)));
    // If triggeredByAura has SPELL_ATTR4_PROC_ONLY_ON_CASTER then it can only proc on a casted spell with TARGET_UNIT_CASTER
    for (uint8 i = 0;i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if ((effMask & (1 << i)) && (!only_on_caster || (m_spellInfo->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_CASTER)))
            return true;
    }
    return false;
}

void Spell::PrepareTriggersExecutedOnHit()
{
    // todo: move this to scripts
    if (m_spellInfo->ClassOptions.SpellClassSet)
    {
        SpellInfo const* excludeTargetSpellInfo = sSpellMgr->GetSpellInfo(m_spellInfo->AuraRestrictions.ExcludeTargetAuraSpell);
        if (excludeTargetSpellInfo && !excludeTargetSpellInfo->IsPositive())
            m_preCastSpell = m_spellInfo->AuraRestrictions.ExcludeTargetAuraSpell;
    }

    // handle SPELL_AURA_ADD_TARGET_TRIGGER auras:
    // save auras which were present on spell caster on cast, to prevent triggered auras from affecting caster
    // and to correctly calculate proc chance when combopoints are present
    Unit::AuraEffectList const* targetTriggers = m_caster->GetAuraEffectsByType(SPELL_AURA_ADD_TARGET_TRIGGER);
    if (!targetTriggers)
        return;

    for (Unit::AuraEffectList::const_iterator i = targetTriggers->begin(); i != targetTriggers->end(); ++i)
    {
        if (!(*i)->IsAffectingSpell(m_spellInfo))
            continue;
        SpellInfo const* auraSpellInfo = (*i)->GetSpellInfo();
        uint32 auraSpellIdx = (*i)->GetEffIndex();
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(auraSpellInfo->Effects[auraSpellIdx]->TriggerSpell))
        {
            // calculate the chance using spell base amount, because aura amount is not updated on combo-points change
            // this possibly needs fixing
            float auraBaseAmount = (*i)->GetBaseAmount();
            // proc chance is stored in effect amount
            float chance = m_caster->CalculateSpellDamage(nullptr, auraSpellInfo, auraSpellIdx, &auraBaseAmount, nullptr, false, nullptr, GetComboPoints());
            // build trigger and add to the list
            HitTriggerSpell spellTriggerInfo;
            spellTriggerInfo.triggeredSpell = spellInfo;
            spellTriggerInfo.triggeredByAura = auraSpellInfo;
            spellTriggerInfo.chance = chance * (*i)->GetBase()->GetStackAmount();
            m_hitTriggerSpells.push_back(spellTriggerInfo);
        }
    }
}

// Global cooldowns management
enum GCDLimits
{
    MIN_GCD = 750,
    MAX_GCD = 1500
};

bool Spell::HasGlobalCooldown()
{
    Player* player = m_caster->ToPlayer();

    // Only player or controlled units have global cooldown
    if (m_caster->GetCharmInfo() || m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER) || (player && GetGlobalCooldown() > 0))
    {
        return m_caster->GetGlobalCooldownMgr().HasGlobalCooldown(m_spellInfo);
    }
    return false;
}

int32 Spell::GetGlobalCooldown()
{
    int32 gcd = m_spellInfo->Cooldowns.StartRecoveryTime;
    if (!gcd)
        return 0;

     if (m_caster->IsPlayer())
          if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_COOLDOWN))
               return 0;

    // Global cooldown can't leave range 1..1.5 secs
    // There are some spells (mostly not casted directly by player) that have < 1 sec and > 1.5 sec global cooldowns
    // but as tests show are not affected by any spell mods.
    if (m_spellInfo->Cooldowns.StartRecoveryTime >= MIN_GCD && m_spellInfo->Cooldowns.StartRecoveryTime <= MAX_GCD)
    {
        // gcd modifier auras are applied only to own spells and only players have such mods
        if (m_caster->IsPlayer())
            m_caster->ToPlayer()->ApplySpellMod(m_spellInfo->Id, SPELLMOD_GLOBAL_COOLDOWN, gcd, this);

        return gcd;
    }
    return 0;
}

void Spell::TriggerGlobalCooldown()
{
    int32 gcd = m_spellInfo->Cooldowns.StartRecoveryTime;
    if (!gcd || !m_spellInfo->Categories.StartRecoveryCategory)
        return;

    if (!m_caster->IsPlayer() && !m_caster->GetCharmInfo() && !m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER))
        return;

    if (m_caster->IsPlayer())
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_COOLDOWN))
            return;

    // Global cooldown can't leave range 1..1.5 secs
    // There are some spells (mostly not casted directly by player) that have < 1 sec and > 1.5 sec global cooldowns
    // but as tests show are not affected by any spell mods.
    if (m_spellInfo->Cooldowns.StartRecoveryTime >= MIN_GCD && m_spellInfo->Cooldowns.StartRecoveryTime <= MAX_GCD)
    {
        if (m_caster->IsPlayer())
            m_caster->ToPlayer()->ApplySpellMod(m_spellInfo->Id, SPELLMOD_GLOBAL_COOLDOWN, gcd, this);

        bool isMeleeOrRangedSpell = m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_MELEE || m_spellInfo->Categories.DefenseType == SPELL_DAMAGE_CLASS_RANGED ||
            m_spellInfo->HasAttribute(SPELL_ATTR0_REQ_AMMO) || m_spellInfo->HasAttribute(SPELL_ATTR0_ABILITY);

        if (gcd > MIN_GCD && ((m_spellInfo->Categories.StartRecoveryCategory == 133 && !isMeleeOrRangedSpell) || m_caster->HasAuraTypeWithAffectMask(SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE, m_spellInfo)))
            gcd = std::min<int32>(std::max<int32>(int32(float(gcd) * m_caster->GetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED)), MIN_GCD), MAX_GCD);

        if (gcd > MIN_GCD && m_caster->HasAuraTypeWithAffectMask(SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE_REGEN, m_spellInfo))
            gcd = std::min<int32>(std::max<int32>(int32(float(gcd) * m_caster->GetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN)), MIN_GCD), MAX_GCD);

        gcd = std::min<int32>(std::max<int32>(int32(float(gcd) * m_caster->GetFloatValue(UNIT_FIELD_MOD_TIME_RATE)), 500), 3000);
    }

    // Only players or controlled units have global cooldown
    if (m_caster->GetCharmInfo() || m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER))
    {
        m_caster->GetGlobalCooldownMgr().AddGlobalCooldown(m_spellInfo, gcd);
    }
    else if (Player* plr = m_caster->ToPlayer())
    {
        uint32 startGCD = plr->GetSpellInQueue() ? plr->GetSpellInQueue()->GCDEnd : m_castedTime;
        m_caster->GetGlobalCooldownMgr().AddGlobalCooldown(m_spellInfo, gcd, startGCD);
    }
}

void Spell::CancelGlobalCooldown()
{
    if (!m_spellInfo->Cooldowns.StartRecoveryTime)
        return;

    // Cancel global cooldown when interrupting current cast
    if (m_caster->GetCurrentSpell(CURRENT_GENERIC_SPELL) != this)
        return;

    // Only players or controlled units have global cooldown
    if (m_caster->IsPlayer() || m_caster->GetCharmInfo() || m_caster->HasUnitTypeMask(UNIT_MASK_CREATED_BY_PLAYER))
        m_caster->GetGlobalCooldownMgr().CancelGlobalCooldown(m_spellInfo);
}

bool Spell::IsCritForTarget(Unit* target) const
{
    if (!target)
        return false;

    for (const auto& itr : m_UniqueTargetInfo)
        if (itr->targetGUID == target->GetGUID() && itr->HasMask(TARGET_INFO_CRIT))
            return true;

    return false;
}

bool Spell::CanSpellProc(Unit* target, uint32 mask) const
{
    if (m_CastItem)
        return false;
    if (m_spellInfo->HasAttribute(SPELL_ATTR0_HIDDEN_CLIENTSIDE))
        return false;
    if (m_spellInfo->HasAttribute(SPELL_ATTR7_CONSOLIDATED_RAID_BUFF))
        return false;
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_FOOD_BUFF))
        return false;
    if (target && !target->CanProc())
        return false;
    if (!CanExecuteTriggersOnHit(mask))
        return false;
    if (m_spellInfo->IsPassive())
        return false;
    if(m_spellInfo->HasAura(SPELL_AURA_MOUNTED))
        return false;

    return true;
}

bool Spell::IsItemCategoryCombat() const
{
    if (!m_CastItem)
        return false;

    bool categoryCombat = false;

    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(m_CastItem->GetEntry()))
    {
        for (ItemEffectEntry const* effectData : proto->Effects)
        {
            if (effectData->SpellID == m_spellInfo->Id)
            {
                categoryCombat = effectData->SpellCategoryID == 4;
                break;
            }
        }
    }

    return categoryCombat;
}

namespace Trinity
{

WorldObjectSpellTargetCheck::WorldObjectSpellTargetCheck(Unit* caster, Unit* referer, SpellInfo const* spellInfo,
            SpellTargetCheckTypes selectionType, ConditionList* condList) : _caster(caster), _referer(referer), _spellInfo(spellInfo),
    _targetSelectionType(selectionType), _condList(condList)
{
    if (condList)
        _condSrcInfo = new ConditionSourceInfo(nullptr, caster);
    else
        _condSrcInfo = nullptr;
}

WorldObjectSpellTargetCheck::~WorldObjectSpellTargetCheck()
{
    delete _condSrcInfo;
}

bool WorldObjectSpellTargetCheck::operator()(WorldObject* target)
{
    SpellCastResult res = _spellInfo->CheckTarget(_caster, target, true);
    if (_condSrcInfo)
    {
        _condSrcInfo->mConditionTargets[0] = target;
        bool check = sConditionMgr->IsObjectMeetToConditions(*_condSrcInfo, *_condList);
        // if(check)
            // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::WorldObjectSpellTargetCheck spell id %u caster %u target %u entry %i", _spellInfo->Id,  _caster->GetGUID(), target->GetGUID(), target->GetEntry());
        return check;
    }

    if (res != SPELL_CAST_OK)
    {
        #ifdef WIN32
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::WorldObjectSpellTargetCheck::checkcast fail. spell id %u res %u caster %s target %s", _spellInfo->Id, res, _caster->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str());
        #endif
        return false;
    }

    Unit* unitTarget = target->ToUnit();
    if (Corpse* corpseTarget = target->ToCorpse())
    {
        // use ofter for party/assistance checks
        if (Player* owner = ObjectAccessor::FindPlayer(corpseTarget->GetOwnerGUID()))
            unitTarget = owner;
        else
            return false;
    }

    // #ifdef WIN32
    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Spell::WorldObjectSpellTargetCheck spell id %u caster %s target %s _condSrcInfo %u res %u ToCorpse %u ToUnit %u _targetSelectionType %u",
    // _spellInfo->Id, _caster->GetGUID().ToString().c_str(), target->GetGUID().ToString().c_str(), bool(_condSrcInfo), res, bool(target->ToCorpse()), bool(target->ToUnit()), _targetSelectionType);
    // #endif

    if (unitTarget)
    {
        if (_spellInfo->TargetRestrictions.MaxTargetLevel && _spellInfo->TargetRestrictions.MaxTargetLevel < unitTarget->getLevelForTarget(_caster))
            return false;

        if (_caster->IsRWVisibility() && _caster->GetDistance(*unitTarget) > _caster->GetRWVisibility())
            return false;

        if (unitTarget->IsPlayer() && unitTarget->ToPlayer()->isGameMaster())
            return false;

        switch (_targetSelectionType)
        {
            case TARGET_CHECK_ENEMY:
                if (unitTarget->isTotem())
                    return false;
                if (!_caster->_IsValidAttackTarget(unitTarget, _spellInfo))
                    return false;
                break;
            case TARGET_CHECK_SUMMON:
                if (unitTarget->isTotem())
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                if (!unitTarget->IsOwnerOrSelf(_caster))
                    return false;
                break;
            case TARGET_CHECK_ALLY:
                if (unitTarget->isTotem())
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                break;
            case TARGET_CHECK_PARTY:
                if (unitTarget->isTotem())
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                if (!_referer->IsInPartyWith(unitTarget))
                    return false;
                break;
            case TARGET_CHECK_RAID_CLASS:
                if (_referer->getClass() != unitTarget->getClass())
                    return false;
                // nobreak;
            case TARGET_CHECK_RAID:
                if (unitTarget->isTotem())
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                if (!_referer->IsInRaidWith(unitTarget))
                    return false;
                break;
            default:
                break;
        }
    }
    return true;
}

WorldObjectSpellNearbyTargetCheck::WorldObjectSpellNearbyTargetCheck(float range, Unit* caster, SpellInfo const* spellInfo,
    SpellTargetCheckTypes selectionType, ConditionList* condList)
    : WorldObjectSpellTargetCheck(caster, caster, spellInfo, selectionType, condList), _range(range), _position(caster)
{
}

bool WorldObjectSpellNearbyTargetCheck::operator()(WorldObject* target)
{
    float dist = target->GetDistance(*_position);
    if (dist < _range && WorldObjectSpellTargetCheck::operator ()(target))
    {
        _range = dist;
        return true;
    }
    return false;
}

WorldObjectSpellAreaTargetCheck::WorldObjectSpellAreaTargetCheck(float range, Position const* position, Unit* caster,
    Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList, bool allowObjectSize)
    : WorldObjectSpellTargetCheck(caster, referer, spellInfo, selectionType, condList), _range(range), _position(position), _allowObjectSize(allowObjectSize)
{
}

bool WorldObjectSpellAreaTargetCheck::operator()(WorldObject* target)
{
    if (!target->IsWithinDist3d(_position, _range, _allowObjectSize))
        return false;

    // TOS: The Desolate Host
    if (auto _target = target->ToUnit())
        if (!_caster->IsValidDesolateHostTarget(_target, _spellInfo))
            return false;

    if (!_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) && !target->IsWithinLOS(_position->m_positionX, _position->m_positionY, _position->m_positionZ))
        return false;

    return WorldObjectSpellTargetCheck::operator ()(target);
}

WorldObjectSpellBetweenTargetCheck::WorldObjectSpellBetweenTargetCheck(float width, float range, Unit* caster, Position const* position, Unit* referer,
    SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList)
    : WorldObjectSpellAreaTargetCheck(range, caster, caster, referer, spellInfo, selectionType, condList), _width(width), _range(range), _position(position)
{
}

bool WorldObjectSpellBetweenTargetCheck::operator()(WorldObject* target)
{
    if (!target->IsInBetween(_caster, _position->GetPositionX(), _position->GetPositionY(), _width))
        return false;

    return WorldObjectSpellAreaTargetCheck::operator ()(target);
}

WorldObjectSpellConeTargetCheck::WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster,
    SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList)
    : WorldObjectSpellAreaTargetCheck(range, caster, caster, caster, spellInfo, selectionType, condList), _coneAngle(coneAngle)
{
}

bool WorldObjectSpellConeTargetCheck::operator()(WorldObject* target)
{
    if (_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_CONE_BACK)
    {
        if (!_caster->isInBack(target, _coneAngle))
            return false;
    }
    else if (_spellInfo->AttributesCu[0] & SPELL_ATTR0_CU_CONE_LINE)
    {
        if (!_caster->HasInLine(target, _caster->GetObjectSize()))
            return false;
    }
    else
    {
        if (_coneAngle < 0.0f)
        {
            if (!_caster->isInBack(target, fabs(_coneAngle)))
                return false;
        }
        else if (!_caster->isInFront(target, _coneAngle))
            return false;
    }
    return WorldObjectSpellAreaTargetCheck::operator ()(target);
}

WorldObjectSpellTrajTargetCheck::WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster, SpellInfo const* spellInfo)
    : WorldObjectSpellAreaTargetCheck(range, position, caster, caster, spellInfo, TARGET_CHECK_DEFAULT, nullptr)
{
}

bool WorldObjectSpellTrajTargetCheck::operator()(WorldObject* target)
{
    // return all targets on missile trajectory (0 - size of a missile)
    if (!_caster->HasInLine(target, 0))
        return false;
    return WorldObjectSpellAreaTargetCheck::operator ()(target);
}

} //namespace Trinity
