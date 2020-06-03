/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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

#include "Containers.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Garrison.h"
#include "GarrisonMgr.h"
#include "ObjectDefines.h"
#include "ObjectMgr.h"
#include "World.h"
#include "QuestData.h"

uint32 GarrisonMgr::getFirstMap(uint32 map)
{
    switch (map)
    {
        case 1152:
        case 1330:
        case 1153:
            return 1152;
        case 1158:
        case 1331:
        case 1159:
            return 1158;
    }
    return 0;
}

void GarrisonMgr::Initialize()
{
    for (GarrSiteLevelPlotInstEntry const* plotInstance : sGarrSiteLevelPlotInstStore)
        _garrisonPlotInstBySiteLevel[plotInstance->GarrSiteLevelID].push_back(plotInstance);

    for (GameObjectsEntry const* gameObject : sGameObjectsStore)
        if (gameObject->TypeID == GAMEOBJECT_TYPE_GARRISON_PLOT)
            _garrisonPlots[gameObject->OwnerID][gameObject->PropValue[0]] = gameObject;

    for (GarrPlotBuildingEntry const* plotBuilding : sGarrPlotBuildingStore)
        _garrisonBuildingsByPlot[plotBuilding->GarrPlotID].insert(plotBuilding->GarrBuildingID);

    for (GarrBuildingPlotInstEntry const* buildingPlotInst : sGarrBuildingPlotInstStore)
        _garrisonBuildingPlotInstances[MAKE_PAIR64(buildingPlotInst->GarrBuildingID, buildingPlotInst->GarrSiteLevelPlotInstID)] = buildingPlotInst->ID;

    for (GarrBuildingEntry const* building : sGarrBuildingStore)
        _garrisonBuildingsByType[building->BuildingType].push_back(building);

    for (GarrFollowerXAbilityEntry const* followerAbility : sGarrFollowerXAbilityStore)
    {
        if (GarrAbilityEntry const* ability = sGarrAbilityStore.LookupEntry(followerAbility->GarrAbilityID))
        {
            if (!(ability->Flags & GARRISON_ABILITY_CANNOT_ROLL) && ability->Flags & GARRISON_ABILITY_FLAG_TRAIT)
                _garrisonFollowerRandomTraits.insert(ability);

            if (followerAbility->FactionIndex < 2)
            {
                if (ability->Flags & GARRISON_ABILITY_FLAG_TRAIT)
                    _garrisonFollowerAbilities[followerAbility->FactionIndex][followerAbility->GarrFollowerID].Traits.insert(ability);
                else
                    _garrisonFollowerAbilities[followerAbility->FactionIndex][followerAbility->GarrFollowerID].Counters.insert(ability);
            }
        }
    }

    InitializeDbIdSequences();
    LoadMissionsRewards();
    LoadMissionsOwermaxRewards();
    LoadMissionsQuestLink();
    LoadPlotFinalizeGOInfo();
    LoadFollowerClassSpecAbilities();
    LoadBuildingSpawnNPC();
    LoadBuildingSpawnGo();
    LoadMissionLine();
    LoadShipment();
    LoadTradeSkill();
}

GarrSiteLevelEntry const* GarrisonMgr::GetGarrSiteLevelEntry(uint32 garrSiteId, uint32 level) const
{
    for (GarrSiteLevelEntry const* garrSiteLevel : sGarrSiteLevelStore)
        if (garrSiteLevel->GarrSiteID == garrSiteId && garrSiteLevel->GarrLevel == level)
            return garrSiteLevel;

    return nullptr;
}

std::vector<GarrSiteLevelPlotInstEntry const*> const* GarrisonMgr::GetGarrPlotInstForSiteLevel(uint32 garrSiteLevelId) const
{
    return Trinity::Containers::MapGetValuePtr(_garrisonPlotInstBySiteLevel, garrSiteLevelId);
}

GameObjectsEntry const* GarrisonMgr::GetPlotGameObject(uint32 mapId, uint32 garrPlotInstanceId) const
{
    auto mapItr = _garrisonPlots.find(mapId);
    if (mapItr != _garrisonPlots.end())
        return Trinity::Containers::MapGetValuePtr(mapItr->second, garrPlotInstanceId);

    return nullptr;
}

bool GarrisonMgr::IsPlotMatchingBuilding(uint32 garrPlotId, uint32 garrBuildingId) const
{
    auto plotItr = _garrisonBuildingsByPlot.find(garrPlotId);
    if (plotItr != _garrisonBuildingsByPlot.end())
        return plotItr->second.count(garrBuildingId) > 0;

    return false;
}

uint32 GarrisonMgr::GetGarrBuildingPlotInst(uint32 garrBuildingId, uint32 garrSiteLevelPlotInstId) const
{
    auto itr = _garrisonBuildingPlotInstances.find(MAKE_PAIR64(garrBuildingId, garrSiteLevelPlotInstId));
    if (itr != _garrisonBuildingPlotInstances.end())
        return itr->second;

    return 0;
}

GarrBuildingEntry const* GarrisonMgr::GetPreviousLevelBuilding(uint32 buildingType, uint32 currentLevel) const
{
    auto itr = _garrisonBuildingsByType.find(buildingType);
    if (itr != _garrisonBuildingsByType.end())
        for (GarrBuildingEntry const* building : itr->second)
            if (building->UpgradeLevel == currentLevel - 1)
                return building;

    return nullptr;
}

FinalizeGarrisonPlotGOInfo const* GarrisonMgr::GetPlotFinalizeGOInfo(uint32 garrPlotInstanceID) const
{
    return Trinity::Containers::MapGetValuePtr(_finalizePlotGOInfo, garrPlotInstanceID);
}

uint64 GarrisonMgr::GenerateFollowerDbId()
{
    if (_followerDbIdGenerator >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Garrison follower db id overflow! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }

    return _followerDbIdGenerator++;
}

uint64 GarrisonMgr::GenerateMissionDbId()
{
    if (_missionDbIdGenerator >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Garrison mission db id overflow! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }

    return _missionDbIdGenerator++;
}

uint64 GarrisonMgr::GenerateShipmentDbId()
{
    if (_shipmentDbIdGenerator >= std::numeric_limits<uint64>::max())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Garrison shipment db id overflow! Can't continue, shutting down server. ");
        World::StopNow(ERROR_EXIT_CODE);
    }

    return _shipmentDbIdGenerator++;
}

std::vector<uint32> GarrisonMgr::RollFollowerAbilities(GarrFollowerEntry const* follower, uint32 quality, uint32 faction, bool initial) const
{
    ASSERT(faction < 2);

    uint8 abilityCount = 0;
    uint8 traitCount = 0;

    for (auto const entry : sGarrFollowerQualityStore)
    {
        if (entry->GarrFollowerTypeID != follower->GarrFollowerTypeID || entry->Quality != quality)
            continue;

        abilityCount = entry->AbilityCount;
        traitCount = entry->TraitCount;
        break;
    }

    GarrAbilities const* abilities = nullptr;
    auto itr = _garrisonFollowerAbilities[faction].find(follower->ID);
    if (itr != _garrisonFollowerAbilities[faction].end())
        abilities = &itr->second;

    std::list<GarrAbilityEntry const*> abilityList, forcedAbilities, traitList, forcedTraits;
    if (abilities)
    {
        for (auto ability : abilities->Counters)
        {
            if (ability->Flags & GARRISON_ABILITY_HORDE_ONLY && faction != GARRISON_FACTION_INDEX_HORDE)
                continue;
            if (ability->Flags & GARRISON_ABILITY_ALLIANCE_ONLY && faction != GARRISON_FACTION_INDEX_ALLIANCE)
                continue;

            //@TODO GARRISON_ABILITY_FLAG_IS_EMPTY_SLOT - what about quality tho?

            if (ability->Flags & GARRISON_ABILITY_FLAG_CANNOT_REMOVE)
                forcedAbilities.push_back(ability);
            else
                abilityList.push_back(ability);
        }

        for (auto ability : abilities->Traits)
        {
            if (ability->Flags & GARRISON_ABILITY_HORDE_ONLY && faction != GARRISON_FACTION_INDEX_HORDE)
                continue;
            if (ability->Flags & GARRISON_ABILITY_ALLIANCE_ONLY && faction != GARRISON_FACTION_INDEX_ALLIANCE)
                continue;

            //@TODO GARRISON_ABILITY_FLAG_IS_EMPTY_SLOT - what about quality tho?

            if (ability->Flags & GARRISON_ABILITY_FLAG_CANNOT_REMOVE)
                forcedTraits.push_back(ability);
            else
                traitList.push_back(ability);
        }
    }

    Trinity::Containers::RandomResizeList(abilityList, std::max<int32>(0, abilityCount - forcedAbilities.size()));
    Trinity::Containers::RandomResizeList(traitList, std::max<int32>(0, traitCount - forcedTraits.size()));

    // Add abilities specified in GarrFollowerXAbility.db2 before generic classspec ones on follower creation
    if (initial)
    {
        forcedAbilities.splice(forcedAbilities.end(), abilityList);
        forcedTraits.splice(forcedTraits.end(), traitList);
    }

    forcedAbilities.sort();
    abilityList.sort();
    forcedTraits.sort();
    traitList.sort();

    bool hasForcedExclusiveTrait = false;
    // check if we have a trait from exclusive category
    for (auto ability : forcedTraits)
    {
        if (ability->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE)
        {
            hasForcedExclusiveTrait = true;
            break;
        }
    }

    if (abilityCount > forcedAbilities.size() + abilityList.size())
    {
        auto classSpecAbilities = GetClassSpecAbilities(follower, faction);
        std::list<GarrAbilityEntry const*> classSpecAbilitiesTemp, classSpecAbilitiesTemp2;
        classSpecAbilitiesTemp2.swap(abilityList);
        std::set_difference(classSpecAbilities.begin(), classSpecAbilities.end(), forcedAbilities.begin(), forcedAbilities.end(), std::back_inserter(classSpecAbilitiesTemp));
        std::set_union(classSpecAbilitiesTemp.begin(), classSpecAbilitiesTemp.end(), classSpecAbilitiesTemp2.begin(), classSpecAbilitiesTemp2.end(), std::back_inserter(abilityList));

        Trinity::Containers::RandomResizeList(abilityList, std::max<int32>(0, abilityCount - forcedAbilities.size()));
    }

    if (traitCount > forcedTraits.size() + traitList.size())
    {
        std::list<GarrAbilityEntry const*> genericTraits, genericTraitsTemp;
        for (auto ability : _garrisonFollowerRandomTraits)
        {
            if (ability->Flags & GARRISON_ABILITY_HORDE_ONLY && faction != GARRISON_FACTION_INDEX_HORDE)
                continue;
            if (ability->Flags & GARRISON_ABILITY_ALLIANCE_ONLY && faction != GARRISON_FACTION_INDEX_ALLIANCE)
                continue;

            // forced exclusive trait exists, skip other ones entirely
            if (hasForcedExclusiveTrait && ability->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE)
                continue;

            genericTraitsTemp.push_back(ability);
        }

        std::set_difference(genericTraitsTemp.begin(), genericTraitsTemp.end(), forcedTraits.begin(), forcedTraits.end(), std::back_inserter(genericTraits));
        genericTraits.splice(genericTraits.begin(), traitList);
        // "split" the list into two parts [nonexclusive, exclusive] to make selection later easier
        genericTraits.sort([](GarrAbilityEntry const* a1, GarrAbilityEntry const* a2)
        {
            uint32 e1 = a1->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE;
            uint32 e2 = a2->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE;
            if (e1 != e2)
                return e1 < e2;

            return a1->ID < a2->ID;
        });
        genericTraits.unique();

        size_t firstExclusive = 0, total = genericTraits.size();
        for (auto itr_ = genericTraits.begin(); itr_ != genericTraits.end(); ++itr_, ++firstExclusive)
            if ((*itr_)->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE)
                break;

        while (traitList.size() < std::max<int32>(0, traitCount - forcedTraits.size()) && total)
        {
            auto itr_2 = genericTraits.begin();
            std::advance(itr_2, urand(0, total-- - 1));
            if ((*itr_2)->Flags & GARRISON_ABILITY_FLAG_EXCLUSIVE)
                total = firstExclusive; // selected exclusive trait - no other can be selected now
            else
                --firstExclusive;

            traitList.push_back(*itr_2);
            genericTraits.erase(itr_2);
        }
    }

    std::list<GarrAbilityEntry const*> result;
    result.splice(result.end(), forcedAbilities);
    result.splice(result.end(), abilityList);
    result.splice(result.end(), forcedTraits);
    result.splice(result.end(), traitList);

    std::vector<uint32> output;
    for (auto xx : result)
        output.emplace_back(xx->ID);
    return output;
}

std::list<GarrAbilityEntry const*> GarrisonMgr::GetClassSpecAbilities(GarrFollowerEntry const* follower, uint32 faction) const
{
    uint32 classSpecId;
    switch (faction)
    {
        case GARRISON_FACTION_INDEX_HORDE:
            classSpecId = follower->HordeGarrClassSpecID;
            break;
        case GARRISON_FACTION_INDEX_ALLIANCE:
            classSpecId = follower->AllianceGarrClassSpecID;
            break;
        default:
            return {};
    }

    if (!sGarrClassSpecStore.LookupEntry(classSpecId))
        return {};

    auto itr = _garrisonFollowerClassSpecAbilities.find(classSpecId);
    if (itr != _garrisonFollowerClassSpecAbilities.end())
        return itr->second;

    return {};
}

void GarrisonMgr::InitializeDbIdSequences()
{
    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(dbId) FROM character_garrison_followers"))
        _followerDbIdGenerator = (*result)[0].GetUInt64() + 1;

    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(dbId) FROM character_garrison_missions"))
        _missionDbIdGenerator = (*result)[0].GetUInt64() + 1;

    if (QueryResult result = CharacterDatabase.Query("SELECT MAX(dbId) FROM character_garrison_shipment"))
        _shipmentDbIdGenerator = (*result)[0].GetUInt64() + 1;
}

GarrMissionRewardEntry::GarrMissionRewardEntry(Field* fields)
{
    //SELECT  `MissionID`, `RewardXP`, `RewardItemID`, `ItemAmount`, `CurrencyID`, `CurrencyValue`, `BonusAbilityID`, `Unknown`, `KillCredit`
    MissionID = fields[0].GetUInt32();
    RewardXP = fields[1].GetUInt32();
    RewardItemID = fields[2].GetUInt32();
    ItemAmount = fields[3].GetUInt32();
    CurrencyID = fields[4].GetUInt32();
    CurrencyValue = fields[5].GetUInt32();
    BonusAbilityID = fields[6].GetUInt32();
    Unknown = fields[7].GetUInt32();
    KillCredit = fields[8].GetUInt32();
}

void GarrisonMgr::LoadMissionsRewards()
{
    //                                                      0          1          2              3              4             5                6                7           8
    QueryResult result = WorldDatabase.Query("SELECT  `MissionID`, `RewardXP`, `RewardItemID`, `ItemAmount`, `CurrencyID`, `CurrencyValue`, `BonusAbilityID`, `Unknown`, `KillCredit` FROM mission_reward ORDER BY `CurrencyID` DESC");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 mission reward. DB table `mission_reward` is empty.");
        return;
    }
    uint32 msTime = getMSTime();
    do
    {
        auto rew = GarrMissionRewardEntry(result->Fetch());
        _garrMissionRewardByMissionID[rew.MissionID] = rew;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u mission rewards in %u.", uint32(_garrMissionRewardByMissionID.size()), GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadMissionsOwermaxRewards()
{
    uint32 msTime = getMSTime();

    ////                                                      0          1          2              3              4             5                6                7           8
    //QueryResult result = WorldDatabase.Query("SELECT  `MissionID`, `RewardXP`, `RewardItemID`, `ItemAmount`, `CurrencyID`, `CurrencyValue`, `BonusAbilityID`, `Unknown`, `KillCredit` FROM mission_reward_owermax ORDER BY `CurrencyID` DESC");
    //if (!result)
    //{
    //    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 mission owermax reward. DB table `mission_reward_owermax` is empty.");
    //    return;
    //}

    //do
    //{
    //    auto rew = GarrMissionRewardEntry(result->Fetch());
    //    _garrMissionOwermaxRewardByMissionID[rew.MissionID] = rew;

    //} while (result->NextRow());

    for (auto const& missionEntry : sGarrMissionStore)
    {
        auto rewardPackID = missionEntry->OvermaxRewardPackID;
        if (!rewardPackID)
            continue;

        auto const& rewarPackEntry = sRewardPackStore.LookupEntry(rewardPackID);
        if (!rewarPackEntry)
            continue;

        auto& data = _garrMissionOwermaxRewardByMissionID[missionEntry->ID];

        if (auto const& rewardPackItem = sDB2Manager.GetRewardPackXItem(rewardPackID))
        {
            data.RewardItemID = rewardPackItem->ItemID;
            data.ItemAmount = rewardPackItem->ItemQuantity;

            if (auto itemEntry  = sItemStore[data.RewardItemID])
                data.Unknown = itemEntry->IconFileDataID;
        }

        if (auto money = rewarPackEntry->Money)
        {
            data.CurrencyID = 0;
            data.CurrencyValue = money;

        }
        else if (auto const& rewardPackCurrency = sDB2Manager.GetRewardPackXCurrency(rewardPackID))
        {
            data.CurrencyID = rewardPackCurrency->CurrencyTypeID;
            data.CurrencyValue = rewardPackCurrency->Quantity;
        }

        data.RewardXP = 0; // no extra follower XP at all
        data.BonusAbilityID = 0; // not needed
        data.KillCredit = 0; // not needed
    }

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u mission owermax rewards in %u.", uint32(_garrMissionOwermaxRewardByMissionID.size()), GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadMissionsQuestLink()
{
    //                                                      0          1
    QueryResult result = WorldDatabase.Query("SELECT  `QuestID`, `MissionStartID` FROM mission_quest_link");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 mission quest reward. DB table `mission_quest_link` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();

        uint32 missionID = fields[1].GetUInt32();

        if (!sGarrMissionStore.LookupEntry(missionID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing sGarrMissionStore missionID %u was referenced in `mission_quest_link`.", missionID);
            continue;
        }

        uint32 quest_id = fields[0].GetUInt32();
        if (!sQuestDataStore->GetQuestTemplate(quest_id))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing quest %u was referenced in `mission_quest_link`.", quest_id);
            continue;
        }
        _quest_mission_link_store[quest_id] = missionID;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u mission rewards in %u.", uint32(_quest_mission_link_store.size()), GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadPlotFinalizeGOInfo()
{
    //                                                                0                  1       2       3       4       5
    QueryResult result = WorldDatabase.Query("SELECT garrPlotInstanceId, hordeGameObjectId, hordeX, hordeY, hordeZ, hordeO, "
    //                      6          7          8         9         10                 11
        "allianceGameObjectId, allianceX, allianceY, allianceZ, allianceO FROM garrison_plot_finalize_info");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison follower class spec abilities. DB table `garrison_plot_finalize_info` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    do
    {
        Field* fields = result->Fetch();
        uint32 garrPlotInstanceId = fields[0].GetUInt32();
        uint32 hordeGameObjectId = fields[1].GetUInt32();
        uint32 allianceGameObjectId = fields[6].GetUInt32();
        
        if (!sGarrPlotInstanceStore.LookupEntry(garrPlotInstanceId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrPlotInstance.db2 entry %u was referenced in `garrison_plot_finalize_info`.", garrPlotInstanceId);
            continue;
        }

        GameObjectTemplate const* goTemplate = sObjectMgr->GetGameObjectTemplate(hordeGameObjectId);
        if (!goTemplate)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing gameobject_template entry %u was referenced in `garrison_plot_finalize_info`.`hordeGameObjectId` for garrPlotInstanceId %u goID %u.", hordeGameObjectId, garrPlotInstanceId, hordeGameObjectId);
            continue;
        }

        if (goTemplate->type != GAMEOBJECT_TYPE_GOOBER)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid gameobject type %u (entry %u) was referenced in `garrison_plot_finalize_info`.`hordeGameObjectId` for garrPlotInstanceId %u.", goTemplate->type, hordeGameObjectId, garrPlotInstanceId);
            continue;
        }

        goTemplate = sObjectMgr->GetGameObjectTemplate(allianceGameObjectId);
        if (!goTemplate)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing gameobject_template entry %u was referenced in `garrison_plot_finalize_info`.`allianceGameObjectId` for garrPlotInstanceId %u goID %u.", allianceGameObjectId, garrPlotInstanceId, allianceGameObjectId);
            continue;
        }

        if (goTemplate->type != GAMEOBJECT_TYPE_GOOBER)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Invalid gameobject type %u (entry %u) was referenced in `garrison_plot_finalize_info`.`allianceGameObjectId` for garrPlotInstanceId %u.", goTemplate->type, allianceGameObjectId, garrPlotInstanceId);
            continue;
        }
        
        FinalizeGarrisonPlotGOInfo& info = _finalizePlotGOInfo[garrPlotInstanceId];
        info.FactionInfo[GARRISON_FACTION_INDEX_HORDE].GameObjectId = hordeGameObjectId;
        info.FactionInfo[GARRISON_FACTION_INDEX_HORDE].Pos.Relocate(fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat());

        info.FactionInfo[GARRISON_FACTION_INDEX_ALLIANCE].GameObjectId = allianceGameObjectId;
        info.FactionInfo[GARRISON_FACTION_INDEX_ALLIANCE].Pos.Relocate(fields[7].GetFloat(), fields[8].GetFloat(), fields[9].GetFloat(), fields[10].GetFloat());
    }
    while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison plot finalize entries in %u.", uint32(_finalizePlotGOInfo.size()), GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadFollowerClassSpecAbilities()
{
    QueryResult result = WorldDatabase.Query("SELECT classSpecId, abilityId FROM garrison_follower_class_spec_abilities");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison follower class spec abilities. DB table `garrison_follower_class_spec_abilities` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 classSpecId = fields[0].GetUInt32();
        uint32 abilityId = fields[1].GetUInt32();

        if (!sGarrClassSpecStore.LookupEntry(classSpecId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrClassSpec.db2 entry %u was referenced in `garrison_follower_class_spec_abilities` by row (%u, %u).", classSpecId, classSpecId, abilityId);
            continue;
        }

        GarrAbilityEntry const* ability = sGarrAbilityStore.LookupEntry(abilityId);
        if (!ability)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrAbility.db2 entry %u was referenced in `garrison_follower_class_spec_abilities` by row (%u, %u).", abilityId, classSpecId, abilityId);
            continue;
        }

        _garrisonFollowerClassSpecAbilities[classSpecId].push_back(ability);
        ++count;

    }
    while (result->NextRow());

    for (auto& pair : _garrisonFollowerClassSpecAbilities)
        pair.second.sort();

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison follower class spec abilities in %u.", count, GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadBuildingSpawnNPC()
{
    //                                                  0       1      2   3    4              5           6            7
    QueryResult result = WorldDatabase.Query("SELECT plotID, BuildID, id, map, position_x, position_y, position_z, orientation, building FROM garrison_building_creature");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison building creatures. DB table `garrison_building_creature` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        uint32 garrPlotInstanceId = fields[index++].GetUInt32();
        uint32 BuildID = fields[index++].GetUInt32();
        uint32 entry = fields[index++].GetUInt32();

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry);
        if (!cInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_building_creature` has creature with non existing creature entry %u, skipped.", entry);
            continue;
        }

        if (!sGarrPlotInstanceStore.LookupEntry(garrPlotInstanceId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrPlotInstance.db2 entry %u was referenced in `garrison_building_creature`.", garrPlotInstanceId);
            continue;
        }

        //! BuildID = 0 - empty build spawn.
        if (BuildID && !sGarrBuildingStore.LookupEntry(BuildID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrBuilding.db2 entry %u was referenced in `garrison_building_creature`.", BuildID);
            continue;
        }

        CreatureData data;
        data.id = entry;
        uint32 map = fields[index++].GetUInt16();
        data.mapid = getFirstMap(map);
        data.posX = fields[index++].GetFloat();
        data.posY = fields[index++].GetFloat();
        data.posZ = fields[index++].GetFloat();
        data.orientation = fields[index++].GetFloat();
        data.building = fields[index++].GetBool();
        data.dbData = false;
        _buildSpawnNpc[BuildID][garrPlotInstanceId].push_back(data);

        if (!data.mapid)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Not supported map %u in `garrison_building_creature`.", map);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison building creatures in %u.", count, GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadBuildingSpawnGo()
{
    //                                                  0       1      2   3    4              5           6            7           8           9       10          11          12
    QueryResult result = WorldDatabase.Query("SELECT plotID, BuildID, id, map, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, building FROM garrison_building_gameobject");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison building go. DB table `garrison_building_gameobject` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        uint32 garrPlotInstanceId = fields[index++].GetUInt32();
        uint32 BuildID = fields[index++].GetUInt32();
        uint32 entry = fields[index++].GetUInt32();
        auto templ = sObjectMgr->GetGameObjectTemplate(entry);
        if (!templ)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_building_gameobject` has go with non existing go entry %u, skipped.", entry);
            continue;
        }

        if (templ->type == GAMEOBJECT_TYPE_GARRISON_BUILDING || templ->type == GAMEOBJECT_TYPE_GARRISON_PLOT)
        {
            if (entry  != 239085)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_building_gameobject` has go (%u) with no allowed type %u, skipped.", templ->type, entry);
                continue;
            }
        }

        if (!sGarrPlotInstanceStore.LookupEntry(garrPlotInstanceId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrPlotInstance.db2 entry %u was referenced in `garrison_building_gameobject`.", garrPlotInstanceId);
            continue;
        }

        //! BuildID = 0 - empty build spawn.
        if (BuildID && !sGarrBuildingStore.LookupEntry(BuildID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing GarrBuilding.db2 entry %u was referenced in `garrison_building_gameobject`.", BuildID);
            continue;
        }

        GameObjectData data;
        data.id = entry;
        uint32 map = fields[index++].GetUInt16();
        data.mapid = getFirstMap(map);
        data.posX = fields[index++].GetFloat();
        data.posY = fields[index++].GetFloat();
        data.posZ = fields[index++].GetFloat();
        data.orientation = fields[index++].GetFloat();
        data.rotation.x = fields[index++].GetFloat();
        data.rotation.y = fields[index++].GetFloat();
        data.rotation.z = fields[index++].GetFloat();
        data.rotation.w = fields[index++].GetFloat();
        data.building = fields[index++].GetBool();
        data.dbData = false;
        _buildSpawnGo[BuildID][garrPlotInstanceId].push_back(data);

        if (!data.mapid)
            TC_LOG_ERROR(LOG_FILTER_SQL, "Not supported map %u in `garrison_building_gameobject`.", map);

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison building go in %u.", count, GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadMissionLine()
{
    //                                                  0    1          2                   3
    QueryResult result = WorldDatabase.Query("SELECT ID, NextMission, ReqGarrFollowerID, IsRandom FROM garrison_mission_line");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison mission lines. DB table `garrison_mission_line` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        uint32 missionID = fields[index++].GetUInt32();
        uint32 NextMission = fields[index++].GetUInt32();
        uint32 ReqGarrFollowerID = fields[index++].GetUInt32();
        bool IsRandom = fields[index++].GetBool();

        GarrMissionEntry const* missionEntry = sGarrMissionStore.LookupEntry(missionID);
        if (!missionEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing sGarrMissionStore missionID %u was referenced in `garrison_mission_line`.", missionID);
            continue;
        }

        GarrMissionEntry const* NextMissionEntry = sGarrMissionStore.LookupEntry(NextMission);

        if (NextMission && !NextMissionEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing sGarrMissionStore missionID %u was referenced in `garrison_mission_line`.", NextMission);
            continue;
        }

        GarrFollowerEntry const* followerEntry = sGarrFollowerStore.LookupEntry(ReqGarrFollowerID);
        if (ReqGarrFollowerID && !followerEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing sGarrFollowerStore ReqGarrFollowerID %u was referenced in `garrison_mission_line`.", ReqGarrFollowerID);
            continue;
        }

        GarrMissionLine &data = _MissionLineStore[missionID];
        data.MissionID = missionEntry;
        data.NextMission = NextMissionEntry;
        data.Reqfollower = followerEntry;
        data.isRandom = IsRandom;

        if (NextMissionEntry)
            _nextMission[missionID] = NextMissionEntry;

        if (followerEntry)
            _nextMissionByFollower[ReqGarrFollowerID] = missionEntry;

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison mission lines in %u.", count, GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadShipment()
{
    //                                                  0            1         2         3      4            5
    QueryResult result = WorldDatabase.Query("SELECT SiteID, ContainerID, NpcEntry, questReq, ShipmentID, classReq FROM garrison_shipment");

    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison shipment. DB table `garrison_shipment` is empty.");
        return;
    }

    uint32 msTime = getMSTime();
    uint32 count = 0;
    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();

        GarrShipment data;
        data.SiteID = fields[index++].GetUInt32();
        data.ContainerID = fields[index++].GetUInt32();
        data.NpcEntry = fields[index++].GetUInt32();
        data.questReq = fields[index++].GetUInt32();
        data.ShipmentID = fields[index++].GetUInt32();
        data.classReq = fields[index++].GetUInt32();

        if (data.SiteID && data.SiteID != SITE_ID_GARRISON_ALLIANCE && data.SiteID != SITE_ID_GARRISON_HORDE)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_shipment` has non-existen SiteID %u, skipped.", data.SiteID);
            continue;
        }

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(data.NpcEntry);
        if (!cInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_shipment` has creature with non existing creature entry %u, skipped.", data.NpcEntry);
            continue;
        }

        if (data.ShipmentID)
        {
            if (!sCharShipmentStore.LookupEntry(data.ShipmentID))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing CharShipment.db2 entry %u was referenced in `garrison_shipment`.", data.ShipmentID);
                continue;
            }
        }

        CharShipmentContainerEntry const* shipmentConteinerEntry = sCharShipmentContainerStore.LookupEntry(data.ContainerID);
        if (!shipmentConteinerEntry)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Non-existing CharShipmentContainer.db2 entry %u was referenced in `garrison_shipment`.", data.ContainerID);
            continue;
        }
        data.cEntry = shipmentConteinerEntry;

        const_cast<CreatureTemplate*>(cInfo)->npcflag2 |= UNIT_NPC_FLAG2_AI_OBSTACLE;
        const_cast<CreatureTemplate*>(cInfo)->npcflag |= UNIT_NPC_FLAG_GOSSIP;
        const_cast<CreatureTemplate*>(cInfo)->CursorName = "workorders";

        shipment[SHIPMENT_GET_BY_NPC].insert(std::make_pair(data.NpcEntry, data));
        shipment[SHIPMENT_GET_BY_CONTEINER_ID].insert(std::make_pair(data.ContainerID, data));

        ++count;
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison_shipment in %u.", count, GetMSTimeDiffToNow(msTime));
}

void GarrisonMgr::LoadTradeSkill()
{
    uint32 msTime = getMSTime();
    uint32 count = 0;

    //! WARNING! ORDER IS PART OF LOGIC!
    //                                                  0            1         2
    QueryResult result = WorldDatabase.Query("SELECT npcEntry, spellID, conditionID FROM garrison_tradeskill ORDER BY `npcEntry` DESC, `conditionID` ASC");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded 0 garrison tradeskills. DB table `garrison_tradeskill` is empty.");
        return;
    }

    do
    {
        uint8 index = 0;
        Field* fields = result->Fetch();
        GarrTradeSkill gts_data;

        uint32 npc = fields[index++].GetUInt32();

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(npc);
        if (!cInfo)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "Table `garrison_tradeskill` has creature with non existing creature entry %u, skipped.", npc);
            continue;
        }

        const_cast<CreatureTemplate*>(cInfo)->npcflag2 |= UNIT_NPC_FLAG2_TRADESKILL_NPC;
        const_cast<CreatureTemplate*>(cInfo)->npcflag |= UNIT_NPC_FLAG_GOSSIP;
        const_cast<CreatureTemplate*>(cInfo)->CursorName = "trainer";

        gts_data.spellID = fields[index++].GetUInt32();
        gts_data.conditionID = fields[index++].GetUInt32();

        SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(gts_data.spellID);
        for (auto _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
        {
            if (!sSkillLineStore.LookupEntry(_spell_idx->second->SkillLine))
                continue;

            gts_data.skillID = _spell_idx->second->SkillLine;
        }

        if (gts_data.conditionID)
        {
            bool find_higher = false;
            for (auto data : _garrNpcTradeSkill[npc])
                if (data.conditionID && data.conditionID < gts_data.conditionID)
                    find_higher = true;
            gts_data.reqBuildingLvl = find_higher ? 2 : 1;
        }
        else
            gts_data.reqBuildingLvl = 0;

        _garrNpcTradeSkill[npc].push_back(gts_data);

        ++count;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, ">> Loaded %u garrison_tradeskill in %u.", count, GetMSTimeDiffToNow(msTime));
}

TradeskillList const * GarrisonMgr::GetTradeSkill(uint32 npcID)
{
    return Trinity::Containers::MapGetValuePtr(_garrNpcTradeSkill, npcID);
}

GarrShipment const* GarrisonMgr::GetGarrShipment(uint32 entry, ShipmentGetType type, uint8 classID) const
{
    auto i = shipment.find(type);
    if (i == shipment.end())
        return nullptr;

    auto itr = i->second.find(entry);
    if (itr == i->second.end())
        return nullptr;

    if (itr->second.classReq != 0 && !(itr->second.classReq & ((1<<(classID-1)))))
        return nullptr;

    return &itr->second;
}

GarrMissionEntry const* GarrisonMgr::GetNextMissionInQuestLine(uint32 missionID)
{
    return Trinity::Containers::MapGetValuePtr(_nextMission, missionID);
}

GarrMissionEntry const* GarrisonMgr::GetMissionAtFollowerTaking(uint32 followerID)
{
    return Trinity::Containers::MapGetValuePtr(_nextMissionByFollower, followerID);
}

std::list<GameObjectData> const* GarrisonMgr::GetGoSpawnBuilding(uint32 plotID, uint32 build) const
{
    auto b = _buildSpawnGo.find(build);
    if (b != _buildSpawnGo.end())
        return Trinity::Containers::MapGetValuePtr(b->second, plotID);
    return nullptr;
}

std::list<CreatureData> const* GarrisonMgr::GetNpcSpawnBuilding(uint32 plotID, uint32 build) const
{
    auto b = _buildSpawnNpc.find(build);
    if (b != _buildSpawnNpc.end())
        return Trinity::Containers::MapGetValuePtr(b->second, plotID);
    return nullptr;
}

uint32 GarrisonMgr::GetShipmentID(GarrShipment const* shipment)
{
    if (shipment->cEntry->GarrBuildingType != GARR_BTYPE_TRADING_POST)
        return shipment->ShipmentID;

    if (time(nullptr) > _randShipment[shipment->ContainerID].Timer)
    {
        uint32 count = sDB2Manager._charShipmentConteiner.count(shipment->ContainerID);
        if (!count)
            return 0;

        uint32 idx = urand(1, count);
        uint32 i = 1;
        DB2Manager::ShipmentConteinerMapBounds bounds = sDB2Manager.GetShipmentConteinerBounds(shipment->ContainerID);
        for (auto sh_idx = bounds.first; sh_idx != bounds.second; ++sh_idx)
        {
            if (i == idx)
            {
                _randShipment[shipment->ContainerID].ShipmentID = sh_idx->second->ID;
                break;
            }
            i++;
        }

        _randShipment[shipment->ContainerID].Timer = time(nullptr) + DAY;
    }

    return _randShipment[shipment->ContainerID].ShipmentID;
}

GarrMissionRewardEntry const* GarrisonMgr::GetMissionRewardByRecID(uint32 missionRecID)
{
    return Trinity::Containers::MapGetValuePtr(_garrMissionRewardByMissionID, missionRecID);
}

GarrMissionRewardEntry const* GarrisonMgr::GetMissionOwermaxRewardByRecID(uint32 missionRecID)
{
    return Trinity::Containers::MapGetValuePtr(_garrMissionOwermaxRewardByMissionID, missionRecID);
}

uint32 GarrShipment::selectShipment(Player* p) const
{
    if (ShipmentID)
        return ShipmentID;

    auto g = p->GetGarrison();
    if (!g)
        return ShipmentID;

    DB2Manager::ShipmentConteinerMapBounds bound = sDB2Manager.GetShipmentConteinerBounds(ContainerID);
    for (auto itr2 = bound.first; itr2 != bound.second; ++itr2)
    {
        if (itr2->second->Flags & GARRISON_SHIPMENT_FLAG_REQUIRE_QUEST_NOT_COMPLETE)
            if (p->GetQuestStatus(questReq) == QUEST_STATUS_INCOMPLETE)
                return itr2->second->ID;

        if ((itr2->second->Flags & GARRISON_SHIPMENT_FLAG_REQUIRE_QUEST_NOT_COMPLETE) == 0)
        {
            //special. requirement.
            switch (itr2->second->ContainerID)
            {
                // class halls. mage.
                case 131:
                    //http://ru.wowhead.com/order-advancement=384/elemental-power
                    if (itr2->second->GarrFollowerID == 769 && !g->hasTallent(384))
                        continue;
                    if (itr2->second->GarrFollowerID == 660 && g->hasTallent(384))
                        continue;
                    break;
                case 132:
                    //http://www.wowhead.com/order-advancement=385/higher-learning
                    if (itr2->second->GarrFollowerID == 769 && !g->hasTallent(385))
                        continue;
                    if (itr2->second->GarrFollowerID == 659 && g->hasTallent(385))
                        continue;
                    break;
                // class hall. Dk
                case 134:
                {
                    //if tallent http://www.wowhead.com/order-advancement=428/construct-quarter
                    if (itr2->second->GarrFollowerID == 664 && !g->hasTallent(428))
                        continue;
                    if (itr2->second->GarrFollowerID == 662 && g->hasTallent(428))
                        continue;
                    break;
                }
                case 135:
                    //http://www.wowhead.com/order-advancement=429/live-by-the-sword
                    if (itr2->second->GarrFollowerID == 894 && !g->hasTallent(429))
                        continue;
                    if (itr2->second->GarrFollowerID == 663 && g->hasTallent(429))
                        continue;
                    break;
                // class halls. warrior.
                case 158:
                //http://www.wowhead.com/order-advancement=406/trial-by-fire
                    if (itr2->second->GarrFollowerID == 688 && !g->hasTallent(406))
                        continue;
                    if (itr2->second->GarrFollowerID == 686 && g->hasTallent(406))
                        continue;
                    break;
                case 159:
                    //http://www.wowhead.com/order-advancement=407/ascension
                    if (itr2->second->GarrFollowerID == 852 && !g->hasTallent(407))
                        continue;
                    if (itr2->second->GarrFollowerID == 687 && g->hasTallent(407))
                        continue;
                    break;
                // class halls. druid.
                case 140:
                    //http://www.wowhead.com/order-advancement=351/laughing-sisters
                    if (itr2->second->GarrFollowerID == 668 && !g->hasTallent(351))
                        continue;
                    if (itr2->second->GarrFollowerID == 763 && g->hasTallent(351))
                        continue;
                    break;
                case 141:
                    //http://www.wowhead.com/order-advancement=352/force-of-the-forest
                    if (itr2->second->GarrFollowerID == 670 && !g->hasTallent(352))
                        continue;
                    if (itr2->second->GarrFollowerID == 669 && g->hasTallent(352))
                        continue;
                    break;
               // class halls. prist.
                case 149:
                    //http://www.wowhead.com/order-advancement=450/inquisition
                    if (itr2->second->GarrFollowerID == 678 && !g->hasTallent(450))
                        continue;
                    if (itr2->second->GarrFollowerID == 677 && g->hasTallent(450))
                        continue;
                    break;
                case 150:
                    //http ://www.wowhead.com/order-advancement=451/shadow-heresy
                    if (itr2->second->GarrFollowerID == 920 && !g->hasTallent(451))
                        continue;
                    if (itr2->second->GarrFollowerID == 679 && g->hasTallent(451))
                        continue;
                    break;
                // class halls. monk.
                case 124:
                    //http://www.wowhead.com/order-advancement=250/path-of-the-ox
                    if (itr2->second->GarrFollowerID == 627 && !g->hasTallent(250))
                        continue;
                    if (itr2->second->GarrFollowerID == 622 && g->hasTallent(250))
                        continue;
                    break;
                case 125:
                    //http://www.wowhead.com/order-advancement=251/path-of-the-tiger
                    if (itr2->second->GarrFollowerID == 630 && !g->hasTallent(251))
                        continue;
                    if (itr2->second->GarrFollowerID == 629 && g->hasTallent(251))
                        continue;
                    break;
                // class halls. hunter.
                case 143:
                    //http://www.wowhead.com/order-advancement=373/keen-eye
                    if (itr2->second->GarrFollowerID == 799 && !g->hasTallent(373))
                        continue;
                    if (itr2->second->GarrFollowerID == 671 && g->hasTallent(373))
                        continue;
                    break;
                case 144:
                    //http://www.wowhead.com/order-advancement=374/wild-calling
                    if (itr2->second->GarrFollowerID == 800 && !g->hasTallent(374))
                        continue;
                    if (itr2->second->GarrFollowerID == 672 && g->hasTallent(374))
                        continue;
                    break;
                // class halls. paladin.
                case 146:
                    //http://www.wowhead.com/order-advancement=395/as-one
                    if (itr2->second->GarrFollowerID == 770 && !g->hasTallent(395))
                        continue;
                    if (itr2->second->GarrFollowerID == 674 && g->hasTallent(395))
                        continue;
                    break;
                case 147:
                    //http://www.wowhead.com/order-advancement=396/templar-of-the-silver-hand
                    if (itr2->second->GarrFollowerID == 771 && !g->hasTallent(396))
                        continue;
                    if (itr2->second->GarrFollowerID == 675 && g->hasTallent(396))
                        continue;
                    break;
                // class halls. rogue
                case 152:
                    //http://www.wowhead.com/order-advancement=439/defiant-legacy
                    if (itr2->second->GarrFollowerID == 681 && !g->hasTallent(439))
                        continue;
                    if (itr2->second->GarrFollowerID == 680 && g->hasTallent(439))
                        continue;
                    break;
                case 153:
                    //http://www.wowhead.com/order-advancement=440/crimson-sails
                    if (itr2->second->GarrFollowerID == 907 && !g->hasTallent(440))
                        continue;
                    if (itr2->second->GarrFollowerID == 682 && g->hasTallent(440))
                        continue;
                    break;
                // class halls. warlock
                case 129:
                    //http ://www.wowhead.com/order-advancement=362/dark-mastery
                    if (itr2->second->GarrFollowerID == 681 && !g->hasTallent(362))
                        continue;
                    if (itr2->second->GarrFollowerID == 649 && g->hasTallent(362))
                        continue;
                    break;
                case 128:
                    //http://www.wowhead.com/order-advancement=365/grimoire-of-servitude
                    if (itr2->second->GarrFollowerID == 767 && !g->hasTallent(365))
                        continue;
                    if (itr2->second->GarrFollowerID == 741 && g->hasTallent(365))
                        continue;
                    break;
                // class halls. demon hunter.
                case 137:
                    //http://www.wowhead.com/order-advancement=417/naga-myrmidons
                    if (itr2->second->GarrFollowerID == 876 && !g->hasTallent(417))
                        continue;
                    if (itr2->second->GarrFollowerID == 665 && g->hasTallent(417))
                        continue;
                    break;
                case 138:
                    //http://www.wowhead.com/order-advancement=418/demonic-power
                    if (itr2->second->GarrFollowerID == 877 && !g->hasTallent(418))
                        continue;
                    if (itr2->second->GarrFollowerID == 666 && g->hasTallent(418))
                        continue;
                    break;
                default:
                    break;
            }

            if (questReq)
            {
                QuestStatus status = p->GetQuestStatus(questReq);

				if (status != QUEST_STATUS_INCOMPLETE && status != QUEST_STATUS_REWARDED)
					return 0;

                if (status != QUEST_STATUS_REWARDED)
                    continue;

            }
            return itr2->second->ID;
        }
        return itr2->second->ID;
    }
    return ShipmentID;
}

uint32 GarrisonMgr::getMissionAtQuestTake(uint32 quest) const
{
    auto const& itr = _quest_mission_link_store.find(quest);
    if (itr == _quest_mission_link_store.end())
        return 0;
    return itr->second;
}
