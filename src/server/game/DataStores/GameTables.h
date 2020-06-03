/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
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

#ifndef GameTables_h__
#define GameTables_h__

#include "SharedDefines.h"
#include "Common.h"

struct GtArmorMitigationByLvlEntry
{
    float Level = 0.0f;
    float Mitigation = 0.0f;
};

struct GtBarberShopCostBaseEntry
{
    float Level = 0.0f;
    float Cost = 0.0f;
};

struct GtBaseMPEntry
{
    float Level = 0.0f;
    float Rogue = 0.0f;
    float Druid = 0.0f;
    float Hunter = 0.0f;
    float Mage = 0.0f;
    float Paladin = 0.0f;
    float Priest = 0.0f;
    float Shaman = 0.0f;
    float Warlock = 0.0f;
    float Warrior = 0.0f;
    float DeathKnight = 0.0f;
    float Monk = 0.0f;
    float DemonHunter = 0.0f;
};

struct GtCombatRatingsEntry
{
    float Level = 0.0f;
    float Amplify = 0.0f;
    float DefenseSkill = 0.0f;
    float Dodge = 0.0f;
    float Parry = 0.0f;
    float Block = 0.0f;
    float HitMelee = 0.0f;
    float HitRanged = 0.0f;
    float HitSpell = 0.0f;
    float CritMelee = 0.0f;
    float CritRanged = 0.0f;
    float CritSpell = 0.0f;
    float MultiStrike = 0.0f;
    float Readiness = 0.0f;
    float Speed = 0.0f;
    float ResilienceCritTaken = 0.0f;
    float ResiliencePlayerDamage = 0.0f;
    float Lifesteal = 0.0f;
    float HasteMelee = 0.0f;
    float HasteRanged = 0.0f;
    float HasteSpell = 0.0f;
    float Avoidance = 0.0f;
    float Sturdiness = 0.0f;
    float Unused7 = 0.0f;
    float Expertise = 0.0f;
    float ArmorPenetration = 0.0f;
    float Mastery = 0.0f;
    float PvPPower = 0.0f;
    float Cleave = 0.0f;
    float VersatilityDamageDone = 0.0f;
    float VersatilityHealingDone = 0.0f;
    float VersatilityDamageTaken = 0.0f;
    float Unused12 = 0.0f;
    // float End = 0.0f;
};

struct GtHpPerStaEntry
{
    float Level = 0.0f;
    float Health = 0.0f;
};

struct GtCombatRatingsMultByILvl
{
    float Level = 0.0f;
    float ArmorMultiplier = 0.0f;
    float WeaponMultiplier = 0.0f;
    float TrinketMultiplier = 0.0f;
    float JewelryMultiplier = 0.0f;
};

struct GtItemSocketCostPerLevelEntry
{
    float Level = 0.0f;
    float SocketCost = 0.0f;
};

struct GtNpcDamageByClassEntry
{
    float Level = 0.0f;
    float Rogue = 0.0f;
    float Druid = 0.0f;
    float Hunter = 0.0f;
    float Mage = 0.0f;
    float Paladin = 0.0f;
    float Priest = 0.0f;
    float Shaman = 0.0f;
    float Warlock = 0.0f;
    float Warrior = 0.0f;
    float DeathKnight = 0.0f;
    float Monk = 0.0f;
    float DemonHunter = 0.0f;
};

struct GtNpcManaCostScalerEntry
{
    float Level = 0.0f;
    float Scaler = 0.0f;
};

struct GtNpcTotalHpEntry
{
    float Level = 0.0f;
    float Rogue = 0.0f;
    float Druid = 0.0f;
    float Hunter = 0.0f;
    float Mage = 0.0f;
    float Paladin = 0.0f;
    float Priest = 0.0f;
    float Shaman = 0.0f;
    float Warlock = 0.0f;
    float Warrior = 0.0f;
    float DeathKnight = 0.0f;
    float Monk = 0.0f;
    float DemonHunter = 0.0f;
};

struct GtSpellScalingEntry
{
    float Level = 0.0f;
    float Rogue = 0.0f;
    float Druid = 0.0f;
    float Hunter = 0.0f;
    float Mage = 0.0f;
    float Paladin = 0.0f;
    float Priest = 0.0f;
    float Shaman = 0.0f;
    float Warlock = 0.0f;
    float Warrior = 0.0f;
    float DeathKnight = 0.0f;
    float Monk = 0.0f;
    float DemonHunter = 0.0f;
    float Item = 0.0f;
    float Consumable = 0.0f;
    float Gem1 = 0.0f;
    float Gem2 = 0.0f;
    float Gem3 = 0.0f;
    float Health = 0.0f;
};

struct GtXpEntry
{
    float Level = 0.0f;
    float Total = 0.0f;
    float PerKill = 0.0f;
    float Junk = 0.0f;
    float Stats = 0.0f;
    float Divisor = 0.0f;
};

struct GtHonorLevelEntry
{
    float Level = 0.0f;
    float Prestige[33] = { };
};

struct GtArtifactLevelXPEntry
{
    float Level = 0.0f;
    float XP = 0.0f;
    float XP2 = 0.0f;
};

struct GtArtifactKnowledgeMultiplierEntry
{
    float Level = 0.0f;
    float Multiplier = 0.0f;
};

struct GtBattlePetXPEntry
{
    float Level = 0.0f;
    float Wins = 0.0f;
    float Xp = 0.0f;
};

struct GtBattlePetTypeDamageModEntry
{
    float PetType = 0.0f;
    float Humanoid = 0.0f;
    float Dragonkin = 0.0f;
    float Flying = 0.0f;
    float Undead = 0.0f;
    float Critter = 0.0f;
    float Magic = 0.0f;
    float Elemental = 0.0f;
    float Beast = 0.0f;
    float Aquatic = 0.0f;
    float Mechanical = 0.0f;
};

struct GtChallengeModeDamageEntry
{
    float ChallengeLevel = 0.0f;
    float Scalar = 0.0f;
};

struct GtChallengeModeHealthEntry
{
    float ChallengeLevel = 0.0f;
    float Scalar = 0.0f;
};

template<class T>
class GameTable
{
public:
    T const* GetRow(uint32 row) const
    {
        if (row >= _data.size())
            return nullptr;

        return &_data[row];
    }

    uint32 GetTableRowCount() const { return uint32(_data.size()); }

    void SetData(std::vector<T> data) { _data = std::move(data); }
    std::vector<T>* GetData() { return &_data; }

private:
    std::vector<T> _data;
};

extern GameTable<GtArmorMitigationByLvlEntry>       sArmorMitigationByLvlGameTable;
extern GameTable<GtBarberShopCostBaseEntry>         sBarberShopCostBaseGameTable;
extern GameTable<GtBaseMPEntry>                     sBaseMPGameTable;
extern GameTable<GtCombatRatingsEntry>              sCombatRatingsGameTable;
extern GameTable<GtCombatRatingsMultByILvl>         sCombatRatingsMultByILvlGameTable;
extern GameTable<GtHpPerStaEntry>                   sHpPerStaGameTable;
extern GameTable<GtItemSocketCostPerLevelEntry>     sItemSocketCostPerLevelGameTable;
extern GameTable<GtNpcDamageByClassEntry>           sNpcDamageByClassGameTable[MAX_EXPANSIONS];
extern GameTable<GtNpcManaCostScalerEntry>          sNpcManaCostScalerGameTable;
extern GameTable<GtNpcTotalHpEntry>                 sNpcTotalHpGameTable[MAX_EXPANSIONS];
extern GameTable<GtSpellScalingEntry>               sSpellScalingGameTable;
extern GameTable<GtXpEntry>                         sXpGameTable;
extern GameTable<GtHonorLevelEntry>                 sHonorLevelGameTable;
extern GameTable<GtArtifactLevelXPEntry>            sArtifactLevelXPGameTable;
extern GameTable<GtArtifactKnowledgeMultiplierEntry>sArtifactKnowledgeMultiplierGameTable;
extern GameTable<GtBattlePetXPEntry>                sBattlePetXPTable;
extern GameTable<GtBattlePetTypeDamageModEntry>     sBattlePetTypeDamageModTable;
extern GameTable<GtChallengeModeDamageEntry>        sChallengeModeDamageTable;
extern GameTable<GtChallengeModeHealthEntry>        sChallengeModeHealthTable;


void LoadGameTables(std::string const& dataPath);

template<class T>
float GetGameTableColumnForClass(T const* row, int32 class_)
{
    if (!row)
        return 0.0f;

    switch (class_)
    {
        case CLASS_WARRIOR:
            return row->Warrior;
        case CLASS_PALADIN:
            return row->Paladin;
        case CLASS_HUNTER:
            return row->Hunter;
        case CLASS_ROGUE:
            return row->Rogue;
        case CLASS_PRIEST:
            return row->Priest;
        case CLASS_DEATH_KNIGHT:
            return row->DeathKnight;
        case CLASS_SHAMAN:
            return row->Shaman;
        case CLASS_MAGE:
            return row->Mage;
        case CLASS_WARLOCK:
            return row->Warlock;
        case CLASS_MONK:
            return row->Monk;
        case CLASS_DRUID:
            return row->Druid;
        case CLASS_DEMON_HUNTER:
            return row->DemonHunter;
        default:
            break;
    }

    return 0.0f;
}

inline float GetSpellScalingColumnForClass(GtSpellScalingEntry const* row, int32 class_)
{
    switch (class_)
    {
        case CLASS_WARRIOR:
            return row->Warrior;
        case CLASS_PALADIN:
            return row->Paladin;
        case CLASS_HUNTER:
            return row->Hunter;
        case CLASS_ROGUE:
            return row->Rogue;
        case CLASS_PRIEST:
            return row->Priest;
        case CLASS_DEATH_KNIGHT:
            return row->DeathKnight;
        case CLASS_SHAMAN:
            return row->Shaman;
        case CLASS_MAGE:
            return row->Mage;
        case CLASS_WARLOCK:
            return row->Warlock;
        case CLASS_MONK:
            return row->Monk;
        case CLASS_DRUID:
            return row->Druid;
        case CLASS_DEMON_HUNTER:
            return row->DemonHunter;
        case -1:
            return row->Item;
        case -2:
            return row->Consumable;
        case -3:
            return row->Gem1;
        case -4:
            return row->Gem2;
        case -5:
            return row->Gem3;
        case -6:
            return row->Health;
        default:
            break;
    }

    return 0.0f;
}

#endif // GameTables_h__
