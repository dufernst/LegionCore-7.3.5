
#include "GarrisonFollower.h"
#include "Garrison.h"
#include "GarrisonMgr.h"

using namespace GarrisonConst;

Follower const* Garrison::GetFollower(uint64 dbId) const
{
    for (uint8 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        auto itr = _followers[i].find(dbId);
        if (itr != _followers[i].end())
            return &itr->second;
    }

    return nullptr;
}

Follower* Garrison::GetFollower(uint64 dbId)
{
    for (uint8 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        auto itr = _followers[i].find(dbId);
        if (itr != _followers[i].end())
            return &itr->second;
    }

    return nullptr;
}

Follower* Garrison::GetFollowerByID(uint32 entry)
{
    for (uint8 i = GARRISON_TYPE_GARRISON; i < GARRISON_TYPE_MAX; ++i)
    {
        for (auto& v : _followers[i])
            if (v.second.PacketInfo.GarrFollowerID == entry)
                return &v.second;
    }

    return nullptr;
}

uint32 Follower::GetItemLevel() const
{
    return (PacketInfo.ItemLevelWeapon + PacketInfo.ItemLevelArmor) / 2;
}

void Follower::ModAssistant(SpellInfo const* spellInfo, Player* caster)
{
    enum Types : uint32
    {
        SetWeaponLevel = 0,
        SetArmorLevel = 1,
        IncreaseWeaponLevel = 2,
        IncreaseArmorLevel = 3
    };

    auto itemLevelUpgradeDataEntry = sGarrItemLevelUpgradeDataStore[spellInfo->Effects[0]->MiscValue];
    if (!itemLevelUpgradeDataEntry)
        return;

    auto followerTypeData = sGarrFollowerTypeStore[TypeID];
    if (!followerTypeData)
        return;

    auto maxAllowedItemLevel = itemLevelUpgradeDataEntry->MaxItemLevel;
    if (maxAllowedItemLevel > followerTypeData->MaxItemLevel)
        return;

    if (!maxAllowedItemLevel && TypeID == FollowerType::Garrison)
        maxAllowedItemLevel = followerTypeData->MaxItemLevel;

    auto updateInfo = false;
    uint32 value = spellInfo->Effects[0]->BasePoints;

    switch (itemLevelUpgradeDataEntry->Operation)
    {
    case SetWeaponLevel:
        updateInfo = true;
        PacketInfo.ItemLevelWeapon = value;
        break;
    case SetArmorLevel:
        updateInfo = true;
        PacketInfo.ItemLevelArmor = value;
        break;
    case IncreaseWeaponLevel:
        updateInfo = true;
        PacketInfo.ItemLevelWeapon += value;
        break;
    case IncreaseArmorLevel:
        updateInfo = true;
        PacketInfo.ItemLevelArmor += value;
        break;
    default:
        break;
    }

    if (!updateInfo)
        return;

    PacketInfo.ItemLevelWeapon = std::min(PacketInfo.ItemLevelWeapon, static_cast<uint32>(maxAllowedItemLevel));
    PacketInfo.ItemLevelArmor = std::min(PacketInfo.ItemLevelArmor, static_cast<uint32>(maxAllowedItemLevel));

    DbState = DB_STATE_CHANGED;

    WorldPackets::Garrison::GarrisonFollowerChangedItemLevel update;
    update.Follower = PacketInfo;
    caster->SendDirectMessage(update.Write());
}

void Follower::IncreaseFollowerExperience(SpellInfo const* spellInfo, Player* caster)
{
    WorldPackets::Garrison::GarrisonFollowerChangedXP update;
    update.Follower = PacketInfo;

    if (TypeID == spellInfo->Effects[0]->MiscValueB)
    {
        GiveXP(spellInfo->Effects[0]->BasePoints);
        update.TotalXp = spellInfo->Effects[0]->BasePoints;
    }
    else
        update.Result = GARRISON_ERROR_FOLLOWER_CANNOT_GAIN_XP;

    update.Follower2 = PacketInfo;
    caster->SendDirectMessage(update.Write());
}

uint8 Follower::RollQuality(uint32 baseQuality)
{
    uint8 quality = FOLLOWER_QUALITY_UNCOMMON;
    // 35% - rare, 7% - epic
    uint32 r = urand(0, 100);
    if (r >= 65 && r < 90)
        quality = FOLLOWER_QUALITY_RARE;
    else if (r >= 93 && r <= 100)
        quality = FOLLOWER_QUALITY_EPIC;

    return quality > baseQuality ? quality : baseQuality;
}

uint32 Follower::GiveXP(uint32 xp)
{
    auto requiredLevelUpXP = [this]() -> uint32
    {
        if (PacketInfo.FollowerLevel < FollowerType::MaxLevel[TypeID])
        {
            for (auto const& entry : sGarrFollowerLevelXPStore)
                if (entry->FollowerLevel == PacketInfo.FollowerLevel && entry->GarrFollowerTypeID == TypeID)
                    return entry->XpToNextLevel;
        }
        else
        {
            for (auto const& entry : sGarrFollowerQualityStore)
                if (entry->Quality == PacketInfo.Quality)
                    if (entry->GarrFollowerTypeID == TypeID)
                        return entry->XpToNextQuality;
        }

        return 0;
    }();

    if (!requiredLevelUpXP)
        return 0;

    if (PacketInfo.Xp + xp < requiredLevelUpXP)
    {
        DbState = DB_STATE_CHANGED;
        PacketInfo.Xp += xp;
        return xp;
    }

    auto XPToMax = requiredLevelUpXP - PacketInfo.Xp;
    PacketInfo.Xp = 0;

    if (PacketInfo.FollowerLevel < FollowerType::MaxLevel[TypeID])
    {
        ++PacketInfo.FollowerLevel;
        DbState = DB_STATE_CHANGED;
    }
    else
    {
        ++PacketInfo.Quality;
        PacketInfo.AbilityID = sGarrisonMgr.RollFollowerAbilities(sGarrFollowerStore[PacketInfo.GarrFollowerID], PacketInfo.Quality, Faction, false);
        for (auto const& itr : ItemTraits)
            PacketInfo.AbilityID[itr.first] = itr.second;

        if (PacketInfo.Quality == FOLLOWER_QUALITY_TITLE)
            PacketInfo.Vitality = 1;

        DbState = DB_STATE_CHANGED;
    }

    return xp + GiveXP(xp - XPToMax);
}

void Follower::GiveLevel(uint32 level)
{
    PacketInfo.FollowerLevel = level;
}

void Follower::SetQuality(uint32 quality)
{
    PacketInfo.Quality = quality;
}
