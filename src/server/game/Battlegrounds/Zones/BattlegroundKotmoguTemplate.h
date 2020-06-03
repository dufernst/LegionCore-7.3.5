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

#ifndef __BattleGroundKT_H
#define __BattleGroundKT_H

#include "Battleground.h"
#include "BattlegroundScore.h"


#define BG_KT_MAX_TEAM_SCORE        1500
#define BG_KT_OBJECTIVE_ORB_COUNT   419

enum BG_KT_Objects
{
    BG_KT_OBJECT_A_DOOR         = 0,
    BG_KT_OBJECT_H_DOOR         = 1,

    BG_KT_OBJECT_ORB_GREEN      = 2,
    BG_KT_OBJECT_ORB_PURPLE     = 3,
    BG_KT_OBJECT_ORB_ORANGE     = 4,
    BG_KT_OBJECT_ORB_BLUE       = 5,

    BG_KT_OBJECT_BERSERKBUFF_1  = 6,
    BG_KT_OBJECT_BERSERKBUFF_2  = 7,
    
    BG_KT_OBJECT_MAX            = 8
};

enum BG_KT_Creatures
{
    BG_KT_CREATURE_ORB_AURA_1   = 0,
    BG_KT_CREATURE_ORB_AURA_2   = 1,
    BG_KT_CREATURE_ORB_AURA_3   = 2,
    BG_KT_CREATURE_ORB_AURA_4   = 3,
    
    BG_KT_CREATURE_SPIRIT_1     = 4,
    BG_KT_CREATURE_SPIRIT_2     = 5,

    BG_KT_CREATURE_MAX          = 6
};

enum BG_KT_Objets_Entry
{
    BG_KT_OBJECT_DOOR_ENTRY     = 213172,

    BG_KT_OBJECT_ORB_1_ENTRY    = 212091,
    BG_KT_OBJECT_ORB_2_ENTRY    = 212092,
    BG_KT_OBJECT_ORB_3_ENTRY    = 212093,
    BG_KT_OBJECT_ORB_4_ENTRY    = 212094
};

enum BG_KT_SpellId
{
    BG_KT_SPELL_ORB_PICKED_UP_1 = 121176,   // GREEN
    BG_KT_SPELL_ORB_PICKED_UP_2 = 121175,   // PURPLE
    BG_KT_SPELL_ORB_PICKED_UP_3 = 121177,   // ORANGE
    BG_KT_SPELL_ORB_PICKED_UP_4 = 121164,   // BLUE

    BG_KT_SPELL_ORB_AURA_1      = 121220,   // GREEN
    BG_KT_SPELL_ORB_AURA_2      = 121219,   // PURPLE
    BG_KT_SPELL_ORB_AURA_3      = 121221,   // ORANGE
    BG_KT_SPELL_ORB_AURA_4      = 121217,   // BLUE

    BG_KT_ALLIANCE_INSIGNIA     = 131527,
    BG_KT_HORDE_INSIGNIA        = 131528,

    BG_KT_SPELL_HOTMOGU         = 237738,
    BG_KT_SPELL_BRAWL_OVERRIDE  = 229978,
};

enum BG_KT_Graveyards
{
    KT_GRAVEYARD_RECTANGLEA1    = 3552,
    KT_GRAVEYARD_RECTANGLEA2    = 4058,
    KT_GRAVEYARD_RECTANGLEH1    = 3553,
    KT_GRAVEYARD_RECTANGLEH2    = 4057
};

enum BG_KT_ZONE
{
    KT_ZONE_OUT                 = 0,
    KT_ZONE_IN                  = 1,
    KT_ZONE_MIDDLE              = 2,
    KT_ZONE_MAX                 = 3
};

struct BattleGroundKTScore final : BattlegroundScore
{
    friend class BattlegroundKotmoguTemplate;

protected:
    BattleGroundKTScore(ObjectGuid playerGuid, TeamId team) : BattlegroundScore(playerGuid, team), OrbHandles(0), Score(0) { }

    void UpdateScore(uint32 type, uint32 value) override
    {
        switch (type)
        {
            case SCORE_ORB_HANDLES:
                OrbHandles += value;
                break;
            case SCORE_ORB_SCORE:
                Score += value;
                break;
            default:
                BattlegroundScore::UpdateScore(type, value);
                break;
        }
    }

    void BuildObjectivesBlock(std::vector<int32>& stats) override
    {
        stats.push_back(OrbHandles);
        stats.push_back(Score);
    }

    uint32 OrbHandles;
    uint32 Score;
};

enum BG_TK_Events
{
    KT_EVENT_ORB                  = 0,
    // spiritguides will spawn (same moment, like TP_EVENT_DOOR_OPEN)
    KT_EVENT_SPIRITGUIDES_SPAWN   = 2
};

#define MAX_ORBS                    4

const float BG_KT_DoorPositions[MAX_TEAMS][4] =
{
    {1783.84f, 1100.66f, 20.60f, 1.625020f},
    {1780.15f, 1570.22f, 24.59f, 4.711630f}
};

const float BG_KT_OrbPositions[MAX_ORBS][4] =
{
    {1716.78f, 1416.64f, 13.5709f, 1.57239f},
    {1850.26f, 1416.77f, 13.5709f, 1.56061f},
    {1850.29f, 1250.31f, 13.5708f, 4.70848f},
    {1716.83f, 1249.93f, 13.5706f, 4.71397f}
};

Position const BG_KT_SpiritPositions[MAX_ORBS] =
{
    {1892.61f, 1151.69f, 14.7160f, 2.523528f},
    {1672.40f, 1524.10f, 16.7387f, 6.032206f},
};

uint32 const BG_KT_ORBS_SPELLS[MAX_ORBS] =
{
    BG_KT_SPELL_ORB_PICKED_UP_1,
    BG_KT_SPELL_ORB_PICKED_UP_2,
    BG_KT_SPELL_ORB_PICKED_UP_3,
    BG_KT_SPELL_ORB_PICKED_UP_4
};

uint32 const BG_KT_ORBS_AURA[MAX_ORBS] =
{
    BG_KT_SPELL_ORB_AURA_1,
    BG_KT_SPELL_ORB_AURA_2,
    BG_KT_SPELL_ORB_AURA_3,
    BG_KT_SPELL_ORB_AURA_4
};

const uint16 BgKtBroadCastTextOrbPickedUp[MAX_ORBS] = { 62281, 62282, 62280, 62279 };
const uint16 BgKtBroadCastTextOrbDropped[MAX_ORBS] = { 62286, 62284, 62285, 62283 };

WorldStates const OrbsWS[MAX_ORBS][MAX_TEAMS] =
{
    {WorldStates::BG_KT_PURPLE_ORB_C,   WorldStates::BG_KT_PURPLE_ORB_X},
    {WorldStates::BG_KT_GREEN_ORB_C,    WorldStates::BG_KT_GREEN_ORB_X},
    {WorldStates::BG_KT_ORANGE_ORB_C,   WorldStates::BG_KT_ORANGE_ORB_X},
    {WorldStates::BG_KT_BLUE_ORB_C,     WorldStates::BG_KT_BLUE_ORB_X}
};

WorldStates const OrbsIcons[MAX_ORBS] =
{
    WorldStates::BG_KT_ICON_GREEN_ORB_ICON,
    WorldStates::BG_KT_ICON_PURPLE_ORB_ICON,
    WorldStates::BG_KT_ICON_ORANGE_ORB_ICON,
    WorldStates::BG_KT_ICON_BLUE_ORB_ICON
};

uint32 const BG_KT_TickPoints[3] = { 3, 4, 5 };

class BattlegroundKotmoguTemplate : public Battleground
{
    friend class BattleGroundMgr;

public:
    BattlegroundKotmoguTemplate();
    ~BattlegroundKotmoguTemplate();

    void PostUpdateImpl(uint32 diff) override;

    void AddPlayer(Player* plr) override;
    void StartingEventCloseDoors() override;
    void StartingEventOpenDoors() override;
    void GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const override;

    void EventPlayerDroppedFlag(Player* source) override;

    void EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove) override;

    void RemovePlayer(Player* player, ObjectGuid guid, uint32) override;
    void _CheckPositions(uint32 diff) override;
    void HandleKillPlayer(Player* player, Player* killer) override;
    bool SetupBattleground() override;
    void Reset() override;
    void EndBattleground(uint32 winner) override;
    WorldSafeLocsEntry const* GetClosestGraveYard(Player* player) override;

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;

    ObjectGuid GetFlagPickerGUID(int32 index) const;
    void SetFlagPickerGUID(ObjectGuid guid, int32 index);

    uint32 GetTeamScore(TeamId teamId) const override { return m_TeamScores[teamId]; }
    uint32 GetMaxScore() const override { return BG_KT_MAX_TEAM_SCORE; }
    bool IsScoreIncremental() const override { return true; }
    void HandlePlayerResurrect(Player* /*player*/) override;
private:
    uint8 CheckOrbKeepersPosition(ObjectGuid guid);

    std::map<ObjectGuid, uint8> _playersZone;
    ObjectGuid _orbKeepers[MAX_ORBS];
    uint32 _reputationCapture;
    uint32 _updatePointsTimer;
    TeamId _lastCapturedOrbTeam;
};

#endif
