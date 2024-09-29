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

#ifndef __SPELL_H
#define __SPELL_H

#include "GridDefines.h"
#include "SharedDefines.h"
#include "SpellScript.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "SpellTargetInfo.h"

class Unit;
class Player;
class GameObject;
class DynamicObject;
class WorldObject;
class Aura;
class SpellScript;
class ByteBuffer;

namespace WorldPackets
{
    namespace Spells
    {
        struct SpellCastData;
        struct SpellAmmo;
    }
}

enum SpellCastFlags
{
    CAST_FLAG_NONE               = 0x00000000,
    CAST_FLAG_PENDING            = 0x00000001,              // aoe combat log?
    CAST_FLAG_HAS_TRAJECTORY     = 0x00000002,
    CAST_FLAG_UNKNOWN_3          = 0x00000004,
    CAST_FLAG_UNKNOWN_4          = 0x00000008,              // ignore AOE visual
    CAST_FLAG_UNKNOWN_5          = 0x00000010,
    CAST_FLAG_PROJECTILE         = 0x00000020,
    CAST_FLAG_UNKNOWN_7          = 0x00000040,              // goes with trade skill recasts
    CAST_FLAG_UNKNOWN_8          = 0x00000080,
    CAST_FLAG_UNKNOWN_9          = 0x00000100,
    CAST_FLAG_UNKNOWN_10         = 0x00000200,
    CAST_FLAG_UNKNOWN_11         = 0x00000400,
    CAST_FLAG_POWER_LEFT_SELF    = 0x00000800,
    CAST_FLAG_UNKNOWN_13         = 0x00001000,
    CAST_FLAG_UNKNOWN_14         = 0x00002000,
    CAST_FLAG_UNKNOWN_15         = 0x00004000,
    CAST_FLAG_UNKNOWN_16         = 0x00008000,
    CAST_FLAG_UNKNOWN_17         = 0x00010000,
    CAST_FLAG_ADJUST_MISSILE     = 0x00020000,
    CAST_FLAG_NO_GCD             = 0x00040000,              // related to SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE
    CAST_FLAG_VISUAL_CHAIN       = 0x00080000,
    CAST_FLAG_UNKNOWN_21         = 0x00100000,
    CAST_FLAG_RUNE_LIST          = 0x00200000,
    CAST_FLAG_UNKNOWN_23         = 0x00400000,
    CAST_FLAG_UNKNOWN_24         = 0x00800000,
    CAST_FLAG_UNKNOWN_25         = 0x01000000,
    CAST_FLAG_UNKNOWN_26         = 0x02000000,
    CAST_FLAG_IMMUNITY           = 0x04000000,
    CAST_FLAG_UNKNOWN_28         = 0x08000000,
    CAST_FLAG_UNKNOWN_29         = 0x10000000,
    CAST_FLAG_UNKNOWN_30         = 0x20000000,
    CAST_FLAG_HEAL_PREDICTION    = 0x40000000,
    CAST_FLAG_UNKNOWN_32         = 0x80000000,
};

enum SpellCastFlagsEx
{
    CAST_FLAG_EX_NONE            = 0x00000,
    CAST_FLAG_EX_NO_COOLDOWN     = 0x00001,
    CAST_FLAG_EX_UNKNOWN_2       = 0x00002,
    CAST_FLAG_EX_UNKNOWN_3       = 0x00004,
    CAST_FLAG_EX_UNKNOWN_4       = 0x00008,
    CAST_FLAG_EX_UNKNOWN_5       = 0x00010,
    CAST_FLAG_EX_UNKNOWN_6       = 0x00020,
    CAST_FLAG_EX_UNKNOWN_7       = 0x00040,
    CAST_FLAG_EX_UNKNOWN_8       = 0x00080,
    CAST_FLAG_EX_UNKNOWN_9       = 0x00100,
    CAST_FLAG_EX_NO_CD           = 0x00200,
    CAST_FLAG_EX_UNKNOWN_11      = 0x00400,
    CAST_FLAG_EX_UNKNOWN_12      = 0x00800,
    CAST_FLAG_EX_UNKNOWN_13      = 0x01000,
    CAST_FLAG_EX_UNKNOWN_14      = 0x02000,
    CAST_FLAG_EX_UNKNOWN_15      = 0x04000,
    CAST_FLAG_EX_USE_TOY_SPELL   = 0x08000, // Starts cooldown on toy
    CAST_FLAG_EX_UNKNOWN_17      = 0x10000,
    CAST_FLAG_EX_UNKNOWN_18      = 0x20000,
    CAST_FLAG_EX_UNKNOWN_19      = 0x40000,
    CAST_FLAG_EX_UNKNOWN_20      = 0x80000
};

struct SpellLogEffectPowerDrainParams
{
    ObjectGuid Victim;
    uint32 Points = 0;
    uint32 PowerType = 0;
    float Amplitude = 0;
};

struct SpellLogEffectExtraAttacksParams
{
    ObjectGuid Victim;
    uint32 NumAttacks = 0;
};

struct SpellLogEffectDurabilityDamageParams
{
    ObjectGuid Victim;
    int32 ItemID = 0;
    int32 Amount = 0;
};

struct SpellLogEffectGenericVictimParams
{
    ObjectGuid Victim;
};

struct SpellLogEffectTradeSkillItemParams
{
    int32 ItemID = 0;
};

struct SpellLogEffectFeedPetParams
{
    int32 ItemID = 0;
};

struct SpellValue
{
    explicit  SpellValue(SpellInfo const* proto, uint8 diff);
    float     EffectBasePoints[MAX_SPELL_EFFECTS];
    bool      LockBasePoints[MAX_SPELL_EFFECTS];
    uint32    MaxAffectedTargets;
    float     RadiusMod;
    uint8     AuraStackAmount;
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4,
    SPELL_STATE_DELAYED   = 5
};

enum SpellEffectHandleMode
{
    SPELL_EFFECT_HANDLE_LAUNCH,
    SPELL_EFFECT_HANDLE_LAUNCH_TARGET,
    SPELL_EFFECT_HANDLE_HIT,
    SPELL_EFFECT_HANDLE_HIT_TARGET,
};

class Spell
{
    friend void Unit::SetCurrentCastedSpell(Spell* pSpell);
    friend class SpellScript;
    friend class AreaTrigger;
    public:
        void EffectNULL(SpellEffIndex effIndex);
        void EffectUnused(SpellEffIndex effIndex);
        void EffectDistract(SpellEffIndex effIndex);
        void EffectSchoolDMG(SpellEffIndex effIndex);
        void EffectEnvironmentalDMG(SpellEffIndex effIndex);
        void EffectInstaKill(SpellEffIndex effIndex);
        void EffectDummy(SpellEffIndex effIndex);
        void EffectTeleportUnits(SpellEffIndex effIndex);
        void EffectIncreaseCurrencyCap(SpellEffIndex effIndex);
        void EffectApplyAura(SpellEffIndex effIndex);
        void EffectSendEvent(SpellEffIndex effIndex);
        void EffectPowerBurn(SpellEffIndex effIndex);
        void EffectPowerDrain(SpellEffIndex effIndex);
        void EffectHeal(SpellEffIndex effIndex);
        void EffectBind(SpellEffIndex effIndex);
        void EffectHealthLeech(SpellEffIndex effIndex);
        void EffectQuestComplete(SpellEffIndex effIndex);
        void EffectCreateItem(SpellEffIndex effIndex);
        void EffectDestroyItem(SpellEffIndex effIndex);
        void EffectCreateItem2(SpellEffIndex effIndex);
        void EffectCreateItem3(SpellEffIndex effIndex);
        void EffectCreateRandomItem(SpellEffIndex effIndex);
        void EffectPersistentAA(SpellEffIndex effIndex);
        void EffectEnergize(SpellEffIndex effIndex);
        void EffectOpenLock(SpellEffIndex effIndex);
        void EffectSummonChangeItem(SpellEffIndex effIndex);
        void EffectProficiency(SpellEffIndex effIndex);
        void EffectApplyAreaAura(SpellEffIndex effIndex);
        void EffectSummonType(SpellEffIndex effIndex);
        void EffectLearnSpell(SpellEffIndex effIndex);
        void EffectDispel(SpellEffIndex effIndex);
        void EffectDualWield(SpellEffIndex effIndex);
        void EffectPickPocket(SpellEffIndex effIndex);
        void EffectAddFarsight(SpellEffIndex effIndex);
        void EffectUntrainTalents(SpellEffIndex effIndex);
        void EffectHealMechanical(SpellEffIndex effIndex);
        void EffectJump(SpellEffIndex effIndex);
        void EffectJumpDest(SpellEffIndex effIndex);
        void EffectLeapBack(SpellEffIndex effIndex);
        void EffectQuestClear(SpellEffIndex effIndex);
        void EffectTeleUnitsFaceCaster(SpellEffIndex effIndex);
        void EffectLearnSkill(SpellEffIndex effIndex);
        void EffectIncreaseSkill(SpellEffIndex effIndex);
        void EffectPlayMovie(SpellEffIndex effIndex);
        void EffectTradeSkill(SpellEffIndex effIndex);
        void EffectEnchantItemPerm(SpellEffIndex effIndex);
        void EffectEnchantItemTmp(SpellEffIndex effIndex);
        void EffectTameCreature(SpellEffIndex effIndex);
        void EffectSummonPet(SpellEffIndex effIndex);
        void EffectLearnPetSpell(SpellEffIndex effIndex);
        void EffectWeaponDmg(SpellEffIndex effIndex);
        void EffectForceCast(SpellEffIndex effIndex);
        void EffectTriggerSpell(SpellEffIndex effIndex);
        void EffectTriggerMissileSpell(SpellEffIndex effIndex);
        void EffectThreat(SpellEffIndex effIndex);
        void EffectHealMaxHealth(SpellEffIndex effIndex);
        void EffectInterruptCast(SpellEffIndex effIndex);
        void EffectSummonObjectWild(SpellEffIndex effIndex);
        void EffectScriptEffect(SpellEffIndex effIndex);
        void EffectSanctuary(SpellEffIndex effIndex);
        void EffectDuel(SpellEffIndex effIndex);
        void EffectStuck(SpellEffIndex effIndex);
        void EffectSummonPlayer(SpellEffIndex effIndex);
        void EffectActivateObject(SpellEffIndex effIndex);
        void EffectApplyGlyph(SpellEffIndex effIndex);
        void EffectEnchantHeldItem(SpellEffIndex effIndex);
        void EffectSummonObject(SpellEffIndex effIndex);
        void EffectSurvey(SpellEffIndex effIndex);
        void EffectResurrect(SpellEffIndex effIndex);
        void EffectParry(SpellEffIndex effIndex);
        void EffectBlock(SpellEffIndex effIndex);
        void EffectLeap(SpellEffIndex effIndex);
        void EffectTransmitted(SpellEffIndex effIndex);
        void EffectDisEnchant(SpellEffIndex effIndex);
        void EffectInebriate(SpellEffIndex effIndex);
        void EffectFeedPet(SpellEffIndex effIndex);
        void EffectDismissPet(SpellEffIndex effIndex);
        void EffectReputation(SpellEffIndex effIndex);
        void EffectForceDeselect(SpellEffIndex effIndex);
        void EffectSelfResurrect(SpellEffIndex effIndex);
        void EffectSkinning(SpellEffIndex effIndex);
        void EffectCharge(SpellEffIndex effIndex);
        void EffectChargeDest(SpellEffIndex effIndex);
        void EffectProspecting(SpellEffIndex effIndex);
        void EffectMilling(SpellEffIndex effIndex);
        void EffectRenamePet(SpellEffIndex effIndex);
        void EffectSendTaxi(SpellEffIndex effIndex);
        void EffectKnockBack(SpellEffIndex effIndex);
        void EffectPullTowards(SpellEffIndex effIndex);
        void EffectDispelMechanic(SpellEffIndex effIndex);
        void EffectSummonDeadPet(SpellEffIndex effIndex);
        void EffectDurabilityDamage(SpellEffIndex effIndex);
        void EffectSkill(SpellEffIndex effIndex);
        void EffectTaunt(SpellEffIndex effIndex);
        void EffectDurabilityDamagePCT(SpellEffIndex effIndex);
        void EffectModifyThreatPercent(SpellEffIndex effIndex);
        void EffectResurrectNew(SpellEffIndex effIndex);
        void EffectAddExtraAttacks(SpellEffIndex effIndex);
        void EffectSpiritHeal(SpellEffIndex effIndex);
        void EffectSkinPlayerCorpse(SpellEffIndex effIndex);
        void EffectStealBeneficialBuff(SpellEffIndex effIndex);
        void EffectUnlearnSpecialization(SpellEffIndex effIndex);
        void EffectHealPct(SpellEffIndex effIndex);
        void EffectEnergizePct(SpellEffIndex effIndex);
        void EffectTriggerRitualOfSummoning(SpellEffIndex effIndex);
        void EffectSummonRaFFriend(SpellEffIndex effIndex);
        void EffectKillCreditPersonal(SpellEffIndex effIndex);
        void EffectKillCredit(SpellEffIndex effIndex);
        void EffectQuestFail(SpellEffIndex effIndex);
        void EffectQuestStart(SpellEffIndex effIndex);
        void EffectRedirectThreat(SpellEffIndex effIndex);
        void EffectGameObjectDamage(SpellEffIndex effIndex);
        void EffectGameObjectRepair(SpellEffIndex effIndex);
        void EffectGameObjectSetDestructionState(SpellEffIndex effIndex);
        void EffectCreateTamedPet(SpellEffIndex effIndex);
        void EffectDiscoverTaxi(SpellEffIndex effIndex);
        void EffectTitanGrip(SpellEffIndex effIndex);
        void EffectEnchantItemPrismatic(SpellEffIndex effIndex);
        void EffectPlayMusic(SpellEffIndex effIndex);
        void EffectSpecCount(SpellEffIndex effIndex);
        void EffectActivateSpec(SpellEffIndex effIndex);
        void EffectPlaySound(SpellEffIndex effIndex);
        void EffectRemoveAura(SpellEffIndex effIndex);
        void EffectDamageFromMaxHealthPCT(SpellEffIndex effIndex);
        void EffectCastButtons(SpellEffIndex effIndex);
        void EffectRechargeManaGem(SpellEffIndex effIndex);
        void EffectGiveCurrency(SpellEffIndex effIndex);
        void EffectUpdatePlayerPhase(SpellEffIndex effIndex);
        void EffectUnlearnTalent(SpellEffIndex effIndex);
        void EffectDespawnAreatrigger(SpellEffIndex effIndex);
        void EffectDespawnDynamicObject(SpellEffIndex effIndex);
        void EffectBuyGuilkBankTab(SpellEffIndex effIndex);
        void EffectCreateAreaTrigger(SpellEffIndex effIndex);
        void EffectResurrectWithAura(SpellEffIndex effIndex);
        void EffectSummonRaidMarker(SpellEffIndex effIndex);
        void EffectCorpseLoot(SpellEffIndex effIndex);
        void EffectRandomizeDigsites(SpellEffIndex effIndex);
        void EffectTeleportToDigsite(SpellEffIndex effIndex);
        void EffectUncageBattlePet(SpellEffIndex effIndex);
        void EffectGrantBattlePetLevel(SpellEffIndex effIndex);
        void EffectPlayerMoveWaypoints(SpellEffIndex effIndex);
        void EffectSetMaxBattlePetCount(SpellEffIndex effIndex);
        void EffectUpgradeBattlePet(SpellEffIndex effIndex);
        void EffectUnlockPetBattles(SpellEffIndex effIndex);
        void EffectHealBattlePetPct(SpellEffIndex effIndex);
        void SendScene(SpellEffIndex effIndex);
        void EffectBonusLoot(SpellEffIndex effIndex);
        void EffectJoinOrLeavePlayerParty(SpellEffIndex effIndex);
        void EffectSummonConversation(SpellEffIndex effIndex);
        void EffectLearnGarrisonBuilding(SpellEffIndex effIndex);
        void EffectCreateGarrison(SpellEffIndex effIndex);
        void EffectCreateGarrisonShipment(SpellEffIndex effIndex);
        void EffectAddGarrisonFollower(SpellEffIndex effIndex);
        void EffectAddGarrisonMission(SpellEffIndex effIndex);
        void EffectActivateGarrisonBuilding(SpellEffIndex effIndex);
        void EffectCreateHeirloomItem(SpellEffIndex effIndex);
        void EffectChangeItemBonuses(SpellEffIndex effIndex);
        void EffectUpgradeHeirloom(SpellEffIndex effIndex);
        void EffectGiveExperience(SpellEffIndex effIndex);
        void EffectRemovePhase(SpellEffIndex effIndex);
        void EffectModAssistantEquipmentLevel(SpellEffIndex effIndex);
        void EffectIncreaseFollowerExperience(SpellEffIndex effIndex);
        void EffectReTrainFollower(SpellEffIndex effIndex);
        void EffectChangeFollowerVitality(SpellEffIndex);
        void EffectForceEquipItem(SpellEffIndex effIndex);
        void EffectGiveArtifactPower(SpellEffIndex effIndex);
        void EffectGiveArtifactPowerNoBonus(SpellEffIndex effIndex);
        void EffectLaunchQuestChoice(SpellEffIndex effIndex);
        void EffectUpdateZoneAurasAndPhases(SpellEffIndex effIndex);
        void EffectApplyEnchantIllusion(SpellEffIndex effIndex);
        void EffectCollectItemAppearancesSet(SpellEffIndex effIndex);
        void EffectGiveHonorPoints(SpellEffIndex effIndex);
        void EffectObliterateItem(SpellEffIndex effIndex);
        void EffectBecomeUntargetable(SpellEffIndex effIndex);
        void EffectModReputation(SpellEffIndex effIndex);

        Spell(Unit* caster, SpellInfo const* info, TriggerCastData& triggerData);
        ~Spell();

        void InitExplicitTargets(SpellCastTargets const& targets);
        void SelectExplicitTargets();

        void SelectSpellTargets();
        void SelectEffectImplicitTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32& processedEffectMask);
        void SelectImplicitChannelTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitNearbyTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitConeTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitBetweenTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitGotoMoveTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitAreaTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitCasterDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitTargetDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitDestDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitCasterObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitTargetObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType);
        void SelectImplicitChainTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, WorldObject* target, uint32 effMask);
        void SelectImplicitTargetsFromThreadList(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask);
        void SelectImplicitTrajTargets();
        void AddUnitTarget(Unit* target, uint32 effectMask, bool checkIfValid = true, bool implicit = true, bool jump = false);

        void SelectEffectTypeImplicitTargets(uint8 effIndex);

        std::vector<SpellScript*> const & GetSpellScripts();

        uint32 GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionList* condList);

        template<class SEARCHER>
        void SearchTargets(SEARCHER& searcher, uint32 containerMask, Unit* referer, Position const* pos, float radius);

        WorldObject* SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList = nullptr);
        void SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList, bool allowObjectSize = true);
        void SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, ConditionList* condList, bool isChainHeal);

        void preparePetCast(SpellCastTargets const* targets, Unit* target, Unit* pet, ObjectGuid petGuid, Player* player);
        void prepare(SpellCastTargets const* targets);
        void cancel();
        void update(uint32 difftime);
        void cast(bool skipCheck = false);
        void finish(bool ok = true);
        void TakePower();

        void TakeRunePower();
        void TakeReagents();
        void TakeCastItem();
        bool CanDestoyCastItem();
        void LinkedSpell(Unit* caster, Unit *target, SpellLinkedType type = SPELL_LINK_CAST, uint32 effectMask = 4294967295);

        SpellCastResult CheckCast(bool strict);
        SpellCastResult CheckPetCast(Unit* target);

        void handle_immediate();
        uint64 handle_delayed(uint64 t_offset);
        void _handle_immediate_phase();
        void _handle_finish_phase();

        SpellCastResult CheckItems();
        SpellCastResult CheckRange(bool strict);
        SpellCastResult CheckPower();
        SpellCastResult CheckCasterAuras() const;
        SpellCastResult CheckArenaAndRatedBattlegroundCastRules(Player* player);

        float CalculateDamage(uint8 i, Unit const* target, float* var = nullptr) const;

        bool HaveTargetsForEffect(uint8 effect) const;
        void Delayed();
        void DelayedChannel();
        uint32 getState() const { return m_spellState; }
        void setState(uint32 state) { m_spellState = state; }

        void DoCreateItem(uint32 i, uint32 itemtype, std::vector<uint32> const& bonusListIDs = std::vector<uint32>());
        bool SpellDummyTriggered(SpellEffIndex effIndex);
        DelayCastEvent* GetSpellTriggerDelay(SpellEffIndex effIndex, uint32 triggered_spell_id);

        bool CheckEffectTarget(Unit const* target, uint32 eff) const;
        bool CanAutoCast(Unit* target);
        void CheckSrc();
        void CheckDst();

        void SendCastResult(Player* caster, SpellInfo const* spellInfo, SpellCastResult result, SpellCustomErrors customError = SPELL_CUSTOM_ERROR_NONE, uint32* misc = nullptr, bool pet = false);
        void SendCastResult(SpellCastResult result);
        void SendSpellCastGuids();
        void SendSpellStart();
        void SendSpellGo();
        void SendSpellPendingCast();
        void SendSpellCooldown();
        void SendSpellExecuteLog();
        void ExecuteLogEffectTakeTargetPower(uint8 effIndex, Unit* target, uint32 powerType, uint32 points, float amplitude);
        void ExecuteLogEffectExtraAttacks(uint8 effIndex, Unit* victim, uint32 numAttacks);
        void ExecuteLogEffectInterruptCast(uint8 effIndex, Unit* victim, uint32 spellId);
        void ExecuteLogEffectDurabilityDamage(uint8 effIndex, Unit* victim, int32 itemId, int32 amount);
        void ExecuteLogEffectOpenLock(uint8 effIndex, Object* obj);
        void ExecuteLogEffectCreateItem(uint8 effIndex, uint32 entry);
        void ExecuteLogEffectDestroyItem(uint8 effIndex, uint32 entry);
        void ExecuteLogEffectSummonObject(uint8 effIndex, WorldObject* obj);
        void ExecuteLogEffectUnsummonObject(uint8 effIndex, WorldObject* obj);
        void ExecuteLogEffectResurrect(uint8 effIndex, Unit* target);

        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);

        void HandleEffects(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, uint32 i, SpellEffectHandleMode mode);
        void HandleThreatSpells();
        bool CheckEffFromDummy(Unit* target, uint32 eff);

        SpellInfo const* const m_spellInfo;
        Item* m_CastItem;
        ObjectGuid m_castItemGUID;
        uint32 m_castItemEntry;
        uint32 m_combatItemEntry;
        uint32 m_castFlags[2];
        uint32 m_miscData[2];

        ObjectGuid m_spellGuid;
        ObjectGuid m_castGuid[2];
        uint32 m_preCastSpell;
        SpellCastTargets m_targets;
        uint32 m_SpellVisual;
        SpellCustomErrors m_customError;
        uint8 m_diffMode;
        bool canHitTargetInLOS;
        uint32 m_count_dispeling; // Final count dispell auras
        bool m_interupted;
        bool m_replaced;
        WorldLocation destAtTarget;
        WorldLocation* destTarget;
        Position visualPos;
        std::vector<Position> _positions;
        bool m_castedFromStealth;
        ObjectGuid m_magnetGuid;

        float mCriticalDamageBonus;

        SpellPowerCost m_powerCost;                        // Calculated spell cost initialized only in Spell::prepare
        SpellPowerData m_powerData;

        UsedSpellMods m_appliedMods;
        Uint32Set m_appliedProcMods;

        bool m_canLostTarget;

        int32 GetCastTime() const { return m_casttime; }
        uint32 GetStartCastTime() const { return m_castedTime; }
        void SetStartCastTime(uint32 time);
        bool IsAutoRepeat() const { return m_autoRepeat; }
        void SetAutoRepeat(bool rep) { m_autoRepeat = rep; }
        void ReSetTimer() { m_timer = m_casttime > 0 ? m_casttime : 0; }
        void SetTriggeredCastFlags(uint32 Flag);
        uint32 GetTriggeredCastFlags() const;
        bool IsNextMeleeSwingSpell() const;
        bool IsTriggered() const { return (_triggeredCastFlags & TRIGGERED_FULL_MASK) != 0; };
        bool IsChannelActive() const { return m_caster->GetChannelSpellId() != 0; }
        bool IsAutoActionResetSpell() const;
        bool IsCritForTarget(Unit* target) const;
        bool CanSpellProc(Unit* target, uint32 mask = 7) const;

        bool IsDeletable() const { return !m_referencedFromCurrentSpell && !m_executedCurrently; }
        void SetReferencedFromCurrent(bool yes) { m_referencedFromCurrentSpell = yes; }
        bool IsInterruptable() const { return !m_executedCurrently; }
        void SetExecutedCurrently(bool yes) {m_executedCurrently = yes;}
        uint64 GetDelayStart() const { return m_delayStart; }
        void SetDelayStart(uint64 m_time) { m_delayStart = m_time; }
        uint64 GetDelayMoment() const { return m_delayMoment; }

        bool IsNeedSendToClient() const;

        CurrentSpellTypes GetCurrentContainer() const;

        Unit* GetCaster() const { return m_caster; }
        Unit* GetOriginalCaster() const { return m_originalCaster; }
        SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
        int32 GetPowerCost(int8 power);
        int8 GetComboPoints();

        void UpdatePointers(); // must be used at call Spell code after time delay (non triggered spell cast/update spell call/etc)

        bool IsItemCategoryCombat() const;

        void CleanupTargetList();

        void SetSpellValue(SpellValueMod mod, float value, bool lockValue = false);
        uint32 GetCountDispel() const { return m_count_dispeling; }
        bool GetInterupted() const { return m_interupted; }
        bool GetCastedFromStealth() const { return m_castedFromStealth; }

        void SetSpellDynamicObject(ObjectGuid const& dynObj) { m_spellDynObjGuid = dynObj;}
        ObjectGuid const& GetSpellDynamicObject() const { return m_spellDynObjGuid; }
        void SetEffectTargets (GuidList const& targets) { m_effect_targets = targets; }
        GuidList GetEffectTargets() { return m_effect_targets; }
        void AddEffectTarget (ObjectGuid const& targetGuid) { m_effect_targets.push_back(targetGuid); }
        void RemoveEffectTarget(ObjectGuid const& targetGuid) { m_effect_targets.remove(targetGuid); }
        void ClearEffectTarget () { m_effect_targets.clear(); }
        ObjectGuid GetRndEffectTarget () { return Trinity::Containers::SelectRandomContainerElement(m_effect_targets); }
        AuraEffect const* GetTriggeredAuraEff() const { return m_triggeredByAura; }
        void AddDestTarget(SpellDestination const& dest, uint32 effIndex);
        SpellDestination getDestTarget(uint32 effIndex) { return m_destTargets[effIndex]; }

        size_t GetTargetCount() const { return m_UniqueTargetInfo.size(); }
        std::vector<TargetInfoPtr>* GetUniqueTargetInfo() { return &m_UniqueTargetInfo; }
        uint32 GetTargetParentCount() const { return m_parentTargetCount; }

        int32 GetDamage() const { return m_damage; }

        void AddDst(Position const* pos) { _positions.push_back(*pos); }

        void UpdateSpellCastDataTargets(WorldPackets::Spells::SpellCastData& data);
        void UpdateSpellCastDataAmmo(WorldPackets::Spells::SpellAmmo& data);

        void CallScriptCalculateEffMaskHandlers(uint32 & effMask);

    protected:
        bool HasGlobalCooldown();
        void TriggerGlobalCooldown();
        void CancelGlobalCooldown();
        int32 GetGlobalCooldown();

        void SendLoot(ObjectGuid const& guid, LootType loottype);

        Unit* const m_caster;
        Position m_castPos;

        SpellValue* m_spellValue;

        ObjectGuid m_originalCasterGUID;                        // real source of cast (aura caster/etc), used for spell targets selection
                                                            // e.g. damage around area spell trigered by victim aura and damage enemies of aura caster
        Unit* m_originalCaster;                             // cached pointer for m_originalCaster, updated at Spell::UpdatePointers()
        Unit* m_originalTarget;
        ObjectGuid m_originalTargetGUID;

        Spell** m_selfContainer;                            // pointer to our spell container (if applicable)

        //Spell data
        SpellSchoolMask m_spellSchoolMask;                  // Spell school (can be overwrite for some spells (wand shoot for example)
        WeaponAttackType m_attackType;                      // For weapon based attack
        int32 m_casttime;                                   // Calculated spell cast time initialized only in Spell::prepare
        uint32 m_castedTime;
        bool m_canReflect;                                  // can reflect this spell?
        bool m_autoRepeat;
        uint8 m_runesState;
        int32 m_failedArg[2];
        uint32 m_parentTargetCount;

        TriggerCastData m_triggerData;

        uint8 m_delayAtDamageCount;
        bool isDelayableNoMore();

        // Delayed spells system
        uint64 m_delayStart;                                // time of spell delay start, filled by event handler, zero = just started
        uint64 m_delayMoment;                               // moment of next delay call, used internally
        bool m_immediateHandled;                            // were immediate actions handled? (used by delayed spells only)

        // These vars are used in both delayed spell system and modified immediate spell system
        bool m_referencedFromCurrentSpell;                  // mark as references to prevent deleted and access by dead pointers
        bool m_executedCurrently;                           // mark as executed to prevent deleted and access by dead pointers
        uint8 hasPredictedDispel;
        uint32 m_applyMultiplierMask;

        // Current targets, to be used in SpellEffects (MUST BE USED ONLY IN SPELL EFFECTS)
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        float damage;
        float variance;
        bool damageCalculate[MAX_SPELL_EFFECTS];
        float saveDamageCalculate[MAX_SPELL_EFFECTS];
        SpellEffectHandleMode effectHandleMode;
        // used in effects handlers
        Aura* m_spellAura;
        GuidList m_effect_targets;

        // this is set in Spell Hit, but used in Apply Aura handler
        DiminishingLevels m_diminishLevel;
        DiminishingGroup m_diminishGroup;

        // -------------------------------------------
        GameObject* focusObject;
        ObjectGuid m_spellDynObjGuid;

        // Damage and healing in effects need just calculate
        int32 m_damage;           // Damge   in effects count here
        int32 m_healing;          // Healing in effects count here
        int32 m_final_damage;     // Final damage in effects count here
        int32 m_absorb;           // Absorb
        int32 m_resist;           // Resist
        int32 m_blocked;          // Blocked
        int32 m_addpower;         // if spell add power
        int32 m_addptype;         // if spell add power
        bool m_isDamageSpell;

        // ******************************************
        // Spell trigger system
        // ******************************************
        uint32 m_procAttacker;                // Attacker trigger flags
        uint32 m_procVictim;                  // Victim   trigger flags
        uint32 m_procAttackerowner;           // Attacker trigger flags
        uint32 m_procVictimowner;             // Victim   trigger flags
        uint32 m_procEx;
        void   prepareDataForTriggerSystem(AuraEffect const* triggeredByAura);

        // *****************************************
        // Spell target subsystem
        // *****************************************
        std::vector<TargetInfoPtr> m_UniqueTargetInfo;
        std::vector<TargetInfoPtr> m_VisualHitTargetInfo;
        TargetInfoPtr GetTargetInfo(ObjectGuid const& targetGUID);
        uint32 m_channelTargetEffectMask;                        // Mask req. alive targets

        struct GOTargetInfo
        {
            ObjectGuid targetGUID;
            uint64 timeDelay;
            uint32  effectMask:32;
            bool   processed:1;
        };
        std::list<GOTargetInfo> m_UniqueGOTargetInfo;

        struct ItemTargetInfo
        {
            Item  *item;
            uint32 effectMask;
        };
        std::list<ItemTargetInfo> m_UniqueItemInfo;

        SpellDestination m_destTargets[MAX_SPELL_EFFECTS];

        void AddGOTarget(GameObject* target, uint32 effectMask);
        void AddItemTarget(Item* item, uint32 effectMask);
        void AddTargetVisualHit(Unit* target);
        WorldLocation* GetDestTarget(uint32 effIndex) { return &m_destTargets[effIndex]._position; }

        void DoAllEffectOnTarget(TargetInfoPtr target);
        SpellMissInfo DoSpellHitOnUnit(Unit* unit, uint32 effectMask, bool scaleAura);
        void DoTriggersOnSpellHit(Unit* unit, uint32 effMask);
        void DoAllEffectOnTarget(GOTargetInfo* target);
        void DoAllEffectOnTarget(ItemTargetInfo* target);
        bool UpdateChanneledTargetList();
        bool IsValidDeadOrAliveTarget(Unit const* target) const;
        void HandleLaunchPhase();
        void DoAllEffectOnLaunchTarget(TargetInfoPtr targetInfo, float* multiplier);

        void PrepareTargetProcessing();
        void FinishTargetProcessing();

        // Scripting system
        void LoadScripts();
        void CallScriptBeforeCastHandlers();
        void CallScriptOnCastHandlers();
        void CallScriptAfterCastHandlers();
        void CallScriptTakePowerHandlers(Powers p, int32 &amount);
        SpellCastResult CallScriptCheckCastHandlers();
        void PrepareScriptHitHandlers();
        bool CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode);
        void CallScriptBeforeHitHandlers();
        void CallScriptSuccessfulDispel(SpellEffIndex effIndex);
        void CallScriptOnHitHandlers();
        void CallScriptAfterHitHandlers();
        void CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, Targets targetId);
        void CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex);
        void CallScriptObjectJumpTargetHandlers(int32& AdditionalTarget, SpellEffIndex effIndex);
        void CustomTargetSelector(std::list<WorldObject*>& targets, SpellEffIndex effIndex, Targets targetId);
        std::vector<SpellScript*> m_loadedScripts;
        void CallScriptBeforeStartCastHandlers();
        SpellCastResult CustomCheckCast();
        void CallScriptOnFinishCastHandlers();

        struct HitTriggerSpell
        {
            SpellInfo const* triggeredSpell;
            SpellInfo const* triggeredByAura;
            // uint8 triggeredByEffIdx          This might be needed at a later stage - No need known for now
            int32 chance;
        };

        bool CanExecuteTriggersOnHit(uint32 effMask, SpellInfo const* triggeredByAura = nullptr) const;
        void PrepareTriggersExecutedOnHit();
        typedef std::list<HitTriggerSpell> HitTriggerSpellList;
        HitTriggerSpellList m_hitTriggerSpells;

        // effect helpers
        void SummonGuardian(uint32 i, uint32 entry, SummonPropertiesEntry const* properties, uint32 numSummons);
        void CalculateJumpSpeeds(uint8 i, float dist, float & speedxy, float & speedz);

        SpellCastResult CanOpenLock(uint32 effIndex, uint32 lockid, SkillType& skillid, int32& reqSkillValue, int32& skillValue);
        // -------------------------------------------

        uint32 m_spellState;
        int32 m_timer;

        uint32 _triggeredCastFlags;
        bool m_dispelResetCD;

        // if need this can be replaced by Aura copy
        // we can't store original aura link to prevent access to deleted auras
        // and in same time need aura data and after aura deleting.
        SpellInfo const* m_triggeredByAuraSpell;
        AuraEffect const* m_triggeredByAura;

        bool m_skipCheck;
        uint32 m_spellMissMask;
        uint32 m_auraScaleMask;

        std::vector<SpellLogEffectPowerDrainParams> _powerDrainTargets[MAX_SPELL_EFFECTS];
        std::vector<SpellLogEffectExtraAttacksParams> _extraAttacksTargets[MAX_SPELL_EFFECTS];
        std::vector<SpellLogEffectDurabilityDamageParams> _durabilityDamageTargets[MAX_SPELL_EFFECTS];
        std::vector<SpellLogEffectGenericVictimParams> _genericVictimTargets[MAX_SPELL_EFFECTS];
        std::vector<SpellLogEffectTradeSkillItemParams> _tradeSkillTargets[MAX_SPELL_EFFECTS];
        std::vector<SpellLogEffectFeedPetParams> _feedPetTargets[MAX_SPELL_EFFECTS];

        uint16 m_currentExecutedEffect;       //pointer for get current executed effect in effect functions
};

namespace Trinity
{
    struct WorldObjectSpellTargetCheck
    {
        Unit* _caster;
        Unit* _referer;
        SpellInfo const* _spellInfo;
        SpellTargetCheckTypes _targetSelectionType;
        ConditionSourceInfo* _condSrcInfo;
        ConditionList* _condList;

        WorldObjectSpellTargetCheck(Unit* caster, Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        ~WorldObjectSpellTargetCheck();
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellNearbyTargetCheck : WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        WorldObjectSpellNearbyTargetCheck(float range, Unit* caster, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellAreaTargetCheck : WorldObjectSpellTargetCheck
    {
        float _range;
        Position const* _position;
        bool _allowObjectSize;
        WorldObjectSpellAreaTargetCheck(float range, Position const* position, Unit* caster, Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList, bool allowObjectSize = true);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellBetweenTargetCheck : WorldObjectSpellAreaTargetCheck
    {
        float _width, _range;
        Position const* _position;
        WorldObjectSpellBetweenTargetCheck(float width, float range, Unit* caster, Position const* position, Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellConeTargetCheck : WorldObjectSpellAreaTargetCheck
    {
        float _coneAngle;
        WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList);
        bool operator()(WorldObject* target);
    };

    struct WorldObjectSpellTrajTargetCheck : WorldObjectSpellAreaTargetCheck
    {
        WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster, SpellInfo const* spellInfo);
        bool operator()(WorldObject* target);
    };
}

typedef void(Spell::*pEffect)(SpellEffIndex effIndex);

class SpellEvent : public BasicEvent
{
    public:
        SpellEvent(Spell* spell);
        virtual ~SpellEvent();

        virtual bool Execute(uint64 e_time, uint32 p_time);
        virtual void Abort(uint64 e_time);
        virtual bool IsDeletable() const;
    protected:
        Spell* m_Spell;
};
#endif
