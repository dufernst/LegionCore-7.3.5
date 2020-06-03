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

#include "Battleground.h"
#include "BattlegroundStrandOfTheAncients.h"
#include "Language.h"
#include "Player.h"
#include "GameObject.h"
#include "CreatureAI.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "InstancePackets.h"
#include "WorldStatePackets.h"
#include <G3D/Quat.h>

void BattlegroundSAScore::UpdateScore(uint32 type, uint32 value)
{
    switch (type)
    {
        case SCORE_DESTROYED_DEMOLISHER:
            DemolishersDestroyed += value;
            break;
        case SCORE_DESTROYED_WALL:
            GatesDestroyed += value;
            break;
        default:
            BattlegroundScore::UpdateScore(type, value);
            break;
    }
}

void BattlegroundSAScore::BuildObjectivesBlock(std::vector<int32>& stats)
{
    stats.push_back(DemolishersDestroyed);
    stats.push_back(GatesDestroyed);
}

BattlegroundStrandOfTheAncients::BattlegroundStrandOfTheAncients() : gateDestroyed(false), Attackers(), GateStatus{ BG_SA_GATE_OK }, Status(BG_SA_NOTSTARTED), RoundScores{}, TotalTime(0), EndRoundTimer(0), _notEvenAScratch{}, ShipsStarted(false)
{
    BgObjects.resize(BG_SA_MAXOBJ);
    BgCreatures.resize(BG_SA_MAXNPC + BG_SA_MAX_GY);
    TimerEnabled = false;
    UpdateWaitTimer = 0;
    m_EndTimestamp = 0;
    SignaledRoundTwo = false;
    SignaledRoundTwoHalfMin = false;
    InitSecondRound = false;
    memset(&GraveyardStatus, 0, sizeof GraveyardStatus);
}

BattlegroundStrandOfTheAncients::~BattlegroundStrandOfTheAncients() = default;

void BattlegroundStrandOfTheAncients::Reset()
{
    TotalTime = 0;
    Attackers = urand(0, 1) ? TEAM_ALLIANCE : TEAM_HORDE;
    for (uint8 i = 0; i <= 5; i++)
        GateStatus[i] = BG_SA_GATE_OK;
    ShipsStarted = false;
    gateDestroyed = false;

    for (int8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        _notEvenAScratch[i] = true;

    Status = BG_SA_WARMUP;
}

Position const* BattlegroundStrandOfTheAncients::GetTeamStartPosition(TeamId teamId)
{
    if (teamId == Attackers)
    {
        Position pos1;
        Position pos2;
        if (teamId == TEAM_ALLIANCE)
        {
            pos1 = {1.2f, -0.0f, 9.5f, 2.895f};
            pos2 = {1.2f, -0.0f, 9.5f, 0.807f};
        }
        else
        {
            pos1 = {10.03f, 0.08f, 14.60f, 2.895f};
            pos2 = {10.03f, 0.08f, 14.60f, 0.807f};
        }

        bool onShip1 = false;
        bool onShip2 = false;
        // if (GetBGObject(BG_SA_BOAT_ONE))
        // {
            // GetBGObject(BG_SA_BOAT_ONE)->CalculatePassengerPosition(pos1.m_positionX, pos1.m_positionY, pos1.m_positionZ, &pos1.m_orientation);
            // onShip1 = true;
        // }
        // if (GetBGObject(BG_SA_BOAT_TWO))
        // {
            // GetBGObject(BG_SA_BOAT_TWO)->CalculatePassengerPosition(pos2.m_positionX, pos2.m_positionY, pos2.m_positionZ, &pos2.m_orientation);
            // onShip2 = true;
        // }

        if (urand(0, 1))
            m_TeamStartPos[teamId] = onShip1 ? pos1 : Position{ 2682.936f, -830.368f, 30.0f, 2.895f };
        else
            m_TeamStartPos[teamId] = onShip2 ? pos2 : Position{ 2577.003f, 980.261f, 30.0f, 0.807f };

        if (ShipsStarted && !onShip1)
            m_TeamStartPos[teamId] = { 1600.381f, -106.263f, 8.8745f, 3.78f };
    }
    else
        m_TeamStartPos[teamId] = { 1209.7f, -65.16f, 70.1f, 0.0f };

    return &m_TeamStartPos[teamId];
}

GateInfo const* BattlegroundStrandOfTheAncients::GetGate(uint32 entry)
{
    for (const auto& gate : Gates)
        if (gate.GameObjectId == entry)
            return &gate;

    return nullptr;
}

uint32 BattlegroundStrandOfTheAncients::getGateIdFromDamagedOrDestroyEventId(uint32 id)
{
    switch (id)
    {
            // Green gate
        case BG_SA_EVENT_GREEN_GATE_DAMAGED:
        case BG_SA_EVENT_GREEN_GATE_DESTROYED:
            return BG_SA_GREEN_GATE;
            // Blue gate
        case BG_SA_EVENT_BLUE_GATE_DAMAGED:
        case BG_SA_EVENT_BLUE_GATE_DESTROYED:
            return BG_SA_BLUE_GATE;
            // Red gate
        case BG_SA_EVENT_RED_GATE_DAMAGED:
        case BG_SA_EVENT_RED_GATE_DESTROYED:
            return BG_SA_RED_GATE;
            // Purple gate
        case BG_SA_EVENT_PURPLE_GATE_DAMAGED:
        case BG_SA_EVENT_PURPLE_GATE_DESTROYED:
            return BG_SA_PURPLE_GATE;
            // Yellow gate
        case BG_SA_EVENT_YELLOW_GATE_DAMAGED:
        case BG_SA_EVENT_YELLOW_GATE_DESTROYED:
            return BG_SA_YELLOW_GATE;
            // Ancient gate
        case BG_SA_EVENT_ANCIENT_GATE_DAMAGED:
        case BG_SA_EVENT_ANCIENT_GATE_DESTROYED:
            return BG_SA_ANCIENT_GATE;
        default:
            break;
    }
    return 0;
}

WorldStates BattlegroundStrandOfTheAncients::getWorldStateFromGateId(uint32 id)
{
    switch (id)
    {
        case BG_SA_GREEN_GATE:
            return WorldStates::BG_SA_GREEN_GATEWS;
        case BG_SA_YELLOW_GATE:
            return WorldStates::BG_SA_YELLOW_GATEWS;
        case BG_SA_BLUE_GATE:
            return WorldStates::BG_SA_BLUE_GATEWS;
        case BG_SA_RED_GATE:
            return WorldStates::BG_SA_RED_GATEWS;
        case BG_SA_PURPLE_GATE:
            return WorldStates::BG_SA_PURPLE_GATEWS;
        case BG_SA_ANCIENT_GATE:
            return WorldStates::BG_SA_ANCIENT_GATEWS;
        default:
            break;
    }
    return WorldStates::WS_NONE;
}

bool BattlegroundStrandOfTheAncients::SetupBattleground()
{
    return ResetObjs();
}

bool BattlegroundStrandOfTheAncients::ResetObjs()
{
    uint32 atF = BgFactions[Attackers];
    uint32 defF = BgFactions[Attackers ? TEAM_ALLIANCE : TEAM_HORDE];

    for (uint8 i = 0; i < BG_SA_MAXOBJ; i++)
        DelObject(i);

    for (uint8 i = 0; i < BG_SA_MAXNPC; i++)
        DelCreature(i);

    for (uint8 i = BG_SA_MAXNPC; i < BG_SA_MAXNPC + BG_SA_MAX_GY; i++)
        DelCreature(i);

    for (auto& gateStatus : GateStatus)
        gateStatus = BG_SA_GATE_OK;

    if (!AddCreature(BG_SA_NpcEntries[BG_SA_NPC_KANRETHAD], BG_SA_NPC_KANRETHAD, TEAM_NEUTRAL, BG_SA_NpcSpawnlocs[BG_SA_NPC_KANRETHAD][0], BG_SA_NpcSpawnlocs[BG_SA_NPC_KANRETHAD][1], BG_SA_NpcSpawnlocs[BG_SA_NPC_KANRETHAD][2], BG_SA_NpcSpawnlocs[BG_SA_NPC_KANRETHAD][3]))
        return false;

    for (uint8 i = 0; i < BG_SA_BOAT_ONE; i++)
        if (!AddObject(i, BG_SA_ObjEntries[i], BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY))
            return false;

    for (uint8 i = BG_SA_BOAT_ONE; i < BG_SA_SIGIL_1; i++)
    {
        uint32 boatid = 0;
        switch (i)
        {
            case BG_SA_BOAT_ONE:
                boatid = Attackers ? BG_SA_BOAT_ONE_H : BG_SA_BOAT_ONE_A;
                break;
            case BG_SA_BOAT_TWO:
                boatid = Attackers ? BG_SA_BOAT_TWO_H : BG_SA_BOAT_TWO_A;
                break;
            default:
                break;
        }

        if (!AddObject(i, boatid, BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2] + (Attackers ? -3.750f : 0), BG_SA_ObjSpawnlocs[i][3], 0.0f, 0.0f, 1.0f, -4.371139E-08f, RESPAWN_ONE_DAY))
            return false;
    }

    for (uint8 i = BG_SA_SIGIL_1; i < BG_SA_CENTRAL_FLAG; i++)
        if (!AddObject(i, BG_SA_ObjEntries[i], BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY))
            return false;

    // MAD props for Kiper for discovering those values - 4 hours of his work.
    GetBGObject(BG_SA_BOAT_ONE)->SetParentRotation(G3D::Quat(0.f, 0.f, 1.0f, 0.0002f));
    GetBGObject(BG_SA_BOAT_TWO)->SetParentRotation(G3D::Quat(0.f, 0.f, 1.0f, 0.00001f));
    SpawnBGObject(BG_SA_BOAT_ONE, RESPAWN_IMMEDIATELY);
    SpawnBGObject(BG_SA_BOAT_TWO, RESPAWN_IMMEDIATELY);
    GetBGObject(BG_SA_BOAT_ONE)->SetMoving(false);
    GetBGObject(BG_SA_BOAT_TWO)->SetMoving(false);

    //Cannons and demolishers - NPCs are spawned
    //By capturing GYs.
    for (uint8 i = 0; i < BG_SA_DEMOLISHER_3; i++)
        if (!AddCreature(BG_SA_NpcEntries[i], i, MS::Battlegrounds::GetOtherTeamID(Attackers), BG_SA_NpcSpawnlocs[i][0], BG_SA_NpcSpawnlocs[i][1], BG_SA_NpcSpawnlocs[i][2], BG_SA_NpcSpawnlocs[i][3], 600))
            return false;

    OverrideGunFaction();
    DemolisherStartState(true);

    for (uint8 i = 0; i <= BG_SA_TITAN_RELIC; i++)
    {
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
        if (GetBGObject(i))
            GetBGObject(i)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, defF);
    }

    GetBGObject(BG_SA_TITAN_RELIC)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, atF);
    GetBGObject(BG_SA_TITAN_RELIC)->Refresh();
    GetBGObject(BG_SA_TITAN_RELIC)->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);

    for (uint8 i = 0; i <= 5; i++)
        GateStatus[i] = BG_SA_GATE_OK;

    TotalTime = 0;
    ShipsStarted = false;

    //Graveyards!
    for (uint8 i = 0; i < BG_SA_MAX_GY; i++)
    {
        WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);
        if (!sg)
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "SOTA: Can't find GY entry %u", BG_SA_GYEntries[i]);
            return false;
        }

        if (i == BG_SA_BEACH_GY)
        {
            GraveyardStatus[i] = Attackers;
            AddSpiritGuide(i + BG_SA_MAXNPC, sg->Loc, Attackers);
        }
        else
        {
            GraveyardStatus[i] = MS::Battlegrounds::GetOtherTeamID(Attackers);
            if (!AddSpiritGuide(i + BG_SA_MAXNPC, sg->Loc, MS::Battlegrounds::GetOtherTeamID(Attackers)))
                TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "SOTA: couldn't spawn GY: %u", i);
        }
    }

    //GY capture points
    for (uint8 i = BG_SA_CENTRAL_FLAG; i < BG_SA_PORTAL_DEFFENDER_BLUE; i++)
    {
        AddObject(i, BG_SA_ObjEntries[i] - (Attackers == TEAM_ALLIANCE ? 1 : 0), BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);
        if (GetBGObject(i))
            GetBGObject(i)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, atF);
    }

    for (uint8 i = BG_SA_PORTAL_DEFFENDER_BLUE; i < BG_SA_BOMB; i++)
    {
        AddObject(i, BG_SA_ObjEntries[i], BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);
        if (GetBGObject(i))
            GetBGObject(i)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, defF);
    }

    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
    {
        AddObject(i, BG_SA_ObjEntries[BG_SA_BOMB], BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);
        if (GetBGObject(i))
        {
            GetBGObject(i)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, atF);
            GetBGObject(i)->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
        }
    }

    UpdateWorldState(WorldStates::BG_SA_RIGHT_GY_HORDE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    UpdateWorldState(WorldStates::BG_SA_LEFT_GY_HORDE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    UpdateWorldState(WorldStates::BG_SA_CENTER_GY_HORDE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);

    UpdateWorldState(WorldStates::BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    UpdateWorldState(WorldStates::BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    UpdateWorldState(WorldStates::BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);

    if (InitSecondRound)
    {
        for (auto const& itr : GetPlayers())
            if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
                SendBasicWorldStateUpdate(player);

        WorldPackets::Instance::StartTimer startTimer;
        startTimer.Type = WORLD_TIMER_TYPE_PVP;
        startTimer.TimeRemaining = Minutes(1);
        startTimer.TotalTime = Minutes(1);
        SendPacketToAll(startTimer.Write());
    }

    if (GetStatus() == STATUS_IN_PROGRESS)
        TeleportPlayers();
    return true;
}

void BattlegroundStrandOfTheAncients::HandleStartTimer(TimerType type)
{
    if (type != WORLD_TIMER_TYPE_PVP)
        return;

    WorldPackets::Instance::StartTimer startTimer;
    startTimer.Type = type;
    if (InitSecondRound)
    {
        startTimer.TimeRemaining = Minutes(1);
        startTimer.TotalTime = Minutes(1);
    }
    else if (GetCountdownTimer() >= Seconds(10) && GetElapsedTime() < Minutes(2))
    {
        SetCountdownTimer(Seconds(0));
        startTimer.TimeRemaining = Minutes(2) - std::chrono::duration_cast<Seconds>(GetElapsedTime());
        startTimer.TotalTime = Minutes(2);
    }
    SendPacketToAll(startTimer.Write());
}

void BattlegroundStrandOfTheAncients::StartShips()
{
    if (ShipsStarted)
        return;

    // DoorOpen(BG_SA_BOAT_ONE);
    // DoorOpen(BG_SA_BOAT_TWO);
    GetBGObject(BG_SA_BOAT_ONE)->SetMoving(true);
    GetBGObject(BG_SA_BOAT_TWO)->SetMoving(true);

    ShipsStarted = true;
}

void BattlegroundStrandOfTheAncients::PostUpdateImpl(uint32 diff)
{
    if (InitSecondRound)
    {
        if (UpdateWaitTimer < diff)
        {
            if (!SignaledRoundTwo)
            {
                SignaledRoundTwo = true;
                InitSecondRound = false;
                SendBroadcastText(BG_SA_TEXT_ROUND_TWO_START_ONE_MINUTE, CHAT_MSG_BG_SYSTEM_NEUTRAL);
            }
        }
        else
        {
            UpdateWaitTimer -= diff;
            return;
        }
    }
    TotalTime += diff;

    if (Status == BG_SA_WARMUP)
    {
        EndRoundTimer = BG_SA_ROUNDLENGTH;
        if (TotalTime >= BG_SA_WARMUPLENGTH)
        {
            if (Creature* c = GetBGCreature(BG_SA_NPC_KANRETHAD))
                SendChatMessage(c, TEXT_ROUND_STARTED);

            TotalTime = 0;
            m_EndTimestamp = time(nullptr) + BG_SA_ROUNDLENGTH / IN_MILLISECONDS;
            ToggleTimer();
            DemolisherStartState(false);
            Status = BG_SA_ROUND_ONE;
            StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, Attackers == TEAM_ALLIANCE ? 23748 : 21702);
            StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, Attackers == TEAM_ALLIANCE ? 23748 : 21702);
        }
        if (TotalTime >= BG_SA_BOAT_START)
            StartShips();
        return;
    }
    if (Status == BG_SA_SECOND_WARMUP)
    {
        if (RoundScores[0].time < BG_SA_ROUNDLENGTH)
            EndRoundTimer = RoundScores[0].time;
        else
            EndRoundTimer = BG_SA_ROUNDLENGTH;

        if (TotalTime >= 60000)
        {
            if (auto sender = GetBGCreature(BG_SA_NPC_KANRETHAD))
                SendChatMessage(sender, TEXT_ROUND_STARTED);

            TotalTime = 0;
            m_EndTimestamp = time(nullptr) + EndRoundTimer / IN_MILLISECONDS;
            ToggleTimer();
            DemolisherStartState(false);
            Status = BG_SA_ROUND_TWO;
            StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT, Attackers == TEAM_ALLIANCE ? 23748 : 21702);
            StartTimedAchievement(CRITERIA_TIMED_TYPE_EVENT2, Attackers == TEAM_ALLIANCE ? 23748 : 21702);
        }
        if (TotalTime >= 30000)
        {
            if (!SignaledRoundTwoHalfMin)
            {
                SignaledRoundTwoHalfMin = true;
                SendBroadcastText(BG_SA_TEXT_ROUND_TWO_START_HALF_MINUTE, CHAT_MSG_BG_SYSTEM_NEUTRAL);
            }
        }
        StartShips();
        return;
    }
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (Status == BG_SA_ROUND_ONE)
        {
            if (TotalTime >= BG_SA_ROUNDLENGTH)
            {
                RoundScores[0].winner = Attackers;
                RoundScores[0].time = BG_SA_ROUNDLENGTH;
                TotalTime = 0;
                m_EndTimestamp = 0;
                Status = BG_SA_SECOND_WARMUP;
                Attackers = MS::Battlegrounds::GetOtherTeamID(Attackers);
                UpdateWaitTimer = 5000;
                SignaledRoundTwo = false;
                SignaledRoundTwoHalfMin = false;
                InitSecondRound = true;
                ToggleTimer();
                ResetObjs();
                return;
            }
        }
        else if (Status == BG_SA_ROUND_TWO)
        {
            if (TotalTime >= EndRoundTimer)
            {
                RoundScores[1].time = BG_SA_ROUNDLENGTH;
                RoundScores[1].winner = MS::Battlegrounds::GetOtherTeamID(Attackers);

                m_EndTimestamp = 0;

                if (RoundScores[0].time == RoundScores[1].time)
                    EndBattleground(0);
                else if (RoundScores[0].time < RoundScores[1].time)
                    EndBattleground(MS::Battlegrounds::GetTeamByTeamId(RoundScores[0].winner));
                else
                    EndBattleground(MS::Battlegrounds::GetTeamByTeamId(RoundScores[1].winner));
                return;
            }
        }
        if (Status == BG_SA_ROUND_ONE || Status == BG_SA_ROUND_TWO)
        {
            UpdateDemolisherSpawns();
        }
    }
}

void BattlegroundStrandOfTheAncients::StartingEventCloseDoors()
{
}

void BattlegroundStrandOfTheAncients::StartingEventOpenDoors()
{
}

void BattlegroundStrandOfTheAncients::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    auto ally_attacks = uint32(Attackers == TEAM_ALLIANCE ? 1 : 0);
    auto horde_attacks = uint32(Attackers == TEAM_HORDE ? 1 : 0);

    packet.Worldstates.emplace_back(WorldStates::BG_SA_ANCIENT_GATEWS, GateStatus[BG_SA_ANCIENT_GATE]);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_YELLOW_GATEWS, GateStatus[BG_SA_YELLOW_GATE]);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_GREEN_GATEWS, GateStatus[BG_SA_GREEN_GATE]);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_BLUE_GATEWS, GateStatus[BG_SA_BLUE_GATE]);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_RED_GATEWS, GateStatus[BG_SA_RED_GATE]);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_PURPLE_GATEWS, GateStatus[BG_SA_PURPLE_GATE]);

    packet.Worldstates.emplace_back(WorldStates::BG_SA_HORDE_ATTACKS, horde_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_ALLY_ATTACKS, ally_attacks);

    //Time will be sent on first update...
    packet.Worldstates.emplace_back(WorldStates::BG_SA_RIGHT_GY_HORDE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_LEFT_GY_HORDE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_CENTER_GY_HORDE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);

    packet.Worldstates.emplace_back(WorldStates::BG_SA_HORDE_DEFENCE_TOKEN, ally_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_ALLIANCE_DEFENCE_TOKEN, horde_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_LEFT_ATT_TOKEN_HRD, horde_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_RIGHT_ATT_TOKEN_HRD, horde_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_RIGHT_ATT_TOKEN_ALL, ally_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_LEFT_ATT_TOKEN_ALL, ally_attacks);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_BONUS_TIMER, 0);

    packet.Worldstates.emplace_back(WorldStates::BG_SA_ENABLE_TIMER, TimerEnabled);
    packet.Worldstates.emplace_back(WorldStates::BG_SA_TIMER, TimerEnabled ? m_EndTimestamp : 0);
}

void BattlegroundStrandOfTheAncients::SendBasicWorldStateUpdate(Player* player)
{
    auto ally_attacks = uint32(Attackers == TEAM_ALLIANCE ? 1 : 0);
    auto horde_attacks = uint32(Attackers == TEAM_HORDE ? 1 : 0);

    player->SendUpdateWorldState(WorldStates::BG_SA_RIGHT_GY_HORDE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    player->SendUpdateWorldState(WorldStates::BG_SA_LEFT_GY_HORDE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    player->SendUpdateWorldState(WorldStates::BG_SA_CENTER_GY_HORDE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);

    player->SendUpdateWorldState(WorldStates::BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    player->SendUpdateWorldState(WorldStates::BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    player->SendUpdateWorldState(WorldStates::BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);

    player->SendUpdateWorldState(WorldStates::BG_SA_ALLY_ATTACKS, ally_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_HORDE_ATTACKS, horde_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_RIGHT_ATT_TOKEN_ALL, ally_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_LEFT_ATT_TOKEN_ALL, ally_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_RIGHT_ATT_TOKEN_HRD, horde_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_LEFT_ATT_TOKEN_HRD, horde_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_HORDE_DEFENCE_TOKEN, ally_attacks);
    player->SendUpdateWorldState(WorldStates::BG_SA_ALLIANCE_DEFENCE_TOKEN, horde_attacks);

    player->SendUpdateWorldState(WorldStates::BG_SA_PURPLE_GATEWS, GateStatus[BG_SA_PURPLE_GATE]);
    player->SendUpdateWorldState(WorldStates::BG_SA_RED_GATEWS, GateStatus[BG_SA_RED_GATE]);
    player->SendUpdateWorldState(WorldStates::BG_SA_BLUE_GATEWS, GateStatus[BG_SA_BLUE_GATE]);
    player->SendUpdateWorldState(WorldStates::BG_SA_GREEN_GATEWS, GateStatus[BG_SA_GREEN_GATE]);
    player->SendUpdateWorldState(WorldStates::BG_SA_YELLOW_GATEWS, GateStatus[BG_SA_YELLOW_GATE]);
    player->SendUpdateWorldState(WorldStates::BG_SA_ANCIENT_GATEWS, GateStatus[BG_SA_ANCIENT_GATE]);
}

void BattlegroundStrandOfTheAncients::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundSAScore(player->GetGUID(), player->GetBGTeamId());

    if (!ShipsStarted)
    {
        if (player->GetBGTeamId() == Attackers)
            player->CastSpell(player, 12438, true);//Without this player falls before boat loads...
    }
}

void BattlegroundStrandOfTheAncients::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (!player)
        return;

    player->RemoveAurasDueToSpell(68377);
}

void BattlegroundStrandOfTheAncients::TeleportPlayers()
{
    for (auto const& itr : GetPlayers())
    {
        if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
        {
            // should remove spirit of redemption
            if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

            if (!player->isAlive())
            {
                player->ResurrectPlayer(1.0f);
                player->SpawnCorpseBones();
            }

            player->RemoveAurasWithMechanic(IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK);
            player->ResetAllPowers();
            player->CombatStopWithPets(true);

            TeleportToEntrancePosition(player);
        }
    }
}

void BattlegroundStrandOfTheAncients::TeleportToEntrancePosition(Player* player)
{
    if (player->GetBGTeamId() == Attackers)
    {
        if (!ShipsStarted)
        {
            if (urand(0, 1))
                player->TeleportTo(607, 2682.936f, -830.368f, 30.0f, 2.895f, 0);
            else
                player->TeleportTo(607, 2577.003f, 980.261f, 30.0f, 0.807f, 0);
        }
        else
            player->TeleportTo(607, 1600.381f, -106.263f, 8.8745f, 3.78f, 0);
    }
    else
        player->TeleportTo(607, 1209.7f, -65.16f, 70.1f, 0.0f, 0);
}

void BattlegroundStrandOfTheAncients::EventPlayerDamagedGO(Player* player, GameObject* go, uint32 eventType)
{
    if (!go || !go->GetGOInfo())
        return;

    if (eventType == go->GetGOInfo()->destructibleBuilding.DamagedEvent)
    {
        uint32 i = getGateIdFromDamagedOrDestroyEventId(eventType);
        GateStatus[i] = BG_SA_GATE_DAMAGED;
        WorldStates uws = getWorldStateFromGateId(i);
        if (uws != WorldStates::WS_NONE)
            UpdateWorldState(uws, GateStatus[i]);
    }

    GateInfo const* gate = GetGate(go->GetEntry());
    if (!gate)
        return;

    if (eventType == go->GetGOInfo()->destructibleBuilding.DestroyedEvent)
        if (Creature* c = go->FindNearestCreature(NPC_WORLD_TRIGGER, 500.0f))
            SendChatMessage(c, gate->DestroyedText, player);

    if (eventType == go->GetGOInfo()->destructibleBuilding.DamagedEvent)
        if (Creature* c = go->FindNearestCreature(NPC_WORLD_TRIGGER, 500.0f))
            SendChatMessage(c, gate->DamagedText, player);
}

void BattlegroundStrandOfTheAncients::HandleKillUnit(Creature* creature, Player* killer)
{
    if (creature->GetEntry() == NPC_DEMOLISHER_SA)
    {
        UpdatePlayerScore(killer, SCORE_DESTROYED_DEMOLISHER, 1);
        _notEvenAScratch[Attackers] = false;
    }
}

void BattlegroundStrandOfTheAncients::OverrideGunFaction()
{
    if (!BgCreatures[0])
        return;

    for (uint8 i = BG_SA_GUN_1; i <= BG_SA_GUN_10; i++)
    {
        if (Creature* gun = GetBGCreature(i))
            gun->setFaction(BgFactions[Attackers ? TEAM_ALLIANCE : TEAM_HORDE]);
    }

    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_4; i++)
    {
        if (Creature* dem = GetBGCreature(i))
            dem->setFaction(BgFactions[Attackers]);
    }
}

void BattlegroundStrandOfTheAncients::DemolisherStartState(bool start)
{
    if (!BgCreatures[0])
        return;

    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_4; i++)
    {
        if (Creature* dem = GetBGCreature(i))
        {
            if (start)
            {
                dem->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                dem->SetVisible(false);
            }
            else
            {
                dem->SetVisible(true);
                dem->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
        }
    }

    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
    {
        if (GetBGObject(i))
            if (GetBGObject(i)->GetPositionZ() < 22.0f)
                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
    }
}

void BattlegroundStrandOfTheAncients::DestroyGate(Player* player, GameObject* go)
{
    uint32 i = getGateIdFromDamagedOrDestroyEventId(go->GetGOInfo()->destructibleBuilding.DestroyedEvent);
    if (!GateStatus[i])
        return;

    if (GameObject* g = GetBGObject(i))
    {
        if (g->GetGOValue()->Building.Health == 0)
        {
            GateStatus[i] = BG_SA_GATE_DESTROYED;
            WorldStates uws = getWorldStateFromGateId(i);
            if (uws != WorldStates::WS_NONE)
                UpdateWorldState(uws, GateStatus[i]);
            bool rewardHonor = true;
            gateDestroyed = true;
            switch (i)
            {
                case BG_SA_GREEN_GATE:
                    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
                    {
                        if (GetBGObject(i))
                            if ((GetBGObject(i)->GetPositionX() < 1371.0f && GetBGObject(i)->GetPositionZ() < 35.0f && GetBGObject(i)->GetPositionY() > 100.0f)
                                || (GetBGObject(i)->GetPositionX() < 1382.0f && GetBGObject(i)->GetPositionZ() < 35.0f && GetBGObject(i)->GetPositionY() < -100.0f))
                                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_BLUE_GATE:
                    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
                    {
                        if (GetBGObject(i))
                            if ((GetBGObject(i)->GetPositionX() < 1371.0f && GetBGObject(i)->GetPositionZ() < 35.0f && GetBGObject(i)->GetPositionY() > 100.0f)
                                || (GetBGObject(i)->GetPositionX() < 1382.0f && GetBGObject(i)->GetPositionZ() < 35.0f && GetBGObject(i)->GetPositionY() < -100.0f))
                                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_RED_GATE:
                    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
                    {
                        if (GetBGObject(i))
                            if (GetBGObject(i)->GetPositionZ() < 73.0f && GetBGObject(i)->GetPositionZ() > 68.0f)
                                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (GateStatus[BG_SA_PURPLE_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_PURPLE_GATE:
                    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
                    {
                        if (GetBGObject(i))
                            if (GetBGObject(i)->GetPositionZ() < 73.0f && GetBGObject(i)->GetPositionZ() > 68.0f)
                                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    if (GateStatus[BG_SA_RED_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_ANCIENT_GATE:
                    if (GetBGObject(BG_SA_TITAN_RELIC))
                        GetBGObject(BG_SA_TITAN_RELIC)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                case BG_SA_YELLOW_GATE:
                    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
                    {
                        if (GetBGObject(i))
                            if (GetBGObject(i)->GetPositionZ() < 87.0f && GetBGObject(i)->GetPositionZ() > 78.0f)
                                GetBGObject(i)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                    break;
                default:
                    break;
            }

            if (i < 5)
                DelObject(i + 9);
            UpdatePlayerScore(player, SCORE_DESTROYED_WALL, 1);
            if (rewardHonor)
                UpdatePlayerScore(player, SCORE_BONUS_HONOR, GetBonusHonorFromKill(1));
        }
    }
}

WorldSafeLocsEntry const* BattlegroundStrandOfTheAncients::GetClosestGraveYard(Player* player)
{
    uint32 safeloc = 0;
    float x, y, z;

    player->GetPosition(x, y, z);

    if (player->GetBGTeamId() == Attackers)
        safeloc = BG_SA_GYEntries[BG_SA_BEACH_GY];
    else
        safeloc = BG_SA_GYEntries[BG_SA_DEFENDER_LAST_GY];

    WorldSafeLocsEntry const * closest = sWorldSafeLocsStore.LookupEntry(safeloc);
    float nearest = sqrt((closest->Loc.X - x)*(closest->Loc.X - x) + (closest->Loc.Y - y)*(closest->Loc.Y - y) + (closest->Loc.Z - z)*(closest->Loc.Z - z));

    for (uint8 i = BG_SA_RIGHT_CAPTURABLE_GY; i < BG_SA_MAX_GY; i++)
    {
        if (GraveyardStatus[i] != player->GetBGTeamId())
            continue;

        WorldSafeLocsEntry const * ret = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);
        float dist = sqrt((ret->Loc.X - x)*(ret->Loc.X - x) + (ret->Loc.Y - y)*(ret->Loc.Y - y) + (ret->Loc.Z - z)*(ret->Loc.Z - z));
        if (dist < nearest)
        {
            closest = ret;
            nearest = dist;
        }
    }

    return closest;
}

void BattlegroundStrandOfTheAncients::EventPlayerClickedOnFlag(Player* Source, GameObject* target_obj, bool& canRemove)
{
    switch (target_obj->GetEntry())
    {
        case 191307:
        case 191308:
            if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                CaptureGraveyard(BG_SA_LEFT_CAPTURABLE_GY, Source);
            break;
        case 191305:
        case 191306:
            if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                CaptureGraveyard(BG_SA_RIGHT_CAPTURABLE_GY, Source);
            break;
        case 191310:
        case 191309:
            if ((GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED) && (GateStatus[BG_SA_RED_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_PURPLE_GATE] == BG_SA_GATE_DESTROYED))
                CaptureGraveyard(BG_SA_CENTRAL_CAPTURABLE_GY, Source);
            break;
        default:
            return;
    };
}

void BattlegroundStrandOfTheAncients::CaptureGraveyard(BG_SA_Graveyards i, Player* Source)
{
    if (GraveyardStatus[i] == Attackers)
        return;

    DelCreature(BG_SA_MAXNPC + i);
    GraveyardStatus[i] = Source->GetBGTeamId();
    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);
    if (!sg)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BattlegroundStrandOfTheAncients::CaptureGraveyard: non-existant GY entry: %u", BG_SA_GYEntries[i]);
        return;
    }

    AddSpiritGuide(i + BG_SA_MAXNPC, sg->Loc, GraveyardStatus[i]);
    uint32 npc = 0;
    uint32 flag = 0;

    switch (i)
    {
        case BG_SA_LEFT_CAPTURABLE_GY:
            flag = BG_SA_LEFT_FLAG;
            DelObject(flag);
            AddObject(flag, BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1), BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1], BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            npc = BG_SA_NPC_RIGSPARK;
            if (Creature* rigspark = AddCreature(BG_SA_NpcEntries[npc], npc, Attackers, BG_SA_NpcSpawnlocs[npc][0], BG_SA_NpcSpawnlocs[npc][1], BG_SA_NpcSpawnlocs[npc][2], BG_SA_NpcSpawnlocs[npc][3]))
                rigspark->AI()->Talk(TEXT_SPARKLIGHT_RIGSPARK_SPAWN);

            if (Creature* dem = AddCreature(BG_SA_NpcEntries[BG_SA_DEMOLISHER_4], BG_SA_DEMOLISHER_4, MS::Battlegrounds::GetOtherTeamID(Attackers), BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_4][0], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_4][1], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_4][2], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_4][3], 600))
                dem->setFaction(BgFactions[Attackers]);

            UpdateWorldState(WorldStates::BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0);
            UpdateWorldState(WorldStates::BG_SA_LEFT_GY_HORDE, GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1);

            if (Creature* c = Source->FindNearestCreature(NPC_WORLD_TRIGGER, 500.0f))
                SendChatMessage(c, Source->GetBGTeamId() == TEAM_ALLIANCE ? TEXT_WEST_GRAVEYARD_CAPTURED_A : TEXT_WEST_GRAVEYARD_CAPTURED_H, Source);
            break;
        case BG_SA_RIGHT_CAPTURABLE_GY:
            flag = BG_SA_RIGHT_FLAG;
            DelObject(flag);
            AddObject(flag, BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1), BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1], BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            npc = BG_SA_NPC_SPARKLIGHT;
            if (Creature* sparklight = AddCreature(BG_SA_NpcEntries[npc], npc, Attackers, BG_SA_NpcSpawnlocs[npc][0], BG_SA_NpcSpawnlocs[npc][1], BG_SA_NpcSpawnlocs[npc][2], BG_SA_NpcSpawnlocs[npc][3]))
                sparklight->AI()->Talk(TEXT_SPARKLIGHT_RIGSPARK_SPAWN);

            if (Creature* dem = AddCreature(BG_SA_NpcEntries[BG_SA_DEMOLISHER_3], BG_SA_DEMOLISHER_3, MS::Battlegrounds::GetOtherTeamID(Attackers), BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_3][0], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_3][1], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_3][2], BG_SA_NpcSpawnlocs[BG_SA_DEMOLISHER_3][3], 600))
                dem->setFaction(BgFactions[Attackers]);

            UpdateWorldState(WorldStates::BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0);
            UpdateWorldState(WorldStates::BG_SA_RIGHT_GY_HORDE, GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1);

            if (Creature* c = Source->FindNearestCreature(NPC_WORLD_TRIGGER, 500.0f))
                SendChatMessage(c, Source->GetBGTeamId() == TEAM_ALLIANCE ? TEXT_EAST_GRAVEYARD_CAPTURED_A : TEXT_EAST_GRAVEYARD_CAPTURED_H, Source);
            break;
        case BG_SA_CENTRAL_CAPTURABLE_GY:
            flag = BG_SA_CENTRAL_FLAG;
            DelObject(flag);
            AddObject(flag, BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1), BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1], BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            UpdateWorldState(WorldStates::BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0);
            UpdateWorldState(WorldStates::BG_SA_CENTER_GY_HORDE, GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1);

            if (Creature* c = Source->FindNearestCreature(NPC_WORLD_TRIGGER, 500.0f))
                SendChatMessage(c, Source->GetBGTeamId() == TEAM_ALLIANCE ? TEXT_SOUTH_GRAVEYARD_CAPTURED_A : TEXT_SOUTH_GRAVEYARD_CAPTURED_H, Source);
            break;
        default:
            ASSERT(false);
            break;
    };
}

void BattlegroundStrandOfTheAncients::EventPlayerUsedGO(Player* Source, GameObject* object)
{
    if (object->GetEntry() == BG_SA_ObjEntries[BG_SA_TITAN_RELIC] && GateStatus[BG_SA_ANCIENT_GATE] == BG_SA_GATE_DESTROYED && GateStatus[BG_SA_YELLOW_GATE] == BG_SA_GATE_DESTROYED && (GateStatus[BG_SA_PURPLE_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_RED_GATE] == BG_SA_GATE_DESTROYED) && (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED))
    {
        if (Source->GetBGTeamId() == Attackers)
        {
            if (Source->GetBGTeamId() == TEAM_ALLIANCE)
                SendBroadcastText(BG_SA_TEXT_ALLIANCE_CAPTURED_TITAN_PORTAL, CHAT_MSG_BG_SYSTEM_ALLIANCE);
            else 
                SendBroadcastText(BG_SA_TEXT_HORDE_CAPTURED_TITAN_PORTAL, CHAT_MSG_BG_SYSTEM_HORDE);

            if (Status == BG_SA_ROUND_ONE)
            {
                RoundScores[0].winner = Attackers;
                RoundScores[0].time = TotalTime;
                //Achievement Storm the Beach (1310)
                for (auto const& itr : GetPlayers())
                    if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
                        if (player->GetBGTeamId() == Attackers)
                            player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 65246);

                Attackers = MS::Battlegrounds::GetOtherTeamID(Attackers);
                Status = BG_SA_SECOND_WARMUP;
                TotalTime = 0;
                m_EndTimestamp = 0;
                ToggleTimer();
                if (Creature* c = GetBGCreature(BG_SA_NPC_KANRETHAD))
                    SendChatMessage(c, TEXT_ROUND_1_FINISHED);
                UpdateWaitTimer = 5000;
                SignaledRoundTwo = false;
                SignaledRoundTwoHalfMin = false;
                InitSecondRound = true;
                ResetObjs();
            }
            else if (Status == BG_SA_ROUND_TWO)
            {
                RoundScores[1].winner = Attackers;
                RoundScores[1].time = TotalTime;
                m_EndTimestamp = 0;
                ToggleTimer();
                //Achievement Storm the Beach (1310)
                for (auto const& itr : GetPlayers())
                    if (Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first))
                        if (player->GetBGTeamId() == Attackers && RoundScores[1].winner == Attackers)
                            player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 65246);

                if (RoundScores[0].time == RoundScores[1].time)
                    EndBattleground(0);
                else if (RoundScores[0].time < RoundScores[1].time)
                    EndBattleground(MS::Battlegrounds::GetTeamByTeamId(RoundScores[0].winner));
                else
                    EndBattleground(MS::Battlegrounds::GetTeamByTeamId(RoundScores[1].winner));
            }
        }
    }
}

void BattlegroundStrandOfTheAncients::ToggleTimer()
{
    TimerEnabled = !TimerEnabled;

    UpdateWorldState(WorldStates::BG_SA_ENABLE_TIMER, TimerEnabled);
    UpdateWorldState(WorldStates::BG_SA_TIMER, TimerEnabled ? m_EndTimestamp : 0);
}

void BattlegroundStrandOfTheAncients::EndBattleground(uint32 winner)
{
    Battleground::EndBattleground(winner);
}

void BattlegroundStrandOfTheAncients::UpdateDemolisherSpawns()
{
    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_4; i++)
    {
        if (!BgCreatures[i].IsEmpty())
        {
            if (Creature* Demolisher = GetBGCreature(i))
            {
                if (Demolisher->isDead())
                {
                    // Demolisher is not in list
                    if (DemoliserRespawnList.find(i) == DemoliserRespawnList.end())
                    {
                        DemoliserRespawnList[i] = getMSTime() + 30000;
                    }
                    else
                    {
                        if (DemoliserRespawnList[i] < getMSTime())
                        {
                            Demolisher->Relocate(BG_SA_NpcSpawnlocs[i][0], BG_SA_NpcSpawnlocs[i][1],
                                BG_SA_NpcSpawnlocs[i][2], BG_SA_NpcSpawnlocs[i][3]);

                            Demolisher->Respawn();
                            DemoliserRespawnList.erase(i);
                        }
                    }
                }
            }
        }
    }
}

void BattlegroundStrandOfTheAncients::SendTransportInit(Player* player)
{
    if (!BgObjects[BG_SA_BOAT_ONE].IsEmpty() || !BgObjects[BG_SA_BOAT_TWO].IsEmpty())
    {
        UpdateData transData(player->GetMapId());
        if (!BgObjects[BG_SA_BOAT_ONE].IsEmpty())
            GetBGObject(BG_SA_BOAT_ONE)->BuildCreateUpdateBlockForPlayer(&transData, player);
        if (!BgObjects[BG_SA_BOAT_TWO].IsEmpty())
            GetBGObject(BG_SA_BOAT_TWO)->BuildCreateUpdateBlockForPlayer(&transData, player);
        WorldPacket packet;
        if (transData.BuildPacket(&packet))
            player->GetSession()->SendPacket(&packet);
    }
}

void BattlegroundStrandOfTheAncients::SendTransportsRemove(Player* player)
{
    if (!BgObjects[BG_SA_BOAT_ONE].IsEmpty() || !BgObjects[BG_SA_BOAT_TWO].IsEmpty())
    {
        UpdateData transData(player->GetMapId());
        if (!BgObjects[BG_SA_BOAT_ONE].IsEmpty())
            GetBGObject(BG_SA_BOAT_ONE)->BuildOutOfRangeUpdateBlock(&transData);
        if (!BgObjects[BG_SA_BOAT_TWO].IsEmpty())
            GetBGObject(BG_SA_BOAT_TWO)->BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket packet;
        if (transData.BuildPacket(&packet))
            player->GetSession()->SendPacket(&packet);
    }
}

