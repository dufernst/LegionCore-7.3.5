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
#include "BattlegroundEyeOfTheStorm.h"
#include "Creature.h"
#include "Object.h"
#include "Player.h"
#include "Util.h"
#include "WorldStatePackets.h"

BattlegroundEyeOfTheStorm::BattlegroundEyeOfTheStorm(): _checkFlagPickuperPos(0), _flagsTimer(0), _towerCapCheckTimer(0), _pointAddingTimer(0), _honorTics(0), _flagState(0), _isInformedNearVictory(false)
{
    m_BuffChange = true;
    BgObjects.resize(BG_EY_OBJECT_MAX);
    BgCreatures.resize(BG_EY_CREATURES_MAX);
    _pointsTrigger[FEL_REAVER] = TR_FEL_REAVER_BUFF;
    _pointsTrigger[BLOOD_ELF] = TR_BLOOD_ELF_BUFF;
    _pointsTrigger[DRAENEI_RUINS] = TR_DRAENEI_RUINS_BUFF;
    _pointsTrigger[MAGE_TOWER] = TR_MAGE_TOWER_BUFF;

    m_brawlTimer = 45000;
}

BattlegroundEyeOfTheStorm::~BattlegroundEyeOfTheStorm() = default;

void BattlegroundEyeOfTheStorm::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    _pointAddingTimer -= diff;
    if (_pointAddingTimer <= 0)
    {
        _pointAddingTimer = BG_EY_FPOINTS_TICK_TIME;
        if (_teamPointsCount[TEAM_ALLIANCE])
            AddPoints(TEAM_ALLIANCE, BG_EY_TickPoints[_teamPointsCount[TEAM_ALLIANCE] - 1]);
        if (_teamPointsCount[TEAM_HORDE])
            AddPoints(TEAM_HORDE, BG_EY_TickPoints[_teamPointsCount[TEAM_HORDE] - 1]);
    }

    //if (_flagState == BG_EY_FLAG_STATE_ON_PLAYER)
    {
        _checkFlagPickuperPos -= diff;
        if (_checkFlagPickuperPos <= 0)
        {
            if (Player* player = ObjectAccessor::FindPlayer(GetFlagPickerGUID(), true))
            {
                for (uint8 i = FEL_REAVER; i < EY_POINTS_MAX; i++)
                    if (player->GetDistance(PointData[i]) < 3.0f)
                        if (_pointState[i] == EY_POINT_UNDER_CONTROL && _pointOwnedByTeam[i] == player->GetBGTeam())
                            EventPlayerCapturedFlag(player);
            }

            _checkFlagPickuperPos = 1 * IN_MILLISECONDS;
        }
    }

    if (_flagState == BG_EY_FLAG_STATE_WAIT_RESPAWN || _flagState == BG_EY_FLAG_STATE_ON_GROUND)
    {
        _flagsTimer -= diff;

        if (_flagsTimer < 0)
        {
            _flagsTimer = 0;
            if (_flagState == BG_EY_FLAG_STATE_WAIT_RESPAWN)
                RespawnFlag();
            else
                RespawnFlagAfterDrop();
        }
    }

    _towerCapCheckTimer -= diff;
    if (_towerCapCheckTimer <= 0)
    {
        //check if player joined point
        /*I used this order of calls, because although we will check if one player is in gameobject's distance 2 times
          but we can count of players on current point in _CheckSomeoneLeftPoint
          */
        _CheckSomeoneJoinedPoint();
        _CheckSomeoneLeftPoint();
        UpdatePointStatuses();
        _towerCapCheckTimer = BG_EY_FPOINTS_TICK_TIME;
    }

    if (!IsBrawl())
        return;

    if (m_brawlTimer <= diff)
    {
        GetBgMap()->ApplyOnEveryPlayer([](Player* player)
        {
            if (player->isAlive())
                player->CastSpell(player, BG_EY_BRAWL_GRAVITY_LAPSE, true);
        });

        uint32 maxScore = std::max(m_TeamScores[TEAM_ALLIANCE], m_TeamScores[TEAM_HORDE]);
        if (maxScore >= BG_EY_MAX_TEAM_SCORE/2 && urand(0, BG_EY_MAX_TEAM_SCORE) <= maxScore)
            m_brawlTimer = urand(35, 45) * 1000;
        else
            m_brawlTimer = 45000;

        m_brawlAnnounceWas = false;
    }
    else
    {

        m_brawlTimer -= diff;
        if (m_brawlTimer <= 15000 && !m_brawlAnnounceWas)
        {
            m_brawlAnnounceWas = true;
            SendBroadcastText(124913, CHAT_MSG_RAID_BOSS_WHISPER);
        }
    }
}

void BattlegroundEyeOfTheStorm::GetPlayerPositionData(std::vector<WorldPackets::Battleground::BattlegroundPlayerPosition>* positions) const
{
    ObjectGuid guid = GetFlagPickerGUID();
    Player* player = ObjectAccessor::FindPlayer(guid, true);
    if (!player)
        return;

    WorldPackets::Battleground::BattlegroundPlayerPosition position;
    position.Guid = guid;
    position.Pos = player->GetPosition();
    position.IconID = player->GetBGTeam() == ALLIANCE ? PLAYER_POSITION_ICON_ALLIANCE_FLAG : PLAYER_POSITION_ICON_HORDE_FLAG;
    position.ArenaSlot = PLAYER_POSITION_ARENA_SLOT_NONE;
    positions->push_back(position);
}

void BattlegroundEyeOfTheStorm::StartingEventCloseDoors()
{
    SpawnBGObject(BG_EY_OBJECT_DOOR_A, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_EY_OBJECT_DOOR_H, RESPAWN_IMMEDIATELY);

    for (uint32 i = BG_EY_OBJECT_A_BANNER_FEL_REAVER_CENTER; i < BG_EY_OBJECT_MAX; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);
}

void BattlegroundEyeOfTheStorm::StartingEventOpenDoors()
{
    SpawnBGObject(BG_EY_OBJECT_DOOR_A, RESPAWN_ONE_DAY);
    SpawnBGObject(BG_EY_OBJECT_DOOR_H, RESPAWN_ONE_DAY);

    for (uint32 i = BG_EY_OBJECT_N_BANNER_FEL_REAVER_CENTER; i <= BG_EY_OBJECT_FLAG_NETHERSTORM; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    for (uint32 i = 0; i < EY_POINTS_MAX; ++i)
        SpawnBGObject(BG_EY_OBJECT_SPEEDBUFF_FEL_REAVER + urand(0, 2) + i * 3, RESPAWN_IMMEDIATELY);

    // Achievement: Flurry
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, EY_EVENT_START_BATTLE);
    StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, EY_EVENT_START_BATTLE);
}

void BattlegroundEyeOfTheStorm::AddPoints(TeamId teamID, uint32 points)
{
    m_TeamScores[teamID] += points;
    _honorScoreTics[teamID] += points;

    if (_honorScoreTics[teamID] >= _honorTics)
    {
        RewardHonorToTeam(GetBonusHonorFromKill(1), MS::Battlegrounds::GetTeamByTeamId(teamID));
        _honorScoreTics[teamID] -= _honorTics;
    }

    UpdateTeamScore(teamID);
}

void BattlegroundEyeOfTheStorm::_CheckSomeoneJoinedPoint()
{
    for (uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        GameObject * obj = HashMapHolder<GameObject>::Find(BgObjects[BG_EY_OBJECT_TOWER_CAP_FEL_REAVER + i]);
        if (obj)
        {
            uint8 j = 0;
            while (j < _playersNearPoint[EY_POINTS_MAX].size())
            {
                Player* player = ObjectAccessor::FindPlayer(_playersNearPoint[EY_POINTS_MAX][j]);
                if (!player)
                {
                    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm:_CheckSomeoneJoinedPoint: Player (GUID: %u) not found!", _playersNearPoint[EY_POINTS_MAX][j].GetCounter());
                    ++j;
                    continue;
                }
                if (player->CanCaptureTowerPoint() && player->IsWithinDistInMap(obj, BG_EY_POINT_RADIUS))
                {
                    //player joined point!
                    //show progress bar
                    player->SendUpdateWorldState(WorldStates::PROGRESS_BAR_PERCENT_GREY, BG_EY_PROGRESS_BAR_PERCENT_GREY);
                    player->SendUpdateWorldState(WorldStates::PROGRESS_BAR_STATUS, _pointBarStatus[i]);
                    player->SendUpdateWorldState(WorldStates::PROGRESS_BAR_SHOW, BG_EY_PROGRESS_BAR_SHOW);
                    //add player to point
                    _playersNearPoint[i].push_back(_playersNearPoint[EY_POINTS_MAX][j]);
                    //remove player from "free space"
                    _playersNearPoint[EY_POINTS_MAX].erase(_playersNearPoint[EY_POINTS_MAX].begin() + j);
                }
                else
                    ++j;
            }
        }
    }
}

void BattlegroundEyeOfTheStorm::_CheckSomeoneLeftPoint()
{
    //reset current point counts
    for (uint8 i = 0; i < 2 * EY_POINTS_MAX; ++i)
        _currentPointPlayersCount[i] = 0;
    for (uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        GameObject * obj = HashMapHolder<GameObject>::Find(BgObjects[BG_EY_OBJECT_TOWER_CAP_FEL_REAVER + i]);
        if (obj)
        {
            uint8 j = 0;
            while (j < _playersNearPoint[i].size())
            {
                Player* player = ObjectAccessor::FindPlayer(_playersNearPoint[i][j]);
                if (!player)
                {
                    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm:_CheckSomeoneLeftPoint Player (GUID: %u) not found!", _playersNearPoint[i][j].GetCounter());
                    //move not existed player to "free space" - this will cause many error showing in log, but it is a very important bug
                    _playersNearPoint[EY_POINTS_MAX].push_back(_playersNearPoint[i][j]);
                    _playersNearPoint[i].erase(_playersNearPoint[i].begin() + j);
                    continue;
                }
                if (!player->CanCaptureTowerPoint() || !player->IsWithinDistInMap(obj, BG_EY_POINT_RADIUS))
                    //move player out of point (add him to players that are out of points
                {
                    _playersNearPoint[EY_POINTS_MAX].push_back(_playersNearPoint[i][j]);
                    _playersNearPoint[i].erase(_playersNearPoint[i].begin() + j);
                    player->SendUpdateWorldState(WorldStates::PROGRESS_BAR_SHOW, BG_EY_PROGRESS_BAR_DONT_SHOW);
                }
                else
                {
                    //player is neat flag, so update count:
                    _currentPointPlayersCount[2 * i + player->GetBGTeamId()]++;
                    ++j;
                }
            }
        }
    }
}

void BattlegroundEyeOfTheStorm::UpdatePointStatuses()
{
    for (uint8 point = 0; point < EY_POINTS_MAX; ++point)
    {
        if (_playersNearPoint[point].empty())
            continue;
        //count new point bar status:
        _pointBarStatus[point] += (_currentPointPlayersCount[2 * point] - _currentPointPlayersCount[2 * point + 1] < BG_EY_POINT_MAX_CAPTURERS_COUNT) ? _currentPointPlayersCount[2 * point] - _currentPointPlayersCount[2 * point + 1] : BG_EY_POINT_MAX_CAPTURERS_COUNT;

        if (_pointBarStatus[point] > BG_EY_PROGRESS_BAR_ALI_CONTROLLED)
            _pointBarStatus[point] = BG_EY_PROGRESS_BAR_ALI_CONTROLLED;

        if (_pointBarStatus[point] < BG_EY_PROGRESS_BAR_HORDE_CONTROLLED)
            _pointBarStatus[point] = BG_EY_PROGRESS_BAR_HORDE_CONTROLLED;

        uint32 pointOwnerTeamId = EY_POINT_NO_OWNER;
        //find which team should own this point
        if (_pointBarStatus[point] <= BG_EY_PROGRESS_BAR_NEUTRAL_LOW)
            pointOwnerTeamId = HORDE;
        else if (_pointBarStatus[point] >= BG_EY_PROGRESS_BAR_NEUTRAL_HIGH)
            pointOwnerTeamId = ALLIANCE;

        for (size_t i = 0; i < _playersNearPoint[point].size(); ++i)
        {
            Player* player = ObjectAccessor::FindPlayer(_playersNearPoint[point][i]);
            if (player)
            {
                player->SendUpdateWorldState(WorldStates::PROGRESS_BAR_STATUS, _pointBarStatus[point]);
                //if point owner changed we must evoke event!
                if (pointOwnerTeamId != _pointOwnedByTeam[point])
                {
                    //point was uncontrolled and player is from team which captured point
                    if (_pointState[point] == EY_POINT_STATE_UNCONTROLLED && player->GetBGTeam() == pointOwnerTeamId)
                        EventTeamCapturedPoint(player, point);

                    //point was under control and player isn't from team which controlled it
                    if (_pointState[point] == EY_POINT_UNDER_CONTROL && player->GetBGTeam() != _pointOwnedByTeam[point])
                        EventTeamLostPoint(player, point);
                }
            }
        }
    }
}

void BattlegroundEyeOfTheStorm::UpdateTeamScore(TeamId teamID)
{
    uint32 score = m_TeamScores[teamID];

    if (!_isInformedNearVictory && m_TeamScores[teamID] > BG_EY_WARNING_NEAR_VICTORY_SCORE)
    {
        SendBroadcastText(teamID == TEAM_ALLIANCE ? 10598 : 10599, CHAT_MSG_BG_SYSTEM_NEUTRAL);
        PlaySoundToAll(BG_SOUND_NEAR_VICTORY);

        _isInformedNearVictory = true;
    }

    if (score >= BG_EY_MAX_TEAM_SCORE)
    {
        score = BG_EY_MAX_TEAM_SCORE;
        EndBattleground(teamID);
    }

    Battleground::SendBattleGroundPoints(teamID != TEAM_ALLIANCE, m_TeamScores[teamID]);

    if (teamID == TEAM_ALLIANCE)
        UpdateWorldState(WorldStates::ALLIANCE_RESOUCES, score);
    else
        UpdateWorldState(WorldStates::HORDE_RESOUCES, score);
}

void BattlegroundEyeOfTheStorm::EndBattleground(uint32 winner)
{
    uint32 realWinner = WINNER_NONE;
    if (winner == TEAM_ALLIANCE)
        realWinner = ALLIANCE;
    else if (winner == TEAM_HORDE)
        realWinner = HORDE;

    Battleground::EndBattleground(realWinner);
}

void BattlegroundEyeOfTheStorm::UpdatePointsCount(uint32 Team)
{
    if (Team == ALLIANCE)
        UpdateWorldState(WorldStates::OCCOPIED_BASES_ALLIANCE, _teamPointsCount[TEAM_ALLIANCE]);
    else
        UpdateWorldState(WorldStates::OCCOPIED_BASES_HORDE, _teamPointsCount[TEAM_HORDE]);
}

void BattlegroundEyeOfTheStorm::UpdatePointsIcons(uint32 Team, uint32 Point)
{
    //we MUST firstly send 0, after that we can send 1!!!
    if (_pointState[Point] == EY_POINT_UNDER_CONTROL)
    {
        UpdateWorldState(m_PointsIconStruct[Point].WorldStateControlIndex, 0);
        if (Team == ALLIANCE)
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateAllianceControlledIndex, 1);
        else
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateHordeControlledIndex, 1);
    }
    else
    {
        if (Team == ALLIANCE)
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateAllianceControlledIndex, 0);
        else
            UpdateWorldState(m_PointsIconStruct[Point].WorldStateHordeControlledIndex, 0);
        UpdateWorldState(m_PointsIconStruct[Point].WorldStateControlIndex, 1);
    }
}

void BattlegroundEyeOfTheStorm::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundEYScore(player->GetGUID(), player->GetBGTeamId());

    player->SendDirectMessage(WorldPackets::Battleground::Init(BG_EY_MAX_TEAM_SCORE).Write());
    Battleground::SendBattleGroundPoints(player->GetBGTeamId() != TEAM_ALLIANCE, m_TeamScores[player->GetBGTeamId()], false, player);

    _playersNearPoint[EY_POINTS_MAX].push_back(player->GetGUID());
}

void BattlegroundEyeOfTheStorm::RemovePlayer(Player* player, ObjectGuid guid, uint32 /*team*/)
{
    // sometimes flag aura not removed :(
    for (int j = EY_POINTS_MAX; j >= 0; --j)
    {
        for (size_t i = 0; i < _playersNearPoint[j].size(); ++i)
            if (_playersNearPoint[j][i] == guid)
                _playersNearPoint[j].erase(_playersNearPoint[j].begin() + i);
    }
    if (IsFlagPickedup())
    {
        if (GetFlagPickerGUID() == guid)
        {
            if (player)
                EventPlayerDroppedFlag(player);
            else
            {
                SetFlagPicker(ObjectGuid::Empty);
                RespawnFlag();
            }
        }
    }
}

bool BattlegroundEyeOfTheStorm::SetupBattleground()
{
    if (!AddObject(BG_EY_OBJECT_DOOR_A, BG_OBJECT_A_DOOR_EY_ENTRY, EYObjectPos[0][0], EYObjectPos[0][1], EYObjectPos[0][2], EYObjectPos[0][3], EYObjectPos[0][4], EYObjectPos[0][5], EYObjectPos[0][6], EYObjectPos[0][7], RESPAWN_IMMEDIATELY) ||
        !AddObject(BG_EY_OBJECT_DOOR_H, BG_OBJECT_H_DOOR_EY_ENTRY, EYObjectPos[1][0], EYObjectPos[1][1], EYObjectPos[1][2], EYObjectPos[1][3], EYObjectPos[1][4], EYObjectPos[1][5], EYObjectPos[1][6], EYObjectPos[1][7], RESPAWN_IMMEDIATELY)
        // banners (alliance)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REAVER_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2057.46f, 1735.07f, 1187.91f, -0.925024f, 0, 0, 0.446198f, -0.894934f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REAVER_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2032.25f, 1729.53f, 1190.33f, 1.8675f, 0, 0, 0.803857f, 0.594823f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_FEL_REAVER_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2092.35f, 1775.46f, 1187.08f, -0.401426f, 0, 0, 0.199368f, -0.979925f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2047.19f, 1349.19f, 1189.0f, -1.62316f, 0, 0, 0.725374f, -0.688354f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2074.32f, 1385.78f, 1194.72f, 0.488692f, 0, 0, 0.241922f, 0.970296f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2025.13f, 1386.12f, 1192.74f, 2.3911f, 0, 0, 0.930418f, 0.366501f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2276.8f, 1400.41f, 1196.33f, 2.44346f, 0, 0, 0.939693f, 0.34202f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2305.78f, 1404.56f, 1199.38f, 1.74533f, 0, 0, 0.766044f, 0.642788f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2245.4f, 1366.41f, 1195.28f, 2.21657f, 0, 0, 0.894934f, 0.446198f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_A_BANNER_EY_ENTRY, 2270.84f, 1784.08f, 1186.76f, 2.42601f, 0, 0, 0.936672f, 0.350207f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_A_BANNER_EY_ENTRY, 2269.13f, 1737.7f, 1186.66f, 0.994838f, 0, 0, 0.477159f, 0.878817f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_A_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_A_BANNER_EY_ENTRY, 2300.86f, 1741.25f, 1187.7f, -0.785398f, 0, 0, 0.382683f, -0.92388f, RESPAWN_ONE_DAY)
        // banners (horde)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REAVER_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2057.46f, 1735.07f, 1187.91f, -0.925024f, 0, 0, 0.446198f, -0.894934f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REAVER_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2032.25f, 1729.53f, 1190.33f, 1.8675f, 0, 0, 0.803857f, 0.594823f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_FEL_REAVER_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2092.35f, 1775.46f, 1187.08f, -0.401426f, 0, 0, 0.199368f, -0.979925f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2047.19f, 1349.19f, 1189.0f, -1.62316f, 0, 0, 0.725374f, -0.688354f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2074.32f, 1385.78f, 1194.72f, 0.488692f, 0, 0, 0.241922f, 0.970296f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2025.13f, 1386.12f, 1192.74f, 2.3911f, 0, 0, 0.930418f, 0.366501f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2276.8f, 1400.41f, 1196.33f, 2.44346f, 0, 0, 0.939693f, 0.34202f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2305.78f, 1404.56f, 1199.38f, 1.74533f, 0, 0, 0.766044f, 0.642788f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2245.4f, 1366.41f, 1195.28f, 2.21657f, 0, 0, 0.894934f, 0.446198f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_H_BANNER_EY_ENTRY, 2270.84f, 1784.08f, 1186.76f, 2.42601f, 0, 0, 0.936672f, 0.350207f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_H_BANNER_EY_ENTRY, 2269.13f, 1737.7f, 1186.66f, 0.994838f, 0, 0, 0.477159f, 0.878817f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_H_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_H_BANNER_EY_ENTRY, 2300.86f, 1741.25f, 1187.7f, -0.785398f, 0, 0, 0.382683f, -0.92388f, RESPAWN_ONE_DAY)
        // banners (natural)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REAVER_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2057.46f, 1735.07f, 1187.91f, -0.925024f, 0, 0, 0.446198f, -0.894934f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REAVER_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2032.25f, 1729.53f, 1190.33f, 1.8675f, 0, 0, 0.803857f, 0.594823f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_FEL_REAVER_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2092.35f, 1775.46f, 1187.08f, -0.401426f, 0, 0, 0.199368f, -0.979925f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2047.19f, 1349.19f, 1189.0f, -1.62316f, 0, 0, 0.725374f, -0.688354f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2074.32f, 1385.78f, 1194.72f, 0.488692f, 0, 0, 0.241922f, 0.970296f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_BLOOD_ELF_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2025.13f, 1386.12f, 1192.74f, 2.3911f, 0, 0, 0.930418f, 0.366501f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2276.8f, 1400.41f, 1196.33f, 2.44346f, 0, 0, 0.939693f, 0.34202f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2305.78f, 1404.56f, 1199.38f, 1.74533f, 0, 0, 0.766044f, 0.642788f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_DRAENEI_RUINS_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2245.4f, 1366.41f, 1195.28f, 2.21657f, 0, 0, 0.894934f, 0.446198f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_CENTER, BG_OBJECT_N_BANNER_EY_ENTRY, 2270.84f, 1784.08f, 1186.76f, 2.42601f, 0, 0, 0.936672f, 0.350207f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_LEFT, BG_OBJECT_N_BANNER_EY_ENTRY, 2269.13f, 1737.7f, 1186.66f, 0.994838f, 0, 0, 0.477159f, 0.878817f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_N_BANNER_MAGE_TOWER_RIGHT, BG_OBJECT_N_BANNER_EY_ENTRY, 2300.86f, 1741.25f, 1187.7f, -0.785398f, 0, 0, 0.382683f, -0.92388f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_FLAG_NETHERSTORM, BG_OBJECT_FLAG1_EY_ENTRY, 2174.445f, 1569.422f, 1159.853f, 4.625124f, 0, 0, -0.737277f, 0.6755905f, RESPAWN_ONE_DAY)
        // tower cap
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_FEL_REAVER, BG_OBJECT_FR_TOWER_CAP_EY_ENTRY, 2024.600708f, 1742.819580f, 1195.157715f, 2.443461f, 0, 0, 0.939693f, 0.342020f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_BLOOD_ELF, BG_OBJECT_BE_TOWER_CAP_EY_ENTRY, 2050.493164f, 1372.235962f, 1194.563477f, 1.710423f, 0, 0, 0.754710f, 0.656059f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_DRAENEI_RUINS, BG_OBJECT_DR_TOWER_CAP_EY_ENTRY, 2301.010498f, 1386.931641f, 1197.183472f, 1.570796f, 0, 0, 0.707107f, 0.707107f, RESPAWN_ONE_DAY)
        || !AddObject(BG_EY_OBJECT_TOWER_CAP_MAGE_TOWER, BG_OBJECT_HU_TOWER_CAP_EY_ENTRY, 2282.121582f, 1760.006958f, 1189.707153f, 1.919862f, 0, 0, 0.819152f, 0.573576f, RESPAWN_ONE_DAY)
        )
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundEY: Failed to spawn some object Battleground not created!");
        return false;
    }

    //buffs
    for (int i = 0; i < EY_POINTS_MAX; ++i)
    {
        AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(_pointsTrigger[i]);
        if (!at)
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm: Unknown trigger: %u", _pointsTrigger[i]);
            continue;
        }
        if (!AddObject(BG_EY_OBJECT_SPEEDBUFF_FEL_REAVER + i * 3, Buff_Entries[0], at->Pos.X, at->Pos.Y, at->Pos.Z, 0.907571f, 0, 0, 0.438371f, 0.898794f, RESPAWN_ONE_DAY)
            || !AddObject(BG_EY_OBJECT_SPEEDBUFF_FEL_REAVER + i * 3 + 1, Buff_Entries[1], at->Pos.X, at->Pos.Y, at->Pos.Z, 0.907571f, 0, 0, 0.438371f, 0.898794f, RESPAWN_ONE_DAY)
            || !AddObject(BG_EY_OBJECT_SPEEDBUFF_FEL_REAVER + i * 3 + 2, Buff_Entries[2], at->Pos.X, at->Pos.Y, at->Pos.Z, 0.907571f, 0, 0, 0.438371f, 0.898794f, RESPAWN_ONE_DAY)
            )
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm: Cannot spawn buff");
    }

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(EY_GRAVEYARD_MAIN_ALLIANCE);
    if (!sg || !AddSpiritGuide(EY_SPIRIT_MAIN_ALLIANCE, sg->Loc, TEAM_ALLIANCE))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundEY: Failed to spawn spirit guide! Battleground not created!");
        return false;
    }

    sg = sWorldSafeLocsStore.LookupEntry(EY_GRAVEYARD_MAIN_HORDE);
    if (!sg || !AddSpiritGuide(EY_SPIRIT_MAIN_HORDE, sg->Loc, TEAM_HORDE))
    {
        TC_LOG_ERROR(LOG_FILTER_SQL, "BatteGroundEY: Failed to spawn spirit guide! Battleground not created!");
        return false;
    }

    return true;
}

void BattlegroundEyeOfTheStorm::Reset()
{
    Battleground::Reset();

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        m_TeamScores[i] = TEAM_ALLIANCE;
        _teamPointsCount[i] = 0;
        _honorScoreTics[i] = 0;
    }

    _flagState = BG_EY_FLAG_STATE_ON_BASE;
    _flagKeeper.Clear();
    _droppedFlagGUID.Clear();
    _pointAddingTimer = 0;
    _towerCapCheckTimer = 0;
    _honorTics = 260;
    _checkFlagPickuperPos = 1 * IN_MILLISECONDS;

    for (uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        _pointOwnedByTeam[i] = EY_POINT_NO_OWNER;
        _pointState[i] = EY_POINT_STATE_UNCONTROLLED;
        _pointBarStatus[i] = BG_EY_PROGRESS_BAR_STATE_MIDDLE;
        _playersNearPoint[i].clear();
        _playersNearPoint[i].reserve(15);                  //tip size
    }

    _playersNearPoint[EY_PLAYERS_OUT_OF_POINTS].clear();
    _playersNearPoint[EY_PLAYERS_OUT_OF_POINTS].reserve(30);
    _isInformedNearVictory = false;
}

void BattlegroundEyeOfTheStorm::RespawnFlag()
{
    if (GameObject* obj = HashMapHolder<GameObject>::Find(GetDroppedFlagGUID()))
    {
        obj->Delete();
        SetDroppedFlagGUID(ObjectGuid::Empty);
    }

    _flagState = BG_EY_FLAG_STATE_ON_BASE;
    SpawnBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM, RESPAWN_IMMEDIATELY);

    SendBroadcastText(18364, CHAT_MSG_BG_SYSTEM_NEUTRAL);
    PlaySoundToAll(BG_SOUND_FLAG_RESET);

    UpdateWorldState(WorldStates::NETHERSTORM_FLAG, 1);
    UpdateWorldState(static_cast<WorldStates>(8863), 1);
}

void BattlegroundEyeOfTheStorm::RespawnFlagAfterDrop()
{
    if (GameObject* obj = HashMapHolder<GameObject>::Find(GetDroppedFlagGUID()))
        obj->Delete();

    SetDroppedFlagGUID(ObjectGuid::Empty);

    RespawnFlag();
}

void BattlegroundEyeOfTheStorm::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    Battleground::HandleKillPlayer(player, killer);
    EventPlayerDroppedFlag(player);
}

void BattlegroundEyeOfTheStorm::EventPlayerDroppedFlag(Player* Source)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
    {
        // if not running, do not cast things at the dropper player, neither send unnecessary messages
        // just take off the aura
        if (IsFlagPickedup() && GetFlagPickerGUID() == Source->GetGUID())
        {
            SetFlagPicker(ObjectGuid::Empty);
            Source->RemoveAurasDueToSpell(BG_EY_NETHERSTORM_FLAG_SPELL);
        }
        return;
    }

    if (!IsFlagPickedup())
        return;

    if (GetFlagPickerGUID() != Source->GetGUID())
        return;

    SetFlagPicker(ObjectGuid::Empty);
    Source->RemoveAurasDueToSpell(BG_EY_NETHERSTORM_FLAG_SPELL);
    _flagState = BG_EY_FLAG_STATE_ON_GROUND;
    _flagsTimer = 20000;
    Source->CastSpell(Source, SPELL_BG_RECENTLY_DROPPED_FLAG, true);
    //Source->CastSpell(Source, BG_EY_PLAYER_DROPPED_FLAG_SPELL, true); //Deprecated

    if (GameObject* pGameObj = Source->SummonGameObject(BG_OBJECT_FLAG2_EY_ENTRY, Source->GetPositionX() + 1.0f, Source->GetPositionY() - 1.0f, Source->GetPositionZ() + 0.5f, 0, 0, 0, 0, 0, DAY, ObjectGuid::Empty, nullptr, false))
        SetDroppedFlagGUID(pGameObj->GetGUID());

    //this does not work correctly :((it should remove flag carrier name)
    UpdateWorldState(WorldStates::NETHERSTORM_FLAG_STATE_HORDE, BG_EY_FLAG_STATE_WAIT_RESPAWN);
    UpdateWorldState(WorldStates::NETHERSTORM_FLAG_STATE_ALLIANCE, BG_EY_FLAG_STATE_WAIT_RESPAWN);

    UpdateWorldState(Source->GetTeamId() == TEAM_ALLIANCE ? WorldStates::NETHERSTROM_FLAG_UI_ALLIANCE : WorldStates::NETHERSTROM_FLAG_UI_HORDE, _flagState);

    if (Source->GetTeam() == ALLIANCE)
        SendBroadcastText(BG_EY_TEXT_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_ALLIANCE);
    else
        SendBroadcastText(BG_EY_TEXT_FLAG_DROPPED, CHAT_MSG_BG_SYSTEM_HORDE);
}

void BattlegroundEyeOfTheStorm::EventPlayerClickedOnFlag(Player* Source, GameObject* object, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS || IsFlagPickedup() || !Source->IsWithinDistInMap(object, 10))
        return;

    TeamId teamID = Source->GetBGTeamId();

    UpdateWorldState(teamID == TEAM_ALLIANCE ? WorldStates::NETHERSTORM_FLAG_STATE_ALLIANCE : WorldStates::NETHERSTORM_FLAG_STATE_HORDE, BG_EY_FLAG_STATE_ON_PLAYER);
    PlayeCapturePointSound(NODE_STATUS_ASSAULT, teamID);

    if (_flagState == BG_EY_FLAG_STATE_ON_BASE)
        UpdateWorldState(WorldStates::NETHERSTORM_FLAG, 0);

    _flagState = BG_EY_FLAG_STATE_ON_PLAYER;

    UpdateWorldState(static_cast<WorldStates>(8863), _flagState);

    if (teamID == TEAM_ALLIANCE)
    {
        UpdateWorldState(WorldStates::NETHERSTROM_FLAG_UI_ALLIANCE, BG_EY_FLAG_STATE_ON_PLAYER);
        UpdateWorldState(WorldStates::NETHERSTROM_FLAG_UI_HORDE, BG_EY_FLAG_STATE_WAIT_RESPAWN);
    }
    else
    {
        UpdateWorldState(WorldStates::NETHERSTROM_FLAG_UI_ALLIANCE, BG_EY_FLAG_STATE_WAIT_RESPAWN);
        UpdateWorldState(WorldStates::NETHERSTROM_FLAG_UI_HORDE, BG_EY_FLAG_STATE_ON_PLAYER);
    }

    SpawnBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM, RESPAWN_ONE_DAY);
    SetFlagPicker(Source->GetGUID());
    Source->CastSpell(Source, BG_EY_NETHERSTORM_FLAG_SPELL, true);
    Source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    SendBroadcastText(18375, teamID == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE, Source);
}

void BattlegroundEyeOfTheStorm::EventTeamLostPoint(Player* Source, uint32 Point)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint32 Team = _pointOwnedByTeam[Point];
    if (!Team)
        return;

    if (Team == ALLIANCE)
    {
        _teamPointsCount[TEAM_ALLIANCE]--;
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeAlliance, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeAlliance + 1, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeAlliance + 2, RESPAWN_ONE_DAY);
    }
    else
    {
        _teamPointsCount[TEAM_HORDE]--;
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeHorde, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeHorde + 1, RESPAWN_ONE_DAY);
        SpawnBGObject(m_LosingPointTypes[Point].DespawnObjectTypeHorde + 2, RESPAWN_ONE_DAY);
    }

    SpawnBGObject(m_LosingPointTypes[Point].SpawnNeutralObjectType, RESPAWN_IMMEDIATELY);
    SpawnBGObject(m_LosingPointTypes[Point].SpawnNeutralObjectType + 1, RESPAWN_IMMEDIATELY);
    SpawnBGObject(m_LosingPointTypes[Point].SpawnNeutralObjectType + 2, RESPAWN_IMMEDIATELY);

    //buff isn't despawned

    _pointOwnedByTeam[Point] = EY_POINT_NO_OWNER;
    _pointState[Point] = EY_POINT_NO_OWNER;

    if (Team == ALLIANCE)
        SendBroadcastText(m_LosingPointTypes[Point].MessageIdAlliance, CHAT_MSG_BG_SYSTEM_ALLIANCE, Source);
    else
        SendBroadcastText(m_LosingPointTypes[Point].MessageIdHorde, CHAT_MSG_BG_SYSTEM_HORDE, Source);

    UpdatePointsIcons(Team, Point);
    UpdatePointsCount(Team);

    //remove bonus honor aura trigger creature when node is lost
    if (Point < EY_POINTS_MAX)
        DelCreature(Point + 6);//nullptr checks are in DelCreature! 0-5 spirit guides
}

void BattlegroundEyeOfTheStorm::EventTeamCapturedPoint(Player* Source, uint32 Point)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (Point >= EY_POINTS_MAX)
        return;

    uint32 Team = Source->GetBGTeam();

    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType, RESPAWN_ONE_DAY);
    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType + 1, RESPAWN_ONE_DAY);
    SpawnBGObject(m_CapturingPointTypes[Point].DespawnNeutralObjectType + 2, RESPAWN_ONE_DAY);

    if (Team == ALLIANCE)
    {
        _teamPointsCount[TEAM_ALLIANCE]++;
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance + 1, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeAlliance + 2, RESPAWN_IMMEDIATELY);
    }
    else
    {
        _teamPointsCount[TEAM_HORDE]++;
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde + 1, RESPAWN_IMMEDIATELY);
        SpawnBGObject(m_CapturingPointTypes[Point].SpawnObjectTypeHorde + 2, RESPAWN_IMMEDIATELY);
    }

    //buff isn't respawned

    _pointOwnedByTeam[Point] = Team;
    _pointState[Point] = EY_POINT_UNDER_CONTROL;

    if (Team == ALLIANCE)
        SendBroadcastText(m_CapturingPointTypes[Point].MessageIdAlliance, CHAT_MSG_BG_SYSTEM_ALLIANCE, Source);
    else
        SendBroadcastText(m_CapturingPointTypes[Point].MessageIdHorde, CHAT_MSG_BG_SYSTEM_HORDE, Source);

    if (!BgCreatures[Point].IsEmpty())
        DelCreature(Point);

    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(m_CapturingPointTypes[Point].GraveYardId);
    if (!sg || !AddSpiritGuide(Point, sg->Loc, Source->GetBGTeamId()))
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BatteGroundEY: Failed to spawn spirit guide! point: %u, team: %u, graveyard_id: %u", Point, Team, m_CapturingPointTypes[Point].GraveYardId);

    //    SpawnBGCreature(Point, RESPAWN_IMMEDIATELY);

    UpdatePointsIcons(Team, Point);
    UpdatePointsCount(Team);

    Creature* trigger = GetBGCreature(Point + 6);//0-5 spirit guides
    if (!trigger)
        trigger = AddCreature(WORLD_TRIGGER, Point + 6, Team, BG_EY_TriggerPositions[Point][0], BG_EY_TriggerPositions[Point][1], BG_EY_TriggerPositions[Point][2], BG_EY_TriggerPositions[Point][3]);

    if (trigger)
    {
        trigger->setFaction(Team == ALLIANCE ? 84 : 83);
        trigger->CastSpell(trigger, SPELL_BG_HONORABLE_DEFENDER_60Y, false);
    }
}

void BattlegroundEyeOfTheStorm::EventPlayerCapturedFlag(Player* player)
{
    TeamId teamId = player->GetBGTeamId();

    SetFlagPicker(ObjectGuid::Empty);
    _flagState = BG_EY_FLAG_STATE_WAIT_RESPAWN;
    player->RemoveAura(BG_EY_NETHERSTORM_FLAG_SPELL, player->GetGUID());
    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    _flagsTimer = 20000;

    PlayeCapturePointSound(NODE_STATUS_CAPTURE, teamId);
    if (teamId == TEAM_ALLIANCE)
        SendBroadcastText(BG_EY_TEXT_ALLIANCE_CAPTURED_FLAG, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
    else
        SendBroadcastText(BG_EY_TEXT_HORDE_CAPTURED_FLAG, CHAT_MSG_BG_SYSTEM_HORDE, player);

    if (_teamPointsCount[teamId] > TEAM_ALLIANCE)
        AddPoints(teamId, BG_EY_FlagPoints[_teamPointsCount[teamId] - 1]);

    UpdateWorldState(teamId == TEAM_ALLIANCE ? WorldStates::NETHERSTROM_FLAG_UI_ALLIANCE : WorldStates::NETHERSTROM_FLAG_UI_HORDE, 4);
    RewardHonorToTeam(20, player->GetBGTeam());
    UpdatePlayerScore(player, SCORE_FLAG_CAPTURES, 1);
}

bool BattlegroundEyeOfTheStorm::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
    case SCORE_FLAG_CAPTURES:
        player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, EY_OBJECTIVE_CAPTURE_FLAG, 1);
        break;
    default:
        break;
    }
    return true;
}

void BattlegroundEyeOfTheStorm::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::HORDE_RESOUCES, m_TeamScores[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::ALLIANCE_RESOUCES, m_TeamScores[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::OCCOPIED_BASES_HORDE, _teamPointsCount[TEAM_HORDE]);
    packet.Worldstates.emplace_back(WorldStates::OCCOPIED_BASES_ALLIANCE, _teamPointsCount[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::MAX_TEAM_RESOUCES, BG_EY_MAX_TEAM_SCORE);

    packet.Worldstates.emplace_back(WorldStates::PROGRESS_BAR_SHOW, 0);
    packet.Worldstates.emplace_back(WorldStates::PROGRESS_BAR_STATUS, 0);
    packet.Worldstates.emplace_back(WorldStates::PROGRESS_BAR_PERCENT_GREY, 0);

    packet.Worldstates.emplace_back(WorldStates::BLOOD_ELF_UNCONTROL, _pointState[BLOOD_ELF] != EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::BLOOD_ELF_ALLIANCE_CONTROL, _pointOwnedByTeam[BLOOD_ELF] == ALLIANCE && _pointState[BLOOD_ELF] == EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::BLOOD_ELF_HORDE_CONTROL, _pointOwnedByTeam[BLOOD_ELF] == HORDE && _pointState[BLOOD_ELF] == EY_POINT_UNDER_CONTROL);

    packet.Worldstates.emplace_back(WorldStates::FEL_REAVER_UNCONTROL, _pointState[FEL_REAVER] != EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::FEL_REAVER_ALLIANCE_CONTROL, _pointOwnedByTeam[FEL_REAVER] == ALLIANCE && _pointState[FEL_REAVER] == EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::FEL_REAVER_HORDE_CONTROL, _pointOwnedByTeam[FEL_REAVER] == HORDE && _pointState[FEL_REAVER] == EY_POINT_UNDER_CONTROL);

    packet.Worldstates.emplace_back(WorldStates::MAGE_TOWER_UNCONTROL, _pointState[MAGE_TOWER] != EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::MAGE_TOWER_HORDE_CONTROL, _pointOwnedByTeam[MAGE_TOWER] == HORDE && _pointState[MAGE_TOWER] == EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::MAGE_TOWER_ALLIANCE_CONTROL, _pointOwnedByTeam[MAGE_TOWER] == ALLIANCE && _pointState[MAGE_TOWER] == EY_POINT_UNDER_CONTROL);

    packet.Worldstates.emplace_back(WorldStates::DRAENEI_RUINS_UNCONTROL, _pointState[DRAENEI_RUINS] != EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::DRAENEI_RUINS_ALLIANCE_CONTROL, _pointOwnedByTeam[DRAENEI_RUINS] == ALLIANCE && _pointState[DRAENEI_RUINS] == EY_POINT_UNDER_CONTROL);
    packet.Worldstates.emplace_back(WorldStates::DRAENEI_RUINS_HORDE_CONTROL, _pointOwnedByTeam[DRAENEI_RUINS] == HORDE && _pointState[DRAENEI_RUINS] == EY_POINT_UNDER_CONTROL);

    packet.Worldstates.emplace_back(static_cast<WorldStates>(2735), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2736), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2737), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2738), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2739), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2740), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2741), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2742), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2749), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2750), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2752), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(2753), 0);

    packet.Worldstates.emplace_back(WorldStates::NETHERSTORM_FLAG, _flagState == BG_EY_FLAG_STATE_ON_BASE);
    packet.Worldstates.emplace_back(WorldStates::NETHERSTORM_FLAG_STATE_HORDE, 1);
    packet.Worldstates.emplace_back(WorldStates::NETHERSTORM_FLAG_STATE_ALLIANCE, 1);

    packet.Worldstates.emplace_back(static_cast<WorldStates>(3217), 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(3218), 0);

    Player* player = ObjectAccessor::FindPlayer(GetFlagPickerGUID(), true);
    packet.Worldstates.emplace_back(WorldStates::NETHERSTROM_FLAG_UI_ALLIANCE, player ? player->GetBGTeamId() == TEAM_ALLIANCE ? (_flagState ? _flagState : 1) : 1 : 1);
    packet.Worldstates.emplace_back(WorldStates::NETHERSTROM_FLAG_UI_HORDE, player ? player->GetBGTeamId() == TEAM_HORDE ? (_flagState ? _flagState : 1) : 1 : 1);

    packet.Worldstates.emplace_back(static_cast<WorldStates>(10536), 1);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(10537), 1);
}

WorldSafeLocsEntry const* BattlegroundEyeOfTheStorm::GetClosestGraveYard(Player* player)
{
    uint32 g_id = 0;

    switch (player->GetBGTeam())
    {
        case ALLIANCE: g_id = EY_GRAVEYARD_MAIN_ALLIANCE; break;
        case HORDE:    g_id = EY_GRAVEYARD_MAIN_HORDE;    break;
        default:       return nullptr;
    }

    WorldSafeLocsEntry const* entry = sWorldSafeLocsStore.LookupEntry(g_id);
    WorldSafeLocsEntry const* nearestEntry = entry;
    if (!entry)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm: Not found the main team graveyard. Graveyard system isn't working!");
        return nullptr;
    }

    float plr_x = player->GetPositionX();
    float plr_y = player->GetPositionY();
    float plr_z = player->GetPositionZ();

    float distance = (entry->Loc.X - plr_x)*(entry->Loc.X - plr_x) + (entry->Loc.Y - plr_y)*(entry->Loc.Y - plr_y) + (entry->Loc.Z - plr_z)*(entry->Loc.Z - plr_z);
    float nearestDistance = distance;

    for (uint8 i = 0; i < EY_POINTS_MAX; ++i)
    {
        if (_pointOwnedByTeam[i] == player->GetBGTeam() && _pointState[i] == EY_POINT_UNDER_CONTROL)
        {
            entry = sWorldSafeLocsStore.LookupEntry(m_CapturingPointTypes[i].GraveYardId);
            if (!entry)
                TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundEyeOfTheStorm: Not found graveyard: %u", m_CapturingPointTypes[i].GraveYardId);
            else
            {
                distance = (entry->Loc.X - plr_x)*(entry->Loc.X - plr_x) + (entry->Loc.Y - plr_y)*(entry->Loc.Y - plr_y) + (entry->Loc.Z - plr_z)*(entry->Loc.Z - plr_z);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestEntry = entry;
                }
            }
        }
    }

    return nearestEntry;
}

bool BattlegroundEyeOfTheStorm::IsAllNodesConrolledByTeam(uint32 team) const
{
    uint32 count = 0;
    for (int i = 0; i < EY_POINTS_MAX; ++i)
        if (_pointOwnedByTeam[i] == team && _pointState[i] == EY_POINT_UNDER_CONTROL)
            ++count;

    return count == EY_POINTS_MAX;
}
