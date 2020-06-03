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

#ifndef __BATTLEGROUNDEY_H
#define __BATTLEGROUNDEY_H

#include "Language.h"
#include "Battleground.h"
#include "BattlegroundScore.h"

enum BG_EY_Misc
{
    BG_EY_FPOINTS_TICK_TIME         = 2 * IN_MILLISECONDS
};

enum BG_EY_ProgressBarConsts
{
    BG_EY_POINT_MAX_CAPTURERS_COUNT     = 5,
    BG_EY_POINT_RADIUS                  = 70,
    BG_EY_PROGRESS_BAR_DONT_SHOW        = 0,
    BG_EY_PROGRESS_BAR_SHOW             = 1,
    BG_EY_PROGRESS_BAR_PERCENT_GREY     = 40,
    BG_EY_PROGRESS_BAR_STATE_MIDDLE     = 50,
    BG_EY_PROGRESS_BAR_HORDE_CONTROLLED = 0,
    BG_EY_PROGRESS_BAR_NEUTRAL_LOW      = 30,
    BG_EY_PROGRESS_BAR_NEUTRAL_HIGH     = 70,
    BG_EY_PROGRESS_BAR_ALI_CONTROLLED   = 100
};

enum BG_EY_Spells
{
    BG_EY_NETHERSTORM_FLAG_SPELL        = 34976,
    BG_EY_PLAYER_DROPPED_FLAG_SPELL     = 34991,

    BG_EY_BRAWL_GRAVITY_LAPSE           = 241706,
};

enum EYBattlegroundObjectEntry
{
    BG_OBJECT_A_DOOR_EY_ENTRY           = 184719,           //Alliance door
    BG_OBJECT_H_DOOR_EY_ENTRY           = 184720,           //Horde door
    BG_OBJECT_FLAG1_EY_ENTRY            = 208977,           //Netherstorm flag (flagstand)
    BG_OBJECT_FLAG2_EY_ENTRY            = 228508,           //Netherstorm flag (flagdrop)
    BG_OBJECT_A_BANNER_EY_ENTRY         = 184381,           //Visual Banner (Alliance)
    BG_OBJECT_H_BANNER_EY_ENTRY         = 184380,           //Visual Banner (Horde)
    BG_OBJECT_N_BANNER_EY_ENTRY         = 184382,           //Visual Banner (Neutral)
    BG_OBJECT_BE_TOWER_CAP_EY_ENTRY     = 184080,           //BE Tower Cap Pt
    BG_OBJECT_FR_TOWER_CAP_EY_ENTRY     = 184081,           //Fel Reaver Cap Pt
    BG_OBJECT_HU_TOWER_CAP_EY_ENTRY     = 184082,           //Human Tower Cap Pt
    BG_OBJECT_DR_TOWER_CAP_EY_ENTRY     = 184083            //Draenei Tower Cap Pt
};

const float EYObjectPos[][8] = 
{
    {2527.597f, 1596.906f, 1238.454f, 3.15914f,  0.1736417f, 0.001514435f,  -0.9847698f, 0.008638578f}, // BG_OBJECT_A_DOOR_EY_ENTRY
    {1803.207f, 1539.486f, 1238.454f, 3.138983f, 0.1736479f, 0.0f,          0.984807f,   0.001244878f}, // BG_OBJECT_H_DOOR_EY_ENTRY
};

enum EYBattlegroundPointsTrigger
{
    TR_BLOOD_ELF_BUFF       = 4568,
    TR_FEL_REAVER_BUFF      = 4569,
    TR_MAGE_TOWER_BUFF      = 4570,
    TR_DRAENEI_RUINS_BUFF   = 4571
};

enum EYBattlegroundGaveyards
{
    EY_GRAVEYARD_MAIN_ALLIANCE     = 1103,
    EY_GRAVEYARD_MAIN_HORDE        = 1104,
    EY_GRAVEYARD_FEL_REAVER        = 1105,
    EY_GRAVEYARD_BLOOD_ELF         = 1106,
    EY_GRAVEYARD_DRAENEI_RUINS     = 1107,
    EY_GRAVEYARD_MAGE_TOWER        = 1108
};

enum EYBattlegroundPoints
{
    FEL_REAVER                  = 0,
    BLOOD_ELF                   = 1,
    DRAENEI_RUINS               = 2,
    MAGE_TOWER                  = 3,

    EY_PLAYERS_OUT_OF_POINTS    = 4,
    EY_POINTS_MAX               = 4
};

enum EYBattlegroundCreaturesTypes
{
    EY_SPIRIT_FEL_REAVER      = 0,
    EY_SPIRIT_BLOOD_ELF        = 1,
    EY_SPIRIT_DRAENEI_RUINS    = 2,
    EY_SPIRIT_MAGE_TOWER       = 3,
    EY_SPIRIT_MAIN_ALLIANCE    = 4,
    EY_SPIRIT_MAIN_HORDE       = 5,

    EY_TRIGGER_FEL_REAVER      = 6,
    EY_TRIGGER_BLOOD_ELF        = 7,
    EY_TRIGGER_DRAENEI_RUINS    = 8,
    EY_TRIGGER_MAGE_TOWER       = 9,

    BG_EY_CREATURES_MAX        = 10
};

enum EYBattlegroundObjectTypes
{
    BG_EY_OBJECT_DOOR_A                         = 0,
    BG_EY_OBJECT_DOOR_H                         = 1,
    BG_EY_OBJECT_A_BANNER_FEL_REAVER_CENTER    = 2,
    BG_EY_OBJECT_A_BANNER_FEL_REAVER_LEFT      = 3,
    BG_EY_OBJECT_A_BANNER_FEL_REAVER_RIGHT     = 4,
    BG_EY_OBJECT_A_BANNER_BLOOD_ELF_CENTER      = 5,
    BG_EY_OBJECT_A_BANNER_BLOOD_ELF_LEFT        = 6,
    BG_EY_OBJECT_A_BANNER_BLOOD_ELF_RIGHT       = 7,
    BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_CENTER  = 8,
    BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_LEFT    = 9,
    BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_RIGHT   = 10,
    BG_EY_OBJECT_A_BANNER_MAGE_TOWER_CENTER     = 11,
    BG_EY_OBJECT_A_BANNER_MAGE_TOWER_LEFT       = 12,
    BG_EY_OBJECT_A_BANNER_MAGE_TOWER_RIGHT      = 13,
    BG_EY_OBJECT_H_BANNER_FEL_REAVER_CENTER    = 14,
    BG_EY_OBJECT_H_BANNER_FEL_REAVER_LEFT      = 15,
    BG_EY_OBJECT_H_BANNER_FEL_REAVER_RIGHT     = 16,
    BG_EY_OBJECT_H_BANNER_BLOOD_ELF_CENTER      = 17,
    BG_EY_OBJECT_H_BANNER_BLOOD_ELF_LEFT        = 18,
    BG_EY_OBJECT_H_BANNER_BLOOD_ELF_RIGHT       = 19,
    BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_CENTER  = 20,
    BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_LEFT    = 21,
    BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_RIGHT   = 22,
    BG_EY_OBJECT_H_BANNER_MAGE_TOWER_CENTER     = 23,
    BG_EY_OBJECT_H_BANNER_MAGE_TOWER_LEFT       = 24,
    BG_EY_OBJECT_H_BANNER_MAGE_TOWER_RIGHT      = 25,
    BG_EY_OBJECT_N_BANNER_FEL_REAVER_CENTER    = 26,
    BG_EY_OBJECT_N_BANNER_FEL_REAVER_LEFT      = 27,
    BG_EY_OBJECT_N_BANNER_FEL_REAVER_RIGHT     = 28,
    BG_EY_OBJECT_N_BANNER_BLOOD_ELF_CENTER      = 29,
    BG_EY_OBJECT_N_BANNER_BLOOD_ELF_LEFT        = 30,
    BG_EY_OBJECT_N_BANNER_BLOOD_ELF_RIGHT       = 31,
    BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_CENTER  = 32,
    BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_LEFT    = 33,
    BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_RIGHT   = 34,
    BG_EY_OBJECT_N_BANNER_MAGE_TOWER_CENTER     = 35,
    BG_EY_OBJECT_N_BANNER_MAGE_TOWER_LEFT       = 36,
    BG_EY_OBJECT_N_BANNER_MAGE_TOWER_RIGHT      = 37,
    BG_EY_OBJECT_TOWER_CAP_FEL_REAVER          = 38,
    BG_EY_OBJECT_TOWER_CAP_BLOOD_ELF            = 39,
    BG_EY_OBJECT_TOWER_CAP_DRAENEI_RUINS        = 40,
    BG_EY_OBJECT_TOWER_CAP_MAGE_TOWER           = 41,
    BG_EY_OBJECT_FLAG_NETHERSTORM               = 42,
    //buffs
    BG_EY_OBJECT_SPEEDBUFF_FEL_REAVER,
    BG_EY_OBJECT_REGENBUFF_FEL_REAVER,
    BG_EY_OBJECT_BERSERKBUFF_FEL_REAVER,
    BG_EY_OBJECT_SPEEDBUFF_BLOOD_ELF,
    BG_EY_OBJECT_REGENBUFF_BLOOD_ELF,
    BG_EY_OBJECT_BERSERKBUFF_BLOOD_ELF,
    BG_EY_OBJECT_SPEEDBUFF_DRAENEI_RUINS,
    BG_EY_OBJECT_REGENBUFF_DRAENEI_RUINS,
    BG_EY_OBJECT_BERSERKBUFF_DRAENEI_RUINS,
    BG_EY_OBJECT_SPEEDBUFF_MAGE_TOWER,
    BG_EY_OBJECT_REGENBUFF_MAGE_TOWER,
    BG_EY_OBJECT_BERSERKBUFF_MAGE_TOWER,

    BG_EY_OBJECT_MAX
};

Position const PointData[] = 
{
    {2044.28f, 1729.68f, 1189.96f},
    {2048.83f, 1393.65f, 1194.49f},
    {2286.56f, 1402.36f, 1197.11f},
    {2284.48f, 1731.23f, 1189.99f}
};

#define EY_EVENT_START_BATTLE           13180 // Achievement: Flurry

enum BG_EY_Score
{
    BG_EY_WARNING_NEAR_VICTORY_SCORE    = 1200,
    BG_EY_MAX_TEAM_SCORE                = 1500
};

enum BG_EY_FlagState
{
    BG_EY_FLAG_STATE_ON_BASE      = 0,
    BG_EY_FLAG_STATE_WAIT_RESPAWN = 1,
    BG_EY_FLAG_STATE_ON_PLAYER    = 2,
    BG_EY_FLAG_STATE_ON_GROUND    = 3
};

enum EYBattlegroundPointState
{
    EY_POINT_NO_OWNER           = 0,
    EY_POINT_STATE_UNCONTROLLED = 0,
    EY_POINT_UNDER_CONTROL      = 3
};

enum BG_EY_Objectives
{
    EY_OBJECTIVE_CAPTURE_FLAG   = 183
};

struct BattlegroundEYPointIconsStruct
{
    BattlegroundEYPointIconsStruct(WorldStates _WorldStateControlIndex, WorldStates _WorldStateAllianceControlledIndex, WorldStates _WorldStateHordeControlledIndex)
        : WorldStateControlIndex(_WorldStateControlIndex), WorldStateAllianceControlledIndex(_WorldStateAllianceControlledIndex), WorldStateHordeControlledIndex(_WorldStateHordeControlledIndex) {}
    
    WorldStates WorldStateControlIndex;
    WorldStates WorldStateAllianceControlledIndex;
    WorldStates WorldStateHordeControlledIndex;
};

// x, y, z, o
const float BG_EY_TriggerPositions[EY_POINTS_MAX][4] =
{
    {2044.28f, 1729.68f, 1189.96f, 0.017453f},  // FEL_REAVER center
    {2048.83f, 1393.65f, 1194.49f, 0.20944f},   // BLOOD_ELF center
    {2286.56f, 1402.36f, 1197.11f, 3.72381f},   // DRAENEI_RUINS center
    {2284.48f, 1731.23f, 1189.99f, 2.89725f}    // MAGE_TOWER center
};

struct BattlegroundEYLosingPointStruct
{
    BattlegroundEYLosingPointStruct(uint32 _SpawnNeutralObjectType, uint32 _DespawnObjectTypeAlliance, uint32 _MessageIdAlliance, uint32 _DespawnObjectTypeHorde, uint32 _MessageIdHorde)
        : SpawnNeutralObjectType(_SpawnNeutralObjectType),
        DespawnObjectTypeAlliance(_DespawnObjectTypeAlliance), MessageIdAlliance(_MessageIdAlliance),
        DespawnObjectTypeHorde(_DespawnObjectTypeHorde), MessageIdHorde(_MessageIdHorde)
    {}

    uint32 SpawnNeutralObjectType;
    uint32 DespawnObjectTypeAlliance;
    uint32 MessageIdAlliance;
    uint32 DespawnObjectTypeHorde;
    uint32 MessageIdHorde;
};

struct BattlegroundEYCapturingPointStruct
{
    BattlegroundEYCapturingPointStruct(uint32 _DespawnNeutralObjectType, uint32 _SpawnObjectTypeAlliance, uint32 _MessageIdAlliance, uint32 _SpawnObjectTypeHorde, uint32 _MessageIdHorde, uint32 _GraveYardId)
        : DespawnNeutralObjectType(_DespawnNeutralObjectType),
        SpawnObjectTypeAlliance(_SpawnObjectTypeAlliance), MessageIdAlliance(_MessageIdAlliance),
        SpawnObjectTypeHorde(_SpawnObjectTypeHorde), MessageIdHorde(_MessageIdHorde),
        GraveYardId(_GraveYardId)
    {}

    uint32 DespawnNeutralObjectType;
    uint32 SpawnObjectTypeAlliance;
    uint32 MessageIdAlliance;
    uint32 SpawnObjectTypeHorde;
    uint32 MessageIdHorde;
    uint32 GraveYardId;
};

uint8 const  BG_EY_TickPoints[EY_POINTS_MAX] = {1, 2, 5, 10};
uint32 const BG_EY_FlagPoints[EY_POINTS_MAX] = {75, 85, 100, 500};

enum BG_EY_BroadcastTexts
{
    BG_EY_TEXT_ALLIANCE_TAKEN_FEL_REAVER_RUINS  = 17828,
    BG_EY_TEXT_HORDE_TAKEN_FEL_REAVER_RUINS     = 17829,
    BG_EY_TEXT_ALLIANCE_LOST_FEL_REAVER_RUINS   = 17835,
    BG_EY_TEXT_HORDE_LOST_FEL_REAVER_RUINS      = 17836,

    BG_EY_TEXT_ALLIANCE_TAKEN_BLOOD_ELF_TOWER   = 17819,
    BG_EY_TEXT_HORDE_TAKEN_BLOOD_ELF_TOWER      = 17823,
    BG_EY_TEXT_ALLIANCE_LOST_BLOOD_ELF_TOWER    = 17831,
    BG_EY_TEXT_HORDE_LOST_BLOOD_ELF_TOWER       = 17832,

    BG_EY_TEXT_ALLIANCE_TAKEN_DRAENEI_RUINS     = 17826,
    BG_EY_TEXT_HORDE_TAKEN_DRAENEI_RUINS        = 17827,
    BG_EY_TEXT_ALLIANCE_LOST_DRAENEI_RUINS      = 17833,
    BG_EY_TEXT_HORDE_LOST_DRAENEI_RUINS         = 17834,

    BG_EY_TEXT_ALLIANCE_TAKEN_MAGE_TOWER        = 17824,
    BG_EY_TEXT_HORDE_TAKEN_MAGE_TOWER           = 17825,
    BG_EY_TEXT_ALLIANCE_LOST_MAGE_TOWER         = 17837,
    BG_EY_TEXT_HORDE_LOST_MAGE_TOWER            = 17838,

    BG_EY_TEXT_TAKEN_FLAG                       = 18359,
    BG_EY_TEXT_FLAG_DROPPED                     = 18361,
    BG_EY_TEXT_FLAG_RESET                       = 18364,
    BG_EY_TEXT_ALLIANCE_CAPTURED_FLAG           = 18375,
    BG_EY_TEXT_HORDE_CAPTURED_FLAG              = 18384,
};

//constant arrays:
const BattlegroundEYPointIconsStruct m_PointsIconStruct[EY_POINTS_MAX] =
{
    BattlegroundEYPointIconsStruct(WorldStates::FEL_REAVER_UNCONTROL, WorldStates::FEL_REAVER_ALLIANCE_CONTROL, WorldStates::FEL_REAVER_HORDE_CONTROL),
    BattlegroundEYPointIconsStruct(WorldStates::BLOOD_ELF_UNCONTROL, WorldStates::BLOOD_ELF_ALLIANCE_CONTROL, WorldStates::BLOOD_ELF_HORDE_CONTROL),
    BattlegroundEYPointIconsStruct(WorldStates::DRAENEI_RUINS_UNCONTROL, WorldStates::DRAENEI_RUINS_ALLIANCE_CONTROL, WorldStates::DRAENEI_RUINS_HORDE_CONTROL),
    BattlegroundEYPointIconsStruct(WorldStates::MAGE_TOWER_UNCONTROL, WorldStates::MAGE_TOWER_ALLIANCE_CONTROL, WorldStates::MAGE_TOWER_HORDE_CONTROL)
};
const BattlegroundEYLosingPointStruct m_LosingPointTypes[EY_POINTS_MAX] =
{
    BattlegroundEYLosingPointStruct(BG_EY_OBJECT_N_BANNER_FEL_REAVER_CENTER, BG_EY_OBJECT_A_BANNER_FEL_REAVER_CENTER, BG_EY_TEXT_ALLIANCE_LOST_FEL_REAVER_RUINS, BG_EY_OBJECT_H_BANNER_FEL_REAVER_CENTER, BG_EY_TEXT_HORDE_LOST_FEL_REAVER_RUINS),
    BattlegroundEYLosingPointStruct(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_CENTER, BG_EY_OBJECT_A_BANNER_BLOOD_ELF_CENTER, BG_EY_TEXT_ALLIANCE_LOST_BLOOD_ELF_TOWER, BG_EY_OBJECT_H_BANNER_BLOOD_ELF_CENTER, BG_EY_TEXT_HORDE_LOST_BLOOD_ELF_TOWER),
    BattlegroundEYLosingPointStruct(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_CENTER, BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_CENTER, BG_EY_TEXT_ALLIANCE_LOST_DRAENEI_RUINS, BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_CENTER, BG_EY_TEXT_HORDE_LOST_DRAENEI_RUINS),
    BattlegroundEYLosingPointStruct(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_CENTER, BG_EY_OBJECT_A_BANNER_MAGE_TOWER_CENTER, BG_EY_TEXT_ALLIANCE_LOST_MAGE_TOWER, BG_EY_OBJECT_H_BANNER_MAGE_TOWER_CENTER, BG_EY_TEXT_HORDE_LOST_MAGE_TOWER)
};
const BattlegroundEYCapturingPointStruct m_CapturingPointTypes[EY_POINTS_MAX] =
{
    BattlegroundEYCapturingPointStruct(BG_EY_OBJECT_N_BANNER_FEL_REAVER_CENTER, BG_EY_OBJECT_A_BANNER_FEL_REAVER_CENTER, BG_EY_TEXT_ALLIANCE_TAKEN_FEL_REAVER_RUINS, BG_EY_OBJECT_H_BANNER_FEL_REAVER_CENTER, BG_EY_TEXT_HORDE_TAKEN_FEL_REAVER_RUINS, EY_GRAVEYARD_FEL_REAVER),
    BattlegroundEYCapturingPointStruct(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_CENTER, BG_EY_OBJECT_A_BANNER_BLOOD_ELF_CENTER, BG_EY_TEXT_ALLIANCE_TAKEN_BLOOD_ELF_TOWER, BG_EY_OBJECT_H_BANNER_BLOOD_ELF_CENTER, BG_EY_TEXT_HORDE_TAKEN_BLOOD_ELF_TOWER, EY_GRAVEYARD_BLOOD_ELF),
    BattlegroundEYCapturingPointStruct(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_CENTER, BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_CENTER, BG_EY_TEXT_ALLIANCE_TAKEN_DRAENEI_RUINS, BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_CENTER, BG_EY_TEXT_HORDE_TAKEN_DRAENEI_RUINS, EY_GRAVEYARD_DRAENEI_RUINS),
    BattlegroundEYCapturingPointStruct(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_CENTER, BG_EY_OBJECT_A_BANNER_MAGE_TOWER_CENTER, BG_EY_TEXT_ALLIANCE_TAKEN_MAGE_TOWER, BG_EY_OBJECT_H_BANNER_MAGE_TOWER_CENTER, BG_EY_TEXT_HORDE_TAKEN_MAGE_TOWER, EY_GRAVEYARD_MAGE_TOWER)
};

struct BattlegroundEYScore final : BattlegroundScore
{
    friend class BattlegroundEyeOfTheStorm;

    protected:
        BattlegroundEYScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), FlagCaptures(0) { }

        void UpdateScore(uint32 type, uint32 value) override
        {
            switch (type)
            {
                case SCORE_FLAG_CAPTURES:
                    FlagCaptures += value;
                    break;
                default:
                    BattlegroundScore::UpdateScore(type, value);
                    break;
            }
        }

        void BuildObjectivesBlock(std::vector<int32>& stats) override
        {
            stats.push_back(FlagCaptures);
        }

        uint32 FlagCaptures;
};

class BattlegroundEyeOfTheStorm : public Battleground
{
    public:
        BattlegroundEyeOfTheStorm();
        ~BattlegroundEyeOfTheStorm();

        void AddPlayer(Player* player) override;
        void StartingEventCloseDoors() override;
        void StartingEventOpenDoors() override;
        void GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const override;

        ObjectGuid GetFlagPickerGUID(int32 /*team*/ = -1) const { return _flagKeeper; }
        void SetFlagPicker(ObjectGuid guid) { _flagKeeper = guid; }
        bool IsFlagPickedup() const { return !_flagKeeper.IsEmpty(); }
        uint8 GetFlagState() const { return _flagState; }
        void RespawnFlag();
        void RespawnFlagAfterDrop();

        void RemovePlayer(Player* player, ObjectGuid guid, uint32 team) override;
        void HandleKillPlayer(Player* player, Player* killer) override;
        WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
        bool SetupBattleground() override;
        void Reset() override;
        void UpdateTeamScore(TeamId teamID);
        void EndBattleground(uint32 winner) override;
        bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
        void SetDroppedFlagGUID(ObjectGuid guid) { _droppedFlagGUID = guid; }
        ObjectGuid GetDroppedFlagGUID() const { return _droppedFlagGUID; }

        void EventPlayerClickedOnFlag(Player* Source, GameObject* target_obj, bool& canRemove) override;
        void EventPlayerDroppedFlag(Player* Source) override;

        bool IsAllNodesConrolledByTeam(uint32 team) const override;

        uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
        uint32 GetMaxScore() const override { return BG_EY_MAX_TEAM_SCORE; }
        bool IsScoreIncremental() const override { return true; }
    private:
        void PostUpdateImpl(uint32 diff) override;

        void EventPlayerCapturedFlag(Player* Source);
        void EventTeamCapturedPoint(Player* Source, uint32 Point);
        void EventTeamLostPoint(Player* Source, uint32 Point);
        void UpdatePointsCount(uint32 Team);
        void UpdatePointsIcons(uint32 Team, uint32 Point);

        void _CheckSomeoneLeftPoint();
        void _CheckSomeoneJoinedPoint();
        void UpdatePointStatuses();

        void AddPoints(TeamId teamID, uint32 points);
        
        GuidVector _playersNearPoint[EY_POINTS_MAX + 1];
        ObjectGuid _flagKeeper;
        ObjectGuid _droppedFlagGUID;
        uint32 _honorScoreTics[MAX_TEAMS];
        uint32 _teamPointsCount[MAX_TEAMS];
        uint32 _pointsTrigger[EY_POINTS_MAX];
        int32 _checkFlagPickuperPos;
        int32 _flagsTimer;
        int32 _towerCapCheckTimer;
        uint32 _pointOwnedByTeam[EY_POINTS_MAX];
        int32 _pointBarStatus[EY_POINTS_MAX];
        int32 _pointAddingTimer;
        uint32 _honorTics;
        uint8 _flagState;
        uint8 _pointState[EY_POINTS_MAX];
        uint8 _currentPointPlayersCount[2 * EY_POINTS_MAX];
        bool _isInformedNearVictory;
        uint32 m_brawlTimer;
        bool m_brawlAnnounceWas = false;
};
#endif

