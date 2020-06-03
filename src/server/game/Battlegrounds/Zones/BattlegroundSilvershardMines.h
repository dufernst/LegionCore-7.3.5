/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#ifndef __BATTLEGROUNDSSM_H
#define __BATTLEGROUNDSSM_H

#include "Battleground.h"
#include "BattlegroundScore.h"

enum BgSSMMineCarts
{
    BG_SSM_MINE_CART_1 = 1,
    BG_SSM_MINE_CART_2 = 2,
    BG_SSM_MINE_CART_3 = 3,

    BG_SSM_MINE_CARTS_MAX
};

enum BgSSMProgressBarConsts
{
    BG_SSM_PROGRESS_BAR_DONT_SHOW           = 0,
    BG_SSM_PROGRESS_BAR_SHOW                = 1,
    BG_SSM_PROGRESS_BAR_NEUTRAL             = 50,
    BG_SSM_PROGRESS_BAR_PERCENT_GREY        = 0,
    BG_SSM_PROGRESS_BAR_HORDE_CONTROLLED    = 0,
    BG_SSM_PROGRESS_BAR_ALI_CONTROLLED      = 100
};

enum BgSSMSpells
{
    BG_SSM_CONTROL_VISUAL_ALLIANCE                      = 116086,
    BG_SSM_CONTROL_VISUAL_HORDE                         = 116085,
    BG_SSM_CONTROL_VISUAL_NEUTRAL                       = 118001,

    BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_SOUTH  = 125696,
    BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_NORTH  = 125695,
    BG_SSM_SPELL_CART_CONTROL_CAPTURE_POINT_UNIT_EAST   = 125620,
    BG_SSM_SPELL_DEFENDING_CART_AURA                    = 128646,

    BG_SSM_TRACK_SWITCH_OPENED                          = 120228,
    BG_SSM_TRACK_SWITCH_CLOSED                          = 120229,
    BG_SSM_FEIGN_DEATH_STUN                             = 135781,
    BG_SSM_PREVENTION_AURA                              = 135846,
    BG_SSM_CART_CAP                                     = 115904,
};

enum BgSSMObjectEntry
{
    OBJECT_BG_SSM_RESERVOIR                 = 212080,
    OBJECT_BG_SSM_THE_DESPOSITION_OF_LAVA   = 212081,
    OBJECT_BG_SSM_THE_DESPOSITS_OF_DIAMONDS = 212082,
    OBJECT_BG_SSM_BACKLOG_TROLLS            = 212083,

    OBJECT_BG_SSM_DOOR1                     = 212939,
    OBJECT_BG_SSM_DOOR2                     = 212940,
    OBJECT_BG_SSM_DOOR3                     = 212941,
    OBJECT_BG_SSM_DOOR4                     = 212942,
};

enum BgSSMGaveyards
{
    SSM_GRAVEYARD_MAIN_ALLIANCE     = 4062,
    SSM_GRAVEYARD_MAIN_HORDE        = 4061,
};

enum BgSSMBCreaturesTypes
{
    SSM_SPIRIT_ALLIANCE        = 0,
    SSM_SPIRIT_HORDE           = 1,
    SSM_MINE_CART_TRIGGER      = 2,
    SSM_TRACK_SWITCH_EAST      = 3,
    SSM_TRACK_SWITCH_NORTH     = 4,

    BG_SSM_CREATURES_MAX
};

enum BgSSMDepots
{
    SSM_DIAMOND_DEPOT,
    SSM_WATERFALL_DEPOT,
    SSM_LAVA_DEPOT,
    SSM_TROLL_DEPOT,
};

enum BgSSMTracks
{
    SSM_EAST_TRACK_SWITCH,
    SSM_NORTH_TRACK_SWITCH,
    SSM_MAX_TRACK_SWITCH
};

enum BgSSMObjectTypes
{
    BG_SSM_OBJECT_DOOR_A_1                   = 0,
    BG_SSM_OBJECT_DOOR_H_1                   = 1,
    BG_SSM_OBJECT_DOOR_A_2                   = 2,
    BG_SSM_OBJECT_DOOR_H_2                   = 3,
    BG_SSM_OBJECT_WATERFALL_DEPOT            = 4,
    BG_SSM_OBJECT_LAVA_DEPOT                 = 5,
    BG_SSM_OBJECT_DIAMOND_DEPOT              = 6,
    BG_SSM_OBJECT_TROLL_DEPOT                = 7,
    BG_SSM_OBJECT_BERSERKING_BUFF_EAST       = 8,
    BG_SSM_OBJECT_BERSERKING_BUFF_WEST       = 9,
    BG_SSM_OBJECT_RESTORATION_BUFF_WATERFALL = 10,
    BG_SSM_OBJECT_RESTORATION_BUFF_LAVA      = 11,

    BG_SSM_OBJECT_MAX                        = 12
};

enum BgSSMScore
{
    SSM_MAX_TEAM_POINTS                 = 1500
};

enum BgSSMMineCartState
{
    SSM_MINE_CART_CONTROL_NEUTRAL       = 0,
    SSM_MINE_CART_CONTROL_ALLIANCE      = 1,
    SSM_MINE_CART_CONTROL_HORDE         = 2,
};

enum BgSSMCreatureIds
{
    NPC_TRACK_SWITCH            = 60283,
    NPC_MINE_CART               = 60140,
};

enum BgSSMPaths
{
    SSM_EAST_PATH,
    SSM_NORTH_PATH
};

float const BgSSMBuffPos[4][4] =
{
    {749.444153f, 64.338188f, 369.535797f, 6.058259f},   // Berserking buff East
    {789.979431f, 281.883575f, 355.389984f, 0.652173f},  // Berserking buff West
    {539.873596f, 396.386749f, 345.722412f, 3.994188f},  // Restoration buff Waterfall
    {614.202698f, 120.924660f, 294.430908f, 4.241807f}   // Restoration buff Lava
};

float const BgSSMDepotPos[][8] =
{
    {896.7936f, 25.29027f, 364.1431f, 3.539994f, -0.009816647f, 0.03659821f, -0.9795389f, 0.1976555f},  // OBJECT_BG_SSM_THE_DESPOSITS_OF_DIAMONDS
    {564.8048f, 337.0873f, 347.1427f, 1.571254f, 0.0112772f, 0.006951332f, 0.7072344f, 0.7068551f},     // OBJECT_BG_SSM_RESERVOIR
    {615.5099f, 79.41812f, 298.2867f, 1.654058f, -0.03407955f, -0.02646542f, 0.7354441f, 0.6762102f},   // OBJECT_BG_SSM_THE_DESPOSITION_OF_LAVA
    {777.7377f, 502.3111f, 359.4537f, 0.719448f, -0.01845312f, -0.01729774f, 0.3516912f, 0.9357743f}    // OBJECT_BG_SSM_BACKLOG_TROLLS
};

float const BgSSMDoorPos[4][4] =
{
    {852.02f, 158.21f, 328.76f, 0.15f}, // Alliance 1
    {830.09f, 143.92f, 328.50f, 3.13f}, // Alliance 2
    {652.17f, 228.49f, 328.91f, 0.16f}, // Horde 1
    {636.50f, 208.12f, 328.64f, 3.51f}  // Horde 2
};

float const BgSSMTrackPos[2][4] =
{
    {715.585388f, 101.272034f, 319.994690f, 4.647377f}, // East
    {847.481689f, 308.032562f, 346.573242f, 0.587086f}  // North
};

struct BrawlCarSelector
{
    uint8 GetCurrentCar()
    {
        uint8 result = cars[i];
        if (++i >= 3)
            i = 0;
        return result;
    }

    uint8 i = 0;
    std::vector<uint8> cars{ BG_SSM_MINE_CART_1 , BG_SSM_MINE_CART_3, BG_SSM_MINE_CART_2 };
};

static uint32 const BroadcastCartSpawn = 60444;
static uint32 const SoundKitCartSpawn = 9431;
static uint32 const MaxCarts = 3;
static uint32 const MaxPaths = 2;
static uint32 const PointsPerMineCart = 150;

struct BattleGroundSSMScore final : BattlegroundScore
{
    friend class BattlegroundSilvershardMines;

protected:
    BattleGroundSSMScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), CartsTaken(0) { }

    void UpdateScore(uint32 type, uint32 value) override;

    void BuildObjectivesBlock(std::vector<int32>& stats) override;

    uint32 CartsTaken;
};

class BattlegroundSilvershardMines : public Battleground
{
public:
    BattlegroundSilvershardMines();
    ~BattlegroundSilvershardMines();

    void AddPlayer(Player* player) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    bool SetupBattleground() override;
    void Reset() override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;
    void PostUpdateImpl(uint32 diff) override;
    void EndBattleground(uint32 winner) override;
    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void EventPlayerClickedOnFlag(Player* player, Unit* target) override;

    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return SSM_MAX_TEAM_POINTS; }
    bool IsScoreIncremental() const override { return true; }

private:
    void EventReopenDepot(uint32 diff);
    void EventTeamCapturedMineCart(uint8 mineCart);
    void SummonMineCart(uint32 diff);
    void CheckPlayerNearMineCart(uint32 diff);
    void CheckMineCartNearDepot(uint32 diff);
    void MineCartAddPoints(uint32 diff);
    void ResetDepotsAndMineCarts(uint8 depot, uint8 mineCart);
    void CheckTrackSwitch(uint32 diff);
    void AddPoints(TeamId teamId, uint32 Points);
    TeamId GetMineCartTeamKeeper(uint8 mineCart);
    Creature* GetMineCart(uint8 cart);

    std::unordered_map<uint32, ObjectGuid> _cartsMap;
    int32 _depotCloseTimer[4];
    uint32 _honorScoreTics[2];
    int32 _mineCartsProgressBar[MaxCarts];
    int32 _trackSwitchClickTimer[SSM_MAX_TRACK_SWITCH];
    uint32 _honorTics;
    int32 _mineCartSpawnTimer;
    int32 _mineCartCheckTimer;
    int32 _mineCartAddPointsTimer;
    bool _pathDone[MaxCarts - 1][MaxPaths];                 // Only for first and third mine cart
    bool _depot[4];                                         // 0 = Waterfall, 1 = Lava, 2 = Diamond, 3 = Troll
    bool _mineCartReachedDepot[MaxCarts];
    bool _mineCartNearDepot[MaxCarts];
    bool _waterfallPathDone;
    bool _trackSwitch[SSM_MAX_TRACK_SWITCH];                // East : true = open, false = close | North : true = close, false = open
    bool _trackSwitchCanInterract[SSM_MAX_TRACK_SWITCH];
    BrawlCarSelector m_brawlCarSelector;
};

#endif
