/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
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

#ifndef __BATTLEGROUNDWS_H
#define __BATTLEGROUNDWS_H

#include "Battleground.h"
#include "BattlegroundScore.h"

enum BG_WS_Spells
{
    BG_WS_SCRAMBLE_SPELL                      = 229908,

    SPELL_BG_WS_BRAWL_BUFF_1                  = 238485,
    SPELL_BG_WS_BRAWL_BUFF_2                  = 239223,
    SPELL_BG_WS_BRAWL_BUFF_3                  = 239224,
    SPELL_BG_WS_BRAWL_BUFF_4                  = 239227,
    SPELL_BG_WS_BRAWL_BUFF_GET                = 239221,

};

enum BG_WS_TimerOrScore
{
    BG_WS_MAX_TEAM_SCORE                    = 3,
    BG_WS_MAX_TEAM_SCORE_BRAWL              = 10,
    BG_WS_FLAG_RESPAWN_TIME                 = 10000,
    BG_WS_FLAG_DROP_TIME                    = 10000,
};

enum BG_WS_ObjectTypes
{
    BG_WS_OBJECT_DOOR_A_1       = 0,
    BG_WS_OBJECT_DOOR_A_2       = 1,
    BG_WS_OBJECT_DOOR_A_3       = 2,
    BG_WS_OBJECT_DOOR_A_4       = 3,
    BG_WS_OBJECT_DOOR_A_5       = 4,
    BG_WS_OBJECT_DOOR_A_6       = 5,
    BG_WS_OBJECT_DOOR_H_1       = 6,
    BG_WS_OBJECT_DOOR_H_2       = 7,
    BG_WS_OBJECT_DOOR_H_3       = 8,
    BG_WS_OBJECT_DOOR_H_4       = 9,
    BG_WS_OBJECT_A_FLAG         = 10,
    BG_WS_OBJECT_H_FLAG         = 11,
    BG_WS_OBJECT_SPEEDBUFF_1    = 12,
    BG_WS_OBJECT_SPEEDBUFF_2    = 13,
    BG_WS_OBJECT_REGENBUFF_1    = 14,
    BG_WS_OBJECT_REGENBUFF_2    = 15,
    BG_WS_OBJECT_BERSERKBUFF_1  = 16,
    BG_WS_OBJECT_BERSERKBUFF_2  = 17,
    BG_WS_OBJECT_MAX            = 18,

    BG_WS_BRAWL_A_FLAG_2        = 18,
    BG_WS_BRAWL_H_FLAG_2        = 19, 
    BG_WS_BRAWL_A_FLAG_2_POST   = 20,
    BG_WS_BRAWL_A_FLAG_3_POST   = 21,
    BG_WS_BRAWL_A_FLAG_3        = 22,
    BG_WS_BRAWL_H_FLAG_3        = 23,
    BG_WS_BRAWL_H_FLAG_2_POST   = 24,
    BG_WS_BRAWL_H_FLAG_3_POST   = 25,
    BG_WS_BRAWL_BUFF_1          = 26,
    BG_WS_BRAWL_BUFF_11         = 36,
    BG_WS_BRAWL_OBJECT_MAX      = 37,
};

enum BG_WS_ObjectEntry
{
    BG_OBJECT_DOOR_A_1_WS_ENTRY          = 179918,
    BG_OBJECT_DOOR_A_2_WS_ENTRY          = 179919,
    BG_OBJECT_DOOR_A_3_WS_ENTRY          = 179920,
    BG_OBJECT_DOOR_A_4_WS_ENTRY          = 179921,
    BG_OBJECT_DOOR_A_5_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_A_6_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_H_1_WS_ENTRY          = 179916,
    BG_OBJECT_DOOR_H_2_WS_ENTRY          = 179917,
    BG_OBJECT_DOOR_H_3_WS_ENTRY          = 180322,
    BG_OBJECT_DOOR_H_4_WS_ENTRY          = 180322,
    BG_OBJECT_A_FLAG_WS_ENTRY            = 227741,
    BG_OBJECT_H_FLAG_WS_ENTRY            = 227740,
    BG_OBJECT_A_FLAG_GROUND_WS_ENTRY     = 227745,
    BG_OBJECT_H_FLAG_GROUND_WS_ENTRY     = 227744,
    BG_BRAWL_BUFF_ENTRY                  = 268652,
    BG_BRAWL_H_POSTAMENT                 = 227038,
    BG_BRAWL_A_POSTAMENT                 = 269358,
};

enum BG_WS_FlagState
{
    BG_WS_FLAG_STATE_ON_BASE = 0,
    BG_WS_FLAG_STATE_WAIT_RESPAWN,
    BG_WS_FLAG_STATE_ON_PLAYER,
    BG_WS_FLAG_STATE_ON_GROUND,
};

enum BG_WS_Graveyards
{
    WS_GRAVEYARD_FLAGROOM_ALLIANCE = 769,
    WS_GRAVEYARD_FLAGROOM_HORDE    = 770,
    WS_GRAVEYARD_MAIN_ALLIANCE     = 771,
    WS_GRAVEYARD_MAIN_HORDE        = 772
};

enum BG_WS_CreatureTypes
{
    WS_SPIRIT_MAIN_ALLIANCE   = 0,
    WS_SPIRIT_MAIN_HORDE      = 1,

    BG_CREATURES_MAX_WS       = 2
};

enum BG_WS_Objectives
{
    WS_OBJECTIVE_CAPTURE_FLAG   = 42,
    WS_OBJECTIVE_RETURN_FLAG    = 44
};

struct BattlegroundWGScore final : BattlegroundScore
{
    friend class BattlegroundWarsongGulch;

protected:
    BattlegroundWGScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), FlagCaptures(0), FlagReturns(0) { }

    void UpdateScore(uint32 type, uint32 value) override;

    void BuildObjectivesBlock(std::vector<int32>& stats) override;

    uint32 FlagCaptures;
    uint32 FlagReturns;
};

class BattlegroundWarsongGulch : public Battleground
{
public:
    BattlegroundWarsongGulch();
    ~BattlegroundWarsongGulch();

    void PostUpdateImpl(uint32 diff) override;

    void AddPlayer(Player* player) override;
    void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
    void GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const override;

    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
    void Reset() override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

    void EndBattleground(uint32 winner) override;

    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void HandleKillPlayer(Player *player, Player *killer) override;

    ObjectGuid GetFlagPickerGUID(int32 team, uint8 j) const;
    void SetAllianceFlagPicker(ObjectGuid const& guid, uint8 j) { _flagKeepers[TEAM_ALLIANCE][j] = guid; }
    void SetHordeFlagPicker(ObjectGuid const& guid, uint8 j) { _flagKeepers[TEAM_HORDE][j] = guid; }
    bool IsAllianceFlagPickedup(uint8 j) const { return !_flagKeepers[TEAM_ALLIANCE][j].IsEmpty(); }
    bool IsHordeFlagPickedup(uint8 j) const { return !_flagKeepers[TEAM_HORDE][j].IsEmpty(); }
    void _CheckPositions(uint32 diff) override;
    bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
    void SetDroppedFlagGUID(ObjectGuid guid, uint32 TeamID, ObjectGuid playerguid);
    uint8 GetFlagState(uint32 team) { return _flagState[MS::Battlegrounds::GetTeamIdByTeam(team)][0]; }

    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return 3; }
    bool IsScoreIncremental() const override { return true; }
    void DoAction(uint32, ObjectGuid) override;
private:
    void UpdateFlagState(TeamId teamID, uint32 value, ObjectGuid flagKeeperGUID, uint8 j);
    void RespawnFlag(TeamId teamID, bool captured, uint8 j);
    void EventPlayerDroppedFlag(Player* source) override;
    void EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove) override;
    void EventPlayerCapturedFlag(Player* source);

    ObjectGuid _flagKeepers[MAX_TEAMS][3];
    ObjectGuid _droppedFlagGUID[MAX_TEAMS][3];
    int32 _flagsDropTimer[MAX_TEAMS][3]{{ 0,0,0 }, { 0,0,0 } };
    uint32 _reputationCapture;
    int32 _flagPosTimer;
    int32 _flagsTimer;
    int32 _flagSpellForceTimer;
    uint8 _flagState[MAX_TEAMS][3]{{ 0,0,0 }, { 0,0,0 }};
    uint8 _flagDebuffState;
    bool _bothFlagsKept;

    // brawl
    std::vector<std::pair<uint32, Position>> brawlObjects
    {
        { BG_OBJECT_A_FLAG_WS_ENTRY ,{ 1529.36f,	1474.93f,	351.987f,	2.65f } },
        { BG_OBJECT_H_FLAG_WS_ENTRY ,{ 923.747f, 1441.55f, 345.65f, 0.0f } },
    
        { BG_BRAWL_A_POSTAMENT ,{ 1529.36f,	1474.93f,	351.987f,	2.65f } },
        { BG_BRAWL_A_POSTAMENT ,{ 1530.17f,	1487.84f,	351.961f,	2.22625f } },

        { BG_OBJECT_A_FLAG_WS_ENTRY ,{ 1530.17f,	1487.84f,	351.961f,	2.22625f } },
        { BG_OBJECT_H_FLAG_WS_ENTRY ,{ 924.083f, 1428.02f, 345.69f, 0.0f } },
    
        { BG_BRAWL_H_POSTAMENT ,{ 923.747f, 1441.55f, 345.65f,	0.0f } },
        { BG_BRAWL_H_POSTAMENT ,{ 924.083f, 1428.02f, 345.69f, 0.0f } },

        { BG_BRAWL_BUFF_ENTRY,{ 1529.67f, 1456.66f, 352.054f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 924.41f, 1460.33f, 346.192f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1309.27f, 1433.74f, 314.984f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1519.48f, 1466.16f, 373.688f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1310.81f, 1509.97f, 317.845f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 922.786f, 1454.93f, 355.924f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 936.665f, 1453.63f, 367.29f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1533.11f, 1466.33f, 362.664f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1198.42f, 1512.86f, 306.883f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1049.04f, 1414.23f, 340.01f, 6.27189f } },
        { BG_BRAWL_BUFF_ENTRY,{ 1193.33f, 1403.46f, 307.12f, 6.27189f } },
    };

    std::vector<std::pair<ObjectGuid, uint32>> m_brawlAreatriggers{}; // [i] = guid - time
};

#endif
