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

#ifndef _AreaTriggerDataStoreh_
#define _AreaTriggerDataStoreh_

#include "AreaTrigger.h"

struct AreaTriggerStruct
{
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};
struct AreaTriggerForce
{
    uint32 AuraID;
    uint32 CustomEntry;
    TaggedPosition<Position::XYZ> wind;
    TaggedPosition<Position::XYZ> center;
    float windSpeed;
    uint8 windType;
};

typedef std::unordered_map<uint32/*entry*/, std::vector<AreaTriggerForce> > AreaTriggerForceMap;
typedef std::unordered_map<uint32, AreaTriggerInfo > AreaTriggerInfoMap;
typedef std::unordered_map<uint32, AreaTriggerStruct> AreaTriggerContainer;
typedef std::unordered_map<uint32, uint32> AreaTriggerScriptContainer;
typedef std::unordered_map<uint32, std::unordered_set<uint32>> QuestAreaTriggerContainer;
typedef std::set<uint32> TavernAreaTriggerContainer;

class AreaTriggerDataStoreMgr
{
    AreaTriggerDataStoreMgr();
    ~AreaTriggerDataStoreMgr();

public:
    static AreaTriggerDataStoreMgr* instance();

    void LoadAreaTriggerTeleports();
    void LoadQuestAreaTriggers();
    void LoadAreaTriggerScripts();
    void LoadTavernAreaTriggers();
    void LoadAreaTriggerForces();
    void LoadAreaTriggerActionsAndData();

    std::unordered_set<uint32> const* GetQuestsForAreaTrigger(uint32 areaTriggerID) const;

    bool IsTavernAreaTrigger(uint32 Trigger_ID) const;

    AreaTriggerStruct const* GetAreaTrigger(uint32 trigger) const;
    AreaTriggerStruct const* GetGoBackTrigger(uint32 Map) const;
    AreaTriggerStruct const* GetMapEntranceTrigger(uint32 Map) const;

    uint32 GetAreaTriggerScriptId(uint32 trigger_id);
    AreaTriggerInfo const* GetAreaTriggerInfo(uint32 id);
    AreaTriggerInfo const* GetAreaTriggerInfoByEntry(uint32 entry);
    std::vector<AreaTriggerForce> const* GetAreaTriggerForce(uint32 AuraID) const;
    void AddDataToQuestAreatriggerStore(uint32 questID, uint32 objID) { _questAreaTriggerStore[objID].insert(questID); }
private:
    QuestAreaTriggerContainer _questAreaTriggerStore;
    TavernAreaTriggerContainer _tavernAreaTriggerStore;
    AreaTriggerContainer _areaTriggerStore;
    AreaTriggerScriptContainer _areaTriggerScriptStore;
    AreaTriggerInfoMap _areaTriggerData;
    AreaTriggerInfoMap _areaTriggerDataByEntry;
    AreaTriggerForceMap _areaTriggerForceMap;
};

#define sAreaTriggerDataStore AreaTriggerDataStoreMgr::instance()

#endif
