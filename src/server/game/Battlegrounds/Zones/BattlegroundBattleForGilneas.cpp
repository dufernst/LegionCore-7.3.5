/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2010-2011 Project SkyFire <http://www.projectskyfire.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "World.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "BattlegroundBattleForGilneas.h"
#include "Object.h"
#include "Player.h"
#include "Util.h"
#include "WorldStatePackets.h"

BattlegroundBattleForGilneas::BattlegroundBattleForGilneas(): _HonorTicks(0)
{
    m_BuffChange = true;
    BgObjects.resize(GILNEAS_BG_OBJECT_MAX);
    BgCreatures.resize(GILNEAS_BG_ALL_NODES_COUNT + 3); // +3 for aura triggers

    _IsInformedNearVictory = false;

    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        _teamScores500Disadvantage[i] = false;
}

BattlegroundBattleForGilneas::~BattlegroundBattleForGilneas() = default;

void BattlegroundBattleForGilneas::PostUpdateImpl(Milliseconds diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (GetElapsedTime() >= Minutes(17))
        Battleground::BattlegroundTimedWin();

    int16 teamPoints[MAX_TEAMS] = { };

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        if (_capturePoints[i].Timer > Milliseconds(0))
        {
            if (_capturePoints[i].Timer > diff)
                _capturePoints[i].Timer -= diff;
            else
            {
                _capturePoints[i].Timer = Milliseconds(0);
                _capturePoints[i].PrevStatus = _capturePoints[i].Status;
                _capturePoints[i].Status = NODE_STATUS_CAPTURE;

                _NodeOccupied(i, _capturePoints[i].TeamID);

                UpdateCapturePoint(NODE_STATUS_CAPTURE, _capturePoints[i].TeamID, _capturePoints[i].Point);
            }
        }

        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
            if (_capturePoints[i].TeamID == team && _capturePoints[i].Status == NODE_STATUS_CAPTURE)
                ++teamPoints[team];
    }

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        int16 points = teamPoints[team];
        if (!points)
            continue;

        _lastTick[team] += diff;
        if (_lastTick[team] > BgBFGTickIntervals[points])
        {
            _lastTick[team] -= BgBFGTickIntervals[points];
            m_TeamScores[team] += BgBFGTickPoints[points];
            _honorScoreTicks[team] += BgBFGTickPoints[points];

            if (_honorScoreTicks[team] >= _HonorTicks)
            {
                RewardHonorToTeam(GetBonusHonorFromKill(1), MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(team)));
                _honorScoreTicks[team] -= _HonorTicks;
            }

            if (!_IsInformedNearVictory && m_TeamScores[team] > GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE)
            {
                SendBroadcastText(team == TEAM_ALLIANCE ? 10598 : 10599, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                PlaySoundToAll(BG_SOUND_NEAR_VICTORY);
                _IsInformedNearVictory = true;
            }

            if (m_TeamScores[team] > GILNEAS_BG_MAX_TEAM_SCORE)
                m_TeamScores[team] = GILNEAS_BG_MAX_TEAM_SCORE;

            Battleground::SendBattleGroundPoints(team != TEAM_ALLIANCE, m_TeamScores[team]);

            UpdateWorldState(team == TEAM_ALLIANCE ? WorldStates::ALLIANCE_RESOUCES : WorldStates::HORDE_RESOUCES, m_TeamScores[team]);

            uint8 otherTeam = (team + 1) % MAX_TEAMS;
            if (m_TeamScores[team] > m_TeamScores[otherTeam] + 500)
                _teamScores500Disadvantage[otherTeam] = true;
        }

        if (m_TeamScores[team] >= GILNEAS_BG_MAX_TEAM_SCORE)
            EndBattleground(MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(team)));
    }
}

void BattlegroundBattleForGilneas::StartingEventCloseDoors()
{
    for (uint8 object = BG_BFG_OBJECT_BANNER; object < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++object)
        SpawnBGObject(object, RESPAWN_ONE_DAY);

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT * 3; ++i)
        SpawnBGObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + i, RESPAWN_ONE_DAY);

    DoorsClose(GILNEAS_BG_OBJECT_GATE_A_1, GILNEAS_BG_OBJECT_GATE_H_1);
    DoorsClose(GILNEAS_BG_OBJECT_GATE_A_2, GILNEAS_BG_OBJECT_GATE_H_2);

    _NodeOccupied(GILNEAS_BG_SPIRIT_ALIANCE, TEAM_ALLIANCE);
    _NodeOccupied(GILNEAS_BG_SPIRIT_HORDE, TEAM_HORDE);
}

void BattlegroundBattleForGilneas::StartingEventOpenDoors()
{
    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
        UpdateCapturePoint(NODE_STATUS_NEUTRAL, _capturePoints[i].TeamID, _capturePoints[i].Point, nullptr, true);
    }

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        SpawnBGObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + urand(0, 2) + i * 3, RESPAWN_IMMEDIATELY);

    DoorsOpen(GILNEAS_BG_OBJECT_GATE_A_1, GILNEAS_BG_OBJECT_GATE_H_1);
    DoorsOpen(GILNEAS_BG_OBJECT_GATE_A_2, GILNEAS_BG_OBJECT_GATE_H_2);

    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, BG_EVENT_START_BATTLE);
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, BG_EVENT_START_BATTLE);
}

void BattlegroundBattleForGilneas::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundBFGScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(GILNEAS_BG_MAX_TEAM_SCORE).Write());

    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);
}

void BattlegroundBattleForGilneas::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    uint8 status;

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (GameObject* point = _capturePoints[i].Point) // if player entered the battleground which has already begun
        {
            switch (_capturePoints[i].Status)
            {
                case NODE_STATUS_ASSAULT:
                    status = _capturePoints[i].TeamID == TEAM_ALLIANCE ? NODE_STATE_ALLIANCE_ASSAULT : NODE_STATE_HORDE_ASSAULT;
                    break;
                case NODE_STATUS_CAPTURE:
                    status = _capturePoints[i].TeamID == TEAM_ALLIANCE ? NODE_STATE_ALLIANCE_CAPTURE : NODE_STATE_HORDE_CAPTURE;
                    break;
                default:
                    status = NODE_STATUS_NEUTRAL;
                    break;
            }
            packet.Worldstates.emplace_back(static_cast<WorldStates>(point->GetGOInfo()->capturePoint.worldState1), status);
        }

    packet.Worldstates.emplace_back(WorldStates::OCCOPIED_BASES_ALLIANCE, _GetCapturedNodesForTeam(TEAM_ALLIANCE));
    packet.Worldstates.emplace_back(WorldStates::OCCOPIED_BASES_HORDE, _GetCapturedNodesForTeam(TEAM_HORDE));
    packet.Worldstates.emplace_back(WorldStates::ALLIANCE_RESOUCES, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::HORDE_RESOUCES, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::MAX_TEAM_RESOUCES, GILNEAS_BG_MAX_TEAM_SCORE);
}

void BattlegroundBattleForGilneas::_NodeOccupied(uint8 node, TeamId team)
{
    if (node >= GILNEAS_BG_DYNAMIC_NODES_COUNT)
        return;

    if (!AddSpiritGuide(node, BgBfgSpiritGuidePos[node], team))
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Failed to spawn spirit guide! point: %u, team: %u, ", node, MS::Battlegrounds::GetTeamByTeamId(team));

    UpdateWorldState(_capturePoints[node].Point->GetGOInfo()->capturePoint.worldState1, _capturePoints[node].Status);

    if (_capturePoints[node].Status == NODE_STATUS_CAPTURE)
    {
        UpdateWorldState(WorldStates::OCCOPIED_BASES_ALLIANCE, _GetCapturedNodesForTeam(TEAM_ALLIANCE));
        UpdateWorldState(WorldStates::OCCOPIED_BASES_HORDE, _GetCapturedNodesForTeam(TEAM_HORDE));
    }
}

void BattlegroundBattleForGilneas::_NodeDeOccupied(uint8 node)
{
    if (node >= GILNEAS_BG_DYNAMIC_NODES_COUNT)
        return;

    if (node < GILNEAS_BG_DYNAMIC_NODES_COUNT)
        DelCreature(node + 5);

    RelocateDeadPlayers(BgCreatures[node]);

    if (!BgCreatures[node].IsEmpty())
        DelCreature(node);
}

void BattlegroundBattleForGilneas::EventPlayerClickedOnFlag(Player* source, GameObject* /*object*/, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE;
    GameObject* object = GetBgMap()->GetGameObject(BgObjects[i]);
    while ((i < GILNEAS_BG_DYNAMIC_NODES_COUNT) && ((!object) || (!source->IsWithinDistInMap(object, 10))))
    {
        ++i;
        object = GetBgMap()->GetGameObject(BgObjects[i]);
    }

    if (i == GILNEAS_BG_DYNAMIC_NODES_COUNT)
        return;

    TeamId teamID = source->GetBGTeamId();
    if (_capturePoints[i].TeamID == teamID)
        return;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    auto pointInfo = object->GetGOInfo()->capturePoint;

    switch (_capturePoints[i].Status)
    {
        case NODE_STATUS_NEUTRAL: // If node is neutral, change to contested
            UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
            UpdateCapturePoint(NODE_STATUS_ASSAULT, teamID, _capturePoints[i].Point, source);

            _capturePoints[i].Timer = Milliseconds(pointInfo.CaptureTime);
            _capturePoints[i].PrevStatus = _capturePoints[i].Status;
            _capturePoints[i].Status = NODE_STATUS_ASSAULT;
            _capturePoints[i].TeamID = teamID;
            break;
        case NODE_STATUS_ASSAULT:
            if (_capturePoints[i].PrevStatus < NODE_STATUS_CAPTURE) // If last state is NOT occupied, change node to enemy-contested
            {
                _capturePoints[i].Timer = Milliseconds(pointInfo.CaptureTime);
                _capturePoints[i].PrevStatus = _capturePoints[i].Status;
                _capturePoints[i].TeamID = teamID;
                _capturePoints[i].Status = NODE_STATUS_ASSAULT;
                UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
                UpdateCapturePoint(NODE_STATUS_ASSAULT, teamID, _capturePoints[i].Point, source);
                break;
            }
            // If contested, change back to occupied
            _capturePoints[i].PrevStatus = _capturePoints[i].Status;
            _capturePoints[i].TeamID = teamID;
            _capturePoints[i].Status = NODE_STATUS_CAPTURE;
            UpdatePlayerScore(source, SCORE_BASES_DEFENDED, 1);
            UpdateCapturePoint(NODE_STATUS_CAPTURE, teamID, _capturePoints[i].Point, source);
            _NodeOccupied(i, teamID);
            break;
        default: // If node is occupied, change to enemy-contested
        {
            UpdatePlayerScore(source, SCORE_BASES_ASSAULTED, 1);
            UpdateCapturePoint(NODE_STATUS_ASSAULT, teamID, _capturePoints[i].Point, source);
            _NodeDeOccupied(i);

            _capturePoints[i].Timer = Milliseconds(pointInfo.CaptureTime);
            _capturePoints[i].PrevStatus = _capturePoints[i].Status;
            _capturePoints[i].Status = NODE_STATUS_ASSAULT;
            _capturePoints[i].TeamID = teamID;
            break;
        }
    }

    if (_capturePoints[i].Status == NODE_STATUS_ASSAULT)
    {
        UpdateWorldState(WorldStates::OCCOPIED_BASES_ALLIANCE, _GetCapturedNodesForTeam(TEAM_ALLIANCE));
        UpdateWorldState(WorldStates::OCCOPIED_BASES_HORDE, _GetCapturedNodesForTeam(TEAM_HORDE));
    }
}

bool BattlegroundBattleForGilneas::SetupBattleground()
{
    if (!AddObject(BG_BFG_OBJECT_BANNER, BG_BFG_GO_NODE_BANNER_0, BgBfgNodePosition[0][0], BgBfgNodePosition[0][1], BgBfgNodePosition[0][2], BgBfgNodePosition[0][3], BgBfgNodePosition[0][4], BgBfgNodePosition[0][5], BgBfgNodePosition[0][6], BgBfgNodePosition[0][7], RESPAWN_ONE_DAY) ||
        !AddObject(BG_BFG_OBJECT_BANNER + 1, BG_BFG_GO_NODE_BANNER_1, BgBfgNodePosition[1][0], BgBfgNodePosition[1][1], BgBfgNodePosition[1][2], BgBfgNodePosition[1][3], BgBfgNodePosition[1][4], BgBfgNodePosition[1][5], BgBfgNodePosition[1][6], BgBfgNodePosition[1][7], RESPAWN_ONE_DAY) ||
        !AddObject(BG_BFG_OBJECT_BANNER + 2, BG_BFG_GO_NODE_BANNER_2, BgBfgNodePosition[2][0], BgBfgNodePosition[2][1], BgBfgNodePosition[2][2], BgBfgNodePosition[2][3], BgBfgNodePosition[2][4], BgBfgNodePosition[2][5], BgBfgNodePosition[2][6], BgBfgNodePosition[2][7], RESPAWN_ONE_DAY))
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattleForGilneas: Can't Create Some Object");
        return false;
    }

    if (!AddObject(GILNEAS_BG_OBJECT_GATE_A_1, BG_BFG_GO_GATE_A_1, BgBfgDoorPos[0][0], BgBfgDoorPos[0][1], BgBfgDoorPos[0][2], BgBfgDoorPos[0][3], BgBfgDoorPos[0][4], BgBfgDoorPos[0][5], BgBfgDoorPos[0][6], BgBfgDoorPos[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(GILNEAS_BG_OBJECT_GATE_A_2, BG_BFG_GO_GATE_A_2, BgBfgDoorPos[1][0], BgBfgDoorPos[1][1], BgBfgDoorPos[1][2], BgBfgDoorPos[1][3], BgBfgDoorPos[1][4], BgBfgDoorPos[1][5], BgBfgDoorPos[1][6], BgBfgDoorPos[1][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(GILNEAS_BG_OBJECT_GATE_H_1, BG_BFG_GO_GATE_H_1, BgBfgDoorPos[2][0], BgBfgDoorPos[2][1], BgBfgDoorPos[2][2], BgBfgDoorPos[2][3], BgBfgDoorPos[2][4], BgBfgDoorPos[2][5], BgBfgDoorPos[2][6], BgBfgDoorPos[2][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(GILNEAS_BG_OBJECT_GATE_H_2, BG_BFG_GO_GATE_H_2, BgBfgDoorPos[3][0], BgBfgDoorPos[3][1], BgBfgDoorPos[3][2], BgBfgDoorPos[3][3], BgBfgDoorPos[3][4], BgBfgDoorPos[3][5], BgBfgDoorPos[3][6], BgBfgDoorPos[3][7], RESPAWN_IMMEDIATELY))
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattleForGilneas: Can't Create Doors");
        return false;
    }

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[i]))
            _capturePoints[i].Point = obj;

        if (!AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i, Buff_Entries[0], BgBFGBuffsPos[i][0], BgBFGBuffsPos[i][1], BgBFGBuffsPos[i][2], BgBFGBuffsPos[i][3], 0, 0, sin(BgBFGBuffsPos[i][3] / 2), cos(BgBFGBuffsPos[i][3] / 2), RESPAWN_ONE_DAY) ||
            !AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i + 1, Buff_Entries[1], BgBFGBuffsPos[i][0], BgBFGBuffsPos[i][1], BgBFGBuffsPos[i][2], BgBFGBuffsPos[i][3], 0, 0, sin(BgBFGBuffsPos[i][3] / 2), cos(BgBFGBuffsPos[i][3] / 2), RESPAWN_ONE_DAY) ||
            !AddObject(GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE + 3 * i + 2, Buff_Entries[2], BgBFGBuffsPos[i][0], BgBFGBuffsPos[i][1], BgBFGBuffsPos[i][2], BgBFGBuffsPos[i][3], 0, 0, sin(BgBFGBuffsPos[i][3] / 2), cos(BgBFGBuffsPos[i][3] / 2), RESPAWN_ONE_DAY))
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattleForGilneas: Can't Create Buffs");
    }

    for (uint8 i = GILNEAS_BG_SPIRIT_ALIANCE; i <= GILNEAS_BG_SPIRIT_HORDE; ++i)
        if (!AddSpiritGuide(i, BgBfgSpiritGuidePos[i].GetPositionX(), BgBfgSpiritGuidePos[i].GetPositionY(), BgBfgSpiritGuidePos[i].GetPositionZ(), BgBfgSpiritGuidePos[i].GetOrientation(), i == GILNEAS_BG_SPIRIT_ALIANCE ? Team::ALLIANCE : Team::HORDE))
            return false;

    return true;
}

void BattlegroundBattleForGilneas::Reset()
{
    Battleground::Reset();

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        m_TeamScores[i] = 0;
        _lastTick[i] = Milliseconds(0);
        _honorScoreTicks[i] = 0;
        _teamScores500Disadvantage[i] = false;
    }

    _IsInformedNearVictory = false;
    _HonorTicks = 330;

    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
    {
        _capturePoints[i].Timer = Milliseconds(0);
        _capturePoints[i].Point = nullptr;
        _capturePoints[i].Status = NODE_STATUS_NEUTRAL;
        _capturePoints[i].PrevStatus = NODE_STATUS_NEUTRAL;
        _capturePoints[i].TeamID = TEAM_NONE;
    }

    for (uint8 i = 0; i < GILNEAS_BG_ALL_NODES_COUNT + 3; ++i) // +3 for aura triggers
        if (!BgCreatures[i].IsEmpty())
            DelCreature(i);
}

bool BattlegroundBattleForGilneas::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, BG_OBJECTIVE_ASSAULT_BASE, 1);
            break;
        case SCORE_BASES_DEFENDED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, BG_OBJECTIVE_DEFEND_BASE, 1);
            break;
        default:
            break;
    }
    return true;
}

void BattlegroundBattleForGilneas::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);
}

WorldSafeLocsEntry const* BattlegroundBattleForGilneas::GetClosestGraveYard(Player* player)
{
    TeamId teamIndex = player->GetBGTeamId();

    std::vector<uint8> nodes;
    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePoints[i].TeamID == teamIndex && _capturePoints[i].Status == NODE_STATUS_CAPTURE)
            nodes.push_back(i);

    WorldSafeLocsEntry const* safeLocksEntry = nullptr;

    if (!nodes.empty())
    {
        float X = player->GetPositionX();
        float Y = player->GetPositionY();

        float mindist = 999999.0f; // Temp Hack
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(GILNEAS_BG_GraveyardIds[nodes[i]]);
            if (!entry)
                continue;

            float dist = (entry->Loc.X - X) * (entry->Loc.X - X) + (entry->Loc.Y - Y) * (entry->Loc.Y - Y);

            if (mindist > dist)
            {
                mindist = dist;
                safeLocksEntry = entry;
            }
        }
        nodes.clear();
    }

    if (!safeLocksEntry) // If not, place ghost on starting location
        safeLocksEntry = sWorldSafeLocsStore.LookupEntry(GILNEAS_BG_GraveyardIds[teamIndex + 3]);

    return safeLocksEntry;
}

bool BattlegroundBattleForGilneas::IsAllNodesConrolledByTeam(uint32 teamID) const
{
    uint8 count = 0;
    for (int8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePoints[i].TeamID == teamID && _capturePoints[i].Status == NODE_STATUS_CAPTURE)
            ++count;

    return count == GILNEAS_BG_DYNAMIC_NODES_COUNT;
}

uint8 BattlegroundBattleForGilneas::_GetCapturedNodesForTeam(TeamId teamID)
{
    uint8 nodes = 0;
    for (uint8 i = GILNEAS_BG_NODE_LIGHTHOUSE; i < GILNEAS_BG_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePoints[i].Status == NODE_STATUS_CAPTURE && _capturePoints[i].TeamID == teamID)
            ++nodes;

    return nodes;
}
