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

#ifndef Garrison_h__
#define Garrison_h__


#include "GarrisonGlobal.h"
#include "Player.h"
#include "Packets/GarrisonPackets.h"
#include "GarrisonFollower.h"
#include "GarrisonMission.h"
#include "GarrisonPlot.h"

#define MAX_BUILDING_SAVE_DATA 5

enum GarrisonType : uint8
{
    GARRISON_TYPE_UNK           = 1,
    GARRISON_TYPE_GARRISON      = 2,
    GARRISON_TYPE_CLASS_ORDER   = 3,
    GARRISON_TYPE_MAX
};

enum BuildingDataStore
{
    BUILDING_DATA_SPECIAL_SPAWN = 0,
};

enum GarrisonSiteiD
{
    SITE_ID_GARRISON_ALLIANCE = 2,
    SITE_ID_GARRISON_HORDE = 71,
    SITE_ID_CLASS_ORDER_ALLIANCE = 161,
    SITE_ID_CLASS_ORDER_HORDE = 163,
};

enum GarrisonBuildingFlags
{
    GARRISON_BUILDING_FLAG_NEEDS_PLAN   = 0x1
};

enum GarrisonFollowerFlags
{
    GARRISON_FOLLOWER_FLAG_UNIQUE   = 0x1
};

enum GarrisonAbilityFlags
{
    GARRISON_ABILITY_FLAG_TRAIT                         = 0x0001,
    GARRISON_ABILITY_CANNOT_ROLL                        = 0x0002,
    GARRISON_ABILITY_HORDE_ONLY                         = 0x0004,
    GARRISON_ABILITY_ALLIANCE_ONLY                      = 0x0008,
    GARRISON_ABILITY_FLAG_CANNOT_REMOVE                 = 0x0010,
    GARRISON_ABILITY_FLAG_EXCLUSIVE                     = 0x0020,
    GARRISON_ABILITY_FLAG_SINGLE_MISSION_DURATION       = 0x0040,
    GARRISON_ABILITY_FLAG_ACTIVE_ONLY_ON_ZONE_SUPPORT   = 0x0080,
    GARRISON_ABILITY_FLAG_APPLY_TO_FIRST_MISSION        = 0x0100,
    GARRISON_ABILITY_FLAG_IS_SPECIALIZATION             = 0x0200,
    GARRISON_ABILITY_FLAG_IS_EMPTY_SLOT                 = 0x0400
};

enum GarrisonError
{
    GARRISON_SUCCESS                                            = 0,
    GARRISON_ERROR_NO_GARRISON                                  = 1,
    GARRISON_ERROR_GARRISON_EXISTS                              = 2,
    GARRISON_ERROR_GARRISON_SAME_TYPE_EXISTS                    = 3,
    GARRISON_ERROR_INVALID_GARRISON                             = 4,
    GARRISON_ERROR_INVALID_GARRISON_LEVEL                       = 5,
    GARRISON_ERROR_GARRISON_LEVEL_UNCHANGED                     = 6,
    GARRISON_ERROR_NOT_IN_GARRISON                              = 7,
    GARRISON_ERROR_NO_BUILDING                                  = 8,
    GARRISON_ERROR_BUILDING_EXISTS                              = 9,
    GARRISON_ERROR_INVALID_PLOT_INSTANCEID                      = 10,
    GARRISON_ERROR_INVALID_BUILDINGID                           = 11,
    GARRISON_ERROR_INVALID_UPGRADE_LEVEL                        = 12,
    GARRISON_ERROR_UPGRADE_LEVEL_EXCEEDS_GARRISON_LEVEL         = 13,
    GARRISON_ERROR_PLOTS_NOT_FULL                               = 14,
    GARRISON_ERROR_INVALID_SITE_ID                              = 15,
    GARRISON_ERROR_INVALID_PLOT_BUILDING                        = 16,
    GARRISON_ERROR_INVALID_FACTION                              = 17,
    GARRISON_ERROR_INVALID_SPECIALIZATION                       = 18,
    GARRISON_ERROR_SPECIALIZATION_EXISTS                        = 19,
    GARRISON_ERROR_SPECIALIZATION_ON_COOLDOWN                   = 20,
    GARRISON_ERROR_BLUEPRINT_EXISTS                             = 21,
    GARRISON_ERROR_REQUIRES_BLUEPRINT                           = 22,
    GARRISON_ERROR_INVALID_DOODAD_SET_ID                        = 23,
    GARRISON_ERROR_BUILDING_TYPE_EXISTS                         = 24,
    GARRISON_ERROR_BUILDING_NOT_ACTIVE                          = 25,
    GARRISON_ERROR_CONSTRUCTION_COMPLETE                        = 26,
    GARRISON_ERROR_FOLLOWER_EXISTS                              = 27,
    GARRISON_ERROR_INVALID_FOLLOWER                             = 28,
    GARRISON_ERROR_FOLLOWER_ALREADY_ON_MISSION                  = 29,
    GARRISON_ERROR_FOLLOWER_IN_BUILDING                         = 30,
    GARRISON_ERROR_FOLLOWER_INVALID_FOR_BUILDING                = 31,
    GARRISON_ERROR_INVALID_FOLLOWER_LEVEL                       = 32,
    GARRISON_ERROR_MISSION_EXISTS                               = 33,
    GARRISON_ERROR_INVALID_MISSION                              = 34,
    GARRISON_ERROR_INVALID_MISSION_TIME                         = 35,
    GARRISON_ERROR_INVALID_MISSION_REWARD_INDEX                 = 36,
    GARRISON_ERROR_MISSION_NOT_OFFERED                          = 37,
    GARRISON_ERROR_ALREADY_ON_MISSION                           = 38,
    GARRISON_ERROR_MISSION_SIZE_INVALID                         = 39,
    GARRISON_ERROR_FOLLOWER_SOFT_CAP_EXCEEDED                   = 40,
    GARRISON_ERROR_NOT_ON_MISSION                               = 41,
    GARRISON_ERROR_ALREADY_COMPLETED_MISSION                    = 42,
    GARRISON_ERROR_MISSION_NOT_COMPLETE                         = 43,
    GARRISON_ERROR_MISSION_REWARDS_PENDING                      = 44,
    GARRISON_ERROR_MISSION_EXPIRED                              = 45,
    GARRISON_ERROR_NOT_ENOUGH_CURRENCY                          = 46,
    GARRISON_ERROR_NOT_ENOUGH_GOLD                              = 47,
    GARRISON_ERROR_BUILDING_MISSING                             = 48,
    GARRISON_ERROR_NO_ARCHITECT                                 = 49,
    GARRISON_ERROR_ARCHITECT_NOT_AVAILABLE                      = 50,
    GARRISON_ERROR_NO_MISSION_NPC                               = 51,
    GARRISON_ERROR_MISSION_NPC_NOT_AVAILABLE                    = 52,
    GARRISON_ERROR_INTERNAL_ERROR                               = 53,
    GARRISON_ERROR_INVALID_STATIC_TABLE_VALUE                   = 54,
    GARRISON_ERROR_INVALID_ITEM_LEVEL                           = 55,
    GARRISON_ERROR_INVALID_AVAILABLE_RECRUIT                    = 56,
    GARRISON_ERROR_FOLLOWER_ALREADY_RECRUITED                   = 57,
    GARRISON_ERROR_RECRUITMENT_GENERATION_IN_PROGRESS           = 58,
    GARRISON_ERROR_RECRUITMENT_ON_COOLDOWN                      = 59,
    GARRISON_ERROR_RECRUIT_BLOCKED_BY_GENERATION                = 60,
    GARRISON_ERROR_RECRUITMENT_NPC_NOT_AVAILABLE                = 61,
    GARRISON_ERROR_INVALID_FOLLOWER_QUALITY                     = 62,
    GARRISON_ERROR_PROXY_NOT_OK                                 = 63,
    GARRISON_ERROR_RECALL_PORTAL_USED_LESS_THAN_24_HOURS_AGO    = 64,
    GARRISON_ERROR_ON_REMOVE_BUILDING_SPELL_FAILED              = 65,
    GARRISON_ERROR_OPERATION_NOT_SUPPORTED                      = 66,
    GARRISON_ERROR_FOLLOWER_FATIGUED                            = 67,
    GARRISON_ERROR_UPGRADE_CONDITION_FAILED                     = 68,
    GARRISON_ERROR_FOLLOWER_INACTIVE                            = 69,
    GARRISON_ERROR_FOLLOWER_ACTIVE                              = 70,
    GARRISON_ERROR_FOLLOWER_ACTIVATION_UNAVAILABLE              = 71,
    GARRISON_ERROR_FOLLOWER_TYPE_MISMATCH                       = 72,
    GARRISON_ERROR_INVALID_GARRISON_TYPE                        = 73,
    GARRISON_ERROR_MISSION_START_CONDITION_FAILED               = 74,
    GARRISON_ERROR_INVALID_FOLLOWER_ABILITY                     = 75,
    GARRISON_ERROR_INVALID_MISSION_BONUS_ABILITY                = 76,
    GARRISON_ERROR_HIGHER_BUILDING_TYPE_EXISTS                  = 77,
    GARRISON_ERROR_AT_FOLLOWER_HARD_CAP                         = 78,
    GARRISON_ERROR_FOLLOWER_CANNOT_GAIN_XP                      = 79,
    GARRISON_ERROR_NO_OP                                        = 80,
    GARRISON_ERROR_AT_CLASS_SPEC_CAP                            = 81,
    GARRISON_ERROR_MISSION_REQUIRES_100_TO_START                = 82,
    GARRISON_ERROR_MISSION_MISSING_REQUIRED_FOLLOWER            = 83,
    GARRISON_ERROR_INVALID_TALENT                               = 84,
    GARRISON_ERROR_ALREADY_RESEARCHING_TALENT                   = 85,
    GARRISON_ERROR_FAILED_CONDITION                             = 86,
    GARRISON_ERROR_INVALID_TIER                                 = 87,
    GARRISON_ERROR_INVALID_CLASS                                = 88
};

enum FollowerFlags
{
    FOLLOWER_FLAG_FOLLOWER      = 0,
    FOLLOWER_FLAG_CHAMPION      = 1,
    FOLLOWER_FLAG_TROOP         = 2,
};

enum MissionState
{
    MISSION_STATE_AVAILABLE             = 0,
    MISSION_STATE_IN_PROGRESS           = 1,
    MISSION_STATE_WAITING_BONUS         = 2,
    MISSION_STATE_WAITING_OWERMAX_BONUS = 3,
    MISSION_STATE_COMPLETED             = 5,
    MISSION_STATE_COMPLETED_OWERMAX     = 6
};

enum FollowerQuality
{
    FOLLOWER_QUALITY_COMMON     = 1,
    FOLLOWER_QUALITY_UNCOMMON   = 2,
    FOLLOWER_QUALITY_RARE       = 3,
    FOLLOWER_QUALITY_EPIC       = 4,
    FOLLOWER_QUALITY_LEGENDARY  = 5,
    FOLLOWER_QUALITY_TITLE      = 6
};

enum GarrisonShipmentFlag
{
    GARRISON_SHIPMENT_FLAG_REQUIRE_QUEST_NOT_COMPLETE = 0x1,
    GARRISON_SHIPMENT_FLAG_REQUIRE_QUEST_COMPLETE = 0x4,
};

class GameObject;
class Map;

typedef std::list<WorldPackets::Garrison::GarrisonTalent> TalentSet;
typedef std::list<WorldPackets::Garrison::Shipment> ShipmentSet;
typedef std::unordered_map<uint32/*buildingType*/, ObjectGuid /*guid*/> ShipmentConteinerSpawn;
typedef std::unordered_map<uint16/*buildingType*/, std::unordered_map<uint16, uint32>> buildingData;

class Garrison
{
public:
    explicit Garrison(Player* owner);

    bool LoadFromDB(PreparedQueryResult const& garrison, PreparedQueryResult const& blueprints, PreparedQueryResult const& buildings, PreparedQueryResult const& followers, PreparedQueryResult const& abilities, PreparedQueryResult const& missions, PreparedQueryResult const& shipments, PreparedQueryResult const& talents);
    void SaveToDB(SQLTransaction const& trans);
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, SQLTransaction const& trans);

    bool Create(uint32 garrSiteId, bool skip_teleport = false);
    void Delete();
    void Upgrade();

    void Enter() const;
    void Leave() const;

    void Update(uint32 diff);
    uint32 GoBuildValuesUpdate(const GameObject* go, uint16 index, uint32 value);
    GarrisonFactionIndex GetFaction() const;

    // Plots
    std::vector<Plot*> GetPlots();
    Plot* GetPlot(uint32 garrPlotInstanceId);
    Plot const* GetPlot(uint32 garrPlotInstanceId) const;
    Plot* GetPlotWithBuildingType(uint32 BuildingTypeID);
    Plot* GetPlotWithNpc(uint32 entry);

    // Buildings
    bool LearnBlueprint(uint32 garrBuildingId);
    void UnlearnBlueprint(uint32 garrBuildingId);
    void Swap(uint32 plot1, uint32 plot2);
    void PlaceBuilding(uint32 garrPlotInstanceId, uint32 garrBuildingId, bool byQuest = false, bool swap = false);
    void CancelBuildingConstruction(uint32 garrPlotInstanceId);
    void ActivateBuilding(uint32 garrPlotInstanceId);
    uint32 GetCountOfBluePrints() const;
    uint32 GetCountOFollowers() const;

    uint32 GetBuildingData(uint32 buildingType, uint32 idx);

    void SetBuildingData(uint32 buildingType, uint32 idx, uint32 value);

    uint32 GetSpecialSpawnBuildingTime(uint32 buildingType);

    // Followers
    void AddFollower(uint32 garrFollowerId);
    Follower const* GetFollower(uint64 dbId) const;
    Follower* GetFollower(uint64 dbId);
    Follower* GetFollowerByID(uint32 entry);
    void ReTrainFollower(SpellInfo const* spellInfo, uint32 followerID);
    void ChangeFollowerVitality(SpellInfo const* spellInfo, uint32 followerID);
    uint16 GetMaxFolowerLvl(uint8 i);
    uint16 GetMinFolowerLvl(uint8 i);
    uint16 GetMaxFolowerItemLvl(uint8 i);
    uint16 GetCountFolowerItemLvl(uint32 minLevel);
    uint16 GetTroopLimit(uint32 category, uint32 classSpec) const;

    // Missions
    void CheckBasicRequirements();
    void AddMission(uint32 missionRecID, bool sendLog = true);
    void GenerateRandomMission(uint16 count = 0);
    Mission const* GetMission(uint64 dbId) const;
    std::unordered_map<uint64 /*dbId*/, Mission> const& GetMissions(GarrisonType gt) const { return _missions[gt]; }
    Mission* GetMissionByRecID(uint32 missionRecID);
    void RemoveMissionByGuid(uint64 guid);
    void RewardMission(uint32 missionRecID, bool owermax = false);
    uint32 GetMissionSuccessChance(Mission* mission, GarrMissionEntry const* mInfo);

    void SendInfo();
    void SendRemoteInfo() const;
    void SendBlueprintAndSpecializationData();
    void SendBuildingLandmarks(Player* receiver) const;
    uint32 CanUpgrade(Player* receiver, GarrSiteLevelEntry const* site) const;
    void SendGarrisonUpgradebleResult(Player* receiver, int32 garrSiteID) const;
    void SendMissionListUpdate(bool openMissionNpc) const;

    void ResetFollowerActivationLimit();
    uint32 GetPlotInstanceForBuildingType(uint32 type) const;

    // Map
    int32 GetGarrisonMapID() const;
    uint8 GetGarrisonLevel() const;
    static bool GetAreaIdForTeam(uint32 team, AreaTableEntry const* area);
    bool HasGarrison(GarrisonType type);

    //  Ressource
    uint32 GetResNumber() const;
    void UpdateResTakenTime();

    // Enother
    void OnQuestReward(uint32 questID);

    // Shipment
    void CreateShipment(ObjectGuid const& guid, uint32 count);
    void CreateGarrisonShipment(uint32 shipmentID);
    bool canAddShipmentOrder(Creature* source);
    void OnGossipSelect(WorldObject* source);
    void SendShipmentInfo(ObjectGuid const& guid);
    uint64 PlaceShipment(uint32 shipmentID, uint32 start, uint32 end = 0, uint64 dbID = 0);
    void SendGarrisonShipmentLandingPage();
    void CompleteShipments(GameObject *go);
    void FreeShipmentChest(uint32 shipment);
    uint32 GetShipmentMaxMod();

    //TradeSkill
    bool CanCastTradeSkill(ObjectGuid const& guid, uint32 spellID);
    void OnGossipTradeSkill(WorldObject* source);

    //ClassHalls;
    bool canStartUpgrade();
    void StartClassHallUpgrade(uint32 tallentID);
    void AddTalentToStore(uint32 talentID, uint32 time, uint32 flags, ObjectDBState DbState = DB_STATE_UNCHANGED);
    bool hasTallent(uint32 talentID) const;

    //Advancement
    bool hasLegionFall() const;
    bool hasLegendLimitUp() const;
    std::map<uint32 /*garrPlotInstanceId*/, Plot> _plots;

    void DecrementTroopCount(uint32 id) { --_troopCount[id]; }
    std::unordered_map<uint64 /*dbId*/, Follower>& GetFollowers(uint32 garrTypeID) { return _followers[garrTypeID]; }
protected:
    Map* FindMap() const;
    void InitializePlots(uint8 type);
    GarrisonError CheckBuildingPlacement(uint32 garrPlotInstanceId, uint32 garrBuildingId, bool byQuest = false) const;
    GarrisonError CheckBuildingRemoval(uint32 garrPlotInstanceId) const;
    Player* _owner;
    GarrSiteLevelEntry const* _siteLevel[GARRISON_TYPE_MAX];
    uint32 _followerActivationsRemainingToday;
    uint32 _lastResTaken;
    uint32 _MissionGen = 0;

    std::unordered_set<uint32 /*garrBuildingId*/> _knownBuildings;
    std::unordered_set<uint32 /*garrBuildingId*/> _buildingsToSave;
    std::unordered_map<uint64 /*dbId*/, Follower> _followers[GARRISON_TYPE_MAX];
    std::unordered_set<uint32> _followerIds[GARRISON_TYPE_MAX];
    std::unordered_map<uint64 /*dbId*/, Mission> _missions[GARRISON_TYPE_MAX];
    std::unordered_set<uint32> _missionIds[GARRISON_TYPE_MAX];
    buildingData _buildingData;
    uint32 _troopCount[5];
    std::map<uint32/*shipmentID*/, ShipmentSet> _shipments;
    std::set<uint32/*abilityID*/> _abilities;
    TalentSet _classHallTalentStore;
    uint32 talentResearchTimer = 0;
    IntervalTimer updateTimer;
};

#endif // Garrison_h__
