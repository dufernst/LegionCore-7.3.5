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

#include "ArtifactPackets.h"
#include "BattlegroundAlteracValley.h"
#include "CellImpl.h"
#include "ChallengeMgr.h"
#include "CreatureAISelector.h"
#include "DatabaseEnv.h"
#include "GameEventMgr.h"
#include "GameObjectAI.h"
#include "GameObjectModel.h"
#include "GameObjectPackets.h"
#include "Garrison.h"
#include "GridNotifiersImpl.h"
#include "Group.h"
#include "GroupMgr.h"
#include "MapManager.h"
#include "MiscPackets.h"
#include "MMapFactory.h"
#include "MMapManager.h"
#include "ObjectMgr.h"
#include "ObjectVisitors.hpp"
#include "OutdoorPvPMgr.h"
#include "PlayerDefines.h"
#include "PoolMgr.h"
#include "QuestData.h"
#include "ScriptMgr.h"
#include "ScriptsData.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "UpdateFieldFlags.h"
#include "World.h"

GameObject::GameObject() : WorldObject(false), m_groupLootTimer(0), m_model(nullptr), m_goValue(), m_spellId(0), m_respawnTime(0), m_respawnDelayTime(300),
m_lootState(GO_NOT_READY), m_spawnedByDefault(true), m_cooldownTime(0), m_ritualOwner(nullptr), m_usetimes(0), m_DBTableGuid(0), m_goInfo(nullptr),
m_goData(nullptr), m_manual_anim(false), m_isDynActive(false), m_onUse(false), m_actionVector(nullptr), m_AI(nullptr)
{
    m_valuesCount = GAMEOBJECT_END;
    _dynamicValuesCount = GAMEOBJECT_DYNAMIC_END;
    m_objectType |= TYPEMASK_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;

    m_updateFlag = UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;

    m_IfUpdateTimer = 0;
    m_RateUpdateTimer = MAX_VISIBILITY_DISTANCE;
    m_RateUpdateWait = 0;
    m_packedRotation = 0;

    _animKitId = 0;
    m_stationaryPosition.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
    objectCountInWorld[uint8(HighGuid::GameObject)]++;
}

GameObject::~GameObject()
{
    if (m_goInfo && m_goInfo->type == GAMEOBJECT_TYPE_TRANSPORT)
        delete m_goValue.Transport.StopFrames;

    delete m_AI;
    delete m_model;

    //if (m_uint32Values)                                      // field array can be not exist if GameOBject not loaded
    //    CleanupsBeforeDelete();
    objectCountInWorld[uint8(HighGuid::GameObject)]--;
}

void GameObject::AIM_Destroy()
{
    delete m_AI;
    m_AI = nullptr;
}

bool GameObject::AIM_Initialize()
{
    AIM_Destroy();

    m_AI = FactorySelector::SelectGameObjectAI(this);
    if (!m_AI)
        return false;

    m_AI->InitializeAI();
    return true;
}

std::string GameObject::GetAIName() const
{
    return sObjectMgr->GetGameObjectTemplate(GetEntry())->AIName;
}

void GameObject::CleanupsBeforeDelete(bool finalCleanup)
{
    WorldObject::CleanupsBeforeDelete(finalCleanup);

    if (IsInWorld())
        RemoveFromWorld();

    if (m_uint32Values)                                      // field array can be not exist if GameOBject not loaded
        RemoveFromOwner();
}

void GameObject::RemoveFromOwner()
{
    ObjectGuid ownerGUID = GetOwnerGUID();
    if (ownerGUID.IsEmpty())
        return;

    if (Unit* owner = ObjectAccessor::GetUnit(*this, ownerGUID))
    {
        owner->RemoveGameObject(this, false);
        ASSERT(!GetOwnerGUID());
        return;
    }

    const char * ownerType = "creature";
    if (ownerGUID.IsPlayer())
        ownerType = "player";
    else if (ownerGUID.IsPet())
        ownerType = "pet";

    TC_LOG_FATAL(LOG_FILTER_GENERAL, "Delete GameObject (GUID: %u Entry: %u SpellId %u LinkedGO %u) that lost references to owner (GUID %u Type '%s') GO list. Crash possible later.",
        GetGUIDLow(), GetGOInfo()->entry, m_spellId, GetGOInfo()->GetLinkedGameObjectEntry(), ownerGUID.GetCounter(), ownerType);
    SetOwnerGUID(ObjectGuid::Empty);
}

void GameObject::AddToWorld()
{
    ///- Register the gameobject for guid lookup
    if (!IsInWorld())
    {
        if (m_zoneScript)
        {
            m_zoneScript->OnGameObjectCreate(this);
            m_zoneScript->OnGameObjectCreateForScript(this);
        }

        if (auto bg = GetBattleground())
            bg->OnGameObjectCreate(this);

        // Crashed in OpenPVP update, need rework
        // if (auto pvp = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(GetZoneId()))
            // pvp->OnGameObjectCreate(this);

        sObjectAccessor->AddObject(this);
        // The state can be changed after GameObject::Create but before GameObject::AddToWorld
        bool toggledState = GetGoType() == GAMEOBJECT_TYPE_CHEST ? getLootState() == GO_READY : (GetGoState() == GO_STATE_READY || IsTransport());
        if (m_model)
        {
            if (Transport* trans = ToTransport())
                trans->SetDelayedAddModelToMap();
            else
                GetMap()->InsertGameObjectModel(*m_model);
        }

        if (MaxVisible && !GetOwnerGUID())
            if (Map* mapInfo = GetMap())
                mapInfo->AddMaxVisible(this);

        EnableCollision(toggledState);
        WorldObject::AddToWorld();
    }
}

Battleground* GameObject::GetBattleground()
{
    auto map = GetMap();
    if (!map)
        return nullptr;

    auto bgMap = map->ToBgMap();
    if (!bgMap)
        return nullptr;

    return bgMap->GetBG();
}

void GameObject::RemoveFromWorld()
{
    ///- Remove the gameobject from the accessor
    if (IsInWorld())
    {
        auto map = GetMap();
        if (map && !map->IsMapUnload()) // Don`t delete if map unload, this is autoclear when delete map
        {
            if (m_zoneScript)
            {
                m_zoneScript->OnGameObjectRemove(this);
                m_zoneScript->OnGameObjectRemoveForScript(this);
            }

            if (auto bg = GetBattleground())
                bg->OnGameObjectRemove(this);

            if (MaxVisible && !GetOwnerGUID())
                map->RemoveMaxVisible(this);

            RemoveFromOwner();
            if (m_model)
                if (map->ContainsGameObjectModel(*m_model))
                    map->RemoveGameObjectModel(*m_model);
        }

        WorldObject::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}

bool GameObject::Create(ObjectGuid::LowType guidlow, uint32 name_id, Map* map, uint32 phaseMask, Position const& pos, G3D::Quat const& rotation, uint32 animprogress, GOState go_state, uint32 artKit, uint32 aid, GameObjectData const* data)
{
    ASSERT(map);
    SetMap(map);

    Relocate(pos);
    m_stationaryPosition.Relocate(pos);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "Gameobject (GUID: %u Entry: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", guidlow, name_id, pos.GetPositionX(), pos.GetPositionY());
        return false;
    }

    SetPhaseMask(phaseMask, false);
    if (data)
        SetPhaseId(data->PhaseID, false);

    SetZoneScript();
    if (m_zoneScript)
    {
        name_id = m_zoneScript->GetGameObjectEntry(guidlow, name_id);
        if (!name_id)
            return false;
    }

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(name_id);
    if (!goinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: %u Entry: %u) not created: non-existing entry in `gameobject_template`. Map: %u (X: %f Y: %f Z: %f)", guidlow, name_id, map->GetId(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        return false;
    }

    m_goInfo = goinfo;

    if (IsTransport()) // Can`t create here
        return false;

    _Create(ObjectGuid::Create<HighGuid::GameObject>(map->GetId(), goinfo->entry, guidlow));

    if (goinfo->type >= MAX_GAMEOBJECT_TYPE)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: %u Entry: %u) not created: non-existing GO type '%u' in `gameobject_template`. It will crash client if created.", guidlow, name_id, goinfo->type);
        return false;
    }

    SetWorldRotationAngles(NormalizeOrientation(GetOrientation()), 0.0f, 0.0f);
    // SetWorldRotation(rotation.x, rotation.y, rotation.z, rotation.w);

    // For most of gameobjects is (0, 0, 0, 1) quaternion, there are only some transports with not standard rotation
    G3D::Quat parentRotation(0.0f, 0.0f, 0.0f, 1.0f);
    SetParentRotation(parentRotation);

    if (data && data->personalSize > 0.0f)   
        SetObjectScale(data->personalSize);
    else
        SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, goinfo->flags);

    SetEntry(goinfo->entry);

    // set name for logs usage, doesn't affect anything ingame
    SetName(goinfo->name);

    SetDisplayId(goinfo->displayId);

    m_model = CreateModel();
    MMAP::MMapFactory::createOrGetMMapManager()->loadGameObject(goinfo->displayId, sWorld->GetDataPath());

    // GAMEOBJECT_FIELD_BYTES_1, index at 0, 1, 2 and 3
    SetGoType(GameobjectTypes(goinfo->type));
    SetGoState(go_state);
    SetGoArtKit(artKit);
    SetGoAnimProgress(animprogress);

    SetAnimKitId(aid);
   
    loot.SetSource(GetGUID());

    if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].WorldEffectID)
        m_updateFlag |= UPDATEFLAG_HAS_WORLDEFFECTID;

    if (!m_goInfo->visualQuestID)
    {
        if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID)
            SetUInt32Value(GAMEOBJECT_FIELD_STATE_WORLD_EFFECT_ID, m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].StateWorldEffectID);

        if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateVisualID)
            SetUInt32Value(GAMEOBJECT_FIELD_STATE_SPELL_VISUAL_ID, m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateVisualID);
    }

    if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellVisualID)
        SetUInt32Value(GAMEOBJECT_FIELD_SPELL_VISUAL_ID, m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellVisualID);

    if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimID)
        SetUInt32Value(GAMEOBJECT_FIELD_SPAWN_TRACKING_STATE_ANIM_ID, m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimID);

    if (m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimKitID)
        SetUInt32Value(GAMEOBJECT_FIELD_SPAWN_TRACKING_STATE_ANIM_KIT_ID, m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellStateAnimKitID);

    MaxVisible = m_goInfo->MaxVisible;

    switch (goinfo->type)
    {
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            // TODO: Get the values somehow, no longer in gameobject_template
            m_goValue.Building.Health = goinfo->destructibleBuilding.InteriorVisible ? goinfo->destructibleBuilding.InteriorVisible : 20000;
            m_goValue.Building.MaxHealth = m_goValue.Building.Health;
            SetGoAnimProgress(255);
            SetUInt32Value(GAMEOBJECT_FIELD_PARENT_ROTATION, m_goInfo->destructibleBuilding.DestructibleModelRec);
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            SetGoAnimProgress(255);
            break;
        case GAMEOBJECT_TYPE_TRAP:
            if (GetGOInfo()->trap.stealthed)
            {
                m_stealth.AddFlag(STEALTH_TRAP);
                m_stealth.AddValue(STEALTH_TRAP, 70);
            }

            if (GetGOInfo()->trap.stealthAffected)
            {
                m_invisibility.AddFlag(INVISIBILITY_TRAP);
                m_invisibility.AddValue(INVISIBILITY_TRAP, 300);
            }
            break;
        case GAMEOBJECT_TYPE_PHASEABLE_MO:
            setActive(true);    //for MAX_VISIBILITY_DISTANCE
            SetWorldObject(true);
            break;
        case GAMEOBJECT_TYPE_CAPTURE_POINT:
            SetUInt32Value(GAMEOBJECT_FIELD_SPELL_VISUAL_ID, m_goInfo->capturePoint.SpellVisual1);
            break;
        default:
            SetGoAnimProgress(animprogress);
            break;
    }

    LastUsedScriptID = GetGOInfo()->ScriptId;
    AIM_Initialize();

    m_actionVector = sObjectMgr->GetGameObjectActionData(name_id);
    if (m_actionVector && !m_actionVector->empty())
    {
        m_actionActive = true;
        for (auto const& itr : *m_actionVector)
            if (m_maxActionDistance < itr.Distance)
                m_maxActionDistance = itr.Distance;
    }

    return true;
}

GameObject* GameObject::CreateGameObject(uint32 entry, Map* map, Position const& pos, G3D::Quat const& rotation, uint32 animProgress, GOState goState, uint32 artKit)
{
    if (!sObjectMgr->GetGameObjectTemplate(entry))
        return nullptr;

    auto lowGuid = sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate();
    auto go = sObjectMgr->IsStaticTransport(entry) ? new StaticTransport : new GameObject;
    if (!go->Create(lowGuid, entry, map, PHASEMASK_NORMAL, pos, rotation, animProgress, goState, artKit))
    {
        delete go;
        return nullptr;
    }

    return go;
}

void GameObject::Update(uint32 diff)
{
    if (m_isUpdate)
        return;

    m_isUpdate = true;

    if (!AI())
    {
        if (!AIM_Initialize())
            TC_LOG_ERROR(LOG_FILTER_GENERAL, "Could not initialize GameObjectAI");
    } else
        AI()->UpdateAI(diff);

    // if (!isInCombat()) // Update creature if need
    {
        // if (m_RateUpdateWait <= diff)
        // {
            // m_RateUpdateTimer = MAX_VISIBILITY_DISTANCE;
            // m_RateUpdateWait = 15 * IN_MILLISECONDS;
        // }
        // else
            // m_RateUpdateWait -= diff;

        m_Functions.Update(diff);
        
        uint32 updateDiff = m_RateUpdateTimer * 5;
        if (updateDiff < sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE))
            updateDiff = sWorld->getIntConfig(CONFIG_INTERVAL_OBJECT_UPDATE);

        m_IfUpdateTimer += diff;
        if (m_IfUpdateTimer > updateDiff)
        {
            diff = m_IfUpdateTimer;
            m_IfUpdateTimer = 0;
        }
        else
        {
            m_isUpdate = false;
            return;
        }
    }

    if (m_actionTimeCheck <= diff)
    {
        m_actionTimeCheck = 2500;
        GameObjectAction();
    }
    else
        m_actionTimeCheck -= diff;


    switch (m_lootState)
    {
        case GO_NOT_READY:
        {
            switch (GetGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                {
                    // Arming Time for GAMEOBJECT_TYPE_TRAP (6)
                    GameObjectTemplate const* goInfo = GetGOInfo();
                    if (goInfo->trap.charges == 2)
                        m_cooldownTime = time(nullptr) + 10;   // Hardcoded tooltip value
                    else if (Unit* owner = GetOwner())
                    {
                        if (owner->isInCombat())
                            m_cooldownTime = time(nullptr) + 1;
                    }
                    m_lootState = GO_READY;
                    break;
                }
                case GAMEOBJECT_TYPE_FISHINGNODE:
                {
                    // fishing code (bobber ready)
                    if (time(nullptr) > m_respawnTime - FISHING_BOBBER_READY_TIME)
                    {
                        // splash bobber (bobber ready now)
                        Unit* caster = GetOwner();
                        if (caster && caster->IsPlayer())
                        {
                            SetGoState(GO_STATE_ACTIVE);
                            SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NODESPAWN);

                            UpdateData udata(caster->GetMapId());
                            WorldPacket packet;
                            BuildValuesUpdateBlockForPlayer(&udata, caster->ToPlayer());
                            if (udata.BuildPacket(&packet))
                                caster->ToPlayer()->GetSession()->SendPacket(&packet);

                            SendCustomAnim(0);
                        }

                        m_lootState = GO_READY;                 // can be successfully open with some chance
                    }
                    m_isUpdate = false;
                    return;
                }
                default:
                    m_lootState = GO_READY;                         // for other GOis same switched without delay to GO_READY
                    break;
            }
            // NO BREAK for switch (m_lootState)
        }
        case GO_READY:
        {
            if (m_respawnTime > 0)                          // timer on
            {
                time_t now = time(nullptr);
                if (m_respawnTime <= now)            // timer expired
                {
                    ObjectGuid dbtableHighGuid = ObjectGuid::Create<HighGuid::GameObject>(GetMapId(), GetEntry(), m_DBTableGuid);
                    time_t linkedRespawntime = GetMap()->GetLinkedRespawnTime(dbtableHighGuid);
                    if (linkedRespawntime)             // Can't respawn, the master is dead
                    {
                        ObjectGuid targetGuid = sObjectMgr->GetLinkedRespawnGuid(dbtableHighGuid);
                        if (targetGuid == dbtableHighGuid) // if linking self, never respawn (check delayed to next day)
                            SetRespawnTime(DAY);
                        else
                            m_respawnTime = (now > linkedRespawntime ? now : linkedRespawntime)+urand(5, MINUTE); // else copy time from master and add a little
                        SaveRespawnTime(); // also save to DB immediately
                        m_isUpdate = false;
                        return;
                    }

                    m_respawnTime = 0;
                    m_SkillupList.clear();
                    m_usetimes = 0;

                    switch (GetGoType())
                    {
                        case GAMEOBJECT_TYPE_FISHINGNODE:   //  can't fish now
                        {
                            Unit* caster = GetOwner();
                            if (caster && caster->IsPlayer())
                            {
                                caster->ToPlayer()->RemoveGameObject(this, false);
                                caster->ToPlayer()->SendDirectMessage(WorldPackets::GameObject::FishNotHooked().Write());
                            }
                            // can be delete
                            m_lootState = GO_JUST_DEACTIVATED;
                            m_isUpdate = false;
                            return;
                        }
                        case GAMEOBJECT_TYPE_DOOR:
                        case GAMEOBJECT_TYPE_BUTTON:
                            //we need to open doors if they are closed (add there another condition if this code breaks some usage, but it need to be here for battlegrounds)
                            if (GetGoState() != GO_STATE_READY)
                                ResetDoorOrButton();
                            //flags in AB are type_button and we need to add them here so no break!
                        default:
                            if (!m_spawnedByDefault)        // despawn timer
                            {
                                                            // can be despawned or destroyed
                                SetLootState(GO_JUST_DEACTIVATED);
                                m_isUpdate = false;
                                return;
                            }
                                                            // respawn timer
                            uint32 poolid = GetDBTableGUIDLow() ? sPoolMgr->IsPartOfAPool<GameObject>(GetDBTableGUIDLow()) : 0;
                            if (poolid)
                                sPoolMgr->UpdatePool<GameObject>(poolid, GetDBTableGUIDLow());
                            else
                                GetMap()->AddToMap(this);
                            break;
                    }
                }
            }

            if (isSpawned())
            {
                // traps can have time and can not have
                GameObjectTemplate const* goInfo = GetGOInfo();
                if (goInfo->type == GAMEOBJECT_TYPE_TRAP)
                {
                    if (m_cooldownTime >= time(nullptr))
                    {
                        m_isUpdate = false;
                        return;
                    }

                    // Type 2 - Bomb (will go away after casting it's spell)
                    if (goInfo->trap.charges == 2)
                    {
                        if (goInfo->trap.spell)
                            CastSpell(nullptr, goInfo->trap.spell);  // FIXME: null target won't work for target type 1
                        SetLootState(GO_JUST_DEACTIVATED);
                        break;
                    }
                    // Type 0 and 1 - trap (type 0 will not get removed after casting a spell)
                    Unit* owner = GetOwner();
                    Unit* ok = nullptr;                            // pointer to appropriate target if found any

                    bool IsBattlegroundTrap = false;
                    //FIXME: this is activation radius (in different casting radius that must be selected from spell data)
                    //TODO: move activated state code (cast itself) to GO_ACTIVATED, in this place only check activating and set state
                    float radius = static_cast<float>(goInfo->trap.radius) / 3 * 2;
                    if (!radius)
                    {
                        if (goInfo->trap.cooldown != 3)            // cast in other case (at some triggering/linked go/etc explicit call)
                        {
                            m_isUpdate = false;
                            return;
                        }
                        if (m_respawnTime > 0)
                            break;

                        radius = static_cast<float>(goInfo->trap.cooldown);       // battlegrounds gameobjects has data2 == 0 && data5 == 3
                        IsBattlegroundTrap = true;

                        if (!radius)
                        {
                            m_isUpdate = false;
                            return;
                        }
                    }

                    // Note: this hack with search required until GO casting not implemented
                    // search unfriendly creature
                    if (owner)                    // hunter trap
                    {
                        Trinity::AnyUnfriendlyNoTotemUnitInObjectRangeCheck checker(this, owner, radius);
                        Trinity::UnitSearcher<Trinity::AnyUnfriendlyNoTotemUnitInObjectRangeCheck> searcher(this, ok, checker);
                        Trinity::VisitNearbyGridObject(this, radius, searcher);
                        if (!ok) 
                            Trinity::VisitNearbyWorldObject(this, radius, searcher);
                    }
                    else                                        // environmental trap
                    {
                        // environmental damage spells already have around enemies targeting but this not help in case not existed GO casting support
                        // affect only players
                        Player* player = nullptr;
                        Trinity::AnyPlayerInObjectRangeCheck checker(this, radius);
                        Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, player, checker);
                        Trinity::VisitNearbyWorldObject(this, radius, searcher);
                        ok = player;
                    }

                    if (ok)
                    {
                        if (Player* tmpPlayer = ok->ToPlayer())
                            if (tmpPlayer->IsSpectator())
                            {
                                m_isUpdate = false;
                                return;
                            }

                        Unit* caster = owner ? owner : ok;

                        int32 targetVis = ok->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL);
                        int32 casterVis = caster->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_DUEL);

                        if (targetVis == 1 || casterVis == 1 || targetVis > 1 && casterVis > 1 && targetVis != casterVis)
                        {
                            m_isUpdate = false;
                            return;
                        }

                        // some traps do not have spell but should be triggered
                        if (goInfo->trap.spell)
                            CastSpell(ok, goInfo->trap.spell);

                        m_cooldownTime = time(nullptr) + (goInfo->trap.cooldown ? goInfo->trap.cooldown :  uint32(4));   // template or 4 seconds

                        if (goInfo->trap.charges == 1)
                            SetLootState(GO_JUST_DEACTIVATED);

                        if (IsBattlegroundTrap && ok->IsPlayer())
                        {
                            //Battleground gameobjects case
                            if (ok->ToPlayer()->InBattleground())
                                if (Battleground* bg = ok->ToPlayer()->GetBattleground())
                                    bg->HandleTriggerBuff(GetGUID());
                        }
                    }
                }
                else if (uint32 max_charges = goInfo->GetCharges())
                {
                    if (m_usetimes >= max_charges)
                    {
                        m_usetimes = 0;
                        SetLootState(GO_JUST_DEACTIVATED);      // can be despawned or destroyed
                    }
                }
            }

            break;
        }
        case GO_ACTIVATED:
        {
            switch (GetGoType())
            {
                case GAMEOBJECT_TYPE_DOOR:
                case GAMEOBJECT_TYPE_BUTTON:
                    if (GetGOInfo()->GetAutoCloseTime() && m_cooldownTime < time(nullptr))
                        ResetDoorOrButton();
                    break;
                case GAMEOBJECT_TYPE_GOOBER:
                    if (m_cooldownTime < time(nullptr))
                    {
                        RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);

                        SetLootState(GO_JUST_DEACTIVATED);
                        m_cooldownTime = 0;
                    }
                    break;
                case GAMEOBJECT_TYPE_CHEST:
                    if (m_groupLootTimer)
                    {
                        if (m_groupLootTimer <= diff)
                        {
                            if (Group* group = sGroupMgr->GetGroupByGUID(lootingGroupLowGUID))
                                group->EndRoll(&loot);
                            m_groupLootTimer = 0;
                            lootingGroupLowGUID.Clear();
                        }
                        else 
                            m_groupLootTimer -= diff;
                    }
                default:
                    break;
            }
            break;
        }
        case GO_JUST_DEACTIVATED:
        {
            //if Gameobject should cast spell, then this, but some GOs (type = 10) should be destroyed
            if (GetGoType() == GAMEOBJECT_TYPE_GOOBER)
            {
                if (uint32 spellId = GetGOInfo()->goober.spell)
                {
                    for (auto m_unique_user : m_unique_users)
                        if (Player* owner = ObjectAccessor::GetPlayer(*this, m_unique_user))
                            owner->CastSpell(owner, spellId, false);

                    m_unique_users.clear();
                    m_usetimes = 0;
                }

                //any return here in case battleground traps
                if (GetGOInfo()->flags & GO_FLAG_NODESPAWN)
                {
                    // Test: This falg on Mop perfome despoan go with some animation
                    // Should despawn some go with animations.
                    if (GetGOInfo()->IsDespawnAtAction() && m_respawnDelayTime > 0)
                    {
                        SendObjectDeSpawnAnim(GetGUID());
                        m_respawnTime = time(nullptr) + m_respawnDelayTime;
                        UpdateObjectVisibility();
                        loot.clear();
                    }
                    //CD for using. not need as we set this state after CD launched. So just set ready state.
                    SetGoState(GO_STATE_READY);
                    SetLootState(GO_READY);
                    m_isUpdate = false;
                    return;
                }

                SetGoState(GO_STATE_READY);
            }

            loot.clear();

            //! If this is summoned by a spell with ie. SPELL_EFFECT_SUMMON_OBJECT_WILD, with or without owner, we check respawn criteria based on spell
            //! The GetOwnerGUID() check is mostly for compatibility with hacky scripts - 99% of the time summoning should be done trough spells.
            //! If object is tmp spawn no need collect him all time in depawn mode.
            if (GetSpellId() || GetOwnerGUID() || !m_spawnedByDefault)
            {
                SetRespawnTime(0);
                Delete();
                m_isUpdate = false;
                return;
            }

            SetLootState(GO_READY);

            //burning flags in some battlegrounds, if you find better condition, just add it
            if (GetGOInfo()->IsDespawnAtAction() || GetGoAnimProgress() > 0)
            {
                SendObjectDeSpawnAnim(GetGUID());
                //reset flags
                SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, GetGOInfo()->flags);
            }

            //! depr. see above
            if (!m_spawnedByDefault)
            {
                m_respawnTime = 0;
                UpdateObjectVisibility();
                m_isUpdate = false;
                return;
            }

            int32 _respawnDelay = m_respawnDelayTime;
            if (sWorld->getBoolConfig(CONFIG_RESPAWN_FROM_PLAYER_ENABLED))
            {
                if (_respawnDelay <= 600 && GetGOData() && !GetGOInfo()->GetXpLevel() && !GetMap()->Instanceable()) // квестовые ГО и прочий шлак
                {
                    uint32 targetCount = GetPlayerFromArea(GetGOData()->areaId);
                    if (targetCount)
                    {
                        if (targetCount >= sWorld->getIntConfig(CONFIG_RESPAWN_FROM_PLAYER_COUNT))
                            _respawnDelay /= targetCount; // грубый рассчет, конечно, но лучше уж..

                        if (_respawnDelay < 10)
                            _respawnDelay = urand(10, 15);
                    }
                }
            }
            m_respawnTime = time(nullptr) + _respawnDelay;

            // if option not set then object will be saved at grid unload
            if (sWorld->getBoolConfig(CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY))
                SaveRespawnTime();

            UpdateObjectVisibility();
            break;
        }
    }
    m_isUpdate = false;
}

void GameObject::Refresh()
{
    // not refresh despawned not casted GO (despawned casted GO destroyed in all cases anyway)
    if (m_respawnTime > 0 && m_spawnedByDefault)
        return;

    if (isSpawned())
        GetMap()->AddToMap(this);
}

void GameObject::AddUniqueUse(Player* player)
{
    AddUse();
    m_unique_users.insert(player->GetGUID());
}

void GameObject::AddUse()
{
    ++m_usetimes;
}

uint32 GameObject::GetUseCount() const
{
    return m_usetimes;
}

uint32 GameObject::GetUniqueUseCount() const
{
    return m_unique_users.size();
}

void GameObject::Delete()
{
    if(!this)
        return;

    if (auto mapInfo = GetMap())
        if (mapInfo->IsMapUnload())
            return;

    SetLootState(GO_NOT_READY);
    RemoveFromOwner();

    SendObjectDeSpawnAnim(GetGUID());

    SetGoState(GO_STATE_READY);
    SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, GetGOInfo()->flags);

    if (auto poolid = GetDBTableGUIDLow() ? sPoolMgr->IsPartOfAPool<GameObject>(GetDBTableGUIDLow()) : 0)
        sPoolMgr->UpdatePool<GameObject>(poolid, GetDBTableGUIDLow());
    else
        AddObjectToRemoveList();
}

void GameObject::getFishLoot(Loot* fishloot, Player* loot_owner)
{
    fishloot->clear();

    uint32 zone, subzone;
    GetZoneAndAreaId(zone, subzone);
    fishloot->objGuid = GetGUID();

    // if subzone loot exist use it
    if (!fishloot->FillLoot(subzone, LootTemplates_Fishing, loot_owner, true, true, this))
        // else use zone loot (must exist in like case)
        fishloot->FillLoot(zone, LootTemplates_Fishing, loot_owner, true, false, this);
}

void GameObject::SaveToDB()
{
    // this should only be used when the gameobject has already been loaded
    // preferably after adding to map, because mapid may not be valid otherwise
    GameObjectData const* data = sObjectMgr->GetGOData(m_DBTableGuid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "GameObject::SaveToDB failed, cannot get gameobject data!");
        return;
    }

    SaveToDB(GetMapId(), data->spawnMask, data->phaseMask);
}

void GameObject::SaveToDB(uint32 mapid, uint64 spawnMask, uint32 phaseMask)
{
    auto goI = GetGOInfo();
    if (!goI)
        return;

    if (!m_DBTableGuid)
        m_DBTableGuid = GetGUID().GetGUIDLow();
    // update in loaded data (changing data only in this place)
    GameObjectData& data = sObjectMgr->NewGOData(m_DBTableGuid);

    uint32 zoneId = 0;
    uint32 areaId = 0;
    sMapMgr->GetZoneAndAreaId(zoneId, areaId, mapid, GetPositionX(), GetPositionY(), GetPositionZ());

    // data->guid = guid must not be updated at save
    data.id = GetEntry();
    data.mapid = mapid;
    data.zoneId = zoneId;
    data.areaId = areaId;
    data.phaseMask = phaseMask;
    data.posX = GetPositionX();
    data.posY = GetPositionY();
    data.posZ = GetPositionZ();
    data.orientation = GetOrientation();
    data.rotation = m_worldRotation;
    data.spawntimesecs = m_spawnedByDefault ? m_respawnDelayTime : -static_cast<int32>(m_respawnDelayTime);
    data.animprogress = GetGoAnimProgress();
    data.go_state = GetGoState();
    data.spawnMask = spawnMask;
    data.artKit = GetGoArtKit();
    data.MaxVisible = m_goInfo ? m_goInfo->MaxVisible : false;

    // Update in DB
    SQLTransaction trans = WorldDatabase.BeginTransaction();

    uint8 index = 0;

    PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT);
    stmt->setUInt64(0, m_DBTableGuid);
    trans->Append(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_INS_GAMEOBJECT);
    stmt->setUInt64(index++, m_DBTableGuid);
    stmt->setUInt32(index++, GetEntry());
    stmt->setUInt16(index++, uint16(mapid));
    stmt->setUInt16(index++, zoneId);
    stmt->setUInt16(index++, areaId);
    stmt->setUInt64(index++, spawnMask);
    stmt->setUInt16(index++, uint16(GetPhaseMask()));
    stmt->setFloat(index++, GetPositionX());
    stmt->setFloat(index++, GetPositionY());
    stmt->setFloat(index++, GetPositionZ());
    stmt->setFloat(index++, GetOrientation());
    stmt->setFloat(index++, m_worldRotation.x);
    stmt->setFloat(index++, m_worldRotation.y);
    stmt->setFloat(index++, m_worldRotation.z);
    stmt->setFloat(index++, m_worldRotation.w);
    stmt->setInt32(index++, int32(m_respawnDelayTime));
    stmt->setUInt8(index++, GetGoAnimProgress());
    stmt->setUInt8(index++, uint8(GetGoState()));
    stmt->setUInt8(index++, uint8(isActiveObject()));
    stmt->setFloat(index++, data.personalSize);
    trans->Append(stmt);

    WorldDatabase.CommitTransaction(trans);
}

bool GameObject::LoadFromDB(ObjectGuid::LowType guid, Map* map)
{
    return LoadGameObjectFromDB(guid, map, false);
}

bool GameObject::LoadGameObjectFromDB(ObjectGuid::LowType guid, Map* map, bool addToMap)
{
    GameObjectData const* data = sObjectMgr->GetGOData(guid);
    if (!data)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Gameobject (GUID: %u) not found in table `gameobject`, can't load. ", guid);
        return false;
    }

    uint32 entry = data->id;

    m_DBTableGuid = guid;
    if (map->GetInstanceId() != 0)
        guid = sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate();

    if (!Create(guid, entry, map, data->phaseMask, Position(data->posX, data->posY, data->posZ, data->orientation), data->rotation, data->animprogress, static_cast<GOState>(data->go_state), data->artKit, data->AiID, data))
        return false;

    SetPhaseId(data->PhaseID, false);

    if (data->spawntimesecs >= 0)
    {
        m_spawnedByDefault = true;

        if (!GetGOInfo()->GetDespawnPossibility() && !GetGOInfo()->IsDespawnAtAction())
        {
            SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NODESPAWN);
            m_respawnDelayTime = 0;
            m_respawnTime = 0;
        }
        else
        {
            m_respawnDelayTime = data->spawntimesecs;
            m_respawnTime = GetMap()->GetGORespawnTime(m_DBTableGuid);

            // ready to respawn
            if (m_respawnTime && m_respawnTime <= time(nullptr))
            {
                m_respawnTime = 0;
                GetMap()->RemoveGORespawnTime(m_DBTableGuid);
            }
        }
    }
    else
    {
        m_spawnedByDefault = false;
        m_respawnDelayTime = -data->spawntimesecs;
        m_respawnTime = 0;
    }

    m_goData = data;

    setDynActive(data->isActive);

    loot.SetGUID(ObjectGuid::Create<HighGuid::LootObject>(data->mapid, data->id, sObjectMgr->GetGenerator<HighGuid::LootObject>()->Generate()));

    if (addToMap && !GetMap()->AddToMap(this))
        return false;

    if (data->gameEvent && m_DBTableGuid != guid)
        sGameEventMgr->mGameEventGameobjectSpawns[data->gameEvent].push_back(GetGUID());

    return true;
}

void GameObject::DeleteFromDB()
{
    GetMap()->RemoveGORespawnTime(m_DBTableGuid);
    sObjectMgr->DeleteGOData(m_DBTableGuid);

    auto stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_GAMEOBJECT);
    stmt->setUInt64(0, m_DBTableGuid);
    WorldDatabase.Execute(stmt);

    stmt = WorldDatabase.GetPreparedStatement(WORLD_DEL_EVENT_GAMEOBJECT);
    stmt->setUInt64(0, m_DBTableGuid);
    WorldDatabase.Execute(stmt);
}

void GameObject::SetOwnerGUID(ObjectGuid owner)
{
    // Owner already found and different than expected owner - remove object from old owner
    if (owner && GetOwnerGUID() && GetOwnerGUID() != owner)
    {
        ASSERT(false);
    }
    m_spawnedByDefault = false; // all object with owner is despawned after delay
    SetGuidValue(GAMEOBJECT_FIELD_CREATED_BY, owner);
}

ObjectGuid GameObject::GetOwnerGUID() const
{
    return GetGuidValue(GAMEOBJECT_FIELD_CREATED_BY);
}

GameObject* GameObject::GetGameObject(WorldObject& object, ObjectGuid guid)
{
    return object.GetMap()->GetGameObject(guid);
}

GameObjectTemplate const* GameObject::GetGOInfo() const
{
    return m_goInfo;
}

GameObjectData const* GameObject::GetGOData() const
{
    return m_goData;
}

GameObjectValue const* GameObject::GetGOValue() const
{
    return &m_goValue;
}

uint64 GameObject::GetDBTableGUIDLow() const
{
    return m_DBTableGuid;
}

void GameObject::Say(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterSay(textId, language, TargetGuid);
}

void GameObject::Yell(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterYell(textId, language, TargetGuid);
}

void GameObject::TextEmote(int32 textId, ObjectGuid TargetGuid)
{
    MonsterTextEmote(textId, TargetGuid);
}

void GameObject::Whisper(int32 textId, ObjectGuid receiver)
{
    MonsterWhisper(textId, receiver);
}

void GameObject::YellToZone(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    MonsterYellToZone(textId, language, TargetGuid);
}

GameobjectTypes GameObject::GetGoType() const
{
    return GameobjectTypes(GetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_1_TYPE));
}

void GameObject::SetGoType(GameobjectTypes type)
{
    SetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_1_TYPE, type);
}

GOState GameObject::GetGoState() const
{
    return GOState(GetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOJBECT_BYTES_0_STATE));
}

uint8 GameObject::GetGoArtKit() const
{
    return GetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_2_ART_KIT);
}

uint8 GameObject::GetGoAnimProgress() const
{
    return GetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_3_ANIM_PROGRESS);
}

void GameObject::SetGoAnimProgress(uint8 animprogress)
{
    SetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_3_ANIM_PROGRESS, animprogress);
}

LootState GameObject::getLootState() const
{
    return m_lootState;
}

uint16 GameObject::GetAIAnimKitId() const
{
    return _animKitId;
}

uint32 GameObject::GetScriptId() const
{
    return GetGOInfo()->ScriptId;
}

GameObjectAI* GameObject::AI() const
{
    return m_AI;
}

uint32 GameObject::GetDisplayId() const
{
    return GetUInt32Value(GAMEOBJECT_FIELD_DISPLAY_ID);
}

bool GameObject::isDynActive() const
{
    return m_isDynActive;
}

void GameObject::setDynActive(bool active)
{
    m_isDynActive = active;
}

/*********************************************************/
/***                    QUEST SYSTEM                   ***/
/*********************************************************/
bool GameObject::hasQuest(uint32 quest_id) const
{
    auto qr = sQuestDataStore->GetGOQuestRelationBounds(GetEntry());
    for (auto itr = qr.first; itr != qr.second; ++itr)
        if (itr->second == quest_id)
            return true;
    return false;
}

bool GameObject::hasInvolvedQuest(uint32 quest_id) const
{
    auto qir = sQuestDataStore->GetGOQuestInvolvedRelationBounds(GetEntry());
    for (auto itr = qir.first; itr != qir.second; ++itr)
        if (itr->second == quest_id)
            return true;
    return false;
}

bool GameObject::IsTransport() const
{
    // If something is marked as a transport, don't transmit an out of range packet for it.
    GameObjectTemplate const* gInfo = GetGOInfo();
    if (!gInfo)
        return false;

    return gInfo->type == GAMEOBJECT_TYPE_TRANSPORT || gInfo->type == GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT;
}

// is Dynamic transport = non-stop Transport
bool GameObject::IsDynTransport() const
{
    // If something is marked as a transport, don't transmit an out of range packet for it.
    GameObjectTemplate const* gInfo = GetGOInfo();
    if (!gInfo)
        return false;

    return gInfo->type == GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT || (gInfo->type == GAMEOBJECT_TYPE_TRANSPORT && m_goValue.Transport.StopFrames->empty());
}

bool GameObject::IsDestructibleBuilding() const
{
    GameObjectTemplate const* gInfo = GetGOInfo();
    if (!gInfo) 
        return false;
    return gInfo->type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING;
}

Unit* GameObject::GetOwner() const
{
    return ObjectAccessor::GetUnit(*this, GetOwnerGUID());
}

void GameObject::SetSpellId(uint32 id)
{
    m_spawnedByDefault = false; // all summoned object is despawned after delay
    m_spellId = id;
}

uint32 GameObject::GetSpellId() const
{
    return m_spellId;
}

time_t GameObject::GetRespawnTime() const
{
    return m_respawnTime;
}

time_t GameObject::GetRespawnTimeEx() const
{
    time_t now = time(nullptr);
    if (m_respawnTime > now)
        return m_respawnTime;
    return now;
}

void GameObject::SetRespawnTime(int32 respawn)
{
    m_respawnTime = respawn > 0 ? time(nullptr) + respawn : 0;
    m_respawnDelayTime = respawn > 0 ? respawn : 0;
    if (respawn)
        UpdateObjectVisibility();
}

void GameObject::SetRespawnDelayTime(int32 respawn)
{
    m_respawnDelayTime = respawn > 0 ? respawn : 0;
}

void GameObject::SaveRespawnTime()
{
    if (m_goData && m_goData->dbData && m_respawnTime > time(nullptr) && m_spawnedByDefault)
        GetMap()->SaveGORespawnTime(m_DBTableGuid, m_respawnTime);
}

void GameObject::Respawn()
{
    if (m_spawnedByDefault && m_respawnTime > 0)
    {
        m_respawnTime = time(nullptr);
        GetMap()->RemoveGORespawnTime(m_DBTableGuid);
    }
}

bool GameObject::isSpawned() const
{
    return m_respawnDelayTime == 0 || m_respawnTime > 0 && !m_spawnedByDefault || m_respawnTime == 0 && m_spawnedByDefault;
}

bool GameObject::isSpawnedByDefault() const
{
    return m_spawnedByDefault;
}

void GameObject::SetSpawnedByDefault(bool b)
{
    m_spawnedByDefault = b;
}

uint32 GameObject::GetRespawnDelay() const
{
    return m_respawnDelayTime;
}

bool GameObject::IsAlwaysVisibleFor(WorldObject const* seer) const
{
    if (WorldObject::IsAlwaysVisibleFor(seer))
        return true;

    if (IsDestructibleBuilding())
        return true;

    if (IsTransport())
        return true;

    if (!seer)
        return false;

    if (GetGoType() == GAMEOBJECT_TYPE_TRAP)
    {
        GameObjectTemplate const* goInfo = GetGOInfo();
        if (goInfo->trap.charges == 2)
            return true;
    }

    // Always seen by owner and friendly units
    ObjectGuid guid = GetOwnerGUID();
    if (!guid.IsEmpty())
    {
        if (seer->GetGUID() == guid)
            return true;

        Unit* owner = GetOwner();
        if (owner && seer->IsUnit() && owner->IsFriendlyTo(seer->ToUnit()))
            return true;
    }

    return false;
}

bool GameObject::IsNeverVisible(WorldObject const* obj) const
{
    auto last = m_lastUser.find(obj->GetGUID());
    if (last != m_lastUser.end() && time(nullptr) < last->second)
        return true;

    auto data = GetGOData();
    if (data && data->spawnMask == 256)  // only challenge go check. for hiding them after start.
        if (GetMap()->GetSpawnMode() == DIFFICULTY_HEROIC)
            return true;

    return WorldObject::IsNeverVisible();
}

uint8 GameObject::getLevelForTarget(WorldObject const* target) const
{
    if (Unit* owner = GetOwner())
        return owner->getLevelForTarget(target);

    return 1;
}

bool GameObject::IsInvisibleDueToDespawn() const
{
    if (WorldObject::IsInvisibleDueToDespawn())
        return true;

    // Despawned
    if (!isSpawned())
        return true;

    return false;
}

bool GameObject::ActivateToQuest(Player* target) const
{
    if (target->isGameMaster())
        return true;

    if (target->HasQuestForGO(GetEntry()))
        return true;

    for (auto questItem : GetGOInfo()->QuestItems)
    {
        if (!questItem)
            break;
        if (target->HasQuestForItem(questItem))
            return true;
    }

    if (!sQuestDataStore->IsGameObjectForQuests(GetEntry()))
        return false;

    switch (GetGoType())
    {
        case GAMEOBJECT_TYPE_CHEST: // scan GO chest with loot including quest items
        {
            if (LootTemplates_Gameobject.HaveQuestLootForPlayer(GetEntry(), target))
            {
                //TODO: fix this hack
                //look for battlegroundAV for some objects which are only activated after mine gots captured by own team
                if (GetEntry() == BG_AV_OBJECTID_MINE_N || GetEntry() == BG_AV_OBJECTID_MINE_S)
                    if (auto bg = target->GetBattleground())
                        if (bg->GetTypeID(true) == MS::Battlegrounds::BattlegroundTypeId::BattlegroundAlteracValley && !dynamic_cast<BattlegroundAlteracValley*>(bg)->PlayerCanDoMineQuest(GetEntry(), target->GetTeam()))
                            return false;
                return true;
            }
            break;
        }
        case GAMEOBJECT_TYPE_GENERIC:
            if (GetGOInfo()->generic.questID == -1 || target->GetQuestStatus(GetGOInfo()->generic.questID) == QUEST_STATUS_INCOMPLETE)
                return true;
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            if (GetGOInfo()->goober.questID == -1 || target->GetQuestStatus(GetGOInfo()->goober.questID) == QUEST_STATUS_INCOMPLETE)
                return true;
            break;
        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            if (GetGOInfo()->spellFocus.questID == -1 || target->GetQuestStatus(GetGOInfo()->spellFocus.questID) == QUEST_STATUS_INCOMPLETE)
                return true;
            break;
        default:
            break;
    }

    return false;
}

void GameObject::TriggeringLinkedGameObject(uint32 trapEntry, Unit* target)
{
    GameObjectTemplate const* trapInfo = sObjectMgr->GetGameObjectTemplate(trapEntry);
    if (!trapInfo || trapInfo->type != GAMEOBJECT_TYPE_TRAP)
        return;

    SpellInfo const* trapSpell = sSpellMgr->GetSpellInfo(trapInfo->trap.spell);
    if (!trapSpell)                                          // checked at load already
        return;

    auto range = float(target->GetSpellMaxRangeForTarget(GetOwner(), trapSpell));

    // search nearest linked GO
    GameObject* trapGO = nullptr;
    {
        // using original GO distance
        CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
        Cell cell(p);

        Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*target, trapEntry, range);
        Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> checker(this, trapGO, go_check);

        cell.Visit(p, makeGridVisitor(checker), *GetMap(), *target, range);
    }

    // found correct GO
    if (trapGO)
        trapGO->CastSpell(target, trapInfo->trap.spell);
}

GameObject* GameObject::LookupFishingHoleAround(float range)
{
    GameObject* ok = nullptr;

    CellCoord p(Trinity::ComputeCellCoord(GetPositionX(), GetPositionY()));
    Cell cell(p);
    Trinity::NearestGameObjectFishingHole u_check(*this, range);
    Trinity::GameObjectSearcher<Trinity::NearestGameObjectFishingHole> checker(this, ok, u_check);

    cell.Visit(p, makeGridVisitor(checker), *GetMap(), *this, range);

    return ok;
}

void GameObject::ResetDoorOrButton()
{
    if (m_lootState == GO_READY || m_lootState == GO_JUST_DEACTIVATED)
        return;

    SwitchDoorOrButton(false);
    SetLootState(GO_JUST_DEACTIVATED);
    m_cooldownTime = 0;
}

void GameObject::UseDoorOrButton(uint32 time_to_restore, bool alternative /* = false */, Unit* user /*=NULL*/)
{
    if (m_lootState != GO_READY)
        return;

    if (!time_to_restore)
        time_to_restore = GetGOInfo()->GetAutoCloseTime();

    SwitchDoorOrButton(true, alternative);
    SetLootState(GO_ACTIVATED, user);

    if (GetGoType() == GAMEOBJECT_TYPE_DOOR)
        AI()->OnStateChanged(GO_ACTIVATED, user);

    m_cooldownTime = time(nullptr) + time_to_restore;
}

void GameObject::SetGoArtKit(uint8 kit)
{
    if (!sGameObjectArtKitStore[kit])
        return;

    SetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOBJECT_BYTES_2_ART_KIT, kit);
    if (auto data = const_cast<GameObjectData*>(sObjectMgr->GetGOData(m_DBTableGuid)))
        data->artKit = kit;
}

void GameObject::SetGoArtKit(uint8 artkit, GameObject* go, ObjectGuid::LowType lowguid)
{
    if (!sGameObjectArtKitStore[artkit])
        return;

    const GameObjectData* data = nullptr;
    if (go)
    {
        go->SetGoArtKit(artkit);
        data = go->GetGOData();
    }
    else if (lowguid)
        data = sObjectMgr->GetGOData(lowguid);

    if (data)
        const_cast<GameObjectData*>(data)->artKit = artkit;
}

void GameObject::SwitchDoorOrButton(bool activate, bool alternative /* = false */)
{
    if (activate)
        SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
    else
        RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);

    if (GetGoState() == GO_STATE_READY)                      //if closed -> open
        SetGoState(alternative ? GO_STATE_ACTIVE_ALTERNATIVE : GO_STATE_ACTIVE);
    else                                                    //if open -> close
        SetGoState(GO_STATE_READY);
}

//! Analog SwitchDoorOrButton but with force set state at enable/disable state
void GameObject::EnableOrDisableGo(bool enable, bool alternative)
{
    if (enable)
    {
        SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
        SetGoState(alternative ? GO_STATE_ACTIVE_ALTERNATIVE : GO_STATE_ACTIVE);    //if closed -> open
    }
    else
    {
        RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
        SetGoState(GO_STATE_READY);                                                 //if open -> close
    }
}

uint32 GameObject::GetVignetteId() const
{
    return m_goInfo ? m_goInfo->GetVignetteId() : 0;
}

void GameObject::Use(Unit* user)
{
    // by default spell caster is user
    Unit* spellCaster = user;
    uint32 spellId = 0;
    bool triggered = false;

    if (Player* playerUser = user->ToPlayer())
    {
        if (sScriptMgr->OnGossipHello(playerUser, this))
            return;

        if (AI()->GossipUse(playerUser))
            return;

        // We do not allow players to use the hidden objects.
        if (!isSpawned() || HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE | GO_FLAG_NOT_SELECTABLE))
            return;
    }

    // If cooldown data present in template
    if (uint32 cooldown = GetGOInfo()->GetCooldown())
    {
        if (m_cooldownTime > sWorld->GetGameTime())
            return;

        m_cooldownTime = sWorld->GetGameTime() + cooldown;
    }

    switch (GetGoType())
    {
        case GAMEOBJECT_TYPE_DOOR:                          //0
        case GAMEOBJECT_TYPE_BUTTON:                        //1
            //doors/buttons never really despawn, only reset to default state/flags
            UseDoorOrButton(0, false, user);

            if (Player* player = user->ToPlayer())
                if (Battleground* bg = player->GetBattleground())
                    bg->EventPlayerUsedGO(player, this);

            // activate script
            GetMap()->ScriptsStart(sGameObjectScripts, GetDBTableGUIDLow(), spellCaster, this);
            return;
        case GAMEOBJECT_TYPE_QUESTGIVER:                    //2
        {
            if (!user->IsPlayer())
                return;

            if (!sConditionMgr->IsPlayerMeetingCondition(user, GetGOInfo()->questgiver.conditionID1))
                return;

            Player* player = user->ToPlayer();
            player->PrepareGossipMenu(this, GetGOInfo()->questgiver.gossipID);
            player->SendPreparedGossip(this);
            return;
        }
        case GAMEOBJECT_TYPE_TRAP:                          //6
        {
            GameObjectTemplate const* goInfo = GetGOInfo();
            if (!sConditionMgr->IsPlayerMeetingCondition(user, goInfo->trap.conditionID1))
                return;

            if (goInfo->trap.spell)
                CastSpell(user, goInfo->trap.spell);

            m_cooldownTime = time(nullptr) + (goInfo->trap.cooldown ? goInfo->trap.cooldown : uint32(4));   // template or 4 seconds

            if (goInfo->trap.charges == 1)         // Deactivate after trigger
                SetLootState(GO_JUST_DEACTIVATED);
            return;
        }
        //Sitting: Wooden bench, chairs enzz
        case GAMEOBJECT_TYPE_CHAIR:                         //7
        {
            if (!user->IsPlayer())
                return;

            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!sConditionMgr->IsPlayerMeetingCondition(user, info->chair.conditionID1))
                return;

            if (ChairListSlots.empty())        // this is called once at first chair use to make list of available slots
            {
                if (info->chair.chairslots > 0)     // sometimes chairs in DB have error in fields and we dont know number of slots
                    for (uint32 i = 0; i < info->chair.chairslots; ++i)
                        ChairListSlots[i].Clear(); // Last user of current slot set to 0 (none sit here yet)
                else
                    ChairListSlots[0].Clear();     // error in DB, make one default slot
            }

            Player* player = user->ToPlayer();

            // a chair may have n slots. we have to calculate their positions and teleport the player to the nearest one

            float lowestDist = DEFAULT_VISIBILITY_DISTANCE;

            uint32 nearest_slot = 0;
            float x_lowest = GetPositionX();
            float y_lowest = GetPositionY();

            // the object orientation + 1/2 pi
            // every slot will be on that straight line
            float orthogonalOrientation = GetOrientation()+M_PI*0.5f;
            // find nearest slot
            bool found_free_slot = false;
            for (auto& chairListSlot : ChairListSlots)
            {
                // the distance between this slot and the center of the go - imagine a 1D space
                float relativeDistance = GetObjectScale()* chairListSlot.first - GetObjectScale()*(info->chair.chairslots - 1) / 2.0f;

                float x_i = GetPositionX() + relativeDistance * std::cos(orthogonalOrientation);
                float y_i = GetPositionY() + relativeDistance * std::sin(orthogonalOrientation);

                if (chairListSlot.second)
                {
                    if (Player* ChairUser = ObjectAccessor::FindPlayer(chairListSlot.second))
                    {
                        if (ChairUser->IsSitState() && ChairUser->getStandState() != UNIT_STAND_STATE_SIT && ChairUser->GetExactDist2d(x_i, y_i) < 0.1f)
                            continue;        // This seat is already occupied by ChairUser. NOTE: Not sure if the ChairUser->getStandState() != UNIT_STAND_STATE_SIT check is required.
                        chairListSlot.second.Clear();
                    }
                    else // This seat is unoccupied.
                        chairListSlot.second.Clear();     // The seat may of had an occupant, but they're offline.
                }

                found_free_slot = true;

                // calculate the distance between the player and this slot
                float thisDistance = player->GetDistance2d(x_i, y_i);

                /* debug code. It will spawn a npc on each slot to visualize them.
                Creature* helper = player->SummonCreature(14496, x_i, y_i, GetPositionZ(), GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10000);
                std::ostringstream output;
                output << i << ": thisDist: " << thisDistance;
                helper->MonsterSay(output.str().c_str(), LANG_UNIVERSAL, 0);
                */

                if (thisDistance <= lowestDist)
                {
                    nearest_slot = chairListSlot.first;
                    lowestDist = thisDistance;
                    x_lowest = x_i;
                    y_lowest = y_i;
                }
            }

            if (found_free_slot)
            {
                auto itr = ChairListSlots.find(nearest_slot);
                if (itr != ChairListSlots.end())
                {
                    itr->second = player->GetGUID(); //this slot in now used by player
                    player->TeleportTo(GetMapId(), x_lowest, y_lowest, GetPositionZ(), GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET);
                    player->SetStandState(UNIT_STAND_STATE_SIT_LOW_CHAIR + info->chair.chairheight);
                    return;
                }
            }
            //else
                //player->GetSession()->SendNotification("There's nowhere left for you to sit.");

            return;
        }
        //big gun, its a spell/aura
        case GAMEOBJECT_TYPE_GOOBER:                        //10
        {
            GameObjectTemplate const* info = GetGOInfo();

            if (user->IsPlayer())
            {
                Player* player = user->ToPlayer();

                if (info->goober.pageID)                    // show page...
                    player->SendDirectMessage(WorldPackets::GameObject::PageText(GetGUID()).Write());
                else if (info->goober.gossipID)
                {
                    player->PrepareGossipMenu(this, info->goober.gossipID);
                    player->SendPreparedGossip(this);
                }

                if (info->goober.eventID)
                {
                    TC_LOG_DEBUG(LOG_FILTER_MAPSCRIPTS, "Goober ScriptStart id %u for GO entry %u (GUID %u).", info->goober.eventID, GetEntry(), GetDBTableGUIDLow());
                    GetMap()->ScriptsStart(sEventScripts, info->goober.eventID, player, this);
                    EventInform(info->goober.eventID);
                }

                // possible quest objective for active quests
                if (info->goober.questID && sQuestDataStore->GetQuestTemplate(info->goober.questID))
                {
                    //Quest require to be active for GO using
                    if (player->GetQuestStatus(info->goober.questID) != QUEST_STATUS_INCOMPLETE)
                        break;
                }

                if (Battleground* bg = player->GetBattleground())
                    bg->EventPlayerUsedGO(player, this);

                player->KillCreditGO(info->entry, GetGUID());

                GetMap()->ScriptsStart(sGameObjectScripts, GetDBTableGUIDLow(), player, this);
            }

            if (uint32 trapEntry = info->goober.linkedTrap)
                TriggeringLinkedGameObject(trapEntry, user);

            SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
            SetLootState(GO_ACTIVATED, user);

            // this appear to be ok, however others exist in addition to this that should have custom (ex: 190510, 188692, 187389)
            if (info->goober.customAnim)
                SendCustomAnim(GetGoAnimProgress());
            else
                SetGoState(GO_STATE_ACTIVE);

            m_cooldownTime = time(nullptr) + (info->GetAutoCloseTime() ? info->GetAutoCloseTime() : 60);

            // cast this spell later if provided
            spellId = info->goober.spell;
            spellCaster = nullptr;

            break;
        }
        case GAMEOBJECT_TYPE_CAMERA:                        //13
        {
            if (!user->IsPlayer())
                return;

            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            Player* player = user->ToPlayer();

            if (info->camera.camera)
                player->SendCinematicStart(info->camera.camera);

            if (info->camera.eventID)
                GetMap()->ScriptsStart(sEventScripts, info->camera.eventID, player, this);

            return;
        }
        case GAMEOBJECT_TYPE_FISHINGNODE:                   //17
        {
            Player* player = user->ToPlayer();
            if (!player)
                return;

            if (player->GetGUID() != GetOwnerGUID())
                return;

            switch (getLootState())
            {
                case GO_READY:                              // ready for loot
                {
                    uint32 zone, subzone;
                    GetZoneAndAreaId(zone, subzone);

                    int32 zone_skill = sObjectMgr->GetFishingBaseSkillLevel(subzone);
                    if (!zone_skill)
                        zone_skill = sObjectMgr->GetFishingBaseSkillLevel(zone);

                    //provide error, no fishable zone or area should be 0
                    if (!zone_skill)
                        TC_LOG_ERROR(LOG_FILTER_SQL, "Fishable areaId %u are not properly defined in `skill_fishing_base_level`.", subzone);

                    int32 skill = player->GetSkillValue(SKILL_FISHING);

                    int32 chance;
                    if (skill < zone_skill)
                    {
                        chance = int32(pow(static_cast<double>(skill)/zone_skill, 2) * 100);
                        if (chance < 1)
                            chance = 1;
                    }
                    else
                        chance = 100;

                    int32 roll = irand(1, 100);

                    TC_LOG_DEBUG(LOG_FILTER_GENERAL, "Fishing check (skill: %i zone min skill: %i chance %i roll: %i", skill, zone_skill, chance, roll);

                    // but you will likely cause junk in areas that require a high fishing skill (not yet implemented)
                    if (chance >= roll)
                    {
                        player->UpdateFishingSkill();

                        //TODO: I do not understand this hack. Need some explanation.
                        // prevent removing GO at spell cancel
                        RemoveFromOwner();
                        SetOwnerGUID(player->GetGUID());

                        //TODO: find reasonable value for fishing hole search
                        if (auto ok = LookupFishingHoleAround(20.0f + CONTACT_DISTANCE))
                        {
                            ok->Use(player);
                            SetLootState(GO_JUST_DEACTIVATED);
                        }
                        else
                            player->SendLoot(GetGUID(), LOOT_FISHING);
                    }
                    else // else: junk
                        player->SendLoot(GetGUID(), LOOT_FISHING_JUNK);
                    break;
                }
                case GO_JUST_DEACTIVATED:                   // nothing to do, will be deleted at next update
                    break;
                default:
                {
                    SetLootState(GO_JUST_DEACTIVATED);
                    player->SendDirectMessage(WorldPackets::GameObject::FishNotHooked().Write());
                    break;
                }
            }

            player->FinishSpell(CURRENT_CHANNELED_SPELL);
            return;
        }
        case GAMEOBJECT_TYPE_RITUAL:              //18
        {
            if (!user->IsPlayer())
                return;

            Player* player = user->ToPlayer();

            Unit* owner = GetOwner();

            GameObjectTemplate const* info = GetGOInfo();

            // ritual owner is set for GO's without owner (not summoned)
            if (!m_ritualOwner && !owner)
                m_ritualOwner = player;

            if (owner)
            {
                if (!owner->IsPlayer())
                    return;

                // accept only use by player from same group as owner, excluding owner itself (unique use already added in spell effect)
                if (player == owner->ToPlayer() || info->ritual.castersGrouped && !player->IsInSameRaidWith(owner->ToPlayer()))
                    return;

                // expect owner to already be channeling, so if not...
                if (!owner->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    return;

                // in case summoning ritual caster is GO creator
                spellCaster = owner;
            }
            else
            {
                if (player != m_ritualOwner && (info->ritual.castersGrouped && !player->IsInSameRaidWith(m_ritualOwner)))
                    return;

                spellCaster = player;
            }

            AddUniqueUse(player);

            if (info->ritual.animSpell)
            {
                player->CastSpell(player, info->ritual.animSpell, true);

                // for this case, ritual.spellId is always triggered
                triggered = true;
            }

            // full amount unique participants including original summoner
            if (GetUniqueUseCount() == info->ritual.casters)
            {
                spellCaster = m_ritualOwner ? m_ritualOwner : spellCaster;
                if (m_ritualOwner)
                    spellCaster = m_ritualOwner;

                spellId = info->ritual.spell;

                if (spellId == 62330)                       // GO store nonexistent spell, replace by expected
                {
                    // spell have reagent and mana cost but it not expected use its
                    // it triggered spell in fact casted at currently channeled GO
                    spellId = 61993;
                    triggered = true;
                }

                // Cast casterTargetSpell at a random GO user
                // on the current DB there is only one gameobject that uses this (Ritual of Doom)
                // and its required target number is 1 (outter for loop will run once)
                if (info->ritual.casterTargetSpell && info->ritual.casterTargetSpell != 1) // No idea why this field is a bool in some cases
                    for (uint32 i = 0; i < info->ritual.casterTargetSpellTargets; i++)
                        // m_unique_users can contain only player GUIDs
                        if (Player* target = ObjectAccessor::GetPlayer(*this, Trinity::Containers::SelectRandomContainerElement(m_unique_users)))
                            spellCaster->CastSpell(target, info->ritual.casterTargetSpell, true);

                // finish owners spell
                if (owner)
                    owner->FinishSpell(CURRENT_CHANNELED_SPELL);

                // can be deleted now, if
                if (!info->ritual.ritualPersistent)
                    SetLootState(GO_JUST_DEACTIVATED);
                else
                {
                    // reset ritual for this GO
                    m_ritualOwner = nullptr;
                    m_unique_users.clear();
                    m_usetimes = 0;
                }
            }
            else
                return;

            // go to end function to spell casting
            break;
        }
        case GAMEOBJECT_TYPE_SPELLCASTER:                   //22
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (info->spellCaster.partyOnly)
            {
                Unit* caster = GetOwner();
                if (!caster || !caster->IsPlayer())
                    return;

                if (!user->IsPlayer() || !user->ToPlayer()->IsInSameRaidWith(caster->ToPlayer()))
                    return;
            }

            if (user->NeedDismount())
                user->RemoveAurasByType(SPELL_AURA_MOUNTED);

            spellId = info->spellCaster.spell;

            AddUse();
            break;
        }
        case GAMEOBJECT_TYPE_MEETINGSTONE:                  //23
        {
            GameObjectTemplate const* info = GetGOInfo();

            if (!user->IsPlayer())
                return;

            Player* player = user->ToPlayer();

            Player* targetPlayer = ObjectAccessor::FindPlayer(player->GetSelection());

            // accept only use by player from same raid as caster, except caster itself
            if (!targetPlayer || targetPlayer == player || !targetPlayer->IsInSameRaidWith(player))
                return;

            if (Group* group = player->GetGroup())
                if (group->InChallenge() && player->InInstance() != targetPlayer->InInstance())
                    return;

            //required lvl checks!
            uint8 level = player->getLevel();
            if (level < info->meetingStone.minLevel)
                return;
            level = targetPlayer->getLevel();
            if (level < info->meetingStone.minLevel)
                return;

            if (info->entry == 194097)
                spellId = 61994;                            // Ritual of Summoning
            else
                spellId = 59782;                            // Summoning Stone Effect

            break;
        }

        case GAMEOBJECT_TYPE_FLAGSTAND:                     // 24
        case GAMEOBJECT_TYPE_CAPTURE_POINT:                 // 42
        {
            auto player = user->ToPlayer();
            if (!player)
                return;

            if (player->CanUseBattlegroundObject())
            {
                auto bg = player->GetBattleground();
                if (!bg)
                    return;

                if (player->GetVehicle())
                    return;

                player->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                player->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);
                bool needDelete = true;
                bg->EventPlayerClickedOnFlag(player, this, needDelete);
                return;
            }
            break;
        }
        case GAMEOBJECT_TYPE_FISHINGHOLE:                   // 25
        {
            if (!user->IsPlayer())
                return;

            auto player = user->ToPlayer();

            player->SendLoot(GetGUID(), LOOT_FISHINGHOLE);
            player->UpdateAchievementCriteria(CRITERIA_TYPE_FISH_IN_GAMEOBJECT, GetGOInfo()->entry);
            player->UpdateAchievementCriteria(CRITERIA_TYPE_CATCH_FROM_POOL, GetGOInfo()->entry);
            return;
        }
        case GAMEOBJECT_TYPE_FLAGDROP:                      // 26
        case GAMEOBJECT_TYPE_NEW_FLAG_DROP:
        {
            if (!user->IsPlayer())
                return;

            auto player = user->ToPlayer();
            if (player->CanUseBattlegroundObject())
            {
                auto bg = player->GetBattleground();
                if (!bg)
                    return;

                if (player->GetVehicle())
                    return;

                bool needDelete = true;

                if (auto info = GetGOInfo())
                {
                    switch (info->entry)
                    {
                        case 227745: // Silverwing && Warsong Flags
                        case 227744:
                        case 228508: // Netherstorm Flag
                        {
                            switch (bg->GetTypeID(true))
                            {
                                case MS::Battlegrounds::BattlegroundTypeId::BattlegroundWarsongGulch:
                                case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble:
                                case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundHotmoguTemplate:
                                case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundKotmoguTemplateSix:
                                case MS::Battlegrounds::BattlegroundTypeId::BattlegroundRatedEyeOfTheStorm:
                                case MS::Battlegrounds::BattlegroundTypeId::BattlegroundTwinPeaks:
                                case MS::Battlegrounds::BattlegroundTypeId::BattlegroundEyeOfTheStorm:
                                case MS::Battlegrounds::BattlegroundTypeId::BrawlEyeOfStorm:
                                case MS::Battlegrounds::BattlegroundTypeId::BattlegroundKotmoguTemplate:
                                case MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongSix:
                                    bg->EventPlayerClickedOnFlag(player, this, needDelete);
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                //this cause to call return, all flags must be deleted here!!
                if (needDelete)
                {
                    spellId = 0;
                    Delete();
                }
            }
            break;
        }
        case GAMEOBJECT_TYPE_BARBER_CHAIR:                  //32
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!user->IsPlayer())
                return;

            Player* player = user->ToPlayer();
            if (!player)
                return;

            // fallback, will always work
            player->TeleportTo(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation(), TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET);
            player->SendDirectMessage(WorldPackets::Misc::EnableBarberShop().Write());
            player->SetStandState(UNIT_STAND_STATE_SIT_LOW_CHAIR + info->barberChair.chairheight, info->barberChair.SitAnimKit);

            if (IsHolidayActive(HOLIDAY_TRIAL_OF_STYLE))
                player->RemoveAura(241834);
            return;
        }
        case GAMEOBJECT_TYPE_NEW_FLAG:     //36
        {
            GameObjectTemplate const* goInfo = GetGOInfo();
            if (!goInfo)
                return;

            if (!sConditionMgr->IsPlayerMeetingCondition(user, goInfo->newflag.conditionID1))
                return;

            Player* player = user->ToPlayer();
            if (!player)
                return;

            if (!player->CanUseBattlegroundObject())
                return;

            bool needDelete = true;
            if (Battleground* bg = player->GetBattleground())
            {
                player->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                player->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);
                bg->EventPlayerClickedOnFlag(player, this, needDelete);
                bg->EventPlayerUsedGO(player, this);
            }

            if (needDelete)
            {
                SetLootState(GO_JUST_DEACTIVATED);
                spellId = 0;
            }
            break;
        }
        case GAMEOBJECT_TYPE_ARTIFACT_FORGE:     //47
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            // if (!sConditionMgr->IsPlayerMeetingCondition(user, info->artifactForge.conditionID1))
                // return;

            if (!user->IsPlayer())
                return;

            Player* player = user->ToPlayer();
            Item* artifact = player->GetArtifactWeapon();
            if (!artifact)
            {
                WorldPackets::Misc::DisplayGameError display;
                display.Error = UIErrors::ERR_MUST_EQUIP_ARTIFACT;
                player->SendDirectMessage(display.Write());
                return;
            }
            uint32 categoryID = 0;
            if (ItemTemplate const* proto = artifact->GetTemplate())
                if (uint32 artifactID = proto->GetArtifactID())
                    if (ArtifactEntry const* entry = sArtifactStore.LookupEntry(artifactID))
                        categoryID = entry->ArtifactCategoryID;

            if (GetEntry() == 246520) // Fishing forge
            {
                if (categoryID != ARTIFACT_CATEGORY_FISH)
                {
                    WorldPackets::Misc::DisplayGameError display;
                    display.Error = UIErrors::ERR_MUST_EQUIP_ARTIFACT;
                    player->SendDirectMessage(display.Write());
                    return;
                }
            }
            else if (categoryID == ARTIFACT_CATEGORY_FISH)
            {
                WorldPackets::Misc::DisplayGameError display;
                display.Error = UIErrors::ERR_MUST_EQUIP_ARTIFACT;
                player->SendDirectMessage(display.Write());
                return;
            }

            WorldPackets::Artifact::ForgeOpenResult artifactForgeOpened;
            artifactForgeOpened.ArtifactGUID = artifact->GetGUID();
            artifactForgeOpened.GameObjectGUID = GetGUID();
            player->SendDirectMessage(artifactForgeOpened.Write());
            return;
        }
        case GAMEOBJECT_TYPE_MAILBOX:
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!user->IsPlayer())
                return;

            if (!sConditionMgr->IsPlayerMeetingCondition(user, info->mailbox.conditionID1))
                return;

            return;
        }
        case GAMEOBJECT_TYPE_TEXT:
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!user->IsPlayer())
                return;

            if (!sConditionMgr->IsPlayerMeetingCondition(user, info->text.conditionID1))
                return;

            return;
        }
        case GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD:
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!user->IsPlayer())
                return;

            user->ToPlayer()->SendLoot(GetGUID(), LOOT_CORPSE);
            return;
        }
        case GAMEOBJECT_TYPE_UI_LINK:
        {
            GameObjectTemplate const* info = GetGOInfo();
            if (!info)
                return;

            if (!user->IsPlayer())
                return;

            if (Player* player = user->ToPlayer())
                player->SendDirectMessage(WorldPackets::GameObject::GameObjectUiAction(GetGUID(), info->UILink.UILinkType).Write());
            return;
        }
        default:
            if (GetGoType() >= MAX_GAMEOBJECT_TYPE)
                TC_LOG_ERROR(LOG_FILTER_GENERAL, "GameObject::Use(): unit (type: %u, guid: %u, name: %s) tries to use object (guid: %u, entry: %u, name: %s) of unknown type (%u)",
                    user->GetTypeId(), user->GetGUIDLow(), user->GetName(), GetGUIDLow(), GetEntry(), GetGOInfo()->name.c_str(), GetGoType());
            break;
    }

    if (!spellId)
        return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        if (!user->IsPlayer() || !sOutdoorPvPMgr->HandleCustomSpell(user->ToPlayer(), spellId, this))
            TC_LOG_DEBUG(LOG_FILTER_GENERAL, "WORLD: unknown spell id %u at use action for gameobject (Entry: %u GoType: %u)", spellId, GetEntry(), GetGoType());
        else
            TC_LOG_DEBUG(LOG_FILTER_GENERAL, "WORLD: %u non-dbc spell was handled by OutdoorPvP", spellId);
        return;
    }

    if (spellCaster)
        spellCaster->CastSpell(user, spellInfo, triggered);
    else
        CastSpell(user, spellId);
}

void GameObject::CastSpell(Unit* target, uint32 spellId)
{
    if (target)
        if (Player* tmpPlayer = target->ToPlayer())
            if (tmpPlayer->IsSpectator())
                return;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    bool self = false;

    GameObjectTemplate const* goInfo = GetGOInfo();
    if (goInfo->type != GAMEOBJECT_TYPE_TRAP && target && target->HasAuraType(SPELL_AURA_MOD_STEALTH))
        self = true;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->EffectMask < uint32(1 << i))
            break;

        if (spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_UNIT_CASTER)
        {
            self = true;
            break;
        }
    }

    if (goInfo->type == GAMEOBJECT_TYPE_TRAP && GetOwner() && target && target->HasAuraType(SPELL_AURA_MOD_STEALTH))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spellInfo->EffectMask < uint32(1 << i))
                break;

            if (spellInfo->Effects[i]->TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
            {
                self = true;
                break;
            }
        }
    }

    if (self)
    {
        if (target)
            target->CastSpell(target, spellInfo, true);
        return;
    }

    if (goInfo->type == GAMEOBJECT_TYPE_TRAP && target && target->IsPlayer())
    {
        target->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_USE);
    }

    //summon world trigger
    Creature* trigger = SummonTrigger(GetPositionX(), GetPositionY(), GetPositionZ(), 0, spellInfo->CalcCastTime() + 100);
    if (!trigger)
        return;

    if (Unit* owner = GetOwner())
    {
        trigger->setFaction(owner->getFaction());
        // needed for GO casts for proper target validation checks
        trigger->SetGuidValue(UNIT_FIELD_SUMMONED_BY, owner->GetGUID());
        trigger->CastSpell(target ? target : trigger, spellInfo, true, nullptr, nullptr, owner->GetGUID());
    }
    else
    {
        if (target)
            trigger->SetPhaseMask(target->GetPhaseMask(), true);
        trigger->setFaction(14);
        // Set owner guid for target if no owner available - needed by trigger auras
        // - trigger gets despawned and there's no caster avalible (see AuraEffect::TriggerSpell())
        trigger->CastSpell(target ? target : trigger, spellInfo, true, nullptr, nullptr, target ? target->GetGUID() : ObjectGuid::Empty);
    }
}

void GameObject::SendCustomAnim(uint32 animID, bool playAsDespawn /*= false*/)
{
    WorldPackets::GameObject::GoCustomAnim customAnim;
    customAnim.ObjectGUID = GetGUID();
    customAnim.CustomAnim = animID;
    customAnim.PlayAsDespawn = playAsDespawn;
    SendMessageToSet(customAnim.Write(), true);
}

void GameObject::SendGOPlaySpellVisual(uint32 spellVisualID, ObjectGuid activatorGuid /*= ObjectGuid::Empty*/)
{
    WorldPackets::GameObject::GameObjectPlaySpellVisual objectSpellVisual;
    objectSpellVisual.ActivatorGUID = activatorGuid;
    objectSpellVisual.ObjectGUID = GetGUID();
    objectSpellVisual.SpellVisualID = spellVisualID;
    SendMessageToSet(objectSpellVisual.Write(), true);
}

void GameObject::SetAnimKitId(uint16 animKitID, bool maintain /*= false*/)
{
    if (_animKitId == animKitID)
        return;

    if (animKitID && !sAnimKitStore.LookupEntry(animKitID))
        return;

    if (!maintain)
        _animKitId = animKitID;
    else
        _animKitId = 0;

    WorldPackets::GameObject::GameObjectActivateAnimKit activateAnimKit;
    activateAnimKit.ObjectGUID = GetGUID();
    activateAnimKit.AnimKitID = animKitID;
    activateAnimKit.Maintain = !maintain;
    SendMessageToSet(activateAnimKit.Write(), true);
}

bool GameObject::IsInRange(float x, float y, float z, float radius) const
{
    GameObjectDisplayInfoEntry const* info = sGameObjectDisplayInfoStore.LookupEntry(m_goInfo->displayId);
    if (!info)
        return IsWithinDist3d(x, y, z, radius);

    float sinA = std::sin(GetOrientation());
    float cosA = std::cos(GetOrientation());
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();
    float dz = z - GetPositionZ();
    float dist = sqrt(dx * dx + dy * dy);
    //! Check if the distance between the 2 objects is 0, can happen if both objects are on the same position.
    //! The code below this check wont crash if dist is 0 because 0/0 in float operations is valid, and returns infinite
    if (G3D::fuzzyEq(dist, 0.0f))
        return true;

    float sinB = dx / dist;
    float cosB = dy / dist;
    dx = dist * (cosA * cosB + sinA * sinB);
    dy = dist * (cosA * sinB - sinA * cosB);
    return dx < info->GeoBoxMax.X + radius && dx > info->GeoBoxMin.X - radius
        && dy < info->GeoBoxMax.Y + radius && dy > info->GeoBoxMin.Y - radius
        && dz < info->GeoBoxMax.Z + radius && dz > info->GeoBoxMin.Z - radius;
}

void GameObject::EventInform(uint32 eventId)
{
    if (!eventId)
        return;

    if (AI())
        AI()->EventInform(eventId);

    if (m_zoneScript)
        m_zoneScript->ProcessEvent(this, eventId);
}

// overwrite WorldObject function for proper name localization
const char* GameObject::GetNameForLocaleIdx(LocaleConstant localeConstant) const
{
    if (localeConstant != DEFAULT_LOCALE)
    {
        auto uloc_idx = uint8(localeConstant);
        if (GameObjectLocale const* cl = sObjectMgr->GetGameObjectLocale(GetEntry()))
            if (cl->Name.size() > uloc_idx && !cl->Name[uloc_idx].empty())
                return cl->Name[uloc_idx].c_str();
    }

    return GetName();
}

void GameObject::UpdatePackedRotation()
{
    static const int32 PACK_YZ = 1 << 20;
    static const int32 PACK_X = PACK_YZ << 1;

    static const int32 PACK_YZ_MASK = (PACK_YZ << 1) - 1;
    static const int32 PACK_X_MASK = (PACK_X << 1) - 1;

    int8 w_sign = (m_worldRotation.w >= 0.f ? 1 : -1);
    int64 x = int32(m_worldRotation.x * PACK_X)  * w_sign & PACK_X_MASK;
    int64 y = int32(m_worldRotation.y * PACK_YZ) * w_sign & PACK_YZ_MASK;
    int64 z = int32(m_worldRotation.z * PACK_YZ) * w_sign & PACK_YZ_MASK;
    m_packedRotation = z | (y << 21) | (x << 42);
}

void GameObject::SetWorldRotation(float qx, float qy, float qz, float qw)
{
    G3D::Quat rotation(qx, qy, qz, qw);
    rotation.unitize();
    m_worldRotation.x = rotation.x;
    m_worldRotation.y = rotation.y;
    m_worldRotation.z = rotation.z;
    m_worldRotation.w = rotation.w;
    UpdatePackedRotation();
}

void GameObject::SetParentRotation(G3D::Quat const& rotation)
{
    SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 0, rotation.x);
    SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 1, rotation.y);
    SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 2, rotation.z);
    SetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 3, rotation.w);
}

void GameObject::SetWorldRotationAngles(float z_rot, float y_rot, float x_rot)
{
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(z_rot, y_rot, x_rot));
    SetWorldRotation(quat.x, quat.y, quat.z, quat.w);
}

void GameObject::ModifyHealth(int32 change, Unit* caster /*= NULL*/, uint32 spellId /*= 0*/, ObjectGuid CasterGUID)
{
    if (!m_goValue.Building.MaxHealth || !change)
        return;

    // prevent double destructions of the same object
    if (change < 0 && !m_goValue.Building.Health)
        return;

    if (int32(m_goValue.Building.Health) + change <= 0)
        m_goValue.Building.Health = 0;
    else if (int32(m_goValue.Building.Health) + change >= int32(m_goValue.Building.MaxHealth))
        m_goValue.Building.Health = m_goValue.Building.MaxHealth;
    else
        m_goValue.Building.Health += change;

    // Set the health bar, value = 255 * healthPct;
    SetGoAnimProgress(m_goValue.Building.Health * 255 / m_goValue.Building.MaxHealth);

    Player* player = caster ? caster->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;

    // dealing damage, send packet
    // TODO: is there any packet for healing?
    if (change < 0 && player)
    {
        WorldPackets::GameObject::DestructibleBuildingDamage buildingDamage;
        buildingDamage.Target = GetGUID();
        buildingDamage.Caster = player->GetGUID(); // Origian caster GUID. This Player
        buildingDamage.Owner = CasterGUID;         // Caster GUID. This Player, Unit or GO
        buildingDamage.Damage = -change;
        buildingDamage.SpellID = spellId;
        player->SendDirectMessage(buildingDamage.Write());
    }

    GameObjectDestructibleState newState = GetDestructibleState();

    if (!m_goValue.Building.Health)
        newState = GO_DESTRUCTIBLE_DESTROYED;
    else if (m_goValue.Building.Health <= 10000/*GetGOInfo()->destructibleBuilding.damagedNumHits*/) // TODO: Get health somewhere
        newState = GO_DESTRUCTIBLE_DAMAGED;
    else if (m_goValue.Building.Health == m_goValue.Building.MaxHealth)
        newState = GO_DESTRUCTIBLE_INTACT;

    if (newState == GetDestructibleState())
        return;

    SetDestructibleState(newState, player, false);
}

void GameObject::SetDestructibleState(GameObjectDestructibleState state, Player* eventInvoker /*= NULL*/, bool setHealth /*= false*/)
{
    // the user calling this must know he is already operating on destructible gameobject
    ASSERT(GetGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING);

    switch (state)
    {
        case GO_DESTRUCTIBLE_INTACT:
            RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);
            SetDisplayId(m_goInfo->displayId);
            if (setHealth)
            {
                m_goValue.Building.Health = m_goValue.Building.MaxHealth;
                SetGoAnimProgress(255);
            }
            EnableCollision(true);
            break;
        case GO_DESTRUCTIBLE_DAMAGED:
        {
            EventInform(m_goInfo->destructibleBuilding.DamagedEvent);
            sScriptMgr->OnGameObjectDamaged(this, eventInvoker);
            if (eventInvoker)
                if (Battleground* bg = eventInvoker->GetBattleground())
                    bg->EventPlayerDamagedGO(eventInvoker, this, m_goInfo->destructibleBuilding.DamagedEvent);

            RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);
            SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED);

            uint32 modelId = m_goInfo->destructibleBuilding.DestructibleModelRec;
            if (DestructibleModelDataEntry const* modelData = sDestructibleModelDataStore.LookupEntry(m_goInfo->destructibleBuilding.DestructibleModelRec))
                if (modelData->State1Wmo)
                    modelId = modelData->State1Wmo;
            SetDisplayId(modelId);

            if (setHealth)
            {
                m_goValue.Building.Health = 10000/*m_goInfo->destructibleBuilding.damagedNumHits*/;
                uint32 maxHealth = m_goValue.Building.MaxHealth;
                // in this case current health is 0 anyway so just prevent crashing here
                if (!maxHealth)
                    maxHealth = 1;
                SetGoAnimProgress(m_goValue.Building.Health * 255 / maxHealth);
            }
            break;
        }
        case GO_DESTRUCTIBLE_DESTROYED:
        {
            sScriptMgr->OnGameObjectDestroyed(this, eventInvoker);
            EventInform(m_goInfo->destructibleBuilding.DestroyedEvent);
            if (eventInvoker)
            {
                if (Battleground* bg = eventInvoker->GetBattleground())
                {
                    bg->EventPlayerDamagedGO(eventInvoker, this, m_goInfo->destructibleBuilding.DestroyedEvent);
                    bg->DestroyGate(eventInvoker, this);
                }
            }

            RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED);
            SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED);

            uint32 modelId = m_goInfo->displayId;
            if (DestructibleModelDataEntry const* modelData = sDestructibleModelDataStore.LookupEntry(m_goInfo->destructibleBuilding.DestructibleModelRec))
                if (modelData->State2Wmo)
                    modelId = modelData->State2Wmo;
            SetDisplayId(modelId);

            if (setHealth)
            {
                m_goValue.Building.Health = 0;
                SetGoAnimProgress(0);
            }
            EnableCollision(false);
            break;
        }
        case GO_DESTRUCTIBLE_REBUILDING:
        {
            EventInform(m_goInfo->destructibleBuilding.RebuildingEvent);
            RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);

            uint32 modelId = m_goInfo->displayId;
            if (DestructibleModelDataEntry const* modelData = sDestructibleModelDataStore.LookupEntry(m_goInfo->destructibleBuilding.DestructibleModelRec))
                if (modelData->State3Wmo)
                    modelId = modelData->State3Wmo;
            SetDisplayId(modelId);

            // restores to full health
            if (setHealth)
            {
                m_goValue.Building.Health = m_goValue.Building.MaxHealth;
                SetGoAnimProgress(255);
            }
            EnableCollision(true);
            break;
        }
    }

    UpdateObjectVisibility();
}

GameObjectDestructibleState GameObject::GetDestructibleState() const
{
    if (HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED))
        return GO_DESTRUCTIBLE_DESTROYED;
    if (HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DAMAGED))
        return GO_DESTRUCTIBLE_DAMAGED;
    return GO_DESTRUCTIBLE_INTACT;
}

void GameObject::SetLootState(LootState state, Unit* unit)
{
    m_lootState = state;
    AI()->OnStateChanged(state, unit);
    sScriptMgr->OnGameObjectLootStateChanged(this, state, unit);
    if (m_model)
    {
        bool collision = false;

        // Use the current go state
        if (GetGoState() != GO_STATE_READY && (state == GO_ACTIVATED || state == GO_JUST_DEACTIVATED) || state == GO_READY)
            collision = !collision;

        EnableCollision(collision);
    }
}

void GameObject::SetGoState(GOState state)
{
    SetByteValue(GAMEOBJECT_FIELD_BYTES_1, GAMEOJBECT_BYTES_0_STATE, state);

    sScriptMgr->OnGameObjectStateChanged(this, state);
    if (m_model && !IsTransport())
    {
        if (!IsInWorld())
            return;

        // startOpen determines whether we are going to add or remove the LoS on activation
        auto collision = false;
        if (state == GO_STATE_READY)
            collision = !collision;

        EnableCollision(collision);
    }

    auto visualBeforeComplete = m_goInfo->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST];

    switch (state)
    {
        case GO_STATE_READY:
            if (!m_goInfo->visualQuestID)
            {
                if (auto worldEffectID = visualBeforeComplete.StateWorldEffectID)
                    SetUInt32Value(GAMEOBJECT_FIELD_STATE_WORLD_EFFECT_ID, worldEffectID);
                if (auto stateVisualID = visualBeforeComplete.SpellStateVisualID)
                    SetUInt32Value(GAMEOBJECT_FIELD_STATE_SPELL_VISUAL_ID, stateVisualID);
            }

            if (auto visualID = GetGOInfo()->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellVisualID)
                SetUInt32Value(GAMEOBJECT_FIELD_SPELL_VISUAL_ID, visualID);

            break;
        default:
            if (!m_goInfo->visualQuestID)
            {
                if (visualBeforeComplete.StateWorldEffectID)
                    SetUInt32Value(GAMEOBJECT_FIELD_STATE_WORLD_EFFECT_ID, 0);
                if (auto stateVisualID = visualBeforeComplete.SpellStateVisualID)
                    SetUInt32Value(GAMEOBJECT_FIELD_STATE_SPELL_VISUAL_ID, stateVisualID);
            }
            if (GetGOInfo()->visualData[GO_VISUAL_BEFORE_COMPLETE_QUEST].SpellVisualID)
                SetUInt32Value(GAMEOBJECT_FIELD_SPELL_VISUAL_ID, 0);

            break;
    }
}

uint32 GameObject::GetTransportPeriod() const
{
    return 0;
}

uint32 GameObject::GetPathProgress() const
{
    return 0;
}

void GameObject::SetTransportState(GOState state, uint32 stopFrame /*= 0*/)
{
    if (GetGoState() == state)
        return;

    if (state == GO_STATE_TRANSPORT_ACTIVE)
        SetGoState(GO_STATE_TRANSPORT_ACTIVE);
    else
        SetGoState(GOState(GO_STATE_TRANSPORT_STOPPED + stopFrame));
}

void GameObject::SetDisplayId(uint32 displayid)
{
    SetUInt32Value(GAMEOBJECT_FIELD_DISPLAY_ID, displayid);
    UpdateModel();
}

void GameObject::SetPhaseMask(uint32 newPhaseMask, bool update)
{
    WorldObject::SetPhaseMask(newPhaseMask, update);
    if (m_model && m_model->isCollisionEnabled())
        EnableCollision(true);
}

uint8 GameObject::GetNameSetId() const
{
    switch (GetGoType())
    {
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            if (DestructibleModelDataEntry const* modelData = sDestructibleModelDataStore.LookupEntry(m_goInfo->destructibleBuilding.DestructibleModelRec))
            {
                switch (GetDestructibleState())
                {
                    case GO_DESTRUCTIBLE_INTACT:
                        return modelData->State0NameSet;
                    case GO_DESTRUCTIBLE_DAMAGED:
                        return modelData->State1NameSet;
                    case GO_DESTRUCTIBLE_DESTROYED:
                        return modelData->State2NameSet;
                    case GO_DESTRUCTIBLE_REBUILDING:
                        return modelData->State3NameSet;
                    default:
                        break;
                }
            }
            break;
        case GAMEOBJECT_TYPE_GARRISON_BUILDING:
        case GAMEOBJECT_TYPE_GARRISON_PLOT:
        case GAMEOBJECT_TYPE_PHASEABLE_MO:
            return GetByteValue(GAMEOBJECT_FIELD_FLAGS, 1) & 0xF;
        default:
            break;
    }

    return 0;
}

void GameObject::EnableCollision(bool enable)
{
    if (!m_model || m_goInfo->IgnoreDynLos)
        return;

    /*if (enable && !GetMap()->ContainsGameObjectModel(*m_model))
        GetMap()->InsertGameObjectModel(*m_model);*/

    m_model->enableCollision(enable);
}

void GameObject::UpdateModel()
{
    if (!IsInWorld())
        return;
    if (m_model)
        if (GetMap()->ContainsGameObjectModel(*m_model))
            GetMap()->RemoveGameObjectModel(*m_model);
    delete m_model;
    m_model = CreateModel();
    if (m_model)
        GetMap()->InsertGameObjectModel(*m_model);
}

bool GameObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool ignoreObjectSize) const
{
    //! Following check does check 3d distance
    return IsInRange(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), dist2compare);
}

Player* GameObject::GetLootRecipient() const
{
    if (!m_lootRecipient)
        return nullptr;
    return ObjectAccessor::FindPlayer(m_lootRecipient);
}

Group* GameObject::GetLootRecipientGroup() const
{
    if (!m_lootRecipientGroup)
        return nullptr;
    return sGroupMgr->GetGroupByGUID(m_lootRecipientGroup);
}

void GameObject::SetLootRecipient(Unit* unit)
{
    // set the player whose group should receive the right
    // to loot the creature after it dies
    // should be set to NULL after the loot disappears

    if (!unit)
    {
        m_lootRecipient.Clear();
        m_lootRecipientGroup.Clear();
        return;
    }

    if (!unit->IsPlayer() && !unit->IsVehicle())
        return;

    Player* player = unit->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (!player)                                             // normal creature, no player involved
        return;

    m_lootRecipient = player->GetGUID();
    if (Group* group = player->GetGroup())
        m_lootRecipientGroup = group->GetGUID();
}

bool GameObject::IsLootAllowedFor(Player const* player) const
{
    if (!m_lootRecipient && !m_lootRecipientGroup)
        return true;

    if (player->GetGUID() == m_lootRecipient)
        return true;

    Group const* playerGroup = player->GetGroup();
    if (!playerGroup || playerGroup != GetLootRecipientGroup()) // if we dont have a group we arent the recipient
        return false;                                           // if go doesnt have group bound it means it was solo killed by someone else

    return true;
}

bool GameObject::HasLootRecipient() const
{
    return m_lootRecipient || m_lootRecipientGroup;
}

void GameObject::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    bool forcedFlags = GetGoType() == GAMEOBJECT_TYPE_CHEST && GetGOInfo()->chest.usegrouplootrules && HasLootRecipient();
    bool targetIsGM = target->isGameMaster();

    std::size_t blockCount = UpdateMask::GetBlockCount(m_valuesCount);

    ObjectGuid ownerGUID = GetOwnerGUID();
    uint32* flags = GameObjectUpdateFieldFlags;
    uint32 visibleFlag = UF_FLAG_PUBLIC;
    if (ownerGUID == target->GetGUID())
        visibleFlag |= UF_FLAG_OWNER;

    *data << uint8(blockCount);
    std::size_t maskPos = data->wpos();
    data->resize(data->size() + blockCount * sizeof(UpdateMask::BlockType));

    for (uint16 index = 0; index < m_valuesCount; ++index)
    {
        if (_fieldNotifyFlags & flags[index] || (updateType == UPDATETYPE_VALUES ? _changesMask[index] : m_uint32Values[index]) && flags[index] & visibleFlag || index == GAMEOBJECT_FIELD_FLAGS && forcedFlags)
        {
            UpdateMask::SetUpdateBit(data->contents() + maskPos, index);

            if (index == OBJECT_FIELD_DYNAMIC_FLAGS)
            {
                uint16 dynFlags = 0;
                int16 pathProgress = -1;
                switch (GetGoType())
                {
                    case GAMEOBJECT_TYPE_QUESTGIVER:
                        if (ActivateToQuest(target))
                            dynFlags |= GO_DYNFLAG_LO_ACTIVATE;
                        break;
                    case GAMEOBJECT_TYPE_CHEST:
                    case GAMEOBJECT_TYPE_GOOBER:
                    case GAMEOBJECT_TYPE_CAPTURE_POINT:
                        if (ActivateToQuest(target))
                            dynFlags |= GO_DYNFLAG_LO_ACTIVATE | GO_DYNFLAG_LO_SPARKLE;
                        else if (targetIsGM)
                            dynFlags |= GO_DYNFLAG_LO_ACTIVATE;
                        break;
                    case GAMEOBJECT_TYPE_GENERIC:
                    case GAMEOBJECT_TYPE_SPELL_FOCUS:
                        if (ActivateToQuest(target))
                            dynFlags |= GO_DYNFLAG_LO_SPARKLE;
                        break;
                    case GAMEOBJECT_TYPE_TRANSPORT:
                    {
                        dynFlags = GetUInt16Value(OBJECT_FIELD_DYNAMIC_FLAGS, 0);
                        pathProgress = GetUInt16Value(OBJECT_FIELD_DYNAMIC_FLAGS, 1);
                        break;
                    }
                    case GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT:
                    {
                        if (uint32 transportPeriod = GetTransportPeriod())
                        {
                            auto timer = float(m_goValue.Transport.PathProgress % transportPeriod);
                            pathProgress = int16(timer / float(transportPeriod) * 65535.0f);
                        }
                        break;
                    }
                    case GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD:
                        if (sChallengeMgr->HasOploteLoot(target->GetGUID()))
                            dynFlags |= GO_DYNFLAG_LO_CHALLENGE_UNLOCK; // Unlock Go for 7.2.0
                        else
                            dynFlags |= GO_DYNFLAG_LO_CHALLENGE_ERROR; // Error unlock go
                        break;
                    default:
                        break;
                }

                *data << uint16(dynFlags);
                *data << int16(pathProgress);
            }
            else if (index == GAMEOBJECT_FIELD_FLAGS)
            {
                uint32 goFlags = m_uint32Values[GAMEOBJECT_FIELD_FLAGS];
                if (GetGoType() == GAMEOBJECT_TYPE_CHEST)
                    if (GetGOInfo()->chest.usegrouplootrules && !IsLootAllowedFor(target))
                        goFlags |= GO_FLAG_LOCKED | GO_FLAG_NOT_SELECTABLE;

                *data << goFlags;
            }
            else if (index == GAMEOBJECT_FIELD_STATE_WORLD_EFFECT_ID || index == GAMEOBJECT_FIELD_STATE_SPELL_VISUAL_ID)
            {
                if (GetGOInfo()->visualQuestID)
                    *data << target->GetGoVisualQuestData(this, index);
                else
                    *data << m_uint32Values[index];
            }
            else if (index == GAMEOBJECT_FIELD_DISPLAY_ID)
            {
                uint32 modelID = m_uint32Values[index];
                if (GetGoType() == GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD)
                {
                    if (sChallengeMgr->HasOploteLoot(target->GetGUID()))
                        modelID = GetGOInfo()->challengeModeReward.WhenAvailable;
                }
                else if (GetGoType() == GAMEOBJECT_TYPE_GARRISON_SHIPMENT)
                {
                    if (Garrison* g = target->GetGarrisonPtr())
                        modelID = g->GoBuildValuesUpdate(this, index, m_uint32Values[index]);
                }                    

                *data << modelID;
            }
            else
            {
                //! custom field support by g
                if (GetGoType() == GAMEOBJECT_TYPE_GARRISON_SHIPMENT)
                {   
                    if (Garrison* g = target->GetGarrisonPtr())
                    {
                        *data << g->GoBuildValuesUpdate(this, index, m_uint32Values[index]);
                        continue;   // not do in this case *data << m_uint32Values[index];
                    }
                }
                *data << m_uint32Values[index];                // other cases
            }
        }
    }
}

void GameObject::GetRespawnPosition(float &x, float &y, float &z, float* ori /* = nullptr*/) const
{
    if (m_DBTableGuid)
    {
        if (GameObjectData const* data = sObjectMgr->GetGOData(m_DBTableGuid))
        {
            x = data->posX;
            y = data->posY;
            z = data->posZ;
            if (ori)
                *ori = data->orientation;
            return;
        }
    }

    x = GetPositionX();
    y = GetPositionY();
    z = GetPositionZ();
    if (ori)
        *ori = GetOrientation();
}

float GameObject::GetInteractionDistance() const
{
    switch (GetGoType())
    {
        /// @todo find out how the client calculates the maximal usage distance to spellless working
        // gameobjects like guildbanks and mailboxes - 10.0 is a just an abitrary choosen number
        case GAMEOBJECT_TYPE_GUILD_BANK:
        case GAMEOBJECT_TYPE_MAILBOX:
            return 10.0f;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_FISHINGNODE:
            return 20.0f + CONTACT_DISTANCE; // max spell range
        default:
            return INTERACTION_DISTANCE;
    }
}

void GameObject::UpdateModelPosition(bool full)
{
    if (!m_model)
        return;

    if (GetMap()->ContainsGameObjectModel(*m_model))
    {
        if (full)
            GetMap()->RemoveGameObjectModel(*m_model);
        m_model->UpdatePosition();
        if (full)
            GetMap()->InsertGameObjectModel(*m_model);
    }
}

uint32 GameObject::CalculateAnimDuration(GOState oldState, GOState newState) const
{
    if (oldState == newState || oldState >= MAX_GO_STATE || newState >= MAX_GO_STATE)
        return 0;

    auto const& itr = sDB2Manager._transportAnimationsByEntry.find(GetEntry());
    if (itr == sDB2Manager._transportAnimationsByEntry.end())
        return 0;

    uint32 frameByState[MAX_GO_STATE] = { 0, m_goInfo->transport.Timeto2ndfloor, m_goInfo->transport.Timeto3rdfloor };
    if (oldState == GO_STATE_ACTIVE)
        return frameByState[newState];

    if (newState == GO_STATE_ACTIVE)
        return frameByState[oldState];

    return uint32(std::abs(int32(frameByState[oldState]) - int32(frameByState[newState])));
}

bool GameObject::IsPersonal() const
{
    if(GetGOInfo()->chest.chestPersonalLoot)
        return true;

    if(GetGOInfo()->IsOploteChest())
        return true;

    return false;
}

void GameObject::AddToSkillupList(ObjectGuid const& PlayerGuidLow)
{
    m_SkillupList.insert(PlayerGuidLow);
}

bool GameObject::IsInSkillupList(ObjectGuid const& playerGuid) const
{
    return m_SkillupList.count(playerGuid) > 0;
}

void GameObject::ClearSkillupList()
{
    m_SkillupList.clear();
}

void GameObject::setVisibilityCDForPlayer(ObjectGuid const& guid, uint32 sec/* = 300*/)
{
    // By default set 5 min.
    m_lastUser[guid] = time(nullptr) + sec;
}

void GameObject::GameObjectAction()
{
    if (!m_actionActive)
        return;

    std::list<Player*> playerList;
    GetPlayerListInGrid(playerList, m_maxActionDistance);
    if (playerList.empty())
        return;

    for (const auto& itr : *m_actionVector)
    {
        for (auto player : playerList)
        {
            if (player->GetDistance2d(this) > float(itr.Distance))
                continue;

            if ((itr.SpellID || itr.WorldSafeLocID) && (itr.X != 0.0f || itr.Y != 0.0f || itr.Z != 0.0f))
                if (GetExactDist(itr.X, itr.Y, itr.Z) > float(itr.Distance))
                    continue;

            if (itr.SpellID)
                player->CastSpell(player, itr.SpellID, true);

            if (auto safelockID = itr.WorldSafeLocID)
            {
                if (auto entry = sWorldSafeLocsStore.LookupEntry(safelockID))
                    player->TeleportTo(entry->MapID, entry->Loc.X, entry->Loc.Y, entry->Loc.Z, float(entry->Loc.O * M_PI / 180.0f));
            }
            else if (itr.X != 0.0f || itr.Y != 0.0f || itr.Z != 0.0f)
                player->TeleportTo(itr.MapID, itr.X, itr.Y, itr.Z, itr.O);
        }
    }
}

void GameObject::CalculatePassengerPosition(float& x, float& y, float& z, float* /*o*/)
{
    float inx = x, iny = y, inz = z;
    x = GetPositionX() + inx * std::cos(GetOrientation()) - iny * std::sin(GetOrientation());
    y = GetPositionY() + iny * std::cos(GetOrientation()) + inx * std::sin(GetOrientation());
    z = GetPositionZ() + inz;
}

//! This method transforms supplied global coordinates into local offsets
void GameObject::CalculatePassengerOffset(float& x, float& y, float& z, float* /*o*/)
{
    z -= GetPositionZ();
    y -= GetPositionY();
    x -= GetPositionX();
    float inx = x, iny = y;
    y = (iny - inx * tan(GetOrientation())) / (cos(GetOrientation()) + std::sin(GetOrientation()) * tan(GetOrientation()));
    x = (inx + iny * tan(GetOrientation())) / (cos(GetOrientation()) + std::sin(GetOrientation()) * tan(GetOrientation()));
}

class GameObjectModelOwnerImpl : public GameObjectModelOwnerBase
{
public:
    explicit GameObjectModelOwnerImpl(GameObject const* owner) : _owner(owner) { }

    bool IsSpawned() const override { return _owner->isSpawned(); }
    uint32 GetDisplayId() const override { return _owner->GetDisplayId(); }
    uint8 GetNameSetId() const override { return _owner->GetNameSetId(); }
    bool InSamePhaseId(std::set<uint32> const& phases, bool otherIsPlayer) const override { return _owner->InSamePhaseId(phases, otherIsPlayer); }
    uint32 GetPhaseMask() const override { return _owner->GetPhaseMask(); }
    G3D::Vector3 GetPosition() const override { return G3D::Vector3(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ()); }
    float GetOrientation() const override { return _owner->GetOrientation(); }
    float GetScale() const override { return _owner->GetObjectScale(); }
    bool IsDoor() const override  { return _owner->GetGoType() == GAMEOBJECT_TYPE_DOOR; }
    GameObject const* GetOwner() const override  { return _owner; }
    uint32 GetGUIDLow() const override  { return _owner->GetGUIDLow(); }

    void DebugVisualizeCorner(G3D::Vector3 const& corner) const override
    {
        Position pos;
        pos.Relocate(corner.x, corner.y, corner.z, 0.0f);
        _owner->SummonCreature(1, pos, TEMPSUMMON_MANUAL_DESPAWN);
    }

private:
    GameObject const* _owner;
};

GameObjectModel* GameObject::CreateModel()
{
    return GameObjectModel::Create(Trinity::make_unique<GameObjectModelOwnerImpl>(this), sWorld->GetDataPath());
}

bool GameObject::DistanceCheck()
{
    return !MaxVisible;
}
