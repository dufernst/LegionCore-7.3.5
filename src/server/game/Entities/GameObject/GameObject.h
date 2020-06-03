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

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Common.h"
#include "SharedDefines.h"
#include "Object.h"
#include "LootMgr.h"
#include "GridObject.h"
#include "GameObjectData.h"
#include "MapObject.h"

class Unit;
class GameObjectAI;
struct GameObjectActionData;
class Transport;

enum GoBytes : uint8
{
    GAMEOJBECT_BYTES_0_STATE            = 0,
    GAMEOBJECT_BYTES_1_TYPE             = 1,
    GAMEOBJECT_BYTES_2_ART_KIT          = 2,
    GAMEOBJECT_BYTES_3_ANIM_PROGRESS    = 3
};

enum GoVisualCounter
{
    GO_VISUAL_BEFORE_COMPLETE_QUEST = 0,
    GO_VISUAL_AFTER_COMPLETEQUEST = 1,
};

typedef std::map<ObjectGuid, uint32/*time*/> lastUserList;
typedef std::unordered_map<uint32, GameObjectTemplate> GameObjectTemplateContainer;

class OPvPCapturePoint;
struct TransportAnimation;

union GameObjectValue
{
    //11 GAMEOBJECT_TYPE_TRANSPORT
    struct
    {
        uint32 PathProgress;
        TransportAnimation const* AnimationInfo;
        std::vector<uint32>* StopFrames;
    } Transport;
    //25 GAMEOBJECT_TYPE_FISHINGHOLE
    struct
    {
        uint32 MaxOpens;
    } FishingHole;
    //29 GAMEOBJECT_TYPE_CONTROL_ZONE
    struct
    {
        OPvPCapturePoint *OPvPObj;
    } ControlZone;
    //33 GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING
    struct
    {
        uint32 Health;
        uint32 MaxHealth;
    } Building;
};

// For containers:  [GO_NOT_READY]->GO_READY (close)->GO_ACTIVATED (open) ->GO_JUST_DEACTIVATED->GO_READY        -> ...
// For bobber:      GO_NOT_READY  ->GO_READY (close)->GO_ACTIVATED (open) ->GO_JUST_DEACTIVATED-><deleted>
// For door(closed):[GO_NOT_READY]->GO_READY (close)->GO_ACTIVATED (open) ->GO_JUST_DEACTIVATED->GO_READY(close) -> ...
// For door(open):  [GO_NOT_READY]->GO_READY (open) ->GO_ACTIVATED (close)->GO_JUST_DEACTIVATED->GO_READY(open)  -> ...
enum LootState
{
    GO_NOT_READY = 0,
    GO_READY,                                               // can be ready but despawned, and then not possible activate until spawn
    GO_ACTIVATED,
    GO_JUST_DEACTIVATED
};

class Unit;
class GameObjectModel;

// 5 sec for bobber catch
#define FISHING_BOBBER_READY_TIME 5

class GameObject : public WorldObject, public GridObject<GameObject>, public MapObject
{
    public:
        explicit GameObject();
        ~GameObject();

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        void AddToWorld() override;
        Battleground* GetBattleground();
        void RemoveFromWorld() override;
        void CleanupsBeforeDelete(bool finalCleanup = true) override;

        virtual bool Create(ObjectGuid::LowType guidlow, uint32 name_id, Map* map, uint32 phaseMask, Position const& pos, G3D::Quat const& rotation, uint32 animprogress, GOState go_state, uint32 artKit = 0, uint32 aid = 0, GameObjectData const* data = nullptr);
        static GameObject* CreateGameObject(uint32 entry, Map* map, Position const& pos, G3D::Quat const& rotation, uint32 animProgress, GOState goState, uint32 artKit = 0);

        void Update(uint32 p_time) override;
        static GameObject* GetGameObject(WorldObject& object, ObjectGuid guid);
        GameObjectTemplate const* GetGOInfo() const;
        GameObjectData const* GetGOData() const;
        GameObjectValue const* GetGOValue() const;

        bool IsTransport() const;
        bool IsDynTransport() const;
        bool IsDestructibleBuilding() const;

        uint64 GetDBTableGUIDLow() const;

         // z_rot, y_rot, x_rot - rotation angles around z, y and x axes
        void SetWorldRotationAngles(float z_rot, float y_rot, float x_rot);
        void SetWorldRotation(float qx, float qy, float qz, float qw);
        void SetParentRotation(G3D::Quat const& rotation);      // transforms(rotates) transport's path
        int64 GetPackedWorldRotation() const { return m_packedRotation; }

        void Say(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void Yell(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void TextEmote(int32 textId, ObjectGuid TargetGuid);
        void Whisper(int32 textId, ObjectGuid receiver);
        void YellToZone(int32 textId, uint32 language, ObjectGuid TargetGuid);

        // overwrite WorldObject function for proper name localization
        const char* GetNameForLocaleIdx(LocaleConstant locale_idx) const override;

        void SaveToDB();
        void SaveToDB(uint32 mapid, uint64 spawnMask, uint32 phaseMask);
        bool LoadFromDB(ObjectGuid::LowType guid, Map* map);
        bool LoadGameObjectFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap = true);
        void DeleteFromDB();

        void SetOwnerGUID(ObjectGuid owner);
        ObjectGuid GetOwnerGUID() const;
        Unit* GetOwner() const;
        void RemoveFromOwner();

        void SetSpellId(uint32 id);
        uint32 GetSpellId() const;

        time_t GetRespawnTime() const;
        time_t GetRespawnTimeEx() const;

        void SetRespawnTime(int32 respawn);
        void SetRespawnDelayTime(int32 respawn);
        void Respawn();
        bool isSpawned() const;
        bool isSpawnedByDefault() const;
        void SetSpawnedByDefault(bool b);
        uint32 GetRespawnDelay() const;
        void Refresh();
        void Delete();
        void getFishLoot(Loot* loot, Player* loot_owner);
        GameobjectTypes GetGoType() const;
        void SetGoType(GameobjectTypes type);
        GOState GetGoState() const;
        void SetGoState(GOState state);
        virtual uint32 GetTransportPeriod() const;
        virtual uint32 GetPathProgress() const;
        void SetTransportState(GOState state, uint32 stopFrame = 0);
        uint32 CalculateAnimDuration(GOState oldState, GOState newState) const;
        uint8 GetGoArtKit() const;
        void SetGoArtKit(uint8 artkit);
        uint8 GetGoAnimProgress() const;
        void SetGoAnimProgress(uint8 animprogress);
        static void SetGoArtKit(uint8 artkit, GameObject* go, ObjectGuid::LowType lowguid = 0);

        void SetPhaseMask(uint32 newPhaseMask, bool update) override;
        void EnableCollision(bool enable);

        void Use(Unit* user);

        LootState getLootState() const;
        // Note: unit is only used when s = GO_ACTIVATED
        void SetLootState(LootState s, Unit* unit = nullptr);
        bool IsPersonal() const;

        void AddToSkillupList(ObjectGuid const& PlayerGuidLow);
        bool IsInSkillupList(ObjectGuid const& playerGuid) const;
        void ClearSkillupList();

        void AddUniqueUse(Player* player);
        void AddUse();
        uint32 GetUseCount() const;
        uint32 GetUniqueUseCount() const;

        void SaveRespawnTime() override;

        Loot loot;
        uint16 garrBuildingType = 0;

        Player* GetLootRecipient() const;
        Group* GetLootRecipientGroup() const;
        void SetLootRecipient(Unit* unit);
        bool IsLootAllowedFor(Player const* player) const;
        bool HasLootRecipient() const;
        uint32 m_groupLootTimer;                            // (msecs)timer used for group loot
        ObjectGuid lootingGroupLowGUID;                     // used to find group which is looting

        bool hasQuest(uint32 quest_id) const override;
        bool hasInvolvedQuest(uint32 quest_id) const override;
        bool ActivateToQuest(Player* target) const;
        void UseDoorOrButton(uint32 time_to_restore = 0, bool alternative = false, Unit* user = nullptr);
                                                            // 0 = use `gameobject`.`spawntimesecs`
        void ResetDoorOrButton();

        void TriggeringLinkedGameObject(uint32 trapEntry, Unit* target);

        bool IsAlwaysVisibleFor(WorldObject const* seer) const override;
        bool IsInvisibleDueToDespawn() const override;
        bool IsNeverVisible(WorldObject const* obj) const override;

        uint8 getLevelForTarget(WorldObject const* target) const override;

        GameObject* LookupFishingHoleAround(float range);

        void CastSpell(Unit* target, uint32 spell);
        void SendCustomAnim(uint32 animID, bool playAsDespawn = false);
        void SendGOPlaySpellVisual(uint32 spellVisualID, ObjectGuid activatorGuid = ObjectGuid::Empty);
        void SetAnimKitId(uint16 animKitID, bool maintain = false);
        uint16 GetAIAnimKitId() const override;
        bool IsInRange(float x, float y, float z, float radius) const;

        void ModifyHealth(int32 change, Unit* attackerOrHealer = nullptr, uint32 spellId = 0, ObjectGuid CasterGUID = ObjectGuid::Empty);
        // sets GameObject type 33 destruction flags and optionally default health for that state
        void SetDestructibleState(GameObjectDestructibleState state, Player* eventInvoker = nullptr, bool setHealth = false);
        GameObjectDestructibleState GetDestructibleState() const;

        void EventInform(uint32 eventId);

        uint64 GetRotation() const;
        virtual uint32 GetScriptId() const;
        GameObjectAI* AI() const;

        std::string GetAIName() const;
        void SetDisplayId(uint32 displayid);
        uint32 GetDisplayId() const;
        uint8 GetNameSetId() const;

        bool isDynActive() const;
        void setDynActive(bool active);

        GameObjectModel * m_model;
        void GetRespawnPosition(float &x, float &y, float &z, float* ori = nullptr) const;

        Transport* ToTransport() { if (GetGOInfo()->type == GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT) return reinterpret_cast<Transport*>(this); else return nullptr; }
        Transport const* ToTransport() const { if (GetGOInfo()->type == GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT) return reinterpret_cast<Transport const*>(this); else return nullptr; }

        StaticTransport* ToStaticTransport() { if (GetGOInfo()->type == GAMEOBJECT_TYPE_TRANSPORT) return reinterpret_cast<StaticTransport*>(this); else return nullptr; }
        StaticTransport const* ToStaticTransport() const { if (GetGOInfo()->type == GAMEOBJECT_TYPE_TRANSPORT) return reinterpret_cast<StaticTransport const*>(this); else return nullptr; }

        float GetStationaryX() const override { return m_stationaryPosition.GetPositionX(); }
        float GetStationaryY() const override { return m_stationaryPosition.GetPositionY(); }
        float GetStationaryZ() const override { return m_stationaryPosition.GetPositionZ(); }
        float GetStationaryO() const override { return m_stationaryPosition.GetOrientation(); }
        void RelocateStationaryPosition(float x, float y, float z, float o) { m_stationaryPosition.Relocate(x, y, z, o); }

        float GetInteractionDistance() const;

        void UpdateModelPosition(bool full = true);

        void EnableOrDisableGo(bool activate, bool alternative = false);

        uint32 GetVignetteId() const;

        void setVisibilityCDForPlayer(ObjectGuid const& guid, uint32 sec = 300);

        void GameObjectAction();

        uint32 m_IfUpdateTimer;
        float m_RateUpdateTimer;
        uint32 m_RateUpdateWait;

        /// This method transforms supplied transport offsets into global coordinates
        virtual void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr);

        /// This method transforms supplied global coordinates into local offsets
        virtual void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr);

        void AddDelayedEvent(uint64 timeOffset, std::function<void()>&& function)
        {
            m_Functions.AddDelayedEvent(timeOffset, std::move(function));
        }

        void KillAllDelayedEvents()
        {
            m_Functions.KillAllFunctions();
        }

        void AIM_Destroy();
        bool AIM_Initialize();

        GameObjectValue m_goValue;

        virtual bool IsMoving() const { return false; }
        virtual void SetMoving(bool /*val*/) { }

        bool DistanceCheck() override;

    protected:
        GameObjectModel* CreateModel();
        void UpdateModel();                                 // updates model in case displayId were changed
        lastUserList m_lastUser;
        uint32      m_spellId;
        time_t      m_respawnTime;                          // (secs) time of next respawn (or despawn if GO have owner()),
        uint32      m_respawnDelayTime;                     // (secs) if 0 then current GO state no dependent from timer
        LootState   m_lootState;
        bool        m_spawnedByDefault;
        time_t      m_cooldownTime;                         // used as internal reaction delay time store (not state change reaction).
                                                            // For traps this: spell casting cooldown, for doors/buttons: reset time.
        GuidSet m_SkillupList;

        Player* m_ritualOwner;                              // used for GAMEOBJECT_TYPE_RITUAL where GO is not summoned (no owner)
        GuidSet m_unique_users;
        uint32 m_usetimes;

        typedef std::map<uint32, ObjectGuid> ChairSlotAndUser;
        ChairSlotAndUser ChairListSlots;

        uint64 m_DBTableGuid;                   ///< For new or temporary gameobjects is 0 for saved it is lowguid
        GameObjectTemplate const* m_goInfo;
        GameObjectData const* m_goData;

        int64 m_packedRotation;
        G3D::Quat m_worldRotation;
        Position m_stationaryPosition;

        ObjectGuid m_lootRecipient;
        ObjectGuid m_lootRecipientGroup;
        bool   m_manual_anim;
        bool   m_isDynActive;
        bool   m_onUse;

        bool m_actionActive = false;
        uint32 m_actionTimeCheck = 2500;
        std::vector<GameObjectActionData> const* m_actionVector;
        uint32 m_maxActionDistance = 0;

        uint16 _animKitId;

    private:
        void SwitchDoorOrButton(bool activate, bool alternative = false);
        void UpdatePackedRotation();

        //! Object distance/size - overridden from Object::_IsWithinDist. Needs to take in account proper GO size.
        bool _IsWithinDist(WorldObject const* obj, float dist2compare, bool /*is3D*/, bool ignoreObjectSize = false) const override;
        GameObjectAI* m_AI;
        
        FunctionProcessor m_Functions;
};
#endif
