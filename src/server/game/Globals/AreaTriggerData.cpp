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

#include "AreaTriggerData.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "QuestData.h"

AreaTriggerDataStoreMgr::AreaTriggerDataStoreMgr()
{
}

AreaTriggerDataStoreMgr::~AreaTriggerDataStoreMgr()
{
}

AreaTriggerDataStoreMgr* AreaTriggerDataStoreMgr::instance()
{
    static AreaTriggerDataStoreMgr instance;
    return &instance;
}

void AreaTriggerDataStoreMgr::LoadQuestAreaTriggers()
{
    uint32 oldMSTime = getMSTime();

    _questAreaTriggerStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT id, quest FROM areatrigger_questender");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadQuestAreaTriggers() >> Loaded 0 quest trigger points. DB table `areatrigger_questender` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 trigger_ID = fields[0].GetUInt32();
        uint32 quest_ID = fields[1].GetUInt32();

        if (!sAreaTriggerStore.LookupEntry(trigger_ID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadQuestAreaTriggers() >> Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", trigger_ID);
            continue;
        }

        Quest const* quest = sQuestDataStore->GetQuestTemplate(quest_ID);
        if (!quest)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadQuestAreaTriggers() >> Table `areatrigger_questender` has record (id: %u) for not existing quest %u", trigger_ID, quest_ID);
            continue;
        }

        if (!quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadQuestAreaTriggers() >> Table `areatrigger_questender` has record (id: %u) for not quest %u, but quest not have flag QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT. Trigger or quest flags must be fixed, quest modified to require objective.", trigger_ID, quest_ID);
            const_cast<Quest*>(quest)->SetSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT); // this will prevent quest completing without objective
        }

        _questAreaTriggerStore[trigger_ID].insert(quest_ID);

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadQuestAreaTriggers() >> Loaded %u quest trigger points in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AreaTriggerDataStoreMgr::LoadTavernAreaTriggers()
{
    uint32 oldMSTime = getMSTime();

    _tavernAreaTriggerStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT id FROM areatrigger_tavern");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadTavernAreaTriggers() >> Loaded 0 tavern triggers. DB table `areatrigger_tavern` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 Trigger_ID = fields[0].GetUInt32();

        if (!sAreaTriggerStore.LookupEntry(Trigger_ID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadTavernAreaTriggers() >>Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", Trigger_ID);
            continue;
        }

        _tavernAreaTriggerStore.insert(Trigger_ID);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadTavernAreaTriggers() >> Loaded %u tavern triggers in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AreaTriggerDataStoreMgr::LoadAreaTriggerScripts()
{
    uint32 oldMSTime = getMSTime();

    _areaTriggerScriptStore.clear();

    QueryResult result = WorldDatabase.Query("SELECT entry, ScriptName FROM areatrigger_scripts");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerScripts() >> Loaded 0 areatrigger scripts. DB table `areatrigger_scripts` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 Trigger_ID = fields[0].GetUInt32();
        const char *scriptName = fields[1].GetCString();

        // if (!sAreaTriggerStore.LookupEntry(Trigger_ID))
        // {
            // TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadAreaTriggerScripts() >>Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", Trigger_ID);
            // continue;
        // }

        _areaTriggerScriptStore[Trigger_ID] = sObjectMgr->GetScriptId(scriptName);
    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerScripts() >> Loaded %u areatrigger scripts in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports()
{
    uint32 oldMSTime = getMSTime();

    _areaTriggerStore.clear();
    sObjectMgr->ClearInstanceGraveYardStore();

                                                                //                                                        0            1                  2                  3                  4                   5
    QueryResult result = WorldDatabase.Query("SELECT id,  target_map, target_position_x, target_position_y, target_position_z, target_orientation FROM areatrigger_teleport");
    if (!result)
    {
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports() >> Loaded 0 area trigger teleport definitions. DB table `areatrigger_teleport` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        ++count;

        uint32 Trigger_ID = fields[0].GetUInt32();

        AreaTriggerStruct at;
        at.target_mapId = fields[1].GetUInt16();
        at.target_X = fields[2].GetFloat();
        at.target_Y = fields[3].GetFloat();
        at.target_Z = fields[4].GetFloat();
        at.target_Orientation = fields[5].GetFloat();

        sObjectMgr->AddInstanceGraveYard(at.target_mapId, at.target_X, at.target_Y, at.target_Z, at.target_Orientation);

        if (!sAreaTriggerStore.LookupEntry(Trigger_ID))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports() >> Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", Trigger_ID);
            continue;
        }

        if (!sMapStore.LookupEntry(at.target_mapId))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports() >> Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Trigger_ID, at.target_mapId);
            continue;
        }

        if (at.target_X == 0 && at.target_Y == 0 && at.target_Z == 0)
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports() >> Area trigger (ID:%u) target coordinates not provided.", Trigger_ID);
            continue;
        }

        _areaTriggerStore[Trigger_ID] = at;

    } while (result->NextRow());

    TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerTeleports() >> Loaded %u area trigger teleport definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

AreaTriggerStruct const* AreaTriggerDataStoreMgr::GetGoBackTrigger(uint32 Map) const
{
    bool useParentDbValue = false;
    uint32 parentId = 0;
    auto mapEntry = sMapStore.LookupEntry(Map);
    if (!mapEntry || mapEntry->CorpseMapID < 0)
        return nullptr;

    if (mapEntry->IsDungeon())
    {
        auto iTemplate = sObjectMgr->GetInstanceTemplate(Map);
        if (!iTemplate)
            return nullptr;

        parentId = iTemplate->Parent;
        useParentDbValue = true;
    }

    uint32 corpseMapID = uint32(mapEntry->CorpseMapID);
    for (auto itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
        if ((!useParentDbValue && itr->second.target_mapId == corpseMapID) || (useParentDbValue && itr->second.target_mapId == parentId))
        {
            auto atEntry = sAreaTriggerStore.LookupEntry(itr->first);
            if (atEntry && atEntry->ContinentID == Map)
                return &itr->second;
        }

    return nullptr;
}

AreaTriggerStruct const* AreaTriggerDataStoreMgr::GetMapEntranceTrigger(uint32 Map) const
{
    for (auto itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
        if (itr->second.target_mapId == Map)
            if (sAreaTriggerStore.LookupEntry(itr->first))
                return &itr->second;

    return nullptr;
}

uint32 AreaTriggerDataStoreMgr::GetAreaTriggerScriptId(uint32 trigger_id)
{
    auto i = _areaTriggerScriptStore.find(trigger_id);
    if (i != _areaTriggerScriptStore.end())
        return i->second;

    return 0;
}

void AreaTriggerDataStoreMgr::LoadAreaTriggerActionsAndData()
{
    _areaTriggerData.clear();
    _areaTriggerDataByEntry.clear();

    //                                                  0         1            2           3            4            5            6            7          8
    QueryResult result = WorldDatabase.Query("SELECT `entry`, `spellId`, `customEntry`, `Radius`, `RadiusTarget`, `Height`, `HeightTarget`, `Float4`, `Float5`,"
        //    9             10           11       12       13         14          15          16           17               18              19                20
        "`MoveCurveID`, `ElapsedTime`, `windX`, `windY`, `windZ`, `windSpeed`, `windType`, `polygon`, `MorphCurveID`, `FacingCurveID`, `ScaleCurveID`, `HasFollowsTerrain`,"
        //    21                    22                   23                 24
        "`HasAttached`, `HasAbsoluteOrientation`, `HasDynamicShape`, `HasFaceMovementDir`,`hasAreaTriggerBox`,`RollPitchYaw1X`, `RollPitchYaw1Y`, `RollPitchYaw1Z`, "
        //
        "`TargetRollPitchYawX`, `TargetRollPitchYawY`, `TargetRollPitchYawZ`, `DecalPropertiesId`, `Distance`, `isMoving`, `VisualID`, `Speed`, `RePatch`, `RePatchSpeed` FROM areatrigger_template");

    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 id = fields[i++].GetUInt32();
            uint32 spellId = fields[i++].GetUInt32();
            uint32 customEntry = fields[i++].GetUInt32();
            AreaTriggerInfo& info = id ? _areaTriggerData[id] : _areaTriggerDataByEntry[customEntry];
            info.spellId = spellId ? spellId : customEntry;
            info.customEntry = customEntry;
            info.Radius = fields[i++].GetFloat();
            info.RadiusTarget = fields[i++].GetFloat();
            info.Polygon.Height = fields[i++].GetFloat();
            info.Polygon.HeightTarget = fields[i++].GetFloat();
            info.LocationZOffset = fields[i++].GetFloat();
            info.LocationZOffsetTarget = fields[i++].GetFloat();
            info.MoveCurveID = fields[i++].GetUInt32();
            info.ElapsedTime = fields[i++].GetUInt32();
            info.windX = fields[i++].GetFloat();
            info.windY = fields[i++].GetFloat();
            info.windZ = fields[i++].GetFloat();
            info.windSpeed = fields[i++].GetFloat();
            info.windType = fields[i++].GetUInt32();
            info.polygon = fields[i++].GetUInt32();
            info.MorphCurveID = fields[i++].GetUInt32();
            info.FacingCurveID = fields[i++].GetUInt32();
            info.ScaleCurveID = fields[i++].GetUInt32();
            info.HasFollowsTerrain = fields[i++].GetUInt32();
            info.HasAttached = fields[i++].GetUInt32();
            info.HasAbsoluteOrientation = fields[i++].GetUInt32();
            info.HasDynamicShape = fields[i++].GetUInt32();
            info.HasFaceMovementDir = fields[i++].GetUInt32();
            info.hasAreaTriggerBox = fields[i++].GetUInt32();
            info.RollPitchYaw1X = fields[i++].GetFloat();
            info.RollPitchYaw1Y = fields[i++].GetFloat();
            info.RollPitchYaw1Z = fields[i++].GetFloat();
            info.TargetRollPitchYawX = fields[i++].GetFloat();
            info.TargetRollPitchYawY = fields[i++].GetFloat();
            info.TargetRollPitchYawZ = fields[i++].GetFloat();
            info.DecalPropertiesId = fields[i++].GetUInt32();
            info.Distance = fields[i++].GetFloat();
            info.isMoving = fields[i++].GetBool();
            info.VisualID = fields[i++].GetInt32();
            info.Speed = fields[i++].GetFloat();
            info.RePatch = fields[i++].GetBool();
            info.RePatchSpeed = fields[i++].GetFloat();

            if (info.isMoving && !info.Speed)
                info.Speed = 7.0f;

            if (info.RePatch && !info.RePatchSpeed)
                info.RePatchSpeed = info.Speed ? info.Speed : 7.0f;

            if (info.polygon && info.customEntry)
            {
                if (QueryResult resultPolygon = WorldDatabase.PQuery("SELECT `id`, `x`, `y` FROM areatrigger_polygon WHERE `entry` = '%u' AND `spellId` = '%u' AND `type` = 1 order by id", info.customEntry, info.spellId))
                {
                    do
                    {
                        Field* fieldP = resultPolygon->Fetch();
                        info.Polygon.Vertices.emplace_back(fieldP[1].GetFloat(), fieldP[2].GetFloat());
                    } while (resultPolygon->NextRow());
                }

                if (QueryResult resultPolygonTarget = WorldDatabase.PQuery("SELECT `id`, `x`, `y` FROM areatrigger_polygon WHERE `entry` = '%u' AND `spellId` = '%u' AND `type` = 2 order by id", info.customEntry, info.spellId))
                {
                    do
                    {
                        Field* fieldP = resultPolygonTarget->Fetch();
                        info.Polygon.VerticesTarget.emplace_back(fieldP[1].GetFloat(), fieldP[2].GetFloat());
                    } while (resultPolygonTarget->NextRow());
                }
            }

            //Create Box
            if (info.hasAreaTriggerBox)
            {
                info.polygon = 1;
                info.Polygon.Vertices.emplace_back(info.Radius, info.Polygon.Height);
                info.Polygon.Vertices.emplace_back(-info.Radius, info.Polygon.Height);
                info.Polygon.Vertices.emplace_back(-info.RadiusTarget, -info.Polygon.HeightTarget);
                info.Polygon.Vertices.emplace_back(info.RadiusTarget, -info.Polygon.HeightTarget);

                float halfX = info.Radius / 2.0f;
                float halfY = info.Polygon.Height / 2.0f;
                float halfZ = info.LocationZOffset / 2.0f;
                info.box = G3D::AABox({ -halfX, -halfY, -halfZ }, { halfX, halfY, halfZ });
            }

            if (info.windSpeed)
            {
                AreaTriggerAction actionEnter;
                actionEnter.id = 100;
                actionEnter.moment = AT_ACTION_MOMENT_ENTER;
                actionEnter.actionType = AT_ACTION_TYPE_APPLY_MOVEMENT_FORCE;
                actionEnter.targetFlags = AreaTriggerTargetFlags(AT_TARGET_FLAG_PLAYER);
                actionEnter.spellId = info.spellId;
                actionEnter.maxCharges = 0;
                actionEnter.chargeRecoveryTime = 0;
                actionEnter.hasAura = 0;
                actionEnter.hasAura2 = 0;
                actionEnter.hasAura3 = 0;
                actionEnter.hasspell = 0;
                actionEnter.scaleStep = 0.0f;
                actionEnter.scaleMin = 0.0f;
                actionEnter.scaleMax = 0.0f;
                actionEnter.scaleVisualUpdate = true;
                actionEnter.hitMaxCount = 0;
                actionEnter.amount = 0;
                actionEnter.onDespawn = false;
                actionEnter.auraCaster = 0;
                actionEnter.minDistance = 0.0f;
                info.actions.push_back(actionEnter);

                AreaTriggerAction actionLeave;
                actionLeave.id = 101;
                actionLeave.moment = AreaTriggerActionMoment(AT_ACTION_MOMENT_LEAVE | AT_ACTION_MOMENT_DESPAWN | AT_ACTION_MOMENT_REMOVE);
                actionLeave.actionType = AT_ACTION_TYPE_REMOVE_MOVEMENT_FORCE;
                actionLeave.targetFlags = AT_TARGET_FLAG_PLAYER;
                actionLeave.spellId = info.spellId;
                actionLeave.maxCharges = 0;
                actionLeave.chargeRecoveryTime = 0;
                actionLeave.hasAura = 0;
                actionLeave.hasAura2 = 0;
                actionLeave.hasAura3 = 0;
                actionLeave.hasspell = 0;
                actionLeave.scaleStep = 0.0f;
                actionLeave.scaleMin = 0.0f;
                actionLeave.scaleMax = 0.0f;
                actionLeave.scaleVisualUpdate = true;
                actionLeave.hitMaxCount = 0;
                actionLeave.amount = 0;
                actionLeave.onDespawn = false;
                actionLeave.auraCaster = 0;
                actionLeave.minDistance = 0.0f;
                info.actions.push_back(actionLeave);
            }
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded %u areatrigger template.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded 0 areatrigger template. DB table `areatrigger_template` is empty.");

    result = WorldDatabase.Query("SELECT `entry`, `customEntry`, `spellId`, `moveType`, `activationDelay`, `updateDelay`, `maxCount`, `hitType`, `waitTime`, `AngleToCaster`, `AnglePointA`, `AnglePointB`, `maxActiveTargets`, `Param`, `RandomRadiusOfSpawn`, `MoveEndDespawn`, `WithObjectSize`, `AliveOnly`, `AllowBoxCheck` FROM areatrigger_data");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 id = fields[i++].GetUInt32();
            uint32 customEntry = fields[i++].GetUInt32();
            AreaTriggerInfo& info = id ? _areaTriggerData[id] : _areaTriggerDataByEntry[customEntry];
            uint32 spellId = fields[i++].GetUInt32();
            info.moveType = fields[i++].GetUInt32();
            info.activationDelay = fields[i++].GetUInt32();
            info.updateDelay = fields[i++].GetUInt32();
            info.maxCount = fields[i++].GetUInt8();
            info.hitType = fields[i++].GetUInt32();
            info.waitTime = fields[i++].GetUInt32();
            info.AngleToCaster = fields[i++].GetFloat();
            info.AnglePointA = fields[i++].GetFloat();
            info.AnglePointB = fields[i++].GetFloat();
            info.maxActiveTargets = fields[i++].GetUInt8();
            info.Param = fields[i++].GetFloat();
            info.RandomRadiusOfSpawn = fields[i++].GetFloat();
            info.OnDestinationReachedDespawn = fields[i++].GetBool();
            info.WithObjectSize = fields[i++].GetBool();
            info.AliveOnly = fields[i++].GetBool();
            info.AllowBoxCheck = fields[i++].GetBool();
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded %u areatrigger data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded 0 areatrigger data. DB table `areatrigger_data` is empty.");

    result = WorldDatabase.Query("SELECT `entry`,`spellId`, `Radius`, `Speed`, `HasTarget`, `HasCenterPoint`, `IsReverse`, `IsActive`, `RandRevers`, `IsDinamicRadius` FROM areatrigger_template_circle");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 id = fields[i++].GetUInt32();
            AreaTriggerInfo& info = _areaTriggerData[id];
            info.isCircle = true;
            uint32 spellId = fields[i++].GetUInt32();
            info.circleTemplate.Radius = fields[i++].GetFloat();
            info.circleTemplate.Speed = fields[i++].GetFloat();
            info.circleTemplate.HasTarget = fields[i++].GetBool();
            info.circleTemplate.HasCenterPoint = fields[i++].GetBool();
            info.circleTemplate.CounterClockwise = fields[i++].GetBool();
            info.circleTemplate.CanLoop = fields[i++].GetBool();
            info.circleTemplate.RandRevers = fields[i++].GetBool();
            info.circleTemplate.IsDinamicRadius = fields[i++].GetBool();
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded %u areatrigger template circle.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded 0 areatrigger template circle. DB table `areatrigger_template_circle` is empty.");

    result = WorldDatabase.Query("SELECT `entry`,`spellId`, `oncreated`, `entered`, `animationid`,  `timer1`, `entered1`, `animationid1`, `timer2`, `entered2`, `animationid2`, `cycle` FROM areatrigger_template_sequence");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            uint32 id = fields[i++].GetUInt32();
            AreaTriggerInfo& info = _areaTriggerData[id];
            info.isSequence = true;
            uint32 spellId = fields[i++].GetUInt32();
            info.sequenceTemplate.oncreated = fields[i++].GetBool();
            info.sequenceTemplate.entered = fields[i++].GetBool();
            info.sequenceTemplate.animationid = fields[i++].GetUInt32();
            info.sequenceTemplate.timer1 = fields[i++].GetUInt32();
            info.sequenceTemplate.entered1 = fields[i++].GetBool();
            info.sequenceTemplate.animationid1 = fields[i++].GetUInt32();
            info.sequenceTemplate.timer2 = fields[i++].GetUInt32();
            info.sequenceTemplate.entered2 = fields[i++].GetBool();
            info.sequenceTemplate.animationid2 = fields[i++].GetUInt32();
            info.sequenceTemplate.cycle = fields[i++].GetBool();
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded %u areatrigger_template_sequence.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >> Loaded 0 areatrigger template circle. DB table `areatrigger_template_sequence` is empty.");

    //                                                0           1        2    3         4            5          6         7               8              9        10        11       12         13         14        15           16               17         18        19        20           21
    QueryResult result2 = WorldDatabase.Query("SELECT entry, customEntry, id, moment, actionType, targetFlags, spellId, maxCharges, chargeRecoveryTime, hasAura, hasAura2, hasAura3, hasspell, scaleStep, scaleMin, scaleMax, scaleVisualUpdate, hitMaxCount, amount, onDespawn, auraCaster, minDistance FROM areatrigger_actions");
    if (result2)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result2->Fetch();

            uint8 i = 0;
            AreaTriggerAction action;
            uint32 entry = fields[i++].GetUInt32();
            uint32 customEntry = fields[i++].GetUInt32();
            action.id = fields[i++].GetUInt32();
            action.moment = (AreaTriggerActionMoment)fields[i++].GetUInt32();
            action.actionType = (AreaTriggerActionType)fields[i++].GetUInt32();
            action.targetFlags = (AreaTriggerTargetFlags)fields[i++].GetUInt32();
            action.spellId = fields[i++].GetUInt32();
            action.maxCharges = fields[i++].GetInt8();
            action.chargeRecoveryTime = fields[i++].GetUInt32();
            action.hasAura = fields[i++].GetInt32();
            action.hasAura2 = fields[i++].GetInt32();
            action.hasAura3 = fields[i++].GetInt32();
            action.hasspell = fields[i++].GetInt32();
            action.scaleStep = fields[i++].GetFloat();
            action.scaleMin = fields[i++].GetFloat();
            action.scaleMax = fields[i++].GetFloat();
            action.scaleVisualUpdate = fields[i++].GetBool();
            action.hitMaxCount = fields[i++].GetInt32();
            action.amount = fields[i++].GetInt32();
            action.onDespawn = fields[i++].GetBool();
            action.auraCaster = fields[i++].GetInt32();
            action.minDistance = fields[i++].GetFloat();

            //TC_LOG_ERROR(LOG_FILTER_SQL, "DB table `areatrigger_actions` customEntry %u entry %u spellId %u", customEntry, entry, action.spellId);

            if (action.actionType >= AT_ACTION_TYPE_MAX)
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadAreaTriggerActionsAndData() >> DB table `areatrigger_actions` has invalid action type '%u' for areatrigger entry %u",
                    action.actionType, entry);
                continue;
            }

            if (action.actionType == AT_ACTION_TYPE_CHANGE_AMOUNT_FROM_HEALT && !(action.targetFlags & AT_TARGET_FLAG_NOT_FULL_HP))
                action.targetFlags = AreaTriggerTargetFlags(action.targetFlags | AT_TARGET_FLAG_NOT_FULL_HP);

            if (!sSpellMgr->GetSpellInfo(action.spellId))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "LoadAreaTriggerActionsAndData() >> DB table `areatrigger_actions` has non-existant spell id '%u' for areatrigger entry %u",
                    action.spellId, entry);
                continue;
            }

            AreaTriggerInfo& info = entry ? _areaTriggerData[entry] : _areaTriggerDataByEntry[customEntry];
            info.actions.push_back(action);
            ++counter;
        } while (result2->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >>  Loaded %u areatrigger actions.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "LoadAreaTriggerActionsAndData() >>  Loaded 0 areatrigger actions. DB table `areatrigger_actions` is empty.");
}

std::unordered_set<uint32> const* AreaTriggerDataStoreMgr::GetQuestsForAreaTrigger(uint32 areaTriggerID) const
{
    return Trinity::Containers::MapGetValuePtr(_questAreaTriggerStore, areaTriggerID);
}

bool AreaTriggerDataStoreMgr::IsTavernAreaTrigger(uint32 Trigger_ID) const
{
    return _tavernAreaTriggerStore.find(Trigger_ID) != _tavernAreaTriggerStore.end();
}

AreaTriggerStruct const* AreaTriggerDataStoreMgr::GetAreaTrigger(uint32 trigger) const
{
    return Trinity::Containers::MapGetValuePtr(_areaTriggerStore, trigger);
}

void AreaTriggerDataStoreMgr::LoadAreaTriggerForces()
{
    _areaTriggerForceMap.clear();

    //                                                  0             1          2        3        4         5            6           7          8          9
    QueryResult result = WorldDatabase.Query("SELECT `AuraID`, `CustomEntry`, `windX`, `windY`, `windZ`, `windType`, `windSpeed`, `centerX`, `centerY`, `centerZ` FROM `areatrigger_force`");
    if (result)
    {
        uint32 counter = 0;
        do
        {
            Field* fields = result->Fetch();

            uint8 i = 0;
            AreaTriggerForce data;
            data.AuraID = fields[i++].GetUInt32();
            data.CustomEntry = fields[i++].GetUInt32();
            float windX = fields[i++].GetFloat();
            float windY = fields[i++].GetFloat();
            float windZ = fields[i++].GetFloat();
            data.wind = Position(windX, windY, windZ);
            data.windType = fields[i++].GetUInt8();
            data.windSpeed = fields[i++].GetFloat();
            float centerX = fields[i++].GetFloat();
            float centerY = fields[i++].GetFloat();
            float centerZ = fields[i++].GetFloat();
            data.center = Position(centerX, centerY, centerZ);

            _areaTriggerForceMap[data.AuraID].push_back(data);
            ++counter;
        } while (result->NextRow());

        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerForces() >> Loaded %u areatrigger_force data.", counter);
    }
    else
        TC_LOG_INFO(LOG_FILTER_SERVER_LOADING, "AreaTriggerDataStoreMgr::LoadAreaTriggerForces() >> Loaded 0 areatrigger_force data. DB table `areatrigger_force` is empty.");
}

const std::vector<AreaTriggerForce>* AreaTriggerDataStoreMgr::GetAreaTriggerForce(uint32 AuraID) const
{
    return Trinity::Containers::MapGetValuePtr(_areaTriggerForceMap, AuraID);
}

AreaTriggerInfo const* AreaTriggerDataStoreMgr::GetAreaTriggerInfo(uint32 id)
{
    return Trinity::Containers::MapGetValuePtr(_areaTriggerData, id);
}

AreaTriggerInfo const* AreaTriggerDataStoreMgr::GetAreaTriggerInfoByEntry(uint32 entry)
{
    return Trinity::Containers::MapGetValuePtr(_areaTriggerDataByEntry, entry);
}
