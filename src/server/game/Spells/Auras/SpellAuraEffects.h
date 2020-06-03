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

#ifndef TRINITY_SPELLAURAEFFECTS_H
#define TRINITY_SPELLAURAEFFECTS_H

class Unit;
class AuraEffect;
class Aura;

#include "SpellAuras.h"

typedef void(AuraEffect::*pAuraEffectHandler)(AuraApplication const* aurApp, uint8 mode, bool apply) const;

class AuraEffect
{
    friend void Aura::_InitEffects(uint32 effMask, Unit* caster, float *baseAmount);
    friend Aura* Unit::_TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint32 effMask, Unit* caster, float* baseAmount, Item* castItem, ObjectGuid casterGUID);
    friend Aura::~Aura();
    private:
        explicit AuraEffect(Aura* base, uint8 effIndex, float *baseAmount, Unit* caster, uint8 diffMode);
    public:
        ~AuraEffect();
        Unit* GetCaster() const { return GetBase()->GetCaster(); }
        Unit* GetSaveTarget() const { return saveTarget; }
        ObjectGuid GetCasterGUID() const { return GetBase()->GetCasterGUID(); }
        Aura* GetBase() const { return m_base; }
        void GetTargetList(std::list<Unit*> & targetList) const;
        void GetApplicationList(std::list<AuraApplication*> & applicationList) const;
        SpellModifier* GetSpellModifier() const { return m_spellmod; }

        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
        uint32 GetId() const { return m_spellInfo->Id; }
        uint32 GetEffIndex() const { return m_effIndex; }
        float GetBaseAmount() const { return m_baseAmount; }
        float GetBaseSendAmount() const { return m_send_baseAmount; }
        void SetBaseSendAmount(int32 amount) { m_send_baseAmount = amount; }
        float GetOldBaseAmount() const { return m_oldbaseAmount; }
        void SetOldBaseAmount(int32 amount) { m_oldbaseAmount = amount;}
        float GetCalcAmount() const { return m_calc_amount; }
        int32 GetPeriod() const { return m_period; }
        void SetAmplitude(int32 amplitude) { m_period = amplitude; }

        int32 GetMiscValueB() const
        {
            if (m_spellInfo) return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->MiscValueB;
            return 0;
        }
        int32 GetMiscValue() const
        {
            if (m_spellInfo) return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->MiscValue;
            return 0;
        }
        uint32 GetTriggerSpell() const
        {
            if (m_spellInfo) return m_spellInfo->GetEffect(m_effIndex, m_diffMode)->TriggerSpell;
            return 0;
        }
        AuraType GetAuraType() const
        {
            if (m_spellInfo) return static_cast<AuraType>(m_spellInfo->GetEffect(m_effIndex, m_diffMode)->ApplyAuraName);
            return static_cast<AuraType>(0);
        }
        float GetAmount() const { return m_amount; }
        void SetAmount(float amount)
        {
            if (m_amount != amount)
            {
                m_amount = amount;
                GetBase()->SetNeedClientUpdateForTargets();
                GetBase()->UpdateConcatenateAura(GetCaster(), m_amount, m_effIndex);
            }

            m_canBeRecalculated = false;
        }

        int32 GetPeriodicTimer() const { return m_periodicTimer; }
        void SetPeriodicTimer(int32 periodicTimer) { m_periodicTimer = periodicTimer; }

        float CalculateAmount(Unit* caster);
        void CalculateFromDummyAmount(Unit* caster, Unit* target, float &amount);
        void CalculatePeriodic(Unit* caster, bool resetPeriodicTimer = true, bool load = false);
        void CalculateSpellMod();
        void ChangeAmount(float newAmount, bool mark = true, bool onStackOrReapply = false);
        void RecalculateAmount() { if (!CanBeRecalculated()) return; ChangeAmount(CalculateAmount(GetCaster()), false); }
        void RecalculateAmount(Unit* caster) { if (!CanBeRecalculated()) return; ChangeAmount(CalculateAmount(caster), false); }
        bool CanBeRecalculated() const { return m_canBeRecalculated; }
        void SetCanBeRecalculated(bool val) { m_canBeRecalculated = val; }
        void HandleEffect(AuraApplication * aurApp, uint8 mode, bool apply);
        void HandleEffect(Unit* target, uint8 mode, bool apply);
        void ApplySpellMod(Unit* target, bool apply);
        void RecalculateTickPeriod(Unit* caster);

        void Update(uint32 diff, Unit* caster);
        void UpdatePeriodic(Unit* caster);

        uint32 GetTickNumber() const;
        void SetTickNumber(uint32 tick);
        uint32 GetTotalTicks() const;
        void ResetPeriodic(bool resetPeriodicTimer = false);
        float GetPeriodMod();
        void SetPeriodMod(float mod);
        bool IsPeriodic() const;
        void SetPeriodic(bool isPeriodic);
        bool IsAffectingSpell(SpellInfo const* spell) const;

        bool HasSpellClassMask() const;
        flag128 GetSpellClassMask() const;

        void SendTickImmune(Unit* target, Unit* caster) const;
        void PeriodicTick(AuraApplication * aurApp, Unit* caster, SpellEffIndex effIndex) const;

        void HandleProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex);

        void CleanupTriggeredSpells(Unit* target);

        // add/remove SPELL_AURA_MOD_SHAPESHIFT (36) linked auras
        void HandleShapeshiftBoosts(Unit* target, bool apply) const;

        SpellEffectInfo const* GetSpellEffectInfo() const { return _effectInfo; }
    private:
        Aura* const m_base;

        SpellInfo const* const m_spellInfo;
        SpellEffectInfo const* _effectInfo;
        float const m_baseAmount;

        float m_amount;
        float m_calc_amount;
        float m_amount_add;
        float m_amount_mod;
        float m_crit_mod;
        float m_oldbaseAmount;
        float m_send_baseAmount;
        Unit* saveTarget;

        SpellModifier* m_spellmod;

        int32 m_periodicTimer;
        int32 m_period;
        uint32 m_tickNumber;
        float m_period_mod;
        float m_activation_time;
        bool m_nowInTick;

        uint8 const m_effIndex;
        bool m_canBeRecalculated;
        bool m_isPeriodic;
        uint8 m_diffMode;

    public:
        // aura effect apply/remove handlers
        void HandleNULL(AuraApplication const* /*aurApp*/, uint8 /*mode*/, bool /*apply*/) const
        {
            // not implemented
        }
        void HandleUnused(AuraApplication const* /*aurApp*/, uint8 /*mode*/, bool /*apply*/) const
        {
            // useless
        }
        void HandleNoImmediateEffect(AuraApplication const* /*aurApp*/, uint8 /*mode*/, bool /*apply*/) const
        {
            // aura type not have immediate effect at add/remove and handled by ID in other code place
        }

        void HandleModInvisibilityDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModInvisibility(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModStealth(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModStealthLevel(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAllowTalentSwapping(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModStealthDetect(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleSpiritOfRedemption(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraGhost(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandlePhase(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModShapeshift(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraTransform(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModScale(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraCloneCaster(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraInitializeImages(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModExpertise(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleFeignDeath(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModUnattackable(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModDisarm(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModPacify(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModPacifyAndSilence(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraAllowOnlyAbility(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraTrackResources(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraTrackCreatures(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraTrackStealthed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModStalked(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraUntrackable(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraPvpTalents(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleEnablePvpStatsScaling(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModSkill(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraAllowFlight(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraWaterWalk(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraFeatherFall(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraCanTurnWhileFalling(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraHover(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleWaterBreathing(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleForceMoveForward(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModTotalThreat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModTaunt(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModConfuse(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModFear(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModStun(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModRoot(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandlePreventFleeing(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPossess(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModCharm(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleCharmConvert(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraControlVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseMountedSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseFlightSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseSwimSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModDecreaseSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModUseNormalSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModStateImmunityMask(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModMechanicImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModEffectImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModStateImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModSchoolImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModDmgImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModDispelImmunity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModBaseResistancePCT(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModResistancePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModBaseResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModTargetResistance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModStat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModSpellDamagePercentFromStat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModSpellHealingPercentFromStat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModSpellPowerPercent(AuraApplication const * aurApp, uint8 mode, bool apply) const;
        void HandleModHealingDone(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModTotalPercentStat(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleOverrideSpellPowerByAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleOverrideAutoattack(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleIncreaseModRatingPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModStatBonusPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraBonusArmor(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModLifeStealPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModVersalityPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPowerRegen(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPowerRegenPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModManaRegen(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleEnableExtraTalent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseMaxHealth(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseEnergy(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModMaxPower(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModMaxManaPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModAddEnergyPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseHealthPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModPetStatsModifier(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModIncreaseHealthOrPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModParryPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModDodgePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModBlockPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModRegenInterrupt(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModWeaponCritPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModSpellHitChance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModSpellCritChance(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModCritPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModResiliencePct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModCastingSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModMeleeRangedSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModCombatSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModAttackSpeed(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModMeleeSpeedPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModRangedHaste(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModRating(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleConverCritRatingPctToParryRating(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModRangedAttackPower(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModRangedAttackPowerPercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleOverrideAttackPowerBySpellPower(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModDamageDone(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModDamagePercentDone(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModOffhandDamagePercent(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleShieldBlockValue(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPowerCostPCT(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModPowerCost(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleArenaPreparation(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraDummy(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleChannelDeathItem(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleBindSight(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleForceReaction(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraEmpathy(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModFaction(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleComprehendLanguage(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraLinked(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraStrangulate(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModFakeInebriation(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraOverrideSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraSetVehicle(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandlePreventResurrection(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraForceWeather(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleProgressBar(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleOverrideActionbarSpells(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModCategoryCooldown(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraSeeWhileInvisible(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraMastery(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModCharges(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModChargeRecoveryMod(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleBattlegroundFlag(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleCreateAreaTrigger(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraActivateScene(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraeEablePowerType(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleShowConfirmationPrompt(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleSummonController(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModNextSpell(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleDisableMovementForce(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAnimReplacementSet(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModNoActions(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleDisableGravity(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandlePeriodicDummyAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicTriggerSpellAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicTriggerSpellWithValueAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicDamageAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex, bool lastTick = false) const;
        void HandlePeriodicWeaponPercentDamageAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicHealthLeechAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicHealthFunnelAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicHealAurasTick(Unit* target, Unit* caster, SpellEffIndex effIndex, bool lastTick = false) const;
        void HandlePeriodicManaLeechAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandleObsModPowerAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicEnergizeAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void HandlePeriodicPowerBurnAuraTick(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        void AuraSpellTrigger(Unit* target, Unit* caster, SpellEffIndex effIndex) const;
        bool AuraCostPower(Unit* caster) const;
        void HandleProcTriggerSpellAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex);
        void HandleProcTriggerSpellWithValueAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex);
        void HandleProcTriggerDamageAuraProc(AuraApplication* aurApp, ProcEventInfo& eventInfo, SpellEffIndex effIndex);
        void HandleAllowUsingGameobjectsWhileMounted(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleOverridePetSpecs(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModMinimumSpeedRate(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleFixate(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModTimeRate(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraContestedPvP(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModCooldownSpeedRate(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraModCooldownPct(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleAuraProcOnHpBelow(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleExpedite(AuraApplication const* aurApp, uint8 mode, bool apply) const;
        void HandleModVisibilityRange(AuraApplication const* aurApp, uint8 mode, bool apply) const;
};

#endif
