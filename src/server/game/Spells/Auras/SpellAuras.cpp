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

#include "AreaTriggerData.h"
#include "AreaTriggerAI.h"
#include "CellImpl.h"
#include "Common.h"
#include "DynamicObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellPackets.h"
#include "SpellScript.h"
#include "Unit.h"
#include "Util.h"
#include "Vehicle.h"

AuraApplication::AuraApplication(Unit* target, Unit* caster, Aura* aura, uint32 effMask) : _target(target), _base(aura), _removeMode(AURA_REMOVE_NONE), _slot(MAX_AURAS), _flags(AFLAG_NONE), _effectMask(NULL), _effectsToApply(effMask), _needClientUpdate(false)
{
    ASSERT(GetTarget() && GetBase());

    uint8 slot = 0;
    for (AuraApplication* visibleAura : GetTarget()->GetVisibleAuras())
    {
        if (slot < visibleAura->GetSlot())
            break;

        ++slot;
    }

    if (slot < MAX_AURAS)
    {
        _slot = slot;
        GetTarget()->SetVisibleAura(this);
        _needClientUpdate = true;
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura: %u EffectMask: %d put to unit visible auras slot: %u", GetBase()->GetId(), effMask, slot);
    }
    else
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura: %u EffectMask: %d could not find empty unit visible slot", GetBase()->GetId(), effMask);

    _InitFlags(caster, effMask);
}

void AuraApplication::_Remove()
{
    if (GetSlot() < MAX_AURAS)
    {
        GetTarget()->RemoveVisibleAura(this);
        ClientUpdate(true);
    }
}

void AuraApplication::_InitFlags(Unit* caster, uint32 effMask)
{
    Aura const* aura = GetBase();
    _flags |= (aura->GetCasterGUID() == GetTarget()->GetGUID()) ? AFLAG_NOCASTER : AFLAG_NONE;

    if (IsSelfcasted() || !caster || !caster->IsFriendlyTo(GetTarget())) // aura is casted by self or an enemy one negative effect and we know aura is negative
    {
        bool negativeFound = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
                break;

            if (((1<<i) & effMask) && !aura->GetSpellInfo()->IsPositiveEffect(i, IsSelfcasted()))
            {
                negativeFound = true;
                break;
            }
        }

        _flags |= negativeFound ? AFLAG_NEGATIVE : AFLAG_POSITIVE;
    }
    else // aura is casted by friend one positive effect and we know aura is positive
    {
        bool positiveFound = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
                break;

            if (((1<<i) & effMask) && aura->GetSpellInfo()->IsPositiveEffect(i))
            {
                positiveFound = true;
                break;
            }
        }
        _flags |= positiveFound ? AFLAG_POSITIVE : AFLAG_NEGATIVE;
    }

    if (aura->GetSpellInfo()->HasAttribute(SPELL_ATTR8_AURA_SEND_AMOUNT) || aura->HasEffectType(SPELL_AURA_MOD_SPELL_CATEGORY_COOLDOWN) || aura->HasEffectType(SPELL_AURA_MOD_MAX_CHARGES) ||
        aura->GetSpellInfo()->HasAura(SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS) || aura->HasEffectType(SPELL_AURA_CHARGE_RECOVERY_MOD) || aura->HasEffectType(SPELL_AURA_CHARGE_RECOVERY_MULTIPLIER)
        || aura->GetSpellInfo()->HasAura(SPELL_AURA_MOD_COOLDOWN_PCT) || aura->GetSpellInfo()->HasAura(SPELL_AURA_MOD_VISIBILITY_RANGE)
        )
        _flags |= AFLAG_SCALABLE;
}

void AuraApplication::_HandleEffect(uint8 effIndex, bool apply)
{
    AuraEffect* aurEff = GetBase()->GetEffect(effIndex);
    // ASSERT(aurEff);
    if (!aurEff)
        return;

    if(HasEffect(effIndex) != (!apply))
        return;
    ASSERT((1<<effIndex) & _effectsToApply);
    #ifdef WIN32
    TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "AuraApplication::_HandleEffect: GetId %i, GetAuraType %u, apply: %u: amount: %i, m_send_baseAmount: %i, effIndex: %i GetDuration %i, guid %u GetStackAmount %u GetComboPoints %u GetCharges %u",
    GetBase()->GetId(), aurEff->GetAuraType(), apply, aurEff->GetAmount(), aurEff->GetBaseSendAmount(), effIndex, GetBase()->GetDuration(), GetBase()->GetOwner()->GetGUIDLow(), GetBase()->GetStackAmount(), GetBase()->GetComboPoints(), GetBase()->GetCharges());
    #endif

    if (apply)
    {
        ASSERT(!(_effectMask & (1<<effIndex)));
        _effectMask |= 1<<effIndex;
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, true);
    }
    else
    {
        if (GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            switch (GetBase()->GetSpellInfo()->GetEffect(effIndex, GetBase()->GetSpawnMode())->ApplyAuraName)
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                {
                    aurEff->HandlePeriodicDamageAurasTick(GetTarget(), GetBase()->GetCaster(), SpellEffIndex(effIndex), true);
                    break;
                }
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_OBS_MOD_HEALTH:
                {
                    aurEff->HandlePeriodicHealAurasTick(GetTarget(), GetBase()->GetCaster(), SpellEffIndex(effIndex), true);
                    break;
                }
                default:
                    break;
            }
        }
        ASSERT(_effectMask & (1<<effIndex));
        _effectMask &= ~(1<<effIndex);
        aurEff->HandleEffect(this, AURA_EFFECT_HANDLE_REAL, false);

        // Remove all triggered by aura spells vs unlimited duration
        aurEff->CleanupTriggeredSpells(GetTarget());
    }
    SetNeedClientUpdate();
}

void AuraApplication::SetNeedClientUpdate()
{
    if (IsNeedClientUpdate() || GetRemoveMode() != AURA_REMOVE_NONE)
        return;

    _needClientUpdate = true;
    _target->SetVisibleAuraUpdate(this);
}

void AuraApplication::BuildUpdatePacket(WorldPackets::Spells::AuraInfo& auraInfo, bool remove, uint32 overrideAura /*= 0*/)
{
    //ASSERT(_target->HasVisibleAura(this) != remove);

    auraInfo.Slot = GetSlot();
    if (remove)
        return;

    Aura const* aura = GetBase();
    if (!aura)
        return;

    WorldPackets::Spells::AuraDataInfo auraData;
    auraData.CastGuid = ObjectGuid::Create<HighGuid::Cast>(GetTarget()->GetMapId(), (overrideAura ? overrideAura : aura->GetId()), sObjectMgr->GetGenerator<HighGuid::Cast>()->Generate(), SPELL_CAST_TYPE_NORMAL);
    auraData.SpellID = overrideAura ? overrideAura : aura->GetId();
    auraData.SpellXSpellVisualID = aura->GetSpellVisual();
    auraData.Flags = GetFlags();
    if (aura->GetMaxDuration() > 0 && !(aura->GetSpellInfo()->HasAttribute(SPELL_ATTR5_HIDE_DURATION)))
        auraData.Flags |= AFLAG_DURATION;

    // if (auraData.Flags & AFLAG_SCALABLE)
    auraData.ActiveFlags = GetEffectMask();
    // else
        // auraData.ActiveFlags = 1;
    auraData.CastLevel = aura->GetCasterLevel();
    if (aura->TimeMod != 1.0f)
        auraData.TimeMod = aura->TimeMod;

    // send stack amount for aura which could be stacked (never 0 - causes incorrect display) or charges
    // stack amount has priority over charges (checked on retail with spell 50262)
    uint8 m_diffMode = GetTarget() ? GetTarget()->GetMapId() ? GetTarget()->GetMap()->GetDifficultyID() : 0 : 0;
    auraData.Applications = uint8((aura->GetStackAmount() > 1 || !aura->GetSpellInfo()->GetAuraOptions(m_diffMode)->ProcCharges) ? ((!aura->GetStackAmount() && aura->GetCharges()) ? aura->GetCharges() : aura->GetStackAmount()) : aura->GetCharges());
    if (!(auraData.Flags & AFLAG_NOCASTER))
        auraData.CastUnit = aura->GetCasterGUID();

    if (auraData.Flags & AFLAG_DURATION)
    {
        auraData.Duration = aura->GetMaxDuration();
        auraData.Remaining = aura->GetDuration();
    }

    WorldPackets::Spells::SandboxScalingData sandboxScalingData;
    if (sandboxScalingData.GenerateDataForUnits(ObjectAccessor::GetUnit(*GetTarget(), aura->GetCasterGUID()), GetTarget()))
        auraData.SandboxScaling = sandboxScalingData;

    if (auraData.Flags & AFLAG_SCALABLE)
    {
        bool sendEffect = false;
        bool nosendEffect = false;
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
                break;

            if (AuraEffect const* effect = aura->GetEffect(i))
            {
                switch (effect->GetAuraType())
                {
                    case SPELL_AURA_DUMMY:
                    case SPELL_AURA_SCHOOL_ABSORB:
                    case SPELL_AURA_SCHOOL_HEAL_ABSORB:
                    case SPELL_AURA_OVERRIDE_ACTIONBAR_SPELLS:
                    case SPELL_AURA_ADD_FLAT_MODIFIER:
                    case SPELL_AURA_ADD_PCT_MODIFIER:
                        nosendEffect = true;
                        break;
                    case SPELL_AURA_PERIODIC_DAMAGE:
                    case SPELL_AURA_PERIODIC_HEAL:
                        sendEffect = true;
                        break;
                    default:
                        break;
                }
            }
        }

        uint32 count = 0;
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
                break;

            if (aura->GetSpellInfo()->GetEffect(i, aura->GetSpawnMode())->IsEffect())
            {
                ++count;
                if (!(_effectsToApply & (1 << i)))
                {
                    if(sendEffect && !nosendEffect)
                        auraData.Points.push_back(0.0f);
                    auraData.EstimatedPoints.push_back(0.0f);
                    continue;
                }

                if (AuraEffect const* effect = aura->GetEffect(i))
                {
                    if(sendEffect && !nosendEffect)
                    {
                        auraData.EstimatedPoints.push_back(effect->GetBaseSendAmount());
                        auraData.Points.push_back(effect->GetAmount());
                    }
                    else
                        auraData.EstimatedPoints.push_back(effect->GetAmount());
                }
                else
                {
                    if(sendEffect && !nosendEffect)
                        auraData.Points.push_back(0.0f);
                    auraData.EstimatedPoints.push_back(0.0f);
                }
            }
        }
    }

    auraInfo.AuraData = auraData;
}

void AuraApplication::BuildLossOfControlPacket(WorldPackets::Spells::LossOfControlAuraUpdate& loosOfControl)
{
    Aura const* aura = GetBase();
    if (aura->GetDuration() < 0 || aura->GetCasterGUID() == aura->GetOwner()->GetGUID())
        return;

    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (aura->GetSpellInfo()->EffectMask < uint32(1 << i))
            break;

        AuraEffect const* effect = aura->GetEffect(i);
        if (!effect)
            continue;

        auto addLoosOfControlInfo = [i, this, aura, &loosOfControl](uint32 type)->void
        {
            WorldPackets::Spells::LossOfControlInfo loosOfControlInfo;
            loosOfControlInfo.Mechanic = aura->GetSpellInfo()->GetEffectMechanic(i);
            loosOfControlInfo.Type = type;
            loosOfControlInfo.AuraSlot = GetSlot();
            loosOfControlInfo.EffectIndex = i;
            loosOfControl.Infos.push_back(loosOfControlInfo);
        };

        switch (effect->GetAuraType())
        {
        case SPELL_AURA_MOD_DISARM:
        case SPELL_AURA_MOD_DISARM_OFFHAND:
        case SPELL_AURA_MOD_DISARM_RANGED:
            addLoosOfControlInfo(LOC_DISARM);
            break;
        case SPELL_AURA_MOD_SILENCE:
            addLoosOfControlInfo(LOC_SILENCE);
            break;
        case SPELL_AURA_MOD_PACIFY:
            addLoosOfControlInfo(LOC_PACIFY);
            break;
        case SPELL_AURA_MOD_PACIFY_SILENCE:
            addLoosOfControlInfo(LOC_PACIFYSILENCE);
            break;
        case SPELL_AURA_MOD_CONFUSE:
            addLoosOfControlInfo(LOC_CONFUSE);
            break;
        case SPELL_AURA_MOD_FEAR:
        case SPELL_AURA_MOD_FEAR_2:
            addLoosOfControlInfo(LOC_FEAR);
            break;
        case SPELL_AURA_MOD_STUN:
            addLoosOfControlInfo(LOC_STUN_MECHANIC);
            break;
        case SPELL_AURA_MOD_ROOT:
        case SPELL_AURA_MOD_ROOTED:
            addLoosOfControlInfo(LOC_ROOT);
            break;
        case SPELL_AURA_MOD_POSSESS:
            addLoosOfControlInfo(LOC_POSSESS);
            break;
        case SPELL_AURA_MOD_CHARM:
            addLoosOfControlInfo(LOC_CHARM);
            break;
        default:
            break;
        }
    }
}

void AuraApplication::ClientUpdate(bool remove)
{
    if (!GetTarget())
        return;

    _needClientUpdate = false;

    WorldPackets::Spells::AuraUpdate update;
    update.UpdateAll = false;
    update.UnitGUID = GetTarget()->GetGUID();

    WorldPackets::Spells::AuraInfo auraInfo;
    BuildUpdatePacket(auraInfo, remove);
    update.Auras.push_back(auraInfo);

    _target->SendMessageToSet(update.Write(), true);
    if (!remove && _target->IsPlayer())
    {
        WorldPackets::Spells::LossOfControlAuraUpdate lossOfControl;
        lossOfControl.TargetGuid = _target->GetGUID();
        BuildLossOfControlPacket(lossOfControl);
        if (!lossOfControl.Infos.empty())
        {
            Unit* target = _target;
            WorldPacket data = *lossOfControl.Write();
            _target->AddDelayedEvent(10, [target, data]() -> void { if (target) target->ToPlayer()->SendDirectMessage(&data); });
        }
    }
}

uint32 Aura::BuildEffectMaskForOwner(SpellInfo const* spellProto, uint32 avalibleEffectMask, WorldObject* owner)
{
    ASSERT(spellProto);
    ASSERT(owner);
    uint32 effMask = 0;

    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
        {
            if (owner->ToUnit()->IsImmunedToSpell(spellProto))
                return 0;

            for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
            {
                if (spellProto->EffectMask < uint32(1 << i))
                    break;

                if (spellProto->GetEffect(i, owner->GetSpawnMode())->IsUnitOwnedAuraEffect())
                    effMask |= 1 << i;
            }
            break;
        }
        case TYPEID_DYNAMICOBJECT:
            for (uint8 i = 0; i< MAX_SPELL_EFFECTS; ++i)
            {
                if (spellProto->EffectMask < uint32(1 << i))
                    break;

                if (spellProto->GetEffect(i, owner->GetSpawnMode())->Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                    effMask |= 1 << i;
            }
            break;
        default:
            break;
    }
    return effMask & avalibleEffectMask;
}

Aura* Aura::TryRefreshStackOrCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, float* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, ObjectGuid casterGUID /*= 0*/, bool* refresh /*= NULL*/, uint16 stackAmount, Spell* spell)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || !casterGUID.IsEmpty());
    ASSERT(tryEffMask <= MAX_EFFECT_MASK);
    if (refresh)
        *refresh = false;

    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);

    if (caster)
    {
        effMask = CalculateEffMaskFromDummy(caster, owner, effMask, spellproto);

        if (spell)
            spell->CallScriptCalculateEffMaskHandlers(effMask);
    }

    if (!effMask)
        return nullptr;

    Aura* foundAura = owner->ToUnit()->_TryStackingOrRefreshingExistingAura(spellproto, effMask, caster, baseAmount, castItem, casterGUID);
    if (foundAura != nullptr && !stackAmount)
    {
        // we've here aura, which script triggered removal after modding stack amount
        // check the state here, so we won't create new Aura object
        if (foundAura->IsRemoved())
            return nullptr;

        if (refresh)
            *refresh = true;

        int32 curDur = foundAura->GetDuration();
        int32 maxDur = foundAura->CalcMaxDuration(caster);

        // try to increase stack amount
        foundAura->ModStackAmount(1);

        if (spell)
        {
            foundAura->SetPowerCost(spell->m_powerCost);
            if (spellproto->IsRefreshTimers())
            {
                if (spellproto->HasAttribute(SPELL_ATTR13_PANDEMIA) && foundAura->GetDuration() != -1) // Not sure
                {
                    if (curDur >= (maxDur * 0.3))
                        maxDur *= 1.3;
                    else
                        maxDur += curDur;
                }

                foundAura->SetAuraAttribute(AURA_ATTR_REAPPLIED_AURA);
                foundAura->SetMaxDuration(maxDur);
                foundAura->SetDuration(maxDur);
            }
        }

        return foundAura;
    }
    return Create(spellproto, effMask, owner, caster, baseAmount, castItem, casterGUID, stackAmount, spell);
}

Aura* Aura::TryCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, float* baseAmount /*= NULL*/, Item* castItem /*= NULL*/, ObjectGuid casterGUID /*= 0*/)
{
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || !casterGUID.IsEmpty());
    ASSERT(tryEffMask <= MAX_EFFECT_MASK);
    uint32 effMask = Aura::BuildEffectMaskForOwner(spellproto, tryEffMask, owner);
    if(caster)
        effMask = CalculateEffMaskFromDummy(caster, owner, effMask, spellproto);
    if (!effMask)
        return nullptr;
    return Create(spellproto, effMask, owner, caster, baseAmount, castItem, casterGUID);
}

Aura* Aura::Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float* baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, Spell* spell)
{
    ASSERT(effMask);
    ASSERT(spellproto);
    ASSERT(owner);
    ASSERT(caster || !casterGUID.IsEmpty());
    ASSERT(effMask <= MAX_EFFECT_MASK);
    // try to get caster of aura
    if (!casterGUID.IsEmpty())
    {
        if (owner->GetGUID() == casterGUID)
            caster = owner->ToUnit();
        else
            caster = ObjectAccessor::GetUnit(*owner, casterGUID);
    }
    else
        casterGUID = caster->GetGUID();

    // check if aura can be owned by owner
    if (owner->IsUnit())
        if (!owner->IsInWorld() || owner->ToUnit()->IsDuringRemoveFromWorld())
            // owner not in world so don't allow to own not self casted single target auras
            if (casterGUID != owner->GetGUID() && (spellproto->IsSingleTarget(caster, owner->ToUnit()) || spellproto->IsMultiSingleTarget()))
                return nullptr;

    Aura* aura = nullptr;
//     if(spellproto->IsSingleTarget(caster, owner->ToUnit()))
//     {
//         if (caster)
//         {
//             Unit::AuraList& scAuras = caster->GetSingleCastAuras();
//             for (auto itr = scAuras.begin(); itr != scAuras.end();)
//             {
//                 if ((*itr)->GetId() == spellproto->Id)
//                 {
//                     //test code
//                     TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura* Aura::Create aura %u, GetCasterGUID %u", (*itr)->GetId(), caster->GetGUID());
//                     Aura::ApplicationMap const& appMap = (*itr)->GetApplicationMap();
//                     for (Aura::ApplicationMap::const_iterator app = appMap.begin(); app!= appMap.end();)
//                     {
//                         AuraApplication * aurApp = app->second;
//                         ++app;
//                         Unit* target = aurApp->GetTarget();
//                         aurApp->SetRemoveMode(AURA_REMOVE_BY_DEFAULT);
//                         (*itr)->_UnapplyForTarget(target, caster, aurApp);
//                         (*itr)->ChangeOwner(owner);
//                     }
//                     return (*itr);
//                     stackAmount = (*itr)->GetStackAmount();
//                 }
//                 ++itr;
//             }
//         }
//     }

    switch (owner->GetTypeId())
    {
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            aura = new UnitAura(spellproto, effMask, owner, caster, baseAmount, castItem, casterGUID, stackAmount, spell ? &spell->m_powerCost : nullptr);
            if(spell)
                aura->m_damage_amount = spell->GetDamage();
            aura->_InitEffects(effMask, caster, baseAmount);
            aura->GetUnitOwner()->_AddAura(static_cast<UnitAura*>(aura), caster);
            break;
        case TYPEID_DYNAMICOBJECT:
            aura = new DynObjAura(spellproto, effMask, owner, caster, baseAmount, castItem, casterGUID, stackAmount, spell ? &spell->m_powerCost : nullptr);
            aura->_InitEffects(effMask, caster, baseAmount);

            // ASSERT(aura->GetDynobjOwner());
            // ASSERT(aura->GetDynobjOwner()->IsInWorld());
            if (!aura->GetDynobjOwner() || !aura->GetDynobjOwner()->IsInWorld())
            {
                delete aura;
                return nullptr;
            }
            aura->GetDynobjOwner()->SetAura(aura);
            ASSERT(aura->GetDynobjOwner()->GetMap() == aura->GetCaster()->GetMap());
            break;
        default:
            ASSERT(false);
            // ReSharper disable once CppUnreachableCode
            return nullptr;
    }
    // aura can be removed in Unit::_AddAura call
    if (aura->IsRemoved())
        return nullptr;

    if (!spell)
        aura->SetSpellVisual(spellproto->GetSpellXSpellVisualId(caster, owner->ToUnit()));

    return aura;
}

uint32 Aura::CalculateEffMaskFromDummy(Unit* caster, WorldObject* target, uint32 effMask, SpellInfo const* spellproto)
{
    if (auto spellAuraDummy = sSpellMgr->GetSpellAuraDummy(spellproto->Id))
    {
        for (const auto& itr : *spellAuraDummy)
        {
            Unit* _caster = caster;
            Unit* _targetAura = caster;
            bool check = false;

            if (!_caster)
                return effMask;

            if (itr.targetaura == 1 && _caster->ToPlayer()) //get target pet
            {
                if (Pet* pet = _caster->ToPlayer()->GetPet())
                    _targetAura = static_cast<Unit*>(pet);
            }
            if (itr.targetaura == 2) //get target owner
            {
                if (Unit* owner = _caster->GetOwner())
                    _targetAura = owner;
            }
            if (itr.targetaura == 3 && target->ToUnit()) //get target
                _targetAura = target->ToUnit();

            if (!_targetAura)
                _targetAura = _caster;

            if (itr.hastalent)
                if (_caster->HasAuraLinkedSpell(_caster, target->ToUnit(), itr.hastype, itr.hastalent, itr.hasparam))
                    continue;

            if (itr.hastalent2)
                if (_caster->HasAuraLinkedSpell(_caster, target->ToUnit(), itr.hastype2, itr.hastalent2, itr.hasparam2))
                    continue;

            switch (itr.option)
            {
            case SPELL_DUMMY_MOD_EFFECT_MASK: //4
            {
                if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                    continue;
                if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                    continue;

                if (itr.spellDummyId > 0 && !_targetAura->HasAura(itr.spellDummyId))
                {
                    effMask &= ~itr.effectmask;
                    check = true;
                }
                if (itr.spellDummyId < 0 && _targetAura->HasAura(abs(itr.spellDummyId)))
                {
                    effMask &= ~itr.effectmask;
                    check = true;
                }
                break;
            }
            default:
                break;
            }
            if (check && itr.removeAura)
                _caster->RemoveAurasDueToSpell(itr.removeAura);
        }
    }

    return effMask;
}

void Aura::CalculateDurationFromDummy(int32 &duration)
{
    Unit* _caster = GetCaster();
    if (!_caster)
        return;

    auto spellAuraDummy = sSpellMgr->GetSpellAuraDummy(GetId());
    if (!spellAuraDummy)
        return;

    for (const auto& itr : *spellAuraDummy)
    {
        if (itr.type != SPELL_DUMMY_DURATION)
            continue;

        Unit* _targetAura = _caster;
        bool check = false;

        if (itr.targetaura == 1 && _caster->ToPlayer()) //get target pet
        {
            if (Pet* pet = _caster->ToPlayer()->GetPet())
                _targetAura = pet->ToUnit();
        }
        if (itr.targetaura == 2) //get target owner
        {
            if (Unit* owner = _caster->GetOwner())
                _targetAura = owner;
        }
        if (itr.targetaura == 3 && m_owner->ToUnit()) //get target
            _targetAura = m_owner->ToUnit();

        if (!_targetAura)
            _targetAura = _caster;

        if (itr.hastalent)
            if (_caster->HasAuraLinkedSpell(_caster, _targetAura, itr.hastype, itr.hastalent, itr.hasparam))
                continue;

        if (itr.hastalent2)
            if (_caster->HasAuraLinkedSpell(_caster, _targetAura, itr.hastype2, itr.hastalent2, itr.hasparam2))
                continue;

        switch (itr.option)
        {
        case SPELL_DUMMY_DURATION_ADD_PERC: //11
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0 && _caster->HasAura(itr.spellDummyId))
            {
                if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(itr.spellDummyId))
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    duration += CalculatePct(duration, bp);
                    check = true;
                }
            }
            if (itr.spellDummyId < 0 && _caster->HasAura(abs(itr.spellDummyId)))
            {
                if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(abs(itr.spellDummyId)))
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    duration -= CalculatePct(duration, bp);
                    check = true;
                }
            }
            break;
        }
        case SPELL_DUMMY_DURATION_ADD_VALUE: //12
        {
            if (itr.aura > 0 && !_targetAura->HasAura(itr.aura))
                continue;
            if (itr.aura < 0 && _targetAura->HasAura(abs(itr.aura)))
                continue;

            if (itr.spellDummyId > 0 && _caster->HasAura(itr.spellDummyId))
            {
                if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(itr.spellDummyId))
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    duration += bp;
                    check = true;
                }
            }
            if (itr.spellDummyId < 0 && _caster->HasAura(abs(itr.spellDummyId)))
            {
                if (SpellInfo const* dummyInfo = sSpellMgr->GetSpellInfo(abs(itr.spellDummyId)))
                {
                    float bp = itr.custombp;
                    if (!bp)
                        bp = dummyInfo->Effects[itr.effectDummy]->BasePoints;
                    duration -= bp;
                    check = true;
                }
            }
            break;
        }
        default:
            break;
        }
        if (check && itr.removeAura)
            _caster->RemoveAurasDueToSpell(itr.removeAura);
    }
}

Aura::Aura(SpellInfo const* spellproto, WorldObject* owner, Unit* caster, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, SpellPowerCost* powerCost) :
m_damage_amount(0), TimeMod(1.0f), m_spellInfo(spellproto), m_casterGuid(!casterGUID.IsEmpty() ? casterGUID : caster->GetGUID()), m_castItemGuid(castItem ? castItem->GetGUID() : ObjectGuid::Empty),
m_applyTime(time(nullptr)), m_applyMSTime(getMSTime()), m_owner(owner), m_SpellVisual(0), m_timeCla(0), m_updateTargetMapInterval(0), m_casterLevel(caster ? caster->getLevelForTarget(owner) : m_spellInfo->SpellLevel),
m_procCharges(0), m_stackAmount(stackAmount ? stackAmount: 1), m_diffMode(caster ? caster->GetSpawnMode() : 0), m_customData(0), m_isRemoved(0), m_auraId(spellproto->Id), _triggeredCastFlags(0)
{
    m_powerCost.assign(MAX_POWERS + 1, 0);
    if(powerCost)
    {
        for (uint32 i = 0; i < MAX_POWERS; ++i)
            m_powerCost[i] = (*powerCost)[i];
    }

    auraAttributes = 0;

    if (!GetSpellInfo()->NoPower())
    {
        if (!GetSpellInfo()->GetSpellPowerByCasterPower(GetCaster(), powerData))
            if (SpellPowerEntry const* power = GetSpellInfo()->GetPowerInfo(0))
                powerData.push_back(power);

        bool findPercCost = false;
        for (SpellPowerEntry const* power : powerData)
        {
            if (power->ManaPerSecond || power->PowerPctPerSecond)
                findPercCost = true;
        }
        if (!findPercCost)
            powerData.clear();
    }

    LoadScripts();

    m_maxDuration = CalcMaxDuration(caster);
    m_duration = m_maxDuration;
    m_allDuration = 0;
    m_procCharges = CalcMaxCharges(caster);
    SetAuraAttribute(AURA_ATTR_IS_USING_CHARGES, m_procCharges != 0);
    int32 _level = 0;
    if (caster)
        _level = caster->GetEffectiveLevel();

    if (m_owner && (m_spellInfo->GetSpellSpecific() == SPELL_SPECIFIC_DRINK || m_spellInfo->GetSpellSpecific() == SPELL_SPECIFIC_FOOD_AND_DRINK))
    {
        if (Player* plr = m_owner->ToPlayer())
        {
            if (plr->HasPvpStatsScalingEnabled())
            {
                if (m_maxDuration > 5000)
                    SetAuraAttribute(AURA_ATTR_DRINK_ARENA_DELAY);
            }
        }
    }

    if (castItem && castItem->IsEquipable())
        SetAuraAttribute(AURA_ATTR_FROM_EQUIPABLE_ITEM);

    //For scaling trinket
    if((m_spellInfo->HasAttribute(SPELL_ATTR11_SEND_ITEM_LEVEL)) && castItem)
        m_casterLevel = castItem->GetItemLevel(_level);

    if (m_spellInfo->Scaling.ScalesFromItemLevel && castItem)
        m_casterLevel = castItem->GetItemLevel(_level);

    if (m_spellInfo->Scaling.MaxScalingLevel && caster && caster->getLevelForTarget(m_owner) > m_spellInfo->Scaling.MaxScalingLevel)
        m_casterLevel = m_spellInfo->Scaling.MaxScalingLevel;

    //For info fishing
    if (caster && caster->IsPlayer())
        if(m_spellInfo->HasAttribute(SPELL_ATTR1_IS_FISHING))
            m_casterLevel = caster->ToPlayer()->GetSkillTempBonusValue(SKILL_FISHING);

    if (GetId() == 192081) // Ironfur
    {
        durationByStack.resize(m_stackAmount);
        durationByStack[m_stackAmount - 1] = m_maxDuration;
    }
}

void Aura::_InitEffects(uint32 effMask, Unit* caster, float *baseAmount)
{
    // shouldn't be in constructor - functions in AuraEffect::AuraEffect use polymorphism
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (effMask & (uint8(1) << i))
        {
            m_effects[i] = new AuraEffect(this, i, baseAmount ? baseAmount + i : nullptr, caster, m_diffMode);

            m_effects[i]->CalculatePeriodic(caster, true, false);
            m_effects[i]->SetAmount(m_effects[i]->CalculateAmount(caster));
            m_effects[i]->CalculateSpellMod();
        }
        else
            m_effects[i] = nullptr;
    }
    UpdateConcatenateAura(caster, 0, 0, CONCATENATE_ON_APPLY_AURA);
}

Aura::~Aura()
{
    volatile uint32 _auraId = m_auraId;

    // unload scripts
    while (!m_loadedScripts.empty())
    {
        auto itr = m_loadedScripts.begin();
        (*itr)->_Unload();
        delete (*itr);
        m_loadedScripts.erase(itr);
    }

    // free effects memory
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
         delete m_effects[i];

    _DeleteRemovedApplications();
}

Unit* Aura::GetCaster() const
{
    if (!GetOwner())
        return nullptr;
    if (GetOwner()->GetGUID() == GetCasterGUID())
        return GetUnitOwner();
    if (AuraApplication const* aurApp = const_cast<Aura*>(this)->GetApplicationOfTarget(GetCasterGUID()))
        return aurApp->GetTarget();

    return ObjectAccessor::GetUnit(*GetOwner(), GetCasterGUID());
}

AuraObjectType Aura::GetType() const
{
    return (m_owner->IsDynObject()) ? DYNOBJ_AURA_TYPE : UNIT_AURA_TYPE;
}

void Aura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr auraApp)
{
    if (IsRemoved())
        return;

    ASSERT(target);
    ASSERT(auraApp);
    // aura mustn't be already applied on target
    // ASSERT (!IsAppliedOnTarget(target->GetGUID()) && "Aura::_ApplyForTarget: aura musn't be already applied on target");
    if (IsAppliedOnTarget(target->GetGUID()))
        return;

    m_applications.insert(target->GetGUID(), auraApp);

    // set infinity cooldown state for spells
    if (caster && caster->IsPlayer())
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE))
        {
            Item* castItem = m_castItemGuid ? caster->ToPlayer()->GetItemByGuid(m_castItemGuid) : nullptr;
            caster->ToPlayer()->AddSpellAndCategoryCooldowns(m_spellInfo, castItem ? castItem->GetEntry() : 0, nullptr, true);
        }
    }

    if (target)
        if (Creature* creature = target->ToCreature())
            if (creature->IsAIEnabled)
                creature->AI()->OnApplyOrRemoveAura(auraApp->GetBase()->GetId(), auraApp->GetRemoveMode(), true);
}

void Aura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr auraApp)
{
    m_applications.erase(target->GetGUID());
    // m_applications.update(target->GetGUID(), [](ApplicationMap::value_type& item, ApplicationMap::value_type* old) {item.second = nullptr; if (old) old->second = nullptr; });
    m_removedApplications.push_back(auraApp);

    // reset cooldown state for spells
    if (caster && caster->IsPlayer())
    {
        if (GetSpellInfo()->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE))
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existed cases)
            caster->ToPlayer()->SendCooldownEvent(GetSpellInfo());
    }

    if (Creature* creature = target->ToCreature())
        if (creature->IsAIEnabled)
            creature->AI()->OnApplyOrRemoveAura(auraApp->GetBase()->GetId(), auraApp->GetRemoveMode(), false);
}

// removes aura from all targets
// and marks aura as removed
void Aura::_Remove(AuraRemoveMode removeMode)
{
    ASSERT (!m_isRemoved);
    m_isRemoved = 1;

    for (ApplicationMap::iterator appItr = m_applications.begin(); appItr != m_applications.end(); ++appItr)
    {
        if (appItr == m_applications.end()) // Duble check
            break;

        if (AuraApplicationPtr aurApp = appItr->second)
        {
            if (Unit* target = aurApp->GetTarget())
                target->_UnapplyAura(aurApp.get(), removeMode);
        }
    }

    UpdateConcatenateAura(GetCaster(), 0, 0, CONCATENATE_ON_REMOVE_AURA);
}

Aura::ApplicationMap const& Aura::GetApplicationMap()
{
    return m_applications;
}

AuraApplication* Aura::GetApplicationOfTarget(ObjectGuid const& guid)
{
    if (auto ptr = m_applications.get(guid))
        return ptr->second.get();

    return nullptr;
}

bool Aura::IsAppliedOnTarget(ObjectGuid const& guid)
{
    if (auto ptr = m_applications.get(guid))
        return ptr->second != nullptr;

    return false;
}

void Aura::UpdateTargetMap(Unit* caster, bool apply)
{
    if (!this || IsRemoved())
        return;

    m_updateTargetMapInterval = UPDATE_TARGET_MAP_INTERVAL;
    volatile uint32 auraId = GetId();

    // fill up to date target list
    //       target, effMask
    std::map<Unit*, uint32> targets;

    FillTargetMap(targets, caster);

    UnitList targetsToRemove;

    // mark all auras as ready to remove
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if (appIter == m_applications.end())
            return;

        AuraApplicationPtr aurApp = appIter->second;
        if (!aurApp)
        {
            m_applications.erase_at(appIter);
            continue;
        }

        auto existing = targets.find(aurApp->GetTarget());
        // not found in current area - remove the aura
        if (existing == targets.end())
            targetsToRemove.push_back(aurApp->GetTarget());
        else
        {
            // needs readding - remove now, will be applied in next update cycle
            // (dbcs do not have auras which apply on same type of targets but have different radius, so this is not really needed)
            if (aurApp->GetEffectMask() != existing->second || !CanBeAppliedOn(existing->first))
                targetsToRemove.push_back(aurApp->GetTarget());
            // nothing todo - aura already applied
            // remove from auras to register list
            targets.erase(existing);
        }
    }

    // register auras for units
    for (auto itr = targets.begin(); itr!= targets.end();)
    {
        // aura mustn't be already applied on target
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // the core created 2 different units with same guid
            // this is a major failue, which i can't fix right now
            // let's remove one unit from aura list
            // this may cause area aura "bouncing" between 2 units after each update
            // but because we know the reason of a crash we can remove the assertion for now
            if (aurApp->GetTarget() != itr->first)
            {
                // remove from auras to register list
                targets.erase(itr++);
                continue;
            }
            // ok, we have one unit twice in target map (impossible, but...)
            ASSERT(false);
        }

        bool addUnit = true;

        if (caster != itr->first)
        {
            // check target immunities
            for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
            {
                if (m_spellInfo->EffectMask < uint32(1 << effIndex))
                    break;

                if (itr->first->IsImmunedToSpellEffect(GetSpellInfo(), effIndex))
                    itr->second &= ~(1 << effIndex);
            }
        }
        if (!itr->second
            || itr->first->IsImmunedToSpell(GetSpellInfo())
            || !CanBeAppliedOn(itr->first))
            addUnit = false;

        if (addUnit)
        {
            // persistent area aura does not hit flying targets
            if (GetType() == DYNOBJ_AURA_TYPE)
            {
                if (itr->first->isInFlight())
                    addUnit = false;
            }
            // unit auras can not stack with each other
            else // (GetType() == UNIT_AURA_TYPE)
            {
                // Allow to remove by stack when aura is going to be applied on owner
                if (itr->first != GetOwner())
                {
                    // check if not stacking aura already on target
                    // this one prevents unwanted usefull buff loss because of stacking and prevents overriding auras periodicaly by 2 near area aura owners
                    for (auto& iter : itr->first->GetAppliedAuras())
                    {
                        if (!CanStackWith(iter.second->GetBase()))
                        {
                            addUnit = false;
                            break;
                        }
                    }
                }
            }
        }
        if (!addUnit)
            targets.erase(itr++);
        else
        {
            // owner has to be in world, or effect has to be applied to self
            if (!GetOwner()->IsSelfOrInSameMap(itr->first))
            {
                //TODO: There is a crash caused by shadowfiend load addon
                TC_LOG_FATAL(LOG_FILTER_SPELLS_AURAS, "Aura %u: Owner %s (map %u) is not in the same map as target %s (map %u).", GetSpellInfo()->Id,
                    GetOwner()->GetName(), GetOwner()->IsInWorld() ? GetOwner()->GetMap()->GetId() : uint32(-1),
                    itr->first->GetName(), itr->first->IsInWorld() ? itr->first->GetMap()->GetId() : uint32(-1));
                // ASSERT(false); // Crash when load in world
                targets.erase(itr++);
                continue;
            }
            itr->first->_CreateAuraApplication(this, itr->second);
            ++itr;
        }
    }

    // remove auras from units no longer needing them
    for (auto unit : targetsToRemove)
        if(unit)
            if (AuraApplication * aurApp = GetApplicationOfTarget(unit->GetGUID()))
                unit->_UnapplyAura(aurApp, AURA_REMOVE_BY_DEFAULT);

    if (!apply)
        return;

    // apply aura effects for units
    for (auto itr = targets.begin(); itr!= targets.end();++itr)
    {
        if (AuraApplication * aurApp = GetApplicationOfTarget(itr->first->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == itr->first) || GetOwner()->IsInMap(itr->first));
            itr->first->_ApplyAura(aurApp, itr->second);
        }
    }
}

// targets have to be registered and not have effect applied yet to use this function
void Aura::_ApplyEffectForTargets(uint8 effIndex)
{
    // prepare list of aura targets
    UnitList targetList;
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if (AuraApplicationPtr aurApp = appIter->second)
            if ((aurApp->GetEffectsToApply() & (1<<effIndex)) && !aurApp->HasEffect(effIndex))
                targetList.push_back(aurApp->GetTarget());
    }

    // apply effect to targets
    for (auto& itr : targetList)
    {
        if (GetApplicationOfTarget(itr->GetGUID()))
        {
            // owner has to be in world, or effect has to be applied to self
            ASSERT((!GetOwner()->IsInWorld() && GetOwner() == itr) || GetOwner()->IsInMap(itr));
            itr->_ApplyAuraEffect(this, effIndex);
        }
    }
}
void Aura::UpdateOwner(uint32 diff, WorldObject* owner)
{
    if(owner != m_owner || IsRemoved())
        return;

    Unit* caster = GetCaster();
    // Apply spellmods for channeled auras
    // used for example when triggered spell of spell:10 is modded
    Spell* modSpell = nullptr;
    Player* modOwner = nullptr;
    if (caster)
    {
        modOwner = caster->GetSpellModOwner();

        if (modOwner)
        {
            modSpell = modOwner->FindCurrentSpellBySpellId(GetId());

            if (modSpell)
                modOwner->SetSpellModTakingSpell(modSpell, true);

            if (HasAuraAttribute(AURA_ATTR_DRINK_ARENA_DELAY))
            {
                if ((m_maxDuration - 5000) >= m_duration)
                {
                    SetAuraAttribute(AURA_ATTR_DRINK_ARENA_DELAY, false);
                    RecalculateAmountOfEffects(true);
                }
            }

        }
    }

    // update aura effects
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (m_effects[i])
            m_effects[i]->Update(diff, caster);
    }

    Update(diff, caster);

    if (m_spellInfo->NeedAuraUpdateTarget || GetType() == DYNOBJ_AURA_TYPE)
    {
        if (m_updateTargetMapInterval <= int32(diff))
            UpdateTargetMap(caster);
        else
            m_updateTargetMapInterval -= diff;
    }

    // remove spellmods after effects update
    if (modSpell)
        modOwner->SetSpellModTakingSpell(modSpell, false);

    _DeleteRemovedApplications();
}

void Aura::Update(uint32 diff, Unit* caster)
{
    if (m_duration > 0 || (!IsPassive() && m_duration == -1))
    {
        if (m_duration > 0)
        {
            m_duration -= diff;
            m_allDuration += diff;
            if (m_duration < 0)
                m_duration = 0;
        }

        if (GetId() == 192081 && !durationByStack.empty()) // Ironfur
        {
            for (uint8 i = 0; i < durationByStack.size(); ++i)
            {
                if (durationByStack[i] <= diff)
                {
                    if (GetStackAmount() > 1)
                        durationByStack.erase(durationByStack.begin()+i);
                    ModStackAmount(-1);
                    break;
                }
                durationByStack[i] -= diff;
            }
        }
    }

    CallScriptAuraUpdateHandlers(diff);
}

int32 Aura::CalcMaxDuration(Unit* caster)
{
    Player* modOwner = nullptr;
    int32 maxDuration = m_spellInfo->GetMisc(m_diffMode)->Duration.MaxDuration;

    if (caster)
    {
        modOwner = caster->GetSpellModOwner();
        maxDuration = caster->CalcSpellDuration(m_spellInfo, GetComboPoints());
    }
    else
        maxDuration = m_spellInfo->GetDuration(m_diffMode);

    if (IsPassive() && !m_spellInfo->GetMisc(m_diffMode)->Duration.MaxDuration)
        maxDuration = -1;

    CallScriptCalcMaxDurationHandlers(maxDuration);

    // IsPermanent() checks max duration (which we are supposed to calculate here)
    if (maxDuration != -1 && modOwner)
        modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, maxDuration);

    return maxDuration;
}

void Aura::SetDuration(int32 duration, bool withMods)
{
     //! no need chech for -1 or 0
    if (withMods && duration > 0)
    {
        if (Unit* caster = GetCaster())
            if (Player* modOwner = caster->GetSpellModOwner())
                modOwner->ApplySpellMod(GetId(), SPELLMOD_DURATION, duration);
    }
    m_duration = duration;
    SetNeedClientUpdateForTargets();
}

void Aura::RefreshDuration()
{
     // Don`t update dur
    switch (GetId())
    {
        case 231895: // Crusade
            return;
        default:
            break;
    }

    SetDuration(GetMaxDuration());

    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (HasEffect(i))
            GetEffect(i)->CalculatePeriodic(caster, false, false);
    }
}

void Aura::RefreshTimers()
{
    m_maxDuration = CalcMaxDuration();
    RefreshDuration();
}

void Aura::SetCharges(uint8 charges)
{
    if (m_procCharges == charges)
        return;
    m_procCharges = charges;
    SetAuraAttribute(AURA_ATTR_IS_USING_CHARGES, m_procCharges != 0);
    SetNeedClientUpdateForTargets();
}

uint8 Aura::CalcMaxCharges(Unit* caster, bool add) const
{
    uint32 maxProcCharges = m_spellInfo->GetAuraOptions(m_diffMode)->ProcCharges;

    if (SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId()))
    {
        if (add)
            maxProcCharges = procEntry->modcharges;
        else
            maxProcCharges = procEntry->charges;
    }

    if (caster)
        if (Player* modOwner = caster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHARGES, maxProcCharges);
            bool canStack = maxProcCharges && !GetStackAmount();
            switch (GetId())
            {
                case 16870:
                    canStack = true;
                    break;
                default:
                    break;
            }
            if (canStack)
                modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT2, maxProcCharges);
        }

    return uint8(maxProcCharges);
}

bool Aura::ModCharges(int32 num, AuraRemoveMode removeMode)
{
    if (HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES) || !m_spellInfo->IsStack())
    {
        //if aura not modify and have stack and have charges aura use stack for drop stack visual
        if (m_spellInfo->HasAttribute(SPELL_ATTR1_CU_IS_USING_STACKS))
        {
            bool _useStack = true;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (m_spellInfo->EffectMask < uint32(1 << i))
                    break;

                if (AuraEffect* aurEff = GetEffect(i))
                    if (aurEff->GetAuraType() == SPELL_AURA_ADD_FLAT_MODIFIER || aurEff->GetAuraType() == SPELL_AURA_ADD_PCT_MODIFIER)
                        _useStack = false;
            }

            if(_useStack || !m_spellInfo->IsStack())
            {
                ModStackAmount(num);
                return false;
            }
        }

        int32 charges = m_procCharges + num;
        int32 maxCharges = CalcMaxCharges();

        // limit charges (only on charges increase, charges may be changed manually)
        if ((num > 0) && (charges > int32(maxCharges)))
            charges = maxCharges;
        // we're out of charges, remove
        else if (charges <= 0)
        {
            Remove(removeMode);
            return true;
        }

        SetCharges(charges);
    }
    return false;
}

void Aura::SetStackAmount(uint16 stackAmount)
{
    m_stackAmount = stackAmount;
    Unit* caster = GetCaster();

    std::list<AuraApplication*> applications;
    GetApplicationList(applications);

    for (std::list<AuraApplication*>::const_iterator apptItr = applications.begin(); apptItr != applications.end(); ++apptItr)
        if (!(*apptItr)->GetRemoveMode())
            HandleAuraSpecificMods(*apptItr, caster, false, true);

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (HasEffect(i))
            m_effects[i]->ChangeAmount(m_effects[i]->CalculateAmount(caster), false, true);
    }

    for (std::list<AuraApplication*>::const_iterator apptItr = applications.begin(); apptItr != applications.end(); ++apptItr)
        if (!(*apptItr)->GetRemoveMode())
            HandleAuraSpecificMods(*apptItr, caster, true, true);

    SetNeedClientUpdateForTargets();
}

bool Aura::ModStackAmount(int16 num, AuraRemoveMode removeMode)
{
    if (SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId()))
        if (procEntry->modcharges != 0)
        {
            ModCharges(num, removeMode);
            return false;
        }

    int16 stackAmount = m_stackAmount + num;
    int16 maxStackAmount = m_spellInfo->GetAuraOptions(GetSpawnMode())->CumulativeAura;

    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT, maxStackAmount);
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT2, maxStackAmount);
        }

    // limit the stack amount (only on stack increase, stack amount may be changed manually)
    if ((num > 0) && (stackAmount > maxStackAmount))
    {
        // not stackable aura - set stack amount to 1
        if (!maxStackAmount)
            stackAmount = 1;
        else
            stackAmount = maxStackAmount;
    }
    // we're out of stacks, remove
    else if (stackAmount <= 0)
    {
        Remove(removeMode);
        return true;
    }

    bool refresh = stackAmount >= GetStackAmount();

    // Update stack amount
    SetStackAmount(stackAmount);

    if (refresh)
    {
        if (GetId() == 192081) // Ironfur
        {
            if (durationByStack.size() <= stackAmount)
                durationByStack.resize(stackAmount);
            durationByStack[stackAmount - 1] = GetMaxDuration();
        }

        RefreshSpellMods();
        if (m_spellInfo->IsRefreshTimers())
            RefreshTimers();

        // reset charges
        SetCharges(CalcMaxCharges());

        // FIXME: not a best way to synchronize charges, but works
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (AuraEffect* aurEff = GetEffect(i))
                if (aurEff->GetAuraType() == SPELL_AURA_ADD_FLAT_MODIFIER || aurEff->GetAuraType() == SPELL_AURA_ADD_PCT_MODIFIER)
                    if (SpellModifier* mod = aurEff->GetSpellModifier())
                        mod->charges = GetCharges();
        }
    }

    SetNeedClientUpdateForTargets();
    return false;
}

void Aura::SetMaxStackAmount()
{
    int32 maxStackAmount = m_spellInfo->GetAuraOptions(GetSpawnMode())->CumulativeAura;

    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT, maxStackAmount);
            if (maxStackAmount || !HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES))
                modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT2, maxStackAmount);
        }

    bool refresh = maxStackAmount >= GetStackAmount();

    // Update stack amount
    SetStackAmount(maxStackAmount);

    if (refresh)
    {
        RefreshSpellMods();
        RefreshTimers();

        SetCharges(CalcMaxCharges());

        // FIXME: not a best way to synchronize charges, but works
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->EffectMask < uint32(1 << i))
                break;

            if (AuraEffect* aurEff = GetEffect(i))
                if (aurEff->GetAuraType() == SPELL_AURA_ADD_FLAT_MODIFIER || aurEff->GetAuraType() == SPELL_AURA_ADD_PCT_MODIFIER)
                    if (SpellModifier* mod = aurEff->GetSpellModifier())
                        mod->charges = GetCharges();
        }
    }

    SetNeedClientUpdateForTargets();
}

void Aura::RefreshSpellMods()
{
    for (Aura::ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        if (AuraApplicationPtr aurApp = appIter->second)
            if (Player* player = aurApp->GetTarget()->ToPlayer())
                player->RestoreAllSpellMods(0, this);
}

bool Aura::HasMoreThanOneEffectForType(AuraType auraType) const
{
    uint32 count = 0;

    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (GetSpellInfo()->EffectMask < uint32(1 << effIndex))
            break;

        if (!HasEffect(effIndex))
            continue;

        if (auto const& effect = GetSpellInfo()->GetEffect(effIndex, m_diffMode))
            if (effect && HasEffect(effect->EffectIndex) && AuraType(effect->ApplyAuraName) == auraType)
                ++count;
    }

    return count > 1;
}

bool Aura::IsArea() const
{
    return m_spellInfo->HasAreaAuraEffect();
}

bool Aura::IsPassive() const
{
    return GetSpellInfo() && GetSpellInfo()->IsPassive();
}

bool Aura::IsDeathPersistent() const
{
    return GetSpellInfo()->IsDeathPersistent();
}

bool Aura::CanBeSaved() const
{
    if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_NEED_BE_SAVED_IN_DB))
        return true;

    if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_CANT_BE_SAVED_IN_DB))
        return false;

    if (m_spellInfo->HasAura(SPELL_AURA_ENABLE_EXTRA_TALENT))
        return false;

    if (IsPassive())
        return false;

    if (GetCasterGUID() != GetOwner()->GetGUID())
        if (GetSpellInfo()->IsSingleTarget(GetCaster(), GetUnitOwner()) || IsMultiSingleTarget())
            return false;

    // don't save auras casted by entering areatriggers
    if (auraAttributes & AURA_ATTR_FROM_AREATRIGGER)
        return false;

    if (HasAuraAttribute(AURA_ATTR_FROM_EQUIPABLE_ITEM))
        return false;

    // don't save auras casted by summons
    if (GetCaster() && !GetCaster()->IsPlayer() && GetCaster()->isAnySummons())
        return false;

    // not save area auras (phase auras, e.t.c)
    SpellAreaMapBounds saBounds = sSpellMgr->GetSpellAreaMapBounds(GetId());
    if (saBounds.first != saBounds.second)
        return false;

    // don't save auras removed by proc system
    if (HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES) && !GetCharges())
        return false;

    if (m_spellInfo->HasAura(SPELL_AURA_CLONE_CASTER))
        return false;

    return true;
}

bool Aura::IsSingleTargetWith(Aura const* aura) const
{
    if (!GetSpellInfo())
        return false;

    // Same spell?
    if (GetSpellInfo()->IsRankOf(aura->GetSpellInfo()))
        return true;

    SpellSpecificType spec = GetSpellInfo()->GetSpellSpecific();
    switch (spec)
    {
        case SPELL_SPECIFIC_MAGE_POLYMORPH:
            if (aura->GetSpellInfo()->GetSpellSpecific() == spec)
                return true;
            break;
        default:
            break;
    }

    return HasEffectType(SPELL_AURA_CONTROL_VEHICLE) && aura->HasEffectType(SPELL_AURA_CONTROL_VEHICLE);
}

void Aura::UnregisterCasterAuras(AuraRemoveMode removeMode)
{
    Unit* caster = GetCaster();
    // TODO: find a better way to do this.
    if (!caster)
        caster = ObjectAccessor::GetObjectInOrOutOfWorld(GetCasterGUID(), static_cast<Unit*>(nullptr));
    if(!caster)
        return;
    caster->RemoveMyCastAuras(GetId(), this);

    if (removeMode == AURA_REMOVE_BY_DEFAULT || removeMode == AURA_REMOVE_BY_EXPIRE || removeMode == AURA_REMOVE_BY_DEATH)
        return;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (GetSpellInfo()->GetEffect(i, m_diffMode)->TargetA.GetTarget() == TARGET_UNIT_CASTER_AREA_SUMMON)
        {
            if (Player* player = caster->ToPlayer())
                if (player->GetSession() && player->GetSession()->PlayerLogout())
                    return;

            if(!caster->IsInWorld() || !GetUnitOwner() || !GetUnitOwner()->IsInWorld())
                return;

            Unit* owner = caster->GetAnyOwner() ? caster->GetAnyOwner() : caster;
            uint32 spellID = GetId();
            Unit* unitOwner = GetUnitOwner();
            owner->AddDelayedEvent(1000, [=]() -> void
            {
                owner->RemovePetAndOwnerAura(spellID, unitOwner);
            });
            return;
        }
    }
}

void Aura::UnregisterSingleTarget()
{
    ASSERT(HasAuraAttribute(AURA_ATTR_IS_SINGLE_TARGET));
    Unit* caster = GetCaster();
    // TODO: find a better way to do this.
    if (!caster)
        caster = ObjectAccessor::GetObjectInOrOutOfWorld(GetCasterGUID(), static_cast<Unit*>(nullptr));
    //ASSERT(caster);
    if(!caster)
        return;
    caster->GetSingleCastAuras().remove(this);
    SetAuraAttribute(AURA_ATTR_IS_SINGLE_TARGET, false);
}

void Aura::UnregisterMultiSingleTarget()
{
    Unit* caster = GetCaster();
    // TODO: find a better way to do this.
    if (!caster)
        caster = ObjectAccessor::GetObjectInOrOutOfWorld(GetCasterGUID(), static_cast<Unit*>(nullptr));
    //ASSERT(caster);
    if(!caster)
        return;
    caster->GetMultiSingleTargetAuras().remove(this);
}

bool Aura::IsMultiSingleTarget() const
{
    return m_spellInfo->IsMultiSingleTarget();
}

uint32 Aura::GetMultiSingleTargetCount() const
{
    return m_spellInfo->GetMultiSingleTargetCount();
}

int32 Aura::CalcDispelChance(Unit* /*auraTarget*/, bool /*offensive*/) const
{
    // we assume that aura dispel chance is 100% on start
    // need formula for level difference based chance
    int32 resistChance = 0;

    // Apply dispel mod from aura caster
    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_RESIST_DISPEL_CHANCE, resistChance);

    resistChance = resistChance < 0 ? 0 : resistChance;
    resistChance = resistChance > 100 ? 100 : resistChance;

    return 100 - resistChance;
}

bool Aura::HasAuraAttribute(AuraAttr attribute) const
{
    return auraAttributes & attribute;
}

void Aura::SetAuraAttribute(AuraAttr attribute, bool apply)
{
    if (!this)
        return;

    if (apply)
    {
        auraAttributes |= attribute;
        return;
    }

    auraAttributes &= ~attribute;
}

void Aura::SetLoadedState(int32 maxduration, int32 duration, int32 charges, uint8 stackamount, uint32 recalculateMask, int32 * amount)
{
    m_maxDuration = maxduration;
    m_duration = duration;
    m_procCharges = charges;
    SetAuraAttribute(AURA_ATTR_IS_USING_CHARGES, m_procCharges != 0);
    m_stackAmount = stackamount;
    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (m_effects[i])
        {
            m_effects[i]->SetAmount(amount[i]);
            m_effects[i]->SetCanBeRecalculated((recalculateMask & (1 << i)) != 0);
            m_effects[i]->CalculatePeriodic(caster, false, true);
            m_effects[i]->CalculateSpellMod();
            m_effects[i]->RecalculateAmount(caster);
        }
    }
}

bool Aura::HasEffectType(AuraType type) const
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (HasEffect(i) && m_effects[i]->GetAuraType() == type)
            return true;
    }
    return false;
}

void Aura::RecalculateAmountOfEffects(bool setCanRecalc)
{
    ASSERT (!IsRemoved());
    Unit* caster = GetCaster();
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (HasEffect(i))
        {
            if(setCanRecalc && !IsRemoved())
                m_effects[i]->SetCanBeRecalculated(true);
            m_effects[i]->RecalculateAmount(caster);
        }
    }
}

void Aura::HandleAllEffects(AuraApplication * aurApp, uint8 mode, bool apply)
{
    ASSERT (!IsRemoved());
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        if (m_effects[i] && !IsRemoved())
            m_effects[i]->HandleEffect(aurApp, mode, apply);
    }
}

void Aura::GetApplicationList(std::list<AuraApplication*> & applicationList)
{
    for (Aura::ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
    {
        if (AuraApplicationPtr aurApp = appIter->second)
            if (aurApp->GetEffectMask())
                applicationList.push_back(aurApp.get());
    }
}

void Aura::SetNeedClientUpdateForTargets()
{
    for (ApplicationMap::iterator appIter = m_applications.begin(); appIter != m_applications.end(); ++appIter)
        if (AuraApplicationPtr aurApp = appIter->second)
            aurApp->SetNeedClientUpdate();
}

// trigger effects on real aura apply/remove
void Aura::HandleAuraSpecificMods(AuraApplication const* aurApp, Unit* caster, bool apply, bool onReapply)
{
    Unit* target = aurApp->GetTarget();
    if (!target)
        return;

    AuraRemoveMode removeMode = aurApp->GetRemoveMode();
    // handle spell_area table
    SpellAreaForAreaMapBounds saBounds = sSpellMgr->GetSpellAreaForAuraMapBounds(GetId());
    if (saBounds.first != saBounds.second)
    {
        uint32 zone = target->GetCurrentZoneID();
        uint32 area = target->GetCurrentAreaID();

        for (auto itr = saBounds.first; itr != saBounds.second; ++itr)
        {
            // some auras remove at aura remove
            if (!itr->second->IsFitToRequirements(target->ToPlayer(), zone, area))
                target->RemoveAurasDueToSpell(itr->second->spellId);
            // some auras applied at aura apply
            else if (itr->second->autocast)
            {
                if (!target->HasAura(itr->second->spellId))
                    target->CastSpell(target, itr->second->spellId, true);
            }
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "HandleAuraSpecificMods GetId %u removeMode %u caster %u", GetId(), removeMode, bool(caster));

    // handle spell_linked_spell table
    if (!onReapply)
    {
        //Phase
        if (Player* player = target->ToPlayer())
        {
            if (apply)    
                player->GetPhaseMgr().RegisterPhasingAura(GetId(), target);
            else    
                player->GetPhaseMgr().UnRegisterPhasingAura(GetId(), target);
        }

        Item* castItem = nullptr;
        if (m_castItemGuid && caster)
            if(Player* _lplayer = caster->ToPlayer())
            {
                castItem = _lplayer->GetItemByGuid(m_castItemGuid);
                if (castItem && !castItem->IsInWorld())
                    castItem = nullptr;
            }

        // apply linked auras
        if (apply)
        {                
            if (std::vector<SpellLinked> const* spellTriggered = sSpellMgr->GetSpellLinked(SPELL_LINK_AURA, GetId()))
            {
                for (const auto& itr : *spellTriggered)
                {
                    Unit* _target = target;
                    Unit* _caster = caster;

                    if (itr.target)
                        _target = target->GetUnitForLinkedSpell(caster, target, itr.target);

                    if (itr.caster && caster)
                        _caster = caster->GetUnitForLinkedSpell(caster, target, itr.caster);

                    if(itr.hastalent)
                        if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype, itr.hastalent, itr.hasparam))
                            continue;

                    if(itr.hastalent2)
                        if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                            continue;

                    if(!_target)
                        continue;

                    if (itr.effect < 0)
                    {
                        switch (itr.actiontype)
                        {
                            case LINK_ACTION_DEFAULT:
                                _target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(itr.effect), true);
                                break;
                            case LINK_ACTION_LEARN:
                            {
                                if(Player* _lplayer = _target->ToPlayer())
                                    _lplayer->removeSpell(abs(itr.effect));
                                break;
                            }
                            case LINK_ACTION_AURATYPE:
                                _target->RemoveAurasByType(AuraType(itr.hastalent2));
                                break;
                            default:
                                break;
                        }
                    }
                    else if (_caster)
                    {
                        if(itr.chance != 0 && !roll_chance_i(itr.chance))
                            continue;
                        if(itr.cooldown != 0 && _target->HasSpellCooldown(itr.effect))
                            continue;

                        switch (itr.actiontype)
                        {
                            case LINK_ACTION_DEFAULT:
                                _caster->AddAura(itr.effect, _target);
                                break;
                            case LINK_ACTION_LEARN:
                            {
                                if(Player* _lplayer = _caster->ToPlayer())
                                    _lplayer->learnSpell(itr.effect, false);
                                break;
                            }
                            case LINK_ACTION_CASTINAURA:
                                _caster->CastSpell(_target, itr.effect, true, castItem, nullptr, GetCasterGUID());
                                break;
                            default:
                                break;
                        }

                        if(itr.cooldown != 0)
                            _target->AddSpellCooldown(itr.effect, 0, getPreciseTime() + static_cast<double>(itr.cooldown));
                    }
                }
            }
            if (std::vector<SpellLinked> const* spellTriggered = sSpellMgr->GetSpellLinked(SPELL_LINK_AURA_HIT, GetId()))
            {
                for (const auto& itr : *spellTriggered)
                {
                    Unit* _target = target;
                    Unit* _caster = caster;

                    if (itr.target)
                        _target = target->GetUnitForLinkedSpell(caster, target, itr.target);

                    if (itr.caster && caster)
                        _caster = caster->GetUnitForLinkedSpell(caster, target, itr.caster);

                    if(itr.hastalent)
                        if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype, itr.hastalent, itr.hasparam))
                            continue;

                    if(itr.hastalent2)
                        if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                            continue;
                    if(!_target)
                        continue;

                    if (itr.effect < 0)
                    {
                        switch (itr.actiontype)
                        {
                            case LINK_ACTION_DEFAULT:
                                _target->RemoveAurasDueToSpell(-(itr.effect));
                                break;
                            case LINK_ACTION_LEARN:
                            {
                                if(Player* _lplayer = _target->ToPlayer())
                                    _lplayer->removeSpell(abs(itr.effect));
                                break;
                            }
                            case LINK_ACTION_AURATYPE:
                                _target->RemoveAurasByType(AuraType(itr.hastalent2));
                                break;
                            default:
                                break;
                        }
                    }
                    else if (_caster)
                    {
                        if(itr.chance != 0 && !roll_chance_i(itr.chance))
                            continue;
                        if(itr.cooldown != 0 && _target->HasSpellCooldown(itr.effect))
                            continue;

                        switch (itr.actiontype)
                        {
                            case LINK_ACTION_DEFAULT:
                                _caster->AddAura(itr.effect, _target);
                                break;
                            case LINK_ACTION_LEARN:
                            {
                                if(Player* _lplayer = _caster->ToPlayer())
                                    _lplayer->learnSpell(itr.effect, false);
                                break;
                            }
                            case LINK_ACTION_CASTINAURA:
                                _caster->CastSpell(_target, itr.effect, true, castItem, nullptr, GetCasterGUID());
                                break;
                            default:
                                break;
                        }

                        if(itr.cooldown != 0)
                            _target->AddSpellCooldown(itr.effect, 0, getPreciseTime() + static_cast<double>(itr.cooldown));
                    }
                }
            }
        }
        else
        {
            bool loginOut = false;

            if (caster)
                if (!caster->IsInWorld())
                    loginOut = true;

            // remove linked auras
            if (!loginOut)
            {
                if (caster && caster->IsCreature() && caster->ToCreature()->IsAIEnabled)
                    caster->ToCreature()->AI()->OnRemoveAuraTarget(target, GetId(), removeMode);

                if (std::vector<SpellLinked> const* spellTriggered = sSpellMgr->GetSpellLinked(SPELL_LINK_REMOVE, -static_cast<int32>(GetId())))
                {
                    for (const auto& itr : *spellTriggered)
                    {
                        Unit* _target = target;
                        Unit* _caster = caster ? caster : target;

                        if (itr.target)
                            _target = target->GetUnitForLinkedSpell(caster, target, itr.target);

                        if (itr.caster && caster)
                            _caster = caster->GetUnitForLinkedSpell(caster, target, itr.caster);

                        if(itr.hastalent)
                            if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype, itr.hastalent, itr.hasparam))
                                continue;

                        if(itr.hastalent2)
                            if(target->HasAuraLinkedSpell(_caster, _target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                                continue;

                        if(!_target)
                            continue;

                        if (itr.effect < 0)
                        {
                            switch (itr.actiontype)
                            {
                                case LINK_ACTION_DEFAULT:
                                    _target->RemoveAurasDueToSpell(-(itr.effect));
                                    break;
                                case LINK_ACTION_LEARN:
                                {
                                    if(Player* _lplayer = _target->ToPlayer())
                                        _lplayer->removeSpell(abs(itr.effect));
                                    break;
                                }
                                case LINK_ACTION_AURATYPE:
                                    _target->RemoveAurasByType(AuraType(itr.hastalent2));
                                    break;
                                case LINK_ACTION_SEND_COOLDOWN: // 3
                                    _target->SendSpellCooldown(itr.effect, GetId(), itr.duration);
                                    break;
                                case LINK_ACTION_CHANGE_CHARGES: // 12
                                    if (Aura* aura = _target->GetAura(abs(itr.effect)))
                                        aura->ModCharges(itr.param ? -int32(itr.param) : -1);
                                    break;
                                case LINK_ACTION_CHANGE_STACK:
                                    if (Aura* aura = _target->GetAura(abs(itr.effect)))
                                        aura->ModStackAmount(itr.param ? -int32(itr.param) : -1);
                                    break;
                                case LINK_ACTION_CAST_DURATION: // 21
                                    _target->CastSpellDuration(_target, abs(itr.effect), true, itr.duration);
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            if (!itr.removeMask && removeMode == AURA_REMOVE_BY_DEATH)
                                continue;

                            if (itr.removeMask && !(itr.removeMask & (1 << removeMode)))
                                continue;

                            if(itr.chance != 0 && !roll_chance_i(itr.chance))
                                continue;
                            if(itr.cooldown != 0 && _target->HasSpellCooldown(itr.effect))
                                continue;

                            switch (itr.actiontype)
                            {
                                case LINK_ACTION_DEFAULT:
                                    if(_caster)
                                        _caster->CastSpell(_target, itr.effect, true, castItem, nullptr, GetCasterGUID());
                                    break;
                                case LINK_ACTION_LEARN:
                                {
                                    if(Player* _lplayer = _target->ToPlayer())
                                        _lplayer->learnSpell(itr.effect, false);
                                    break;
                                }
                                case LINK_ACTION_SEND_COOLDOWN: // 3
                                    _target->SendSpellCooldown(itr.effect, GetId(), itr.duration);
                                    break;
                                case LINK_ACTION_CASTINAURA:
                                    if(_caster)
                                        _caster->CastSpell(_target, itr.effect, true, castItem, nullptr, GetCasterGUID());
                                    break;
                                case LINK_ACTION_REMOVE_COOLDOWN: //8
                                {
                                    if (Player* _lplayer = _target->ToPlayer())
                                        _lplayer->RemoveSpellCooldown(itr.effect, true);
                                    break;
                                }
                                case LINK_ACTION_CHANGE_STACK: //7
                                    if(!_caster)
                                        continue;

                                    if (Aura* aura = _target->GetAura(itr.effect))
                                        aura->ModStackAmount(itr.param ? int32(itr.param) : 1);
                                    else if (itr.param)
                                    {
                                        for (uint8 count = 0; count < uint8(itr.param); ++count)
                                            _caster->CastSpell(_target, itr.effect, true, castItem);
                                    }
                                    else
                                        _caster->CastSpell(_target, itr.effect, true, castItem);
                                    break;
                                case LINK_ACTION_REMOVE_MOVEMENT: //9
                                    _target->RemoveMovementImpairingEffects();
                                    break;
                                case LINK_ACTION_CHANGE_CHARGES: // 12
                                    if (Aura* aura = _target->GetAura(abs(itr.effect)))
                                        aura->ModCharges(itr.param ? int32(itr.param) : 1);
                                    break;
                                case LINK_ACTION_RECALCULATE_AMOUNT: //16
                                    if(Aura* aura = _target->GetAura(itr.effect))
                                        aura->RecalculateAmountOfEffects(true);
                                    break;
                                case LINK_ACTION_MODIFY_COOLDOWN: //13
                                    if (_target->IsPlayer())
                                        _target->ToPlayer()->ModifySpellCooldown(itr.effect, int32(itr.param));
                                    break;
                                case LINK_ACTION_CATEGORY_COOLDOWN: //14
                                    if (_target->IsPlayer())
                                        _target->ToPlayer()->ModSpellChargeCooldown(itr.effect, int32(itr.param));
                                    break;
                                case LINK_ACTION_CATEGORY_CHARGES: //15
                                    if (_target->IsPlayer())
                                        _target->ToPlayer()->ModSpellCharge(itr.effect, int32(itr.param));
                                    break;
                                case LINK_ACTION_REMOVE_CATEGORY_CD: // 19
                                    if (Player* _lplayer = _target->ToPlayer())
                                        _lplayer->RemoveCategoryCooldownBySpell(itr.effect, true);
                                    break;
                                case LINK_ACTION_CAST_DURATION: // 21
                                    if(_caster)
                                        _caster->CastSpellDuration(_target, abs(itr.effect), true, itr.duration);
                                    break;
                                default:
                                    break;
                            }

                            if(itr.cooldown != 0)
                                _target->AddSpellCooldown(itr.effect, 0, getPreciseTime() + static_cast<double>(itr.cooldown));
                        }
                    }
                }
                if (std::vector<SpellLinked> const* spellTriggered = sSpellMgr->GetSpellLinked(SPELL_LINK_AURA, GetId()))
                {
                    for (const auto& itr : *spellTriggered)
                    {
                        Unit* _target = target;

                        if (itr.target)
                            _target = target->GetUnitForLinkedSpell(caster, target, itr.target);

                        if(!_target)
                            continue;

                        if (itr.effect < 0)
                        {
                            switch (itr.actiontype)
                            {
                                case LINK_ACTION_DEFAULT:
                                    _target->ApplySpellImmune(GetId(), IMMUNITY_ID, -(itr.effect), false);
                                    break;
                                case LINK_ACTION_LEARN:
                                {
                                    if(Player* _lplayer = _target->ToPlayer())
                                        _lplayer->removeSpell(abs(itr.effect));
                                    break;
                                }
                                case LINK_ACTION_AURATYPE:
                                    _target->RemoveAurasByType(AuraType(itr.hastalent2));
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                            _target->RemoveAura(itr.effect, GetCasterGUID(), 0, removeMode);
                    }
                }
            }
        }
    }
    else if (apply)
    {
        // modify stack amount of linked auras
        if (std::vector<SpellLinked> const* spellTriggered = sSpellMgr->GetSpellLinked(SPELL_LINK_AURA, GetId()))
        {
            for (const auto& itr : *spellTriggered)
                if (itr.effect > 0)
                {
                    if(itr.hastalent)
                        if(target->HasAuraLinkedSpell(caster, target, itr.hastype, itr.hastalent, itr.hasparam))
                            continue;

                    if(itr.hastalent2)
                        if(target->HasAuraLinkedSpell(caster, target, itr.hastype2, itr.hastalent2, itr.hasparam2))
                            continue;

                    if(itr.chance != 0 && !roll_chance_i(itr.chance))
                        continue;
                    if(itr.cooldown != 0 && target->HasSpellCooldown(itr.effect))
                        continue;

                    switch (itr.actiontype)
                    {
                        case LINK_ACTION_DEFAULT:
                        {
                            if (Aura* triggeredAura = target->GetAura(itr.effect, GetCasterGUID()))
                                triggeredAura->ModStackAmount(GetStackAmount() - triggeredAura->GetStackAmount());
                            break;
                        }
                        case LINK_ACTION_LEARN:
                        {
                            if(Player* _lplayer = target->ToPlayer())
                                _lplayer->learnSpell(itr.effect, false);
                            break;
                        }
                        default:
                            break;
                    }

                    if(itr.cooldown != 0)
                        target->AddSpellCooldown(itr.effect, 0, getPreciseTime() + static_cast<double>(itr.cooldown));
                }
        }
    }

    // mods at aura apply
    if (apply)
    {
        //Hack use AT for auras, when AT very big radius and not visible
        std::vector<AreaTriggerForce> const* forceData = sAreaTriggerDataStore->GetAreaTriggerForce(GetId());
        if (forceData && !forceData->empty())
        {
            for (const auto& itr : *forceData)
            {
                if (Player* player = target->ToPlayer())
                {
                    m_areaTrGuid = ObjectGuid::Create<HighGuid::AreaTrigger>(target->GetMapId(), itr.CustomEntry, sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate());
                    player->SendMovementForceAura(m_areaTrGuid, itr.wind, itr.center, itr.windSpeed, itr.windType, true);
                }
            }
        }
        switch (GetSpellInfo()->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {
                    case 32474: // Buffeting Winds of Susurrus
                        if (target->IsPlayer())
                            target->ToPlayer()->ActivateTaxiPathTo(506, GetId());
                        break;
                    case 33572: // Gronn Lord's Grasp, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(33652))
                            target->CastSpell(target, 33652, true);
                        break;
                    case 50836: //Petrifying Grip, becomes stoned
                        if (GetStackAmount() >= 5 && !target->HasAura(50812))
                            target->CastSpell(target, 50812, true);
                        break;
                    default:
                        break;
                }
                break;
            case SPELLFAMILY_ROGUE:
            {
                if (!caster)
                    break;

                switch (m_spellInfo->Id)
                {
                    case 1784: // Stealth
                    {
                        caster->AddAura(158185, caster);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
    // mods at aura remove
    else
    {
        if (m_areaTrGuid)
        {
            if (Player* player = target->ToPlayer())
                player->SendMovementForceAura(m_areaTrGuid, Position(), Position(), 0.0f, 0, false);
        }

        switch (GetSpellInfo()->ClassOptions.SpellClassSet)
        {
            case SPELLFAMILY_GENERIC:
                switch (GetId())
                {
                    case 195901: // Adapted
                    {
                        if (Aura * aura = target->GetAura(214027))
                            aura->SetAuraAttribute(AURA_ATTR_IS_NOT_ACTIVE, false);

                        break;
                    }
                    case 72368:  // Shared Suffering
                    {
                        if (AuraEffect* aurEff = GetEffect(0))
                        {
                            float remainingDamage = aurEff->GetAmount() * (aurEff->GetTotalTicks() - aurEff->GetTickNumber());
                            if (remainingDamage > 0)
                                caster->CastCustomSpell(caster, 72373, nullptr, &remainingDamage, nullptr, true);
                        }
                        break;
                    }
                    case 49440:  // Racer Slam, Slamming
                    {
                        if (Creature* racerBunny = target->FindNearestCreature(27674, 25.0f))
                            target->CastSpell(racerBunny, 49302, false);
                        break;
                    }
                    case 224121: // Lightning Burst Aura
                    {
                        if (removeMode != AURA_REMOVE_BY_EXPIRE)
                        {
                            caster->CastSpell(caster, 224128, true);
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            case SPELLFAMILY_MAGE:
                switch (GetId())
                {
                    case 148012: // Icicles
                    case 148013:
                    case 148014:
                    case 148015:
                    case 148016:
                    {
                        if (Aura* aur = caster->GetAura(205473))
                            aur->ModStackAmount(-1);

                        if (Aura* aura = caster->GetAura(76613))
                        {
                            for (auto itr : aura->m_loadedScripts)
                                itr->CallSpecialFunction(1);
                        }
                        break;
                    }
                    case 66: // Invisibility
                        if (removeMode != AURA_REMOVE_BY_EXPIRE)
                            break;
                        target->CastSpell(target, 32612, true, nullptr, GetEffect(1));
                        target->CombatStop();
                        break;
                    default:
                        break;
                }
                if (!caster)
                    break;
                break;
            case SPELLFAMILY_ROGUE:
            {
                switch (GetId())
                {
                    case 1784:  // Stealth
                    {
                        caster->RemoveAurasDueToSpell(158185);
                        break;
                    }
                    case 11327: // Vanish
                    {
                        if (removeMode != AURA_REMOVE_BY_EXPIRE)
                            break;

                        if (Player* player =  caster->ToPlayer())
                        {
                            player->RemoveSpellCooldown(1784, false);
                            caster->CastSpell(caster, 1784, true);
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    // mods at aura apply or remove
    switch (GetSpellInfo()->ClassOptions.SpellClassSet)
    {
        case SPELLFAMILY_ROGUE:
        {
            if (!caster)
                return;

            switch (GetId())
            {
                case 13750:  // Adrenaline Rush
                {
                    if (AreaTrigger* AT = caster->GetAreaObject(212217))
                    {
                        if (AreaTriggerAI* ATAI = AT->AI())
                            ATAI->CallSpecialFunction(apply);
                    }
                    break;
                }
                case 1784:   // Stealth
                case 11327:  // Vanish
                {
                    if (!apply && (removeMode == AURA_REMOVE_BY_INTERRUPT || removeMode == AURA_REMOVE_BY_DEFAULT))
                    {
                        if (caster->HasAura(108208))
                            caster->CastSpell(caster, 115192, true);
                    }
                    if (caster->HasAura(231718)) // Shadowstrike (lvl 2)
                    {
                        if (apply)
                        {
                            caster->AddAura(245623, caster);
                        }
                        else
                        {
                            if (!caster->HasAura(1784) && !caster->HasAura(11327))
                            {
                                caster->RemoveAurasDueToSpell(245623);
                            }
                        }
                    }
                }
                case 115192: // Subterfuge
                case 185313: // Shadowdance (Shapeshift)
                {
                    bool removeTalentAura = false;

                    if (apply)
                    {
                        caster->AddAura(158188, caster);
                    }
                    else
                    {
                        if (!caster->HasAura(1784) && !caster->HasAura(11327) && !caster->HasAura(185313))
                        {
                            removeTalentAura = true;

                            if (!caster->HasAura(115192))
                                caster->RemoveAurasDueToSpell(158188);
                        }
                    }

                    if (caster->HasAura(108209)) // Shadow Focus
                    {
                        if (apply)
                        {
                            caster->AddAura(112942, caster);
                        }
                        else if (removeTalentAura)
                        {
                            caster->RemoveAurasDueToSpell(112942);
                        }
                    }

                    if (caster->HasAura(31223)) // Master of subtlety
                    {
                        if (apply)
                        {
                            caster->AddAura(31665, caster);
                        }
                        else if (removeTalentAura)
                        {
                            if (Aura * aur = caster->GetAura(31665))
                                aur->SetAuraTimer(5000);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        /*case SPELLFAMILY_HUNTER:
        {
            switch (GetId())
            {
                case 19574: // Bestial Wrath
                    if (target->HasAura(197248)) // Master of Beasts
                        if (Unit* hati = target->GetHati())
                            target->CastSpell(hati, 207033, true);
                    break;
                default:
                    break;
            }
            break;
        }*/
        case SPELLFAMILY_PALADIN:
        {
            switch (GetId())
            {
                case 203539: // Greater Blessing of Wisdom
                case 203538: // Greater Blessing of Kings
                {
                    if (!onReapply && caster)
                    {
                        if (Aura* aura = caster->GetAura(238098)) // Blessing of the Ashbringer
                        {
                            for (auto itr : aura->m_loadedScripts)
                            {
                                if (itr->CallSpecialFunction())
                                    return;
                            }
                        }
                    }
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_MONK:
        {
            switch (GetId())
            {
                if (!caster)
                    return;

                case 248646: // Tigereye Brew (Honor Talent)
                {
                    if (apply)
                    {
                        if (GetStackAmount() >= 10)
                            caster->AddAura(248648, caster);
                    }
                    else
                    {
                        caster->RemoveAura(248648);
                    }
                }
                default:
                    break;
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            if (!caster)
                return;

            switch (GetId())
            {
                case 47585:  // Dispersion
                case 219772: // Sustained Sanity
                case 205065: // Void Torrent
                {
                    if (Aura* aura = caster->GetAura(194249)) // Voidform
                    {
                        for (auto itr : aura->m_loadedScripts)
                        {
                            if (itr->CallSpecialFunction())
                                return;
                        }
                    }
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
}

bool Aura::CanBeAppliedOn(Unit* target)
{
    // unit not in world or during remove from world
    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld())
    {
        // area auras mustn't be applied
        if (GetOwner() != target)
            return false;
        // not selfcasted single target auras mustn't be applied
        if (GetCasterGUID() != GetOwner()->GetGUID() && (GetSpellInfo()->IsSingleTarget(GetCaster(), target) || IsMultiSingleTarget()))
            return false;
        return true;
    }
    return CheckAreaTarget(target);
}

bool Aura::CheckAreaTarget(Unit* target)
{
    return CallScriptCheckAreaTargetHandlers(target);
}

bool Aura::CanStackWith(Aura const* existingAura) const
{
    if (!existingAura)
        return false;

    // Can stack with self
    if (this == existingAura)
        return true;

    // Dynobj auras always stack
    if (existingAura->GetType() == DYNOBJ_AURA_TYPE)
        return true;

    SpellInfo const* existingSpellInfo = existingAura->GetSpellInfo();
    bool sameCaster = GetCasterGUID() == existingAura->GetCasterGUID();

    // passive auras don't stack with another rank of the spell cast by same caster
    if (IsPassive() && sameCaster && m_spellInfo->IsDifferentRankOf(existingSpellInfo))
        return false;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (existingSpellInfo->EffectMask < uint32(1 << i))
            break;
        if (m_spellInfo->EffectMask < uint32(1 << i))
            break;

        // prevent remove triggering aura by triggered aura // prevent remove triggered aura by triggering aura refresh
        if (existingSpellInfo->GetEffect(i, m_diffMode)->TriggerSpell == GetId() || m_spellInfo->GetEffect(i, m_diffMode)->TriggerSpell == existingAura->GetId())
            return true;
    }

    // check spell specific stack rules
    if (m_spellInfo->IsAuraExclusiveBySpecificWith(existingSpellInfo, sameCaster) || (sameCaster && m_spellInfo->IsAuraExclusiveBySpecificPerCasterWith(existingSpellInfo)))
        return false;

    // check spell group stack rules
    switch (sSpellMgr->CheckSpellGroupStackRules(m_spellInfo, existingSpellInfo))
    {
        case SPELL_GROUP_STACK_RULE_EXCLUSIVE:
        case SPELL_GROUP_STACK_RULE_EXCLUSIVE_HIGHEST: // if it reaches this point, existing aura is lower/equal
            return false;
        case SPELL_GROUP_STACK_RULE_EXCLUSIVE_FROM_SAME_CASTER:
            if (sameCaster)
                return false;
            break;
        default:
            break;
    }

    if (m_spellInfo->ClassOptions.SpellClassSet != existingSpellInfo->ClassOptions.SpellClassSet)
        return true;

    if (!sameCaster)
    {
        // Channeled auras can stack if not forbidden by db or aura type
        if (existingAura->GetSpellInfo()->IsChanneled())
            return true;

        if (m_spellInfo->HasAttribute(SPELL_ATTR3_STACK_FOR_DIFF_CASTERS) || m_spellInfo->HasAttribute(SPELL_ATTR6_ONLY_VISIBLE_TO_CASTER) || m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_SELF))
            return true;

        // check same periodic auras
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            switch (m_spellInfo->GetEffect(i, m_diffMode)->ApplyAuraName)
            {
                // DOT or HOT or frame from different casters will stack
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PERIODIC_DUMMY:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_POWER_BURN:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_OBS_MOD_HEALTH:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE:
                    // periodic auras which target areas are not allowed to stack this way (replenishment for example) - Deprecated?(example 247932)
                    //if (m_spellInfo->GetEffect(i, m_diffMode)->IsTargetingArea() || existingSpellInfo->GetEffect(i, m_diffMode)->IsTargetingArea())
                    //    break;
                    return true;
                case SPELL_AURA_ENABLE_BOSS1_UNIT_FRAME:
                case SPELL_AURA_MOD_DAMAGE_FROM_CASTER:                // Vendetta-like auras
                case SPELL_AURA_BYPASS_ARMOR_FOR_CASTER:               // Find Weakness-like auras
                case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:    // Hunter's Mark-like auras
                case SPELL_AURA_SHARE_DAMAGE_PCT:
                    return true;
                default:
                    break;
            }
        }
    }

    // negative and positive spells
    if (m_spellInfo->_IsPositiveSpell() && !existingSpellInfo->_IsPositiveSpell() || !m_spellInfo->_IsPositiveSpell() && existingSpellInfo->_IsPositiveSpell())
        return true;

    if (HasEffectType(SPELL_AURA_CONTROL_VEHICLE) && existingAura->HasEffectType(SPELL_AURA_CONTROL_VEHICLE))
    {
        Vehicle* veh = nullptr;
        if (GetOwner()->ToUnit())
            veh = GetOwner()->ToUnit()->GetVehicleKit();

        if (!veh)           // We should probably just let it stack. Vehicle system will prevent undefined behaviour later
            return true;

        if (!veh->GetAvailableSeatCount())
            return false;   // No empty seat available

        return true; // Empty seat available (skip rest)
    }

    if (HasEffectType(SPELL_AURA_SHOW_CONFIRMATION_PROMPT) || HasEffectType(SPELL_AURA_SHOW_CONFIRMATION_PROMPT_WITH_DIFFICULTY))
        if (existingAura->HasEffectType(SPELL_AURA_SHOW_CONFIRMATION_PROMPT) || existingAura->HasEffectType(SPELL_AURA_SHOW_CONFIRMATION_PROMPT_WITH_DIFFICULTY))
            return false;

    // spell of same spell rank chain
    if (m_spellInfo->IsRankOf(existingSpellInfo))
    {
        if (GetCastItemGUID() && existingAura->GetCastItemGUID())
            if (GetCastItemGUID() == existingAura->GetCastItemGUID())
                return false;

        // don't allow passive area auras to stack
        if (m_spellInfo->IsMultiSlotAura() && !IsArea())
            return true;

        // same spell with same caster should not stack
        return false;
    }

    return true;
}

bool Aura::IsProcOnCooldown() const
{
    /*if (m_procCooldown)
    {
        if (m_procCooldown > time(NULL))
            return true;
    }*/
    return false;
}

void Aura::AddProcCooldown(uint32 /*msec*/)
{
    //m_procCooldown = time(NULL) + msec;
}

void Aura::PrepareProcToTrigger(AuraApplication* /*aurApp*/, ProcEventInfo& /*eventInfo*/)
{
    // take one charge, aura expiration will be handled in Aura::TriggerProcOnEvent (if needed)
    if (HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES))
    {
        --m_procCharges;
        SetNeedClientUpdateForTargets();
    }

    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());

    ASSERT(procEntry);

    // cooldowns should be added to the whole aura (see 51698 area aura)
    AddProcCooldown(procEntry->cooldown);
}

bool Aura::IsProcTriggeredOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo) const
{
    SpellProcEntry const* procEntry = sSpellMgr->GetSpellProcEntry(GetId());
    // only auras with spell proc entry can trigger proc
    if (!procEntry)
        return false;

    // check if we have charges to proc with
    if (HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES) && !GetCharges())
        return false;

    // check proc cooldown
    if (IsProcOnCooldown())
        return false;

    // TODO:
    // something about triggered spells triggering, and add extra attack effect

    // do checks against db data
    if (!sSpellMgr->CanSpellTriggerProcOnEvent(*procEntry, eventInfo))
        return false;

    // do checks using conditions table
    ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_SPELL_PROC, GetId());
    ConditionSourceInfo condInfo = ConditionSourceInfo(eventInfo.GetActor(), eventInfo.GetActionTarget());
    if (!sConditionMgr->IsObjectMeetToConditions(condInfo, conditions))
        return false;

    // TODO:
    // do allow additional requirements for procs
    // this is needed because this is the last moment in which you can prevent aura charge drop on proc
    // and possibly a way to prevent default checks (if there're going to be any)

    // Check if current equipment meets aura requirements
    // do that only for passive spells
    // TODO: this needs to be unified for all kinds of auras
    Unit* target = aurApp->GetTarget();
    if (IsPassive() && target->IsPlayer())
    {
        if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            if (target->ToPlayer()->IsInFeralForm())
                return false;

            if (eventInfo.GetDamageInfo())
            {
                WeaponAttackType attType = eventInfo.GetDamageInfo()->GetAttackType();
                Item* item = nullptr;
                if (attType == BASE_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                else if (attType == OFF_ATTACK)
                    item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

                if (!item || item->CantBeUse() || item->GetTemplate()->GetClass() != ITEM_CLASS_WEAPON)
                    return false;

                if (GetSpellInfo()->EquippedItemSubClassMask && !((1<<item->GetTemplate()->GetSubClass()) & GetSpellInfo()->EquippedItemSubClassMask))
                    return false;
            }
        }
        else if (GetSpellInfo()->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item* item = target->ToPlayer()->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (!item || item->CantBeUse() || item->GetTemplate()->GetClass() != ITEM_CLASS_ARMOR)
                return false;

            if (GetSpellInfo()->EquippedItemSubClassMask && !((1<<item->GetTemplate()->GetSubClass()) & GetSpellInfo()->EquippedItemSubClassMask))
                return false;
        }
    }

    return roll_chance_f(CalcProcChance(*procEntry, eventInfo));
}

float Aura::CalcProcChance(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo) const
{
    float chance = procEntry.chance;
    // calculate chances depending on unit with caster's data
    // so talents modifying chances and judgements will have properly calculated proc chance
    if (Unit* caster = GetCaster())
    {
        // calculate ppm chance if present and we're using weapon
        if (eventInfo.GetDamageInfo() && procEntry.ratePerMinute != 0)
        {
            uint32 WeaponSpeed = caster->GetAttackTime(eventInfo.GetDamageInfo()->GetAttackType());
            chance = caster->GetPPMProcChance(WeaponSpeed, procEntry.ratePerMinute, GetSpellInfo());
        }
        // apply chance modifer aura, applies also to ppm chance (see improved judgement of light spell)
        if (Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(GetId(), SPELLMOD_CHANCE_OF_SUCCESS, chance);
    }
    return chance;
}

void Aura::TriggerProcOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo)
{
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (GetSpellInfo()->EffectMask < uint32(1 << i))
            break;

        if (aurApp->HasEffect(i))
            // OnEffectProc / AfterEffectProc hooks handled in AuraEffect::HandleProc()
            GetEffect(i)->HandleProc(aurApp, eventInfo, static_cast<SpellEffIndex>(i));
    }

    // Remove aura if we've used last charge to proc
    if (HasAuraAttribute(AURA_ATTR_IS_USING_CHARGES) && !GetCharges())
        Remove();
}

void Aura::_DeleteRemovedApplications()
{
    while (!m_removedApplications.empty())
        m_removedApplications.pop_front();
}

void Aura::LoadScripts()
{
    sScriptMgr->CreateAuraScripts(m_spellInfo->Id, m_loadedScripts, this);
    for (auto& loadedScript : m_loadedScripts)
    {
        TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::LoadScripts: Script `%s` for aura `%u` is loaded now", loadedScript->_GetScriptName()->c_str(), m_spellInfo->Id);
        loadedScript->Register();
    }
}

void Aura::CallScriptCheckTargetsListHandlers(std::list<Unit*>& unitTargets)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_CHECK_TARGETS_LIST);
        auto hookItrEnd = loadedScript->DoCheckTargetsList.end(), hookItr = loadedScript->DoCheckTargetsList.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(loadedScript, unitTargets);
        loadedScript->_FinishScriptCall();
    }
}

bool Aura::CallScriptCheckAreaTargetHandlers(Unit* target)
{
    if (IsRemoved())
        return false;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_CHECK_AREA_TARGET);
        auto hookItrEnd = loadedScript->DoCheckAreaTarget.end(), hookItr = loadedScript->DoCheckAreaTarget.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (!(*hookItr).Call(loadedScript, target))
                return false;
        loadedScript->_FinishScriptCall();
    }
    return true;
}

void Aura::CallScriptDispel(DispelInfo* dispelInfo)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_DISPEL);
        auto hookItrEnd = loadedScript->OnDispel.end(), hookItr = loadedScript->OnDispel.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(loadedScript, dispelInfo);
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptAfterDispel(DispelInfo* dispelInfo)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_AFTER_DISPEL);
        auto hookItrEnd = loadedScript->AfterDispel.end(), hookItr = loadedScript->AfterDispel.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(loadedScript, dispelInfo);
        loadedScript->_FinishScriptCall();
    }
}

bool Aura::CallScriptEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    if (IsRemoved())
        return false;

    auto preventDefault = false;
    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_APPLY, aurApp);
        auto effEndItr = loadedScript->OnEffectApply.end(), effItr = loadedScript->OnEffectApply.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, mode);
        }
        if (!preventDefault)
            preventDefault = loadedScript->_IsDefaultActionPrevented();
        loadedScript->_FinishScriptCall();
    }
    return preventDefault;
}

bool Aura::CallScriptEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    auto preventDefault = false;
    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_REMOVE, aurApp);
        auto effEndItr = loadedScript->OnEffectRemove.end(), effItr = loadedScript->OnEffectRemove.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, mode);
        }
        if (!preventDefault)
            preventDefault = loadedScript->_IsDefaultActionPrevented();
        loadedScript->_FinishScriptCall();
    }
    return preventDefault;
}

void Aura::CallScriptAfterEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_APPLY, aurApp);
        auto effEndItr = loadedScript->AfterEffectApply.end(), effItr = loadedScript->AfterEffectApply.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, mode);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptAfterEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode)
{
    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_REMOVE, aurApp);
        auto effEndItr = loadedScript->AfterEffectRemove.end(), effItr = loadedScript->AfterEffectRemove.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, mode);
        }
        loadedScript->_FinishScriptCall();
    }
}

bool Aura::CallScriptEffectPeriodicHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp)
{
    if (IsRemoved())
        return false;

    auto preventDefault = false;
    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_PERIODIC, aurApp);
        auto effEndItr = loadedScript->OnEffectPeriodic.end(), effItr = loadedScript->OnEffectPeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff);
        }
        if (!preventDefault)
            preventDefault = loadedScript->_IsDefaultActionPrevented();
        loadedScript->_FinishScriptCall();
    }
    return preventDefault;
}

void Aura::CallScriptEffectUpdateHandlers(uint32 diff, AuraEffect* aurEff)
{
    if (IsRemoved())
        return;

    for (std::vector<AuraScript*>::iterator loadedScript = m_loadedScripts.begin(), next; loadedScript != m_loadedScripts.end(); loadedScript = next)
    {
        next = loadedScript;
        ++next;
        (*loadedScript)->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_UPDATE);
        auto effEndItr = (*loadedScript)->OnEffectUpdate.end(), effItr = (*loadedScript)->OnEffectUpdate.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(*loadedScript, diff, aurEff);
        }
        (*loadedScript)->_FinishScriptCall();
    }
}

void Aura::CallScriptAuraUpdateHandlers(uint32 diff)
{
    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_ON_UPDATE);
        auto hookItrEnd = loadedScript->OnAuraUpdate.end(), hookItr = loadedScript->OnAuraUpdate.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(loadedScript, diff);
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectUpdatePeriodicHandlers(AuraEffect* aurEff)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_UPDATE_PERIODIC);
        auto effEndItr = loadedScript->OnEffectUpdatePeriodic.end(), effItr = loadedScript->OnEffectUpdatePeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectCalcAmountHandlers(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT);
        auto effEndItr = loadedScript->DoEffectCalcAmount.end(), effItr = loadedScript->DoEffectCalcAmount.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, amount, canBeRecalculated);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectBeforeCalcAmountHandlers(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT);
        auto effEndItr = loadedScript->DoEffectBeforeCalcAmount.end(), effItr = loadedScript->DoEffectBeforeCalcAmount.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, amount, canBeRecalculated);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptCalcMaxDurationHandlers(int32& maxDuration)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_CALC_MAX_DURATION);
        auto hookItrEnd = loadedScript->DoCalcMaxDuration.end(), hookItr = loadedScript->DoCalcMaxDuration.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(loadedScript, maxDuration);
        loadedScript->_FinishScriptCall();
    }
    CalculateDurationFromDummy(maxDuration);
}

void Aura::CallScriptEffectChangeTickDamageHandlers(AuraEffect const* aurEff, float & amount, Unit* target)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_AMOUNT);
        auto effEndItr = loadedScript->DoEffectChangeTickDamage.end(), effItr = loadedScript->DoEffectChangeTickDamage.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, amount, target);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectCalcPeriodicHandlers(AuraEffect const* aurEff, bool & isPeriodic, int32 & amplitude)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_PERIODIC);
        auto effEndItr = loadedScript->DoEffectCalcPeriodic.end(), effItr = loadedScript->DoEffectCalcPeriodic.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, isPeriodic, amplitude);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectCalcSpellModHandlers(AuraEffect const* aurEff, SpellModifier* & spellMod)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_CALC_SPELLMOD);
        auto effEndItr = loadedScript->DoEffectCalcSpellMod.end(), effItr = loadedScript->DoEffectCalcSpellMod.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, spellMod);
        }
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount, bool& defaultPrevented)
{
    if (IsRemoved())
        return;

    for (auto& loadedScript : m_loadedScripts)
    {
        loadedScript->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_ABSORB, aurApp);
        auto effEndItr = loadedScript->OnEffectAbsorb.end(), effItr = loadedScript->OnEffectAbsorb.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(loadedScript, aurEff, dmgInfo, absorbAmount);
        }
        defaultPrevented = loadedScript->_IsDefaultActionPrevented();
        loadedScript->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectAfterAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
    {
        itr->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_ABSORB, aurApp);
        auto effEndItr = itr->AfterEffectAbsorb.end(), effItr = itr->AfterEffectAbsorb.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(itr, aurEff, dmgInfo, absorbAmount);
        }
        itr->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount, bool & /*defaultPrevented*/)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
    {
        itr->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_MANASHIELD, aurApp);
        auto effEndItr = itr->OnEffectManaShield.end(), effItr = itr->OnEffectManaShield.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(itr, aurEff, dmgInfo, absorbAmount);
        }
        itr->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectAfterManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
    {
        itr->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_AFTER_MANASHIELD, aurApp);
        auto effEndItr = itr->AfterEffectManaShield.end(), effItr = itr->AfterEffectManaShield.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(itr, aurEff, dmgInfo, absorbAmount);
        }
        itr->_FinishScriptCall();
    }
}

void Aura::CallScriptEffectSplitDamageHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
    {
        itr->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_SPLIT_DAMAGE, aurApp);
        auto effEndItr = itr->OnEffectSplitDamage.end(), effItr = itr->OnEffectSplitDamage.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(itr, aurEff, dmgInfo, absorbAmount);
        }
        itr->_FinishScriptCall();
    }
}

void Aura::SetScriptData(uint32 type, uint32 data)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
        itr->SetData(type, data);
}

void Aura::SetScriptGuid(uint32 type, ObjectGuid const& data)
{
    if (IsRemoved())
        return;

    for (auto& itr : m_loadedScripts)
        itr->SetGuid(type, data);
}

bool Aura::CallScriptEffectProcHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo)
{
    if (IsRemoved())
        return false;

    auto preventDefault = false;
    for (auto& itr : m_loadedScripts)
    {
        itr->_PrepareScriptCall(AURA_SCRIPT_HOOK_EFFECT_PROC, aurApp);
        auto effEndItr = itr->OnEffectProc.end(), effItr = itr->OnEffectProc.begin();
        for (; effItr != effEndItr; ++effItr)
        {
            if ((*effItr).IsEffectAffected(m_spellInfo, aurEff->GetEffIndex()))
                (*effItr).Call(itr, aurEff, eventInfo);
        }
        if (!preventDefault)
            preventDefault = itr->_IsDefaultActionPrevented();
        itr->_FinishScriptCall();
    }
    return preventDefault;
}

UnitAura::UnitAura(SpellInfo const* spellproto, uint32 /*effMask*/, WorldObject* owner, Unit* caster, float* /*baseAmount*/, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, SpellPowerCost* powerCost)
    : Aura(spellproto, owner, caster, castItem, casterGUID, stackAmount, powerCost)
{
    m_AuraDRGroup = DIMINISHING_NONE;
}

void UnitAura::_ApplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr aurApp)
{
    Aura::_ApplyForTarget(target, caster, aurApp);

    // register aura diminishing on apply
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, true);
}

void UnitAura::_UnapplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr aurApp)
{
    Aura::_UnapplyForTarget(target, caster, aurApp);

    // unregister aura diminishing (and store last time)
    if (DiminishingGroup group = GetDiminishGroup())
        target->ApplyDiminishingAura(group, false);
}

void UnitAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    GetUnitOwner()->RemoveOwnedAura(this, removeMode);
}

void UnitAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster)
{
    volatile uint32 auraId = GetId();

    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (GetSpellInfo()->EffectMask < uint32(1 << effIndex))
            break;

        if (!HasEffect(effIndex))
            continue;
        UnitList targetList;
        // non-area aura
        if (GetSpellInfo()->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_APPLY_AURA)
            targetList.push_back(GetUnitOwner());
        else
        {
            if (!GetUnitOwner()->HasUnitState(UNIT_STATE_ISOLATED))
            {
                float radius = GetSpellInfo()->GetEffect(effIndex, m_diffMode)->CalcRadius(caster);
                switch (GetSpellInfo()->GetEffect(effIndex, m_diffMode)->Effect)
                {
                    case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
                    case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
                    {
                        std::list<Player*> pList;
                        targetList.push_back(GetUnitOwner());
                        Trinity::AnyGroupedUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius, GetSpellInfo()->GetEffect(effIndex, m_diffMode)->Effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID);
                        Trinity::PlayerListSearcher<Trinity::AnyGroupedUnitInObjectRangeCheck> searcher(GetUnitOwner(), pList, u_check);
                        Trinity::VisitNearbyObject(GetOwner(), radius, searcher);

                        for (auto itr : pList)
                            targetList.push_back(itr);

                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
                    {
                        targetList.push_back(GetUnitOwner());
                        Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius);
                        Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        Trinity::VisitNearbyObject(GetOwner(), radius, searcher);
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
                    {
                        Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(GetUnitOwner(), GetUnitOwner(), radius); // No GetCharmer in searcher
                        Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(GetUnitOwner(), targetList, u_check);
                        if (GetUnitOwner()->isAlive())
                            Trinity::VisitNearbyObject(GetOwner(), radius, searcher);
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_PET:
                    {
                        if (GetUnitOwner()->isAlive())
                            targetList.push_back(GetUnitOwner());
                        if (Unit* owner = GetUnitOwner()->GetCharmerOrOwner())
                            if (GetUnitOwner()->IsWithinDistInMap(owner, radius))
                                targetList.push_back(owner);
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
                    {
                        if (Unit* owner = GetUnitOwner()->GetCharmerOrOwner())
                        {
                            if (GetUnitOwner()->IsWithinDistInMap(owner, radius))
                                targetList.push_back(owner);
                        }
                        else
                            targetList.push_back(GetUnitOwner());
                        break;
                    }
                    case SPELL_EFFECT_APPLY_AURA_ON_PET_OR_SELF:
                    case SPELL_EFFECT_APPLY_AURA_WITH_VALUE:
                    {
                        if (!(m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_SELF)))
                            targetList.push_back(GetUnitOwner());
                        for (auto& guid : GetUnitOwner()->m_Controlled)
                            if(Unit* unit = ObjectAccessor::GetUnit(*GetUnitOwner(), guid))
                                targetList.push_back(unit);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        for (auto& itr : targetList)
        {
            auto existing = targets.find(itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
            {
                if (m_spellInfo->HasAttribute(SPELL_ATTR7_CONSOLIDATED_RAID_BUFF))
                    if (!itr->IsPlayer() && itr != caster)
                        if (itr->GetOwner() && itr->GetOwner()->IsPlayer())
                            continue;

                targets[itr] = 1<<effIndex;
            }
        }
    }
}

DynObjAura::DynObjAura(SpellInfo const* spellproto, uint32 /*effMask*/, WorldObject* owner, Unit* caster, float* /*baseAmount*/, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, SpellPowerCost* powerCost)
    : Aura(spellproto, owner, caster, castItem, casterGUID, stackAmount, powerCost)
{
}

void DynObjAura::Remove(AuraRemoveMode removeMode)
{
    if (IsRemoved())
        return;
    _Remove(removeMode);
}

void DynObjAura::FillTargetMap(std::map<Unit*, uint32> & targets, Unit* /*caster*/)
{
    Unit* dynObjOwnerCaster = GetDynobjOwner()->GetCaster();
    float radius = GetDynobjOwner()->GetRadius();

    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (GetSpellInfo()->EffectMask < uint32(1 << effIndex))
            break;

        if (!HasEffect(effIndex))
            continue;
        UnitList targetList;
        if (GetSpellInfo()->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_DEST_DYNOBJ_ALLY
            || GetSpellInfo()->GetEffect(effIndex, m_diffMode)->TargetB.GetTarget() == TARGET_UNIT_DEST_AREA_ALLY)
        {
            Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            Trinity::VisitNearbyObject(GetDynobjOwner(), radius, searcher);
        }
        else
        {
            Trinity::AnyAoETargetUnitInObjectRangeCheck u_check(GetDynobjOwner(), dynObjOwnerCaster, radius);
            Trinity::UnitListSearcher<Trinity::AnyAoETargetUnitInObjectRangeCheck> searcher(GetDynobjOwner(), targetList, u_check);
            Trinity::VisitNearbyObject(GetDynobjOwner(), radius, searcher);
        }

        CallScriptCheckTargetsListHandlers(targetList);
        for (auto& itr : targetList)
        {
            auto existing = targets.find(itr);
            if (existing != targets.end())
                existing->second |= 1<<effIndex;
            else
                targets[itr] = 1<<effIndex;
        }
    }
}

void Aura::SetAuraTimer(int32 time, ObjectGuid guid)
{
    if(GetDuration() == -1 || GetDuration() > time)
    {
        SetDuration(time);
        SetMaxDuration(time);
        if(AuraApplication *aur = GetApplicationOfTarget(!guid.IsEmpty() ? guid : m_casterGuid))
            aur->ClientUpdate();
    }
}

uint32 Aura::CalcAgonyTickDamage(uint32 damage)
{
    uint16 stack = GetStackAmount();
    int16 maxStackAmount = m_spellInfo->GetAuraOptions(GetSpawnMode())->CumulativeAura;

    if (Unit* caster = GetCaster())
        if (Player* modOwner = caster->GetSpellModOwner())
        {
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT, maxStackAmount);
            modOwner->ApplySpellMod(GetId(), SPELLMOD_STACK_AMOUNT2, maxStackAmount);
        }

    if (AuraEffect* eff = GetEffect(EFFECT_0))
    {
        if (stack < maxStackAmount)
            m_stackAmount = stack + 1;

        damage = eff->GetAmount() * m_stackAmount;
        eff->SetAmount(damage);

        if (AuraEffect* eff1 = GetEffect(EFFECT_1))
            eff1->SetAmount(damage);
    }
    return damage;
}

void Aura::UpdateConcatenateAura(Unit* caster, float amount, int32 effIndex, int8 type)
{
    if(!caster)
        return;

    switch (type)
    {
        case CONCATENATE_ON_UPDATE_AMOUNT: // 0
        {
            if (std::vector<SpellConcatenateAura> const* spellConcatenateAura = sSpellMgr->GetSpellConcatenateUpdate(GetId()))
            {
                for (const auto& itr : *spellConcatenateAura)
                {
                    if(effIndex != itr.effectSpell)
                        continue;

                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_UPDATE_AMOUNT Id %i amount %i effIndex %i type %i option %i", GetId(), amount, effIndex, type, itr->option);

                    Unit* _target = caster;
                    if (itr.target)
                        _target = caster->GetUnitForLinkedSpell(caster, caster, itr.target);
                    if(!_target)
                        continue;
                    AuraEffect* effectAura = _target->GetAuraEffect(itr.auraId, itr.effectAura);
                    if(!effectAura)
                        continue;

                    if (itr.option & CONCATENATE_CHANGE_AMOUNT) // 0x001 auraId set amount from spellid
                        effectAura->ChangeAmount(amount);

                    if (itr.option & CONCATENATE_RECALCULATE_AURA) // 0x002 auraId recalculate amount when spellid change amount
                    {
                        effectAura->SetCanBeRecalculated(true);
                        effectAura->RecalculateAmount();
                    }
                    // if (itr->option & CONCATENATE_RECALCULATE_SPELL) // 0x004 when auraId is apply spellid is recalculate amount
                        // RecalculateAmountOfEffects(true);

                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_UPDATE_AMOUNT end Id %i amount %i effIndex %i type %i option %i GetAmount %i", GetId(), amount, effIndex, type, itr->option, effectAura->GetAmount());
                }
            }
            break;
        }
        case CONCATENATE_ON_APPLY_AURA: // 1 
        {
            if (std::vector<SpellConcatenateAura> const* spellConcatenateAura = sSpellMgr->GetSpellConcatenateApply(GetId()))
            {
                for (const auto& itr : *spellConcatenateAura)
                {
                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_APPLY_AURA start Id %i amount %i effIndex %i type %i option %i", GetId(), amount, effIndex, type, itr->option);

                    Unit* _caster = caster;
                    if (itr.caster)
                        _caster = caster->GetUnitForLinkedSpell(caster, caster, itr.caster);
                    if(!_caster)
                        continue;
                    AuraEffect* effectAura = GetEffect(itr.effectAura);
                    if(!effectAura)
                        continue;

                    if (itr.option & CONCATENATE_CHANGE_AMOUNT) // 0x001 auraId set amount when auraId is apply
                    {
                        if (AuraEffect* effectSpell = _caster->GetAuraEffect(itr.spellid, itr.effectSpell))
                            effectAura->ChangeAmount(effectSpell->GetAmount());
                        else
                            effectAura->ChangeAmount(0);
                    }

                    if (itr.option & CONCATENATE_RECALCULATE_AURA) // 0x002 when auraId is apply spellid is recalculate amount
                        RecalculateAmountOfEffects(true);

                    if (itr.option & CONCATENATE_RECALCULATE_SPELL) // 0x004 when auraId is apply spellid is recalculate amount
                        if (_caster->GetAura(itr.spellid))
                        {
                            uint32 auraId = itr.spellid;
                            _caster->AddDelayedEvent(1000, [_caster, auraId]() -> void
                            {
                                if (Aura* m_aura = _caster->GetAura(auraId))
                                    m_aura->RecalculateAmountOfEffects(true);
                            });
                        }
                            // auraSpell->RecalculateAmountOfEffects(true);

                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_APPLY_AURA end Id %i amount %i effIndex %i type %i option %i GetAmount %i", GetId(), amount, effIndex, type, itr->option, effectAura->GetAmount());
                }
            }
            break;
        }
        case CONCATENATE_ON_REMOVE_AURA: // 2
        {
            if (std::vector<SpellConcatenateAura> const* spellConcatenateAura = sSpellMgr->GetSpellConcatenateUpdate(-int32(GetId()))) // int32(GetId()) ?
            {
                for (const auto& itr : *spellConcatenateAura)
                {
                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_REMOVE_AURA start Id %i amount %i effIndex %i type %i option %i", GetId(), amount, effIndex, type, itr->option);

                    Unit* _target = caster;
                    if (itr.target)
                        _target = caster->GetUnitForLinkedSpell(caster, caster, itr.target);
                    if(!_target)
                        continue;
                    AuraEffect* effectAura = _target->GetAuraEffect(itr.auraId, itr.effectAura);
                    if(!effectAura)
                        continue;

                    if (itr.option & CONCATENATE_CHANGE_AMOUNT) // 0x001 auraId set amount to 0 when spellid remove
                        effectAura->ChangeAmount(0);

                    if (itr.option & CONCATENATE_RECALCULATE_AURA) // 0x002 auraId recalculate amount when spellid remove
                    {
                        effectAura->SetCanBeRecalculated(true);
                        effectAura->RecalculateAmount();
                    }

                    if (itr.option & CONCATENATE_RECALCULATE_SPELL) // 0x004 when auraId is apply spellid is recalculate amount
                        if (effectAura->GetBase())
                        {
                            uint32 auraId = itr.auraId;
                            _target->AddDelayedEvent(100, [_target, auraId]() -> void
                            {
                                if (Aura* m_aura = _target->GetAura(auraId))
                                    m_aura->RecalculateAmountOfEffects(true);
                            });
                        }
                            // _aura->RecalculateAmountOfEffects(true);

                    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Aura::UpdateConcatenateAura CONCATENATE_ON_REMOVE_AURA end Id %i amount %i effIndex %i type %i option %i GetAmount %i", GetId(), amount, effIndex, type, itr->option, effectAura->GetAmount());
                }
            }
            break;
        }
        default:
            break;
    }
}
