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

#ifndef __BattlegroundBFG_H
#define __BattlegroundBFG_H

enum GILNEAS_BG_ObjectType
{
    BG_BFG_OBJECT_BANNER                        = 0,

    GILNEAS_BG_OBJECT_GATE_A_1                  = 24,
    GILNEAS_BG_OBJECT_GATE_A_2                  = 25,
    GILNEAS_BG_OBJECT_GATE_H_1                  = 26,
    GILNEAS_BG_OBJECT_GATE_H_2                  = 27,

    //buffs
    GILNEAS_BG_OBJECT_SPEEDBUFF_LIGHTHOUSE      = 28,
    GILNEAS_BG_OBJECT_REGENBUFF_LIGHTHOUSE      = 29,
    GILNEAS_BG_OBJECT_BERSERKBUFF_LIGHTHOUSE    = 30,
    GILNEAS_BG_OBJECT_SPEEDBUFF_WATERWORKS      = 31,
    GILNEAS_BG_OBJECT_REGENBUFF_WATERWORKS      = 32,
    GILNEAS_BG_OBJECT_BERSERKBUFF_WATERWORKS    = 33,
    GILNEAS_BG_OBJECT_SPEEDBUFF_MINE            = 34,
    GILNEAS_BG_OBJECT_REGENBUFF_MINE            = 35,
    GILNEAS_BG_OBJECT_BERSERKBUFF_MINE          = 36,

    GILNEAS_BG_OBJECT_MAX                       = 37
};

enum GILNEAS_BG_ObjectTypes
{
    BG_BFG_GO_NODE_BANNER_0                     = 228050, // Lighthouse banner
    BG_BFG_GO_NODE_BANNER_1                     = 228052, // Waterworks banner
    BG_BFG_GO_NODE_BANNER_2                     = 228053, // Mines banner

    BG_BFG_GO_GATE_A_1                          = 207177,
    BG_BFG_GO_GATE_A_2                          = 205496,

    BG_BFG_GO_GATE_H_1                          = 207178,
    BG_BFG_GO_GATE_H_2                          = 205495,
};

enum GILNEAS_BG_Score
{
    GILNEAS_BG_WARNING_NEAR_VICTORY_SCORE       = 1200,
    GILNEAS_BG_MAX_TEAM_SCORE                   = 1500
};

enum GILNEAS_BG_BattlegroundNodes
{
    GILNEAS_BG_NODE_LIGHTHOUSE                  = 0,
    GILNEAS_BG_NODE_WATERWORKS                  = 1,
    GILNEAS_BG_NODE_MINE                        = 2,

    GILNEAS_BG_DYNAMIC_NODES_COUNT              = 3,

    GILNEAS_BG_SPIRIT_ALIANCE                   = 3,
    GILNEAS_BG_SPIRIT_HORDE                     = 4,

    GILNEAS_BG_ALL_NODES_COUNT
};

enum GILNEAS_BG_Objectives
{
    BG_OBJECTIVE_ASSAULT_BASE                   = 370,
    BG_OBJECTIVE_DEFEND_BASE                    = 371
};

const float BgBfgNodePosition[GILNEAS_BG_DYNAMIC_NODES_COUNT][8] =
{
    {1057.74f, 1278.262f, 3.179371f, 4.909808f, 0.0f, 0.0f, -0.6339798f, 0.7733496f}, // 228050 Lighthouse
    {980.033f, 948.7379f, 12.73536f, 5.877464f, 0.0f, 0.0f, -0.2014723f, 0.9794942f}, // 228052 Waterworks
    {1251.009f, 958.2691f, 5.668475f, 5.86425f, 0.0f, 0.0f, -0.2079391f, 0.9781418f}, // 228053 Mine
};

const float BgBfgDoorPos[4][8] =
{
    {918.3906f, 1336.641f, 27.4252f, 2.844883f, 0.0f, 0.0f, 0.9890156f, 0.1478114f},    // BG_BFG_GO_GATE_A_1
    {918.2986f, 1336.49f, 20.455f, 2.82743f, 0.0f, 0.0f, 0.9876881f, 0.1564362f},       // BG_BFG_GO_GATE_A_2
    {1395.97f, 977.0903f, 7.63597f, 6.274459f, 0.0f, 0.0f, -0.00436306f, 0.9999905f},   // BG_BFG_GO_GATE_H_1
    {1395.96f, 977.257f, -13.7897f, 6.265733f, 0.0f, 0.0f, -0.00872612f, 0.9999619f},   // BG_BFG_GO_GATE_H_2
};

Milliseconds const BgBFGTickIntervals[4] = {Seconds(0), Seconds(9), Seconds(3), Seconds(1)};
uint32 const BgBFGTickPoints[4] = {0, 10, 10, 30};
uint32 const GILNEAS_BG_GraveyardIds[GILNEAS_BG_ALL_NODES_COUNT] = { 1736, 1738, 1735, 1740, 1739 };

Position const BgBfgSpiritGuidePos[GILNEAS_BG_ALL_NODES_COUNT] =
{
    { 1034.82f, 1335.58f, 13.00f, 5.27f },     // Lighthouse
    { 887.57f,  937.337f, 23.77f, 4.88f },     // Waterworks
    { 1252.23f, 836.547f, 27.78f, 5.51f },     // Mine    
    { 909.46f,  1337.36f, 27.64f, 6.04f },     // Alliance
    { 1401.38f, 977.12f,   7.43f, 3.14f },     // Horde
};

float const BgBFGBuffsPos[GILNEAS_BG_DYNAMIC_NODES_COUNT][4] =
{
    { 1063.57f, 1313.42f, 4.91f, 4.14f },        // Lighthouse
    { 961.830f, 977.03f, 14.15f, 4.55f },        // Waterworks
    { 1193.09f, 1017.46f, 7.98f, 0.24f },        // Mine
};

class BattlegroundBFGScore final : public BattlegroundScore
{
    friend class BattlegroundBattleForGilneas;

    protected:
        BattlegroundBFGScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), BasesAssaulted(0), BasesDefended(0) { }

        void UpdateScore(uint32 type, uint32 value) override
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

        void BuildObjectivesBlock(std::vector<int32>& stats) override
        {
            stats.push_back(BasesAssaulted);
            stats.push_back(BasesDefended);
        }

        uint32 BasesAssaulted;
        uint32 BasesDefended;
};

class BattlegroundBattleForGilneas : public Battleground
{
    struct CapturePointInfo
    {
        GameObject* Point;
        Milliseconds Timer;
        BgNodeStatus Status;
        BgNodeStatus PrevStatus;
        TeamId TeamID;
    };

    public:
        BattlegroundBattleForGilneas();
        ~BattlegroundBattleForGilneas();
        
        void PostUpdateImpl(Milliseconds diff) override;

        void AddPlayer(Player* player) override;
        void StartingEventCloseDoors() override;
        void StartingEventOpenDoors() override;
        void Reset() override;
        WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;

        bool SetupBattleground() override;
        void EndBattleground(uint32 winner) override;

        bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

        void EventPlayerClickedOnFlag(Player* source, GameObject* /*object*/, bool& canRemove) override;

        bool IsAllNodesConrolledByTeam(uint32 teamID) const override;
        uint8 _GetCapturedNodesForTeam(TeamId teamID);

        bool IsTeamScores500Disadvantage(uint32 team) const { return _teamScores500Disadvantage[MS::Battlegrounds::GetTeamIdByTeam(team)]; }

        uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
        uint32 GetMaxScore() const override { return GILNEAS_BG_MAX_TEAM_SCORE; }
        bool IsScoreIncremental() const override { return true; }
    private:

        void _NodeOccupied(uint8 node, TeamId team);
        void _NodeDeOccupied(uint8 node);

        CapturePointInfo _capturePoints[GILNEAS_BG_DYNAMIC_NODES_COUNT];

        Milliseconds _lastTick[MAX_TEAMS];
        uint32 _honorScoreTicks[MAX_TEAMS];
        bool _teamScores500Disadvantage[MAX_TEAMS];

        bool _IsInformedNearVictory;
        uint32 _HonorTicks;
};

#endif
