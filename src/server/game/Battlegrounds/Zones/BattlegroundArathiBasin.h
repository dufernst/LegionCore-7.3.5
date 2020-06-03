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
#ifndef __BATTLEGROUNDAB_H
#define __BATTLEGROUNDAB_H

#include "Battleground.h"
#include "BattlegroundScore.h"

enum BG_AB_NodeObjectId
{
    BG_AB_OBJECTID_NODE_BANNER_0    = 227420,       // Stables banner
    BG_AB_OBJECTID_NODE_BANNER_1    = 227522,       // Blacksmith banner
    BG_AB_OBJECTID_NODE_BANNER_2    = 227536,       // Farm banner
    BG_AB_OBJECTID_NODE_BANNER_3    = 227544,       // Lumber mill banner
    BG_AB_OBJECTID_NODE_BANNER_4    = 227538        // Gold mine banner
};

uint32 const BgABNodes[] = { BG_AB_OBJECTID_NODE_BANNER_0, BG_AB_OBJECTID_NODE_BANNER_1, BG_AB_OBJECTID_NODE_BANNER_2, BG_AB_OBJECTID_NODE_BANNER_3, BG_AB_OBJECTID_NODE_BANNER_4 };
uint32 const BGBuffsIDs[] = { BG_OBJECTID_SPEEDBUFF_ENTRY, BG_OBJECTID_REGENBUFF_ENTRY, BG_OBJECTID_BERSERKERBUFF_ENTRY };

enum BG_AB_ObjectType
{
    // for all 5 node points 8*5=40 objects
    BG_AB_OBJECT_BANNER                  = 0,


    BG_AB_OBJECT_GATE_A                  = 40,
    BG_AB_OBJECT_GATE_H                  = 41,

    BG_AB_OBJECT_SPEEDBUFF_STABLES       = 42,
    BG_AB_OBJECT_REGENBUFF_STABLES       = 43,
    BG_AB_OBJECT_BERSERKBUFF_STABLES     = 44,
    BG_AB_OBJECT_SPEEDBUFF_BLACKSMITH    = 45,
    BG_AB_OBJECT_REGENBUFF_BLACKSMITH    = 46,
    BG_AB_OBJECT_BERSERKBUFF_BLACKSMITH  = 47,
    BG_AB_OBJECT_SPEEDBUFF_FARM          = 48,
    BG_AB_OBJECT_REGENBUFF_FARM          = 49,
    BG_AB_OBJECT_BERSERKBUFF_FARM        = 50,
    BG_AB_OBJECT_SPEEDBUFF_LUMBER_MILL   = 51,
    BG_AB_OBJECT_REGENBUFF_LUMBER_MILL   = 52,
    BG_AB_OBJECT_BERSERKBUFF_LUMBER_MILL = 53,
    BG_AB_OBJECT_SPEEDBUFF_GOLD_MINE     = 54,
    BG_AB_OBJECT_REGENBUFF_GOLD_MINE     = 55,
    BG_AB_OBJECT_BERSERKBUFF_GOLD_MINE   = 56,
    BG_AB_OBJECT_MAX                     = 57,
};

enum BG_AB_ObjectTypes
{
    BG_AB_OBJECTID_GATE_A               = 180255,
    BG_AB_OBJECTID_GATE_H               = 180256
};

enum BG_AB_Score
{
    BG_AB_WARNING_NEAR_VICTORY_SCORE    = 1200,
    BG_AB_MAX_TEAM_SCORE                = 1500
};

enum BG_AB_BattlegroundNodes
{
    BG_AB_NODE_STABLES          = 0,
    BG_AB_NODE_BLACKSMITH       = 1,
    BG_AB_NODE_FARM             = 2,
    BG_AB_NODE_LUMBER_MILL      = 3,
    BG_AB_NODE_GOLD_MINE        = 4,

    BG_AB_DYNAMIC_NODES_COUNT   = 5,                        // dynamic nodes that can be captured

    BG_AB_SPIRIT_ALIANCE        = 5,
    BG_AB_SPIRIT_HORDE          = 6,

    BG_AB_ALL_NODES_COUNT       = 7,                        // all nodes (dynamic and static)
};


WorldStates const AbNodeStatesWS[BG_AB_DYNAMIC_NODES_COUNT][5] =
{
    {WorldStates::BG_AB_OP_STABLE_STATE_ALIENCE,     WorldStates::BG_AB_OP_STABLE_STATE_HORDE,       WorldStates::BG_AB_OP_STABLE_STATE_CON_ALI,     WorldStates::BG_AB_OP_STABLE_STATE_CON_HOR},
    {WorldStates::BG_AB_OP_BLACKSMITH_STATE_ALIENCE, WorldStates::BG_AB_OP_BLACKSMITH_STATE_HORDE,   WorldStates::BG_AB_OP_BLACKSMITH_STATE_CON_ALI, WorldStates::BG_AB_OP_BLACKSMITH_STATE_CON_HOR},
    {WorldStates::BG_AB_OP_FARM_STATE_ALIENCE,       WorldStates::BG_AB_OP_FARM_STATE_HORDE,         WorldStates::BG_AB_OP_FARM_STATE_CON_ALI,       WorldStates::BG_AB_OP_FARM_STATE_CON_HOR},
    {WorldStates::BG_AB_OP_LUMBERMILL_STATE_ALIENCE, WorldStates::BG_AB_OP_LUMBERMILL_STATE_HORDE,   WorldStates::BG_AB_OP_LUMBERMILL_STATE_CON_ALI, WorldStates::BG_AB_OP_LUMBERMILL_STATE_CON_HOR},
    {WorldStates::BG_AB_OP_GOLDMINE_STATE_ALIENCE,   WorldStates::BG_AB_OP_GOLDMINE_STATE_HORDE,     WorldStates::BG_AB_OP_GOLDMINE_STATE_CON_ALI,   WorldStates::BG_AB_OP_GOLDMINE_STATE_CON_HOR}
};

WorldStates const AbNodeStatesIconsWS[5] =
{
    WorldStates::BG_AB_OP_STABLE_ICON,
    WorldStates::BG_AB_OP_BLACKSMITH_ICON,
    WorldStates::BG_AB_OP_FARM_ICON,
    WorldStates::BG_AB_OP_LUMBERMILL_ICON,
    WorldStates::BG_AB_OP_GOLDMINE_ICON
};

enum BG_AB_Objectives
{
    AB_OBJECTIVE_ASSAULT_BASE           = 122,
    AB_OBJECTIVE_DEFEND_BASE            = 123
};

#define AB_EVENT_START_BATTLE               9158 // Achievement: Let's Get This Done

const float BgABNodePositions[BG_AB_DYNAMIC_NODES_COUNT][8] = ///< Position + Rotation
{
    {1166.856f, 1200.122f, -56.720f,  0.8860929f, 0.0f, 0.0f, 0.4286938f, 0.9034498f}, // BG_AB_OBJECTID_NODE_BANNER_0
    {977.5052f, 1051.073f, -44.792f,  0.497109f,  0.0f, 0.0f, 0.2460032f, 0.969269f},  // BG_AB_OBJECTID_NODE_BANNER_1
    {806.25f,   874.2795f, -56.0102f, 0.8182427f, 0.0f, 0.0f, 0.3978033f, 0.9174708f}, // BG_AB_OBJECTID_NODE_BANNER_2
    {856.8663f, 1150.214f, 11.38484f, 0.9897078f, 0.0f, 0.0f, 0.4749031f, 0.8800381f}, // BG_AB_OBJECTID_NODE_BANNER_3
    {1146.929f, 848.2274f, -110.917f, 2.417101f,  0.0f, 0.0f, 0.9351034f, 0.354375f}   // BG_AB_OBJECTID_NODE_BANNER_4
};

const float BgABDoorPositions[2][8] =
{
    {1284.597f, 1281.167f, -15.97792f, 0.7068594f, 0.012957f, -0.060288f, 0.344959f, 0.93659f},
    {708.0903f, 708.4479f, -17.8342f, -2.391099f, 0.050291f, 0.015127f, 0.929217f, -0.365784f}
};

Position const BgABSpiritGuidePos[BG_AB_ALL_NODES_COUNT] =
{
    { 1200.03f, 1171.09f, -56.47f, 5.15f },                   // stables
    { 1017.43f, 960.61f, -42.95f, 4.88f },                    // blacksmith
    { 833.00f, 793.00f, -57.25f, 5.27f },                     // farm
    { 775.17f, 1206.40f, 15.79f, 1.90f },                     // lumber mill
    { 1207.48f, 787.00f, -83.36f, 5.51f },                    // gold mine
    { 1354.05f, 1275.48f, -11.30f, 4.77f },                   // alliance starting base
    { 714.61f, 646.15f, -10.87f, 4.34f }                      // horde starting base
};

Milliseconds const BgABTickIntervals[6] = { Seconds(0), Seconds(12), Seconds(9), Seconds(6), Seconds(3), Seconds(1) };
uint32 const BgABTickPoints[6] = { 0, 10, 10, 10, 10, 60 };

Position const BgAbBuffPositions[BG_AB_DYNAMIC_NODES_COUNT] =
{
    {1185.71f, 1185.24f, -56.36f, 2.56f}, // stables
    {990.75f,  1008.18f, -42.60f, 2.43f}, // blacksmith
    {817.66f,  843.34f,  -56.54f, 3.01f}, // farm
    {807.46f,  1189.16f, 11.92f,  5.44f}, // lumber mill
    {1146.62f, 816.94f,  -98.49f, 6.14f}  // gold mine
};

Position const BgAbSpiritsPos[BG_AB_ALL_NODES_COUNT] =
{
    {1200.03f, 1171.09f, -56.47f, 5.15f}, // stables
    {1017.43f, 960.61f,  -42.95f, 4.88f}, // blacksmith
    {833.00f,  793.00f,  -57.25f, 5.27f}, // farm
    {775.17f,  1206.40f, 15.79f,  1.90f}, // lumber mill
    {1207.48f, 787.00f,  -83.36f, 5.51f}, // gold mine
    {1354.05f, 1275.48f, -11.30f, 4.77f}, // alliance starting base
    {714.61f,  646.15f,  -10.87f, 4.34f}  // horde starting base
};

struct BattlegroundABScore final : BattlegroundScore
{
    friend class BattlegroundArathiBasin;

protected:
    BattlegroundABScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), BasesAssaulted(0), BasesDefended(0) { }

    void UpdateScore(uint32 type, uint32 value) override;
    void BuildObjectivesBlock(std::vector<int32>& stats) override;

    uint32 BasesAssaulted;
    uint32 BasesDefended;
};

class BattlegroundArathiBasin : public Battleground
{
    struct CapturePointInfo
    {
        CapturePointInfo() = default;

        GameObject* Point = nullptr;
        Milliseconds Timer = Milliseconds(0);
        BgNodeStatus Status = NODE_STATUS_NEUTRAL;
        BgNodeStatus PrevStatus = NODE_STATUS_NEUTRAL;
        TeamId TeamID = TEAM_NONE;
    };

    CapturePointInfo _capturePoints[BG_AB_DYNAMIC_NODES_COUNT];
    Milliseconds _lastTick[MAX_TEAMS];
    uint32 _honorScoreTics[MAX_TEAMS];
    uint32 _reputationScoreTics[MAX_TEAMS];
    uint32 _honorTics;
    uint32 _reputationTics;
    bool _teamScores500Disadvantage[MAX_TEAMS];
    bool _isInformedNearVictory;
public:
    BattlegroundArathiBasin();
    ~BattlegroundArathiBasin();

    bool IsTeamScores500Disadvantage(uint32 team) const { return _teamScores500Disadvantage[MS::Battlegrounds::GetTeamIdByTeam(team)]; }

    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
    bool SetupBattleground() override;
    void Reset() override;
    void EndBattleground(uint32 winner) override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove) override;
    bool IsAllNodesConrolledByTeam(uint32 team) const override;
    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return BG_AB_MAX_TEAM_SCORE; }
    bool IsScoreIncremental() const override { return true; }
    void PostUpdateImpl(Milliseconds diff) override;
    uint8 _GetCapturedNodesForTeam(TeamId teamID);
    void _NodeOccupied(uint8 node, TeamId team);
    void _NodeDeOccupied(uint8 node);
};

#endif
