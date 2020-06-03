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

#include "World.h"
#include "BattlegroundMgr.h"
#include "Battleground.h"
#include "BattlegroundArathiBasin.h"
#include "Creature.h"
#include "Object.h"
#include "Player.h"
#include "Util.h"
#include "WorldStatePackets.h"

namespace
{
    uint32 const SpellFogOfWar = 231275;
}

void BattlegroundABScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            BasesAssaulted += value;
            break;
        case SCORE_BASES_DEFENDED:
            BasesDefended += value;
            break;
        default:
            BattlegroundScore::UpdateScore(type, value);
            break;
    }
}

void BattlegroundABScore::BuildObjectivesBlock(std::vector<int32>& stats)
{
    stats.push_back(BasesAssaulted);
    stats.push_back(BasesDefended);
}

BattlegroundArathiBasin::BattlegroundArathiBasin() : _honorScoreTics{}, _reputationScoreTics{}, _honorTics(0), _reputationTics(0)
{
    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        _teamScores500Disadvantage[i] = false;

    _isInformedNearVictory = false;
    m_BuffChange = true;
    BgObjects.resize(BG_AB_OBJECT_MAX);
    BgCreatures.resize(BG_AB_ALL_NODES_COUNT + 5);
}

BattlegroundArathiBasin::~BattlegroundArathiBasin() = default;

void BattlegroundArathiBasin::PostUpdateImpl(Milliseconds diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint16 teamPoints[MAX_TEAMS] = { };

    for (int8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
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

    // Accumulate points
    for (int8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        uint16 points = teamPoints[team];
        if (!points)
            continue;

        _lastTick[team] += diff;

        if (_lastTick[team] > BgABTickIntervals[points])
        {
            _lastTick[team] -= BgABTickIntervals[points];
            m_TeamScores[team] += BgABTickPoints[points];
            _honorScoreTics[team] += BgABTickPoints[points];
            _reputationScoreTics[team] += BgABTickPoints[points];

            if (_reputationScoreTics[team] >= _reputationTics)
            {
                RewardReputationToTeam(509, 510, 50, MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(team)));
                _reputationScoreTics[team] -= _reputationTics;
            }

            if (_honorScoreTics[team] >= _honorTics) // should be 9 all time?
            {
                RewardHonorToTeam(GetBonusHonorFromKill(1), MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(team)));
                _honorScoreTics[team] -= _honorTics;
            }

            if (!_isInformedNearVictory && m_TeamScores[team] > BG_AB_WARNING_NEAR_VICTORY_SCORE)
            {
                SendBroadcastText(team == TEAM_ALLIANCE ? 10598 : 10599, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                PlaySoundToAll(BG_SOUND_NEAR_VICTORY);
                _isInformedNearVictory = true;
            }

            if (m_TeamScores[team] > BG_AB_MAX_TEAM_SCORE)
                m_TeamScores[team] = BG_AB_MAX_TEAM_SCORE;

            Battleground::SendBattleGroundPoints(team != TEAM_ALLIANCE, m_TeamScores[team]);

            UpdateWorldState(team == TEAM_ALLIANCE ? WorldStates::ALLIANCE_RESOUCES : WorldStates::HORDE_RESOUCES, m_TeamScores[team]);

            uint8 otherTeam = (team + 1) % MAX_TEAMS;
            if (m_TeamScores[team] > m_TeamScores[otherTeam] + 500)
                _teamScores500Disadvantage[otherTeam] = true;
        }

        if (m_TeamScores[team] >= BG_AB_MAX_TEAM_SCORE)
            EndBattleground(MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(team)));
    }
}

void BattlegroundArathiBasin::StartingEventCloseDoors()
{
    for (int8 i = BG_AB_OBJECT_BANNER; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);

    for (int8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT * 3; ++i)
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + i, RESPAWN_ONE_DAY);

    DoorsClose(BG_AB_OBJECT_GATE_A, BG_AB_OBJECT_GATE_H);

    SpawnBGObject(BG_AB_OBJECT_GATE_A, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_AB_OBJECT_GATE_H, RESPAWN_IMMEDIATELY);

    _NodeOccupied(BG_AB_SPIRIT_ALIANCE, TEAM_ALLIANCE);
    _NodeOccupied(BG_AB_SPIRIT_HORDE, TEAM_HORDE);
}

void BattlegroundArathiBasin::StartingEventOpenDoors()
{
    for (int8 i = BG_AB_OBJECT_BANNER; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

        if (GameObject* obj = GetBgMap()->GetGameObject(BgObjects[i]))
            _capturePoints[i].Point = obj;

        UpdateCapturePoint(NODE_STATUS_NEUTRAL, _capturePoints[i].TeamID, _capturePoints[i].Point, nullptr, true);
    }

    for (int i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        SpawnBGObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + urand(0, 2) + i * 3, RESPAWN_IMMEDIATELY); //randomly select buff to spawn

    DoorsOpen(BG_AB_OBJECT_GATE_A, BG_AB_OBJECT_GATE_H);

    // Achievement: Let's Get This Done
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, AB_EVENT_START_BATTLE);
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, AB_EVENT_START_BATTLE);
}

void BattlegroundArathiBasin::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundABScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(BG_AB_MAX_TEAM_SCORE).Write());

    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);

    if (IsBrawl())
        player->CastSpell(player, SpellFogOfWar);
}

void BattlegroundArathiBasin::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (player)
        player->RemoveAura(SpellFogOfWar);
}

void BattlegroundArathiBasin::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    uint8 status;

    for (uint8 i = BG_AB_NODE_STABLES; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if (GameObject* point = _capturePoints[i].Point)
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
    packet.Worldstates.emplace_back(WorldStates::MAX_TEAM_RESOUCES, BG_AB_MAX_TEAM_SCORE);
    packet.Worldstates.emplace_back(WorldStates::BG_AB_OP_RESOURCES_WARNING, BG_AB_WARNING_NEAR_VICTORY_SCORE);
    packet.Worldstates.emplace_back(WorldStates::ALLIANCE_RESOUCES, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::HORDE_RESOUCES, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(1861), 2);
}

void BattlegroundArathiBasin::_NodeOccupied(uint8 node, TeamId teamID)
{
    Team team = MS::Battlegrounds::GetTeamByTeamId(teamID);

    if (!AddSpiritGuide(node, BgAbSpiritsPos[node], teamID))
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Failed to spawn spirit guide! point: %u, team: %u, ", node, team);

    uint8 capturedNodes = 0;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePoints[node].TeamID == MS::Battlegrounds::GetTeamIdByTeam(team) && _capturePoints[node].Timer <= Milliseconds(0)) // evil code
            ++capturedNodes;

    if (capturedNodes >= 5)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_5_BASES, team);

    if (capturedNodes >= 4)
        CastSpellOnTeam(SPELL_AB_QUEST_REWARD_4_BASES, team);

    if (node >= BG_AB_DYNAMIC_NODES_COUNT)
        return;

    Creature* trigger = !BgCreatures[node + 7].IsEmpty() ? GetBGCreature(node + 7) : nullptr;//0-6 spirit guides
    if (!trigger)
        trigger = AddCreature(WORLD_TRIGGER, node + 7, team, BgABNodePositions[node][0], BgABNodePositions[node][1], BgABNodePositions[node][2], BgABNodePositions[node][3]);

    if (trigger)
    {
        trigger->setFaction(teamID == TEAM_ALLIANCE ? 84 : 83);
        trigger->CastSpell(trigger, SPELL_BG_HONORABLE_DEFENDER_25Y, false);
    }

    UpdateWorldState(AbNodeStatesIconsWS[node], _capturePoints[node].Status); // recheck is this needed here
    UpdateWorldState(AbNodeStatesWS[node][_capturePoints[node].Status], _capturePoints[node].Status);

    if (_capturePoints[node].Status == NODE_STATUS_CAPTURE)
    {
        UpdateWorldState(WorldStates::OCCOPIED_BASES_ALLIANCE, _GetCapturedNodesForTeam(TEAM_ALLIANCE));
        UpdateWorldState(WorldStates::OCCOPIED_BASES_HORDE, _GetCapturedNodesForTeam(TEAM_HORDE));
    }
}

void BattlegroundArathiBasin::_NodeDeOccupied(uint8 node)
{
    if (node >= BG_AB_DYNAMIC_NODES_COUNT)
        return;

    if (node < BG_AB_DYNAMIC_NODES_COUNT)
        DelCreature(node + 7);

    RelocateDeadPlayers(BgCreatures[node]);

    if (!BgCreatures[node].IsEmpty())
        DelCreature(node);
}

void BattlegroundArathiBasin::EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    auto pointInfo = object->GetGOInfo()->capturePoint;

    // garbage
    uint8 i = BG_AB_NODE_STABLES;
    GameObject* obj = GetBgMap()->GetGameObject(BgObjects[i]);
    while ((i < BG_AB_DYNAMIC_NODES_COUNT) && (!obj || !source->IsWithinDistInMap(obj, 10)))
    {
        ++i;
        obj = GetBgMap()->GetGameObject(BgObjects[i]);
    }

    if (i == BG_AB_DYNAMIC_NODES_COUNT)
        return;

    TeamId teamID = source->GetBGTeamId();
    if (_capturePoints[i].TeamID == teamID)
        return;

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

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

            _capturePoints[i].Timer = Milliseconds(pointInfo.CaptureTime);
            _capturePoints[i].PrevStatus = _capturePoints[i].Status;
            _capturePoints[i].Status = NODE_STATUS_ASSAULT;
            _capturePoints[i].TeamID = teamID;
            _NodeDeOccupied(i);
            break;
        }
    }

    if (_capturePoints[i].Status == NODE_STATUS_ASSAULT)
    {
        UpdateWorldState(WorldStates::OCCOPIED_BASES_ALLIANCE, _GetCapturedNodesForTeam(TEAM_ALLIANCE));
        UpdateWorldState(WorldStates::OCCOPIED_BASES_HORDE, _GetCapturedNodesForTeam(TEAM_HORDE));
    }
}

bool BattlegroundArathiBasin::SetupBattleground()
{
    for (int8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
    {
        if (!AddObject(BG_AB_OBJECT_BANNER + i, BgABNodes[i], BgABNodePositions[i][0], BgABNodePositions[i][1], BgABNodePositions[i][2], BgABNodePositions[i][3], BgABNodePositions[i][4], BgABNodePositions[i][5], BgABNodePositions[i][6], BgABNodePositions[i][7], RESPAWN_ONE_DAY) ||
            !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i, BGBuffsIDs[0], BgAbBuffPositions[i], { }, RESPAWN_ONE_DAY) ||
            !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 1, BGBuffsIDs[1], BgAbBuffPositions[i], { }, RESPAWN_ONE_DAY) ||
            !AddObject(BG_AB_OBJECT_SPEEDBUFF_STABLES + 3 * i + 2, BGBuffsIDs[2], BgAbBuffPositions[i], { }, RESPAWN_ONE_DAY))
            return false;
    }

    if (!AddObject(BG_AB_OBJECT_GATE_A, BG_AB_OBJECTID_GATE_A, BgABDoorPositions[0][0], BgABDoorPositions[0][1], BgABDoorPositions[0][2], BgABDoorPositions[0][3], BgABDoorPositions[0][4], BgABDoorPositions[0][5], BgABDoorPositions[0][6], BgABDoorPositions[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_AB_OBJECT_GATE_H, BG_AB_OBJECTID_GATE_H, BgABDoorPositions[1][0], BgABDoorPositions[1][1], BgABDoorPositions[1][2], BgABDoorPositions[1][3], BgABDoorPositions[1][4], BgABDoorPositions[1][5], BgABDoorPositions[1][6], BgABDoorPositions[1][7], RESPAWN_IMMEDIATELY))
        return false;

    for (uint8 i = BG_AB_SPIRIT_ALIANCE; i <= BG_AB_SPIRIT_HORDE; ++i)
        if (!AddSpiritGuide(i, BgABSpiritGuidePos[i].GetPositionX(), BgABSpiritGuidePos[i].GetPositionY(), BgABSpiritGuidePos[i].GetPositionZ(), BgABSpiritGuidePos[i].GetOrientation(), i == BG_AB_SPIRIT_ALIANCE ? Team::ALLIANCE : Team::HORDE))
            return false;

    return true;
}

void BattlegroundArathiBasin::Reset()
{
    Battleground::Reset();

    _honorTics = 260;
    _reputationTics = 160;

    _isInformedNearVictory = false;

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        m_TeamScores[i] = 0;
        _honorScoreTics[i] = 0;
        _lastTick[i] = Milliseconds(0);
        _reputationScoreTics[i] = 0;
        _teamScores500Disadvantage[i] = false;
    }

    for (auto& _capturePoint : _capturePoints)
        _capturePoint = CapturePointInfo();

    for (uint8 i = 0; i < BG_AB_ALL_NODES_COUNT + 5; ++i)
        if (!BgCreatures[i].IsEmpty())
            DelCreature(i);
}

void BattlegroundArathiBasin::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);

    for (auto const& itr : GetPlayers())
        if (auto const& player = GetPlayer(itr, "BattlegroundArathiBasin::EndBattleground"))
            player->RemoveAura(SpellFogOfWar);
}

WorldSafeLocsEntry const* BattlegroundArathiBasin::GetClosestGraveYard(Player* player)
{
    static uint32 const BgAbGraveyardIds[BG_AB_ALL_NODES_COUNT] = { 895, 894, 893, 897, 896, 898, 899 };
    static uint32 const BgAbGraveyardIdsBrawl[BG_AB_ALL_NODES_COUNT] = { 5969, 5971, 5970, 5967, 5968, 5973, 5972 };

    TeamId teamIndex = player->GetBGTeamId();

    std::vector<uint8> nodes;
    for (uint8 i = 0; i < BG_AB_DYNAMIC_NODES_COUNT; ++i)
        if (_capturePoints[i].TeamID == teamIndex && _capturePoints[i].Status == NODE_STATUS_CAPTURE)
            nodes.push_back(i);

    WorldSafeLocsEntry const* safeLockEntry = nullptr;
    if (!nodes.empty())
    {
        float X = player->GetPositionX();
        float Y = player->GetPositionY();

        float mindist = 999999.0f;
        for (auto node : nodes)
        {
            WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(IsBrawl() ? BgAbGraveyardIdsBrawl[node] : BgAbGraveyardIds[ node]);
            if (!entry)
                continue;

            float dist = (entry->Loc.X - X) * (entry->Loc.X - X) + (entry->Loc.Y - Y) * (entry->Loc.Y - Y);
            if (mindist > dist)
            {
                mindist = dist;
                safeLockEntry = entry;
            }
        }
        nodes.clear();
    }

    if (!safeLockEntry) // If not, place ghost on starting location
        safeLockEntry = sWorldSafeLocsStore.LookupEntry(IsBrawl() ? BgAbGraveyardIdsBrawl[teamIndex + 5] : BgAbGraveyardIds[teamIndex + 5]);

    return safeLockEntry;
}

bool BattlegroundArathiBasin::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, AB_OBJECTIVE_ASSAULT_BASE, 1);
            break;
        case SCORE_BASES_DEFENDED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, AB_OBJECTIVE_DEFEND_BASE, 1);
            break;
        default:
            break;
    }
    return true;
}

bool BattlegroundArathiBasin::IsAllNodesConrolledByTeam(uint32 teamID) const
{
    uint8 count = 0;
    for (const auto& _capturePoint : _capturePoints)
        if (_capturePoint.TeamID == teamID && _capturePoint.Status == NODE_STATUS_CAPTURE)
            ++count;

    return count == BG_AB_DYNAMIC_NODES_COUNT;
}

uint8 BattlegroundArathiBasin::_GetCapturedNodesForTeam(TeamId teamID)
{
    uint8 nodes = 0;
    for (auto& _capturePoint : _capturePoints)
        if (_capturePoint.Status == NODE_STATUS_CAPTURE && _capturePoint.TeamID == teamID)
            ++nodes;

    return nodes;
}
