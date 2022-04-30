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

#ifndef TRINITY_MAP_H
#define TRINITY_MAP_H

#include <bitset>

#include "Cell.h"
#include "DB2Structure.h"
#include "DynamicTree.h"
#include "GameObjectModel.h"
#include "GridDefines.h"
#include "MapRefManager.h"
#include "SharedDefines.h"
#include "Timer.h"
#include "Weather.h"
#include "NGrid.h"
#include "FunctionProcessor.h"
#include "World.h"
#include "ThreadPoolMap.hpp"

#include <cds/container/feldman_hashset_hp.h>
#include "HashFuctor.h"
#include <safe_ptr.h>

struct Position;
struct ScriptAction;
struct ScriptInfo;

enum WeatherState : uint32;

class Battlefield;
class Battleground;
class BattlegroundMap;
class CreatureGroup;
class Group;
class InstanceMap;
class InstanceSave;
class InstanceScript;
class MapInstanced;
class Object;
class OutdoorPvP;
class Player;
class TempSummon;
class Unit;
class Weather;
class WorldLocation;
class WorldObject;
class WorldPacket;
class BrawlersGuild;
class StaticTransport;
struct WildBattlePetPool;
class Scenario;

namespace G3D { class Plane; }

typedef cds::container::FeldmanHashSet< cds::gc::HP, WorldObject*, WorldObjectHashAccessor > WorldObjectSet;
typedef cds::container::FeldmanHashSet< cds::gc::HP, Transport*, TransportHashAccessor > TransportHashSet;
typedef std::map<ObjectGuid, StaticTransport*> StaticTransportMap;

struct ScriptAction
{
    ObjectGuid sourceGUID;
    ObjectGuid targetGUID;
    ObjectGuid ownerGUID;                                       // owner of source if source is item
    ScriptInfo const* script;                               // pointer to static script data
};

// ******************************************
// Map file format defines
// ******************************************
struct map_fileheader
{
    uint32 mapMagic;
    uint32 versionMagic;
    uint32 buildMagic;
    uint32 areaMapOffset;
    uint32 areaMapSize;
    uint32 heightMapOffset;
    uint32 heightMapSize;
    uint32 liquidMapOffset;
    uint32 liquidMapSize;
    uint32 holesOffset;
    uint32 holesSize;
};

#define MAP_AREA_NO_AREA      0x0001

struct map_areaHeader
{
    uint32 fourcc;
    uint16 flags;
    uint16 gridArea;
};

#define MAP_HEIGHT_NO_HEIGHT            0x0001
#define MAP_HEIGHT_AS_INT16             0x0002
#define MAP_HEIGHT_AS_INT8              0x0004
#define MAP_HEIGHT_HAS_FLIGHT_BOUNDS    0x0008

struct map_heightHeader
{
    uint32 fourcc;
    uint32 flags;
    float  gridHeight;
    float  gridMaxHeight;
};

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

struct map_liquidHeader
{
    uint32 fourcc;
    uint8 flags;
    uint8 liquidFlags;
    uint16 liquidType;
    uint8  offsetX;
    uint8  offsetY;
    uint8  width;
    uint8  height;
    float  liquidLevel;
};

enum ZLiquidStatus
{
    LIQUID_MAP_NO_WATER     = 0x00000000,
    LIQUID_MAP_ABOVE_WATER  = 0x00000001,
    LIQUID_MAP_WATER_WALK   = 0x00000002,
    LIQUID_MAP_IN_WATER     = 0x00000004,
    LIQUID_MAP_UNDER_WATER  = 0x00000008
};

enum MMAP_LOAD_RESULT
{
    MMAP_LOAD_RESULT_ERROR,
    MMAP_LOAD_RESULT_OK,
    MMAP_LOAD_RESULT_IGNORED,
};

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

constexpr uint8 MAP_ALL_LIQUIDS = (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME);

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

struct LiquidData
{
    uint32 type_flags = 0;
    uint32 entry = 0;
    float level = 0.0f;
    float depth_level = 0.0f;
};

class GridMap
{
    uint32  _flags;
    union{
        float* m_V9;
        uint16* m_uint16_V9;
        uint8* m_uint8_V9;
    };
    union{
        float* m_V8;
        uint16* m_uint16_V8;
        uint8* m_uint8_V8;
    };
    G3D::Plane* _minHeightPlanes;
    // Height level data
    float _gridHeight;
    float _gridIntHeightMultiplier;

    // Area data
    uint16* _areaMap;

    // Liquid data
    float _liquidLevel;
    uint16* _liquidEntry;
    uint8* _liquidFlags;
    float* _liquidMap;
    uint16 _gridArea;
    uint16 _liquidGlobalEntry;
    uint8 _liquidGlobalFlags;
    uint8 _liquidOffX;
    uint8 _liquidOffY;
    uint8 _liquidWidth;
    uint8 _liquidHeight;
    bool _fileExists;

    bool loadAreaData(FILE* in, uint32 offset, uint32 size);
    bool loadHeightData(FILE* in, uint32 offset, uint32 size);
    bool loadLiquidData(FILE* in, uint32 offset, uint32 size);

    // Get height functions and pointers
    typedef float (GridMap::*GetHeightPtr) (float x, float y) const;
    GetHeightPtr _gridGetHeight;
    float getHeightFromFloat(float x, float y) const;
    float getHeightFromUint16(float x, float y) const;
    float getHeightFromUint8(float x, float y) const;
    float getHeightFromFlat(float x, float y) const;

public:
    GridMap();
    ~GridMap();
    bool loadData(const char* filename);
    void unloadData();

    uint16 getArea(float x, float y) const;

    float getHeight(float x, float y) const {return (this->*_gridGetHeight)(x, y);}
    float getMinHeight(float x, float y) const;
    float getLiquidLevel(float x, float y) const;
    uint8 getTerrainType(float x, float y) const;
    ZLiquidStatus getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, LiquidData* data = nullptr);
    bool fileExists() const { return _fileExists; }
};

#pragma pack(push, 1)

struct InstanceTemplate
{
    uint32 Parent;
    uint32 ScriptId;
    bool AllowMount;
    uint32 bonusChance;
};

enum LevelRequirementVsMode
{
    LEVELREQUIREMENT_HEROIC = 70
};

struct ZoneDynamicInfo
{
    ZoneDynamicInfo();
    
    WeatherState WeatherID;
    std::unique_ptr<Weather> DefaultWeather;
    uint32 MusicID;
    uint32 OverrideLightID;
    uint32 LightFadeInTime;
    float WeatherGrade;
};

#pragma pack(pop)

#define MAX_HEIGHT            100000.0f                     // can be use for find ground height at surface
#define INVALID_HEIGHT       -100000.0f                     // for check, must be equal to VMAP_INVALID_HEIGHT, real value for unknown height is VMAP_INVALID_HEIGHT_VALUE
#define MAX_FALL_DISTANCE     250000.0f                     // "unlimited fall" to find VMap ground if it is available, just larger than MAX_HEIGHT - INVALID_HEIGHT
#define DEFAULT_HEIGHT_SEARCH    250.0f                     // default search distance to find height at nearby locations
#define MIN_UNLOAD_DELAY      60000                         // immediate unload

typedef std::map<ObjectGuid::LowType/*leaderDBGUID*/, CreatureGroup*> CreatureGroupHolderType;
typedef std::unordered_map<uint32 /*zoneId*/, ZoneDynamicInfo> ZoneDynamicInfoMap;

typedef std::unordered_map<ObjectGuid, std::shared_ptr<WorldObject>> SharedObjectPtr;

class Map
{
    friend class MapReference;
    public:

    class ObjectUpdater final
    {
    public:
        void Visit(PlayerMapType &) {}
        void Visit(CorpseMapType &) {}
        template <typename OtherMapType>
        void Visit(OtherMapType &m)
        {
            i_collectObjects.insert(i_collectObjects.end(), m.begin(), m.end());
        }

        std::vector<WorldObject*> i_collectObjects;
    };

        // we can't use unordered_map due to possible iterator invalidation at insert
        typedef std::map<std::size_t, NGrid> GridContainerType;

        Map(uint32 id, time_t, uint32 InstanceId, Difficulty difficulty, Map* _parent = nullptr);
        virtual ~Map();

        MapEntry const* GetEntry() const;

        bool CanUnload(uint32 diff);

        virtual bool AddPlayerToMap(Player*, bool initPlayer = true);
        virtual void RemovePlayerFromMap(Player*, bool);
        template<class T> bool AddToMap(T *);
        template<class T> void RemoveFromMap(T *, bool);

        virtual void Update(const uint32);
        virtual void UpdateSessions(uint32 diff);
        void UpdateOutdoorPvP(uint32 diff);

        uint32 GetCurrentDiff() const;

        float GetVisibilityRange(uint32 zoneId = 0, uint32 areaId = 0) const;
        //function for setting up visibility distance for maps on per-type/per-Id basis
        virtual void InitVisibilityDistance();

        void PlayerRelocation(Player*, float x, float y, float z, float orientation);
        void CreatureRelocation(Creature* creature, float x, float y, float z, float ang, bool respawnRelocationOnFail = true);
        void GameObjectRelocation(GameObject* go, float x, float y, float z, float orientation, bool respawnRelocationOnFail = true);
        void DynamicObjectRelocation(DynamicObject* go, float x, float y, float z, float orientation);
        void AreaTriggerRelocation(AreaTrigger* at, float x, float y, float z, float orientation);

        template <typename Visitor>
        void Visit(const Cell& cell, Visitor &&visitor);

        void loadGridsInRange(Position const &center, float radius);
        bool IsRemovalGrid(float x, float y) const;
        bool IsGridLoaded(float x, float y) const;
        void LoadGrid(Position& pos);
        void LoadGrid(float x, float y);
        bool UnloadGrid(GridContainerType::iterator itr, bool unloadAll);
        virtual void UnloadAll();
        void ResetGridExpiry(NGrid& grid, float factor = 1) const;
        time_t GetGridExpiry() const;

        uint32 GetId() const;
        uint32 GetParentMap() const;
        static bool ExistMap(uint32 mapid, int gx, int gy);
        static bool ExistVMap(uint32 mapid, int gx, int gy);
        Map const* GetParent() const;
        void AddChildTerrainMap(Map* map);
        void UnlinkAllChildTerrainMaps();

        // some calls like isInWater should not use vmaps due to processor power
        // can return INVALID_HEIGHT if under z+2 z coord not found height
        float GetHeight(float x, float y, float z, bool checkVMap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH) const;
        float GetMinHeight(Position pos) const;
        float GetVmapHeight(float x, float y, float z) const;
        float GetGridMapHeigh(float x, float y) const;

        ZLiquidStatus getLiquidStatus(float x, float y, float z, uint8 ReqLiquidType, LiquidData* data = nullptr) const;
        
        uint32 GetAreaId(float x, float y, float z, bool *isOutdoors) const;
        bool GetAreaInfo(float x, float y, float z, uint32& mogpflags, int32& adtId, int32& rootId, int32& groupId) const;
        uint32 GetAreaId(float x, float y, float z) const;
        uint32 GetZoneId(float x, float y, float z) const;
        void GetZoneAndAreaId(uint32& zoneid, uint32& areaid, float x, float y, float z) const;

        bool IsOutdoors(float x, float y, float z) const;

        uint8 GetTerrainType(float x, float y) const;
        float GetWaterLevel(float x, float y) const;
        bool IsInWater(float x, float y, float z, LiquidData* data = nullptr) const;
        bool IsUnderWater(G3D::Vector3 pos) const;

        void MoveAllCreaturesInMoveList();
        void MoveAllGameObjectsInMoveList();
        void MoveAllDynamicObjectsInMoveList();
        void MoveAllAreaTriggersInMoveList();
        void RemoveAllObjectsInRemoveList();
        virtual void RemoveAllPlayers();

        // used only in MoveAllCreaturesInMoveList and ObjectGridUnloader
        bool CreatureRespawnRelocation(Creature* c, bool diffGridOnly);
        bool GameObjectRespawnRelocation(GameObject* go, bool diffGridOnly);

        virtual bool CanEnter(Player* /*player*/) { return true; }
        const char* GetMapName() const;

        uint32 GetInstanceId() const;

        // have meaning only for instanced map (that have set real difficulty)
        uint8 GetSpawnMode() const;
        void SetSpawnMode(Difficulty difficulty);

        Difficulty GetDifficultyID() const;
        Difficulty GetLootDifficulty() const;
        void SetLootDifficulty(Difficulty difficulty);

        bool IsRegularDifficulty() const;

        MapDifficultyEntry const* GetMapDifficulty() const;
        uint32 GetDifficultyLootItemContext(bool isQuest = true, bool maxLevel = false, bool isBoss = false) const;

        SharedObjectPtr m_objectHolder;

        uint16 GetMapMaxPlayers() const;
        bool Instanceable() const { return i_mapEntry && i_mapEntry->Instanceable(); }
        bool IsDungeon() const { return i_mapEntry && i_mapEntry->IsDungeon(); }
        bool IsDungeonOrRaid() const { return i_mapEntry && i_mapEntry->Is5pplDungeonOrRaid() && !i_mapEntry->IsContinent(); }
        bool IsNonRaidDungeon() const { return i_mapEntry && i_mapEntry->IsNonRaidDungeon(); }
        bool IsRaid() const { return i_mapEntry && i_mapEntry->IsRaid(); }
        bool IsLfr() const { return i_difficulty == DIFFICULTY_LFR || i_difficulty == DIFFICULTY_LFR_RAID || i_difficulty == DIFFICULTY_HC_SCENARIO || i_difficulty == DIFFICULTY_N_SCENARIO; }
        bool isChallenge() const { return i_difficulty == DIFFICULTY_MYTHIC_KEYSTONE; }
        bool IsNeedRecalc() const;
        bool IsCanScale() const;
        bool IsNeedRespawn(uint32 lastRespawn) const { return lastRespawn < m_respawnChallenge; }
        bool IsScenario() const { return i_mapEntry && i_mapEntry->IsScenario(); }
        bool IsRaidOrHeroicDungeon() const { return IsRaid() || IsHeroic(); }
        bool IsHeroic() const;
        bool Is10ManRaid() const { return IsRaid() && (i_difficulty == DIFFICULTY_10_N || i_difficulty == DIFFICULTY_25_N); }
        bool Is25ManRaid() const { return IsRaid() && (i_difficulty == DIFFICULTY_25_N || i_difficulty == DIFFICULTY_25_HC); }   // since 25man difficulties are 1 and 3, we can check them like that
        bool IsLfrRaid()    const { return i_difficulty == DIFFICULTY_LFR_RAID;   }
        bool IsNormalRaid() const { return i_difficulty == DIFFICULTY_NORMAL_RAID; }
        bool IsHeroicRaid() const { return i_difficulty == DIFFICULTY_HEROIC_RAID; }
        bool IsMythicRaid() const { return i_difficulty == DIFFICULTY_MYTHIC_RAID; }
        bool IsHeroicPlusRaid() const { return i_difficulty == DIFFICULTY_HEROIC_RAID || i_difficulty == DIFFICULTY_MYTHIC_RAID; }
        bool IsEventScenario() const { return i_difficulty == DIFFICULTY_EVENT_SCENARIO_6 || i_difficulty == DIFFICULTY_EVENT_SCENARIO; }
        bool IsBattleground() const { return i_mapEntry && i_mapEntry->IsBattleground(); }
        bool IsBattleArena() const { return i_mapEntry && i_mapEntry->IsBattleArena(); }
        bool IsBattlegroundOrArena() const { return i_mapEntry && i_mapEntry->IsBattlegroundOrArena(); }
        bool IsGarrison() const { return i_mapEntry && i_mapEntry->IsGarrison(); }
        bool IsContinent() const { return i_mapEntry && i_mapEntry->IsContinent(); }
        bool CanCreatedZone() const;
        bool CanCreatedThread() const;
        BattlegroundMap* ToBgMap()
        {
            if (IsBattlegroundOrArena()) return reinterpret_cast<BattlegroundMap*>(this);
            return nullptr;
        }
        uint32 GetMaxPlayer() const;
        uint32 GetMinPlayer() const;

        bool GetEntrancePos(int32& mapid, float& x, float& y);

        void AddObjectToRemoveList(WorldObject* obj);
        void AddObjectToSwitchList(WorldObject* obj, bool on);
        virtual void DelayedUpdate(const uint32 diff);

        void UpdateObjectVisibility(WorldObject* obj, Cell cell, CellCoord cellpair);
        void UpdateObjectsVisibilityFor(Player* player, Cell cell, CellCoord cellpair);

        void resetMarkedCells();
        bool isCellMarked(uint32 pCellId);
        void markCell(uint32 pCellId);

        bool HavePlayers() const;
        uint32 GetPlayersCountExceptGMs() const;
        bool ActiveObjectsNearGrid(NGrid const& ngrid) const;

        void AddWorldObject(WorldObject* obj);
        void RemoveWorldObject(WorldObject* obj);
        WorldObjectSet& GetAllWorldObjectOnMap();
        WorldObjectSet const& GetAllWorldObjectOnMap() const;

        uint32 GetGridCount();

        void SendToPlayers(WorldPacket const* data) const;

        typedef MapRefManager PlayerList;
        PlayerList const& GetPlayers() const { return m_mapRefManager; }

        void ApplyOnEveryPlayer(std::function<void(Player*)> function);

        uint32 GetPlayerCount() const { return m_mapRefManager.getSize(); }

        //per-map script storage
        void ScriptsStart(std::map<uint32, std::multimap<uint32, ScriptInfo> > const& scripts, uint32 id, Object* source, Object* target);
        void ScriptCommandStart(ScriptInfo const& script, uint32 delay, Object* source, Object* target);

        // must called with AddToWorld
        template <typename T>
        void AddToGrid(T *obj, Cell const &cell)
        {
            auto const ngrid = getNGrid(cell.GridX(), cell.GridY());
            if (obj->IsWorldObject())
                ngrid->GetGrid(cell.CellX(), cell.CellY()).AddWorldObject<T>(obj);
            else
                ngrid->GetGrid(cell.CellX(), cell.CellY()).AddGridObject<T>(obj);
        }

        void AddToGrid(Player *obj, Cell const &cell);

        void AddToGrid(GameObject *obj, Cell const &cell);

        void AddToGrid(Creature *obj, Cell const &cell);

        // must called with AddToWorld
        template <typename T>
        void AddToActive(T* obj)
        {
            AddToActiveHelper(obj);
        }

        void AddToActive(Creature* obj);

        // must called with RemoveFromWorld
        template <typename T>
        void RemoveFromActive(T* obj)
        {
            RemoveFromActiveHelper(obj);
        }

        void RemoveFromActive(Creature* obj);

        void SwitchGridContainers(Creature* creature, bool toWorldContainer);
        void SwitchGridContainers(GameObject* obj, bool toWorldContainer);
        template<class NOTIFIER> void VisitAll(const float &x, const float &y, float radius, NOTIFIER &notifier);
        template<class NOTIFIER> void VisitFirstFound(const float &x, const float &y, float radius, NOTIFIER &notifier);
        template<class NOTIFIER> void VisitWorld(const float &x, const float &y, float radius, NOTIFIER &notifier);
        template<class NOTIFIER> void VisitGrid(const float &x, const float &y, float radius, NOTIFIER &notifier);
        CreatureGroupHolderType CreatureGroupHolder;
        sf::contention_free_shared_mutex< > _creatureGroupLock;

        void UpdateIteratorBack(Player* player);

        TempSummon* SummonCreature(uint32 entry, Position const& pos, SummonPropertiesEntry const* properties = nullptr, uint32 duration = 0, Unit* summoner = nullptr, ObjectGuid targetGuid = ObjectGuid::Empty, uint32 spellId = 0, int32 vehId = 0, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr);
        GameObject* SummonGameObject(uint32 entry, Position pos, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime);
        GameObject* SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime, ObjectGuid viewerGuid = ObjectGuid::Empty, GuidUnorderedSet* viewersList = nullptr, bool hasCreator = true);

        void SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list = nullptr);
        AreaTrigger* GetAreaTrigger(ObjectGuid const& guid);
        Creature* GetCreature(ObjectGuid const& guid);
        GameObject* GetGameObject(ObjectGuid const& guid);
        DynamicObject* GetDynamicObject(ObjectGuid const& guid);
        EventObject* GetEventObject(ObjectGuid const& guid);
        Transport* GetTransport(ObjectGuid const& guid);
        StaticTransport* GetStaticTransport(ObjectGuid const& guid);

        MapInstanced* ToMapInstanced();
        MapInstanced const* ToMapInstanced() const;
        InstanceMap* ToInstanceMap();
        InstanceMap const* ToInstanceMap() const;

        float GetWaterOrGroundLevel(std::set<uint32> const& phases, float x, float y, float z, float* ground = nullptr, bool swim = false) const;
        float GetHeight(std::set<uint32> const& phases, float x, float y, float z, bool vmap = true, float maxSearchDist = DEFAULT_HEIGHT_SEARCH, DynamicTreeCallback* dCallback = nullptr) const;
        bool isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, std::set<uint32> const& phases, DynamicTreeCallback* dCallback = nullptr) const;
        void Balance() { _dynamicTree.balance(); }
        void RemoveGameObjectModel(GameObjectModel const& model) { _dynamicTree.remove(model); }
        void InsertGameObjectModel(GameObjectModel const& model) { _dynamicTree.insert(model); }
        bool ContainsGameObjectModel(GameObjectModel const& model) const { return _dynamicTree.contains(model);}
        bool getObjectHitPos(std::set<uint32> const& phases, bool otherIsPlayer, Position startPos, Position destPos, float modifyDist, DynamicTreeCallback* dCallback = nullptr);
        bool getObjectHitPos(std::set<uint32> const& phases, bool otherIsPlayer, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float &ry, float& rz, float modifyDist, DynamicTreeCallback* dCallback = nullptr);
        void UpdateEncounterState(EncounterCreditType type, uint32 creditEntry, Unit* sourc, Unit* player);

        virtual ObjectGuid::LowType GetOwnerGuildId(uint32 /*team*/ = TEAM_OTHER) const { return 0; }
        /*
            RESPAWN TIMES
        */
        time_t GetLinkedRespawnTime(ObjectGuid const& guid) const;
        time_t GetCreatureRespawnTime(ObjectGuid::LowType const& dbGuid) const;
        time_t GetGORespawnTime(ObjectGuid::LowType const& dbGuid) const;
        void SaveCreatureRespawnTime(ObjectGuid::LowType const& dbGuid, time_t respawnTime);
        void RemoveCreatureRespawnTime(ObjectGuid::LowType const& dbGuid);
        void SaveGORespawnTime(ObjectGuid::LowType const& dbGuid, time_t respawnTime);
        void RemoveGORespawnTime(ObjectGuid::LowType const& dbGuid);
        void LoadRespawnTimes();
        void DeleteRespawnTimes();

        static void DeleteRespawnTimesInDB(uint16 mapId, uint32 instanceId);
        WorldObject* GetActiveObjectWithEntry(uint32 entry);    ///< Hard iteration of all active object on map

        void SetZoneMusic(uint32 zoneID, uint32 musicID);
        Weather* GetOrGenerateZoneDefaultWeather(uint32 zoneId);
        void SendZoneDynamicInfo(uint32 zoneId, Player* player) const;
        void SendZoneWeather(uint32 zoneId, Player* player) const;
        void SendZoneWeather(ZoneDynamicInfo const& zoneDynamicInfo, Player* player) const; 
        void SetZoneWeather(uint32 zoneID, WeatherState weatherID, float weatherGrade);
        void SetZoneOverrideLight(uint32 zoneID, uint32 lightID, uint32 fadeInTime);

        bool IsMapUnload() { return b_isMapUnload; }
        void SetMapUnload(bool unload = true) { b_isMapUnload = unload; }
        void SetMapStop(bool _stop = true) { b_isMapStop = _stop; }

        // Update object in map
        void AddUpdateObject(Object* obj);
        void RemoveUpdateObject(Object* obj);
        void UpdateLoop(volatile uint32 _mapID);
        uint32 GetUpdateTime() const;
        uint32 GetSessionTime() const;
        void SetMapUpdateInterval();

        void TerminateThread();
        ThreadPoolMap* threadPool;
        std::set<ObjectGuid> i_objects;

        void AddToMapWait(Object* obj);
        std::set<Object*> i_objectsAddToMap;
        std::recursive_mutex m_objectsAddToMap_lock;

        FunctionProcessor m_Functions;
        time_t m_respawnChallenge;

        SessionMap m_sessions;
        WorldSessionPtr FindSession(uint32 id) const;
        void AddSession(WorldSessionPtr s);
        void AddSession_(WorldSessionPtr s);
        LockedQueue<WorldSessionPtr> addSessQueue;
        
        void LoadAllGrids(float p_MinX, float p_MaxX, float p_MinY, float p_MaxY, Player* p_Player);

        void AddTransport(Transport * t);
        void RemoveTransport(Transport * t);

        TransportHashSet m_Transports;
        virtual void UpdateTransport(uint32 diff);

        void AddStaticTransport(StaticTransport* t);
        void RemoveStaticTransport(StaticTransport* t);

        StaticTransportMap m_StaticTransports;

        BrawlersGuild* m_brawlerGuild;

        void AddMaxVisible(Object* obj);
        void RemoveMaxVisible(Object* obj);
        std::set<Object*> m_MaxVisibleList;
        std::recursive_mutex i_MaxVisibleList_lock;

        uint32 m_updateTime;
        uint32 m_sessionTime;
        uint32 m_mapLoopCounter;

        std::set<OutdoorPvP*>* OutdoorPvPList{};
        std::set<Battlefield*>* BattlefieldList;

        bool CanUnloadMap();
        void AddBattlePet(Creature* creature);
        void RemoveBattlePet(Creature* creature);
        WildBattlePetPool* GetWildBattlePetPool(Creature* creature);

        uint32 m_activeEntry;
        uint32 m_activeEncounter;

        void updateCollected(std::vector<WorldObject*>& objectsToUpdate, uint32 diff, volatile uint32 _mapId, volatile uint32 _instanceId);
        std::map<uint32, std::vector<WorldObject*>> i_objectUpdater[2][2];
        std::set<WorldObject*> i_objectTest;
        void VisitNearbyCellsOf(WorldObject* obj);

        std::set<Scenario*> m_scenarios;

        void UpdateOutdoorPvPScript();

    private:
        void LoadMapAndVMap(int gx, int gy);
        void LoadVMap(int gx, int gy);
        void LoadMap(int gx, int gy, bool reload = false);
        static void LoadMapImpl(Map* map, int gx, int gy, bool reload);
        void UnloadMap(int gx, int gy);
        static void UnloadMapImpl(Map* map, int gx, int gy);
        void LoadMMap(int gx, int gy);
        GridMap* GetGrid(float x, float y);

        void SetTimer(uint32 t) { i_gridExpiry = t < MIN_GRID_DELAY ? MIN_GRID_DELAY : t; }

        void SendInitSelf(Player* player);

        void SendInitTransports(Player* player);
        void SendRemoveTransports(Player* player);
        void SendUpdateTransportVisibility(Player* player, std::set<uint32> const& previousPhases);

        bool CreatureCellRelocation(Creature* creature, Cell new_cell);
        bool GameObjectCellRelocation(GameObject* go, Cell new_cell);
        bool DynamicObjectCellRelocation(DynamicObject* go, Cell new_cell);
        bool AreaTriggerCellRelocation(AreaTrigger* at, Cell new_cell);

        template<class T> void InitializeObject(T* obj);
        void AddCreatureToMoveList(Creature* c, float x, float y, float z, float ang);
        void RemoveCreatureFromMoveList(Creature* c);
        void AddGameObjectToMoveList(GameObject* go, float x, float y, float z, float ang);
        void RemoveGameObjectFromMoveList(GameObject* go);
        void AddDynamicObjectToMoveList(DynamicObject* go, float x, float y, float z, float ang);
        void RemoveDynamicObjectFromMoveList(DynamicObject* go);
        void AddAreaTriggerToMoveList(AreaTrigger* at, float x, float y, float z, float ang);
        void RemoveAreaTriggerFromMoveList(AreaTrigger* at);

        sf::contention_free_shared_mutex< > _creatureToMoveLock;
        sf::contention_free_shared_mutex< > _gameObjectsToMoveLock;
        sf::contention_free_shared_mutex< > _dynamicObjectsToMoveLock;
        sf::contention_free_shared_mutex< > _areaTriggersToMoveLock;

        std::vector<Creature*> _creaturesToMove;
        std::vector<GameObject*> _gameObjectsToMove;
        std::vector<DynamicObject*> _dynamicObjectsToMove;
        std::vector<AreaTrigger*> _areaTriggersToMove;

        bool IsGridLoaded(const GridCoord &) const;
        void EnsureGridCreated(const GridCoord &);
        void EnsureGridCreated_i(const GridCoord &);
        bool EnsureGridLoaded(Cell const&);
        virtual bool onEnsureGridLoaded(NGrid* grid, Cell const& cell) { return true; }
        void EnsureGridLoadedForActiveObject(Cell const&, WorldObject* object);

        NGrid * getNGrid(uint32 x, uint32 y) const
        {
            return i_grids[x][y];
        }

        void setNGrid(NGrid *grid, uint32 x, uint32 y);
        void ScriptsProcess();

        void PopulateBattlePet(uint32 diff);
        void DepopulateBattlePet();
        std::map<uint16, std::map<uint32, WildBattlePetPool>> m_wildBattlePetPool;

    protected:
        void SetUnloadReferenceLock(const GridCoord &p, bool on) { getNGrid(p.x_coord, p.y_coord)->setUnloadReferenceLock(on); }

        MapEntry const* i_mapEntry;
        Difficulty i_difficulty;
        Difficulty i_lootDifficulty;
        uint32 i_InstanceId;
        uint32 m_unloadTimer;
        float m_VisibleDistance;
        DynamicMapTree _dynamicTree;

        MapRefManager m_mapRefManager;
        MapRefManager::iterator m_mapRefIter;

        int32 m_VisibilityNotifyPeriod;

        typedef std::set<WorldObject*> ActiveNonPlayers;
        ActiveNonPlayers m_activeNonPlayers;
        ActiveNonPlayers::iterator m_activeNonPlayersIter;

    private:
        Player* _GetScriptPlayerSourceOrTarget(Object* source, Object* target, const ScriptInfo* scriptInfo) const;
        Creature* _GetScriptCreatureSourceOrTarget(Object* source, Object* target, const ScriptInfo* scriptInfo, bool bReverse = false) const;
        Unit* _GetScriptUnit(Object* obj, bool isSource, const ScriptInfo* scriptInfo) const;
        Player* _GetScriptPlayer(Object* obj, bool isSource, const ScriptInfo* scriptInfo) const;
        Creature* _GetScriptCreature(Object* obj, bool isSource, const ScriptInfo* scriptInfo) const;
        WorldObject* _GetScriptWorldObject(Object* obj, bool isSource, const ScriptInfo* scriptInfo) const;
        void _ScriptProcessDoor(Object* source, Object* target, const ScriptInfo* scriptInfo) const;
        GameObject* _FindGameObject(WorldObject* pWorldObject, ObjectGuid::LowType const& guid) const;

        time_t i_gridExpiry;

        uint32 _defaultLight;
        ZoneDynamicInfoMap _zoneDynamicInfo;
        IntervalTimer _weatherUpdateTimer;

        //used for fast base_map (e.g. MapInstanced class object) search for
        //InstanceMaps and BattlegroundMaps...
        Map* m_parentMap;                                           // points to MapInstanced* or self (always same map id)
        Map* m_parentTerrainMap;                                    // points to m_parentMap of MapEntry::ParentMapID
        std::vector<Map*>* m_childTerrainMaps;                      // contains m_parentMap of maps that have MapEntry::ParentMapID == GetId()

        GridContainerType i_loadedGrids;

        typedef std::recursive_mutex GridLockType;
        typedef std::lock_guard<GridLockType> GridGuardType;
        GridLockType i_gridLock;

        NGrid* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        GridMap* GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
        std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

        std::atomic<bool> i_scriptLock;
        std::set<WorldObject*> i_objectsToRemove;
        std::recursive_mutex i_objectsToRemove_lock;
        std::map<WorldObject*, bool> i_objectsToSwitch;
        WorldObjectSet i_worldObjects;

        typedef std::multimap<time_t, ScriptAction> ScriptScheduleMap;
        ScriptScheduleMap m_scriptSchedule;
        std::recursive_mutex m_scriptScheduleLock;

        template<class T>
        void DeleteFromWorld(T*);

        template <typename T>
        void AddToActiveHelper(T* obj)
        {
            m_activeNonPlayers.insert(obj);
        }

        template <typename T>
        void RemoveFromActiveHelper(T* obj)
        {
            m_activeNonPlayers.erase(obj);
        }

        std::map<ObjectGuid::LowType /*dbGUID*/, time_t> _creatureRespawnTimes;
        std::map<ObjectGuid::LowType /*dbGUID*/, time_t> _goRespawnTimes;
        sf::contention_free_shared_mutex< > i_lockCreatureRespawn;
        sf::contention_free_shared_mutex< > i_lockGoRespawn;

        bool b_isMapUnload;
        bool b_isMapStop;
        IntervalTimer i_timer;
        IntervalTimer i_timer_se;
        IntervalTimer i_timer_op;
        IntervalTimer i_timer_bp;
        IntervalTimer i_timer_obj;
        std::recursive_mutex i_objectLock;

        WorldSession* m_currentSession;
};

enum InstanceResetMethod
{
    INSTANCE_RESET_ALL,
    INSTANCE_RESET_CHANGE_DIFFICULTY,
    INSTANCE_RESET_GLOBAL,
    INSTANCE_RESET_GROUP_DISBAND,
    INSTANCE_RESET_GROUP_JOIN,
    INSTANCE_RESET_RESPAWN_DELAY
};

class InstanceMap : public Map
{
public:
    InstanceMap(uint32 id, time_t, uint32 InstanceId, Difficulty difficulty, Map* _parent);
    ~InstanceMap();
    bool AddPlayerToMap(Player*, bool initPlayer = true) override;
    void RemovePlayerFromMap(Player*, bool) override;
    void Update(const uint32) override;
    void CreateInstanceData(InstanceSave* save);
    bool Reset(uint8 method);
    uint32 GetScriptId() { return i_script_id; }
    InstanceScript* GetInstanceScript() { return i_data; }
    std::string const& GetScriptName() const;
    void PermBindAllPlayers(Player* source);
    void UnloadAll() override;
    bool CanEnter(Player* player) override;
    void SendResetWarnings(uint32 timeLeft) const;
    void SendInstanceGroupSizeChanged() const;
    void SetResetSchedule(bool on);

    uint32 GetMaxPlayers() const;
    uint32 GetMaxResetDelay() const;
    const WorldLocation* GetClosestGraveYard(float x, float y, float z);

    void InitVisibilityDistance() override;
    void UpdatePhasing();
private:
    bool m_resetAfterUnload;
    bool m_unloadWhenEmpty;
    InstanceScript* i_data;
    uint32 i_script_id;
};

class ZoneMap : public Map
{
    public:
        ZoneMap(uint32 id, time_t, uint32 zoneId, Map* _parent, Difficulty difficulty);
        ~ZoneMap();

        bool AddPlayerToMap(Player*, bool initPlayer = true);
        void RemovePlayerFromMap(Player*, bool);
        bool CanEnter(Player* player);
        void SetUnload();

        virtual void InitVisibilityDistance();
};

template <typename Visitor>
void Map::Visit(Cell const& cell, Visitor &&visitor)
{
    if (!cell.NoCreate())
        EnsureGridLoaded(cell);

    auto const grid = getNGrid(cell.GridX(), cell.GridY());
    if (grid && grid->isGridObjectDataLoaded())
        grid->VisitGrid(cell.CellX(), cell.CellY(), std::forward<Visitor>(visitor));
}

template <typename Notifier>
void Map::VisitAll(float const& x, float const& y, float radius, Notifier& notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.SetNoCreate();

    cell.Visit(p, Trinity::makeWorldVisitor(notifier), *this, radius, x, y);
    cell.Visit(p, Trinity::makeGridVisitor(notifier), *this, radius, x, y);
}

// should be used with Searcher notifiers, tries to search world if nothing found in grid
template <typename Notifier>
void Map::VisitFirstFound(const float &x, const float &y, float radius, Notifier &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.SetNoCreate();

    cell.Visit(p, Trinity::makeWorldVisitor(notifier), *this, radius, x, y);

    if (!notifier.i_object)
        cell.Visit(p, Trinity::makeGridVisitor(notifier), *this, radius, x, y);
}

template <typename Notifier>
void Map::VisitWorld(const float &x, const float &y, float radius, Notifier &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.SetNoCreate();

    cell.Visit(p, Trinity::makeWorldVisitor(notifier), *this, radius, x, y);
}

template <typename Notifier>
void Map::VisitGrid(const float &x, const float &y, float radius, Notifier &notifier)
{
    CellCoord p(Trinity::ComputeCellCoord(x, y));
    Cell cell(p);
    cell.SetNoCreate();

    cell.Visit(p, Trinity::makeGridVisitor(notifier), *this, radius, x, y);
}
#endif
