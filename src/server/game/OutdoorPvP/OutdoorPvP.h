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

#ifndef OUTDOOR_PVP_H_
#define OUTDOOR_PVP_H_

#include "Utilities/Util.h"
#include "SharedDefines.h"
#include "ZoneScript.h"
#include <safe_ptr.h>

namespace WorldPackets
{
    namespace WorldState
    {
        class InitWorldStates;
    }
}

class GameObject;

enum OutdoorPvPTypes
{
    OUTDOOR_PVP_HP              = 1,
    OUTDOOR_PVP_NA              = 2,
    OUTDOOR_PVP_TF              = 3,
    OUTDOOR_PVP_ZM              = 4,
    OUTDOOR_PVP_SILITHUS        = 5,
    OUTDOOR_PVP_ASHRAN          = 6,
    OUTDOOR_PVP_TARRENMILL      = 7,
    OUTDOOR_PVP_RG              = 8,
    OUTDOOR_PVP_SENTINAX        = 9,
    OUTDOOR_PVP_ARGUS_INVASION  = 10,
    OUTDOOR_PVP_PARAXIS         = 11,
    OUTDOOR_PVP_DALARAN_EVENT   = 12,
    OUTDOOR_PVP_THOUSAND_NEEDLES = 13,
    OUTDOOR_PVP_AB_WINTER_EVENT = 14,
    OUTDOOR_PVP_AB_WNTR_EVENT_EV= 15,

    MAX_OUTDOORPVP_TYPES
};

enum ObjectiveStates
{
    OBJECTIVESTATE_NEUTRAL,
    OBJECTIVESTATE_ALLIANCE,
    OBJECTIVESTATE_HORDE,
    OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE,
    OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE,
    OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE,
    OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE
};

#define OTHER_TEAM(a) (a == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE)

// struct for go spawning
struct go_type
{
    uint32 entry;
    uint32 map;
    float x;
    float y;
    float z;
    float o;
    float rot0;
    float rot1;
    float rot2;
    float rot3;
};

// struct for creature spawning
struct creature_type
{
    uint32 entry;
    uint32 teamval;
    uint32 map;
    float x;
    float y;
    float z;
    float o;
};

// some class predefs
class Player;
class GameObject;
class WorldPacket;
class Creature;
class Unit;
struct GossipMenuItems;
class OutdoorPvP;
class OutdoorGraveyard;

typedef std::set<Player*> PlayerSet;
typedef std::vector<OutdoorGraveyard*> GraveyardVector;

class OPvPCapturePoint
{
    public:

        explicit OPvPCapturePoint(OutdoorPvP* pvp);

        virtual ~OPvPCapturePoint() {}

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

        // send world state update to all players present
        void SendUpdateWorldState(uint32 field, uint32 value);
        void SendUpdateWorldState(WorldStates field, uint32 value);

        // send kill notify to players in the controlling faction
        void SendObjectiveComplete(uint32 id, ObjectGuid guid);

        // used when player is activated/inactivated in the area
        virtual bool HandlePlayerEnter(Player* player);
        virtual void HandlePlayerLeave(Player* player);

        // checks if player is in range of a capture credit marker
        bool IsInsideObjective(Player* player) const;

        virtual bool HandleCustomSpell(Player* player, uint32 spellId, GameObject* go);

        virtual int32 HandleOpenGo(Player* player, ObjectGuid guid);

        // returns true if the state of the objective has changed, in this case, the OutdoorPvP must send a world state ui update.
        virtual bool Update(uint32 diff);

        virtual void ChangeState() = 0;

        virtual void ChangeTeam(TeamId /*oldTeam*/) {}

        virtual void SendChangePhase();

        virtual bool HandleGossipOption(Player* player, ObjectGuid guid, uint32 gossipid);

        virtual bool CanTalkTo(Player* player, Creature* c, GossipMenuItems const& gso);

        virtual bool HandleDropFlag(Player* player, uint32 spellId);

        virtual void DeleteSpawns();

        ObjectGuid m_capturePointGUID;

        GameObject* m_capturePoint;

        void AddGO(uint32 type, uint32 mapId, ObjectGuid::LowType guid, uint32 entry = 0);
        void AddCre(uint32 type, uint32 mapId, ObjectGuid::LowType guid, uint32 entry = 0);

        bool SetCapturePointData(go_type data);
        bool SetCapturePointData(uint32 entry, uint32 map, float x, float y, float z, float o = 0, float rotation0 = 0, float rotation1 = 0, float rotation2 = 0, float rotation3 = 0);

        void SetState(ObjectiveStates p_State);
        void SetValue(float p_Value);

    protected:

        bool AddObject(uint32 p_Type, go_type data);
        bool AddObject(uint32 type, uint32 entry, uint32 map, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3);
        bool AddCreature(uint32 p_Type, creature_type data, uint32 p_SpawnTime = 0);
        bool AddCreature(uint32 type, uint32 entry, uint32 teamval, uint32 map, float x, float y, float z, float o, uint32 spawntimedelay = 0);

        bool DelCreature(uint32 type);
        bool DelObject(uint32 type);

        bool DelCapturePoint();

        // active players in the area of the objective, 0 - alliance, 1 - horde
        PlayerSet m_activePlayers[MAX_TEAMS];

        // total shift needed to capture the objective
        float m_maxValue;
        float m_minValue;

        // maximum speed of capture
        float m_maxSpeed;

        // the status of the objective
        float m_value;

        TeamId m_team;

        // objective states
        ObjectiveStates m_OldState;
        ObjectiveStates m_State;

        // neutral value on capture bar
        uint32 m_neutralValuePct;

        // pointer to the OutdoorPvP this objective belongs to
        OutdoorPvP* m_PvP;

        // map to store the various gameobjects and creatures spawned by the objective
        //        type, guid
        std::map<uint32, ObjectGuid> m_Objects;
        std::map<uint32, ObjectGuid> m_Creatures;
        std::map<ObjectGuid, uint32> m_ObjectTypes;
        std::map<ObjectGuid, uint32> m_CreatureTypes;
};

// base class for specific outdoor pvp handlers
class OutdoorPvP : public ZoneScript
{
    friend class OutdoorPvPMgr;

    public:

        // ctor
        OutdoorPvP();

        // dtor
        virtual ~OutdoorPvP();

        // deletes all gos/creatures spawned by the pvp
        void DeleteSpawns();

        virtual void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& /*packet*/) { }

        // called when a player triggers an areatrigger
        virtual void HandleAreaTrigger(Player* player, uint32 trigger, bool entered);

        // called on custom spell
        virtual bool HandleCustomSpell(Player* player, uint32 spellId, GameObject* go);

        // called on go use
        virtual bool HandleOpenGo(Player* player, ObjectGuid guid);

        // setup stuff
        virtual bool SetupOutdoorPvP() {return true;}

        virtual void OnCreatureCreate(Creature* /*creature*/) {}
        virtual void OnCreatureRemove(Creature* /*creature*/) {}
        virtual void OnGameObjectCreate(GameObject* go);
        virtual void OnGameObjectRemove(GameObject* go);

        // send world state update to all players present
        void SendUpdateWorldState(uint32 field, uint32 value);
        void SendUpdateWorldState(WorldStates field, uint32 value);

        // called by OutdoorPvPMgr, updates the objectives and if needed, sends new worldstateui information
        virtual bool Update(uint32 diff);

        // handle npc/player kill
        virtual void HandleKill(Player* killer, Unit* killed);
        virtual void HandleKill(Unit* killer, Unit* killed) {};
        virtual void HandleKillImpl(Player* /*killer*/, Unit* /*killed*/) {}
        virtual void HandlePlayerKilled(Player* p_Player) { }
        virtual void HandleRewardHonor(Player* p_Player) { }
        virtual void FillCustomPvPLoots(Player* /*looter*/, Loot& /*loot*/, ObjectGuid /*container*/) { }

        virtual void HandleSpellClick(Player* player, Unit* target) {}

        // checks if player is in range of a capture credit marker
        bool IsInsideObjective(Player* player) const;

        // awards rewards for player kill
        virtual void AwardKillBonus(Player* /*player*/) {}

        uint32 GetTypeId();

        virtual bool HandleDropFlag(Player* player, uint32 spellId);

        virtual bool HandleGossipOption(Player* player, ObjectGuid guid, uint32 gossipid);

        virtual bool CanTalkTo(Player* player, Creature* c, GossipMenuItems const& gso);

        void TeamApplyBuff(TeamId team, uint32 spellId, uint32 spellId2 = 0);

        void SetMap(Map* map);
        Map* GetMap() const;
        virtual void HandleBFMGREntryInviteResponse(bool /*accepted*/, Player* /*p_Player*/) { }

        virtual WorldSafeLocsEntry const* GetClosestGraveyard(Player* p_Player);

        void SendAreaSpiritHealerQueryOpcode(Player* p_Player, ObjectGuid const& guid);
        void AddPlayerToResurrectQueue(ObjectGuid p_SpiritGuid, ObjectGuid p_PlayerGuid);
        OutdoorGraveyard* GetGraveyardById(uint32 p_ID);
        void SetGraveyardNumber(uint32 p_Count);

        ObjectGuid GetCreature(uint32 p_Type);

        GuidSet const& GetPlayers(uint32 p_Team);
        Player* GetFirstPlayer();

        bool ThisZone(uint32 zoneID) const { return m_zoneSet.find(zoneID) != m_zoneSet.end(); }
        size_t SizeZones() const { return m_zoneSet.size(); }
        void RemoveZone(uint32 zoneId) { m_zoneSet.erase(zoneId); }
        std::set<uint32> m_zoneSet;

        virtual void Initialize(uint32 zone) {}
        virtual void BroadcastPacketByZone(const WorldPacket & data, uint32 zone) { BroadcastPacket(data); }
        virtual void ApplyOnEveryPlayerInZone(std::function<void(Player*)> function, uint32 zone = 0);
        virtual void HandleGameEventStart(uint32 eventId) {}
        void RegisterZone(uint32 zoneid);
    protected:
        virtual void SendRemoveWorldStates(Player* /*player*/) {}

        void BroadcastPacket(const WorldPacket & data) const;

        virtual void HandlePlayerEnterZone(ObjectGuid guid, uint32 zone);
        virtual void HandlePlayerLeaveZone(ObjectGuid guid, uint32 zone);

        virtual void HandlePlayerEnterMap(ObjectGuid /*guid*/, uint32 /*zoneID*/) { }
        virtual void HandlePlayerLeaveMap(ObjectGuid /*guid*/, uint32 /*zoneID*/) { }

        virtual void HandlePlayerEnterArea(ObjectGuid /*guid*/, uint32 /*areaID*/);
        virtual void HandlePlayerLeaveArea(ObjectGuid /*guid*/, uint32 /*areaID*/);

        virtual void HandlePlayerResurrects(Player* player, uint32 zone);

        void AddCapturePoint(OPvPCapturePoint* cp);

        OPvPCapturePoint* GetCapturePoint(ObjectGuid guid) const;

        bool HasPlayer(Player* player) const;

        void TeamCastSpell(TeamId team, int32 spellId);

        bool AddCreature(uint32 p_Type, creature_type data, uint32 p_SpawnTime = 0);
        bool AddCreature(uint32 p_Type, uint32 p_Entry, uint32 p_Team, uint32 mapID, float p_X, float p_Y, float p_Z, float p_O, uint32 p_SpawnTime = 0);
        bool DelCreature(uint32 p_Type);

        bool AddObject(uint32 p_Type, go_type data);
        bool AddObject(uint32 p_Type, uint32 p_Entry, uint32 p_Map, float p_X, float p_Y, float p_Z, float p_O, float p_Rot0, float p_Rot1, float p_Rot2, float p_Rot3);
        bool DelObject(uint32 p_Type);

        std::map<ObjectGuid, OPvPCapturePoint*> m_capturePoints;

        GuidSet m_players[MAX_TEAMS];
        GuidSet m_playersInArea;
        sf::contention_free_shared_mutex< > i_playersInAreaLock;

        std::map<uint32, ObjectGuid> m_Creatures;
        std::map<ObjectGuid, uint32> m_CreatureTypes;
        std::map<uint32, ObjectGuid> m_Objects;
        std::map<ObjectGuid, uint32> m_ObjectTypes;

        GraveyardVector m_GraveyardList;

        Map* m_map;

        uint32 m_TypeId;
        uint32 m_LastResurectTimer;

        bool m_sendUpdate;
};

class OutdoorGraveyard
{
    public:
        explicit OutdoorGraveyard(OutdoorPvP* p_OutdoorPvP);

        void GiveControlTo(TeamId p_Team);
        TeamId GetControlTeamId();

        float GetDistance(Player* p_Player) const;

        void Initialize(TeamId p_Team, uint32 p_Graveyard);

        void SetSpirit(Creature* p_Spirit, TeamId p_Team);

        void AddPlayer(ObjectGuid guid);

        void RemovePlayer(ObjectGuid guid);

        void Resurrect();

        void RelocateDeadPlayers();

        bool HasNpc(ObjectGuid guid);

        bool HasPlayer(ObjectGuid guid);

        uint32 GetGraveyardId();

    protected:
        OutdoorPvP* m_OutdoorPvP;
        std::set<ObjectGuid> m_ResurrectQueue;
        ObjectGuid m_SpiritGuide[2];
        TeamId m_ControlTeam;
        uint32 m_GraveyardId;
};

#endif /*OUTDOOR_PVP_H_*/
