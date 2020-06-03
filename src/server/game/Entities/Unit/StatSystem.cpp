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

#include "Unit.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "SharedDefines.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "GameTables.h"
#include "ObjectMgr.h"

inline bool _ModifyUInt32(bool apply, uint32& baseValue, int32& amount)
{
    // If amount is negative, change sign and value of apply.
    if (amount < 0)
    {
        apply = !apply;
        amount = -amount;
    }
    if (apply)
        baseValue += amount;
    else
    {
        // Make sure we do not get uint32 overflow.
        if (amount > int32(baseValue))
            amount = baseValue;
        baseValue -= amount;
    }
    return apply;
}

/*#######################################
########                         ########
########   PLAYERS STAT SYSTEM   ########
########                         ########
#######################################*/

void Player::UpdateStatsByMask()
{
    if (m_operationsAfterDelayMask & OAD_ARENA_DESERTER)
        HandleArenaDeserter();

    if (m_operationsAfterDelayMask & OAD_RECALC_CHAR_STAT)
        InitStatsForLevel(true);

    if (m_operationsAfterDelayMask & OAD_RECALC_ITEMS)
        RescaleAllItemsIfNeeded(true);

    if (m_operationsAfterDelayMask & OAD_RECALC_ITEM_LVL)
        UpdateItemLevels();

    if (m_operationsAfterDelayMask & OAD_RECALC_PVP_BP)
        RecalculatePvPAmountOfAuras();

    if (m_operationsAfterDelayMask & OAD_RECALC_AURAS)
        RecalculateAmountAllAuras();

    if (m_updateCRsMask != 0)
    {
        for (uint8 cr = 0; cr < MAX_COMBAT_RATING; ++cr)
        {
            if (m_updateCRsMask & 1 << cr)
                UpdateRating(CombatRating(cr));
        }

        m_updateCRsMask = 0;
    }

    if (m_updateStatsMask != 0)
    {
        if (m_updateStatsMask & USM_STRENGTH)
            UpdateStats(STAT_STRENGTH);

        if (m_updateStatsMask & USM_AGILITY)
            UpdateStats(STAT_AGILITY);

        if (m_updateStatsMask & USM_INTELLECT)
            UpdateStats(STAT_INTELLECT);

        if (m_updateStatsMask & USM_STAMINA)
            UpdateStats(STAT_STAMINA);

        if (m_updateStatsMask & (USM_MELEE_HAST|USM_RANGE_HAST|USM_CAST_HAST|USM_HAST))
            UpdateHast();

        if (m_updateStatsMask & USM_ARMOR)
            UpdateArmor();

        if (m_updateStatsMask & USM_MELEE_AP)
            UpdateAttackPowerAndDamage();

        if (m_updateStatsMask & USM_RANGE_AP)
            UpdateAttackPowerAndDamage(true);

        if (m_updateStatsMask & USM_MANA_REGEN)
            UpdateManaRegen();

        m_updateStatsMask = 0;
    }

    if (m_operationsAfterDelayMask & OAD_LOAD_PET)
        LoadPet();

    if ((m_operationsAfterDelayMask & OAD_UPDATE_RUNES_REGEN) && getClass() == CLASS_DEATH_KNIGHT)
        UpdatePowerRegen(POWER_RUNES);

    if (m_operationsAfterDelayMask & OAD_RESET_SPELL_QUEUE)
        SetSpellInQueue(0, 0, nullptr);

    m_operationsAfterDelayMask = 0;
}

bool Player::UpdateStats(Stats stat)
{
    if (stat >= MAX_STATS)
        return false;

    float value = GetTotalStatValue(stat);
    float bonusVal = value - GetCreateStat(stat);

    SetStat(stat, int32(value));
    SetFloatValue(UNIT_FIELD_STAT_POS_BUFF + stat, bonusVal > 0.f ? bonusVal : 0.f);
    SetFloatValue(UNIT_FIELD_STAT_NEG_BUFF + stat, bonusVal < 0.f ? bonusVal : 0.f);

    switch (stat)
    {
        case STAT_STRENGTH:
            UpdateParryPercentage();
            SendUpdateStat(USM_MELEE_AP);
            break;
        case STAT_AGILITY:
            UpdateDodgePercentage();
            SendUpdateStat(USM_MELEE_AP | USM_RANGE_AP);
            break;
        case STAT_STAMINA:
        {
            uint8 healthPct = RoundingFloatValue(GetHealthPct());
            UpdateMaxHealth();
            SetHealth(CountPctFromMaxHealth(healthPct));
            break;
        }
        case STAT_INTELLECT:
            UpdateMaxPower(POWER_MANA);
            UpdateSpellCritChance();
            SendUpdateStat(USM_ARMOR);
            break;
        default:
            break;
    }

    UpdateSpellDamageAndHealingBonus();
    SendUpdateStat(USM_MANA_REGEN);

    if (stat == STAT_INTELLECT)
    {
        if (Aura* aura = GetAura(108300))
        {
            aura->GetEffect(EFFECT_0)->SetCanBeRecalculated(true);
            aura->GetEffect(EFFECT_0)->RecalculateAmount();
        }
    }

    if (stat == STAT_STAMINA || stat == STAT_INTELLECT || stat == STAT_STRENGTH)
        if (Pet* pet = GetPet())
            pet->UpdateStats(stat);

    return true;
}

float Player::GetTotalStatValue(Stats stat)
{
    float baseStat = GetCreateStat(stat);
    float bonusStat = 0.f;
    float value = 0.f;
    float otherMod = 0.f;

    if (HasPvpStatsScalingEnabled())
    {
        CalcPvPTemplate(SPELL_AURA_MOD_STAT, value, otherMod, [stat](AuraEffect const* aurEff) -> bool
        {
            return aurEff->GetMiscValue() < 0 || aurEff->GetMiscValue() == stat;
        });

        float templateStat = value;
        value = templateStat * GetPvpStatScalar();

        if (stat == STAT_STAMINA)
        {
            if (value < templateStat)
                value = templateStat;

            if (Item* art = GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            {
                if (ItemTemplate const* temp = art->GetTemplate())
                {
                    if (temp->GetArtifactID())
                    {
                        int32 artLvlDiff = art->GetTotalPurchasedArtifactPowers() - 36;

                        if (artLvlDiff > 0)
                        {
                            float stamMod = 1.f + ((artLvlDiff > 16) ? 16 : artLvlDiff) * 0.00571f;
                            artLvlDiff -= 16;

                            if (artLvlDiff > 0)
                                stamMod += artLvlDiff * 0.00115f;

                            value *= stamMod;
                        }
                    }
                }
            }
        }
        bonusStat = value - baseStat;
    }
    else
    {
        UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);
        bonusStat = m_auraModifiersGroup[unitMod][BASE_VALUE];
        otherMod = m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    }

    if (bonusStat > 0.f)
        bonusStat *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_STAT_BONUS_PCT, stat, true);

    value = baseStat + bonusStat + otherMod;

    value *= GetTotalAuraMultiplierByMiscMaskB(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, 1 << stat);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_PERCENT_STAT, stat, true);
        
    return value;
}

void Player::ApplySpellPowerBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseSpellPower, amount);
    UpdateSpellDamageAndHealingBonus();
}

bool Player::GetCustomPvPMods(float& val, uint32 type, uint32 specID) const
{
    switch (specID)
    {
//         case SPEC_DEMON_HUNER_HAVOC:
//         {
//             if (type == AGILITY_MULTIPLICATIVE)
//             {
//                 val = 0.75f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_DEMON_HUNER_VENGEANCE:
        {
            switch (type)
            {
//                 case AGILITY_MULTIPLICATIVE:
//                 {
//                     val = 0.95f;
//                     return true;
//                 }
                case ATTACK_POWER_FOR_ATTACKER:
                {
                    val = 7.f;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
//         case SPEC_DK_FROST:
//         {
//             if (type == STRENGTH_MULTIPLICATIVE)
//             {
//                 val = 0.8f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_DK_BLOOD:
        {
            switch (type)
            {
//                 case STAMINA_MULTIPLICATIVE:
//                 {
//                     val = 0.85f;
//                     return true;
//                 }
                case ATTACK_POWER_FOR_ATTACKER:
                {
                    val = 7.f;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
        case SPEC_DRUID_BALANCE:
        {
            if (type == INTELLECT_MULTIPLICATIVE)
            {
                val = 1.33f;
                return true;
            }
            else if (type == VERSATILITY_1_2_MULTIPLICATIVE || type == VERSATILITY_2_2_MULTIPLICATIVE)
            {
                val = 1.05f;
                return true;
            }
            break;
        }
//         case SPEC_DRUID_CAT:
//         {
//             if (type == AGILITY_MULTIPLICATIVE)
//             {
//                 val = 0.87f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_DRUID_BEAR:
        {
            switch (type)
            {
//                 case AGILITY_MULTIPLICATIVE:
//                 {
//                     val = 0.85f;
//                     return true;
//                 }
                case ATTACK_POWER_FOR_ATTACKER:
                {
                    val = 7.f;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
//         case SPEC_DRUID_RESTORATION:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 1.27f;
//                 return true;
//             }
//             break;
//         }
//         case SPEC_WARLOCK_AFFLICTION:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 1.1f;
//                 return true;
//             }
//             break;
//         }
//         case SPEC_WARLOCK_DEMONOLOGY:
//         {
//             if (type == STAMINA_MULTIPLICATIVE)
//             {
//                 val = 1.05f;
//                 return true;
//             }
//             break;
//         }
//         case SPEC_MAGE_FROST:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 1.08f;
//                 return true;
//             }
//             break;
//         }
//         case SPEC_MAGE_FIRE:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 0.98f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_MAGE_ARCANE:
        {
            if (type == INTELLECT_MULTIPLICATIVE)
            {
                val = 1.14f;
                return true;
            }
            break;
        }
        case SPEC_SHAMAN_ELEMENTAL:
        {
            if (type == INTELLECT_MULTIPLICATIVE)
            {
                val = 1.335f;
                return true;
            }
            break;
        }
        case SPEC_WARRIOR_PROTECTION:
        {
            if (type == ATTACK_POWER_FOR_ATTACKER)
            {
                val = 7.f;
                return true;
            }
            break;
        }
        case SPEC_ROGUE_ASSASSINATION:
        {
            if (type == AGILITY_MULTIPLICATIVE)
            {
                val = 0.594f;
                return true;
            }
            break;
        }
        case SPEC_ROGUE_SUBTLETY:
        {
            if (type == AGILITY_MULTIPLICATIVE)
            {
                val = 0.72f;
                return true;
            }
            break;
        }
        case SPEC_MONK_WINDWALKER:
        {
            if (type == AGILITY_MULTIPLICATIVE)
            {
                val = 0.685f;
                return true;
            }
            break;
        }
        case SPEC_MONK_BREWMASTER:
        {
            if (type == ATTACK_POWER_FOR_ATTACKER)
            {
                val = 7.f;
                return true;
            }
            break;
        }
//         case SPEC_PALADIN_HOLY:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 1.35f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_PALADIN_PROTECTION:
        {
            switch (type)
            {
//                 case STRENGTH_MULTIPLICATIVE:
//                 {
//                     val = 0.76f;
//                     return true;
//                 }
                case ATTACK_POWER_FOR_ATTACKER:
                {
                    val = 7.f;
                    return true;
                }
                default:
                    break;
            }
            break;
        }
//         case SPEC_PRIEST_SHADOW:
//         {
//             if (type == INTELLECT_MULTIPLICATIVE)
//             {
//                 val = 1.17f;
//                 return true;
//             }
//             break;
//         }
        case SPEC_PRIEST_HOLY:
        {
            if (type == INTELLECT_MULTIPLICATIVE)
            {
                val = 1.49f;
                return true;
            }
            break;
        }
        case SPEC_HUNTER_BEASTMASTER:
        {
            if (type == AGILITY_MULTIPLICATIVE)
            {
                val = 0.69f;
                return true;
            }
            break;
        }
        case SPEC_HUNTER_MARKSMAN:
        {
            if (type == AGILITY_MULTIPLICATIVE)
            {
                val = 0.985f;
                return true;
            }
            break;
        }
//         case SPEC_HUNTER_SURVIVAL:
//         {
//             if (type == AGILITY_MULTIPLICATIVE)
//             {
//                 val = 0.85f;
//                 return true;
//             }
//             break;
//         }
        default:
            break;
    }
    return false;
}

void Player::CalcPvPTemplate(AuraType auratype, float & templateMod, float & otherMod, std::function<bool(AuraEffect const*)> const& predicate)
{
    if (auto const* mTotalAuraList = GetAuraEffectsByType(auratype))
    {
        for (auto const& auraEffect : *mTotalAuraList)
        {
            if (predicate(auraEffect))
            {
                if (auraEffect->GetId() == SPELL_PVP_STATS_TEMPLATE)
                {
                    templateMod += auraEffect->GetAmount();
                }
                else
                {
                    otherMod += auraEffect->GetAmount();
                }
            }
        }
    }
}

void Player::UpdateSpellDamageAndHealingBonus()
{
    int32 amount = m_baseSpellPower;

    AuraEffectList const* mOverrideSpellPowerAuras = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT);
    if (mOverrideSpellPowerAuras && mOverrideSpellPowerAuras->begin() != mOverrideSpellPowerAuras->end())
    {
        for (AuraEffectList::const_iterator itr = mOverrideSpellPowerAuras->begin(); itr != mOverrideSpellPowerAuras->end(); ++itr)
            amount = int32(GetTotalAttackPowerValue(BASE_ATTACK) * (*itr)->GetAmount() / 100.0f);

        SetStatFloatValue(PLAYER_FIELD_OVERRIDE_SPELL_POWER_BY_APPERCENT, GetTotalAuraModifier(SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT));
        SetStatInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, amount);

        for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, amount);
    }
    else
    {
        SetStatInt32Value(PLAYER_FIELD_MOD_HEALING_DONE_POS, SpellBaseHealingBonusDone(SPELL_SCHOOL_MASK_ALL));
        for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            SetStatInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i, SpellBaseDamageBonusDone(SpellSchoolMask(1 << i)));
    }
}

float Player::GetPvpStatScalar() const
{
    float itemLevel = GetFloatValue(PLAYER_FIELD_AVG_ITEM_LEVEL + EquippedAvgItemLevel);
    float statsModifier = 1.0f;

    if (itemLevel > 1000.f)
        itemLevel = 1000.f;

    if (itemLevel < 800.f)
    {
        statsModifier /= 800.f;
        statsModifier *= itemLevel;
    }
    else
    {
        statsModifier += (itemLevel - 800.0f) * 0.001f;
    }

    return statsModifier;
}

uint32 Player::GetExtraPvpStatSpells(uint16 specID) const
{
    switch (specID)
    {
        case SPEC_MAGE_ARCANE:
            return 241121;
        case SPEC_MAGE_FIRE:
            return 241124;
        case SPEC_MAGE_FROST:
            return 241125;
        case SPEC_PALADIN_HOLY:
            return 241145;
        case SPEC_PALADIN_PROTECTION:
            return 241146;
        case SPEC_PALADIN_RETRIBUTION:
            return 241147;
        case SPEC_WARRIOR_ARMS:
            return 241264;
        case SPEC_WARRIOR_FURY:
            return 241269;
        case SPEC_WARRIOR_PROTECTION:
            return 241270;
        case SPEC_DRUID_BALANCE:
            return 241099;
        case SPEC_DRUID_CAT:
            return 241100;
        case SPEC_DRUID_BEAR:
            return 241101;
        case SPEC_DRUID_RESTORATION:
            return 241102;
        case SPEC_DK_BLOOD:
            return 241018;
        case SPEC_DK_FROST:
            return 241047;
        case SPEC_DK_UNHOLY:
            return 241050;
        case SPEC_HUNTER_BEASTMASTER:
            return 241110;
        case SPEC_HUNTER_MARKSMAN:
            return 241114;
        case SPEC_HUNTER_SURVIVAL:
            return 241115;
        case SPEC_PRIEST_DISCIPLINE:
            return 241148;
        case SPEC_PRIEST_HOLY:
            return 241149;
        case SPEC_PRIEST_SHADOW:
            return 241150;
        case SPEC_ROGUE_ASSASSINATION:
            return 241152;
        case SPEC_ROGUE_COMBAT:
            return 241153;
        case SPEC_ROGUE_SUBTLETY:
            return 241154;
        case SPEC_SHAMAN_ELEMENTAL:
            return 241202;
        case SPEC_SHAMAN_ENHANCEMENT:
            return 241203;
        case SPEC_SHAMAN_RESTORATION:
            return 241205;
        case SPEC_WARLOCK_AFFLICTION:
            return 241257;
        case SPEC_WARLOCK_DEMONOLOGY:
            return 241252;
        case SPEC_WARLOCK_DESTRUCTION:
            return 241253;
        case SPEC_MONK_BREWMASTER:
            return 241131;
        case SPEC_MONK_WINDWALKER:
            return 241136;
        case SPEC_MONK_MISTWEAVER:
            return 241134;
        case SPEC_DEMON_HUNER_HAVOC:
            return 241090;
        case SPEC_DEMON_HUNER_VENGEANCE:
            return 241091;
        default:
            return 0;
    }
}

float Player::GetPvpArmorTemplate(uint16 specID) const
{
    switch (specID)
    {
        case SPEC_MAGE_ARCANE:
        case SPEC_MAGE_FIRE:
        case SPEC_MAGE_FROST:
        case SPEC_PRIEST_DISCIPLINE:
        case SPEC_PRIEST_HOLY:
        case SPEC_PRIEST_SHADOW:
            return 1823.f;
        case SPEC_ROGUE_ASSASSINATION:
            return 2447.77f;
        case SPEC_ROGUE_COMBAT:
            return 2430.39f;
        case SPEC_ROGUE_SUBTLETY:
            return 2450.f;
        case SPEC_MONK_BREWMASTER:
        case SPEC_MONK_WINDWALKER:
        case SPEC_MONK_MISTWEAVER:
            return 2430.72f;
        case SPEC_DRUID_BALANCE:
            return 2185.34f;
        case SPEC_DRUID_CAT:
            return 2792.82f;
        case SPEC_DRUID_BEAR:
            return 2549.1f;
        case SPEC_DRUID_RESTORATION:
            return 2428.05f;
        case SPEC_WARLOCK_AFFLICTION:
        case SPEC_WARLOCK_DEMONOLOGY:
        case SPEC_WARLOCK_DESTRUCTION:
            return 1945.f;
        case SPEC_WARRIOR_ARMS:
            return 5244.05f;
        case SPEC_WARRIOR_FURY:
            return 4766.23f;
        case SPEC_WARRIOR_PROTECTION:
            return 6195.8f;
        case SPEC_PALADIN_HOLY:
            return 5809.19f;
        case SPEC_PALADIN_PROTECTION:
            return 5807.f;
        case SPEC_PALADIN_RETRIBUTION:
            return 4468.f;
        case SPEC_DK_BLOOD:
            return 5473.04f;
        case SPEC_DK_FROST:
            return 4913.08f;
        case SPEC_DK_UNHOLY:
            return 4468.f;
        case SPEC_HUNTER_BEASTMASTER:
            return 3402.29f;
        case SPEC_HUNTER_MARKSMAN:
            return 3400.83f;
        case SPEC_HUNTER_SURVIVAL:
            return 3542.4f;
        case SPEC_SHAMAN_ELEMENTAL:
            return 4420.f;
        case SPEC_SHAMAN_ENHANCEMENT:
            return 3543.32f;
        case SPEC_SHAMAN_RESTORATION:
            return 4789.23f;
        case SPEC_DEMON_HUNER_HAVOC:
            return 2279.f;
        case SPEC_DEMON_HUNER_VENGEANCE:
            return 2504.57f;
        default:
            break;
    }
    return 0.f;
}

float Player::GetPvpDamageTemplate(uint16 specID) const
{
    switch (specID)
    {
        case SPEC_MAGE_ARCANE:
        case SPEC_MAGE_FROST:
        case SPEC_WARLOCK_AFFLICTION:
        case SPEC_WARLOCK_DESTRUCTION:
        case SPEC_DRUID_RESTORATION:
        case SPEC_DRUID_BALANCE:
        case SPEC_MONK_MISTWEAVER:
        case SPEC_PRIEST_DISCIPLINE:
        case SPEC_PRIEST_HOLY:
            return 1856.f;
        case SPEC_MAGE_FIRE:
        case SPEC_WARLOCK_DEMONOLOGY:
        case SPEC_PRIEST_SHADOW:
        case SPEC_SHAMAN_ELEMENTAL:
        case SPEC_SHAMAN_RESTORATION:
            return 1442.f;
        case SPEC_HUNTER_MARKSMAN:
        case SPEC_HUNTER_BEASTMASTER:
        case SPEC_WARRIOR_FURY:
        case SPEC_MONK_BREWMASTER:
        case SPEC_HUNTER_SURVIVAL:
        case SPEC_PALADIN_HOLY:
        case SPEC_PALADIN_RETRIBUTION:
        case SPEC_DK_BLOOD:
        case SPEC_DK_UNHOLY:
        case SPEC_WARRIOR_ARMS:
            return 3713.f;
        case SPEC_DRUID_BEAR:
        case SPEC_DRUID_CAT:
        case SPEC_WARRIOR_PROTECTION:
        case SPEC_MONK_WINDWALKER:
        case SPEC_ROGUE_ASSASSINATION:
        case SPEC_ROGUE_COMBAT:
        case SPEC_ROGUE_SUBTLETY:
        case SPEC_PALADIN_PROTECTION:
        case SPEC_DK_FROST:
        case SPEC_SHAMAN_ENHANCEMENT:
        case SPEC_DEMON_HUNER_HAVOC:
        case SPEC_DEMON_HUNER_VENGEANCE:
            return 2885.f;
        default:
            break;
    }
    return 1.f;
}

bool Player::UpdateAllStats()
{
    SendUpdateStat(USM_AGILITY | USM_INTELLECT | USM_STRENGTH | USM_STAMINA | USM_ARMOR | USM_MANA_REGEN | USM_RANGE_AP);

    for (uint8 i = POWER_MANA; i < MAX_POWERS; ++i)
        UpdateMaxPower(Powers(i));

    SendUpdateCR(-1);
    UpdateAllCritPercentages();
    UpdateSpellCritChance();
    UpdateBlockPercentage();
    UpdateParryPercentage();
    UpdateDodgePercentage();
    UpdateSpellDamageAndHealingBonus();
    UpdateExpertise();
    UpdateVersality();
    UpdateCRSpeed();
    UpdateLifesteal();
    UpdateAvoidance();

    for (int i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Player::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        SetResistance(SpellSchools(school), int32(GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school))));

        if (Pet* pet = GetPet())
            pet->UpdateResistances(school);
    }
    else
        SendUpdateStat(USM_ARMOR);
}

void Player::UpdateArmor()
{
    UnitMods unitMod = UNIT_MOD_ARMOR;

    float value = 0.f;

    if (!HasPvpStatsScalingEnabled())
    {
        value = GetModifierValue(unitMod, BASE_VALUE); // base armor (from items)
        value += GetModifierValue(unitMod, TOTAL_VALUE);
    }
    else
    {
        value = (GetPvpArmorTemplate(GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID)) + GetModifierValue(unitMod, TOTAL_VALUE)) * GetPvpStatScalar();
    }

    value *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_BASE_RESISTANCE_PCT, 1);     // armor percent from items
    value *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_RESISTANCE_PCT, 1); 
    value *= GetTotalAuraMultiplier(SPELL_AURA_MOD_BONUS_ARMOR_PCT);

    SetArmor(int32(value));

    if (Pet* pet = GetPet())
        pet->UpdateArmor();

    SendUpdateStat(USM_MELEE_AP);
}

float Player::GetHealthBonusFromStamina()
{
    float ratio = 10.0f;
    if (GtHpPerStaEntry const* hpBase = sHpPerStaGameTable.GetRow(GetEffectiveLevel()))
        ratio = hpBase->Health;

    return GetStat(STAT_STAMINA) * ratio;
}

void Player::UpdateMaxHealth()
{
    UnitMods unitMod = UNIT_MOD_HEALTH;

    float value = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
    value *= GetModifierValue(unitMod, BASE_PCT);
    value += GetModifierValue(unitMod, TOTAL_VALUE) + GetHealthBonusFromStamina();
    value *= GetModifierValue(unitMod, TOTAL_PCT);

    SetMaxHealth(static_cast<uint32>(value)); // uint32 sure?

    if (Pet* pet = GetPet())
        if (pet->IsWarlockPet() || pet->isHunterPet())
            pet->UpdateMaxHealth();
}

void Player::UpdateMaxPower(Powers power)
{
    if (power == POWER_ALTERNATE)
        return;

    int32 cur_maxpower = GetMaxPower(power);
    int32 value = GetCreatePowers(power);

    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY, power);
    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MAX_POWER, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_MAX_MANA, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_ADD_ENERGY_PERCENT, power);

    if (cur_maxpower != value)
        SetMaxPower(power, value);
}

void Player::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mod_pos = UNIT_FIELD_ATTACK_POWER_MOD_POS;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mod_pos = UNIT_FIELD_RANGED_ATTACK_POWER_MOD_POS;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }

    if (!HasAuraType(SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT))
    {
        float val2 = 0.0f;
        auto level = float(GetEffectiveLevel());
        ChrClassesEntry const* entry = sChrClassesStore.LookupEntry(getClass());

        if (ranged)
            val2 = (level + std::max(GetStat(STAT_AGILITY), 0.0f)) * entry->RangedAttackPowerPerAgility;
        else
        {
            float strengthValue = std::max(GetStat(STAT_STRENGTH) * entry->AttackPowerPerStrength, 0.0f);
            float agilityValue = std::max(GetStat(STAT_AGILITY) * entry->AttackPowerPerAgility, 0.0f);

            SpellShapeshiftFormEntry const* form = sSpellShapeshiftFormStore.LookupEntry(GetShapeshiftForm());
            if (form && form->Flags & SHAPESHIFT_FORM_AP_FROM_STRENGTH)
                agilityValue += std::max(GetStat(STAT_AGILITY) * entry->AttackPowerPerStrength, 0.0f);

            val2 = strengthValue + agilityValue;
        }

        SetModifierValue(unitMod, BASE_VALUE, val2);

        float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
        float attPowerMod = GetModifierValue(unitMod, TOTAL_VALUE);

        base_attPower += ranged ? GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_ATTACK_POWER) : GetTotalAuraModifier(SPELL_AURA_MOD_ATTACK_POWER);
        base_attPower *= GetTotalAuraMultiplier(ranged ? SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT : SPELL_AURA_MOD_ATTACK_POWER_PCT);

        float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

        SetFloatValue(PLAYER_FIELD_OVERRIDE_APBY_SPELL_POWER_PERCENT, 0.0f);

        SetInt32Value(index, static_cast<uint32>(base_attPower));        //UNIT_FIELD_(RANGED)_ATTACK_POWER field
        SetInt32Value(index_mod_pos, static_cast<uint32>(attPowerMod));  //UNIT_FIELD_(RANGED)_ATTACK_POWER_MOD_POS field

        SetFloatValue(index_mult, attPowerMultiplier);      //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    }
    else
    {
        auto ApBySpellPct = float(GetTotalAuraModifier(SPELL_AURA_OVERRIDE_AP_BY_SPELL_POWER_PCT));
        int32 spellPower = GetSpellPowerDamage();

        SetFloatValue(PLAYER_FIELD_OVERRIDE_APBY_SPELL_POWER_PERCENT, ApBySpellPct);
        SetModifierValue(unitMod, BASE_VALUE, ApBySpellPct / 100.0f * spellPower);
        SetInt32Value(index, ApBySpellPct / 100.0f * spellPower);

        SetInt32Value(index_mod_pos, 0);                    //UNIT_FIELD_(RANGED)_ATTACK_POWER_MOD_POS field
        float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;
        if (attPowerMultiplier < 0)
            SetFloatValue(index_mult, attPowerMultiplier);  //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    }

    Pet* pet = GetPet();                                //update pet's AP
    //automatically update weapon damage after attack power modification
    if (ranged)
    {
        UpdateDamagePhysical(RANGED_ATTACK);
        if (pet && pet->isHunterPet()) // At ranged attack change for hunter pet
            pet->UpdateAttackPowerAndDamage();
    }
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        if (Item* offhand = GetWeaponForAttack(OFF_ATTACK, true))
            if (CanDualWield() || offhand->GetTemplate()->GetFlags3() & ITEM_FLAG3_ALWAYS_ALLOW_DUAL_WIELD)
                UpdateDamagePhysical(OFF_ATTACK);
        if (getClass() == CLASS_SHAMAN || getClass() == CLASS_PALADIN)                      // mental quickness
            UpdateSpellDamageAndHealingBonus();

        if (pet)
            pet->UpdateAttackPowerAndDamage();
    }

    if (HasAuraType(SPELL_AURA_OVERRIDE_SPELL_POWER_BY_AP_PCT))
        UpdateSpellDamageAndHealingBonus();
}

void Player::CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage)
{
    UnitMods unitMod;
    uint8 slot;
    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            slot = EQUIPMENT_SLOT_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            slot = EQUIPMENT_SLOT_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            slot = EQUIPMENT_SLOT_MAINHAND;
            break;
    }

    float const attackPowerMod = std::max(GetAPMultiplier(attType, normalized), 0.25f);

    float baseValue = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) / 3.5f * attackPowerMod;
    float basePct = GetModifierValue(unitMod, BASE_PCT);
    float totalValue = GetModifierValue(unitMod, TOTAL_VALUE);
    float totalPct = addTotalPct ? GetModifierValue(unitMod, TOTAL_PCT) : unitMod == UNIT_MOD_DAMAGE_OFFHAND ? 0.5f : 1.0f;

    AddPct(totalPct, GetFloatValue(PLAYER_FIELD_VERSATILITY) + GetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS));

    float weaponMinDamage = 1.f;
    float weaponMaxDamage = 1.f;

    if (HasPvpStatsScalingEnabled())
    {
        if (Item* pItem = GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (ItemTemplate const* temp = pItem->GetTemplate())
            {
                float pvpDamageTemp = GetPvpDamageTemplate(GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID));

                weaponMinDamage = (temp->GetDelay() / 1000.f) * pvpDamageTemp;
                weaponMaxDamage = (temp->GetDelay() / 1000.f) * pvpDamageTemp;
            }
        }
    }
    else
    {
        weaponMinDamage = GetWeaponDamageRange(attType, MINDAMAGE);
        weaponMaxDamage = GetWeaponDamageRange(attType, MAXDAMAGE);
    }

    auto const& shapeshift = sSpellShapeshiftFormStore.LookupEntry(GetShapeshiftForm());
    if (shapeshift && shapeshift->CombatRoundTime)
    {
        float weaponSpeed = BASE_ATTACK_TIME / 1000.f;
        float combatRoundTime = shapeshift->CombatRoundTime / 1000.f;

        if (Item* weapon = GetWeaponForAttack(BASE_ATTACK, false))
            weaponSpeed = weapon->GetTemplate()->GetDelay() / 1000.f;

        weaponMinDamage = (baseValue + weaponMinDamage) / weaponSpeed * combatRoundTime;
        weaponMaxDamage = (baseValue + weaponMaxDamage) / weaponSpeed * combatRoundTime;

        minDamage = (weaponMinDamage * basePct + totalValue) * totalPct;
        maxDamage = (weaponMaxDamage * basePct + totalValue) * totalPct;
        return;
    }
    else if (!CanUseAttackType(attType))      //check if player not in form but still can't use (disarm case)
    {
        //cannot use ranged/off attack, set values to 0
        if (attType != BASE_ATTACK)
        {
            minDamage = 0;
            maxDamage = 0;
            return;
        }
        weaponMinDamage = BASE_MINDAMAGE;
        weaponMaxDamage = BASE_MAXDAMAGE;
    }

    minDamage = ((baseValue + weaponMinDamage) * basePct + totalValue) * totalPct;
    maxDamage = ((baseValue + weaponMaxDamage) * basePct + totalValue) * totalPct;
}

void Player::UpdateDamagePhysical(WeaponAttackType attType)
{
    float mindamage = 1.0f;
    float maxdamage = 1.0f;

    CalculateMinMaxDamage(attType, false, true, mindamage, maxdamage);

    switch (attType)
    {
        case BASE_ATTACK:
        default:
            SetStatFloatValue(UNIT_FIELD_MIN_DAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAX_DAMAGE, maxdamage);
            break;
        case OFF_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MIN_OFF_HAND_DAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAX_OFF_HAND_DAMAGE, maxdamage);
            break;
        case RANGED_ATTACK:
            SetStatFloatValue(UNIT_FIELD_MIN_RANGED_DAMAGE, mindamage);
            SetStatFloatValue(UNIT_FIELD_MAX_RANGED_DAMAGE, maxdamage);
            break;
    }
}

void Player::UpdateBlockPercentage()
{
    float value = 0.0f;

    if (CanBlock())
    {
        value = 5.0f;
        value += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
        value += GetRatingBonusValue(CR_BLOCK);
    }

    SetStatFloatValue(PLAYER_FIELD_BLOCK_PERCENTAGE, std::max(0.0f, value));
}

void Player::UpdateCritPercentage(WeaponAttackType attType)
{
    BaseModGroup modGroup;
    uint16 index;
    CombatRating cr;

    switch (attType)
    {
        case OFF_ATTACK:
            modGroup = OFFHAND_CRIT_PERCENTAGE;
            index = PLAYER_FIELD_OFFHAND_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
        case RANGED_ATTACK:
            modGroup = RANGED_CRIT_PERCENTAGE;
            index = PLAYER_FIELD_RANGED_CRIT_PERCENTAGE;
            cr = CR_CRIT_RANGED;
            break;
        case BASE_ATTACK:
        default:
            modGroup = CRIT_PERCENTAGE;
            index = PLAYER_FIELD_CRIT_PERCENTAGE;
            cr = CR_CRIT_MELEE;
            break;
    }

    float value = GetTotalPercentageModValue(modGroup);
    value += GetRatingBonusValue(cr);
    SetStatFloatValue(index, value);

    if (HasAuraType(SPELL_AURA_CONVERT_CRIT_RATING_PCT_TO_PARRY_RATING))
        UpdateParryPercentage();

    if (attType == BASE_ATTACK && getClass() == CLASS_MONK && GetSpecializationId() == SPEC_MONK_BREWMASTER)
    {
        if (AuraEffect* CelestialFortune = GetAuraEffect(216519, EFFECT_1))
        {
            CelestialFortune->SetCanBeRecalculated(true);
            CelestialFortune->RecalculateAmount(this);
        }
    }
}

void Player::UpdateHast()
{
//     SPELL_AURA_MELEE_SLOW                  = 0;
//     SPELL_AURA_MOD_MELEE_HASTE             = 1;
//     SPELL_AURA_MOD_MELEE_HASTE_2           = 2;
//     SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK = 3;
//     SPELL_AURA_MOD_RANGED_HASTE            = 4;
//     SPELL_AURA_MOD_RANGED_HASTE_3          = 5;
//     SPELL_AURA_MOD_MELEE_RANGED_HASTE      = 6;
//     SPELL_AURA_HASTE_SPELLS                = 7;
//     SPELL_AURA_MOD_CASTING_SPEED           = 8;

    std::vector<float> hastAurasMod;
    hastAurasMod.assign(9, 1.0f);

    if (m_updateStatsMask & USM_MELEE_HAST)
    {
        hastAurasMod[0] = GetTotalAuraMultiplier(SPELL_AURA_MELEE_SLOW);
        hastAurasMod[1] = GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_HASTE);
        hastAurasMod[2] = GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_HASTE_2);
        hastAurasMod[6] = GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_RANGED_HASTE);

        UpdateMeleeHastMod(hastAurasMod[0] * hastAurasMod[1] * hastAurasMod[2] * hastAurasMod[6]);
    }

    if (m_updateStatsMask & USM_RANGE_HAST)
    {
        hastAurasMod[4] = GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_HASTE);
        hastAurasMod[5] = GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_HASTE_3);

        if (!(m_updateStatsMask & USM_MELEE_HAST))
        {
            hastAurasMod[6] = GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_RANGED_HASTE);
            hastAurasMod[0] = GetTotalAuraMultiplier(SPELL_AURA_MELEE_SLOW);
        }

        UpdateRangeHastMod(hastAurasMod[0] * hastAurasMod[4] * hastAurasMod[5] * hastAurasMod[6]);
    }

    if (m_updateStatsMask & USM_HAST)
    {
        hastAurasMod[3] = GetTotalAuraMultiplier(SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK);
        hastAurasMod[8] = GetTotalAuraMultiplier(SPELL_AURA_MOD_CASTING_SPEED);
        hastAurasMod[7] = GetTotalAuraMultiplier(SPELL_AURA_HASTE_SPELLS);

        if (!(m_updateStatsMask & (USM_MELEE_HAST|USM_RANGE_HAST)))
            hastAurasMod[0] = GetTotalAuraMultiplier(SPELL_AURA_MELEE_SLOW);

        UpdateHastMod(hastAurasMod[3] * hastAurasMod[8] * hastAurasMod[7] * hastAurasMod[0]);
    }

    if (m_updateStatsMask & USM_CAST_HAST)
    {
        if (!(m_updateStatsMask & USM_HAST))
            hastAurasMod[3] = GetTotalAuraMultiplier(SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK);

        if (!(m_updateStatsMask & (USM_MELEE_HAST|USM_RANGE_HAST|USM_HAST)))
            hastAurasMod[0] = GetTotalAuraMultiplier(SPELL_AURA_MELEE_SLOW);

        UpdateCastHastMods(hastAurasMod[0] * hastAurasMod[3]);
    }

    if (!(m_updateStatsMask & (USM_MELEE_HAST|USM_RANGE_HAST|USM_HAST|USM_CAST_HAST)))
        hastAurasMod[0] = GetTotalAuraMultiplier(SPELL_AURA_MELEE_SLOW);

    UpdateHastRegen(hastAurasMod[0]);

    if (GetPower(POWER_FOCUS))
        UpdatePowerRegen(POWER_FOCUS);

    if (GetPower(POWER_ENERGY))
        UpdatePowerRegen(POWER_ENERGY);

    if (Pet* pet = GetPet())
    {
        if ((m_updateStatsMask & USM_MELEE_HAST) && pet->m_baseMHastRatingPct != m_baseMHastRatingPct)
            pet->UpdateMeleeHastMod();

        if ((m_updateStatsMask & USM_HAST) && pet->m_baseHastRatingPct != m_baseHastRatingPct)
            pet->UpdateHastMod();

        if ((m_updateStatsMask & USM_RANGE_HAST) && pet->m_baseRHastRatingPct != m_baseRHastRatingPct)
            pet->UpdateRangeHastMod();
    }

    SendOperationsAfterDelay(OAD_UPDATE_RUNES_REGEN);
}

void Player::UpdateHastRegen(float auraMods)
{
    float val = 1.0f / m_baseMHastRatingPct;
    float amount = 100.0f;

    amount *= auraMods;
    amount -= 100.0f;

    if (amount > 0)
        ApplyPercentModFloatVar(val, amount, false);
    else
        ApplyPercentModFloatVar(val, -amount, true);

    SetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN, std::min(1.0f, val));

    if (AuraEffectList const* CooldownByMeleeHaste = GetAuraEffectsByType(SPELL_AURA_MOD_COOLDOWN_BY_HASTE_REGEN))
    {
        for (AuraEffectList::const_iterator itr = CooldownByMeleeHaste->begin(); itr != CooldownByMeleeHaste->end(); ++itr)
        {
            (*itr)->SetCanBeRecalculated(true);
            (*itr)->RecalculateAmount(this);
        }
    }
    if (AuraEffectList const* GcdByMeleeHaste = GetAuraEffectsByType(SPELL_AURA_MOD_GLOBAL_COOLDOWN_BY_HASTE_REGEN))
    {
        for (AuraEffectList::const_iterator itr = GcdByMeleeHaste->begin(); itr != GcdByMeleeHaste->end(); ++itr)
        {
            (*itr)->SetCanBeRecalculated(true);
            (*itr)->RecalculateAmount(this);
        }
    }
    if (AuraEffectList const* RecHasteReg = GetAuraEffectsByType(SPELL_AURA_CHARGE_RECOVERY_AFFECTED_BY_HASTE_REGEN))
    {
        for (AuraEffectList::const_iterator itr = RecHasteReg->begin(); itr != RecHasteReg->end(); ++itr)
        {
            (*itr)->SetCanBeRecalculated(true);
            (*itr)->RecalculateAmount(this);
        }
    }
}

void Player::UpdateCastHastMods(float auraMods)
{
    float amount = 100.0f;
    amount += GetRatingBonusValue(CR_HASTE_MELEE);

    m_baseMHastRatingPct = amount / 100.0f;

    amount *= auraMods;
    amount -= 100.0f;

    float value = 1.0f;

    if (amount > 0)
        ApplyPercentModFloatVar(value, amount, false);
    else
        ApplyPercentModFloatVar(value, -amount, true);

    SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, value);
}

void Player::UpdateMeleeHastMod(float auraMods)
{
    float amount = 100.0f;
    amount += GetRatingBonusValue(CR_HASTE_MELEE);

    m_baseMHastRatingPct = amount / 100.0f;

    amount *= auraMods;
    amount -= 100.0f;

    //TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "UpdateMeleeHastMod mod %f", mod);

    float value = 1.0f;

    if (amount > 0)
        ApplyPercentModFloatVar(value, amount, false);
    else
        ApplyPercentModFloatVar(value, -amount, true);

    SetFloatValue(UNIT_FIELD_MOD_HASTE, value);

    for (uint8 i = BASE_ATTACK; i < RANGED_ATTACK; ++i)
        CalcAttackTimePercentMod(WeaponAttackType(i), value);
}

void Player::UpdateHastMod(float auraMods)
{
    float amount = 100.0f;
    amount += GetRatingBonusValue(CR_HASTE_SPELL);

    m_baseHastRatingPct = amount / 100.0f;

    amount *= auraMods;
    amount -= 100.0f;

    //TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "UpdateHastMod amount %f", amount);

    float value = 1.0f;

    if (amount > 0)
        ApplyPercentModFloatVar(value, amount, false);
    else
        ApplyPercentModFloatVar(value, -amount, true);

    SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, value);

    SendUpdateStat(USM_MANA_REGEN);
}

void Player::UpdateRangeHastMod(float auraMods)
{
    float amount = 100.0f;
    amount += GetRatingBonusValue(CR_HASTE_RANGED);
    m_baseRHastRatingPct = amount / 100.0f;

    amount *= auraMods;
    amount -= 100.0f;

    //TC_LOG_ERROR(LOG_FILTER_NETWORKIO, "UpdateRangeHastMod mod %f", mod);

    float value = 1.0f;

    if (amount > 0)
        ApplyPercentModFloatVar(value, amount, false);
    else
        ApplyPercentModFloatVar(value, -amount, true);

    if (value > 1.0f)
        value = 1.0f;

    SetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE, value);

    CalcAttackTimePercentMod(RANGED_ATTACK, value);
}

void Player::UpdateShieldBlock()
{
    SetUInt32Value(PLAYER_FIELD_SHIELD_BLOCK, GetTotalPercentageModValue(SHIELD_BLOCK_VALUE));
}

void Player::UpdateAllCritPercentages()
{
    float value = 5.0f;

    SetBaseModValue(CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(OFFHAND_CRIT_PERCENTAGE, PCT_MOD, value);
    SetBaseModValue(RANGED_CRIT_PERCENTAGE, PCT_MOD, value);

    UpdateCritPercentage(BASE_ATTACK);
    UpdateCritPercentage(OFF_ATTACK);
    UpdateCritPercentage(RANGED_ATTACK);
}

const float m_diminishing_k[MAX_CLASSES] =
{
    0.9560f,  // Warrior
    0.8860f,  // Paladin
    0.9880f,  // Hunter
    0.9880f,  // Rogue
    0.9560f,  // Priest
    0.9560f,  // DK
    0.9880f,  // Shaman
    0.9830f,  // Mage
    0.9830f,  // Warlock
    1.4220f,  // Monk
    1.2220f,  // Druid
    0.9880f,  // DemonHunter
};

const float coeff[11] = // agility per dodge / strength per parry (100 - 110 lvl)
{
    176.376068f,
    192.707186f,
    207.949562f,
    224.280680f,
    248.232985f,
    296.137596f,
    352.752137f,
    400.656748f,
    497.554712f,
    617.316239f,
    786.071120f
};

void Player::UpdateParryPercentage()
{
    int8 lDiff = GetEffectiveLevel() - 100;
    lDiff = lDiff > 0 ? lDiff : 0;
    lDiff = lDiff > 10 ? 10 : lDiff;

    float baseParry = 3.f + GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT) + GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT2);

    float ratingParry = GetRatingBonusValue(CR_PARRY);
    ratingParry += CalculatePct(GetRatingBonusValue(CR_CRIT_SPELL), GetTotalAuraModifier(SPELL_AURA_CONVERT_CRIT_RATING_PCT_TO_PARRY_RATING));
    float bonusParry = GetFloatValue(UNIT_FIELD_STAT_POS_BUFF + STAT_STRENGTH) / (coeff[lDiff] * 1.666f) + ratingParry;

    ApplyDiminishingStat(bonusParry, getClass());

    float val = baseParry + bonusParry;
    SetStatFloatValue(PLAYER_FIELD_PARRY_PERCENTAGE, std::max(0.f, val));
}

void Player::UpdateDodgePercentage()
{
    int8 lDiff = GetEffectiveLevel() - 100;
    lDiff = lDiff > 0 ? lDiff : 0;
    lDiff = lDiff > 10 ? 10 : lDiff;

    float baseDodge = 3.f + GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);

    float ratingDodge = GetRatingBonusValue(CR_DODGE);
    ratingDodge += CalculatePct(GetRatingBonusValue(CR_CRIT_SPELL), GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_CHANCE_FROM_CRIT_CHANCE_BY_MASK));
    float bonusDodge = GetFloatValue(UNIT_FIELD_STAT_POS_BUFF + STAT_AGILITY) / (coeff[lDiff] * 1.666f) + ratingDodge;

    ApplyDiminishingStat(bonusDodge, getClass());

    float val = baseDodge + bonusDodge;
    SetStatFloatValue(PLAYER_FIELD_DODGE_PERCENTAGE, std::max(0.f, val));

    if (Aura* aura = GetAura(155578)) // Guardian of Elune
        aura->RecalculateAmountOfEffects(true);
}

void Player::SendOperationsAfterDelay(uint32 mask)
{
    m_operationsAfterDelayMask |= mask;
}

void Player::SendUpdateStat(uint32 updateStatMask)
{
    m_updateStatsMask |= updateStatMask;
}

void Player::SendUpdateCR(int32 updateCRMask)
{
    m_updateCRsMask |= updateCRMask;
}

void Player::UpdateSpellCritChance()
{
    float crit = 5.0f;
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
    crit += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
    crit += GetRatingBonusValue(CR_CRIT_SPELL);

    SetFloatValue(PLAYER_FIELD_SPELL_CRIT_PERCENTAGE, crit);
}

void Player::UpdateMeleeHitChances()
{
    m_modMeleeHitChance = GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
    SetFloatValue(PLAYER_FIELD_UI_HIT_MODIFIER, m_modMeleeHitChance);

    m_modMeleeHitChance += GetRatingBonusValue(CR_HIT_MELEE);
}

void Player::UpdateRangedHitChances()
{
    m_modRangedHitChance = GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
    m_modRangedHitChance += GetRatingBonusValue(CR_HIT_RANGED);
}

void Player::UpdateSpellHitChances()
{
    m_modSpellHitChance = GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
    SetFloatValue(PLAYER_FIELD_UI_SPELL_HIT_MODIFIER, m_modSpellHitChance);

    m_modSpellHitChance += GetRatingBonusValue(CR_HIT_SPELL);
}

void Player::UpdateExpertise()
{
    bool first = true;
    for (uint8 i = BASE_ATTACK; i < MAX_ATTACK; ++i)
    {
        auto const& weapon = GetWeaponForAttack(WeaponAttackType(i), true);
        float expertise = GetRatingBonusValue(CR_EXPERTISE) + GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE, [weapon](AuraEffect const* aurEff) -> bool
        {
            if (aurEff->GetSpellInfo()->EquippedItemClass == -1)
                return true;
            else if (weapon && weapon->IsFitToSpellRequirements(aurEff->GetSpellInfo()))
                return true;
            return false;
        });

        if (expertise < 0)
            expertise = 0.0f;

        if (first || expertise > m_expertise)
        {
            m_expertise = expertise;
            first = false;
        }

        SetFloatValue(PLAYER_FIELD_MAINHAND_EXPERTISE + i, expertise);
    }
}

void Player::ApplyManaRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseManaRegen, amount);
    SendUpdateStat(USM_MANA_REGEN);
}

void Player::ApplyHealthRegenBonus(int32 amount, bool apply)
{
    _ModifyUInt32(apply, m_baseHealthRegen, amount);
}

void Player::_ApplyAllStatBonuses()
{
    SetCanModifyStats(false);

    _ApplyAllAuraStatMods();
    _ApplyAllItemMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

void Player::_RemoveAllStatBonuses()
{
    SetCanModifyStats(false);

    _RemoveAllItemMods();
    _RemoveAllAuraStatMods();

    SetCanModifyStats(true);

    UpdateAllStats();
}

/*#######################################
+ ########                         ########
+ ########   UNITS STAT SYSTEM     ########
+ ########                         ########
+ #######################################*/

void Unit::UpdateManaRegen()
{
    // Skip regeneration for power type we cannot have
    uint32 powerIndex = GetPowerIndex(POWER_MANA);
    if (powerIndex == MAX_POWERS || powerIndex >= MAX_POWERS_PER_CLASS)
        return;

    // Mana regen from spirit
    // percent of base mana per 5 sec
    float manaMod = 5.0f;

    // manaMod% of base mana every 5 seconds is base for all classes
    float baseRegen = CalculatePct(float(GetCreateMana()), manaMod) / 5.0f;
    float auraMp5regen = 0.0f;
    if (AuraEffectList const* ModPowerRegenAuras = GetAuraEffectsByType(SPELL_AURA_MOD_POWER_REGEN))
    {
        for (AuraEffectList::const_iterator i = ModPowerRegenAuras->begin(); i != ModPowerRegenAuras->end(); ++i)
        {
            if (Powers((*i)->GetMiscValue()) == POWER_MANA)
            {
                bool periodic = false;
                if (Aura* aur = (*i)->GetBase())
                    if (AuraEffect const* aurEff = aur->GetEffect(1))
                        if (aurEff->GetAuraType() == SPELL_AURA_PERIODIC_DUMMY)
                        {
                            periodic = true;
                            auraMp5regen += aurEff->GetAmount() / 5.0f;
                        }
                if (!periodic)
                    auraMp5regen += (*i)->GetAmount() / 5.0f;
            }
        }
    }

    float interruptMod = GetTotalAuraMultiplier(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
    float pctManaMod = GetTotalAuraMultiplier(SPELL_AURA_MOD_MANA_REGEN_PERCENT);
    float pctRegenMod = GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, POWER_MANA);

    float manaRegen = (baseRegen * pctManaMod + auraMp5regen) * pctRegenMod;
    float manaRegenInterupted = (baseRegen * pctManaMod + auraMp5regen) * interruptMod * pctRegenMod;

    if (manaRegen < 0.0f)
        manaRegen = 0.0f;
    if (manaRegenInterupted < 0.0f)
        manaRegenInterupted = 0.0f;

    SetStatFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + powerIndex, manaRegen);
    SetStatFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER + powerIndex, manaRegenInterupted);
}

void Unit::UpdateCastHastMods()
{
    std::list<AuraType> auratypelist;
    auratypelist.push_back(SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK);
    auratypelist.push_back(SPELL_AURA_MELEE_SLOW);

    float value = 1.0f;
    if (isAnySummons())
        if (Unit* owner = GetAnyOwner())
            value = owner->GetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE);

    value /= GetTotalForAurasMultiplier(&auratypelist);

    if (value > 1.0f)
        value = 1.0f;

    SetFloatValue(UNIT_FIELD_MOD_SPELL_HASTE, value);
}

void Unit::UpdateMeleeHastMod()
{
    std::list<AuraType> auratypelist;
    auratypelist.push_back(SPELL_AURA_MOD_MELEE_HASTE);
    auratypelist.push_back(SPELL_AURA_MOD_MELEE_HASTE_2);
    auratypelist.push_back(SPELL_AURA_MOD_MELEE_RANGED_HASTE);
    auratypelist.push_back(SPELL_AURA_MELEE_SLOW);

    float value = 1.0f;
    if (isAnySummons())
        if (Unit* owner = GetAnyOwner())
            value = owner->GetFloatValue(UNIT_FIELD_MOD_HASTE);

    value /= GetTotalForAurasMultiplier(&auratypelist);

    if (value > 1.0f)
        value = 1.0f;

    SetFloatValue(UNIT_FIELD_MOD_HASTE, value);

    for (uint8 i = BASE_ATTACK; i < RANGED_ATTACK; ++i)
        CalcAttackTimePercentMod(WeaponAttackType(i), value);

    // Disable crashed http://pastebin.com/Dxzha0se
    // if (Unit* owner = GetOwner())
        // if (owner->IsInWorld() && owner->getClass() == CLASS_HUNTER && (amount != 0.0f || (oldValue != 1.0f && value != oldValue)))
            // owner->SetFloatValue(PLAYER_FIELD_MOD_PET_HASTE, value);

    if (GetPower(POWER_ENERGY))
        UpdatePowerRegen(POWER_ENERGY);
}

void Unit::UpdateHastMod()
{
    std::list<AuraType> auratypelist;
    auratypelist.push_back(SPELL_AURA_MOD_CASTING_SPEED_NOT_STACK);
    auratypelist.push_back(SPELL_AURA_MOD_CASTING_SPEED);
    auratypelist.push_back(SPELL_AURA_HASTE_SPELLS);
    auratypelist.push_back(SPELL_AURA_MELEE_SLOW);

    //TC_LOG_DEBUG(LOG_FILTER_NETWORKIO, "UpdateHastMod amount %f", amount);

    float value = 1.0f;
    if (isAnySummons())
        if (Unit* owner = GetCharmerOrOwner())
            value = owner->GetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED);

    value /= GetTotalForAurasMultiplier(&auratypelist);

    if (value > 1.0f)
        value = 1.0f;

    SetFloatValue(UNIT_FIELD_MOD_CASTING_SPEED, value);

    UpdateManaRegen();
}

void Unit::UpdateRangeHastMod()
{
    std::list<AuraType> auratypelist;
    auratypelist.push_back(SPELL_AURA_MOD_RANGED_HASTE);
    auratypelist.push_back(SPELL_AURA_MOD_RANGED_HASTE_3);
    auratypelist.push_back(SPELL_AURA_MOD_MELEE_RANGED_HASTE);
    auratypelist.push_back(SPELL_AURA_MELEE_SLOW);

    float value = 1.0f;
    if (isAnySummons())
        if (Unit* owner = GetCharmerOrOwner())
            value = owner->GetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE);

    value /= GetTotalForAurasMultiplier(&auratypelist);

    if (value > 1.0f)
        value = 1.0f;

    SetFloatValue(UNIT_FIELD_MOD_RANGED_HASTE, value);

    CalcAttackTimePercentMod(RANGED_ATTACK, value);

    if (GetPower(POWER_FOCUS))
        UpdatePowerRegen(POWER_FOCUS);
}

void Unit::UpdatePowerRegen(uint32 power)
{
    PowerTypeEntry const* powerEntry = sDB2Manager.GetPowerType(power);
    if (!powerEntry)
        return;

    uint32 powerIndex = GetPowerIndex(power);
    if (powerIndex == MAX_POWERS || powerIndex >= MAX_POWERS_PER_CLASS)
        return;

    float powerRegenPct = GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_POWER_REGEN_PERCENT, power);
    float hastReg = 1.f; 

    if (isAnySummons())
    {
        if (Unit* owner = GetOwner())
            hastReg = owner->GetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN);
    }
    else
    {
        hastReg = GetFloatValue(UNIT_FIELD_MOD_HASTE_REGEN);
    }

    if (power == POWER_RUNES)
    {
        if (Player* plr = ToPlayer())
        {
            float value = hastReg;
            int32 pRP = powerRegenPct * 100.f - 100.f;

            if (pRP > 0)
            {
                ApplyPercentModFloatVar(value, pRP, false);
            }
            else
            {
                ApplyPercentModFloatVar(value, -pRP, true);
            }

            plr->SetRuneCooldownCoef(value);
        }
    }

    hastReg = 100.f / hastReg * powerRegenPct;

    if (hastReg < 0.0f) //Example 200040
        hastReg = 0.0f;

    float valOutCOmbat = powerEntry->RegenPeace * (hastReg / 100.0f) - powerEntry->RegenPeace;
    valOutCOmbat += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) / 5.0f;
    SetFloatValue(UNIT_FIELD_POWER_REGEN_FLAT_MODIFIER + powerIndex, valOutCOmbat);

    float valInCOmbat = powerEntry->RegenCombat * (hastReg / 100.0f) - powerEntry->RegenCombat;
    valInCOmbat += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_POWER_REGEN, power) / 5.0f;
    SetFloatValue(UNIT_FIELD_POWER_REGEN_INTERRUPTED_FLAT_MODIFIER + powerIndex, valInCOmbat);

    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Unit::UpdatePowerRegen RegenPeace valOutCOmbat %f, powerIndex %i, power %i", valOutCOmbat, powerIndex, power);
    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Unit::UpdatePowerRegen RegenCombat valInCOmbat %f, powerIndex %i, power %i", valInCOmbat, powerIndex, power);
    // TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Unit::UpdatePowerRegen powerIndex %i, power %i, RegenCombat %f, RegenPeace %f", powerIndex, power, powerEntry->RegenCombat, powerEntry->RegenPeace);
}

/*#######################################
########                         ########
########    MOBS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Creature::UpdateStats(Stats /*stat*/)
{
    return true;
}

float Creature::GetTotalStatValue(Stats stat)
{
    auto unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if (m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    float value = m_auraModifiersGroup[unitMod][BASE_VALUE] * m_auraModifiersGroup[unitMod][BASE_PCT_EXCLUDE_CREATE];
    value += GetCreateStat(stat);
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

bool Creature::UpdateAllStats()
{
    UpdateMaxHealth();
    UpdateAttackPowerAndDamage();
    UpdateAttackPowerAndDamage(true);
    UpdateMaxPower(getPowerType());
    UpdateMaxPower(POWER_ALTERNATE);

    for (int8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Creature::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
        SetResistance(SpellSchools(school), int32(GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school))));
    else
        UpdateArmor();
}

void Creature::UpdateArmor()
{
    SetArmor(int32(GetTotalAuraModValue(UNIT_MOD_ARMOR)));
}

void Creature::UpdateMaxHealth()
{
    float value = GetTotalAuraModValue(UNIT_MOD_HEALTH);
    float mod = 1.0f;
    float percHealth = GetHealthPct();
    if (!GetMap()->Instanceable())
    {
        auto count = GetSizeSaveThreat();
        if (IsPersonal() && count)
        {
            mod += _GetHealthModPersonal(count) * count;
            value *= mod;
        }
            
        SetMaxHealth(uint64(value));

        if (IsPersonal() && count)
            SetHealth(CalculatePct(GetMaxHealth(), percHealth));
        return;
    }
    if (GetMap()->IsNeedRecalc())
    {
        if (auto count = GetPlayerCount())
        {
            mod += _GetHealthModPersonal(count) * count;
            value *= mod;

            SetMaxHealth(static_cast<uint64>(value));
            SetHealth(CalculatePct(GetMaxHealth(), percHealth));
        }
        return;
    }
    SetMaxHealth(static_cast<uint64>(value));
}

void Creature::UpdateMaxPower(Powers power)
{
    int32 cur_maxpower = GetMaxPower(power);
    int32 value = GetCreatePowers(power);

    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY, power);
    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MAX_POWER, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_MAX_MANA, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_ADD_ENERGY_PERCENT, power);

    if (cur_maxpower != value)
        SetMaxPower(power, value);
}

void Creature::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }

    float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
    float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

    SetInt32Value(index, static_cast<uint32>(base_attPower));            //UNIT_FIELD_(RANGED)_ATTACK_POWER field
    SetFloatValue(index_mult, attPowerMultiplier);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field

    //automatically update weapon damage after attack power modification
    if (ranged)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }
}

void Creature::UpdateDamagePhysical(WeaponAttackType attType)
{
    UnitMods unitMod;
    switch (attType)
    {
        case BASE_ATTACK:
        default:
            unitMod = UNIT_MOD_DAMAGE_MAINHAND;
            break;
        case OFF_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_OFFHAND;
            break;
        case RANGED_ATTACK:
            unitMod = UNIT_MOD_DAMAGE_RANGED;
            break;
    }

    m_base_value = GetModifierValue(unitMod, BASE_VALUE);
    m_base_pct = GetModifierValue(unitMod, BASE_PCT);
    m_total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    m_total_pct = GetModifierValue(unitMod, TOTAL_PCT);
    m_dmg_multiplier = GetCreatureTemplate()->dmg_multiplier;

    if (GetCreatureDiffStat() && GetCreatureDiffStat()->dmg_multiplier > 0.0f)
        m_dmg_multiplier *= GetCreatureDiffStat()->dmg_multiplier;

    // Example Spell 243042
    if (GetAnyOwner() && GetAnyOwner()->IsPlayer())
    {
        if (auto tempSum = ToTempSummon())
            if (tempSum->m_Properties && tempSum->m_Properties->Flags & SUMMON_PROP_FLAG_UNK18) //Companions?
                return;
    }

    switch (GetMap()->GetDifficultyID())
    {
        case DIFFICULTY_NONE:
            if (m_dmg_multiplier == 1.0f)
            {
                if (isElite())
                    m_dmg_multiplier = 7.0f;
                else if (isWorldBoss())
                    m_dmg_multiplier = 40.0f;
            }
            break;
        case DIFFICULTY_N_SCENARIO:
            m_dmg_multiplier *= 1.5f;
            break;
        case DIFFICULTY_NORMAL:
        case DIFFICULTY_HC_SCENARIO:
        case DIFFICULTY_10_N:
        case DIFFICULTY_10_HC:
            m_dmg_multiplier *= 2.0f;
            break;
        case DIFFICULTY_HEROIC:
        case DIFFICULTY_25_N:
        case DIFFICULTY_25_HC:
        case DIFFICULTY_LFR:
            m_dmg_multiplier *= 5.0f;
            break;
        case DIFFICULTY_MYTHIC_DUNGEON:
        case DIFFICULTY_MYTHIC_KEYSTONE:
            m_dmg_multiplier *= IsDungeonBoss() ? 15.0f : 10.0f;
            break;
        case DIFFICULTY_LFR_RAID:
            m_dmg_multiplier *= IsDungeonBoss() ? 20.0f : 9.0f;
            break;
        case DIFFICULTY_NORMAL_RAID:
            m_dmg_multiplier *= IsDungeonBoss() ? 40.0f : 10.0f;
            break;
        case DIFFICULTY_HEROIC_RAID:
            m_dmg_multiplier *= IsDungeonBoss() ? 50.0f : 11.0f;
            break;
        case DIFFICULTY_MYTHIC_RAID:
            m_dmg_multiplier *= IsDungeonBoss() ? 60.0f : 12.0f;
            break;
        default:
            break;
    }

    //Map Difficulty Mod
    if (float mapMod = sObjectMgr->GetMapDifficultyStat(GetMapId(), GetMap()->GetDifficultyID()))
        m_dmg_multiplier *= mapMod;
}

/*#######################################
########                         ########
########    PETS STAT SYSTEM     ########
########                         ########
#######################################*/

bool Guardian::UpdateStats(Stats stat)
{
    if (stat >= MAX_STATS)
        return false;

    float value = GetTotalStatValue(stat);
    ApplyStatBuffMod(stat, m_statFromOwner[stat], false);
    float ownersBonus = 0.0f;

    Unit* owner = GetOwner();
    // Handle Death Knight Glyphs and Talents
    float mod = 0.75f;

    switch (stat)
    {
        case STAT_STAMINA:
        {
            if (IsPetGhoul() || IsPetGargoyle())
                mod = 0.45f;
            else if (owner->getClass() == CLASS_WARLOCK && isPet())
                mod = 0.75f;
            else if (owner->getClass() == CLASS_MAGE && isPet())
                mod = 0.75f;
            else
            {
                mod = 0.45f;

                if (isPet())
                {
                    switch (ToPet()->GetSpecializationId())
                    {
                        case SPEC_PET_ADAPTATION_FEROCITY:
                        case SPEC_PET_FEROCITY: mod = 0.67f; break;
                        case SPEC_PET_ADAPTATION_TENACITY:
                        case SPEC_PET_TENACITY: mod = 0.78f; break;
                        case SPEC_PET_ADAPTATION_CUNNING:
                        case SPEC_PET_CUNNING: mod = 0.725f; break;
                        default:
                            break;
                    }
                }
            }

            ownersBonus = float(owner->GetStat(stat)) * mod;
            ownersBonus *= GetModifierValue(UNIT_MOD_STAT_STAMINA, TOTAL_PCT);
            value += ownersBonus;
            break;
        }
        case STAT_STRENGTH:
        {
            mod = 0.7f;

            ownersBonus = owner->GetStat(stat) * mod;
            value += ownersBonus;
            break;
        }
        case STAT_INTELLECT:
        {
            mod = 0.3f;

            ownersBonus = owner->GetStat(stat) * mod;
            value += ownersBonus;
            break;
        }
        default:
            break;
    }

    SetStat(stat, int32(value));
    m_statFromOwner[stat] = ownersBonus;
    ApplyStatBuffMod(stat, m_statFromOwner[stat], true);

    switch (stat)
    {
        case STAT_STRENGTH:
            UpdateAttackPowerAndDamage();
            break;
        case STAT_AGILITY:
            UpdateArmor();
            break;
        case STAT_STAMINA:
        {
            uint8 healthPct = RoundingFloatValue(GetHealthPct());
            UpdateMaxHealth();
            SetHealth(CountPctFromMaxHealth(healthPct));
            break;
        }
        case STAT_INTELLECT:
        {
            if (getPowerType() == POWER_MANA)
                UpdateMaxPower(POWER_MANA);
            if (owner->getClass() == CLASS_MAGE)
                UpdateAttackPowerAndDamage();
            break;
        }
        default:
            break;
    }

    return true;
}

bool Guardian::UpdateAllStats()
{
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        UpdateStats(Stats(i));

    UpdateMaxPower(getPowerType());
    UpdateMaxPower(POWER_ALTERNATE);

    for (uint8 i = SPELL_SCHOOL_NORMAL; i < MAX_SPELL_SCHOOL; ++i)
        UpdateResistances(i);

    return true;
}

void Guardian::UpdateResistances(uint32 school)
{
    if (school > SPELL_SCHOOL_NORMAL)
    {
        float value;
        if (!isHunterPet())
        {
            value = GetTotalAuraModValue(UnitMods(UNIT_MOD_RESISTANCE_START + school));

            // hunter and warlock pets gain 40% of owner's resistance
            if (isPet())
                value += float(CalculatePct(m_owner->GetResistance(SpellSchools(school)), 40));
        }
        else
            value = m_owner->GetResistance(SpellSchools(school));

        SetResistance(SpellSchools(school), int32(value));
    }
    else
        UpdateArmor();
}

void Guardian::UpdateArmor()
{
    float value = 0.0f;
    UnitMods unitMod = UNIT_MOD_ARMOR;
    uint32 creature_ID = isHunterPet() ? 1 : GetEntry();

    if (PetStats const* pStats = sObjectMgr->GetPetStats(creature_ID))
        value = m_owner->GetModifierValue(unitMod, BASE_VALUE) * pStats->armor;
    else
        value = m_owner->GetModifierValue(unitMod, BASE_VALUE);

    //TC_LOG_DEBUG(LOG_FILTER_PETS, "Guardian::UpdateArmor value %f creature_ID %i", value, creature_ID);

    value *= GetModifierValue(unitMod, BASE_PCT);
    value *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_RESISTANCE_PCT, 1);
    value *= m_owner->GetTotalAuraMultiplierByMiscValues(SPELL_AURA_MOD_PET_STATS_MODIFIER, int32(PETSPELLMOD_ARMOR), GetEntry());

    SetArmor(int32(value));
}

void Guardian::UpdateMaxHealth()
{
    float multiplicator = 0.3f;
    float value;

    if (Unit* owner = GetOwner())
    {
        uint32 creature_ID = isHunterPet() ? 1 : GetEntry();
        if (PetStats const* pStats = sObjectMgr->GetPetStats(creature_ID))
        {
            if (pStats->hp)
                multiplicator = pStats->hp;
        }
        else
        {
            SetMaxHealth(GetCreateHealth());
            return;
        }

        multiplicator *= owner->GetTotalAuraMultiplierByMiscValues(SPELL_AURA_MOD_PET_STATS_MODIFIER, int32(PETSPELLMOD_MAX_HP), GetEntry());
        value = owner->GetMaxHealth() * multiplicator;

        UnitMods unitMod = UNIT_MOD_HEALTH;
        //value += GetModifierValue(unitMod, BASE_VALUE);
        //value *= GetModifierValue(unitMod, BASE_PCT);
        //value += GetModifierValue(unitMod, TOTAL_VALUE);
        value *= GetModifierValue(unitMod, TOTAL_PCT);

        //TC_LOG_DEBUG(LOG_FILTER_PETS, "Guardian::UpdateMaxHealth multiplicator %f creature_ID %i hp %f", multiplicator, creature_ID, value);
    }
    else
    {
        UnitMods unitMod = UNIT_MOD_HEALTH;
        float stamina = GetStat(STAT_STAMINA) - GetCreateStat(STAT_STAMINA);
        multiplicator = 8.0f;
        value = GetModifierValue(unitMod, BASE_VALUE) + GetCreateHealth();
        value *= GetModifierValue(unitMod, BASE_PCT);
        value += GetModifierValue(unitMod, TOTAL_VALUE) + stamina * multiplicator;
        value *= GetModifierValue(unitMod, TOTAL_PCT);
    }

    SetMaxHealth(static_cast<uint32>(value));
}

void Guardian::UpdateMaxPower(Powers power)
{
    int32 value = GetCreatePowers(power);

    uint32 creature_ID = isHunterPet() ? 1 : GetEntry();
    if (PetStats const* pStats = sObjectMgr->GetPetStats(creature_ID))
    {
        if (!pStats->energy && pStats->energy_type == 1)
            value = pStats->energy;
        else if (pStats->energy && pStats->energy_type == power)
            value = pStats->energy;
    }

    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY, power);
    value += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MAX_POWER, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_MAX_MANA, power);
    value *= GetTotalAuraMultiplierByMiscValue(SPELL_AURA_MOD_ADD_ENERGY_PERCENT, power);

    // TC_LOG_DEBUG(LOG_FILTER_PETS, "Guardian::UpdateMaxPower value %f creature_ID %i", value, creature_ID);
    SetMaxPower(power, value);
}

void Guardian::UpdateAttackPowerAndDamage(bool /*ranged*/)
{
    float AP = 0.0f; // Attack Power
    uint32 SPD = 0.0f; // Pet Spell Power
    UnitMods unitMod = UNIT_MOD_ATTACK_POWER;

    Unit* owner = GetOwner();
    if (owner && owner->IsPlayer())
    {
        uint32 creature_ID = isHunterPet() ? 1 : GetEntry();

        if (PetStats const* pStats = sObjectMgr->GetPetStats(creature_ID))
        {
            int32 temp_ap = owner->GetTotalAttackPowerValue(WeaponAttackType(pStats->ap_type));
            int32 school_spd = 0;
            int32 school_temp_spd = 0;
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if (pStats->school_mask & 1 << i)
                {
                    school_temp_spd = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i) + owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i);
                    if (school_temp_spd > school_spd)
                        school_spd = school_temp_spd;
                }
            }

            if (pStats->ap > 0)
                AP = int32(temp_ap * pStats->ap);
            else if (pStats->ap < 0)
            {
                if (school_spd)
                    AP = -int32(school_spd * pStats->ap);
            }
            if (pStats->spd < 0)
                SPD = -int32(temp_ap * pStats->spd);
            else if (pStats->spd > 0)
            {
                if (school_spd)
                    SPD = int32(school_spd * pStats->spd);
            }
            if (pStats->maxspdorap > 0)
            {
                AP = temp_ap > school_spd ? int32(temp_ap * fabs(pStats->ap)) : int32(school_spd * fabs(pStats->spd));
                SPD = temp_ap > school_spd ? int32(temp_ap * fabs(pStats->ap)) : int32(school_spd * fabs(pStats->spd));
            }
            else if (pStats->maxspdorap < 0)
            {
                AP = int32((temp_ap + school_spd) / 2 * fabs(pStats->ap));
                SPD = int32((temp_ap + school_spd) / 2 * fabs(pStats->spd));
            }
        }
        else
        {
            int32 school_temp_spd = 0;
            int32 school_spd = 0;
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                school_temp_spd = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i) + owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i);
                if (school_temp_spd > school_spd)
                    school_spd = school_temp_spd;
            }
            if (school_spd)
                SPD = school_spd;
        }

        switch (GetEntry())
        {
            case ENTRY_RUNE_WEAPON:
            {
                if (AuraEffect const* aurEff = owner->GetAuraEffect(63330, 1))
                    AP += int32(AP * aurEff->GetAmount() / 100);
                break;
            }
            default:
                break;
        }

        AddPct(AP, GetMaxPositiveAuraModifier(SPELL_AURA_MOD_ATTACK_POWER_PCT));

        //UNIT_FIELD_(RANGED)_ATTACK_POWER field
        SetInt32Value(UNIT_FIELD_ATTACK_POWER, AP);
        SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, AP);

        //PLAYER_FIELD_PET_SPELL_POWER field
        SetBonusDamage(SPD);
    }
    else
    {
        SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, 0.0f);

        //in BASE_VALUE of UNIT_MOD_ATTACK_POWER for creatures we store data of meleeattackpower field in DB
        float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
        float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

        //UNIT_FIELD_(RANGED)_ATTACK_POWER field
        SetInt32Value(UNIT_FIELD_ATTACK_POWER, static_cast<int32>(base_attPower));
        //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
        SetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, attPowerMultiplier);
    }

    //automatically update weapon damage after attack power modification
    UpdateDamagePhysical(BASE_ATTACK);
    UpdateDamagePhysical(OFF_ATTACK);
}

void Guardian::UpdateDamagePhysical(WeaponAttackType attType)
{
    float APCoefficient = 3.5f;
    UnitMods unitMod = UNIT_MOD_DAMAGE_MAINHAND;

    float att_speed = BASE_ATTACK_TIME / 1000.0f;

    m_base_value = GetModifierValue(unitMod, BASE_VALUE) + GetTotalAttackPowerValue(attType) * att_speed / APCoefficient;
    m_base_pct = GetModifierValue(unitMod, BASE_PCT);
    m_total_value = GetModifierValue(unitMod, TOTAL_VALUE);
    m_total_pct = GetModifierValue(unitMod, TOTAL_PCT);

    m_dmg_multiplier = 1.0f;
}

void Guardian::SetBonusDamage(int32 SPD)
{
    m_bonusSpellDamage = SPD;
    if (GetOwner()->IsPlayer())
        GetOwner()->SetUInt32Value(PLAYER_FIELD_PET_SPELL_POWER, SPD);
}

void Player::UpdateMasteryAuras()
{
    if (!HasAuraType(SPELL_AURA_MASTERY))
    {
        SetFloatValue(PLAYER_FIELD_MASTERY, 0.0f);
        return;
    }

    SetFloatValue(PLAYER_FIELD_MASTERY, GetTotalAuraModifier(SPELL_AURA_MASTERY) + GetRatingBonusValue(CR_MASTERY));

    // TODO: rewrite 115556 Master Demonologist
    ChrSpecializationEntry const* specialization = sChrSpecializationStore.LookupEntry(GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID));
    if (!specialization)
        return;

    for (uint8 i = 0; i < MAX_MASTERY_SPELLS; ++i)
    {
        if (!specialization->MasterySpellID[i])
            continue;

        Aura* aura = GetAura(specialization->MasterySpellID[i]);
        if (!aura)
            continue;

        // update aura modifiers
        for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            AuraEffect* auraEff = aura->GetEffect(j);
            if (!auraEff)
                continue;

            auraEff->SetCanBeRecalculated(true);
            auraEff->RecalculateAmount(this);
        }
    }
}

void Player::UpdateVersality(CombatRating /*cr*/)
{
    SetFloatValue(PLAYER_FIELD_VERSATILITY, GetRatingBonusValue(CR_VERSATILITY_DAMAGE_DONE));
    SetFloatValue(PLAYER_FIELD_VERSATILITY_BONUS, GetTotalAuraModifier(SPELL_AURA_MOD_VERSALITY_PCT));

    /// Update damage after applying versatility rating
    if (getClass() == CLASS_HUNTER)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }
}

void Player::UpdateCRSpeed()
{
    float val = GetRatingBonusValue(CR_SPEED);
    if (val > 20.0f) // Is Cap
        val = 20.0f;

    SetFloatValue(PLAYER_FIELD_SPEED, std::max(0.0f, val));

    for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i)
        UpdateSpeed(UnitMoveType(i), true);
}

void Player::UpdateLifesteal()
{
    float val = 0.0f;

    val += GetRatingBonusValue(CR_LIFESTEAL);
    if (val > 20.0f) // Is Cap
        val = 20.0f;

    val += GetTotalAuraModifier(SPELL_AURA_MOD_LIFE_STEAL_PCT, true);

    SetFloatValue(PLAYER_FIELD_LIFESTEAL, std::max(0.0f, val));

    //TC_LOG_DEBUG(LOG_FILTER_SPELLS_AURAS, "Player: UpdateLifesteal val %f", val);
}

void Player::UpdateAvoidance()
{
    float val = GetRatingBonusValue(CR_AVOIDANCE);
    if (val > 20.0f) // Is Cap
        val = 20.0f;

    SetFloatValue(PLAYER_FIELD_AVOIDANCE, std::max(0.0f, val));
}

uint32 Player::GetLootSpecID() const
{
    if (uint32 specID = GetFieldLootSpecID())
        return specID;
    if (uint32 specID = GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID))
        return specID;
    return GetDefaultLootSpecID();
}

uint32 Player::GetDefaultLootSpecID() const
{
    for (ChrSpecializationEntry const* specialization : sChrSpecializationStore)
        if (specialization->ClassID == getClass() && specialization->OrderIndex == 0)
            return specialization->ID;
    return 0;
}

void TempSummon::UpdateAttackPowerAndDamage(bool ranged)
{
    UnitMods unitMod = ranged ? UNIT_MOD_ATTACK_POWER_RANGED : UNIT_MOD_ATTACK_POWER;

    uint16 index = UNIT_FIELD_ATTACK_POWER;
    uint16 index_mult = UNIT_FIELD_ATTACK_POWER_MULTIPLIER;

    if (ranged)
    {
        index = UNIT_FIELD_RANGED_ATTACK_POWER;
        index_mult = UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER;
    }

    bool needCalculate = true;
    Unit* owner = GetAnyOwner();
    if (owner && owner->IsPlayer())
    {
        if (PetStats const* pStats = sObjectMgr->GetPetStats(GetEntry()))
        {
            float AP = 0.0f; // Attack Power
            uint32 SPD = 0.0f; // Pet Spell Power

            int32 temp_ap = owner->GetTotalAttackPowerValue(WeaponAttackType(pStats->ap_type));
            int32 school_spd = 0;
            int32 school_temp_spd = 0;
            for (int i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
            {
                if (pStats->school_mask & 1 << i)
                {
                    school_temp_spd = owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + i) + owner->GetUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_NEG + i);
                    if (school_temp_spd > school_spd)
                        school_spd = school_temp_spd;
                }
            }

            if (pStats->ap > 0)
                AP = int32(temp_ap * pStats->ap);
            else if (pStats->ap < 0)
            {
                if (school_spd)
                    AP = -int32(school_spd * pStats->ap);
            }
            if (pStats->spd < 0)
                SPD = -int32(temp_ap * pStats->spd);
            else if (pStats->spd > 0)
            {
                if (school_spd)
                    SPD = int32(school_spd * pStats->spd);
            }
            if (pStats->maxspdorap > 0)
            {
                AP = temp_ap > school_spd ? int32(temp_ap * fabs(pStats->ap)) : int32(school_spd * fabs(pStats->spd));
                SPD = temp_ap > school_spd ? int32(temp_ap * fabs(pStats->ap)) : int32(school_spd * fabs(pStats->spd));
            }
            else if (pStats->maxspdorap < 0)
            {
                AP = int32((temp_ap + school_spd) / 2 * fabs(pStats->ap));
                SPD = int32((temp_ap + school_spd) / 2 * fabs(pStats->spd));
            }

            AddPct(AP, GetMaxPositiveAuraModifier(SPELL_AURA_MOD_ATTACK_POWER_PCT));

            //UNIT_FIELD_(RANGED)_ATTACK_POWER field
            SetInt32Value(UNIT_FIELD_ATTACK_POWER, AP);
            SetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER, AP);

            //PLAYER_FIELD_PET_SPELL_POWER field
            m_bonusSpellDamage = SPD;
            needCalculate = false;
        }
    }

    if (needCalculate)
    {
        SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, 0.0f);

        float base_attPower = GetModifierValue(unitMod, BASE_VALUE) * GetModifierValue(unitMod, BASE_PCT);
        float attPowerMultiplier = GetModifierValue(unitMod, TOTAL_PCT) - 1.0f;

        SetInt32Value(index, static_cast<uint32>(base_attPower));            //UNIT_FIELD_(RANGED)_ATTACK_POWER field
        SetFloatValue(index_mult, attPowerMultiplier);          //UNIT_FIELD_(RANGED)_ATTACK_POWER_MULTIPLIER field
    }

    //automatically update weapon damage after attack power modification
    if (ranged)
        UpdateDamagePhysical(RANGED_ATTACK);
    else
    {
        UpdateDamagePhysical(BASE_ATTACK);
        UpdateDamagePhysical(OFF_ATTACK);
    }
}

void Unit::ApplyDiminishingStat(float& pct, uint8 whichClass)
{
    float VerticalStretch = whichClass == CLASS_DEMON_HUNTER ? 0.0315f : 0.018f;
    float HorizontalShift = whichClass == CLASS_DRUID ? 1/0.97f : 1/0.94f;

    pct = pct / (pct * VerticalStretch + HorizontalShift);
}
