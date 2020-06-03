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

#include "BattlegroundWarsongGulch.h"
#include "GameObject.h"
#include "Object.h"
#include "BattlegroundMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldStatePackets.h"
#include "ObjectMgr.h"
#include "AreaTrigger.h"
#include "SpellScript.h"
#include "ScriptMgr.h"

void BattlegroundWGScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
        case SCORE_FLAG_CAPTURES:
            FlagCaptures += value;
            break;
        case SCORE_FLAG_RETURNS:
            FlagReturns += value;
            break;
        default:
            BattlegroundScore::UpdateScore(type, value);
            break;
    }
}

void BattlegroundWGScore::BuildObjectivesBlock(std::vector<int32>& stats)
{
    stats.push_back(FlagCaptures);
    stats.push_back(FlagReturns);
}

BattlegroundWarsongGulch::BattlegroundWarsongGulch() : _flagPosTimer(0), _flagsTimer(0), _flagState{}
{
    BgObjects.resize(BG_WS_BRAWL_OBJECT_MAX);
    BgCreatures.resize(BG_CREATURES_MAX_WS);
    m_brawlAreatriggers.resize(11);

    _flagSpellForceTimer = 0;
    _bothFlagsKept = false;
    _flagDebuffState = 0;

    _reputationCapture = 0;

    for (uint8 i = TEAM_ALLIANCE; i <= TEAM_HORDE;++i)
        for (uint8 j = 0; j < 3; ++j)
        {
            _flagKeepers[i][j] = ObjectGuid::Empty;
            _droppedFlagGUID[i][j] = ObjectGuid::Empty;
        }
}

BattlegroundWarsongGulch::~BattlegroundWarsongGulch()
{
    for (uint8 i = 0; i < 11; ++i)
        if (!m_brawlAreatriggers[i].first.IsEmpty())
            if (auto at = GetBgMap()->GetAreaTrigger(m_brawlAreatriggers[i].first))
                at->Remove(false);
};

void BattlegroundWarsongGulch::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;
    if (GetElapsedTime() >= Minutes(17))
        Battleground::BattlegroundTimedWin(2);

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            switch (_flagState[team][i])
            {
            case BG_WS_FLAG_STATE_WAIT_RESPAWN:
                _flagsTimer -= diff;
                if (_flagsTimer <= 0)
                {
                    _flagsTimer = 0;
                    RespawnFlag(static_cast<TeamId>(team), true, i);
                    UpdateWorldState(WorldStates::BG_WS_FLAG_UNKNOWN, 0);
                }
                break;
            case BG_WS_FLAG_STATE_ON_GROUND:
                _flagsDropTimer[team][i] -= diff;
                if (_flagsDropTimer[team][i] < 0)
                {
                    _flagsDropTimer[team][i] = 0;

                    RespawnFlag(static_cast<TeamId>(team), false, i);

                    if (GameObject* obj = GetBgMap()->GetGameObject(_droppedFlagGUID[team][i]))
                        obj->Delete();

                    _droppedFlagGUID[team][i].Clear();
                    if (_bothFlagsKept)
                    {
                        bool allFlagsStand = true;
                        for (uint8 j = 0; j < 3; ++j)
                            if (_flagState[team][j] == BG_WS_FLAG_STATE_ON_PLAYER || _flagState[team ^ 1][j] == BG_WS_FLAG_STATE_ON_GROUND)
                                allFlagsStand = false;

                        if (allFlagsStand)
                        {
                            _bothFlagsKept = false;
                            _flagDebuffState = 0;
                            _flagSpellForceTimer = 0;
                        }
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    if (_bothFlagsKept)
    {
        _flagSpellForceTimer += diff;

        if (_flagSpellForceTimer >= 30 * IN_MILLISECONDS && _flagDebuffState < 10)
        {
            _flagDebuffState++;
            _flagSpellForceTimer = 0;

            if (_flagDebuffState <= 5)
            {
                for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                    for (uint8 j = 0; j < 3; ++j)
                        if (Player* player = ObjectAccessor::FindPlayer(_flagKeepers[team][j]))
                            player->CastSpell(player, SPELL_BG_FOCUSED_ASSAULT, true);
            }
            else if (_flagDebuffState == 6)
            {
                for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                {
                    for (uint8 j = 0; j < 3; ++j)
                        if (Player* player = ObjectAccessor::FindPlayer(_flagKeepers[team][j]))
                        {
                            player->RemoveAurasDueToSpell(SPELL_BG_FOCUSED_ASSAULT);
                            player->CastCustomSpell(SPELL_BG_BRUTAL_ASSAULT, SPELLVALUE_AURA_STACK, _flagDebuffState, player);
                        }
                }
            }
            else
                for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                    for (uint8 j = 0; j < 3; ++j)
                        if (Player* player = ObjectAccessor::FindPlayer(_flagKeepers[team][j]))
                            player->CastSpell(player, SPELL_BG_BRUTAL_ASSAULT, true);
        }
    }

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
    {
        for (uint8 i = 0; i < 11; ++i)
            if (m_brawlAreatriggers[i].first.IsEmpty())
            {
                if (m_brawlAreatriggers[i].second <= diff)
                {
                    AreaTrigger* areaTrigger = new AreaTrigger;
                    if (areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, nullptr, NULL, brawlObjects[i + 8].second, brawlObjects[i + 8].second, NULL, ObjectGuid::Empty, 15012, ObjectGuid::Empty, GetBgMap()))
                        m_brawlAreatriggers[i] = { areaTrigger->GetGUID(), 0 };
                    else
                        delete areaTrigger;
                }
                else
                    m_brawlAreatriggers[i].second -= diff;
            }
    }
}

void BattlegroundWarsongGulch::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundWGScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble ? BG_WS_MAX_TEAM_SCORE_BRAWL : BG_WS_MAX_TEAM_SCORE).Write());
    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);
    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
        player->CastSpell(player, BG_WS_SCRAMBLE_SPELL, true);
}

void BattlegroundWarsongGulch::GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const
{
    for (uint8 i = TEAM_ALLIANCE; i <= TEAM_HORDE; ++i)
        for (uint8 j = 0; j < 3; ++j)
            if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), _flagKeepers[i][j]))
            {
                WorldPackets::Battleground::BattlegroundPlayerPosition position;
                position.Guid = player->GetGUID();
                position.Pos = player->GetPosition();
                position.IconID = i == TEAM_ALLIANCE ? PLAYER_POSITION_ICON_ALLIANCE_FLAG : PLAYER_POSITION_ICON_HORDE_FLAG;
                position.ArenaSlot = PLAYER_POSITION_ARENA_SLOT_NONE;
                positions->push_back(position);
            }
}

void BattlegroundWarsongGulch::StartingEventCloseDoors()
{
    for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_H_3; ++i)
    {
        DoorClose(i);
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
    }

    for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
        for (uint32 i = BG_WS_BRAWL_A_FLAG_2; i <= BG_WS_BRAWL_H_FLAG_3_POST; ++i)
            SpawnBGObject(i, RESPAWN_ONE_DAY);
}

void BattlegroundWarsongGulch::StartingEventOpenDoors()
{
    for (uint32 i = BG_WS_OBJECT_DOOR_A_1; i <= BG_WS_OBJECT_DOOR_A_6; ++i)
        DoorOpen(i);
    for (uint32 i = BG_WS_OBJECT_DOOR_H_1; i <= BG_WS_OBJECT_DOOR_H_4; ++i)
        DoorOpen(i);

    for (uint32 i = BG_WS_OBJECT_A_FLAG; i <= BG_WS_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
        for (uint32 i = BG_WS_BRAWL_A_FLAG_2; i <= BG_WS_BRAWL_H_FLAG_3_POST; ++i)
            SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    SpawnBGObject(BG_WS_OBJECT_DOOR_A_5, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_A_6, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_H_3, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_WS_OBJECT_DOOR_H_4, RESPAWN_ONE_DAY);

    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, BG_EVENT_START_BATTLE);
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, BG_EVENT_START_BATTLE);

    UpdateWorldState(WorldStates::BG_WS_ENABLE_TIMER, 1);
    UpdateWorldState(WorldStates::BG_WS_CURRENT_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(15)).count()));
}

bool BattlegroundWarsongGulch::SetupBattleground()
{
    if (!AddObject(BG_WS_OBJECT_SPEEDBUFF_1, BG_OBJECTID_SPEEDBUFF_ENTRY, 1449.93f, 1470.71f, 342.6346f, -1.64061f, 0, 0, 0.7313537f, -0.6819983f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_SPEEDBUFF_2, BG_OBJECTID_SPEEDBUFF_ENTRY, 1005.171f, 1447.946f, 335.9032f, 1.64061f, 0, 0, 0.7313537f, 0.6819984f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_REGENBUFF_1, BG_OBJECTID_REGENBUFF_ENTRY, 1317.506f, 1550.851f, 313.2344f, -0.2617996f, 0, 0, 0.1305263f, -0.9914448f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_REGENBUFF_2, BG_OBJECTID_REGENBUFF_ENTRY, 1110.451f, 1353.656f, 316.5181f, -0.6806787f, 0, 0, 0.333807f, -0.9426414f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_BERSERKBUFF_1, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1320.09f, 1378.79f, 314.7532f, 1.186824f, 0, 0, 0.5591929f, 0.8290376f, BUFF_RESPAWN_TIME)
        || !AddObject(BG_WS_OBJECT_BERSERKBUFF_2, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1139.688f, 1560.288f, 306.8432f, -2.443461f, 0, 0, 0.9396926f, -0.3420201f, BUFF_RESPAWN_TIME)

        || !AddObject(BG_WS_OBJECT_DOOR_A_1, BG_OBJECT_DOOR_A_1_WS_ENTRY, 1503.335f, 1493.466f, 352.1888f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_2, BG_OBJECT_DOOR_A_2_WS_ENTRY, 1492.478f, 1457.912f, 342.9689f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_3, BG_OBJECT_DOOR_A_3_WS_ENTRY, 1468.503f, 1494.357f, 351.8618f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_4, BG_OBJECT_DOOR_A_4_WS_ENTRY, 1471.555f, 1458.778f, 362.6332f, 3.115414f, 0, 0, 0.9999143f, 0.01308903f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_5, BG_OBJECT_DOOR_A_5_WS_ENTRY, 1492.347f, 1458.34f, 342.3712f, -0.03490669f, 0, 0, 0.01745246f, -0.9998477f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_A_6, BG_OBJECT_DOOR_A_6_WS_ENTRY, 1503.466f, 1493.367f, 351.7352f, -0.03490669f, 0, 0, 0.01745246f, -0.9998477f, RESPAWN_IMMEDIATELY)

        || !AddObject(BG_WS_OBJECT_DOOR_H_1, BG_OBJECT_DOOR_H_1_WS_ENTRY, 949.1663f, 1423.772f, 345.6241f, -0.5756807f, -0.01673368f, -0.004956111f, -0.2839723f, 0.9586737f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_2, BG_OBJECT_DOOR_H_2_WS_ENTRY, 953.0507f, 1459.842f, 340.6526f, -1.99662f, -0.1971825f, 0.1575096f, -0.8239487f, 0.5073641f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_3, BG_OBJECT_DOOR_H_3_WS_ENTRY, 949.9523f, 1422.751f, 344.9273f, 0.0f, 0, 0, 0, 1, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_WS_OBJECT_DOOR_H_4, BG_OBJECT_DOOR_H_4_WS_ENTRY, 950.7952f, 1459.583f, 342.1523f, 0.05235988f, 0, 0, 0.02617695f, 0.9996573f, RESPAWN_IMMEDIATELY)
        )
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
        return false;
    }

    if (!IsBrawl())
    {
        if (!AddObject(BG_WS_OBJECT_A_FLAG, BG_OBJECT_A_FLAG_WS_ENTRY, 1540.423f, 1481.325f, 351.8284f, 3.089233f, 0.0f, 0.0f, 0.9996573f, 0.02617699f, BG_WS_FLAG_RESPAWN_TIME / 1000) ||
            !AddObject(BG_WS_OBJECT_H_FLAG, BG_OBJECT_H_FLAG_WS_ENTRY, 916.0226f, 1434.405f, 345.413f, 0.01745329f, 0.0f, 0.0f, 0.008726535f, 0.9999619f, BG_WS_FLAG_RESPAWN_TIME / 1000))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
            return false;
        }
    }
    else if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
    {
        if (!AddObject(BG_WS_OBJECT_A_FLAG, BG_OBJECT_A_FLAG_WS_ENTRY, 1357.948f, 1462.004f, 325.2907f, 3.064396f, 0.0f, 0.0f, 0.9996573f, 0.02617699f, BG_WS_FLAG_RESPAWN_TIME / 1000) ||
            !AddObject(BG_WS_OBJECT_H_FLAG, BG_OBJECT_H_FLAG_WS_ENTRY, 1119.123f, 1462.448f, 316.926f, 0.01745246f, 0.0f, 0.0f, 0.008726535f, 0.9999619f, BG_WS_FLAG_RESPAWN_TIME / 1000))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
            return false;
        }
    }
    else if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
    {
        if (!AddObject(BG_WS_OBJECT_A_FLAG, BG_OBJECT_A_FLAG_WS_ENTRY, 1540.423f, 1481.325f, 351.8284f, 3.089233f, 0.0f, 0.0f, 0.9996573f, 0.02617699f, BG_WS_FLAG_RESPAWN_TIME / 1000) ||
            !AddObject(BG_WS_OBJECT_H_FLAG, BG_OBJECT_H_FLAG_WS_ENTRY, 916.0226f, 1434.405f, 345.413f, 0.01745329f, 0.0f, 0.0f, 0.008726535f, 0.9999619f, BG_WS_FLAG_RESPAWN_TIME / 1000))
        {
            TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
            return false;
        }

        uint8 i = BG_WS_BRAWL_A_FLAG_2;
        for (auto& pair : brawlObjects)
        {
            if (!AddObject(i, pair.first, pair.second, {}, BG_WS_FLAG_RESPAWN_TIME / 1000))
            {
                TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn some object Battleground not created!");
                return false;
            }
            else if (i - BG_WS_BRAWL_A_FLAG_2 >= 8)
            {
                AreaTrigger* areaTrigger = new AreaTrigger;
                if (areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), 0, nullptr, NULL, pair.second, pair.second, NULL, ObjectGuid::Empty, 15012, ObjectGuid::Empty, GetBgMap()))
                    m_brawlAreatriggers[i - BG_WS_BRAWL_A_FLAG_2 - 8] = { areaTrigger->GetGUID(), 0 };
                else
                    delete areaTrigger;
            }
            ++i;
        }
    }

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_ALLIANCE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_ALLIANCE, sg->Loc, TEAM_ALLIANCE))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn Alliance spirit guide! Battleground not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(WS_GRAVEYARD_MAIN_HORDE);
    if (!sg || !AddSpiritGuide(WS_SPIRIT_MAIN_HORDE, sg->Loc, TEAM_HORDE))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundWS: Failed to spawn Horde spirit guide! Battleground not created!");
        return false;
    }

    return true;
}

void BattlegroundWarsongGulch::Reset()
{
    Battleground::Reset();

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        for (uint8 j = 0; j < 3; ++j)
        {
            _flagKeepers[i][j].Clear();
            _droppedFlagGUID[i][j].Clear();
            _flagsDropTimer[i][j] = 0;
            _flagState[i][j] = BG_WS_FLAG_STATE_ON_BASE;
        }
        m_TeamScores[i] = 0;
    }

    _reputationCapture = 35;
    _flagsTimer = 0;
}

void BattlegroundWarsongGulch::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_CAPTURES_ALLIANCE, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_CAPTURES_HORDE, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_CAPTURES_MAX, GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble ? BG_WS_MAX_TEAM_SCORE_BRAWL: BG_WS_MAX_TEAM_SCORE);

    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        {
            switch (std::max(_flagState[team][0], std::max(_flagState[team][1], _flagState[team][2])))
            {
                case BG_WS_FLAG_STATE_ON_GROUND:
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_UNK_ALLIANCE : WorldStates::BG_WS_FLAG_UNK_HORDE, -1);
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_STATE_ALLIANCE : WorldStates::BG_WS_FLAG_STATE_HORDE, BG_WS_FLAG_STATE_ON_GROUND);
                    break;
                case BG_WS_FLAG_STATE_ON_PLAYER:
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_UNK_ALLIANCE : WorldStates::BG_WS_FLAG_UNK_HORDE, 1);
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_STATE_ALLIANCE : WorldStates::BG_WS_FLAG_STATE_HORDE, BG_WS_FLAG_STATE_ON_PLAYER);
                    break;
                default:
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_UNK_ALLIANCE : WorldStates::BG_WS_FLAG_UNK_HORDE, 0);
                    packet.Worldstates.emplace_back(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_STATE_ALLIANCE : WorldStates::BG_WS_FLAG_STATE_HORDE, BG_WS_FLAG_STATE_WAIT_RESPAWN);
                    break;
            }
        }

        packet.Worldstates.emplace_back(WorldStates::BG_WS_ENABLE_TIMER, 1);
        packet.Worldstates.emplace_back(WorldStates::BG_WS_CURRENT_TIMER, int32(time(nullptr) + std::chrono::duration_cast<Seconds>(Minutes(15) - GetElapsedTime()).count()));
    }
    else
    {
        packet.Worldstates.emplace_back(WorldStates::BG_WS_ENABLE_TIMER, 0);
        packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_UNK_ALLIANCE, 0);
        packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_UNK_HORDE, 0);
        packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_STATE_HORDE, 1);
        packet.Worldstates.emplace_back(WorldStates::BG_WS_FLAG_STATE_ALLIANCE, 1);
    }
}

void BattlegroundWarsongGulch::EndBattleground(uint32 winner)
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (uint8 j = 0; j < 3; ++j)
            if (Player* player = ObjectAccessor::FindPlayer(_flagKeepers[team][j]))
            {
                player->RemoveAurasDueToSpell(SPELL_BG_FOCUSED_ASSAULT);
                player->RemoveAurasDueToSpell(SPELL_BG_BRUTAL_ASSAULT);
            }

    UpdateWorldState(WorldStates::BG_WS_FLAG_UNK_ALLIANCE, 0);
    UpdateWorldState(WorldStates::BG_WS_FLAG_UNK_HORDE, 0);
    UpdateWorldState(WorldStates::BG_WS_FLAG_STATE_ALLIANCE, 1);
    UpdateWorldState(WorldStates::BG_WS_FLAG_STATE_HORDE, 1);
    UpdateWorldState(WorldStates::BG_WS_ENABLE_TIMER, 0);

    uint32 realWinner = WINNER_NONE;
    if (winner == TEAM_ALLIANCE)
        realWinner = ALLIANCE;
    else if (winner == TEAM_HORDE)
        realWinner = HORDE;
    else if (winner > WINNER_NONE)
        realWinner = winner;

    Battleground::EndBattleground(realWinner);
}

void BattlegroundWarsongGulch::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);

    Battleground::HandleKillPlayer(player, killer);
}

ObjectGuid BattlegroundWarsongGulch::GetFlagPickerGUID(int32 team, uint8 j) const
{
    if (team == TEAM_ALLIANCE || team == TEAM_HORDE)
        return _flagKeepers[team][j];

    return ObjectGuid::Empty;
}

void BattlegroundWarsongGulch::_CheckPositions(uint32 diff)
{
    for (auto const& itr : GetPlayers())
    {
        Player* player = ObjectAccessor::FindPlayer(itr.first);
        if (!player)
            continue;

        if (!IsBrawl())
        {
            if (player->IsInAreaTriggerRadius(3646) && _flagState[TEAM_HORDE][0] && !_flagState[TEAM_ALLIANCE][0] && GetStatus() == STATUS_IN_PROGRESS) // Alliance Flag spawn
            {
                if (_flagKeepers[TEAM_HORDE][0] == player->GetGUID())
                    EventPlayerCapturedFlag(player);
                break;
            }

            if (player->IsInAreaTriggerRadius(3647) && _flagState[TEAM_ALLIANCE][0] && !_flagState[TEAM_HORDE][0] && GetStatus() == STATUS_IN_PROGRESS) // Horde Flag spawn
            {
                if (_flagKeepers[TEAM_ALLIANCE][0] == player->GetGUID())
                    EventPlayerCapturedFlag(player);
                break;
            }
        }
        else if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlAllSix)
        {
            if (player->IsInRange2d(1357.948f, 1462.004f, 0.1f, 5.0f) && _flagState[TEAM_HORDE][0] && !_flagState[TEAM_ALLIANCE][0] && GetStatus() == STATUS_IN_PROGRESS) // Alliance Flag spawn
            {
                if (_flagKeepers[TEAM_HORDE][0] == player->GetGUID())
                    EventPlayerCapturedFlag(player);
                break;
            }

            if (player->IsInRange2d(1119.123f, 1462.448f, 0.1f, 5.0f) && _flagState[TEAM_ALLIANCE][0] && !_flagState[TEAM_HORDE][0] && GetStatus() == STATUS_IN_PROGRESS) // Horde Flag spawn
            {
                if (_flagKeepers[TEAM_ALLIANCE][0] == player->GetGUID())
                    EventPlayerCapturedFlag(player);
                break;
            }
        }
        else if (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble)
        {
            if (player->IsInAreaTriggerRadius(3646) && GetStatus() == STATUS_IN_PROGRESS) // Alliance Flag spawn
            {
                for (uint8 j = 0; j < 3; ++j)
                    if (_flagKeepers[TEAM_HORDE][j] == player->GetGUID())
                        EventPlayerCapturedFlag(player);
                break;
            }

            if (player->IsInAreaTriggerRadius(3647) && GetStatus() == STATUS_IN_PROGRESS) // Horde Flag spawn
            {
                for (uint8 j = 0; j < 3; ++j)
                    if (_flagKeepers[TEAM_ALLIANCE][j] == player->GetGUID())
                        EventPlayerCapturedFlag(player);
                break;
            }
        }
    }

    Battleground::_CheckPositions(diff);
}

bool BattlegroundWarsongGulch::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_FLAG_CAPTURES:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, WS_OBJECTIVE_CAPTURE_FLAG, 1);
            break;
        case SCORE_FLAG_RETURNS:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, WS_OBJECTIVE_RETURN_FLAG, 1);
            break;
        default:
            break;
    }
    return true;
}

void BattlegroundWarsongGulch::SetDroppedFlagGUID(ObjectGuid guid, uint32 TeamID, ObjectGuid playerguid)
{
    for (uint8 i = 0; i < 3; ++i)
    {
        if (_flagKeepers[TeamID][i] == playerguid)
        {
            _droppedFlagGUID[TeamID][i] = guid;
            _flagKeepers[TeamID][i] = ObjectGuid::Empty;
        }
    }

}

WorldSafeLocsEntry const* BattlegroundWarsongGulch::GetClosestGraveYard(Player* player)
{
    static uint32 const graveyardsInProgress[2] = { WS_GRAVEYARD_MAIN_ALLIANCE, WS_GRAVEYARD_MAIN_HORDE };
    static uint32 const graveyards[2] = { WS_GRAVEYARD_FLAGROOM_ALLIANCE, WS_GRAVEYARD_FLAGROOM_HORDE };

    return sWorldSafeLocsStore.LookupEntry(GetStatus() == STATUS_IN_PROGRESS ? graveyardsInProgress[player->GetBGTeamId()] : graveyards[player->GetBGTeamId()]);
}

void BattlegroundWarsongGulch::EventPlayerDroppedFlag(Player* Source)
{
    TeamId teamID = Source->GetBGTeamId();

    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        if (teamID == TEAM_ALLIANCE)
        {
            for (uint8 j = 0; j < 3; ++j)
            {
                if (!IsHordeFlagPickedup(j))
                    continue;;

                if (GetFlagPickerGUID(TEAM_HORDE, j) == Source->GetGUID())
                {
                    SetHordeFlagPicker(ObjectGuid::Empty, j);
                    Source->RemoveAurasDueToSpell(SPELL_BG_HORDE_FLAG);
                    break;
                }
            }
        }
        else
        {
            for (uint8 j = 0; j < 3; ++j)
            {
                if (!IsAllianceFlagPickedup(j))
                    continue;

                if (GetFlagPickerGUID(TEAM_ALLIANCE, j) == Source->GetGUID())
                {
                    SetAllianceFlagPicker(ObjectGuid::Empty, j);
                    Source->RemoveAurasDueToSpell(SPELL_BG_ALLIANCE_FLAG);
                    break;
                }
            }
        }
        return;
    }

    bool set = false;

    uint8 j = 0;
    if (teamID == TEAM_ALLIANCE)
    {
        for (j = 0; j < 3; ++j)
        {
            if (!IsHordeFlagPickedup(j))
                continue;

            if (GetFlagPickerGUID(TEAM_HORDE, j) == Source->GetGUID())
            {
                // SetHordeFlagPicker(ObjectGuid::Empty, j);
                Source->RemoveAurasDueToSpell(SPELL_BG_HORDE_FLAG);
                _flagState[TEAM_HORDE][j] = BG_WS_FLAG_STATE_ON_GROUND;
                Source->CastSpell(Source, SPELL_BG_HORDE_FLAG_DROPPED, true);
                set = true;
                break;
            }
        }
    }
    else
    {
        for (j = 0; j < 3; ++j)
        {
            if (!IsAllianceFlagPickedup(j))
                continue;

            if (GetFlagPickerGUID(TEAM_ALLIANCE, j) == Source->GetGUID())
            {
                // SetAllianceFlagPicker(ObjectGuid::Empty, j);
                Source->RemoveAurasDueToSpell(SPELL_BG_ALLIANCE_FLAG);
                _flagState[TEAM_ALLIANCE][j] = BG_WS_FLAG_STATE_ON_GROUND;
                Source->CastSpell(Source, SPELL_BG_ALLIANCE_FLAG_DROPPED, true);
                set = true;
                break;
            }
        }
    }

    if (set)
    {
        Source->CastSpell(Source, SPELL_BG_RECENTLY_DROPPED_FLAG, true);

        SendBroadcastText(teamID == TEAM_ALLIANCE ? 9806 : 9805, teamID == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, Source);
        UpdateWorldState(teamID == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_UNK_ALLIANCE : WorldStates::BG_WS_FLAG_UNK_HORDE, uint32(-1));

        _flagsDropTimer[Source->GetBGTeamId() ? 0 : 1][j] = BG_WS_FLAG_DROP_TIME;
    }
}

void BattlegroundWarsongGulch::EventPlayerClickedOnFlag(Player* source, GameObject* object, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    TeamId team = source->GetBGTeamId();

    if (source->IsWithinDistInMap(object, 10))
    {
        if (!source->HasAura(SPELL_BG_HORDE_FLAG) && !source->HasAura(SPELL_BG_ALLIANCE_FLAG))
        {
            uint32 obj[3]{ BG_WS_OBJECT_A_FLAG, BG_WS_BRAWL_A_FLAG_2, BG_WS_BRAWL_A_FLAG_3 };
            for (uint8 j = 0; j < 3; ++j)
            {
                switch (_flagState[team ^ 1][j])
                {
                case BG_WS_FLAG_STATE_ON_BASE:
                    if (BgObjects[obj[j] + (team ^ 1)] == object->GetGUID())
                    {
                        SpawnBGObject(obj[j] + (team ^ 1), RESPAWN_ONE_DAY);
                        UpdateFlagState(MS::Battlegrounds::GetOtherTeamID(team), BG_WS_FLAG_STATE_ON_PLAYER, source->GetGUID(), j);
                        source->CastSpell(source, team == TEAM_ALLIANCE ? SPELL_BG_HORDE_FLAG : SPELL_BG_ALLIANCE_FLAG, true);
                        source->GetAchievementMgr()->StartTimedAchievement(CRITERIA_TIMED_TYPE_SPELL_TARGET2, team == TEAM_ALLIANCE ? SPELL_BG_HORDE_FLAG_PICKED_UP : SPELL_BG_ALLIANCE_FLAG_PICKED_UP);

                        for (uint8 z = 0; z < 3; ++z)
                            if (!_flagKeepers[team][z].IsEmpty())
                                _bothFlagsKept = true;

                        SendBroadcastText(team == TEAM_ALLIANCE ? 9807 : 9804, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);
                        PlayeCapturePointSound(NODE_STATUS_ASSAULT, team);
                    }
                    break;
                case BG_WS_FLAG_STATE_ON_GROUND:
                    if (_droppedFlagGUID[team ^ 1][j] == object->GetGUID())
                    {
                        source->CastSpell(source, team == TEAM_ALLIANCE ? SPELL_BG_HORDE_FLAG : SPELL_BG_ALLIANCE_FLAG, true);
                        UpdateFlagState(MS::Battlegrounds::GetOtherTeamID(team), BG_WS_FLAG_STATE_ON_PLAYER, source->GetGUID(), j);

                        if (_flagDebuffState  && _flagDebuffState < 6)
                            source->CastCustomSpell(SPELL_BG_FOCUSED_ASSAULT, SPELLVALUE_AURA_STACK, _flagDebuffState, source);
                        else if (_flagDebuffState >= 6)
                            source->CastCustomSpell(SPELL_BG_BRUTAL_ASSAULT, SPELLVALUE_AURA_STACK, _flagDebuffState, source);

                        _flagsDropTimer[team ^ 1][j] = 0;

                        PlayeCapturePointSound(NODE_STATUS_ASSAULT, team);
                        SendBroadcastText(team == TEAM_ALLIANCE ? 9807 : 9804, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        else
            canRemove = false;

        for (uint8 j = 0; j < 3; ++j)
        {
            if (_flagState[team][j] == BG_WS_FLAG_STATE_ON_GROUND && _droppedFlagGUID[team][j] == object->GetGUID())
            {
                UpdateFlagState(team, BG_WS_FLAG_STATE_WAIT_RESPAWN, ObjectGuid::Empty, j);
                UpdatePlayerScore(source, SCORE_FLAG_RETURNS, 1);

                RespawnFlag(team, false, j);

                PlaySoundToAll(BG_SOUND_FLAG_RESET);
                SendBroadcastText(team == TEAM_ALLIANCE ? 9808 : 9809, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);

                _bothFlagsKept = false;
                _flagSpellForceTimer = 0;
                canRemove = true;
            }
        }
    }

    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
}

void BattlegroundWarsongGulch::EventPlayerCapturedFlag(Player* source)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    TeamId team = source->GetBGTeamId();

    for (uint8 j = 0; j < 3; ++j)
    {
        if (_flagKeepers[team ^ 1][j] != source->GetGUID())
            continue;

        source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
        _flagDebuffState = 0;

        uint32 flags[3]{ BG_WS_OBJECT_A_FLAG, BG_WS_BRAWL_A_FLAG_2, BG_WS_BRAWL_A_FLAG_3 };
        SpawnBGObject(flags[j] + (team^1), RESPAWN_ONE_DAY);


        source->RemoveAurasDueToSpell(SPELL_BG_FOCUSED_ASSAULT);
        source->RemoveAurasDueToSpell(SPELL_BG_BRUTAL_ASSAULT);

        RewardHonorToTeam(20, source->GetBGTeam());

        RewardReputationToTeam(890, 889, _reputationCapture, source->GetBGTeam());

        m_TeamScores[team]++;

        UpdateWorldState(team == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_CAPTURES_ALLIANCE : WorldStates::BG_WS_FLAG_CAPTURES_HORDE, m_TeamScores[team]);

        UpdateWorldState(WorldStates::BG_WS_FLAG_UNKNOWN, -1);
        UpdateFlagState(MS::Battlegrounds::GetOtherTeamID(team), BG_WS_FLAG_STATE_WAIT_RESPAWN, ObjectGuid::Empty, j);
        UpdatePlayerScore(source, SCORE_FLAG_CAPTURES, 1);
        UpdateWorldState(WorldStates::BG_WS_UNKNOWN, 1);

        m_lastFlagCaptureTeam = source->GetBGTeam();

        PlayeCapturePointSound(NODE_STATUS_CAPTURE, team);
        SendBroadcastText(team == TEAM_ALLIANCE ? 9801 : 9802, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, source);

        source->RemoveAurasDueToSpell(team == TEAM_ALLIANCE ? SPELL_BG_HORDE_FLAG : SPELL_BG_ALLIANCE_FLAG);

        Battleground::SendBattleGroundPoints(team != TEAM_ALLIANCE, m_TeamScores[team]);

        if (m_TeamScores[team] == (GetTypeID() == MS::Battlegrounds::BattlegroundTypeId::BrawlBattlegroundWarsongScramble ? BG_WS_MAX_TEAM_SCORE_BRAWL : BG_WS_MAX_TEAM_SCORE))
            EndBattleground(team);
        else
            _flagsTimer = BG_WS_FLAG_RESPAWN_TIME;
    }
}

void BattlegroundWarsongGulch::RemovePlayer(Player* player, ObjectGuid guid, uint32 /*team*/)
{
    if (!player)
        return;

    TeamId team = player->GetBGTeamId();
    for (uint8 j = 0; j < 3; ++j)
    {
        if (_flagKeepers[team ^ 1][j] == guid)
        {
            RespawnFlag(MS::Battlegrounds::GetOtherTeamID(team), false, j);
            player->RemoveAurasDueToSpell(team == TEAM_ALLIANCE ? SPELL_BG_HORDE_FLAG : SPELL_BG_ALLIANCE_FLAG);

            if (_bothFlagsKept)
            {
                for (uint8 z = 0; z < 3; ++z)
                    if (!_flagKeepers[team ^ 1][z].IsEmpty())
                        return;

                for (uint8 z = 0; z < 3; ++z)
                {
                    if (Player* pl = ObjectAccessor::FindPlayer(_flagKeepers[team][z]))
                    {
                        if (_flagDebuffState && _flagDebuffState < 6)
                            pl->RemoveAurasDueToSpell(SPELL_BG_FOCUSED_ASSAULT);
                        else if (_flagDebuffState >= 6)
                            pl->RemoveAurasDueToSpell(SPELL_BG_BRUTAL_ASSAULT);
                    }
                }

                _bothFlagsKept = false;
                _flagDebuffState = 0;
                _flagSpellForceTimer = 0;
            }
        }
        player->RemoveAurasDueToSpell(BG_WS_SCRAMBLE_SPELL);
    }
}

void BattlegroundWarsongGulch::RespawnFlag(TeamId teamID, bool captured, uint8 j)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (captured)
    {
        uint32 flags[3]{ BG_WS_OBJECT_A_FLAG, BG_WS_BRAWL_A_FLAG_2, BG_WS_BRAWL_A_FLAG_3 };
        SpawnBGObject(flags[j] + teamID, RESPAWN_IMMEDIATELY);
        SendBroadcastText(9803, CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
    else
    {
        uint32 flags[3]{ BG_WS_OBJECT_A_FLAG, BG_WS_BRAWL_A_FLAG_2, BG_WS_BRAWL_A_FLAG_3 };
        SpawnBGObject(flags[j] + teamID, RESPAWN_IMMEDIATELY);
        SendBroadcastText(teamID == TEAM_ALLIANCE ? 24891 : 24892, CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }

    PlayeCapturePointSound(NODE_STATUS_NEUTRAL, teamID);

    UpdateFlagState(teamID, BG_WS_FLAG_STATE_ON_BASE, ObjectGuid::Empty, j);
}

void BattlegroundWarsongGulch::DoAction(uint32, ObjectGuid atGuid)
{
    for (uint8 i = 0; i < 11; ++i)
        if (m_brawlAreatriggers[i].first == atGuid)
        {
            m_brawlAreatriggers[i].first = ObjectGuid::Empty;
            m_brawlAreatriggers[i].second = 25000;
        }
}

void BattlegroundWarsongGulch::UpdateFlagState(TeamId teamID, uint32 value, ObjectGuid flagKeeperGUID, uint8 j)
{
    auto ws1 = teamID == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_UNK_ALLIANCE : WorldStates::BG_WS_FLAG_UNK_HORDE;
    auto ws2 = teamID == TEAM_ALLIANCE ? WorldStates::BG_WS_FLAG_STATE_HORDE : WorldStates::BG_WS_FLAG_STATE_ALLIANCE;

    switch (value)
    {
        case BG_WS_FLAG_STATE_WAIT_RESPAWN:
            UpdateWorldState(ws1, 0);
            UpdateWorldState(ws2, BG_WS_FLAG_STATE_WAIT_RESPAWN);
            break;
        case BG_WS_FLAG_STATE_ON_BASE:
            UpdateWorldState(ws1, 0);
            UpdateWorldState(ws2, 1); // strange
            break;
        case BG_WS_FLAG_STATE_ON_PLAYER:
            UpdateWorldState(ws1, 1);
            UpdateWorldState(ws2, BG_WS_FLAG_STATE_ON_PLAYER);
            break;
        case BG_WS_FLAG_STATE_ON_GROUND:
            UpdateWorldState(ws1, -1);
            UpdateWorldState(ws2, BG_WS_FLAG_STATE_ON_GROUND);
            break;
        default:
            break;
    }

    _flagState[teamID][j] = value;
    _flagKeepers[teamID][j] = flagKeeperGUID;
}

// 238253
class spell_ws_gripping_chain : public SpellScript
{
    PrepareSpellScript(spell_ws_gripping_chain);

    void HandleHit(SpellEffIndex /*effectIndex*/)
    {
        if (GetCaster() && GetHitUnit())
            GetHitUnit()->GetMotionMaster()->MoveJump(GetCaster()->GetPosition(), 10.0f, 10.0f);
    }

    void Register()
    {
        OnEffectHitTarget += SpellEffectFn(spell_ws_gripping_chain::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 238435
class spell_ws_discombobulator : public AuraScript
{
    PrepareAuraScript(spell_ws_discombobulator);

    void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget())
        {
            GetTarget()->Dismount();
            GetTarget()->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }
    }

    void Register()
    {
        OnEffectApply += AuraEffectApplyFn(spell_ws_discombobulator::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_battleground_warsong()
{
    RegisterSpellScript(spell_ws_gripping_chain);
    RegisterAuraScript(spell_ws_discombobulator);
}
