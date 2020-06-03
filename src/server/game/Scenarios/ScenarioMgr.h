/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
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

#ifndef TRINITY_SCENARIOMGR_H
#define TRINITY_SCENARIOMGR_H

#include "Common.h"
#include "Scenario.h"

class Scenario;
struct ScenarioStepEntry;

typedef std::vector<ScenarioStepEntry const*> ScenarioSteps;
typedef std::map<uint32 /*instance_id*/, Scenario*> ScenarioMap;
typedef std::map<uint32, ScenarioSteps> ScenarioStepsByScenarioMap;
typedef std::vector<ScenarioSteps*> ScenarioStepsByScenarioVector;

class ScenarioMgr
{
public:
    ScenarioMgr();
    ~ScenarioMgr();

    void UnloadAll();
    static ScenarioMgr* instance();

    Scenario* AddScenario(Map* map, lfg::LFGDungeonData const* _dungeonData, Player* player, bool find = false);
    void RemoveScenario(uint32 instanceId);
    Scenario* GetScenario(uint32 instanceId);

    ScenarioSteps const* GetScenarioSteps(uint32 scenarioId, bool Teeming = false);
    bool HasScenarioStep(lfg::LFGDungeonData const* _dungeonData, Player* player);
    bool HasAffixesTeeming(uint16 CriteriaTreeID);

private:
    ScenarioMap _scenarioStore;

    ScenarioStepsByScenarioMap m_stepMap;
    ScenarioStepsByScenarioMap m_stepTeemingMap;
    ScenarioStepsByScenarioVector m_stepVector;
};

#define sScenarioMgr ScenarioMgr::instance()

#endif
