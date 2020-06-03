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

#include "OutdoorPvP.h"
#include "OutdoorPvPMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "MapManager.h"
#include "Group.h"
#include "WorldPacket.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectVisitors.hpp"
#include "BattlegroundPackets.h"

OPvPCapturePoint::OPvPCapturePoint(OutdoorPvP* pvp) : m_capturePoint(nullptr), m_maxValue(0.0f), m_minValue(0.0f), m_maxSpeed(0),
m_value(0), m_team(TEAM_NEUTRAL), m_OldState(OBJECTIVESTATE_NEUTRAL),
m_State(OBJECTIVESTATE_NEUTRAL), m_neutralValuePct(0), m_PvP(pvp)
{
}

bool OPvPCapturePoint::HandlePlayerEnter(Player* player)
{
    if (m_capturePoint)
    {
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "OPvPCapturePoint::HandlePlayerEnter!");

        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldState1, 1);
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldstate2, static_cast<uint32>(ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f)));
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldstate3, m_neutralValuePct);
    }
    else
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "OPvPCapturePoint::HandlePlayerEnter !!!!!!m_capturePoint!");

    if (player->GetTeamId() < 2)
        return m_activePlayers[player->GetTeamId()].insert(player).second;

    return false;
}

void OPvPCapturePoint::HandlePlayerLeave(Player* player)
{
    if (m_capturePoint)
        player->SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldState1, 0);

    if (player->GetTeamId() < 2)
        m_activePlayers[player->GetTeamId()].erase(player);
}

void OPvPCapturePoint::SendChangePhase()
{
    if (!m_capturePoint)
        return;

    // send this too, sometimes the slider disappears, dunno why :(
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldState1, 1);
    // send these updates to only the ones in this objective
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldstate2, static_cast<uint32>(ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f)));
    // send this too, sometimes it resets :S
    SendUpdateWorldState(m_capturePoint->GetGOInfo()->controlZone.worldstate3, m_neutralValuePct);
}

void OPvPCapturePoint::AddGO(uint32 type, uint32 mapId, ObjectGuid::LowType guid, uint32 entry)
{
    if (!entry)
        if (auto data = sObjectMgr->GetGOData(guid))
            entry = data->id;

    m_Objects[type] = ObjectGuid::Create<HighGuid::GameObject>(mapId, entry, guid);
    m_ObjectTypes[m_Objects[type]] = type;
}

void OPvPCapturePoint::AddCre(uint32 type, uint32 mapId, ObjectGuid::LowType guid, uint32 entry)
{
    if (!entry)
    {
        const CreatureData* data = sObjectMgr->GetCreatureData(guid);
        if (!data)
            return;
        entry = data->id;
    }
    m_Creatures[type] = ObjectGuid::Create<HighGuid::Creature>(mapId, entry, guid);
    m_CreatureTypes[m_Creatures[type]] = type;
}

bool OPvPCapturePoint::SetCapturePointData(go_type data)
{
    return SetCapturePointData(data.entry, data.map, data.x, data.y, data.z, data.o, data.rot0, data.rot1, data.rot2, data.rot3);
}

bool OPvPCapturePoint::AddObject(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3)
{
    if (ObjectGuid::LowType guid = sObjectMgr->AddGOData(entry, map, x, y, z, o, 0, rotation0, rotation1, rotation2, rotation3))
    {
        AddGO(type, guid, entry);
        return true;
    }

    return false;
}

bool OPvPCapturePoint::AddCreature(uint32 p_Type, creature_type data, uint32 p_SpawnTime)
{
    return AddCreature(p_Type, data.entry, data.teamval, data.map, data.x, data.y, data.z, data.o, p_SpawnTime);
}

bool OPvPCapturePoint::AddCreature(uint32 type, uint32 entry, uint32 team, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay)
{
    if (ObjectGuid::LowType guid = sObjectMgr->AddCreData(entry, team, map, x, y, z, o, spawntimedelay))
    {
        AddCre(type, guid, entry);
        return true;
    }

    return false;
}

bool OPvPCapturePoint::SetCapturePointData(uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3)
{
    TC_LOG_TRACE(LOG_FILTER_GENERAL, "Creating capture point %u", entry);
    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo || goinfo->type != GAMEOBJECT_TYPE_CONTROL_ZONE)
    {
        TC_LOG_TRACE(LOG_FILTER_OUTDOORPVP, "OutdoorPvP: GO %u is not capture point!", entry);
        return false;
    }

    m_capturePointGUID = ObjectGuid::Create<HighGuid::GameObject>(map, entry, sObjectMgr->AddGOData(entry, map, x, y, z, o, 0, rotation0, rotation1, rotation2, rotation3));
    if (!m_capturePointGUID)
    {
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "OPvPCapturePoint::SetCapturePointData  - NOT CREATEDCreating capture point %u", entry);
        return false;
    }

    TC_LOG_TRACE(LOG_FILTER_GENERAL, "OPvPCapturePoint::SetCapturePointData::Creating capture point entry %u, map%u, x %f, y %f, z %f m_capturePointGUID %s", entry, map, x, y, z, m_capturePointGUID.ToString().c_str());

    m_maxValue = static_cast<float>(goinfo->controlZone.maxTime);
    m_maxSpeed = m_maxValue / (goinfo->controlZone.minTime ? goinfo->controlZone.minTime : 60);
    m_neutralValuePct = goinfo->controlZone.neutralPercent;
    m_minValue = CalculatePct(m_maxValue, m_neutralValuePct);

    return true;
}

void OPvPCapturePoint::SetState(ObjectiveStates p_State)
{
    m_State = p_State;
}

void OPvPCapturePoint::SetValue(float p_Value)
{
    m_value = p_Value;
}

bool OPvPCapturePoint::AddObject(uint32 p_Type, go_type data)
{
    return AddObject(p_Type, data.entry, data.map, data.x, data.y, data.z, data.o, data.rot0, data.rot1, data.rot2, data.rot3);
}

bool OPvPCapturePoint::DelCreature(uint32 type)
{
    if (!m_Creatures[type])
    {
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "opvp creature type %u was already deleted", type);
        return false;
    }

    Creature* cr = HashMapHolder<Creature>::Find(m_Creatures[type]);
    if (!cr)
    {
        // can happen when closing the core
        m_Creatures[type].Clear();
        return false;
    }
    TC_LOG_TRACE(LOG_FILTER_GENERAL, "deleting opvp creature type %u", type);
    ObjectGuid::LowType guid = cr->GetDBTableGUIDLow();
    // Don't save respawn time
    cr->SetRespawnTime(0);
    cr->RemoveCorpse();
    // explicit removal from map
    // beats me why this is needed, but with the recent removal "cleanup" some creatures stay in the map if "properly" deleted
    // so this is a big fat workaround, if AddObjectToRemoveList and DoDelayedMovesAndRemoves worked correctly, this wouldn't be needed
    //if (Map* map = sMapMgr->FindMap(cr->GetMapId()))
    //    map->Remove(cr, false);
    // delete respawn time for this creature
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CREATURE_RESPAWN);
    stmt->setUInt64(0, guid);
    stmt->setUInt16(1, cr->GetMapId());
    stmt->setUInt32(2, 0);  // instance id, always 0 for world maps
    CharacterDatabase.Execute(stmt);

    cr->AddObjectToRemoveList();
    sObjectMgr->DeleteCreatureData(guid);
    m_CreatureTypes[m_Creatures[type]] = 0;
    m_Creatures[type].Clear();
    return true;
}

bool OPvPCapturePoint::DelObject(uint32 type)
{
    if (!m_Objects[type])
        return false;

    GameObject* obj = HashMapHolder<GameObject>::Find(m_Objects[type]);
    if (!obj)
    {
        m_Objects[type].Clear();
        return false;
    }
    ObjectGuid::LowType guid = obj->GetDBTableGUIDLow();
    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    sObjectMgr->DeleteGOData(guid);
    m_ObjectTypes[m_Objects[type]] = 0;
    m_Objects[type].Clear();
    return true;
}

bool OPvPCapturePoint::DelCapturePoint()
{
    sObjectMgr->DeleteGOData(m_capturePointGUID.GetCounter());
    m_capturePointGUID = ObjectGuid::Empty;

    if (m_capturePoint)
    {
        m_capturePoint->SetRespawnTime(0);                                 // not save respawn time
        m_capturePoint->Delete();
    }

    return true;
}

void OPvPCapturePoint::DeleteSpawns()
{
    for (std::map<uint32, ObjectGuid>::iterator i = m_Objects.begin(); i != m_Objects.end(); ++i)
        DelObject(i->first);
    for (std::map<uint32, ObjectGuid>::iterator i = m_Creatures.begin(); i != m_Creatures.end(); ++i)
        DelCreature(i->first);
    DelCapturePoint();
}

void OutdoorPvP::DeleteSpawns()
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        itr->second->DeleteSpawns();
        delete itr->second;
    }

    m_capturePoints.clear();
}

OutdoorPvP::OutdoorPvP() : m_map(nullptr), m_TypeId(0), m_sendUpdate(true)
{
    m_LastResurectTimer = 30 * IN_MILLISECONDS;
}

OutdoorPvP::~OutdoorPvP()
{
    DeleteSpawns();
}

void OutdoorPvP::HandlePlayerEnterZone(ObjectGuid guid, uint32 /*zone*/)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    m_players[player->GetTeamId()].insert(guid);
}

void OutdoorPvP::HandlePlayerLeaveZone(ObjectGuid guid, uint32 /*zone*/)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    // inform the objectives of the leaving
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        itr->second->HandlePlayerLeave(player);

    // remove the world state information from the player (we can't keep everyone up to date, so leave out those who are not in the concerning zones)
    SendRemoveWorldStates(player);

    m_players[player->GetTeamId()].erase(guid);

    TC_LOG_TRACE(LOG_FILTER_GENERAL, "Player %s left an outdoorpvp zone", player->GetName());
}

void OutdoorPvP::HandlePlayerEnterArea(ObjectGuid guid, uint32 /*area*/)
{
    i_playersInAreaLock.lock();
    m_playersInArea.insert(guid);
    i_playersInAreaLock.unlock();
}

void OutdoorPvP::HandlePlayerLeaveArea(ObjectGuid guid, uint32 /*area*/)
{
    i_playersInAreaLock.lock();
    m_playersInArea.erase(guid);
    i_playersInAreaLock.unlock();
}

void OutdoorPvP::HandlePlayerResurrects(Player* /*player*/, uint32 /*zone*/)
{
}

bool OutdoorPvP::Update(uint32 diff)
{
    bool objective_changed = false;
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
    {
        if (itr->second->Update(diff))
            objective_changed = true;
    }

    if (m_LastResurectTimer <= diff)
    {
        for (GraveyardVector::iterator itr = m_GraveyardList.begin(); itr != m_GraveyardList.end(); ++itr)
            if (*itr)
                (*itr)->Resurrect();

        m_LastResurectTimer = RESURRECTION_INTERVAL;
    }
    else
        m_LastResurectTimer -= diff;

    return objective_changed;
}

bool OPvPCapturePoint::Update(uint32 diff)
{
    if (!m_capturePoint)
        return false;

    float radius = static_cast<float>(m_capturePoint->GetGOInfo()->controlZone.radius);

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        for (PlayerSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end();)
        {
            Player* player = *itr;
            ++itr;
            if (!m_capturePoint->IsWithinDistInMap(player, radius) || !player->IsOutdoorPvPActive())
                HandlePlayerLeave(player);
        }
    }

    std::list<Player*> players;
    Trinity::AnyPlayerInObjectRangeCheck checker(m_capturePoint, radius);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(m_capturePoint, players, checker);
    VisitNearbyWorldObject(m_capturePoint, radius, searcher);

    for (std::list<Player*>::iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        if ((*itr)->IsOutdoorPvPActive() && (*itr)->GetTeamId() != TEAM_NEUTRAL)
        {
            if (m_activePlayers[(*itr)->GetTeamId()].insert(*itr).second)
                HandlePlayerEnter(*itr);
        }
    }

    // get the difference of numbers
    float fact_diff = (static_cast<float>(m_activePlayers[0].size()) - static_cast<float>(m_activePlayers[1].size())) * diff / OUTDOORPVP_OBJECTIVE_UPDATE_INTERVAL;
    if (!fact_diff)
        return false;

    uint32 Challenger = 0;
    float maxDiff = m_maxSpeed * diff;

    if (fact_diff < 0)
    {
        // horde is in majority, but it's already horde-controlled -> no change
        if (m_State == OBJECTIVESTATE_HORDE && m_value <= -m_maxValue)
            return false;

        if (fact_diff < -maxDiff)
            fact_diff = -maxDiff;

        Challenger = HORDE;
    }
    else
    {
        // ally is in majority, but it's already ally-controlled -> no change
        if (m_State == OBJECTIVESTATE_ALLIANCE && m_value >= m_maxValue)
            return false;

        if (fact_diff > maxDiff)
            fact_diff = maxDiff;

        Challenger = ALLIANCE;
    }

    float oldValue = m_value;
    TeamId oldTeam = m_team;

    m_OldState = m_State;

    m_value += fact_diff;

    if (m_value < -m_minValue) // red
    {
        if (m_value < -m_maxValue)
            m_value = -m_maxValue;
        m_State = OBJECTIVESTATE_HORDE;
        m_team = TEAM_HORDE;
    }
    else if (m_value > m_minValue) // blue
    {
        if (m_value > m_maxValue)
            m_value = m_maxValue;
        m_State = OBJECTIVESTATE_ALLIANCE;
        m_team = TEAM_ALLIANCE;
    }
    else if (oldValue * m_value <= 0) // grey, go through mid point
    {
        // if challenger is ally, then n->a challenge
        if (Challenger == ALLIANCE)
            m_State = OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE;
        // if challenger is horde, then n->h challenge
        else if (Challenger == HORDE)
            m_State = OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE;
        m_team = TEAM_NEUTRAL;
    }
    else // grey, did not go through mid point
    {
        // old phase and current are on the same side, so one team challenges the other
        if (Challenger == ALLIANCE && (m_OldState == OBJECTIVESTATE_HORDE || m_OldState == OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE))
            m_State = OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE;
        else if (Challenger == HORDE && (m_OldState == OBJECTIVESTATE_ALLIANCE || m_OldState == OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE))
            m_State = OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE;
        m_team = TEAM_NEUTRAL;
    }

    if (m_value != oldValue)
        SendChangePhase();

    if (m_OldState != m_State)
    {
        //TC_LOG_TRACE(LOG_FILTER_OUTDOORPVP, "%u->%u", m_OldState, m_State);
        if (oldTeam != m_team)
            ChangeTeam(oldTeam);
        ChangeState();
        return true;
    }

    return false;
}

void OutdoorPvP::SendUpdateWorldState(uint32 field, uint32 value)
{
    if (m_sendUpdate)
        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
            for (GuidSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                    player->SendUpdateWorldState(field, value);
}

void OPvPCapturePoint::SendUpdateWorldState(uint32 field, uint32 value)
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (PlayerSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end(); ++itr)
            (*itr)->SendUpdateWorldState(field, value);
}

void OutdoorPvP::SendUpdateWorldState(WorldStates field, uint32 value)
{
    if (m_sendUpdate)
        for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
            for (GuidSet::iterator itr = m_players[i].begin(); itr != m_players[i].end(); ++itr)
                if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                    player->SendUpdateWorldState(field, value);
}

void OPvPCapturePoint::SendUpdateWorldState(WorldStates field, uint32 value)
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (PlayerSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end(); ++itr)
            (*itr)->SendUpdateWorldState(field, value);
}

void OPvPCapturePoint::SendObjectiveComplete(uint32 id, ObjectGuid guid)
{
    uint32 team;
    switch (m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
            team = 0;
            break;
        case OBJECTIVESTATE_HORDE:
            team = 1;
            break;
        default:
            return;
    }

    // send to all players present in the area
    for (PlayerSet::iterator itr = m_activePlayers[team].begin(); itr != m_activePlayers[team].end(); ++itr)
        (*itr)->KilledMonsterCredit(id, guid);
}

void OutdoorPvP::HandleKill(Player* killer, Unit* killed)
{
    if (Group* group = killer->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* groupGuy = itr->getSource();

            if (!groupGuy)
                continue;

            // skip if too far away
            if (!groupGuy->IsAtGroupRewardDistance(killed))
                continue;

            // creature kills must be notified, even if not inside objective / not outdoor pvp active
            // player kills only count if active and inside objective
            if ((groupGuy->IsOutdoorPvPActive() && IsInsideObjective(groupGuy)) || killed->IsCreature())
            {
                HandleKillImpl(groupGuy, killed);
            }
        }
    }
    else
    {
        // creature kills must be notified, even if not inside objective / not outdoor pvp active
        if (killer && ((killer->IsOutdoorPvPActive() && IsInsideObjective(killer)) || killed->IsCreature()))
        {
            HandleKillImpl(killer, killed);
        }
    }
}

bool OutdoorPvP::IsInsideObjective(Player* player) const
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->IsInsideObjective(player))
            return true;

    return false;
}

bool OPvPCapturePoint::IsInsideObjective(Player* player) const
{
    if (player && player->GetTeamId() < 2)
        return m_activePlayers[player->GetTeamId()].find(player) != m_activePlayers[player->GetTeamId()].end();

    return false;
}

bool OutdoorPvP::HandleCustomSpell(Player* player, uint32 spellId, GameObject* go)
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->HandleCustomSpell(player, spellId, go))
            return true;

    return false;
}

bool OPvPCapturePoint::HandleCustomSpell(Player* player, uint32 /*spellId*/, GameObject* /*go*/)
{
    if (!player->IsOutdoorPvPActive())
        return false;
    return false;
}

bool OutdoorPvP::HandleOpenGo(Player* player, ObjectGuid guid)
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->HandleOpenGo(player, guid) >= 0)
            return true;

    return false;
}

bool OutdoorPvP::HandleGossipOption(Player* player, ObjectGuid guid, uint32 id)
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->HandleGossipOption(player, guid, id))
            return true;

    return false;
}

bool OutdoorPvP::CanTalkTo(Player* player, Creature* c, GossipMenuItems const& gso)
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->CanTalkTo(player, c, gso))
            return true;

    return false;
}

uint32 OutdoorPvP::GetTypeId()
{
    return m_TypeId;
}

bool OutdoorPvP::HandleDropFlag(Player* player, uint32 id)
{
    for (auto itr = m_capturePoints.begin(); itr != m_capturePoints.end(); ++itr)
        if (itr->second->HandleDropFlag(player, id))
            return true;

    return false;
}

bool OPvPCapturePoint::HandleGossipOption(Player* /*player*/, ObjectGuid /*guid*/, uint32 /*id*/)
{
    return false;
}

bool OPvPCapturePoint::CanTalkTo(Player* /*player*/, Creature* /*c*/, GossipMenuItems const& /*gso*/)
{
    return false;
}

bool OPvPCapturePoint::HandleDropFlag(Player* /*player*/, uint32 /*id*/)
{
    return false;
}

int32 OPvPCapturePoint::HandleOpenGo(Player* /*player*/, ObjectGuid guid)
{
    std::map<ObjectGuid, uint32>::iterator itr = m_ObjectTypes.find(guid);
    if (itr != m_ObjectTypes.end())
        return itr->second;

    return -1;
}

void OutdoorPvP::HandleAreaTrigger(Player* player, uint32 trigger, bool entered)
{
    //player->GetSession()->SendNotification("Warning: Unhandled AreaTrigger in Battleground: %u", trigger);
}

void OutdoorPvP::ApplyOnEveryPlayerInZone(std::function<void(Player*)> function, uint32 zone)
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                    player->AddDelayedEvent(10, [player, function]() -> void{function(player);});
}

void OutdoorPvP::BroadcastPacket(const WorldPacket &data) const
{
    // This is faster than sWorld->SendZoneMessage
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                player->SendDirectMessage(&data);
}

void OutdoorPvP::AddCapturePoint(OPvPCapturePoint* cp)
{
    m_capturePoints[cp->m_capturePointGUID] = cp;
    if (auto go = sObjectAccessor->FindGameObject(cp->m_capturePointGUID))
        cp->m_capturePoint = go;
    else
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "OutdoorPvP::AddCapturePoint !!!!!!m_capturePoint! %s", cp->m_capturePointGUID.ToString().c_str());
}

OPvPCapturePoint* OutdoorPvP::GetCapturePoint(ObjectGuid guid) const
{
    return Trinity::Containers::MapGetValuePtr(m_capturePoints, guid);
}

void OutdoorPvP::RegisterZone(uint32 zoneId)
{
    sOutdoorPvPMgr->AddZone(zoneId, this);
}

bool OutdoorPvP::HasPlayer(Player* player) const
{
    if (player && player->GetTeamId() < 2)
        return m_players[player->GetTeamId()].find(player->GetGUID()) != m_players[player->GetTeamId()].end();

    return false;
}

void OutdoorPvP::TeamCastSpell(TeamId team, int32 spellId)
{
    if (spellId > 0)
    {
        for (GuidSet::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                player->CastSpell(player, static_cast<uint32>(spellId), true);
    }
    else
        for (GuidSet::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = ObjectAccessor::GetObjectInMap(*itr, m_map, (Player*)nullptr))
                player->RemoveAura(static_cast<uint32>(-spellId));
}

bool OutdoorPvP::AddCreature(uint32 p_Type, creature_type data, uint32 p_SpawnTime)
{
    return AddCreature(p_Type, data.entry, data.teamval, data.map, data.x, data.y, data.z, data.o, p_SpawnTime);
}

bool OutdoorPvP::AddCreature(uint32 type, uint32 entry, uint32 team, uint32 mapID, float x, float y, float z, float o, uint32 p_SpawnTime /*= 0*/)
{
    if (uint64 guid = sObjectMgr->AddCreData(entry, team, mapID, x, y, z, o, p_SpawnTime))
    {
        if (!entry)
        {
            CreatureData const* data = sObjectMgr->GetCreatureData(guid);
            if (!data)
                return false;

            entry = data->id;
        }

        CreatureTemplate const* l_Template = sObjectMgr->GetCreatureTemplate(entry);
        if (!l_Template)
            return false;

        if (l_Template->VehicleId)
            m_Creatures[type] = ObjectGuid::Create<HighGuid::Vehicle>(mapID, guid, entry);
        else
            m_Creatures[type] = ObjectGuid::Create<HighGuid::Creature>(mapID, guid, entry);

        m_CreatureTypes[m_Creatures[type]] = type;
        return true;
    }

    return false;
}

bool OutdoorPvP::DelCreature(uint32 type)
{
    if (!m_Creatures[type])
    {
        TC_LOG_TRACE(LOG_FILTER_GENERAL, "OutdoorPvP::DelCreature, creature type %u was already deleted", type);
        return false;
    }

    Creature* l_Creature = HashMapHolder<Creature>::Find(m_Creatures[type]);
    if (!l_Creature)
    {
        m_Creatures[type] = ObjectGuid::Empty;
        return false;
    }

    TC_LOG_TRACE(LOG_FILTER_GENERAL, "OutdoorPvP::DelCreature, deleting creature type %u", type);

    uint64 guid = l_Creature->GetDBTableGUIDLow();

    l_Creature->SetRespawnTime(0);
    l_Creature->RemoveCorpse();

    //PreparedStatement* l_Statement = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CREATURE_RESPAWN);
    //l_Statement->setUInt64(0, guid);
    //l_Statement->setUInt16(1, l_Creature->GetMapId());
    //l_Statement->setUInt32(2, 0);  // InstanceID, always 0 for world maps
    //CharacterDatabase.Execute(l_Statement);

    l_Creature->AddObjectToRemoveList();
    sObjectMgr->DeleteCreatureData(guid);
    m_CreatureTypes[m_Creatures[type]] = 0;
    m_Creatures[type] = ObjectGuid::Empty;
    return true;
}

bool OutdoorPvP::AddObject(uint32 p_Type, go_type data)
{
    return AddObject(p_Type, data.entry, data.map, data.x, data.y, data.z, data.o, data.rot0, data.rot1, data.rot2, data.rot3);
}

bool OutdoorPvP::AddObject(uint32 type, uint32 entry, uint32 mapID, float x, float y, float z, float o, float r0, float r1, float r2, float r3)
{
    uint64 guid = sObjectMgr->GetGenerator<HighGuid::GameObject>()->Generate();
    if (sObjectMgr->AddGOData(guid, entry, mapID, x, y, z, o, 0, r0, r1, r2, r3))
    {
        m_Objects[type] = ObjectGuid::Create<HighGuid::GameObject>(mapID, guid, entry);
        m_ObjectTypes[m_Objects[type]] = type;
        return true;
    }

    return false;
}

bool OutdoorPvP::DelObject(uint32 type)
{
    if (!m_Objects[type])
        return false;

    GameObject* gameObj = HashMapHolder<GameObject>::Find(m_Objects[type]);
    if (!gameObj)
    {
        m_Objects[type] = ObjectGuid::Empty;
        return false;
    }

    uint64 guid = gameObj->GetDBTableGUIDLow();

    gameObj->SetRespawnTime(0);

    //PreparedStatement* l_Statement = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GO_RESPAWN);
    //l_Statement->setUInt64(0, guid);
    //l_Statement->setUInt16(1, gameObj->GetMapId());
    //l_Statement->setUInt32(2, 0);  ///< InstanceID, always 0 for world maps
    //CharacterDatabase.Execute(l_Statement);

    gameObj->AddObjectToRemoveList();
    sObjectMgr->DeleteGOData(guid);
    m_ObjectTypes[m_Objects[type]] = 0;
    m_Objects[type] = ObjectGuid::Empty;
    return true;
}

void OutdoorPvP::TeamApplyBuff(TeamId team, uint32 spellId, uint32 spellId2)
{
    TeamCastSpell(team, spellId);
    TeamCastSpell(OTHER_TEAM(team), spellId2 ? -static_cast<int32>(spellId2) : -static_cast<int32>(spellId));
}

void OutdoorPvP::SetMap(Map* map)
{
    m_map = map;
}

Map* OutdoorPvP::GetMap() const
{
    return m_map;
}

void OutdoorPvP::OnGameObjectCreate(GameObject* go)
{
    if (go->GetGoType() != GAMEOBJECT_TYPE_CONTROL_ZONE)
        return;

    if (OPvPCapturePoint *cp = GetCapturePoint(go->GetGUID()))
        cp->m_capturePoint = go;
}

void OutdoorPvP::OnGameObjectRemove(GameObject* go)
{
    if (go->GetGoType() != GAMEOBJECT_TYPE_CONTROL_ZONE)
        return;

    if (OPvPCapturePoint* cp = GetCapturePoint(go->GetGUID()))
        cp->m_capturePoint = nullptr;

    ZoneScript::OnGameObjectRemove(go);
}

WorldSafeLocsEntry const* OutdoorPvP::GetClosestGraveyard(Player* player)
{
    OutdoorGraveyard* closestGrave = nullptr;
    float maxDist = 1000.0f;

    for (uint8 i = 0; i < m_GraveyardList.size(); ++i)
    {
        if (m_GraveyardList[i])
        {
            if (m_GraveyardList[i]->GetControlTeamId() != player->GetTeamId())
                continue;

            float dist = m_GraveyardList[i]->GetDistance(player);
            if (dist < maxDist || maxDist < 0)
            {
                closestGrave = m_GraveyardList[i];
                maxDist = dist;
            }
        }
    }

    if (closestGrave)
        return sWorldSafeLocsStore.LookupEntry(closestGrave->GetGraveyardId());

    return nullptr;
}

void OutdoorPvP::SendAreaSpiritHealerQueryOpcode(Player* player, ObjectGuid const& guid)
{
    ASSERT(player && player->GetSession());

    WorldPackets::Battleground::AreaSpiritHealerTime healerTime;
    healerTime.HealerGuid = guid;
    healerTime.TimeLeft = m_LastResurectTimer;
    ASSERT(player && player->GetSession());
    player->SendDirectMessage(healerTime.Write());
}

void OutdoorPvP::AddPlayerToResurrectQueue(ObjectGuid p_SpiritGuid, ObjectGuid p_PlayerGuid)
{
    for (uint8 i = 0; i < m_GraveyardList.size(); i++)
    {
        if (m_GraveyardList[i] == nullptr)
            continue;

        if (m_GraveyardList[i]->HasNpc(p_SpiritGuid))
        {
            m_GraveyardList[i]->AddPlayer(p_PlayerGuid);
            break;
        }
    }
}

OutdoorGraveyard* OutdoorPvP::GetGraveyardById(uint32 id)
{
    if (id < m_GraveyardList.size())
    {
        if (m_GraveyardList[id])
            return m_GraveyardList[id];
        TC_LOG_TRACE(LOG_FILTER_BATTLEFIELD, "OutdoorPvP::GetGraveyardById Id:%u not existed", id);
    }
    else
        TC_LOG_TRACE(LOG_FILTER_BATTLEFIELD, "OutdoorPvP::GetGraveyardById Id:%u cant be found", id);

    return nullptr;
}

void OutdoorPvP::SetGraveyardNumber(uint32 p_Count)
{
    m_GraveyardList.resize(p_Count);
}

ObjectGuid OutdoorPvP::GetCreature(uint32 type)
{
    if (m_Creatures.find(type) == m_Creatures.end())
        return ObjectGuid::Empty;

    return m_Creatures[type];
}

GuidSet const& OutdoorPvP::GetPlayers(uint32 p_Team)
{
    return m_players[p_Team];
}

Player* OutdoorPvP::GetFirstPlayer()
{
    for (uint8 i = 0; i < MAX_TEAMS; ++i)
        for (const auto& guid : GetPlayers(i))
            if (auto player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr))
                return player;

    return nullptr;
}

OutdoorGraveyard::OutdoorGraveyard(OutdoorPvP* p_OutdoorPvP)
{
    m_OutdoorPvP = p_OutdoorPvP;
    m_GraveyardId = 0;
    m_ControlTeam = TEAM_NEUTRAL;
    m_SpiritGuide[0] = ObjectGuid::Empty;
    m_SpiritGuide[1] = ObjectGuid::Empty;
    m_ResurrectQueue.clear();
}

void OutdoorGraveyard::Initialize(TeamId team, uint32 p_Graveyard)
{
    m_ControlTeam = team;
    m_GraveyardId = p_Graveyard;
}

void OutdoorGraveyard::SetSpirit(Creature* p_Spirit, TeamId team)
{
    if (p_Spirit == nullptr)
    {
        TC_LOG_TRACE(LOG_FILTER_BATTLEFIELD, "OutdoorGraveyard::SetSpirit -> Invalid Spirit.");
        return;
    }

    m_SpiritGuide[team] = p_Spirit->GetGUID();
    p_Spirit->SetReactState(REACT_PASSIVE);
}

float OutdoorGraveyard::GetDistance(Player* player) const
{
    WorldSafeLocsEntry const* l_SafeLoc = sWorldSafeLocsStore.LookupEntry(m_GraveyardId);
    return player->GetDistance2d(l_SafeLoc->Loc.X, l_SafeLoc->Loc.Y);
}

void OutdoorGraveyard::AddPlayer(ObjectGuid guid)
{
    if (!m_ResurrectQueue.count(guid))
    {
        m_ResurrectQueue.insert(guid);

        if (Player* player = sObjectAccessor->FindPlayer(guid))
            player->CastSpell(player, SPELL_WAITING_FOR_RESURRECT, true);
    }
}

void OutdoorGraveyard::RemovePlayer(ObjectGuid guid)
{
    m_ResurrectQueue.erase(m_ResurrectQueue.find(guid));

    if (Player* player = sObjectAccessor->FindPlayer(guid))
        player->RemoveAurasDueToSpell(SPELL_WAITING_FOR_RESURRECT);
}

void OutdoorGraveyard::Resurrect()
{
    if (m_ResurrectQueue.empty())
        return;

    for (std::set<ObjectGuid>::const_iterator l_Iter = m_ResurrectQueue.begin(); l_Iter != m_ResurrectQueue.end(); ++l_Iter)
    {
        Player* player = sObjectAccessor->FindPlayer(*l_Iter);
        if (!player)
            continue;

        if (player->IsInWorld())
            if (Unit* l_Spirit = sObjectAccessor->FindUnit(m_SpiritGuide[m_ControlTeam]))
                l_Spirit->CastSpell(l_Spirit, SPELL_SPIRIT_HEAL, true);

        player->CastSpell(player, SPELL_RESURRECTION_VISUAL, true);
        player->ResurrectPlayer(1.0f);
        player->CastSpell(player, SPELL_SPIRIT_HEAL_MANA, true);

        if (!player->HasSpell(155228) && !player->HasSpell(205024) && player->GetSpecializationId() != SPEC_MAGE_FIRE &&
            player->GetSpecializationId() != SPEC_MAGE_ARCANE && player->GetSpecializationId() != SPEC_DK_BLOOD && player->GetSpecializationId() != SPEC_DK_FROST)
            player->CastSpell(player, SPELL_PET_SUMMONED, true);

        sObjectAccessor->ConvertCorpseForPlayer(player->GetGUID());
    }

    m_ResurrectQueue.clear();
}

void OutdoorGraveyard::GiveControlTo(TeamId team)
{
    m_ControlTeam = team;
    RelocateDeadPlayers();
}

TeamId OutdoorGraveyard::GetControlTeamId()
{
    return m_ControlTeam;
}

void OutdoorGraveyard::RelocateDeadPlayers()
{
    WorldSafeLocsEntry const* closestGrave = nullptr;
    for (std::set<ObjectGuid>::const_iterator l_Iter = m_ResurrectQueue.begin(); l_Iter != m_ResurrectQueue.end(); ++l_Iter)
    {
        Player* player = sObjectAccessor->FindPlayer(*l_Iter);
        if (!player)
            continue;

        if (closestGrave)
            player->TeleportTo(player->GetMapId(), closestGrave->Loc.X, closestGrave->Loc.Y, closestGrave->Loc.Z, player->GetOrientation());
        else
        {
            closestGrave = m_OutdoorPvP->GetClosestGraveyard(player);
            if (closestGrave)
                player->TeleportTo(player->GetMapId(), closestGrave->Loc.X, closestGrave->Loc.Y, closestGrave->Loc.Z, player->GetOrientation());
        }
    }
}

bool OutdoorGraveyard::HasNpc(ObjectGuid guid)
{
    if (!m_SpiritGuide[0] && !m_SpiritGuide[1])
        return false;

    if (!sObjectAccessor->FindUnit(m_SpiritGuide[0]) && !sObjectAccessor->FindUnit(m_SpiritGuide[1]))
        return false;

    return (m_SpiritGuide[0] == guid || m_SpiritGuide[1] == guid);
}

bool OutdoorGraveyard::HasPlayer(ObjectGuid guid)
{
    return m_ResurrectQueue.find(guid) != m_ResurrectQueue.end();
}

uint32 OutdoorGraveyard::GetGraveyardId()
{
    return m_GraveyardId;
}
