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

#ifndef __BATTLEGROUNDTP_H
#define __BATTLEGROUNDTP_H

#include "Battleground.h"
#include "BattlegroundScore.h"

enum BG_TP_TimerOrScore
{
    BG_TP_MAX_TEAM_SCORE                    = 3,
    BG_TP_FLAG_RESPAWN_TIME                 = 23000,
    BG_TP_FLAG_DROP_TIME                    = 10000,
    BG_TP_SPELL_FORCE_TIME                  = 600000,
    BG_TP_SPELL_BRUTAL_TIME                 = 900000
};

enum BG_TP_ObjectTypes
{
    BG_TP_OBJECT_DOOR_A_1 = 0,
    BG_TP_OBJECT_DOOR_A_2,
    BG_TP_OBJECT_DOOR_A_3,
    BG_TP_OBJECT_DOOR_H_1,
    BG_TP_OBJECT_DOOR_H_2,
    BG_TP_OBJECT_DOOR_H_3,
    BG_TP_OBJECT_A_FLAG,
    BG_TP_OBJECT_H_FLAG,
    BG_TP_OBJECT_SPEEDBUFF_1,
    BG_TP_OBJECT_SPEEDBUFF_2,
    BG_TP_OBJECT_REGENBUFF_1,
    BG_TP_OBJECT_REGENBUFF_2,
    BG_TP_OBJECT_BERSERKBUFF_1,
    BG_TP_OBJECT_BERSERKBUFF_2,
    BG_TP_OBJECT_MAX
};

enum BG_TP_ObjectEntry
{
    BG_OBJECT_DOOR_A_1_TP_ENTRY             = 206653,
    BG_OBJECT_DOOR_A_2_TP_ENTRY             = 206654,
    BG_OBJECT_DOOR_A_3_TP_ENTRY             = 206655,
    BG_OBJECT_DOOR_H_1_TP_ENTRY             = 208205,
    BG_OBJECT_DOOR_H_2_TP_ENTRY             = 208206,
    BG_OBJECT_DOOR_H_3_TP_ENTRY             = 208207,
    BG_OBJECT_A_FLAG_TP_ENTRY               = 227741,
    BG_OBJECT_H_FLAG_TP_ENTRY               = 227740,
    BG_OBJECT_A_FLAG_GROUND_TP_ENTRY        = 227745,
    BG_OBJECT_H_FLAG_GROUND_TP_ENTRY        = 227744
};

enum BG_TP_FlagState
{
    BG_TP_FLAG_STATE_ON_BASE,
    BG_TP_FLAG_STATE_WAIT_RESPAWN,
    BG_TP_FLAG_STATE_ON_PLAYER,
    BG_TP_FLAG_STATE_ON_GROUND,
};

enum BG_TP_Graveyards
{
    TP_GRAVEYARD_FLAGROOM_ALLIANCE          = 0,
    TP_GRAVEYARD_FLAGROOM_HORDE             = 1,

    TP_GRAVEYARD_START_ALLIANCE             = 2,
    TP_GRAVEYARD_START_HORDE                = 3,

    TP_GRAVEYARD_MIDDLE_ALLIANCE            = 4,
    TP_GRAVEYARD_MIDDLE_HORDE               = 5,

    TP_MAX_GRAVEYARDS                       = 6
};

uint32 const BG_TP_GraveyardIds[TP_MAX_GRAVEYARDS] = { 1726, 1727, 1729, 1728, 1749, 1750 };

enum BG_TP_CreatureTypes
{
    TP_SPIRIT_ALLIANCE = 0,
    TP_SPIRIT_HORDE,

    BG_CREATURES_MAX_TP
};

enum BG_TP_Objectives
{
    TP_OBJECTIVE_CAPTURE_FLAG               = 290,
    TP_OBJECTIVE_RETURN_FLAG                = 291
};

class BattlegroundTPScore final : public BattlegroundScore
{
    friend class BattlegroundTwinPeaks;

protected:
    BattlegroundTPScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), FlagCaptures(0), FlagReturns(0) { }

    void UpdateScore(uint32 type, uint32 value) override;

    void BuildObjectivesBlock(std::vector<int32>& stats) override;

    uint32 FlagCaptures;
    uint32 FlagReturns;
};

class BattlegroundTwinPeaks : public Battleground
{
public:
    BattlegroundTwinPeaks();
    ~BattlegroundTwinPeaks();

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
    void HandleKillPlayer(Player* player, Player* killer) override;

    ObjectGuid GetFlagPickerGUID(int32 team) const;
    void SetAllianceFlagPicker(ObjectGuid const& guid) { _flagKeepers[TEAM_ALLIANCE] = guid; }
    void SetHordeFlagPicker(ObjectGuid const& guid) { _flagKeepers[TEAM_HORDE] = guid; }
    bool IsAllianceFlagPickedup() const { return !_flagKeepers[TEAM_ALLIANCE].IsEmpty(); }
    bool IsHordeFlagPickedup() const { return !_flagKeepers[TEAM_HORDE].IsEmpty(); }

    void _CheckPositions(uint32 diff) override;

    bool UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true) override;
    void SetDroppedFlagGUID(ObjectGuid guid, uint32 TeamID) { _droppedFlagGUID[TeamID] = guid; }

    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return 3; }
    bool IsScoreIncremental() const override { return true; }
private:
    void UpdateFlagState(uint32 team, uint32 value, ObjectGuid flagKeeperGUID = ObjectGuid::Empty);
    void RespawnFlag(uint32 team, bool captured = false);
    void EventPlayerDroppedFlag(Player* source) override;
    void EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove) override;
    void EventPlayerCapturedFlag(Player* source);

    ObjectGuid _flagKeepers[MAX_TEAMS];
    ObjectGuid _droppedFlagGUID[MAX_TEAMS];
    int32 _flagsDropTimer[MAX_TEAMS];
    int32 _flagsTimer;
    int32 _flagSpellForceTimer;
    uint8 _flagState[MAX_TEAMS];
    uint8 _flagDebuffState;
    bool _bothFlagsKept;
};

#endif
