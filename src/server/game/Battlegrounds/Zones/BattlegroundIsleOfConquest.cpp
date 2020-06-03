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

#include "Player.h"
#include "Battleground.h"
#include "BattlegroundIsleOfConquest.h"
#include "Language.h"
#include "WorldPacket.h"
#include "GameObject.h"
#include "CreatureAI.h"
#include "ObjectMgr.h"
#include "Vehicle.h"
#include "Transport.h"
#include "MoveSplineInit.h"
#include "WorldStatePackets.h"

enum ICBroadcastTexts
{
    BG_IC_TEXT_FRONT_GATE_HORDE_DESTROYED       = 35409,
    BG_IC_TEXT_FRONT_GATE_ALLIANCE_DESTROYED    = 35410,
    BG_IC_TEXT_WEST_GATE_HORDE_DESTROYED        = 35411,
    BG_IC_TEXT_WEST_GATE_ALLIANCE_DESTROYED     = 35412,
    BG_IC_TEXT_EAST_GATE_HORDE_DESTROYED        = 35413,
    BG_IC_TEXT_EAST_GATE_ALLIANCE_DESTROYED     = 35414
};

struct ICNodeInfo
{
    uint32 NodeId;
    uint32 TextAssaulted;
    uint32 TextDefended;
    uint32 TextAllianceTaken;
    uint32 TextHordeTaken;
};

ICNodeInfo const ICNodes[MAX_NODE_TYPES] =
{
    { NODE_TYPE_REFINERY,    35377, 35378, 35379, 35380 },
    { NODE_TYPE_QUARRY,      35373, 35374, 35375, 35376 },
    { NODE_TYPE_DOCKS,       35365, 35366, 35367, 35368 },
    { NODE_TYPE_HANGAR,      35369, 35370, 35371, 35372 },
    { NODE_TYPE_WORKSHOP,    35278, 35286, 35279, 35280 },
    { NODE_TYPE_GRAVEYARD_A, 35461, 35459, 35463, 35466 },
    { NODE_TYPE_GRAVEYARD_H, 35462, 35460, 35464, 35465 }
};

BattlegroundIsleOfConquest::BattlegroundIsleOfConquest(): docksTimer(0)
{
    BgObjects.resize(MAX_NORMAL_GAMEOBJECTS_SPAWNS + MAX_AIRSHIPS_SPAWNS + MAX_HANGAR_TELEPORTERS_SPAWNS + MAX_FORTRESS_TELEPORTERS_SPAWNS + MAX_HANGAR_TELEPORTER_EFFECTS_SPAWNS);
    BgCreatures.resize(MAX_NORMAL_NPCS_SPAWNS + MAX_WORKSHOP_SPAWNS + MAX_DOCKS_SPAWNS + MAX_SPIRIT_GUIDES_SPAWNS + MAX_CAPTAIN_SPAWNS_PER_FACTION + MAX_TRIGGER_SPAWNS_PER_FACTION);

    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
        factionReinforcements[i] = MAX_REINFORCEMENTS;

    for (uint8 i = 0; i < BG_IC_MAXDOOR; i++)
        GateStatus[i] = BG_IC_GATE_OK;

    closeFortressDoorsTimer = CLOSE_DOORS_TIME; // the doors are closed again... in a special way
    doorsClosed = false;
    resourceTimer = IC_RESOURCE_TIME;

    for (uint8 i = NODE_TYPE_REFINERY; i < MAX_NODE_TYPES; i++)
        nodePoint[i] = nodePointInitial[i];

    siegeEngineWorkshopTimer = WORKSHOP_UPDATE_TIME;

    gunshipHorde = nullptr;
    gunshipAlliance = nullptr;
}

BattlegroundIsleOfConquest::~BattlegroundIsleOfConquest() = default;

void BattlegroundIsleOfConquest::HandlePlayerResurrect(Player* player)
{
    if (nodePoint[NODE_TYPE_QUARRY].nodeState == (player->GetBGTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H))
        player->CastSpell(player, SPELL_QUARRY, true);

    if (nodePoint[NODE_TYPE_REFINERY].nodeState == (player->GetBGTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H))
        player->CastSpell(player, SPELL_OIL_REFINERY, true);
}

void BattlegroundIsleOfConquest::DoAction(uint32 action, ObjectGuid guid)
{
    if (action != ACTION_TELEPORT_PLAYER_TO_TRANSPORT)
        return;

    Player* player = ObjectAccessor::FindPlayer(GetBgMap(), guid);

    if (!player || !gunshipAlliance || !gunshipHorde)
        return;

    float x, y, z, o = 0.0f;
    if (player->GetBGTeamId() == TEAM_ALLIANCE)
    {
        x = BG_IC_HangarTrigger[0].GetPositionX();
        y = BG_IC_HangarTrigger[0].GetPositionY();
        z = BG_IC_HangarTrigger[0].GetPositionZ();
        o = BG_IC_HangarTrigger[0].GetOrientation();
        gunshipAlliance->CalculatePassengerPosition(x, y, z, &o);
    }
    else
    {
        x = BG_IC_HangarTrigger[1].GetPositionX();
        y = BG_IC_HangarTrigger[1].GetPositionY();
        z = BG_IC_HangarTrigger[1].GetPositionZ();
        o = BG_IC_HangarTrigger[1].GetOrientation();
        gunshipHorde->CalculatePassengerPosition(x, y, z, &o);
    }

    player->NearTeleportTo(x, y, z, o);
}

void BattlegroundIsleOfConquest::PostUpdateImpl(uint32 diff)
{

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!doorsClosed)
    {
        if (closeFortressDoorsTimer <= diff)
        {
            GetBGObject(BG_IC_GO_DOODAD_ND_HUMAN_GATE_CLOSEDFX_DOOR01)->RemoveFromWorld();
            GetBGObject(BG_IC_GO_DOODAD_ND_WINTERORC_WALL_GATEFX_DOOR01)->RemoveFromWorld();

            GetBGObject(BG_IC_GO_ALLIANCE_GATE_1)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            GetBGObject(BG_IC_GO_HORDE_GATE_1)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            GetBGObject(BG_IC_GO_ALLIANCE_GATE_2)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            GetBGObject(BG_IC_GO_HORDE_GATE_2)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            GetBGObject(BG_IC_GO_ALLIANCE_GATE_3)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            GetBGObject(BG_IC_GO_HORDE_GATE_3)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);

            doorsClosed = true;
        } else closeFortressDoorsTimer -= diff;
    }

    for (uint8 i = NODE_TYPE_REFINERY; i < MAX_NODE_TYPES; i++)
    {
        if (nodePoint[i].nodeType == NODE_TYPE_DOCKS)
        {
            if (nodePoint[i].nodeState == NODE_STATE_CONTROLLED_A ||
                nodePoint[i].nodeState == NODE_STATE_CONTROLLED_H)
            {
                if (docksTimer <= diff)
                {
                    // we need to confirm this, i am not sure if this every 3 minutes
                    for (uint8 u = (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_CATAPULT_1_A : BG_IC_NPC_CATAPULT_1_H); u < (nodePoint[i].faction  == TEAM_ALLIANCE ? BG_IC_NPC_CATAPULT_4_A : BG_IC_NPC_CATAPULT_4_H); u++)
                    {
                        if (Creature* catapult = GetBGCreature(u))
                        {
                            if (!catapult->isAlive())
                                catapult->Respawn(true);
                        }
                    }

                    // we need to confirm this is blizzlike, not sure if it is every 3 minutes
                    for (uint8 u = (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_GLAIVE_THROWER_1_A : BG_IC_NPC_GLAIVE_THROWER_1_H); u < (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_GLAIVE_THROWER_2_A : BG_IC_NPC_GLAIVE_THROWER_2_H); u++)
                    {
                        if (Creature* glaiveThrower = GetBGCreature(u))
                        {
                            if (!glaiveThrower->isAlive())
                                glaiveThrower->Respawn(true);
                        }
                    }

                    docksTimer = DOCKS_UPDATE_TIME;
                } else docksTimer -= diff;
            }
        }

        if (nodePoint[i].nodeType == NODE_TYPE_WORKSHOP)
        {
            if (nodePoint[i].nodeState == NODE_STATE_CONTROLLED_A ||
                nodePoint[i].nodeState == NODE_STATE_CONTROLLED_H)
            {
                if (siegeEngineWorkshopTimer <= diff)
                {
                    uint8 siegeType = (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_SIEGE_ENGINE_A : BG_IC_NPC_SIEGE_ENGINE_H);

                    if (Creature* siege = GetBGCreature(siegeType)) // this always should be true
                    {
                        if (siege->isAlive())
                        {
                            if (siege->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE| UNIT_FLAG_CANNOT_SWIM |UNIT_FLAG_IMMUNE_TO_PC))
                                // following sniffs the vehicle always has UNIT_FLAG_CANNOT_SWIM
                                siege->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE|UNIT_FLAG_IMMUNE_TO_PC);
                            else
                                siege->SetHealth(siege->GetMaxHealth());
                        }
                        else
                            siege->Respawn(true);
                    }

                    // we need to confirm if it is every 3 minutes
                    for (uint8 u = (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_DEMOLISHER_1_A : BG_IC_NPC_DEMOLISHER_1_H); u < (nodePoint[i].faction == TEAM_ALLIANCE ? BG_IC_NPC_DEMOLISHER_4_A : BG_IC_NPC_DEMOLISHER_4_H); u++)
                    {
                        if (Creature* demolisher = GetBGCreature(u))
                        {
                            if (!demolisher->isAlive())
                                demolisher->Respawn(true);
                        }
                    }
                    siegeEngineWorkshopTimer = WORKSHOP_UPDATE_TIME;
                } else siegeEngineWorkshopTimer -= diff;
            }
        }

        // the point is waiting for a change on its banner
        if (nodePoint[i].needChange)
        {
            if (nodePoint[i].timer <= diff)
            {
                uint32 nextBanner = GetNextBanner(&nodePoint[i], nodePoint[i].faction, true);

                nodePoint[i].last_entry = nodePoint[i].gameobject_entry;
                nodePoint[i].gameobject_entry = nextBanner;
                // nodePoint[i].faction = the faction should be the same one...

                GameObject* banner = GetBGObject(nodePoint[i].gameobject_type);

                if (!banner) // this should never happen
                    return;

                float cords[4] = {banner->GetPositionX(), banner->GetPositionY(), banner->GetPositionZ(), banner->GetOrientation() };

                DelObject(nodePoint[i].gameobject_type);
                AddObject(nodePoint[i].gameobject_type, nodePoint[i].gameobject_entry, cords[0], cords[1], cords[2], cords[3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

                GetBGObject(nodePoint[i].gameobject_type)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, nodePoint[i].faction == TEAM_ALLIANCE ? BgFactions[1] : BgFactions[0]);

                UpdateNodeWorldState(&nodePoint[i]);
                HandleCapturedNodes(&nodePoint[i], false);

                uint8 team = nodePoint[i].faction;

                switch (nodePoint[i].nodeType)
                {
                    case NODE_TYPE_REFINERY:
                        SendBroadcastText(team == TEAM_ALLIANCE ? 35379 : 35380, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE);
                        break;
                    case NODE_TYPE_QUARRY:
                        SendBroadcastText(team == TEAM_ALLIANCE ? 35375 : 35376, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE);
                        break;
                    case NODE_TYPE_DOCKS:
                        SendBroadcastText(team == TEAM_ALLIANCE ? 35367 : 35368, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE);
                        break;
                    case NODE_TYPE_HANGAR:
                        SendBroadcastText(team == TEAM_ALLIANCE ? 35371 : 35372, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE);
                        break;
                    case NODE_TYPE_WORKSHOP:
                        SendBroadcastText(team == TEAM_ALLIANCE ? 35279 : 35280, team == TEAM_ALLIANCE ? CHAT_MSG_BG_SYSTEM_ALLIANCE : CHAT_MSG_BG_SYSTEM_HORDE);
                        break;
                    default:
                        break;
                }

                if (nodePoint[i].faction == TEAM_ALLIANCE)
                    SendBroadcastText(ICNodes[i].TextAllianceTaken, CHAT_MSG_BG_SYSTEM_ALLIANCE);
                else
                    SendBroadcastText(ICNodes[i].TextHordeTaken, CHAT_MSG_BG_SYSTEM_HORDE);

                nodePoint[i].needChange = false;
                nodePoint[i].timer = BANNER_STATE_CHANGE_TIME;
            } else nodePoint[i].timer -= diff;
        }
    }

    if (resourceTimer <= diff)
    {
        for (uint8 i = 0; i < NODE_TYPE_DOCKS; i++)
        {
            if (nodePoint[i].nodeState == NODE_STATE_CONTROLLED_A ||
                nodePoint[i].nodeState == NODE_STATE_CONTROLLED_H)
            {
                factionReinforcements[nodePoint[i].faction] += 1;
                RewardHonorToTeam(RESOURCE_HONOR_AMOUNT, MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(nodePoint[i].faction)));
                UpdateWorldState((nodePoint[i].faction == TEAM_ALLIANCE ? WorldStates::BG_IC_ALLIANCE_RENFORT : WorldStates::BG_IC_HORDE_RENFORT), factionReinforcements[nodePoint[i].faction]);
            }
        }
        resourceTimer = IC_RESOURCE_TIME;
    } else resourceTimer -= diff;
}

void BattlegroundIsleOfConquest::StartingEventCloseDoors()
{
    GetBGObject(BG_IC_GO_ALLIANCE_GATE_1)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
    GetBGObject(BG_IC_GO_HORDE_GATE_1)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
    GetBGObject(BG_IC_GO_ALLIANCE_GATE_2)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
    GetBGObject(BG_IC_GO_HORDE_GATE_2)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
    GetBGObject(BG_IC_GO_ALLIANCE_GATE_3)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
    GetBGObject(BG_IC_GO_HORDE_GATE_3)->SetDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
}

void BattlegroundIsleOfConquest::StartingEventOpenDoors()
{
    //after 20 seconds they should be despawned
    DoorsOpen(BG_IC_GO_DOODAD_ND_HUMAN_GATE_CLOSEDFX_DOOR01, BG_IC_GO_DOODAD_ND_WINTERORC_WALL_GATEFX_DOOR01);
    DoorsOpen(BG_IC_GO_DOODAD_HU_PORTCULLIS01_1, BG_IC_GO_DOODAD_VR_PORTCULLIS01_1);
    DoorsOpen(BG_IC_GO_DOODAD_HU_PORTCULLIS01_2, BG_IC_GO_DOODAD_VR_PORTCULLIS01_2);

    for (uint8 i = 0; i < MAX_FORTRESS_TELEPORTERS_SPAWNS; i++)
    {
        if (!AddObject(BG_IC_Teleporters[i].type, BG_IC_Teleporters[i].entry,
            BG_IC_Teleporters[i].x, BG_IC_Teleporters[i].y,
            BG_IC_Teleporters[i].z, BG_IC_Teleporters[i].o,
            0, 0, 0, 0, RESPAWN_ONE_DAY))
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest | Starting Event Open Doors: There was an error spawning gameobject %u", BG_IC_Teleporters[i].entry);
    }
}

bool BattlegroundIsleOfConquest::IsAllNodesConrolledByTeam(uint32 team) const
{
    uint32 count = 0;
    ICNodeState controlledState = team == ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H;
    for (int i = 0; i < NODE_TYPE_WORKSHOP; ++i)
    {
        if (nodePoint[i].nodeState == controlledState)
            count++;
    }

    return count == NODE_TYPE_WORKSHOP;
}

void BattlegroundIsleOfConquest::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundICScore(player->GetGUID(), player->GetBGTeamId());

    if (nodePoint[NODE_TYPE_QUARRY].nodeState == (player->GetBGTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H))
        player->CastSpell(player, SPELL_QUARRY, true);

    if (nodePoint[NODE_TYPE_REFINERY].nodeState == (player->GetBGTeamId() == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H))
        player->CastSpell(player, SPELL_OIL_REFINERY, true);
}

void BattlegroundIsleOfConquest::RemovePlayer(Player* player, ObjectGuid /*guid*/, uint32 /*team*/)
{
    if (player)
    {
        player->RemoveAura(SPELL_QUARRY);
        player->RemoveAura(SPELL_OIL_REFINERY);
    }
}

void BattlegroundIsleOfConquest::_CheckPositions(uint32 diff)
{
    for (auto const& itr : GetPlayers())
    {
        Player* player = ObjectAccessor::FindPlayer(GetBgMap(), itr.first);
        if (!player)
            continue;

        if (player->IsInAreaTriggerRadius(5555) && player->GetBGTeamId() == TEAM_HORDE && GetStatus() == STATUS_IN_PROGRESS)
            if (GateStatus[BG_IC_A_FRONT] != BG_IC_GATE_DESTROYED && GateStatus[BG_IC_A_WEST] != BG_IC_GATE_DESTROYED && GateStatus[BG_IC_A_EAST] != BG_IC_GATE_DESTROYED)
            {
                player->CastSpell(player, SPELL_BACK_DOOR_JOB_ACHIEVEMENT, true);
                break;
            }

        if (player->IsInAreaTriggerRadius(5535) && player->GetBGTeamId() == TEAM_ALLIANCE && GetStatus() == STATUS_IN_PROGRESS)
            if (GateStatus[BG_IC_H_FRONT] != BG_IC_GATE_DESTROYED && GateStatus[BG_IC_H_WEST] != BG_IC_GATE_DESTROYED && GateStatus[BG_IC_H_EAST] != BG_IC_GATE_DESTROYED)
            {
                player->CastSpell(player, SPELL_BACK_DOOR_JOB_ACHIEVEMENT, true);
                break;
            }
    }

    Battleground::_CheckPositions(diff);
}

bool BattlegroundIsleOfConquest::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor /*= true*/)
{
    if (!Battleground::UpdatePlayerScore(player, type, value, doAddHonor))
        return false;

    switch (type)
    {
        case SCORE_BASES_ASSAULTED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, IC_OBJECTIVE_ASSAULT_BASE, 1);
            break;
        case SCORE_BASES_DEFENDED:
            player->UpdateAchievementCriteria(CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, IC_OBJECTIVE_DEFEND_BASE, 1);
            break;
        default:
            break;
    }
    
    return true;
}

void BattlegroundIsleOfConquest::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::BG_IC_ALLIANCE_RENFORT_SET, 1);
    packet.Worldstates.emplace_back(WorldStates::BG_IC_HORDE_RENFORT_SET, 1);
    packet.Worldstates.emplace_back(WorldStates::BG_IC_ALLIANCE_RENFORT, factionReinforcements[TEAM_ALLIANCE]);
    packet.Worldstates.emplace_back(WorldStates::BG_IC_HORDE_RENFORT, factionReinforcements[TEAM_HORDE]);

    for (uint8 i = 0; i < MAX_FORTRESS_GATES_SPAWNS; i++)
    {
        WorldStates uws = GetWorldStateFromGateEntry(BG_IC_ObjSpawnlocs[i].entry, (GateStatus[GetGateIDFromEntry(BG_IC_ObjSpawnlocs[i].entry)] == BG_IC_GATE_DESTROYED ? true : false));
        packet.Worldstates.emplace_back(uws, 1);
    }

    for (uint8 i = 0; i < MAX_NODE_TYPES; i++)
        packet.Worldstates.emplace_back(nodePoint[i].worldStates[nodePoint[i].nodeState], 1);
}

bool BattlegroundIsleOfConquest::SetupBattleground()
{
    for (uint8 i = 0; i < MAX_NORMAL_GAMEOBJECTS_SPAWNS; i++)
    {
        if (!AddObject(BG_IC_ObjSpawnlocs[i].type, BG_IC_ObjSpawnlocs[i].entry,
            BG_IC_ObjSpawnlocs[i].x, BG_IC_ObjSpawnlocs[i].y,
            BG_IC_ObjSpawnlocs[i].z, BG_IC_ObjSpawnlocs[i].o,
            0, 0, 0, 0, RESPAWN_ONE_DAY))
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: There was an error spawning gameobject %u", BG_IC_ObjSpawnlocs[i].entry);
            return false;
        }
    }

    for (uint8 i = 2; i < MAX_NORMAL_NPCS_SPAWNS; i++)
    {
        if (!AddCreature(BG_IC_NpcSpawnlocs[i].entry, BG_IC_NpcSpawnlocs[i].type, BG_IC_NpcSpawnlocs[i].team,
            BG_IC_NpcSpawnlocs[i].x, BG_IC_NpcSpawnlocs[i].y,
            BG_IC_NpcSpawnlocs[i].z, BG_IC_NpcSpawnlocs[i].o,
            RESPAWN_ONE_DAY))
        {
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: There was an error spawning creature %u", BG_IC_NpcSpawnlocs[i].entry);
            return false;
        }
    }

    if (!AddSpiritGuide(BG_IC_NPC_SPIRIT_GUIDE_1 + 3, BG_IC_SpiritGuidePos[5], TEAM_ALLIANCE) ||
        !AddSpiritGuide(BG_IC_NPC_SPIRIT_GUIDE_1 + 4, BG_IC_SpiritGuidePos[6], TEAM_HORDE) ||
        !AddSpiritGuide(BG_IC_NPC_SPIRIT_GUIDE_1 + 5, BG_IC_SpiritGuidePos[7], TEAM_ALLIANCE) ||
        !AddSpiritGuide(BG_IC_NPC_SPIRIT_GUIDE_1 + 6, BG_IC_SpiritGuidePos[8], TEAM_HORDE))
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: Failed to spawn initial spirit guide!");
        return false;
    }

    gunshipHorde = sTransportMgr->GetTransport(GetBgMap(), GO_HORDE_GUNSHIP);
    gunshipAlliance = sTransportMgr->GetTransport(GetBgMap(), GO_ALLIANCE_GUNSHIP);

    if (!gunshipAlliance || !gunshipHorde)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: There was an error creating gunships!");
        return false;
    }

    // setting correct factions for Keep Cannons
    for (uint8 i = BG_IC_NPC_KEEP_CANNON_1; i < BG_IC_NPC_KEEP_CANNON_12; i++)
        GetBGCreature(i)->setFaction(BgFactions[0]);
    for (uint8 i = BG_IC_NPC_KEEP_CANNON_13; i < BG_IC_NPC_KEEP_CANNON_25; i++)
        GetBGCreature(i)->setFaction(BgFactions[1]);

    // correcting spawn time for keeps bombs
    for (uint8 i = BG_IC_GO_HUGE_SEAFORIUM_BOMBS_A_1; i < BG_IC_GO_HUGE_SEAFORIUM_BOMBS_H_4; i++)
        GetBGObject(i)->SetRespawnTime(10);

    return true;
}

void BattlegroundIsleOfConquest::HandleKillUnit(Creature* unit, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
       return;

    uint32 entry = unit->GetEntry();
    if (entry == NPC_HIGH_COMMANDER_HALFORD_WYRMBANE)
    {
        RewardHonorToTeam(WINNER_HONOR_AMOUNT, HORDE);
        EndBattleground(HORDE);
    }
    else if (entry == NPC_OVERLORD_AGMAR)
    {
        RewardHonorToTeam(WINNER_HONOR_AMOUNT, ALLIANCE);
        EndBattleground(ALLIANCE);
    }

    //Achievement Mowed Down
    // TO-DO: This should be done on the script of each vehicle of the BG.
    if (unit->IsVehicle())
        killer->CastSpell(killer, SPELL_DESTROYED_VEHICLE_ACHIEVEMENT, true);
}

void BattlegroundIsleOfConquest::HandleKillPlayer(Player* player, Player* killer)
{
    if (!player)
        return;
        
    TeamId teamID = player->GetBGTeamId();
    if (GetStatus() != STATUS_IN_PROGRESS || teamID >= MAX_TEAMS)
        return;

    Battleground::HandleKillPlayer(player, killer);

    factionReinforcements[teamID] -= 1;

    UpdateWorldState((teamID == TEAM_ALLIANCE ? WorldStates::BG_IC_ALLIANCE_RENFORT : WorldStates::BG_IC_HORDE_RENFORT), factionReinforcements[teamID]);

    // we must end the battleground
    if (factionReinforcements[teamID] < TEAM_HORDE)
        EndBattleground(killer->GetBGTeam());
}

void BattlegroundIsleOfConquest::EventPlayerClickedOnFlag(Player* player, GameObject* target_obj, bool& canRemove)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    // All the node points are iterated to find the clicked one
    for (uint8 i = 0; i < MAX_NODE_TYPES; i++)
    {
        if (nodePoint[i].gameobject_entry == target_obj->GetEntry())
        {
            // THIS SHOULD NEEVEER HAPPEN
            if (nodePoint[i].faction == player->GetBGTeamId())
                return;

            uint32 nextBanner = GetNextBanner(&nodePoint[i], player->GetBGTeamId(), false);

            // we set the new settings of the nodePoint
            nodePoint[i].faction = player->GetBGTeamId();
            nodePoint[i].last_entry = nodePoint[i].gameobject_entry;
            nodePoint[i].gameobject_entry = nextBanner;

            // this is just needed if the next banner is grey
            if (nodePoint[i].banners[BANNER_A_CONTESTED] == nextBanner || nodePoint[i].banners[BANNER_H_CONTESTED] == nextBanner)
            {
                nodePoint[i].timer = BANNER_STATE_CHANGE_TIME; // 1 minute for last change (real faction banner)
                nodePoint[i].needChange = true;

                RelocateDeadPlayers(BgCreatures[BG_IC_NPC_SPIRIT_GUIDE_1 + nodePoint[i].nodeType - 2]);

                // if we are here means that the point has been lost, or it is the first capture

                if (nodePoint[i].nodeType != NODE_TYPE_REFINERY && nodePoint[i].nodeType != NODE_TYPE_QUARRY)
                    if (!BgCreatures[BG_IC_NPC_SPIRIT_GUIDE_1+(nodePoint[i].nodeType)-2].IsEmpty())
                        DelCreature(BG_IC_NPC_SPIRIT_GUIDE_1+(nodePoint[i].nodeType)-2);

                UpdatePlayerScore(player, SCORE_BASES_ASSAULTED, 1);

                if (nodePoint[i].faction == TEAM_ALLIANCE)
                    SendBroadcastText(ICNodes[i].TextAssaulted, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
                else
                    SendBroadcastText(ICNodes[i].TextAssaulted, CHAT_MSG_BG_SYSTEM_HORDE, player);

                HandleContestedNodes(&nodePoint[i]);
            } else if (nextBanner == nodePoint[i].banners[BANNER_A_CONTROLLED] || nextBanner == nodePoint[i].banners[BANNER_H_CONTROLLED])
            {
                nodePoint[i].timer = BANNER_STATE_CHANGE_TIME;
                nodePoint[i].needChange = false;
                if (nodePoint[i].faction == TEAM_ALLIANCE)

                    SendBroadcastText(ICNodes[i].TextDefended, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
                else
                    SendBroadcastText(ICNodes[i].TextDefended, CHAT_MSG_BG_SYSTEM_HORDE, player);
                
                HandleCapturedNodes(&nodePoint[i], true);
                UpdatePlayerScore(player, SCORE_BASES_DEFENDED, 1);
            }

            GameObject* banner = GetBGObject(nodePoint[i].gameobject_type);

            if (!banner) // this should never happen
                return;

            float cords[4] = {banner->GetPositionX(), banner->GetPositionY(), banner->GetPositionZ(), banner->GetOrientation() };

            DelObject(nodePoint[i].gameobject_type);
            AddObject(nodePoint[i].gameobject_type, nodePoint[i].gameobject_entry, cords[0], cords[1], cords[2], cords[3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            GetBGObject(nodePoint[i].gameobject_type)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, nodePoint[i].faction == TEAM_ALLIANCE ? BgFactions[1] : BgFactions[0]);

            if (nodePoint[i].nodeType == NODE_TYPE_WORKSHOP)
            {
                DelObject(BG_IC_GO_SEAFORIUM_BOMBS_1);
                DelObject(BG_IC_GO_SEAFORIUM_BOMBS_2);
            }

            UpdateNodeWorldState(&nodePoint[i]);
            // we dont need iterating if we are here
            // If the needChange bool was set true, we will handle the rest in the Update Map function.
            return;
        }
    }
}

void BattlegroundIsleOfConquest::UpdateNodeWorldState(ICNodePoint* nodePoint)
{
    //updating worldstate
    if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_A_CONTROLLED])
        nodePoint->nodeState = NODE_STATE_CONTROLLED_A;
    else if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_A_CONTESTED])
        nodePoint->nodeState = NODE_STATE_CONFLICT_A;
    else if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_H_CONTROLLED])
        nodePoint->nodeState = NODE_STATE_CONTROLLED_H;
    else if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_H_CONTESTED])
        nodePoint->nodeState = NODE_STATE_CONFLICT_H;

    WorldStates worldstate = nodePoint->worldStates[nodePoint->nodeState];

    // with this we are sure we dont bug the client
    for (uint8 i = 0; i < 4; i++)
        UpdateWorldState(nodePoint->worldStates[i], 0);

    UpdateWorldState(worldstate, 1);
}

uint32 BattlegroundIsleOfConquest::GetNextBanner(ICNodePoint* nodePoint, uint32 team, bool returnDefinitve)
{
    // this is only used in the update map function
    if (returnDefinitve)
        // here is a special case, here we must return the definitve faction banner after the grey banner was spawned 1 minute
        return nodePoint->banners[(team == TEAM_ALLIANCE ? BANNER_A_CONTROLLED : BANNER_H_CONTROLLED)];

    // there were no changes, this point has never been captured by any faction or at least clicked
    if (nodePoint->last_entry == 0)
        // 1 returns the CONTESTED ALLIANCE BANNER, 3 returns the HORDE one
        return nodePoint->banners[(team == TEAM_ALLIANCE ? BANNER_A_CONTESTED : BANNER_H_CONTESTED)];

    // If the actual banner is the definitive faction banner, we must return the grey banner of the player's faction
    if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_A_CONTROLLED] || nodePoint->gameobject_entry == nodePoint->banners[BANNER_H_CONTROLLED])
        return nodePoint->banners[(team == TEAM_ALLIANCE ? BANNER_A_CONTESTED : BANNER_H_CONTESTED)];

    // If the actual banner is the grey faction banner, we must return the previous banner
    if (nodePoint->gameobject_entry == nodePoint->banners[BANNER_A_CONTESTED] || nodePoint->banners[BANNER_H_CONTESTED])
        return nodePoint->last_entry;

    // we should never be here...
    TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle Of Conquest: Unexpected return in GetNextBanner function");
    return 0;
}

void BattlegroundIsleOfConquest::HandleContestedNodes(ICNodePoint* nodePoint)
{
    if (nodePoint->nodeType == NODE_TYPE_HANGAR)
    {
        if (gunshipAlliance && gunshipHorde)
            (nodePoint->faction == TEAM_ALLIANCE ? gunshipHorde : gunshipAlliance)->EnableMovement(false);

        for (uint8 u = BG_IC_GO_HANGAR_TELEPORTER_1; u < BG_IC_GO_HANGAR_TELEPORTER_3; u++)
            DelObject(u);

        for (uint8 u = BG_IC_GO_HANGAR_TELEPORTER_EFFECT_1; u <= BG_IC_GO_HANGAR_TELEPORTER_EFFECT_3; ++u)
            DelObject(u);

        DelCreature(BG_IC_NPC_WORLD_TRIGGER_NOT_FLOATING);

        for (uint8 u = 0; u < MAX_CAPTAIN_SPAWNS_PER_FACTION; ++u)
        {
            uint8 type = BG_IC_NPC_GUNSHIP_CAPTAIN_1 + u;
            DelCreature(type);
        }

        std::list<Creature*> cannons;
        if (nodePoint->faction == TEAM_HORDE)
            gunshipAlliance->GetCreatureListWithEntryInGrid(cannons, NPC_ALLIANCE_GUNSHIP_CANNON, 150.0f);
        else
            gunshipHorde->GetCreatureListWithEntryInGrid(cannons, NPC_HORDE_GUNSHIP_CANNON, 150.0f);

        for (Creature* cannon : cannons)
        {
            cannon->GetVehicleKit()->RemoveAllPassengers();
            cannon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    }
}

void BattlegroundIsleOfConquest::HandleCapturedNodes(ICNodePoint* nodePoint, bool recapture)
{
    if (nodePoint->nodeType != NODE_TYPE_REFINERY && nodePoint->nodeType != NODE_TYPE_QUARRY)
    {
        if (!AddSpiritGuide(BG_IC_NPC_SPIRIT_GUIDE_1 + nodePoint->nodeType - 2, BG_IC_SpiritGuidePos[nodePoint->nodeType], static_cast<TeamId>(nodePoint->faction)))
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: Failed to spawn spirit guide! point: %u, team: %u, ", nodePoint->nodeType, nodePoint->faction);
    }

    switch (nodePoint->gameobject_type)
    {
        case BG_IC_GO_HANGAR_BANNER:
            // all the players on the stopped transport should be teleported out
            if (!gunshipAlliance || !gunshipHorde)
                break;

            for (uint8 u = 0; u < MAX_HANGAR_TELEPORTERS_SPAWNS; u++)
            {
                uint8 type = BG_IC_GO_HANGAR_TELEPORTER_1+u;
                AddObject(type, (nodePoint->faction == TEAM_ALLIANCE ? GO_ALLIANCE_GUNSHIP_PORTAL : GO_HORDE_GUNSHIP_PORTAL),
                    BG_IC_HangarTeleporters[u].GetPositionX(), BG_IC_HangarTeleporters[u].GetPositionY(),
                    BG_IC_HangarTeleporters[u].GetPositionZ(), BG_IC_HangarTeleporters[u].GetOrientation(),
                    0, 0, 0, 0, RESPAWN_ONE_DAY);
            }

            for (uint8 u = 0; u < MAX_HANGAR_TELEPORTER_EFFECTS_SPAWNS; ++u)
            {
                uint8 type = BG_IC_GO_HANGAR_TELEPORTER_EFFECT_1 + u;
                AddObject(type, (nodePoint->faction == TEAM_ALLIANCE ? GO_ALLIANCE_GUNSHIP_PORTAL_EFFECTS : GO_HORDE_GUNSHIP_PORTAL_EFFECTS),
                    BG_IC_HangarTeleporterEffects[u].GetPositionX(), BG_IC_HangarTeleporterEffects[u].GetPositionY(),
                    BG_IC_HangarTeleporterEffects[u].GetPositionZ(), BG_IC_HangarTeleporterEffects[u].GetOrientation(),
                    0, 0, 0, 0, RESPAWN_ONE_DAY, GO_STATE_ACTIVE);
            }

            for (uint8 u = 0; u < MAX_TRIGGER_SPAWNS_PER_FACTION; ++u)
                AddCreature(NPC_WORLD_TRIGGER_NOT_FLOATING, BG_IC_NPC_WORLD_TRIGGER_NOT_FLOATING,
                    BG_IC_HangarTrigger[nodePoint->faction].GetPositionX(), BG_IC_HangarTrigger[nodePoint->faction].GetPositionY(),
                    BG_IC_HangarTrigger[nodePoint->faction].GetPositionZ(), BG_IC_HangarTrigger[nodePoint->faction].GetOrientation(),
                    nodePoint->faction, RESPAWN_ONE_DAY, nodePoint->faction == TEAM_ALLIANCE ? gunshipAlliance : gunshipHorde);

            for (uint8 u = 0; u < MAX_CAPTAIN_SPAWNS_PER_FACTION; ++u)
            {
                uint8 type = BG_IC_NPC_GUNSHIP_CAPTAIN_1 + u;

                if (type == BG_IC_NPC_GUNSHIP_CAPTAIN_1)
                    if (AddCreature(nodePoint->faction == TEAM_ALLIANCE ? NPC_ALLIANCE_GUNSHIP_CAPTAIN : NPC_HORDE_GUNSHIP_CAPTAIN, type,
                        BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 2 : 0].GetPositionX(), BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 2 : 0].GetPositionY(),
                        BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 2 : 0].GetPositionZ(), BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 2 : 0].GetOrientation(),
                        nodePoint->faction, RESPAWN_ONE_DAY))
                        if (Creature* gunShipCaptain = GetBGCreature(BG_IC_NPC_GUNSHIP_CAPTAIN_1))
                            gunShipCaptain->GetAI()->DoAction(ACTION_GUNSHIP_READY);

                if (type == BG_IC_NPC_GUNSHIP_CAPTAIN_2)
                    AddCreature(nodePoint->faction == TEAM_ALLIANCE ? NPC_ALLIANCE_GUNSHIP_CAPTAIN : NPC_HORDE_GUNSHIP_CAPTAIN, type,
                        BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 3 : 1].GetPositionX(), BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 3 : 1].GetPositionY(),
                        BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 3 : 1].GetPositionZ(), BG_IC_HangarCaptains[nodePoint->faction == TEAM_ALLIANCE ? 3 : 1].GetOrientation(),
                        nodePoint->faction, RESPAWN_ONE_DAY, nodePoint->faction == TEAM_ALLIANCE ? gunshipAlliance : gunshipHorde);
            }

            //TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "BG_IC_GO_HANGAR_BANNER CAPTURED Faction: %u", nodePoint->faction);

            (nodePoint->faction == TEAM_ALLIANCE ? gunshipAlliance : gunshipHorde)->EnableMovement(true);
            // we should spawn teleporters
            break;
        case BG_IC_GO_QUARRY_BANNER:
            RemoveAuraOnTeam(SPELL_QUARRY, MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(nodePoint->faction)));
            CastSpellOnTeam(SPELL_QUARRY, MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(nodePoint->faction)));
            break;
        case BG_IC_GO_REFINERY_BANNER:
            RemoveAuraOnTeam(SPELL_OIL_REFINERY, (nodePoint->faction == TEAM_ALLIANCE ? HORDE : ALLIANCE));
            CastSpellOnTeam(SPELL_OIL_REFINERY, MS::Battlegrounds::GetTeamByTeamId(static_cast<TeamId>(nodePoint->faction)));
            break;
        case BG_IC_GO_DOCKS_BANNER:

            if (recapture)
                break;

            if (docksTimer < DOCKS_UPDATE_TIME)
                docksTimer = DOCKS_UPDATE_TIME;

            // we must del opposing faction vehicles when the node is captured (unused ones)
            for (uint8 i = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_GLAIVE_THROWER_1_H : BG_IC_NPC_GLAIVE_THROWER_1_A); i < (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_GLAIVE_THROWER_2_H : BG_IC_NPC_GLAIVE_THROWER_2_A); i++)
            {
                if (Creature* glaiveThrower = GetBGCreature(i))
                {
                    if (Vehicle* vehicleGlaive = glaiveThrower->GetVehicleKit())
                    {
                        if (!vehicleGlaive->GetPassenger(0))
                            DelCreature(i);
                    }
                }
            }

            for (uint8 i = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_CATAPULT_1_H : BG_IC_NPC_CATAPULT_1_A); i < (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_CATAPULT_4_H  : BG_IC_NPC_CATAPULT_4_A); i++)
            {
                if (Creature* catapult = GetBGCreature(i))
                {
                    if (Vehicle* vehicleGlaive = catapult->GetVehicleKit())
                    {
                        if (!vehicleGlaive->GetPassenger(0))
                            DelCreature(i);
                    }
                }
            }

            // spawning glaive throwers
            for (uint8 i = 0; i < MAX_GLAIVE_THROWERS_SPAWNS_PER_FACTION; i++)
            {
                uint8 type = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_GLAIVE_THROWER_1_A : BG_IC_NPC_GLAIVE_THROWER_1_H)+i;

                if (GetBGCreature(type) && GetBGCreature(type)->isAlive())
                    continue;

                if (AddCreature(nodePoint->faction == TEAM_ALLIANCE ? NPC_GLAIVE_THROWER_A : NPC_GLAIVE_THROWER_H, type, nodePoint->faction,
                        BG_IC_DocksVehiclesGlaives[i].GetPositionX(), BG_IC_DocksVehiclesGlaives[i].GetPositionY(),
                        BG_IC_DocksVehiclesGlaives[i].GetPositionZ(), BG_IC_DocksVehiclesGlaives[i].GetOrientation(),
                        RESPAWN_ONE_DAY))
                        GetBGCreature(type)->setFaction(BgFactions[(nodePoint->faction == TEAM_ALLIANCE ? 0 : 1)]);
            }

            // spawning catapults
            for (uint8 i = 0; i < MAX_CATAPULTS_SPAWNS_PER_FACTION; i++)
            {
                uint8 type = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_CATAPULT_1_A : BG_IC_NPC_CATAPULT_1_H)+i;

                if (GetBGCreature(type) && GetBGCreature(type)->isAlive())
                    continue;

                if (AddCreature(NPC_CATAPULT, type, nodePoint->faction,
                        BG_IC_DocksVehiclesCatapults[i].GetPositionX(), BG_IC_DocksVehiclesCatapults[i].GetPositionY(),
                        BG_IC_DocksVehiclesCatapults[i].GetPositionZ(), BG_IC_DocksVehiclesCatapults[i].GetOrientation(),
                        RESPAWN_ONE_DAY))
                        GetBGCreature(type)->setFaction(BgFactions[(nodePoint->faction == TEAM_ALLIANCE ? 0 : 1)]);
            }
            break;
        case BG_IC_GO_WORKSHOP_BANNER:
            {
                if (siegeEngineWorkshopTimer < WORKSHOP_UPDATE_TIME)
                    siegeEngineWorkshopTimer = WORKSHOP_UPDATE_TIME;

                if (!recapture)
                {
                    // we must del opposing faction vehicles when the node is captured (unused ones)
                    for (uint8 i = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_DEMOLISHER_1_H : BG_IC_NPC_DEMOLISHER_1_A); i < (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_DEMOLISHER_4_H : BG_IC_NPC_DEMOLISHER_4_A); i++)
                    {
                        if (Creature* demolisher = GetBGCreature(i))
                        {
                            if (Vehicle* vehicleDemolisher = demolisher->GetVehicleKit())
                            {
                                // is IsVehicleInUse working as expected?
                                if (!vehicleDemolisher->IsVehicleInUse())
                                    DelCreature(i);
                            }
                        }
                    }

                    for (uint8 i = 0; i < MAX_DEMOLISHERS_SPAWNS_PER_FACTION; i++)
                    {
                        uint8 type = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_DEMOLISHER_1_A : BG_IC_NPC_DEMOLISHER_1_H)+i;

                        if (GetBGCreature(type) && GetBGCreature(type)->isAlive())
                            continue;

                        if (AddCreature(NPC_DEMOLISHER, type, nodePoint->faction,
                            BG_IC_WorkshopVehicles[i].GetPositionX(), BG_IC_WorkshopVehicles[i].GetPositionY(),
                            BG_IC_WorkshopVehicles[i].GetPositionZ(), BG_IC_WorkshopVehicles[i].GetOrientation(),
                            RESPAWN_ONE_DAY))
                            GetBGCreature(type)->setFaction(BgFactions[(nodePoint->faction == TEAM_ALLIANCE ? 0 : 1)]);
                    }

                    // we check if the opossing siege engine is in use
                    int8 enemySiege = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_SIEGE_ENGINE_H : BG_IC_NPC_SIEGE_ENGINE_A);

                    if (Creature* siegeEngine = GetBGCreature(enemySiege))
                    {
                        if (Vehicle* vehicleSiege = siegeEngine->GetVehicleKit())
                        {
                            // is VehicleInUse working as expected ?
                            if (!vehicleSiege->IsVehicleInUse())
                                DelCreature(enemySiege);
                        }
                    }

                    uint8 siegeType = (nodePoint->faction == TEAM_ALLIANCE ? BG_IC_NPC_SIEGE_ENGINE_A : BG_IC_NPC_SIEGE_ENGINE_H);
                    if (!GetBGCreature(siegeType) || !GetBGCreature(siegeType)->isAlive())
                    {
                        AddCreature((nodePoint->faction == TEAM_ALLIANCE ? NPC_SIEGE_ENGINE_A : NPC_SIEGE_ENGINE_H), siegeType, nodePoint->faction,
                            BG_IC_WorkshopVehicles[4].GetPositionX(), BG_IC_WorkshopVehicles[4].GetPositionY(),
                            BG_IC_WorkshopVehicles[4].GetPositionZ(), BG_IC_WorkshopVehicles[4].GetOrientation(),
                            RESPAWN_ONE_DAY);

                        if (Creature* siegeEngine = GetBGCreature(siegeType))
                        {
                            siegeEngine->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE| UNIT_FLAG_CANNOT_SWIM |UNIT_FLAG_IMMUNE_TO_PC);
                            siegeEngine->setFaction(BgFactions[(nodePoint->faction == TEAM_ALLIANCE ? 0 : 1)]);
                        }
                    }
                }

                for (uint8 i = 0; i < MAX_WORKSHOP_BOMBS_SPAWNS_PER_FACTION; i++)
                {
                    AddObject(BG_IC_GO_SEAFORIUM_BOMBS_1+i, GO_SEAFORIUM_BOMBS,
                    workshopBombs[i].GetPositionX(), workshopBombs[i].GetPositionY(),
                    workshopBombs[i].GetPositionZ(), workshopBombs[i].GetOrientation(),
                    0, 0, 0, 0, 10);

                    if (GameObject* seaforiumBombs = GetBGObject(BG_IC_GO_SEAFORIUM_BOMBS_1+i))
                    {
                        seaforiumBombs->SetRespawnTime(10);
                        seaforiumBombs->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, BgFactions[(nodePoint->faction == TEAM_ALLIANCE ? 0 : 1)]);
                    }
                }
                break;
            }
        default:
            break;
    }
}

void BattlegroundIsleOfConquest::DestroyGate(Player* player, GameObject* go)
{
    GateStatus[GetGateIDFromEntry(go->GetEntry())] = BG_IC_GATE_DESTROYED;
    uint32 uws_open = (uint32)GetWorldStateFromGateEntry(go->GetEntry(), true);
    uint32 uws_close = (uint32)GetWorldStateFromGateEntry(go->GetEntry(), false);
    if (uws_open)
    {
        UpdateWorldState(uws_close, 0);
        UpdateWorldState(uws_open, 1);
    }
    
    DoorOpen((player->GetBGTeamId() == TEAM_ALLIANCE ? BG_IC_GO_HORDE_KEEP_PORTCULLIS : BG_IC_GO_DOODAD_PORTCULLISACTIVE02));
    GetBGObject(player->GetBGTeamId() == TEAM_ALLIANCE ? BG_IC_GO_HORDE_BANNER : BG_IC_GO_ALLIANCE_BANNER)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);

    RewardHonorToTeam(GetBonusHonorFromKill(8), player->GetBGTeam());

    uint32 textId;
    ChatMsg msgType;
    switch (go->GetEntry())
    {
    case GO_HORDE_GATE_1:
        textId = BG_IC_TEXT_FRONT_GATE_HORDE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        break;
    case GO_HORDE_GATE_2:
        textId = BG_IC_TEXT_WEST_GATE_HORDE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        break;
    case GO_HORDE_GATE_3:
        textId = BG_IC_TEXT_EAST_GATE_HORDE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_ALLIANCE;
        break;
    case GO_ALLIANCE_GATE_1:
        textId = BG_IC_TEXT_WEST_GATE_ALLIANCE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_HORDE;
        break;
    case GO_ALLIANCE_GATE_2:
        textId = BG_IC_TEXT_EAST_GATE_ALLIANCE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_HORDE;
        break;
    case GO_ALLIANCE_GATE_3:
        textId = BG_IC_TEXT_FRONT_GATE_ALLIANCE_DESTROYED;
        msgType = CHAT_MSG_BG_SYSTEM_HORDE;
        break;
    default:
        return;
    }

    if (go->GetEntry() == GO_HORDE_GATE_1 || go->GetEntry() == GO_HORDE_GATE_2 || go->GetEntry() == GO_HORDE_GATE_3)
    {
        if (!GetBgMap()->GetCreature(BgCreatures[BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].type]) && !AddCreature(BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].entry, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].type, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].team, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].x, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].y, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].z, BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].o, RESPAWN_ONE_DAY))
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: There was an error spawning creature %u", BG_IC_NpcSpawnlocs[BG_IC_NPC_OVERLORD_AGMAR].entry);
    }
    else if (go->GetEntry() == GO_ALLIANCE_GATE_1 || go->GetEntry() == GO_ALLIANCE_GATE_2 || go->GetEntry() == GO_ALLIANCE_GATE_3)
    {
        if (!GetBgMap()->GetCreature(BgCreatures[BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].type]) && !AddCreature(BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].entry, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].type, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].team, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].x, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].y, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].z, BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].o, RESPAWN_ONE_DAY))
            TC_LOG_ERROR(LOG_FILTER_BATTLEGROUND, "Isle of Conquest: There was an error spawning creature %u", BG_IC_NpcSpawnlocs[BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE].entry);
    }

    SendBroadcastText(textId, msgType);
}

void BattlegroundIsleOfConquest::EventPlayerDamagedGO(Player* /*player*/, GameObject* /*go*/, uint32 /*eventType*/)
{

}

WorldSafeLocsEntry const* BattlegroundIsleOfConquest::GetClosestGraveYard(Player* player)
{
    TeamId teamIndex = player->GetBGTeamId();

    // Is there any occupied node for this team?
    std::vector<uint8> nodes;
    for (uint8 i = 0; i < MAX_NODE_TYPES; ++i)
        if (nodePoint[i].faction == teamIndex)
            nodes.push_back(i);

    WorldSafeLocsEntry const* good_entry = nullptr;
    // If so, select the closest node to place ghost on
    if (!nodes.empty())
    {
        float plr_x = player->GetPositionX();
        float plr_y = player->GetPositionY();

        float mindist = 999999.0f;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            WorldSafeLocsEntry const*entry = sWorldSafeLocsStore.LookupEntry(BG_IC_GraveyardIds[nodes[i]]);
            if (!entry)
                continue;
            float dist = (entry->Loc.X - plr_x)*(entry->Loc.X - plr_x) + (entry->Loc.Y - plr_y)*(entry->Loc.Y - plr_y);
            if (mindist > dist)
            {
                mindist = dist;
                good_entry = entry;
            }
        }
        nodes.clear();
    }
    // If not, place ghost on starting location
    if (!good_entry)
        good_entry = sWorldSafeLocsStore.LookupEntry(BG_IC_GraveyardIds[teamIndex + MAX_NODE_TYPES]);

    return good_entry;
}
