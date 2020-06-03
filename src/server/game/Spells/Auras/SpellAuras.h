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

#ifndef TRINITY_SPELLAURAS_H
#define TRINITY_SPELLAURAS_H

#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Containers.h"
#include "HashFuctor.h"

class Unit;
class SpellInfo;
struct SpellModifier;
struct ProcTriggerSpell;
struct SpellProcEntry;

// forward decl
class AuraEffect;
class Aura;
class DynamicObject;
class AreaTrigger;
class AuraScript;
class ProcInfo;

namespace WorldPackets
{
    namespace Spells
    {
        struct AuraInfo;
        class LossOfControlAuraUpdate;
    }
}

// update aura target map every 500 ms instead of every update - reduce amount of grid searcher calls
#define UPDATE_TARGET_MAP_INTERVAL 500

class AuraApplication
{
    friend void Unit::_ApplyAura(AuraApplication * aurApp, uint32 effMask);
    friend void Unit::_UnapplyAura(/*AuraApplicationMap*/std::multimap<uint32, AuraApplicationPtr>::iterator &i, AuraRemoveMode removeMode);
    friend void Unit::_ApplyAuraEffect(Aura* aura, uint32 effIndex);
    friend void Unit::RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode);
    friend void Unit::RemoveOwnedAuraAll();
    friend AuraApplication * Unit::_CreateAuraApplication(Aura* aura, uint32 effMask);
        Unit* const _target;
        Aura* const _base;
        AuraRemoveMode _removeMode:8;                  // Store info for know remove aura reason
        uint8 _slot;                                   // Aura slot on unit
        uint8 _flags;                                  // Aura info flag
        uint32 _effectMask;
        uint32 _effectsToApply;                         // Used only at spell hit to determine which effect should be applied
        bool _needClientUpdate:1;

    public:
        void _Remove();
        void _InitFlags(Unit* caster, uint32 effMask);
        void _HandleEffect(uint8 effIndex, bool apply);

        explicit AuraApplication(Unit* target, Unit* caster, Aura* base, uint32 effMask);

        Unit* GetTarget() const { return _target; }
        Aura* GetBase() const { return _base; }

        uint8 GetSlot() const { return _slot; }
        uint8 GetFlags() const { return _flags; }
        uint32 GetEffectMask() const { return _effectMask; }
        bool HasEffect(uint8 effect) const { ASSERT(effect < MAX_SPELL_EFFECTS);  return (_effectMask & (1<<effect)) != 0; }
        bool IsPositive() const { return (_flags & AFLAG_POSITIVE) != 0; }
        bool IsSelfcasted() const { return !(_flags & AFLAG_NOCASTER); }
        uint32 GetEffectsToApply() const { return _effectsToApply; }

        void SetRemoveMode(AuraRemoveMode mode) { _removeMode = mode; }
        AuraRemoveMode GetRemoveMode() const {return _removeMode;}

        void SetNeedClientUpdate();
        bool IsNeedClientUpdate() const { return _needClientUpdate; }
        void BuildUpdatePacket(WorldPackets::Spells::AuraInfo& auraInfo, bool remove, uint32 overrideAura = 0);
        void ClientUpdate(bool remove = false);
        void BuildLossOfControlPacket(WorldPackets::Spells::LossOfControlAuraUpdate& loosOfControl);
};

typedef std::array<AuraEffect*, MAX_SPELL_EFFECTS> AuraEffectVector;

class Aura
{
    friend Aura* Unit::_TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint32 effMask, Unit* caster, float *baseAmount, Item* castItem, ObjectGuid casterGUID);
    public:
        void SetAuraTimer(int32 time, ObjectGuid guid = ObjectGuid::Empty);
        typedef cds::container::FeldmanHashMap< cds::gc::HP, ObjectGuid, AuraApplicationPtr, guidTraits > ApplicationMap;

        static uint32 BuildEffectMaskForOwner(SpellInfo const* spellProto, uint32 avalibleEffectMask, WorldObject* owner);
        static Aura* TryRefreshStackOrCreate(SpellInfo const* spellproto, uint32 tryEffMask, WorldObject* owner, Unit* caster, float* baseAmount = nullptr, Item* castItem = nullptr, ObjectGuid casterGUID = ObjectGuid::Empty, bool* refresh = nullptr, uint16 stackAmount = 0, Spell* spell = nullptr);
        static Aura* TryCreate(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float *baseAmount = nullptr, Item* castItem = nullptr, ObjectGuid casterGUID = ObjectGuid::Empty);
        static Aura* Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float* baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount = 0, Spell* spell = nullptr);
        explicit Aura(SpellInfo const* spellproto, WorldObject* owner, Unit* caster, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount = 0, SpellPowerCost* powerCost = nullptr);
        void _InitEffects(uint32 effMask, Unit* caster, float *baseAmount);
        virtual ~Aura();
        static uint32 CalculateEffMaskFromDummy(Unit* caster, WorldObject* target, uint32 effMask, SpellInfo const* spellproto);
        void CalculateDurationFromDummy(int32 &duration);

        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
        uint32 GetId() const{ return m_auraId; }

        ObjectGuid GetCastItemGUID() const { return m_castItemGuid; }
        ObjectGuid GetCasterGUID() const { return m_casterGuid; }
        Unit* GetCaster() const;
        WorldObject* GetOwner() const { return m_owner; }
        WorldObject* ChangeOwner(WorldObject* owner) { return m_owner = owner; }
        Unit* GetUnitOwner() const
        {
            if(GetType() == UNIT_AURA_TYPE) return m_owner->ToUnit();
            return nullptr;
        }
        DynamicObject* GetDynobjOwner() const { ASSERT(GetType() == DYNOBJ_AURA_TYPE); return (DynamicObject*)m_owner; }
        void SetSpellDynamicObject(ObjectGuid dynObj) { m_spellDynObjGuid = dynObj; }
        ObjectGuid GetSpellDynamicObject() const { return m_spellDynObjGuid; }
        void SetSpellAreaTrigger(ObjectGuid areaTr) { m_spellAreaTrGuid = areaTr; }
        ObjectGuid GetSpellAreaTrigger() const { return m_spellAreaTrGuid; }
        void SetTriggeredAuraEff(AuraEffect const* trigger) { m_triggeredByAura = trigger;}
        AuraEffect const* GetTriggeredAuraEff() const { return m_triggeredByAura; }
        uint32 CalcAgonyTickDamage(uint32 damage = 0);
        void SetSpellVisual(uint32 visual) { m_SpellVisual = visual;}
        uint32 GetSpellVisual() const { return m_SpellVisual; }
        void SetTriggeredCastFlags(uint32 flag) { _triggeredCastFlags = flag; }
        uint32 GetTriggeredCastFlags() const { return _triggeredCastFlags; }

        AuraObjectType GetType() const;

        virtual void _ApplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr auraApp);
        virtual void _UnapplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr auraApp);
        void _Remove(AuraRemoveMode removeMode);
        virtual void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) = 0;

        virtual void FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster) = 0;
        void UpdateTargetMap(Unit* caster, bool apply = true);

        void _RegisterForTargets() {Unit* caster = GetCaster(); UpdateTargetMap(caster, false);}
        void ApplyForTargets() {Unit* caster = GetCaster(); UpdateTargetMap(caster, true);}
        void _ApplyEffectForTargets(uint8 effIndex);

        void UpdateOwner(uint32 diff, WorldObject* owner);
        void Update(uint32 diff, Unit* caster);

        time_t GetApplyTime() const { return m_applyTime; }
        uint32 GetApplyMSTime() const { return m_applyMSTime; }
        int32 GetMaxDuration() const { return m_maxDuration; }
        void SetMaxDuration(int32 duration) { m_maxDuration = duration; }
        int32 CalcMaxDuration() { return CalcMaxDuration(GetCaster()); }
        int32 CalcMaxDuration(Unit* caster);
        int32 GetDuration() const { return m_duration; }
        int32 GetAllDuration() const { return m_allDuration; }
        void SetDuration(int32 duration, bool withMods = false);
        void RefreshDuration();
        void RefreshTimers();
        bool IsExpired() const { return !GetDuration();}
        bool IsPermanent() const { return GetMaxDuration() == -1; }

        uint8 GetCharges() const { return m_procCharges; }
        void SetCharges(uint8 charges);
        uint8 CalcMaxCharges(Unit* caster, bool add = false) const;
        uint8 CalcMaxCharges() const { return CalcMaxCharges(GetCaster(), true); }
        bool ModCharges(int32 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        bool DropCharge(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) { return ModCharges(-1, removeMode); }

        uint16 GetStackAmount() const { return m_stackAmount; }
        void SetStackAmount(uint16 num);
        void SetMaxStackAmount();
        bool ModStackAmount(int16 num, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        //Position from spell
        void AddDst(Position const* pos) { _positions.push_back(*pos); }
        Position const* GetDstPos(uint32 pointId = 0) const
        {
            if(_positions.empty()) return nullptr;
            return &_positions[pointId];
        }
        std::vector<Position>* GetDstVector() { return &_positions; }
        void SetDstVector(std::vector<Position>* poss) { _positions = *poss; }

        void RefreshSpellMods();

        bool HasMoreThanOneEffectForType(AuraType auraType) const;

        uint16 GetCasterLevel() const { return m_casterLevel; }

        bool IsArea() const;
        bool IsPassive() const;
        bool IsDeathPersistent() const;
        bool IsRemovedOnShapeLost(Unit* target) const { return (GetCasterGUID() == target->GetGUID() && m_spellInfo->Shapeshift.ShapeshiftMask && !(m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_NEED_SHAPESHIFT)) && !(m_spellInfo->HasAttribute(SPELL_ATTR0_NOT_SHAPESHIFT))); }
        bool CanBeSaved() const;
        bool IsRemoved() const { return m_isRemoved != 0; }
        // Single cast aura helpers
        bool IsMultiSingleTarget() const;
        uint32 GetMultiSingleTargetCount() const;
        bool IsSingleTargetWith(Aura const* aura) const;
        void UnregisterSingleTarget();
        void UnregisterMultiSingleTarget();
        void UnregisterCasterAuras(AuraRemoveMode removeMode);
        int32 CalcDispelChance(Unit* auraTarget, bool offensive) const;

        bool HasAuraAttribute(AuraAttr attribute) const;
        void SetAuraAttribute(AuraAttr attribute, bool apply = true);

        void SetLoadedState(int32 maxduration, int32 duration, int32 charges, uint8 stackamount, uint32 recalculateMask, int32 * amount);

        // helpers for aura effects
        bool HasEffect(uint8 effIndex) const { return GetEffect(effIndex) != nullptr; }
        bool HasEffectType(AuraType type) const;
        AuraEffect* GetEffect(uint8 effIndex) const { ASSERT (effIndex < MAX_SPELL_EFFECTS); return m_effects[effIndex]; }
        uint32 GetEffectMask() const { uint32 effMask = 0; for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i) if (m_effects[i]) effMask |= 1<<i; return effMask; }
        void RecalculateAmountOfEffects(bool setCanRecalc = false);
        void HandleAllEffects(AuraApplication * aurApp, uint8 mode, bool apply);

        //Save list target for custom scripts work
        void SetEffectTargets(GuidList const& targets) { m_effect_targets = targets; }
        GuidList GetEffectTargets() { return m_effect_targets; }
        void AddEffectTarget(ObjectGuid const& targetGuid) { m_effect_targets.push_back(targetGuid); }
        void RemoveEffectTarget(ObjectGuid const& targetGuid) { m_effect_targets.remove(targetGuid); }
        void ClearEffectTarget() { m_effect_targets.clear(); }
        ObjectGuid GetRndEffectTarget() { return Trinity::Containers::SelectRandomContainerElement(m_effect_targets); }

        // Helpers for targets
        ApplicationMap const & GetApplicationMap();
        void GetApplicationList(std::list<AuraApplication*> & applicationList);
        AuraApplication * GetApplicationOfTarget (ObjectGuid const& guid);
        bool IsAppliedOnTarget(ObjectGuid const& guid);

        void SetNeedClientUpdateForTargets();
        void HandleAuraSpecificMods(AuraApplication const* aurApp, Unit* caster, bool apply, bool onReapply);
        bool CanBeAppliedOn(Unit* target);
        bool CheckAreaTarget(Unit* target);
        bool CanStackWith(Aura const* existingAura) const;
        uint8 GetSpawnMode() const { return m_diffMode; }

        // Proc system
        // this subsystem is not yet in use - the core of it is functional, but still some research has to be done
        // and some dependant problems fixed before it can replace old proc system (for example cooldown handling)
        // currently proc system functionality is implemented in Unit::ProcDamageAndSpell
        bool IsProcOnCooldown() const;
        void AddProcCooldown(uint32 msec);
        void PrepareProcToTrigger(AuraApplication* aurApp, ProcEventInfo& eventInfo);
        bool IsProcTriggeredOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo) const;
        float CalcProcChance(SpellProcEntry const& procEntry, ProcEventInfo& eventInfo) const;
        void TriggerProcOnEvent(AuraApplication* aurApp, ProcEventInfo& eventInfo);

        // AuraScript
        void LoadScripts();
        bool CallScriptCheckAreaTargetHandlers(Unit* target);
        void CallScriptCheckTargetsListHandlers(std::list<Unit*>& unitTargets);
        void CallScriptDispel(DispelInfo* dispelInfo);
        void CallScriptAfterDispel(DispelInfo* dispelInfo);
        bool CallScriptEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        bool CallScriptEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        void CallScriptAfterEffectApplyHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        void CallScriptAfterEffectRemoveHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, AuraEffectHandleModes mode);
        bool CallScriptEffectPeriodicHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp);
        void CallScriptEffectUpdateHandlers(uint32 diff, AuraEffect* aurEff);
        void CallScriptAuraUpdateHandlers(uint32 diff);
        void CallScriptEffectUpdatePeriodicHandlers(AuraEffect* aurEff);
        void CallScriptEffectCalcAmountHandlers(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated);
        void CallScriptEffectBeforeCalcAmountHandlers(AuraEffect const* aurEff, float & amount, bool & canBeRecalculated);
        void CallScriptCalcMaxDurationHandlers(int32& maxDuration);
        void CallScriptEffectChangeTickDamageHandlers(AuraEffect const* aurEff, float & amount, Unit* target);
        void CallScriptEffectCalcPeriodicHandlers(AuraEffect const* aurEff, bool & isPeriodic, int32 & amplitude);
        void CallScriptEffectCalcSpellModHandlers(AuraEffect const* aurEff, SpellModifier* & spellMod);
        void CallScriptEffectAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount, bool & defaultPrevented);
        void CallScriptEffectAfterAbsorbHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount);
        void CallScriptEffectManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount, bool & defaultPrevented);
        void CallScriptEffectAfterManaShieldHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount);
        void CallScriptEffectSplitDamageHandlers(AuraEffect* aurEff, AuraApplication const* aurApp, DamageInfo & dmgInfo, float & absorbAmount);
        void SetScriptData(uint32 type, uint32 data);
        void SetScriptGuid(uint32 type, ObjectGuid const& data);
        // Spell Proc Hooks
        bool CallScriptEffectProcHandlers(AuraEffect const* aurEff, AuraApplication const* aurApp, ProcEventInfo& eventInfo);

        std::vector<AuraScript*> m_loadedScripts;
        int32 m_damage_amount;
        std::vector<Position> _positions;
        void UpdateConcatenateAura(Unit* caster, float amount, int32 effIndex, int8 type = 0); // Default CONCATENATE_ON_UPDATE_AMOUNT

        void SetCustomData(int32 data) { m_customData = data; }
        int32 GetCustomData() { return m_customData; }
        void ModCustomData(int32 data) { m_customData += data; }

        SpellPowerCost m_powerCost;
        void SetPowerCost(SpellPowerCost power)
        {
            for (uint32 i = 0; i <= MAX_POWERS; ++i)
                m_powerCost[i] = power[i];
        }
        int32 GetPowerCost(int8 power)
        {
            if (power > MAX_POWERS || power < 0) return m_powerCost[MAX_POWERS];
            return m_powerCost[power];
        }
        int8 GetComboPoints() { return m_powerCost[POWER_COMBO_POINTS]; }
        SpellPowerData powerData;

        AuraEffectVector GetAuraEffects() const { return m_effects; }

        std::vector<uint32> durationByStack;

        float TimeMod;

    private:
        void _DeleteRemovedApplications();
    protected:
        SpellInfo const* const m_spellInfo;
        ObjectGuid const m_casterGuid;
        ObjectGuid const m_castItemGuid;                        // it is NOT safe to keep a pointer to the item because it may get deleted
        time_t const m_applyTime;
        uint32 const m_applyMSTime;
        WorldObject* m_owner;
        ObjectGuid m_spellDynObjGuid;
        ObjectGuid m_spellAreaTrGuid;
        AuraEffect const* m_triggeredByAura;
        ObjectGuid m_areaTrGuid;
        uint32 m_SpellVisual;
        uint32 _triggeredCastFlags;

        int32 m_maxDuration;                                // Max aura duration
        int32 m_duration;                                   // Current time
        int32 m_allDuration;                                // Duration from apply aura
        int32 m_timeCla;                                    // Timer for power per sec calcultion
        int32 m_updateTargetMapInterval;                    // Timer for UpdateTargetMapOfEffect

        uint16 m_casterLevel;                                // Aura level (store caster level for correct show level dep amount)
        uint8 m_procCharges;                                // Aura charges (0 for infinite)
        uint16 m_stackAmount;                                // Aura stack amount
        uint8 m_diffMode;
        int32 m_customData;
        uint32 m_auraId;

        AuraEffectVector m_effects;
        ApplicationMap m_applications;
        GuidList m_effect_targets;

        uint32 auraAttributes;

        std::atomic<uint64> m_isRemoved;

    private:
        Unit::AuraApplicationList m_removedApplications;
};

class UnitAura : public Aura
{
    friend Aura* Aura::Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float *baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, Spell* spell);
    protected:
        explicit UnitAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float *baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, SpellPowerCost* powerCost);
    public:
        void _ApplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr aurApp) override;
        void _UnapplyForTarget(Unit* target, Unit* caster, AuraApplicationPtr aurApp) override;

        void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) override;

        void FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster) override;

        // Allow Apply Aura Handler to modify and access m_AuraDRGroup
        void SetDiminishGroup(DiminishingGroup group) { m_AuraDRGroup = group; }
        DiminishingGroup GetDiminishGroup() const { return m_AuraDRGroup; }

    private:
        DiminishingGroup m_AuraDRGroup:8;               // Diminishing
};

class DynObjAura : public Aura
{
    friend Aura* Aura::Create(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float *baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, Spell* spell);
    protected:
        explicit DynObjAura(SpellInfo const* spellproto, uint32 effMask, WorldObject* owner, Unit* caster, float *baseAmount, Item* castItem, ObjectGuid casterGUID, uint16 stackAmount, SpellPowerCost* powerCost);
    public:
        void Remove(AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT) override;

        void FillTargetMap(std::map<Unit*, uint32> & targets, Unit* caster) override;
};
#endif
