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

#include "Group.h"
#include "ScenarioMgr.h"
#include "LFGMgr.h"
#include "InstanceSaveMgr.h"
#include "WorldSession.h"
#include "ScenarioPackets.h"
#include "InstanceScript.h"
#include "QuestData.h"
#include "OutdoorPvP.h"

Scenario::Scenario(Map* map, lfg::LFGDungeonData const* _dungeonData, Player* player, bool find) : m_achievementMgr(this)
{
    if (!map)
        return;

    curMap = map;
    instanceId = map->GetInstanceId();
    dungeonData = _dungeonData;
    currentStep = 0;
    currentTree = 0;
    bonusRewarded = false;
    rewarded = false;
    scenarioId = dungeonData->dbc->ScenarioID;
    _challenge = nullptr;
    hasbonus = false;

    if (player->GetScenarioId())
    {
        scenarioId = player->GetScenarioId();
        find = true;
        player->SetScenarioId(0);
    }

    if (!find)
        if (ScenarioData const* scenarioData = sObjectMgr->GetScenarioOnMap(dungeonData->map, map->GetDifficultyID(), player->GetTeam(), player->getClass(), dungeonData->id))
            scenarioId = scenarioData->ScenarioID;

    _scenarioEntry = sScenarioStore.LookupEntry(scenarioId);
    if (!_scenarioEntry)
    {
        //ASSERT(_scenarioEntry);
        // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::Scenario instanceId %u scenarioId %u dungeonData %u", instanceId, scenarioId, dungeonData->map);
        return;
    }

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::Scenario instanceId %u scenarioId %u dungeonData %u", instanceId, scenarioId, dungeonData->map);

    ScenarioSteps const* _steps = sScenarioMgr->GetScenarioSteps(scenarioId);
    ASSERT(_steps);

    steps = *_steps;
    currentTree = GetScenarioCriteriaByStep(currentStep);
    ActiveSteps.push_back(steps[currentStep]->ID);

    for (auto const& step : steps)
    {
        SetStepState(step, SCENARIO_STEP_NOT_STARTED);
        if (step->IsBonusObjective())
            hasbonus = true;
    }

    if (curMap->isChallenge())
        CreateChallenge(player);

    objectCountInWorld[uint8(HighGuid::Scenario)]++;
}

Scenario::Scenario(Map* map, uint32 _scenarioId) : m_achievementMgr(this)
{
    if (!map)
        return;

    curMap = map;
    instanceId = map->GetInstanceId();
    dungeonData = nullptr;
    currentStep = 0;
    currentTree = 0;
    bonusRewarded = false;
    rewarded = false;
    scenarioId = _scenarioId;
    _challenge = nullptr;
    hasbonus = false;

    _scenarioEntry = sScenarioStore.LookupEntry(scenarioId);
    if (!_scenarioEntry)
    {
        //ASSERT(_scenarioEntry);
        // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::Scenario instanceId %u scenarioId %u dungeonData %u", instanceId, scenarioId, dungeonData->map);
        return;
    }

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::Scenario instanceId %u scenarioId %u dungeonData %u", instanceId, scenarioId, dungeonData->map);

    ScenarioSteps const* _steps = sScenarioMgr->GetScenarioSteps(scenarioId);
    ASSERT(_steps);

    steps = *_steps;
    currentTree = GetScenarioCriteriaByStep(currentStep);
    ActiveSteps.push_back(steps[currentStep]->ID);

    for (auto const& step : steps)
    {
        SetStepState(step, SCENARIO_STEP_NOT_STARTED);
        if (step->IsBonusObjective())
            hasbonus = true;
    }

    objectCountInWorld[uint8(HighGuid::Scenario)]++;
}

Scenario::~Scenario()
{
    delete _challenge;
    _challenge = nullptr;

    objectCountInWorld[uint8(HighGuid::Scenario)]--;
}

void Scenario::Update(const uint32 t_diff)
{
    m_achievementMgr.UpdateTimedAchievements(t_diff);
}

void Scenario::CreateChallenge(Player* player)
{
    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "CreateChallenge player %u map %u", bool(player), bool(GetMap()));

    Map* map = GetMap();
    if (!player || !map)
        return;

    MapChallengeModeEntry const* m_challengeEntry = player->GetGroup() ? player->GetGroup()->m_challengeEntry : player->m_challengeKeyInfo.challengeEntry;
    if (!m_challengeEntry)
        return;

    _challenge = new Challenge(map, player, GetInstanceId(), this);

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "CreateChallenge _challenge %u _canRun %u", bool(_challenge), bool(_challenge->_canRun));

    if (!_challenge || !_challenge->_canRun)
        return;

    if (Map* map_ = GetMap())
    {
        if (InstanceMap* instanceMap = map_->ToInstanceMap())
        {
            if (InstanceScript* script = instanceMap->GetInstanceScript())
            {
                script->SetChallenge(_challenge);
                _challenge->SetInstanceScript(script);
            }
        }
    }

    if (ScenarioData const* scenarioData = sObjectMgr->GetScenarioOnMap(map->GetId(), DIFFICULTY_MYTHIC_KEYSTONE))
        scenarioId = scenarioData->ScenarioID;

    switch (m_challengeEntry->ID)
    {
        case 197: // Eye of Azshara
            scenarioId = 1169;
            break;
        case 198: // Darkheart Thicket
            scenarioId = 1172;
            break;
        case 199: // Black Rook Hold
            scenarioId = 1166;
            break;
        case 200: // Halls of Valor
            scenarioId = 1046;
            break;
        case 206: // Neltharion's Lair
            scenarioId = 1174;
            break;
        case 207: // Vault of the Wardens
            scenarioId = 1173;
            break;
        case 208: // Maw of Souls
            scenarioId = 1175;
            break;
        case 209: // The Arcway
            scenarioId = 1177;
            break;
        case 210: // Court of Stars
            scenarioId = 1178;
            break;
        case 227: // Return to Karazhan: Lower
            scenarioId = 1309;
            break;
        case 233: // Cathedral of Eternal Night
            scenarioId = 1335;
            break;
        case 234: // Return to Karazhan: Upper
            scenarioId = 1308;
            break;
        default:
            break;
    }

    _scenarioEntry = sScenarioStore.LookupEntry(scenarioId);
    ASSERT(_scenarioEntry);

    ScenarioSteps const* _steps = sScenarioMgr->GetScenarioSteps(scenarioId, _challenge->HasAffix(Affixes::Teeming));
    ASSERT(_steps);

    currentStep = 0;
    steps = *_steps;
    if (steps.size() <= currentStep)
    {
        // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "CreateChallenge steps %u currentStep %u", steps.size(), currentStep);
        return;
    }

    currentTree = GetScenarioCriteriaByStep(currentStep);
    ActiveSteps.clear();
    ActiveSteps.push_back(steps[currentStep]->ID);

    for (auto const& step : steps)
        SetStepState(step, SCENARIO_STEP_NOT_STARTED);

    SetCurrentStep(0);

    // TC_LOG_ERROR(LOG_FILTER_CHALLENGE, "%s %u, mapID: %u, scenarioID: %u", __FUNCTION__, __LINE__, map->GetId(), scenarioId);
}

uint32 Scenario::GetInstanceId() const
{
    return instanceId;
}

Map* Scenario::GetMap()
{
    return curMap;
}

OutdoorPvP* Scenario::GetOutdoorPvP()
{
    return m_outdoorPvp;
}

uint32 Scenario::GetOutdoorPvPZone() const
{
    return m_outdoorPvpZone;
}


uint32 Scenario::GetScenarioId() const
{
    return scenarioId;
}

uint32 Scenario::GetCurrentStep() const
{
    return currentStep;
}

void Scenario::SetStepState(ScenarioStepEntry const* step, ScenarioStepState state)
{
    _stepStates[step] = state;
}

bool Scenario::IsCompleted(bool bonus) const
{
    return currentStep == GetStepCount(bonus);
}

uint8 Scenario::GetStepCount(bool withBonus) const
{
    if (withBonus)
        return steps.size();

    uint8 count = 0;
    for (auto const& v : steps)
        if (!v->IsBonusObjective())
            ++count;

    return count;
}

void Scenario::SetCurrentStep(uint8 step)
{
    currentStep = step;
    currentTree = GetScenarioCriteriaByStep(currentStep);

    SendStepUpdate();

    if (OutdoorPvP* outDoorPvP = GetOutdoorPvP())
        outDoorPvP->SetData(GetOutdoorPvPZone(), currentStep);
    else if (Map *m = GetMap())
    {
        if (InstanceMap* i = m->ToInstanceMap())
        {
            if (InstanceScript *script = i->GetInstanceScript())
            {
                script->setScenarioStep(currentStep);
                script->onScenarionNextStep(currentStep);
                script->UpdatePhasing();
            }
            else
                i->UpdatePhasing(); // if InstanceScript is empty
        }
    }

    ActiveSteps.push_back(steps[currentStep]->ID);
}

uint8 Scenario::UpdateCurrentStep(bool loading)
{
    // i_updateLock.lock();
    uint8 oldStep = currentStep;

    for (ScenarioSteps::const_iterator itr = steps.begin(); itr != steps.end(); ++itr)
    {
        //Not check if ctep already complete
        if (currentStep > (*itr)->OrderIndex)
            continue;

        CriteriaTreeEntry const* criteriaTree = sCriteriaTreeStore.LookupEntry((*itr)->Criteriatreeid);
        if (!criteriaTree)
            continue;

        if (GetAchievementMgr().IsCompletedScenarioTree(criteriaTree))
        {
            currentStep = (*itr)->OrderIndex + 1;
            currentTree = GetScenarioCriteriaByStep(currentStep);
        }
    }

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "UpdateCurrentStep currentStep %u oldStep %u loading %u steps %u", currentStep, oldStep, loading, steps.size());

    if (currentStep != oldStep && !loading)
    {
        if (currentStep != 0 && currentStep < steps.size())
        {
            if (OutdoorPvP* outDoorPvP = GetOutdoorPvP())
                outDoorPvP->SetData(GetOutdoorPvPZone(), currentStep);
            else if (Map *m = GetMap())
            {
                if (InstanceMap* i = m->ToInstanceMap())
                {
                    if (InstanceScript *script = i->GetInstanceScript())
                    {
                        script->setScenarioStep(currentStep);
                        script->onScenarionNextStep(currentStep);
                        script->UpdatePhasing();
                    }
                    else
                        i->UpdatePhasing(); // if InstanceScript is empty
                }
            }
            ActiveSteps.push_back(steps[currentStep]->ID);
        }

        SendStepUpdate();

        if (IsCompleted(false))
            Reward(false, oldStep);
        else if (hasbonus && IsCompleted(true))
            Reward(true, oldStep);
    }

    SetStepState(steps[currentStep], SCENARIO_STEP_IN_PROGRESS);
    SetStepState(steps[oldStep], SCENARIO_STEP_DONE);

    // i_updateLock.unlock();
    //TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "UpdateCurrentStep currentStep %u oldStep %u loading %u", currentStep, oldStep, loading);
    return currentStep;
}

uint32 Scenario::GetScenarioCriteriaByStep(uint8 step)
{
    if (steps.empty())
        return 0;

    for (auto const& step_ : steps)
        if (step_ && step == step_->OrderIndex)
            return step_->Criteriatreeid;

    return 0;
}

void Scenario::Reward(bool bonus, uint32 rewardStep)
{
    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::Reward bonus %u rewarded %u bonusRewarded %u rewardStep %u", bonus, rewarded, bonusRewarded, rewardStep);

    if (bonus && bonusRewarded)
        return;

    if (!bonus && rewarded)
        return;

    if (bonus)
        bonusRewarded = true;
    else
        rewarded = true;

    ObjectGuid groupGuid;
    Map* map = GetMap();
    OutdoorPvP* pvpMap = GetOutdoorPvP();
    // map not created? bye-bye reward
    if (!pvpMap && !map)
        return;

    Quest const* quest = sQuestDataStore->GetQuestTemplate(steps[rewardStep]->RewardQuestID);

    if (pvpMap)
    {
        pvpMap->ApplyOnEveryPlayerInZone([quest, this](Player* player)
        {
            if (player->CanContact())
            {

                if (quest)
                    player->AddDelayedEvent(100, [player, quest]() -> void { if (player) player->RewardQuest(quest, 0, nullptr, false); });

                player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_SCENARIO, scenarioId, 1);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_SCENARIO_COUNT, 1);

                WorldPackets::Scenario::ScenarioCompleted data;
                data.ScenarioID = scenarioId;
                player->SendDirectMessage(data.Write());
            }
        }, GetOutdoorPvPZone());

        pvpMap->SetData(GetOutdoorPvPZone(), 100);
    }
    else if (map)
    {
        map->ApplyOnEveryPlayer([&groupGuid, this, quest](Player* player)
        {
            if (player->CanContact())
            {
                if (groupGuid.IsEmpty())
                    groupGuid = player->GetGroup() ? player->GetGroup()->GetGUID() : ObjectGuid::Empty;

                if (_challenge)
                    player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_CHALLENGE, _challenge->GetChallengeLevel());

                if (quest)
                    player->AddDelayedEvent(100, [player, quest]() -> void { if (player) player->RewardQuest(quest, 0, nullptr, false); });

                player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_SCENARIO, scenarioId, 1);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_COMPLETE_SCENARIO_COUNT, 1);

                WorldPackets::Scenario::ScenarioCompleted data;
                data.ScenarioID = scenarioId;
                player->SendDirectMessage(data.Write());
            }
        });
    }

    // should not happen
    if (groupGuid.IsEmpty())
        return;

    // TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Reward Type %u", _scenarioEntry->Type);

    switch (_scenarioEntry->Type)
    {
        case SCENARIO_TYPE_DEFAULT:
        case SCENARIO_TYPE_USE_DUNGEON_DISPLAY:
        case SCENARIO_TYPE_BOOST_TUTORIAL:
            if (uint32 dungeonId = sLFGMgr->GetDungeon(groupGuid)) // lfg dungeons are rewarded through lfg
            {
                // lfg dungeon that we are in is not current scenario
                if (dungeonId != dungeonData->id)
                    return;

                sLFGMgr->FinishDungeon(groupGuid, dungeonId);
            }
            break;
        case SCENARIO_TYPE_CHALLENGE_MODE:
            if (_challenge)
                _challenge->Complete();
            break;
        case SCENARIO_TYPE_LEGION_INVASION: /// The Broken Shore scenario.
        default:
            break;
    }
}

AchievementMgr<Scenario>& Scenario::GetAchievementMgr()
{
    return m_achievementMgr;
}

AchievementMgr<Scenario> const& Scenario::GetAchievementMgr() const
{
    return m_achievementMgr;
}

ScenarioStepState Scenario::GetStepState(ScenarioStepEntry const* step)
{
    auto const itr = _stepStates.find(step);
    if (itr != _stepStates.end())
        return itr->second;

    return SCENARIO_STEP_INVALID;
}

std::vector<WorldPackets::Scenario::BonusObjectiveData> Scenario::GetBonusObjectivesData()
{
    std::vector<WorldPackets::Scenario::BonusObjectiveData> bonusObjectivesData;
    for (auto const& step : steps)
    {
        if (!step->IsBonusObjective())
            continue;

        if (sAchievementMgr->GetCriteriaTree(step->Criteriatreeid))
        {
            WorldPackets::Scenario::BonusObjectiveData bonusObjectiveData;
            bonusObjectiveData.BonusObjectiveID = step->ID;
            bonusObjectiveData.ObjectiveComplete = GetStepState(step) == SCENARIO_STEP_DONE;
            bonusObjectivesData.push_back(bonusObjectiveData);
        }
    }

    return bonusObjectivesData;
}

void Scenario::SendStepUpdate(Player* player, bool full)
{
    WorldPackets::Scenario::ScenarioState state;
    state.BonusObjectives = GetBonusObjectivesData();
    state.ScenarioID = GetScenarioId();
    state.CurrentStep = currentStep < steps.size() ? steps[currentStep]->ID : -1;
    state.ScenarioComplete = IsCompleted(false);
    state.ActiveSteps = ActiveSteps;

    std::vector<ScenarioSpellData> const* scSpells = sObjectMgr->GetScenarioSpells(GetScenarioId());
    if (scSpells)
    {
        for (std::vector<ScenarioSpellData>::const_iterator itr = scSpells->begin(); itr != scSpells->end(); ++itr)
        {
           // if ((*itr).StepId == state.ActiveSteps)
            if ((*itr).StepId == GetCurrentStep())
            {
                WorldPackets::Scenario::ScenarioState::ScenarioSpellUpdate spellUpdate;
                spellUpdate.Usable = true;
                spellUpdate.SpellID = (*itr).Spells;
                state.Spells.emplace_back(spellUpdate);
            }
        }
    }

    if (full)
    {
        CriteriaProgressMap const* progressMap = GetAchievementMgr().GetCriteriaProgressMap();
        if (!progressMap->empty())
        {
            for (CriteriaProgressMap::const_iterator itr = progressMap->begin(); itr != progressMap->end(); ++itr)
            {
                CriteriaProgress const& treeProgress = itr->second;
                CriteriaTreeEntry const* criteriaTreeEntry = sCriteriaTreeStore.LookupEntry(itr->first);
                if (!criteriaTreeEntry)
                    continue;

                WorldPackets::Achievement::CriteriaTreeProgress progress;
                progress.Id = criteriaTreeEntry->CriteriaID;
                progress.Quantity = treeProgress.Counter;
                progress.Player = ObjectGuid::Create<HighGuid::Scenario>(0, GetScenarioId(), 1); // whats the fuck ?
                progress.Flags = 0;
                progress.Date = time(nullptr) - treeProgress.date;
                progress.TimeFromStart = time(nullptr) - treeProgress.date;
                progress.TimeFromCreate = time(nullptr) - treeProgress.date;
                state.Progress.push_back(progress);
            }
        }
    }

    if (player)
        player->SendDirectMessage(state.Write());
    else
        BroadCastPacket(state.Write());

    if (full && _challenge)
    {
        _challenge->SendChallengeModeStart(player);
        _challenge->SendStartElapsedTimer(player);
    }
}

void Scenario::SendFinishPacket(Player * player)
{
    WorldPackets::Scenario::ScenarioState state;
    state.BonusObjectives = GetBonusObjectivesData();
    state.ScenarioID = GetScenarioId();
    state.CurrentStep = currentStep < steps.size() ? steps[currentStep]->ID : -1;
    state.ScenarioComplete = true;
    state.ActiveSteps = ActiveSteps;
    player->SendDirectMessage(state.Write());
}

void Scenario::SendCriteriaUpdate(CriteriaProgress const* progress, uint32 timeElapsed /* = 0*/)
{
    //TC_LOG_DEBUG(LOG_FILTER_CHALLENGE, "Scenario::SendCriteriaUpdate criteria %u Counter %u", progress->criteriaTree->criteria, progress->Counter);

    if (!progress || !progress->criteriaTree)
        return;

    WorldPackets::Scenario::ScenarioProgressUpdate update;

    WorldPackets::Achievement::CriteriaTreeProgress& progressUpdate = update.Progress;
    progressUpdate.Id = progress->criteriaTree->CriteriaID;
    progressUpdate.Quantity = progress->Counter;
    progressUpdate.Player = progress->PlayerGUID;
    progressUpdate.Flags = 0;
    progressUpdate.Date = progress->date;
    progressUpdate.TimeFromStart = timeElapsed;
    progressUpdate.TimeFromCreate = timeElapsed;
    BroadCastPacket(update.Write());
}

void Scenario::BroadCastPacket(const WorldPacket* data)
{
    if (m_outdoorPvp)
    {
        m_outdoorPvp->BroadcastPacketByZone(*data, m_outdoorPvpZone);
        return;
    }

    Map* map = sMapMgr->FindMap(dungeonData->map, instanceId);
    if (!map || map->IsMapUnload())
        return;

    map->SendToPlayers(data);
}

bool Scenario::CanUpdateCriteria(uint32 criteriaId, uint32 recursTree /*=0*/) const
{
    auto const& cTreeList = sDB2Manager.GetCriteriaTreeList(recursTree ? recursTree : currentTree);
    if (!cTreeList)
        return false;

    for (std::vector<CriteriaTreeEntry const*>::const_iterator itr = cTreeList->begin(); itr != cTreeList->end(); ++itr)
    {
        if (CriteriaTreeEntry const* criteriaTree = *itr)
        {
            if (criteriaTree->CriteriaID == 0)
            {
                if (CanUpdateCriteria(criteriaId, criteriaTree->ID))
                    return true;
            }
            else if (criteriaTree->ID == criteriaId)
                return true;
        }
    }

    return false;
}

Challenge* Scenario::GetChallenge()
{
    return _challenge;
}

void Scenario::SetOutdoorPvP(OutdoorPvP * outdoor, uint32 zone)
{
    m_outdoorPvp = outdoor;
    m_outdoorPvpZone = zone;
}

void Scenario::UpdateAchievementCriteria(CriteriaTypes type, uint32 miscValue1 /*= 0*/, uint32 miscValue2 /*= 0*/, uint32 miscValue3 /*= 0*/, Unit* unit /*= NULL*/, Player* referencePlayer /*= NULL*/)
{
    AchievementCachePtr referenceCache = std::make_shared<AchievementCache>(referencePlayer, unit, type, miscValue1, miscValue2, miscValue3);

    GetAchievementMgr().UpdateAchievementCriteria(referenceCache);
}
