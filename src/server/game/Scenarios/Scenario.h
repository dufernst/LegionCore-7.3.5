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

#ifndef TRINITY_SCENARIO_H
#define TRINITY_SCENARIO_H

#include "Common.h"
#include "AchievementMgr.h"
#include "Challenge.h"
#include <safe_ptr.h>

class Challenge;

struct ScenarioStepEntry;

typedef std::vector<ScenarioStepEntry const*> ScenarioSteps;

namespace WorldPackets
 {
     namespace Scenario
     {
         struct BonusObjectiveData;
     }
 }

namespace lfg
{
    struct LFGDungeonData;
}

enum ScenarioType
{
    SCENARIO_TYPE_DEFAULT               = 0,
    SCENARIO_TYPE_CHALLENGE_MODE        = 1,
    SCENARIO_TYPE_PROVING_GROUNDS       = 2,
    SCENARIO_TYPE_USE_DUNGEON_DISPLAY   = 3,
    SCENARIO_TYPE_LEGION_INVASION       = 4,
    SCENARIO_TYPE_BOOST_TUTORIAL        = 5,
};

enum ScenarioStepState
{
    SCENARIO_STEP_INVALID       = 0,
    SCENARIO_STEP_NOT_STARTED   = 1,
    SCENARIO_STEP_IN_PROGRESS   = 2,
    SCENARIO_STEP_DONE          = 3
};

class Scenario
{
public:
    Scenario(Map* map, lfg::LFGDungeonData const* _dungeonData, Player* player, bool find);
    Scenario(Map* map, uint32 scenarioID);
    ~Scenario();

    void Update(const uint32 t_diff);
    void CreateChallenge(Player* player);

    uint32 GetInstanceId() const;
    Map* GetMap();
    OutdoorPvP* GetOutdoorPvP();
    uint32 GetScenarioId() const;
    uint32 GetCurrentStep() const;
    uint32 GetOutdoorPvPZone() const;

    void SetStepState(ScenarioStepEntry const* step, ScenarioStepState state);
    ScenarioStepState GetStepState(ScenarioStepEntry const* step);

    bool IsCompleted(bool bonus) const;
    uint8 GetStepCount(bool withBonus) const;
    void UpdateCurrentStep(bool loading);
    void SetCurrentStep(uint8 step);
    void Reward(bool bonus, uint32 rewardStep);

    AchievementMgr<Scenario>& GetAchievementMgr();
    AchievementMgr<Scenario> const& GetAchievementMgr() const;
    void UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 = 0, uint32 miscValue2 = 0, uint32 miscValue3 = 0, Unit* unit = nullptr, Player* referencePlayer = nullptr);

    std::vector<WorldPackets::Scenario::BonusObjectiveData> GetBonusObjectivesData();
    void SendStepUpdate(Player* player = nullptr, bool full = false);
    void SendFinishPacket(Player* player);
    void SendCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed = 0);
    void BroadCastPacket(const WorldPacket* data);
    uint32 GetScenarioCriteriaByStep(uint8 step);

    bool CanUpdateCriteria(uint32 criteriaTreeId, uint32 recursTree = 0) const;

    Challenge* GetChallenge();
    sf::contention_free_shared_mutex< > i_updateLock;

    void SetOutdoorPvP(OutdoorPvP* outdoor, uint32 zone);

protected:
    Challenge* _challenge;
    ScenarioEntry const* _scenarioEntry;
    std::map<ScenarioStepEntry const*, ScenarioStepState> _stepStates;

    uint32 instanceId;
    uint32 scenarioId;
    lfg::LFGDungeonData const* dungeonData;
    AchievementMgr<Scenario> m_achievementMgr;
    Map* curMap;

    uint8 currentStep;
    uint32 currentTree;
    ScenarioSteps steps;
    std::vector<uint32> ActiveSteps;

    bool rewarded;
    bool bonusRewarded;
    bool hasbonus;

    OutdoorPvP* m_outdoorPvp = nullptr;
    uint32 m_outdoorPvpZone = 0;
};

#endif
