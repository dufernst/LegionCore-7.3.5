/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "Anticheat.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Common.h"
#include "GameObjectAI.h"
#include "Log.h"
#include "MapManager.h"
#include "MMapFactory.h"
#include "MMapManager.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptsData.h"
#include "ScriptMgr.h"
#include "Spline.h"
#include "Totem.h"
#include "Transport.h"
#include "UpdateData.h"
#include "Vehicle.h"
#include "ZoneScript.h"
#include <G3D/Vector3.h>

Transport::Transport() : GameObject(),
    _transportInfo(nullptr), _isMoving(true), _pendingStop(false),
    _triggeredArrivalEvent(false), _triggeredDepartureEvent(false),
    _delayedAddModel(false)
{
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;
}

Transport::~Transport()
{
    ASSERT(_passengers.empty());
    UnloadStaticPassengers();
}

bool Transport::CreateTransport(ObjectGuid::LowType guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress)
{
    Relocate(x, y, z, ang);
    m_stationaryPosition.Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_TRANSPORTS, "Transport (GUID: " UI64FMTD ") not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, x, y);
        return false;
    }

    Object::_Create(ObjectGuid::Create<HighGuid::Transport>(guidlow));

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Transport not created: entry in `gameobject_template` not found, guidlow: " UI64FMTD " map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    TransportTemplate const* tInfo = sTransportMgr->GetTransportTemplate(entry);
    if (!tInfo)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "Transport %u (name: %s) will not be created, missing `transport_template` entry.", entry, goinfo->name.c_str());
        return false;
    }

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, goinfo->flags);
    SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_TRANSPORT | GO_FLAG_NODESPAWN | GO_FLAG_MAP_OBJECT);

    m_goValue.Transport.PathProgress = 0;
    SetObjectScale(goinfo->size);
    SetPeriod(tInfo->pathTime);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    SetGoState(!goinfo->moTransport.allowstopping ? GO_STATE_READY : GO_STATE_ACTIVE);
    SetGoType(GAMEOBJECT_TYPE_MAP_OBJ_TRANSPORT);
    SetGoAnimProgress(animprogress);
    SetName(goinfo->name);
    // SetWorldRotation(0.0f, 0.0f, 0.0f, 1.0f);
    SetWorldRotationAngles(NormalizeOrientation(ang), 0.0f, 0.0f);
    G3D::Quat parentRotation(0.0f, 0.0f, 0.0f, 1.0f);
    SetParentRotation(parentRotation);

    m_model = CreateModel();
    MMAP::MMapFactory::createOrGetMMapManager()->loadGameObject(goinfo->displayId, sWorld->GetDataPath());
    return true;
}

void Transport::CleanupsBeforeDelete(bool finalCleanup /*= true*/)
{
    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void Transport::Update(uint32 diff)
{
    if (m_isUpdate)
        return;

    m_isUpdate = true;

    uint32 const positionUpdateDelay = 200;

    if (AI())
        AI()->UpdateAI(diff);
    else if (!AIM_Initialize())
        TC_LOG_ERROR(LOG_FILTER_TRANSPORTS, "Could not initialize GameObjectAI for Transport");

    if (GetKeyFrames().size() <= 1)
    {
        m_isUpdate = false;
        return;
    }

    if (IsMoving() || !_pendingStop)
        m_goValue.Transport.PathProgress += diff;

    uint32 timer = m_goValue.Transport.PathProgress % GetTransportPeriod();
    bool justStopped = false;

    // Set current waypoint
    // Desired outcome: _currentFrame->DepartureTime < timer < _nextFrame->ArriveTime
    // ... arrive | ... delay ... | departure
    //      event /         event /
    for (;;)
    {
        if (timer >= _currentFrame->ArriveTime)
        {
            if (!_triggeredArrivalEvent)
            {
                DoEventIfAny(*_currentFrame, false);
                _triggeredArrivalEvent = true;
            }

            if (timer < _currentFrame->DepartureTime)
            {
                SetMoving(false);
                justStopped = true;
                if (_pendingStop && GetGoState() != GO_STATE_READY)
                {
                    SetGoState(GO_STATE_READY);
                    m_goValue.Transport.PathProgress = (m_goValue.Transport.PathProgress / GetTransportPeriod());
                    m_goValue.Transport.PathProgress *= GetTransportPeriod();
                    m_goValue.Transport.PathProgress += _currentFrame->ArriveTime;
                }
                break;  // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

        // not waiting anymore
        SetMoving(true);

        // Enable movement
        if (GetGOInfo()->moTransport.allowstopping)
            SetGoState(GO_STATE_ACTIVE);

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWaypoint();

        sScriptMgr->OnRelocate(this, _currentFrame->Node->NodeIndex, _currentFrame->Node->ContinentID, _currentFrame->Node->Loc.X, _currentFrame->Node->Loc.Y, _currentFrame->Node->Loc.Z);

        TC_LOG_DEBUG(LOG_FILTER_TRANSPORTS, "Transport %u (%s) moved to node %u %u %f %f %f", GetEntry(), GetName(), _currentFrame->Node->NodeIndex, _currentFrame->Node->ContinentID, _currentFrame->Node->Loc.X, _currentFrame->Node->Loc.Y, _currentFrame->Node->Loc.Z);

        // Departure event
        if (_currentFrame->IsTeleportFrame())
            if (TeleportTransport(_nextFrame->Node->ContinentID, _nextFrame->Node->Loc.X, _nextFrame->Node->Loc.Y, _nextFrame->Node->Loc.Z, _nextFrame->InitialOrientation))
            {
                m_isUpdate = false;
                return; // Update more in new map thread
            }
    }

    // Add model to map after we are fully done with moving maps
    if (_delayedAddModel)
    {
        _delayedAddModel = false;
        if (m_model)
            GetMap()->InsertGameObjectModel(*m_model);
    }

    // Set position
    _positionChangeTimer.Update(diff);
    if (_positionChangeTimer.Passed())
    {
        _positionChangeTimer.Reset(positionUpdateDelay);
        if (IsMoving())
        {
            float t = !justStopped ? CalculateSegmentPos(float(timer) * 0.001f) : 1.0f;
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, std::atan2(dir.y, dir.x) + float(M_PI));
        }
        else if (justStopped)
            UpdatePosition(_currentFrame->Node->Loc.X, _currentFrame->Node->Loc.Y, _currentFrame->Node->Loc.Z, _currentFrame->InitialOrientation);
        else
        {
            /* There are four possible scenarios that trigger loading/unloading passengers:
              1. transport moves from inactive to active grid
              2. the grid that transport is currently in becomes active
              3. transport moves from active to inactive grid
              4. the grid that transport is currently in unloads
            */
            bool gridActive = GetMap()->IsGridLoaded(GetPositionX(), GetPositionY());

            if (_staticPassengers.empty() && gridActive) // 2.
                LoadStaticPassengers();
            else if (!_staticPassengers.empty() && !gridActive)
                // 4. - if transports stopped on grid edge, some passengers can remain in active grids
                //      unload all static passengers otherwise passengers won't load correctly when the grid that transport is currently in becomes active
                UnloadStaticPassengers();
        }
    }

    // sScriptMgr->OnTransportUpdate(this, diff);
    m_isUpdate = false;
}

void Transport::AddPassenger(WorldObject* passenger)
{
    if (!IsInWorld())
        return;

    if (_passengers.insert(passenger))
    {
        passenger->SetTransport(this);
        passenger->m_movementInfo.transport.Guid = GetGUID();

        if (Player* plr = passenger->ToPlayer())
        {
            sScriptMgr->OnAddPassenger(this, plr);

            for (Unit::ControlList::const_iterator itr = plr->m_Controlled.begin(); itr != plr->m_Controlled.end(); ++itr)
                if(Unit* unit = ObjectAccessor::GetUnit(*plr, *itr))
                    unit->SetTratsport(this, plr);

            plr->GetCheatData()->OnTransport(plr, GetGUID());
        }
    }
}

void Transport::RemovePassenger(WorldObject* passenger)
{
    bool erased = _passengers.erase(worldObjectHashGen(passenger));
    if (erased || _staticPassengers.erase(worldObjectHashGen(passenger))) // static passenger can remove itself in case of grid unload
    {
        passenger->SetTransport(nullptr);
        passenger->m_movementInfo.transport.Reset();

        if (Player* plr = passenger->ToPlayer())
        {
            sScriptMgr->OnRemovePassenger(this, plr);

            for (Unit::ControlList::const_iterator itr = plr->m_Controlled.begin(); itr != plr->m_Controlled.end(); ++itr)
            {
                if(Unit* unit = ObjectAccessor::GetUnit(*plr, *itr))
                {
                    RemovePassenger(unit);
                    unit->SetTratsport(nullptr, plr);
                }
            }
        }
    }
}

Creature* Transport::CreateNPCPassenger(ObjectGuid::LowType guid, CreatureData const* data)
{
    Map* map = GetMap();
    Creature* creature = new Creature();
    creature->SetTransport(this); // Need to check if creature on transport

    if (!creature->LoadCreatureFromDB(guid, map, false))
    {
        delete creature;
        return nullptr;
    }

    ASSERT(data);

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    creature->m_movementInfo.transport.Guid = GetGUID();
    creature->m_movementInfo.transport.Pos.Relocate(x, y, z, o);
    creature->m_movementInfo.transport.VehicleSeatIndex = -1;
    CalculatePassengerPosition(x, y, z, &o);
    creature->Relocate(creature->m_movementInfo.transport.Pos);
    // creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetHomePosition(creature->m_movementInfo.transport.Pos);
    creature->SetTransportHomePosition(creature->m_movementInfo.transport.Pos);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_TRANSPORTS, "Passenger %s not created. Suggested coordinates aren't valid (X: %f Y: %f)", creature->GetGUID().ToString().c_str(), creature->GetPositionX(), creature->GetPositionY());
        delete creature;
        return nullptr;
    }

    creature->SetPhaseId(data->PhaseID, false);

    map->AddToMapWait(creature);

    _staticPassengers.insert(creature);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

GameObject* Transport::CreateGOPassenger(ObjectGuid::LowType guid, GameObjectData const* data)
{
    Map* map = GetMap();
    GameObject* go = new GameObject();

    if (!go->LoadGameObjectFromDB(guid, map, false))
    {
        delete go;
        return nullptr;
    }

    ASSERT(data);

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    go->SetTransport(this);
    go->m_movementInfo.transport.Guid = GetGUID();
    go->m_movementInfo.transport.Pos.Relocate(x, y, z, o);
    go->m_movementInfo.transport.VehicleSeatIndex = -1;
    CalculatePassengerPosition(x, y, z, &o);
    go->Relocate(x, y, z, o);
    go->RelocateStationaryPosition(x, y, z, o);

    if (!go->IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_TRANSPORTS, "Passenger %s not created. Suggested coordinates aren't valid (X: %f Y: %f)", go->GetGUID().ToString().c_str(), go->GetPositionX(), go->GetPositionY());
        delete go;
        return nullptr;
    }

    map->AddToMapWait(go);

    _staticPassengers.insert(go);
    return go;
}

TempSummon* Transport::SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties /*= NULL*/, uint32 duration /*= 0*/, Unit* summoner /*= NULL*/, uint32 spellId /*= 0*/, uint32 vehId /*= 0*/)
{
    Map* map = FindMap();
    if (!map)
        return nullptr;

    uint32 mask = UNIT_MASK_SUMMON;
    if (properties)
    {
        switch (properties->Control)
        {
            case SUMMON_CATEGORY_PET:
                mask = UNIT_MASK_GUARDIAN;
                break;
            case SUMMON_CATEGORY_PUPPET:
                mask = UNIT_MASK_PUPPET;
                break;
            case SUMMON_CATEGORY_VEHICLE:
                mask = UNIT_MASK_MINION;
                break;
            case SUMMON_CATEGORY_WILD:
            case SUMMON_CATEGORY_ALLY:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Title)
                {
                    case SUMMON_TYPE_MINION:
                    case SUMMON_TYPE_GUARDIAN:
                    case SUMMON_TYPE_GUARDIAN2:
                        mask = UNIT_MASK_GUARDIAN;
                        break;
                    case SUMMON_TYPE_TOTEM:
                    case SUMMON_TYPE_LIGHTWELL:
                        mask = UNIT_MASK_TOTEM;
                        break;
                    case SUMMON_TYPE_VEHICLE:
                    case SUMMON_TYPE_VEHICLE2:
                        mask = UNIT_MASK_SUMMON;
                        break;
                    case SUMMON_TYPE_MINIPET:
                        mask = UNIT_MASK_MINION;
                        break;
                    default:
                        if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                            mask = UNIT_MASK_GUARDIAN;
                        break;
                }
                break;
            }
            default:
                return nullptr;
        }
    }

    TempSummon* summon = nullptr;
    switch (mask)
    {
        case UNIT_MASK_SUMMON:
            summon = new TempSummon(properties, summoner, false);
            break;
        case UNIT_MASK_GUARDIAN:
            summon = new Guardian(properties, summoner, false);
            break;
        case UNIT_MASK_PUPPET:
            summon = new Puppet(properties, summoner);
            break;
        case UNIT_MASK_TOTEM:
            summon = new Totem(properties, summoner);
            break;
        case UNIT_MASK_MINION:
            summon = new Minion(properties, summoner, false);
            break;
        default:
            break;
    }

    float x, y, z, o;
    pos.GetPosition(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);

    if (!summon->Create(sObjectMgr->GetGenerator<HighGuid::Creature>()->Generate(), map, 0, entry, vehId, 0, x, y, z, o))
    {
        delete summon;
        return nullptr;
    }

    summon->SetPhaseId(summoner ? summoner->GetPhases() : GetPhases(), false);

    summon->SetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL, spellId);

    summon->SetTransport(this);
    summon->m_movementInfo.transport.Guid = GetGUID();
    summon->m_movementInfo.transport.Pos.Relocate(pos);
    summon->Relocate(x, y, z, o);
    summon->SetHomePosition(x, y, z, o);
    summon->SetTransportHomePosition(pos);

    summon->InitStats(duration);

    map->AddToMapWait(summon);

    _staticPassengers.insert(summon);

    summon->InitSummon();
    summon->SetTempSummonType(summonType);

    return summon;
}

void Transport::UpdatePosition(float x, float y, float z, float o)
{
    bool newActive = GetMap()->IsGridLoaded(x, y);
    Cell oldCell(GetPositionX(), GetPositionY());

    if (x != GetPositionX() || y != GetPositionY() || z != GetPositionZ()) // Stop update if transport stop
    {
        Relocate(x, y, z, o);
        UpdateModelPosition(oldCell.DiffGrid(Cell(GetPositionX(), GetPositionY())));
    }

    UpdatePassengerPositions(_passengers);

    /* There are four possible scenarios that trigger loading/unloading passengers:
      1. transport moves from inactive to active grid
      2. the grid that transport is currently in becomes active
      3. transport moves from active to inactive grid
      4. the grid that transport is currently in unloads
    */
    if (_staticPassengers.empty() && newActive) // 1.
        LoadStaticPassengers();
    else if (!_staticPassengers.empty() && !newActive && oldCell.DiffGrid(Cell(GetPositionX(), GetPositionY()))) // 3.
        UnloadStaticPassengers();
    else
        UpdatePassengerPositions(_staticPassengers);
    // 4. is handed by grid unload
}

void Transport::LoadStaticPassengers()
{
    if (uint32 mapId = GetGOInfo()->GetSpawnMap())
    {
        auto cells = sObjectMgr->GetMapObjectGuids(mapId, GetMap()->GetSpawnMode());
        if (!cells)
            return;

        for (const auto& cell : *cells)
        {
            // Creatures on transport
            auto guidEnd = cell.second.creatures.end();
            for (auto guidItr = cell.second.creatures.begin(); guidItr != guidEnd; ++guidItr)
                CreateNPCPassenger(*guidItr, sObjectMgr->GetCreatureData(*guidItr));

            // GameObjects on transport
            guidEnd = cell.second.gameobjects.end();
            for (auto guidItr = cell.second.gameobjects.begin(); guidItr != guidEnd; ++guidItr)
                CreateGOPassenger(*guidItr, sObjectMgr->GetGOData(*guidItr));
        }
    }
}

void Transport::UnloadStaticPassengers()
{
    while (!_staticPassengers.empty())
    {
        WorldObject* obj = *_staticPassengers.begin();
        obj->AddObjectToRemoveList();   // also removes from _staticPassengers
    }
}

void Transport::EnableMovement(bool enabled)
{
    if (!GetGOInfo()->moTransport.allowstopping)
        return;

    _pendingStop = !enabled;
}

void Transport::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

float Transport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = _transportInfo->accelTime;
    float accelDist = _transportInfo->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < _transportInfo->accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return segmentPos / frame.NextDistFromPrev;
}

bool Transport::TeleportTransport(uint32 newMapid, float x, float y, float z, float o)
{
    Map* oldMap = GetMap();

    if (oldMap->GetId() != newMapid)
    {
        UnloadStaticPassengers();
        Relocate(x, y, z, o);
        oldMap->RemoveFromMap<Transport>(this, false);

        for (WorldObjectSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
        {
            WorldObject* obj = *itr;
            if (!obj)
                continue;

            float destX, destY, destZ, destO;
            obj->m_movementInfo.transport.Pos.GetPosition(destX, destY, destZ, destO);
            TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

            switch (obj->GetTypeId())
            {
                case TYPEID_PLAYER:
                    if (!obj->ToPlayer()->isAlive())
                        obj->ToPlayer()->ResurrectPlayer(1.0f);
                    if (obj->ToPlayer()->IsChangeMap() || !obj->ToPlayer()->TeleportTo(newMapid, destX, destY, destZ, destO, TELE_TO_NOT_LEAVE_TRANSPORT))
                        RemovePassenger(obj);
                    break;
                case TYPEID_DYNAMICOBJECT:
                case TYPEID_AREATRIGGER:
                    obj->AddObjectToRemoveList();
                    break;
                default:
                    RemovePassenger(obj);
                    break;
            }
        }

        Map* newMap = sMapMgr->CreateBaseMap(newMapid);
        SetMap(newMap);

        ASSERT(GetMap());
        newMap->AddToMap<Transport>(this);
        return true;
    }
    else
    {
        // Teleport players, they need to know it
        for (WorldObjectSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
        {
            if ((*itr)->IsPlayer())
            {
                // will be relocated in UpdatePosition of the vehicle
                if (Unit* veh = (*itr)->ToUnit()->GetVehicleBase())
                    if (veh->GetTransport() == this)
                        continue;

                float destX, destY, destZ, destO;
                (*itr)->m_movementInfo.transport.Pos.GetPosition(destX, destY, destZ, destO);
                TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

                (*itr)->ToUnit()->NearTeleportTo(destX, destY, destZ, destO);
            }
        }

        UpdatePosition(x, y, z, o);
        return false;
    }
}

void Transport::UpdatePassengerPositions(WorldObjectSet& passengers)
{
    for (WorldObjectSet::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        WorldObject* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (!passenger || passenger->GetMap() != GetMap())
            continue;

        // if passenger is on vehicle we have to assume the vehicle is also on transport
        // and its the vehicle that will be updating its passengers
        if (Unit* unit = passenger->ToUnit())
            if (unit->GetVehicle())
                continue;

        // Do not use Unit::UpdatePosition here, we don't want to remove auras
        // as if regular movement occurred
        float x, y, z, o;
        passenger->m_movementInfo.transport.Pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);
        switch (passenger->GetTypeId())
        {
            case TYPEID_UNIT:
            {
                Creature* creature = passenger->ToCreature();
                GetMap()->CreatureRelocation(creature, x, y, z, o, false);
                creature->GetTransportHomePosition(x, y, z, o);
                CalculatePassengerPosition(x, y, z, &o);
                creature->SetHomePosition(x, y, z, o);
                break;
            }
            case TYPEID_PLAYER:
                //relocate only passengers in world and skip any player that might be still logging in/teleporting
                if (passenger->ToPlayer()->CanContact())
                    GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
                break;
            case TYPEID_GAMEOBJECT:
                GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o, false);
                passenger->ToGameObject()->RelocateStationaryPosition(x, y, z, o);
                break;
            case TYPEID_DYNAMICOBJECT:
                GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
                break;
            case TYPEID_AREATRIGGER:
                GetMap()->AreaTriggerRelocation(passenger->ToAreaTrigger(), x, y, z, o);
                break;
            default:
                break;
        }

        if (Unit* unit = passenger->ToUnit())
            if (Vehicle* vehicle = unit->GetVehicleKit())
                vehicle->RelocatePassengers();
    }
}

void Transport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32 eventid = departure ? node.Node->DepartureEventID : node.Node->ArrivalEventID)
    {
        TC_LOG_DEBUG(LOG_FILTER_TRANSPORTS, "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.Node->NodeIndex, GetName());
        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
        EventInform(eventid);
    }
}

void Transport::BuildUpdate(UpdateDataMapType& data_map)
{
    auto const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    for (const auto& player : players)
        if (player.getSource()->HaveAtClient(this))
            BuildFieldsUpdate(player.getSource(), data_map);

    ClearUpdateMask(true);
}

uint32 Transport::GetPathProgress() const
{
    return GetGOValue()->Transport.PathProgress;
}

StaticTransport::StaticTransport() : Transport()
{
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;
    isMapObject = false;
    hasStopFrame = false;
    nextStopFrame = 0;
    FrameUpdateTimer = 0;
    deltaTimer = 0;
    moveSpeed = 1.0f;
}

StaticTransport::~StaticTransport()
{
    ASSERT(_passengers.empty());
    UnloadStaticPassengers();
}

bool StaticTransport::Create(ObjectGuid::LowType guidlow, uint32 name_id, Map* map, uint32 phaseMask, Position const& pos, G3D::Quat const& rotation, uint32 animprogress, GOState go_state, uint32 artKit, uint32 aid, GameObjectData const* data)
{
    ASSERT(map);
    SetMap(map);

    Relocate(pos);
    m_stationaryPosition.Relocate(pos);
    if (!IsPositionValid())
    {
        TC_LOG_ERROR(LOG_FILTER_GENERAL, "StaticTransport (GUID: %u Entry: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", guidlow, name_id, pos.GetPositionX(), pos.GetPositionY());
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
        TC_LOG_ERROR(LOG_FILTER_SQL, "StaticTransport (GUID: %u Entry: %u) not created: non-existing entry in `gameobject_template`. Map: %u (X: %f Y: %f Z: %f)", guidlow, name_id, map->GetId(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        return false;
    }

    m_goInfo = goinfo;

    Object::_Create(ObjectGuid::Create<HighGuid::Transport>(guidlow));

    if (goinfo->type >= MAX_GAMEOBJECT_TYPE)
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "StaticTransport (GUID: %u Entry: %u) not created: non-existing GO type '%u' in `gameobject_template`. It will crash client if created.", guidlow, name_id, goinfo->type);
        return false;
    }

    // pussywizard: temporarily calculate WorldRotation from orientation, do so until values in db are correct
    //SetWorldRotation( /*for StaticTransport we need 2 rotation Quats in db for World- and Path- Rotation*/ );
    SetWorldRotationAngles(NormalizeOrientation(GetOrientation()), 0.0f, 0.0f);
    // pussywizard: PathRotation for StaticTransport (only StaticTransports have PathRotation)
    if (rotation.z != 0.0f || rotation.w != 0.0f)
        SetParentRotation(rotation);
    else
    {
        G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(NormalizeOrientation(GetOrientation()), 0.0f, 0.0f));
        SetParentRotation(quat);
    }

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, goinfo->flags);

    SetEntry(goinfo->entry);
    SetName(goinfo->name);

    SetDisplayId(goinfo->displayId);

    m_model = CreateModel();
    MMAP::MMapFactory::createOrGetMMapManager()->loadGameObject(goinfo->displayId, sWorld->GetDataPath());

    SetGoType(GameobjectTypes(goinfo->type));
    SetGoState(go_state);
    SetGoArtKit(artKit);
    SetAnimKitId(aid);

    isMapObject = HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_MAP_OBJECT);
    if (isMapObject)
        MaxVisible = true;

    m_goValue.Transport.AnimationInfo = sTransportMgr->GetTransportAnimInfo(goinfo->entry);
    m_goValue.Transport.PathProgress = 0;
    m_goValue.Transport.StopFrames = new std::vector<uint32>();
    if (goinfo->transport.Timeto2ndfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto2ndfloor);
    if (goinfo->transport.Timeto3rdfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto3rdfloor);
    if (goinfo->transport.Timeto4thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto4thfloor);
    if (goinfo->transport.Timeto5thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto5thfloor);
    if (goinfo->transport.Timeto6thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto6thfloor);
    if (goinfo->transport.Timeto7thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto7thfloor);
    if (goinfo->transport.Timeto8thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto8thfloor);
    if (goinfo->transport.Timeto9thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto9thfloor);
    if (goinfo->transport.Timeto10thfloor > 0)
        m_goValue.Transport.StopFrames->push_back(goinfo->transport.Timeto10thfloor);

    if (goinfo->transport.startOpen)
        SetGoState(GOState(GO_STATE_TRANSPORT_STOPPED + (goinfo->transport.startOpen - 1)));
    else
        SetGoState(GO_STATE_TRANSPORT_ACTIVE);

    uint32 transportPeriod = GetTransportPeriod();
    if (!m_goValue.Transport.StopFrames->empty() && transportPeriod)
    {
        hasStopFrame = true;
        deltaTimer = getMSTime() % transportPeriod;

        // Need transport offset for offlike use
        SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, getMSTime() - deltaTimer);

        // moveSpeed = float((*m_goValue.Transport.StopFrames)[0]) / (isMapObject ? 60000.0f : 20000.0f);
    }

    SetGoAnimProgress(animprogress);

    LastUsedScriptID = GetGOInfo()->ScriptId;
    AIM_Initialize();

    this->setActive(true);
    return true;
}

void StaticTransport::BuildUpdate(UpdateDataMapType& data_map)
{
    auto const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    for (const auto& player : players)
        if (player.getSource()->HaveAtClient(this))
            BuildFieldsUpdate(player.getSource(), data_map);

    ClearUpdateMask(true);
}

void StaticTransport::Update(uint32 diff)
{
    GameObject::Update(diff);

    if (!IsInWorld())
        return;

    _positionChangeTimer.Update(diff);
    if (_positionChangeTimer.Passed())
    {
        _positionChangeTimer.Reset(200);
        UpdatePassengerPositions(_passengers);
        if (_staticPassengers.empty())
            LoadStaticPassengers();
        else
            UpdatePassengerPositions(_staticPassengers);
    }

    if (!m_goValue.Transport.AnimationInfo)
        return;

    uint32 transportPeriod = GetTransportPeriod();
    if (!transportPeriod)
        return;

    if (isMapObject && GetGoState() != GO_STATE_TRANSPORT_ACTIVE || !IsMoving())
        return;

    if (GetGoState() == GO_STATE_TRANSPORT_ACTIVE)
        m_goValue.Transport.PathProgress += diff * moveSpeed;

    uint32 progress = m_goValue.Transport.PathProgress % transportPeriod;

    if (hasStopFrame)
    {
        UpdateUInt32Value(GAMEOBJECT_FIELD_LEVEL, getMSTime());
        FrameUpdateTimer += diff;
        if (FrameUpdateTimer >= (isMapObject ? 60000.0f : 20000.0f))
        {
            if (GetGoState() != GO_STATE_TRANSPORT_ACTIVE)
                SetTransportState(GO_STATE_TRANSPORT_ACTIVE);
            else
            {
                SetTransportState(GO_STATE_TRANSPORT_STOPPED);
                nextStopFrame++;
                if (nextStopFrame >= m_goValue.Transport.StopFrames->size())
                    nextStopFrame = 0;

                // moveSpeed = float(transportPeriod) / (isMapObject ? 60000.0f : 20000.0f);
            }

            FrameUpdateTimer -= (isMapObject ? 60000.0f : 20000.0f);
        }
    }
    else
        progress = getMSTime() % transportPeriod;

    SetUInt16Value(OBJECT_FIELD_DYNAMIC_FLAGS, 1, int16(float(progress) / float(transportPeriod) * 65535.0f), false);

    RelocateToProgress(progress);
}

void StaticTransport::RelocateToProgress(uint32 progress)
{
    TransportAnimationEntry const *curr = nullptr, *next = nullptr;
    float percPos;
    if (m_goValue.Transport.AnimationInfo->GetAnimNode(progress, curr, next, percPos))
    {
        // curr node offset
        G3D::Vector3 pos = G3D::Vector3(curr->Pos.X, curr->Pos.Y, curr->Pos.Z);

        // move by percentage of segment already passed
        pos += G3D::Vector3(percPos * (next->Pos.X - curr->Pos.X), percPos * (next->Pos.Y - curr->Pos.Y), percPos * (next->Pos.Z - curr->Pos.Z));

        // rotate path by PathRotation
        // pussywizard: PathRotation in db is only simple orientation rotation, so don't use sophisticated and not working code
        // reminder: WorldRotation only influences model rotation, not the path
        float sign = GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 2) >= 0.0f ? 1.0f : -1.0f;
        float pathRotAngle = sign * 2.0f * acos(GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 3));
        float cs = cos(pathRotAngle), sn = sin(pathRotAngle);
        float nx = pos.x * cs - pos.y * sn; 
        float ny = pos.x * sn + pos.y * cs;
        pos.x = nx;
        pos.y = ny;

        // add stationary position to the calculated offset
        pos += G3D::Vector3(GetStationaryX(), GetStationaryY(), GetStationaryZ());

        // rotate by AnimRotation at current segment
        // pussywizard: AnimRotation in dbc is only simple orientation rotation, so don't use sophisticated and not working code
        G3D::Quat currRot, nextRot;
        float percRot;
        m_goValue.Transport.AnimationInfo->GetAnimRotation(progress, currRot, nextRot, percRot);
        float signCurr = currRot.z >= 0.0f ? 1.0f : -1.0f;
        float oriRotAngleCurr = signCurr * 2.0f * acos(currRot.w);
        float signNext = nextRot.z >= 0.0f ? 1.0f : -1.0f;
        float oriRotAngleNext = signNext * 2.0f * acos(nextRot.w);
        float oriRotAngle = oriRotAngleCurr + percRot * (oriRotAngleNext - oriRotAngleCurr);

        // check if position is valid
        if (!Trinity::IsValidMapCoord(pos.x, pos.y, pos.z))
            return;

        // update position to new one
        // also adding simplified orientation rotation here
        UpdatePosition(pos.x, pos.y, pos.z, NormalizeOrientation(GetStationaryO() + oriRotAngle));
    }
}

void StaticTransport::UpdatePosition(float x, float y, float z, float o)
{
    if (!GetMap()->IsGridLoaded(x, y)) // pussywizard: should not happen, but just in case
        GetMap()->LoadGrid(x, y);

    GetMap()->GameObjectRelocation(this, x, y, z, o); // this also relocates the model
}

uint32 StaticTransport::GetTransportPeriod() const
{
    if (m_goValue.Transport.AnimationInfo)
        return m_goValue.Transport.AnimationInfo->TotalTime;

    return 0;
}
