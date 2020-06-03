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

#ifndef TRINITY_INSTANCE_DATA_H
#define TRINITY_INSTANCE_DATA_H

#include "ZoneScript.h"
#include "World.h"
#include "ObjectMgr.h"
#include "CreatureAIImpl.h"
#include "LogsSystem.h"

namespace WorldPackets
{
    namespace WorldState
    {
        class InitWorldStates;
    }
}

#define OUT_SAVE_INST_DATA             TC_LOG_DEBUG(LOG_FILTER_TSCR, "Saving Instance Data for Instance %s (Map %d, Instance Id %d)", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_SAVE_INST_DATA_COMPLETE    TC_LOG_DEBUG(LOG_FILTER_TSCR, "Saving Instance Data for Instance %s (Map %d, Instance Id %d) completed.", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA(a)          TC_LOG_DEBUG(LOG_FILTER_TSCR, "Loading Instance Data for Instance %s (Map %d, Instance Id %d). Input is '%s'", instance->GetMapName(), instance->GetId(), instance->GetInstanceId(), a)
#define OUT_LOAD_INST_DATA_COMPLETE    TC_LOG_DEBUG(LOG_FILTER_TSCR, "Instance Data Load for Instance %s (Map %d, Instance Id: %d) is complete.", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA_FAIL        TC_LOG_ERROR(LOG_FILTER_TSCR, "Unable to load Instance Data for Instance %s (Map %d, Instance Id: %d).", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())

class Map;
class Unit;
class Player;
class GameObject;
class Creature;
class Challenge;

typedef std::set<GameObject*> DoorSet;
typedef std::set<Creature*> MinionSet;

enum EncounterFrameType
{
    ENCOUNTER_FRAME_ENGAGE                  = 2,
    ENCOUNTER_FRAME_DISENGAGE               = 3,
    ENCOUNTER_FRAME_UPDATE_PRIORITY         = 4,
    ENCOUNTER_FRAME_ADD_TIMER               = 5,
    ENCOUNTER_FRAME_ENABLE_OBJECTIVE        = 6,
    ENCOUNTER_FRAME_UPDATE_OBJECTIVE        = 7,
    ENCOUNTER_FRAME_DISABLE_OBJECTIVE       = 8,
    ENCOUNTER_FRAME_UNK7                    = 9,    // Seems to have something to do with sorting the encounter units
    ENCOUNTER_FRAME_INSTANCE_START          = 10,
    ENCOUNTER_FRAME_INSTANCE_END            = 11,
    ENCOUNTER_FRAME_SET_ALLOWING_RELEASE    = 12    // Allow player resurrect
};

enum EncounterState
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4,
    TO_BE_DECIDED = 5,
};

struct MinionData
{
    uint32 entry, bossId;
};

struct BossInfo
{
    BossInfo();
    EncounterState state;
    DoorSet door[MAX_DOOR_TYPES];
    MinionSet minion;
    BossBoundaryMap boundary;
};

struct DoorInfo
{
    explicit DoorInfo(BossInfo* _bossInfo, DoorType _type, BoundaryType _boundary);
    BossInfo* bossInfo;
    DoorType type;
    BoundaryType boundary;
};

struct MinionInfo
{
    explicit MinionInfo(BossInfo* _bossInfo) : bossInfo(_bossInfo) {}
    BossInfo* bossInfo;
};

struct DamageManager
{
    uint32 entry;
    Creature* creature;
    ObjectGuid guid;
};

struct ObjectData
{
    uint32 entry;
    uint32 type;
};

typedef std::multimap<uint32 /*entry*/, DoorInfo> DoorInfoMap;
typedef std::map<uint32 /*entry*/, MinionInfo> MinionInfoMap;
typedef std::map<uint32 /*type*/, ObjectGuid /*guid*/> ObjectGuidMap;
typedef std::map<uint32 /*entry*/, uint32 /*type*/> ObjectInfoMap;  
typedef std::map<int8, std::vector<DamageManager> > DamageManagerMap;
typedef std::map<ObjectGuid, int8> PullDamageManagerMap;
typedef std::map<uint32 /*entry*/, ObjectGuid /*guid*/> WorldObjectMap;

static uint32 const InCombatResurrectionTimer = 90 * IN_MILLISECONDS;
static uint32 const ChallengeModeOrb = 246779;
static uint32 const ChallengeModeDoor = 239323;

class InstanceScript : public ZoneScript
{
    public:
        explicit InstanceScript(Map* map);

        virtual ~InstanceScript();

        Map* instance;

        //On creation, NOT load.
        virtual void Initialize() {}

        //On delete InstanceScript
        static void DestroyInstance();
        void CreateInstance();

        //On load
        virtual void Load(char const* data);

        //When save is needed, this function generates the data
        virtual std::string GetSaveData();

        void SaveToDB();

        virtual void Update(uint32 /*diff*/) {}
        void UpdateForScript(uint32 diff);

        ObjectGuid GetObjectGuid(uint32 type) const;
        ObjectGuid GetGuidData(uint32 type) const override;

        Creature* GetCreature(uint32 type);
        GameObject* GetGameObject(uint32 type);
        Creature* GetCreatureByEntry(uint32 entry);
        GameObject* GetGameObjectByEntry(uint32 entry);

        //Used by the map's CanEnter function.
        //This is to prevent players from entering during boss encounters.
        virtual bool IsEncounterInProgress() const;

        //Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player* /*player*/) {}
        virtual void OnPlayerLeave(Player* /*player*/) {}
        virtual void OnPlayerDies(Player* /*player*/) {}

        virtual WorldLocation* GetClosestGraveYard(float /*x*/, float /*y*/, float /*z*/) { return nullptr; }

        virtual void onScenarionNextStep(uint32 /*newStep*/) {}
        virtual void CreatureDies(Creature* /*creature*/, Unit* /*killer*/) {}
        virtual void OnCreatureCreate(Creature* creature);
        virtual void OnCreatureRemove(Creature* creature);

        virtual void OnGameObjectCreate(GameObject* go);
        virtual void OnGameObjectRemove(GameObject* go);
        virtual void OnLootChestOpen(Player* player, Loot* loot, const GameObject* go) {};

        // For use in InstanceScript
        virtual void OnPlayerEnterForScript(Player* player);
        virtual void OnPlayerLeaveForScript(Player* player);
        virtual void OnPlayerDiesForScript(Player* player);
        virtual void OnCreatureCreateForScript(Creature* creature);
        virtual void OnCreatureRemoveForScript(Creature* creature);
        virtual void OnCreatureUpdateDifficulty(Creature* creature);
        virtual void EnterCombatForScript(Creature* creature, Unit* enemy);
        virtual void CreatureDiesForScript(Creature* creature, Unit* killer);
        virtual void OnGameObjectCreateForScript(GameObject* go);
        virtual void OnGameObjectRemoveForScript(GameObject* go);
        void StartEncounterLogging(uint32 encounterId);
        void LogCompletedEncounter(bool success);

        virtual void OnUnitCharmed(Unit* unit, Unit* charmer);
        virtual void OnUnitRemoveCharmed(Unit* unit, Unit* charmer);

        //Handle open / close objects
        void HandleGameObject(ObjectGuid guid, bool open, GameObject* go = nullptr);

        //change active state of doors or buttons
        void DoUseDoorOrButton(ObjectGuid guid, uint32 withRestoreTime = 0, bool useAlternativeState = false);

        //Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(ObjectGuid guid, uint32 timeToDespawn = MINUTE);

        //sends world state update to all players in instance
        void DoUpdateWorldState(WorldStates variableID, uint32 value);

        // Send Notify to all players in instance
        void DoSendNotifyToInstance(char const* format, ...);

        // Reset Achievement Criteria
        void DoResetAchievementCriteria(CriteriaTypes type, uint64 miscValue1 = 0, uint64 miscValue2 = 0, bool evenIfCriteriaComplete = false);

        // Complete Achievement for all players in instance
        DECLSPEC_DEPRECATED void DoCompleteAchievement(uint32 achievement) ATTR_DEPRECATED;

        // Update Achievement Criteria for all players in instance
        void DoUpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 = 0, uint32 miscValue2 = 0, uint32 miscValue3 = 0, Unit* unit = nullptr);

        // Start/Stop Timed Achievement Criteria for all players in instance
        void DoStartTimedAchievement(CriteriaTimedTypes type, uint32 entry);
        void DoStopTimedAchievement(CriteriaTimedTypes type, uint32 entry);

        // Remove Auras due to Spell on all players in instance
        void DoRemoveAurasDueToSpellOnPlayers(uint32 spell);

        // Remove aura from stack on all players in instance
        void DoRemoveAuraFromStackOnPlayers(uint32 spell, ObjectGuid const& casterGUID = ObjectGuid::Empty, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT, uint32 num = 1);

        // Cast spell on all players in instance
        void DoCastSpellOnPlayers(uint32 spell);

        void DoRemovePlayeresCooldownAndDebuff(bool wipe);

        void DoSetAlternatePowerOnPlayers(int32 value);

        void RepopPlayersAtGraveyard();

        void DoNearTeleportPlayers(const Position pos, bool casting = false);
        
        void DoStartMovie(uint32 movieId);

        // Add aura on all players in instance
        void DoAddAuraOnPlayers(uint32 spell);

        // Return wether server allow two side groups or not
        static bool ServerAllowsTwoSideGroups();

        virtual bool SetBossState(uint32 id, EncounterState state);
        EncounterState GetBossState(uint32 id) const;
        BossBoundaryMap const* GetBossBoundary(uint32 id) const;

        // Achievement criteria additional requirements check
        // NOTE: not use this if same can be checked existed requirement types from AchievementCriteriaRequirementType
        virtual bool CheckAchievementCriteriaMeet(uint32 /*criteria_id*/, Player const* /*source*/, Unit const* /*target*/ = nullptr, uint32 /*miscvalue1*/ = 0);

        // Checks boss requirements (one boss required to kill other)
        virtual bool CheckRequiredBosses(uint32 bossId, uint32 entry, Player const* player = nullptr) const;

        void SetCompletedEncountersMask(uint32 newMask);
        uint32 GetCompletedEncounterMask() const;

        void SendEncounterUnit(uint32 type, Unit* unit = nullptr, uint8 param1 = 0, uint8 param2 = 0);

        bool IsAllowingRelease;

        // Check if all players are dead (except gamemasters)
        virtual bool IsWipe() const;

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

        void UpdatePhasing();
        void SetBossNumber(uint32 number);

        virtual void BroadcastPacket(WorldPacket const* data) const;

        // Garrison
        virtual void OnPlaceBuilding(Player* /*owner*/, Garrison* /*gar*/, uint32 /*garrBuildingId*/, uint32 /*garrPlotInstanceId*/, time_t &time) {};

        void setScenarioStep(uint32 step);
        uint32 getScenarionStep() const;

        void UpdateDamageManager(ObjectGuid caller, int32 damage, bool heal = false);
        void AddToDamageManager(Creature* creature, uint8 pullNum = 0);
        bool CheckDamageManager();
        void SetPullDamageManager(ObjectGuid guid, uint8 pullId);
        int8 GetPullDamageManager(ObjectGuid guid) const;

        DamageManagerMap damageManager;
        PullDamageManagerMap pullDamageManager;
        bool initDamageManager;

        void ResetCombatResurrection();
        void StartCombatResurrection();
        bool CanUseCombatResurrection() const;
        void ConsumeCombatResurrectionCharge();

        /// Challenge
        void SetChallenge(Challenge* challenge);
        Challenge* GetChallenge() const;
        bool IsChallenge() const;
        void ResetChallengeMode();

        void AddChallengeModeChests(ObjectGuid chestGuid, uint8 chestLevel);
        ObjectGuid GetChellngeModeChests(uint8 chestLevel);
        void AddChallengeModeDoor(ObjectGuid doorGuid);
        void AddChallengeModeOrb(ObjectGuid orbGuid);

        std::vector<ObjectGuid> _challengeDoorGuids;
        std::vector<ObjectGuid> _challengeChestGuids;
        ObjectGuid _challengeOrbGuid;
        ObjectGuid _challengeChest;

        void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function);

        void SetDisabledBosses(uint32 p_DisableMask);
        BossInfo* GetBossInfo(uint32 id);
    protected:
        static void LoadObjectData(ObjectData const* creatureData, ObjectInfoMap& objectInfo);
        void LoadObjectData(ObjectData const* creatureData, ObjectData const* gameObjectData);
        void AddObject(Creature* obj, bool add);
        void AddObject(GameObject* obj, bool add);
        void AddObject(WorldObject* obj, uint32 type, bool add);
        void LoadDoorData(DoorData const* data);
        void LoadMinionData(MinionData const* data);

        void AddDoor(GameObject* door, bool add);
        void AddMinion(Creature* minion, bool add);

        void UpdateDoorState(GameObject* door);
        static void UpdateMinionState(Creature* minion, EncounterState state);

        std::string LoadBossState(char const* data);
        std::string GetBossSaveData();

    private:

        Challenge* _challenge;
        std::vector<BossInfo> bosses;
        DoorInfoMap doors;
        MinionInfoMap minions;
        uint32 completedEncounters;         // completed encounter mask, bit indexes are DungeonEncounter.dbc boss numbers, used for packets
        uint32 _inCombatResCount;
        uint32 _maxInCombatResCount;
        uint32 _combatResChargeTime;
        uint32 _nextCombatResChargeTime;
        EventMap _events;
        uint32 scenarioStep;
        FunctionProcessor m_Functions;
        uint32 m_DisabledMask;
        ObjectInfoMap _creatureInfo;
        ObjectInfoMap _gameObjectInfo;
        ObjectGuidMap _objectGuids;
        WorldObjectMap _creatureData; // Now is only one object peer entry, if need all object in this entry, change guid to vector<guid>
        WorldObjectMap _gameObjectData; // Now is only one object peer entry, if need all object in this entry, change guid to vector<guid>
        LogsSystem::MainData _logData;
};
#endif
