/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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

// TODO: Implement proper support for vehicle+player teleportation
// TODO: Use spell victory/defeat in wg instead of RewardMarkOfHonor() && RewardHonor
// TODO: Add proper implement of achievement

#include "BattlefieldWG.h"
#include "SpellAuras.h"
#include "Vehicle.h"
#include "GroupMgr.h"
#include "WorldStatePackets.h"

enum WGVehicles
{
    NPC_WG_SEIGE_ENGINE_ALLIANCE        = 28312,
    NPC_WG_SEIGE_ENGINE_HORDE           = 32627,
    NPC_WG_DEMOLISHER                   = 28094,
    NPC_WG_CATAPULT                     = 27881,
};

BattlefieldWG::~BattlefieldWG()
{
     for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
         delete *itr;

      for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
          delete *itr;
}

bool BattlefieldWG::SetupBattlefield()
{
    InitStalker(BATTLEFIELD_WG_NPC_STALKER, WintergraspStalkerPos[0], WintergraspStalkerPos[1], WintergraspStalkerPos[2], WintergraspStalkerPos[3]);

    m_TypeId = BATTLEFIELD_WG;                              // See enum BattlefieldTypes
    m_BattleId = BATTLEFIELD_BATTLEID_WG;
    m_AreaID = BATTLEFIELD_WG_ZONEID;
    m_MapId = BATTLEFIELD_WG_MAPID;

    m_MaxPlayer = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MAX);
    m_IsEnabled = sWorld->getBoolConfig(CONFIG_WINTERGRASP_ENABLE);
    m_MinPlayer = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MIN);
    m_MinLevel = sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MIN_LVL);
    m_BattleTime = sWorld->getIntConfig(CONFIG_WINTERGRASP_BATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_NoWarBattleTime = sWorld->getIntConfig(CONFIG_WINTERGRASP_NOBATTLETIME) * MINUTE * IN_MILLISECONDS;
    m_RestartAfterCrash = sWorld->getIntConfig(CONFIG_WINTERGRASP_RESTART_AFTER_CRASH) * MINUTE * IN_MILLISECONDS;

    m_TimeForAcceptInvite = 20;
    m_StartGroupingTimer = 15 * MINUTE * IN_MILLISECONDS;
    m_StartGrouping = false;

    m_tenacityStack = 0;

    KickPosition.Relocate(5728.117f, 2714.346f, 697.733f, 0.0f, 0.0f);
    KickPosition.m_mapId = m_MapId;

    RegisterZone(m_AreaID);

    m_Data32.resize(BATTLEFIELD_WG_DATA_MAX);

    m_saveTimer = 60000;

    // Init GraveYards
    SetGraveyardNumber(BATTLEFIELD_WG_GRAVEYARD_MAX);

    // Load from db
    if ((sWorld->getWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_ACTIVE) == 0) && (sWorld->getWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_DEFENDER) == 0)
            && (sWorld->getWorldState(ClockWorldState[0]) == 0))
    {
        sWorld->setWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_ACTIVE, uint64(false));
        sWorld->setWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_DEFENDER, uint64(urand(0, 1)));
        sWorld->setWorldState(ClockWorldState[0], uint64(m_NoWarBattleTime));
    }

    m_isActive = sWorld->getWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_ACTIVE) != 0;
    m_DefenderTeam = TeamId(sWorld->getWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_DEFENDER));

    m_Timer = sWorld->getWorldState(ClockWorldState[0]);
    if (m_isActive)
    {
        m_isActive = false;
        m_Timer = m_RestartAfterCrash;
    }

    for (uint8 i = 0; i < BATTLEFIELD_WG_GRAVEYARD_MAX; i++)
    {
        BfGraveyardWG* graveyard = new BfGraveyardWG(this);

        // When between games, the graveyard is controlled by the defending team
        if (WGGraveYard[i].startcontrol == TEAM_NEUTRAL)
            graveyard->Initialize(m_DefenderTeam, WGGraveYard[i].gyid, WGGraveYard[i].type);
        else
            graveyard->Initialize(WGGraveYard[i].startcontrol, WGGraveYard[i].gyid, WGGraveYard[i].type);

        graveyard->SetTextId(WGGraveYard[i].textid);
        m_GraveyardList[WGGraveYard[i].type] = graveyard;
    }


    // Spawn workshop creatures and gameobjects
    for (uint8 i = 0; i < WG_MAX_WORKSHOP; i++)
    {
        WGWorkshop* workshop = new WGWorkshop(this, i);
        if (i < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
            workshop->GiveControlTo(GetAttackerTeam(), true);
        else
            workshop->GiveControlTo(GetDefenderTeam(), true);

        // Note: Capture point is added once the gameobject is created.
        WorkshopsList.insert(workshop);
    }

    // Spawn NPCs in the defender's keep, both Horde and Alliance
    for (uint8 i = 0; i < WG_MAX_KEEP_NPC; i++)
    {
        // Horde npc
        if (Creature* creature = SpawnCreature(WGKeepNPC[i].entryHorde, WGKeepNPC[i].x, WGKeepNPC[i].y, WGKeepNPC[i].z, WGKeepNPC[i].o, TEAM_HORDE))
            KeepCreature[TEAM_HORDE].insert(creature->GetGUID());
        // Alliance npc
        if (Creature* creature = SpawnCreature(WGKeepNPC[i].entryAlliance, WGKeepNPC[i].x, WGKeepNPC[i].y, WGKeepNPC[i].z, WGKeepNPC[i].o, TEAM_ALLIANCE))
            KeepCreature[TEAM_ALLIANCE].insert(creature->GetGUID());
    }

    // Hide NPCs from the Attacker's team in the keep
    for (GuidSet::const_iterator itr = KeepCreature[GetAttackerTeam()].begin(); itr != KeepCreature[GetAttackerTeam()].end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // Spawn Horde NPCs outside the keep
    for (uint8 i = 0; i < WG_OUTSIDE_ALLIANCE_NPC; i++)
        if (Creature* creature = SpawnCreature(WGOutsideNPC[i].entryHorde, WGOutsideNPC[i].x, WGOutsideNPC[i].y, WGOutsideNPC[i].z, WGOutsideNPC[i].o, TEAM_HORDE))
            OutsideCreature[TEAM_HORDE].insert(creature->GetGUID());

    // Spawn Alliance NPCs outside the keep
    for (uint8 i = WG_OUTSIDE_ALLIANCE_NPC; i < WG_MAX_OUTSIDE_NPC; i++)
        if (Creature* creature = SpawnCreature(WGOutsideNPC[i].entryAlliance, WGOutsideNPC[i].x, WGOutsideNPC[i].y, WGOutsideNPC[i].z, WGOutsideNPC[i].o, TEAM_ALLIANCE))
            OutsideCreature[TEAM_ALLIANCE].insert(creature->GetGUID());

    // Hide units outside the keep that are defenders
    for (GuidSet::const_iterator itr = OutsideCreature[GetDefenderTeam()].begin(); itr != OutsideCreature[GetDefenderTeam()].end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    // Spawn turrets and hide them per default
    for (uint8 i = 0; i < WG_MAX_TURRET; i++)
    {
        Position towerCannonPos;
        WGTurret[i].GetPosition(&towerCannonPos);
        if (Creature* creature = SpawnCreature(NPC_TOWER_CANNON, towerCannonPos, TEAM_ALLIANCE))
        {
            CanonList.insert(creature->GetGUID());
            HideNpc(creature);
        }
    }

    // Spawn all gameobjects
    for (uint8 i = 0; i < WG_MAX_OBJ; i++)
    {
        GameObject* go = SpawnGameObject(WGGameObjectBuilding[i].entry, WGGameObjectBuilding[i].x, WGGameObjectBuilding[i].y, WGGameObjectBuilding[i].z, WGGameObjectBuilding[i].o);
        BfWGGameObjectBuilding* b = new BfWGGameObjectBuilding(this);
        b->Init(go, WGGameObjectBuilding[i].type, WGGameObjectBuilding[i].WorldState, WGGameObjectBuilding[i].nameId);
        if (!IsEnabled() && go->GetGOInfo()->entry == GO_WINTERGRASP_VAULT_GATE)
            go->SetDestructibleState(GO_DESTRUCTIBLE_DESTROYED);
        BuildingsInZone.insert(b);
    }

    // Spawning portal defender
    for (uint8 i = 0; i < WG_MAX_TELEPORTER; i++)
    {
        GameObject* go = SpawnGameObject(WGPortalDefenderData[i].entry, WGPortalDefenderData[i].x, WGPortalDefenderData[i].y, WGPortalDefenderData[i].z, WGPortalDefenderData[i].o);
        DefenderPortalList.insert(go);
        go->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, BfFactions[GetDefenderTeam()]);
    }

    // Spawn banners in the keep
    for (uint8 i = 0; i < WG_KEEPGAMEOBJECT_MAX; i++)
    {
        if (GameObject* go = SpawnGameObject(WGKeepGameObject[i].entryHorde, WGKeepGameObject[i].x, WGKeepGameObject[i].y, WGKeepGameObject[i].z, WGKeepGameObject[i].o))
        {
            go->SetRespawnTime(GetDefenderTeam()? RESPAWN_ONE_DAY : RESPAWN_IMMEDIATELY);
            m_KeepGameObject[1].insert(go);
        }
        if (GameObject* go = SpawnGameObject(WGKeepGameObject[i].entryAlliance, WGKeepGameObject[i].x, WGKeepGameObject[i].y, WGKeepGameObject[i].z, WGKeepGameObject[i].o))
        {
            go->SetRespawnTime(GetDefenderTeam()? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
            m_KeepGameObject[0].insert(go);
        }
    }

    // Show defender banner in keep
    for (GameObjectSet::const_iterator itr = m_KeepGameObject[GetDefenderTeam()].begin(); itr != m_KeepGameObject[GetDefenderTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);

    // Hide attackant banner in keep
    for (GameObjectSet::const_iterator itr = m_KeepGameObject[GetAttackerTeam()].begin(); itr != m_KeepGameObject[GetAttackerTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);

    UpdateCounterVehicle(true);
    return true;
}

bool BattlefieldWG::Update(uint32 diff)
{
    bool m_return = Battlefield::Update(diff);
    if (m_saveTimer <= diff)
    {
        sWorld->setWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_ACTIVE, m_isActive);
        sWorld->setWorldState((uint32)WorldStates::BATTLEFIELD_WG_WORLD_STATE_DEFENDER, m_DefenderTeam);
        sWorld->setWorldState(ClockWorldState[0], m_Timer);
        m_saveTimer = 60 * IN_MILLISECONDS;
    }
    else
        m_saveTimer -= diff;

    return m_return;
}

void BattlefieldWG::OnBattleStart()
{
    // Spawn titan relic
    m_titansRelic = SpawnGameObject(GO_WINTERGRASP_TITAN_S_RELIC, 5440.0f, 2840.8f, 430.43f, 0);
    if (m_titansRelic)
    {
        // Update faction of relic, only attacker can click on
        m_titansRelic->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, BfFactions[GetAttackerTeam()]);
        // Set in use (not allow to click on before last door is broken)
        m_titansRelic->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_IN_USE);
    }
    else
        TC_LOG_ERROR(LOG_FILTER_BATTLEFIELD, "WG: Failed to spawn titan relic.");


    // Update tower visibility and update faction
    for (GuidSet::const_iterator itr = CanonList.begin(); itr != CanonList.end(); ++itr)
    {
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
            {
                ShowNpc(creature, true);
                creature->setFaction(BfFactions[GetDefenderTeam()]);
            }
        }
    }

    // Rebuild all wall
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        if (*itr)
        {
            (*itr)->Rebuild();
            (*itr)->UpdateTurretAttack(false);
        }
    }

    SetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT, 0);
    SetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF, 0);
    SetData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, 0);
    SetData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, 0);

    // Update graveyard (in no war time all graveyard is to deffender, in war time, depend of base)
    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        if (*itr)
            (*itr)->UpdateGraveyardAndWorkshop();

    // Initialize vehicle counter
    UpdateCounterVehicle(true);
    // Send start warning to all players
    SendWarningToAllInZone(BATTLEFIELD_WG_TEXT_START);

    SendInitWorldStatesToAll();
}

void BattlefieldWG::UpdateCounterVehicle(bool init)
{
    if (init)
    {
        SetData(BATTLEFIELD_WG_DATA_VEHICLE_H, 0);
        SetData(BATTLEFIELD_WG_DATA_VEHICLE_A, 0);
    }
    SetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H, 0);
    SetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A, 0);

    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
    {
        if (WGWorkshop* workshop = (*itr))
        {
            if (workshop->teamControl == TEAM_ALLIANCE)
                UpdateData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A, 4);
            else if (workshop->teamControl == TEAM_HORDE)
                UpdateData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H, 4);
        }
    }

    UpdateVehicleCountWG();
}

void BattlefieldWG::OnBattleEnd(bool endByTimer)
{
    // Remove relic
    if (m_titansRelic)
        m_titansRelic->RemoveFromWorld();
    m_titansRelic = nullptr;

    // Remove turret
    for (GuidSet::const_iterator itr = CanonList.begin(); itr != CanonList.end(); ++itr)
    {
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
            {
                if (!endByTimer)
                    creature->setFaction(BfFactions[GetDefenderTeam()]);
                HideNpc(creature);
            }
        }
    }

    if (!endByTimer) // One player triggered the relic
    {
        // Change all npc in keep
        for (GuidSet::const_iterator itr = KeepCreature[GetAttackerTeam()].begin(); itr != KeepCreature[GetAttackerTeam()].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    HideNpc(creature);

        for (GuidSet::const_iterator itr = KeepCreature[GetDefenderTeam()].begin(); itr != KeepCreature[GetDefenderTeam()].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    ShowNpc(creature, true);

        // Change all npc out of keep
        for (GuidSet::const_iterator itr = OutsideCreature[GetDefenderTeam()].begin(); itr != OutsideCreature[GetDefenderTeam()].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    HideNpc(creature);

        for (GuidSet::const_iterator itr = OutsideCreature[GetAttackerTeam()].begin(); itr != OutsideCreature[GetAttackerTeam()].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    ShowNpc(creature, true);
    }

    // Rebuild all wall
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        if (*itr)
        {
            (*itr)->Rebuild();
            (*itr)->UpdateTurretAttack(false);
        }
    }

    // Update graveyard (in no war time all graveyard is to deffender, in war time, depend of base)
    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        if (*itr)
            (*itr)->UpdateGraveyardAndWorkshop();

    // Update all graveyard, control is to defender when no wartime
    for (uint8 i = 0; i < BATTLEFIELD_WG_GY_HORDE; i++)
        if (BfGraveyard* graveyard = GetGraveyardById(i))
            graveyard->GiveControlTo(GetDefenderTeam());

    for (GameObjectSet::const_iterator itr = m_KeepGameObject[GetDefenderTeam()].begin(); itr != m_KeepGameObject[GetDefenderTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_IMMEDIATELY);

    for (GameObjectSet::const_iterator itr = m_KeepGameObject[GetAttackerTeam()].begin(); itr != m_KeepGameObject[GetAttackerTeam()].end(); ++itr)
        (*itr)->SetRespawnTime(RESPAWN_ONE_DAY);

    // Update portal defender faction
    for (GameObjectSet::const_iterator itr = DefenderPortalList.begin(); itr != DefenderPortalList.end(); ++itr)
        (*itr)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, BfFactions[GetDefenderTeam()]);

    // Saving data
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        (*itr)->Save();
    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        (*itr)->Save();

    for (GuidSet::const_iterator itr = m_PlayersInWar[GetDefenderTeam()].begin(); itr != m_PlayersInWar[GetDefenderTeam()].end(); ++itr)
    {
        if (Player* player = sObjectAccessor->FindPlayer(*itr))
        {
            player->PlayDirectSound(GetDefenderTeam()==TEAM_ALLIANCE ? BG_SOUND_ALLIANCE_WIN : BG_SOUND_HORDE_WIN) ; // SoundOnEndWin
            player->CastSpell(player, SPELL_ESSENCE_OF_WINTERGRASP, true);
            //custom check
            //player->CastSpell(player, SPELL_VICTORY_REWARD, true);
            // Send Wintergrasp victory achievement
            DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG, player);
            // Award achievement for succeeding in Wintergrasp in 10 minutes or less
            if (!endByTimer && GetTimer() <= 10000)
                DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WIN_WG_TIMER_10, player);

            if (player->HasAura(SPELL_LIEUTENANT) || player->HasAura(SPELL_CORPORAL))
                player->AreaExploredOrEventHappens(WGQuest[player->GetTeamId()][1]);
        }
    }

    for (GuidSet::const_iterator itr = m_PlayersInWar[GetAttackerTeam()].begin(); itr != m_PlayersInWar[GetAttackerTeam()].end(); ++itr)
        if (Player* player = sObjectAccessor->FindPlayer(*itr))
        {
            player->PlayDirectSound(BG_SOUND_NEAR_VICTORY) ; // SoundOnEndLoose
            //player->CastSpell(player, SPELL_DEFEAT_REWARD, true);
        }

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        uint32 intactNum = 0;
        uint32 damagedNum = 0;
        for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        {
            if ((*itr)->teamControl != team)
                continue;

            if ((*itr)->state == BATTLEFIELD_WG_OBJECTSTATE_HORDE_INTACT || (*itr)->state == BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT)
                ++intactNum;
            if ((*itr)->state == BATTLEFIELD_WG_OBJECTSTATE_HORDE_DAMAGE || (*itr)->state == BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_DAMAGE)
                ++damagedNum;
        }
        for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
            {
                player->AddAura(SPELL_PHASE_NON_BATTLE, player);
                //custom reward
                uint32 marks = 0;
                if (player->HasAura(SPELL_LIEUTENANT))
                    marks = (team == GetDefenderTeam()) ? 3 : 1;
                else if (player->HasAura(SPELL_CORPORAL))
                    marks = (team == GetDefenderTeam()) ? 2 : 1;
                else
                    marks = (team == GetDefenderTeam()) ? 1 : 0;
                RemoveAurasFromPlayer(player);
                player->RewardHonor(nullptr, 1, (team == GetDefenderTeam()) ? 100 : 40);
                RewardMarkOfHonor(player, marks);
                player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, (team == GetDefenderTeam()) ? SPELL_VICTORY_REWARD : SPELL_DEFEAT_REWARD);

                // reward for our workshops not damaged
                for (uint32 i = 0; i < intactNum; ++i)
                    player->CastSpell(player, SPELL_INTACT_BUILDING, true);
                // reward for our workshops  damaged  and not destroyd
                for (uint32 i = 0; i < damagedNum; ++i)
                    player->CastSpell(player, SPELL_DAMAGED_BUILDING, true);

                uint32 damageType = 0;
                uint32 destrType = 0;
                if (team == GetDefenderTeam())
                {
                    damageType = endByTimer ? BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF : BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT;
                    destrType = endByTimer ? BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF : BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT;
                }
                else
                {
                    damageType = endByTimer ? BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT : BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF;
                    destrType = endByTimer ? BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT : BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF;
                }
                uint32 DamagedCount = GetData(damageType);
                uint32 DestrCount = GetData(destrType);

                // reward for damaged tower by our team
                for (uint32 i = 0; i < DamagedCount; ++i)
                    player->CastSpell(player, SPELL_DAMAGED_TOWER, true);
                // reward for destroyed tower by our team
                for (uint32 i = 0; i < DestrCount; ++i)
                    player->CastSpell(player, SPELL_DESTROYED_TOWER, true);
            }

        m_PlayersInWar[team].clear();

        for (GuidSet::const_iterator itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    creature->DespawnOrUnsummon();

        m_vehicles[team].clear();
    }

    //disband
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        for (GuidSet::const_iterator itr = m_Groups[team].begin(); itr != m_Groups[team].end();)
        {
            if (Group* group = sGroupMgr->GetGroupByGUID(*itr++))
                group->Disband(true);
        }
        m_Groups[team].clear();
    }

    if (!endByTimer)
    {
        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        {
            for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            {
                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                {
                    player->RemoveAurasDueToSpell(m_DefenderTeam == TEAM_ALLIANCE ? SPELL_HORDE_CONTROL_PHASE_SHIFT : SPELL_ALLIANCE_CONTROL_PHASE_SHIFT, player->GetGUID());
                    player->AddAura(m_DefenderTeam == TEAM_HORDE ? SPELL_HORDE_CONTROL_PHASE_SHIFT : SPELL_ALLIANCE_CONTROL_PHASE_SHIFT, player);
                }
            }
        }
    }

    if (!endByTimer) // win alli/horde
        SendWarningToAllInZone((GetDefenderTeam() == TEAM_ALLIANCE) ? BATTLEFIELD_WG_TEXT_WIN_KEEP : BATTLEFIELD_WG_TEXT_WIN_KEEP + 1);
    else // defend alli/horde
        SendWarningToAllInZone((GetDefenderTeam() == TEAM_ALLIANCE) ? BATTLEFIELD_WG_TEXT_DEFEND_KEEP : BATTLEFIELD_WG_TEXT_DEFEND_KEEP + 1);
}

// *******************************************************
// ******************* Reward System *********************
// *******************************************************
void BattlefieldWG::DoCompleteOrIncrementAchievement(uint32 achievement, Player* player, uint8 /*incrementNumber*/)
{
    AchievementEntry const* achievementEntry = sAchievementStore.LookupEntry(achievement);

    if (!achievementEntry)
        return;

    switch (achievement)
    {
        case ACHIEVEMENTS_WIN_WG_100:
        {
            // player->GetAchievementMgr()->UpdateAchievementCriteria();
        }
        default:
        {
            if (player)
                player->CompletedAchievement(achievementEntry);
            break;
        }
    }

}

void BattlefieldWG::OnStartGrouping()
{
    SendWarningToAllInZone(BATTLEFIELD_WG_TEXT_WILL_START);
}

uint8 BattlefieldWG::GetSpiritGraveyardId(uint32 areaId) const
{
    switch (areaId)
    {
        case AREA_WINTERGRASP_FORTRESS:
            return BATTLEFIELD_WG_GY_KEEP;
        case AREA_THE_SUNKEN_RING:
            return BATTLEFIELD_WG_GY_WORKSHOP_NE;
        case AREA_THE_BROKEN_TEMPLATE:
            return BATTLEFIELD_WG_GY_WORKSHOP_NW;
        case AREA_WESTPARK_WORKSHOP:
            return BATTLEFIELD_WG_GY_WORKSHOP_SW;
        case AREA_EASTPARK_WORKSHOP:
            return BATTLEFIELD_WG_GY_WORKSHOP_SE;
        case AREA_WINTERGRASP:
            return BATTLEFIELD_WG_GY_ALLIANCE;
        case AREA_THE_CHILLED_QUAGMIRE:
            return BATTLEFIELD_WG_GY_HORDE;
        default:
            TC_LOG_ERROR(LOG_FILTER_BATTLEFIELD, "BattlefieldWG::GetSpiritGraveyardId: Unexpected Area Id %u", areaId);
            break;
    }

    return 0;
}

void BattlefieldWG::OnCreatureCreate(Creature* creature)
{
    // Accessing to db spawned creatures
    switch (creature->GetEntry())
    {
        case NPC_DWARVEN_SPIRIT_GUIDE:
        case NPC_TAUNKA_SPIRIT_GUIDE:
        {
            TeamId teamId = (creature->GetEntry() == NPC_DWARVEN_SPIRIT_GUIDE ? TEAM_ALLIANCE : TEAM_HORDE);
            uint8 graveyardTypeId = GetSpiritGraveyardId(creature->GetAreaId());
            if (BfGraveyard *gr = GetGraveyardById(graveyardTypeId))
                gr->SetSpirit(creature, teamId);
            break;
        }
    }

    // untested code - not sure if it is valid.
    if (IsWarTime())
    {
        switch (creature->GetEntry())
        {
            case NPC_WINTERGRASP_SIEGE_ENGINE_ALLIANCE:
            case NPC_WINTERGRASP_SIEGE_ENGINE_HORDE:
            case NPC_WINTERGRASP_CATAPULT:
            case NPC_WINTERGRASP_DEMOLISHER:
            {
                if (creature->ToTempSummon()->GetSummonerGUID().IsEmpty() || !sObjectAccessor->FindPlayer(creature->ToTempSummon()->GetSummonerGUID()))
                {
                    creature->setDeathState(DEAD);
                    creature->RemoveFromWorld();
                    return;
                }
                Player* creator = sObjectAccessor->FindPlayer(creature->ToTempSummon()->GetSummonerGUID());
                TeamId team = creator->GetTeamId();

                if (team == TEAM_HORDE)
                {
                    if (GetData(BATTLEFIELD_WG_DATA_VEHICLE_H) < GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H))
                    {
                        UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_H, 1);
                        creature->AddAura(SPELL_HORDE_FLAG, creature);
                        m_vehicles[team].insert(creature->GetGUID());
                        UpdateVehicleCountWG();
                    }
                    else
                    {
                        creature->setDeathState(DEAD);
                        creature->RemoveFromWorld();
                        return;
                    }
                }
                else
                {
                    if (GetData(BATTLEFIELD_WG_DATA_VEHICLE_A) < GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A))
                    {
                        UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_A, 1);
                        creature->AddAura(SPELL_ALLIANCE_FLAG, creature);
                        m_vehicles[team].insert(creature->GetGUID());
                        UpdateVehicleCountWG();
                    }
                    else
                    {
                        creature->setDeathState(DEAD);
                        creature->RemoveFromWorld();
                        return;
                    }
                }

                creature->CastSpell(creator, SPELL_GRAB_PASSENGER, true);
                break;
            }
        }
    }
}

void BattlefieldWG::OnCreatureRemove(Creature* /*creature*/)
{
/* possibly can be used later
    if (IsWarTime())
    {
        switch (creature->GetEntry())
        {
            case NPC_WINTERGRASP_SIEGE_ENGINE_ALLIANCE:
            case NPC_WINTERGRASP_SIEGE_ENGINE_HORDE:
            case NPC_WINTERGRASP_CATAPULT:
            case NPC_WINTERGRASP_DEMOLISHER:
            {
                uint8 team;
                if (creature->getFaction() == BfFactions[TEAM_ALLIANCE])
                    team = TEAM_ALLIANCE;
                else if (creature->getFaction() == BfFactions[TEAM_HORDE])
                    team = TEAM_HORDE;
                else
                    return;

                m_vehicles[team].erase(creature->GetGUID());
                if (team == TEAM_HORDE)
                    UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_H, -1);
                else
                    UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_A, -1);
                UpdateVehicleCountWG();

                break;
            }
        }
    }*/
}

void BattlefieldWG::OnGameObjectCreate(GameObject* go)
{
    uint8 workshopId = 0;

    switch (go->GetEntry())
    {
        case GO_WINTERGRASP_FACTORY_BANNER_NE:
            workshopId = BATTLEFIELD_WG_WORKSHOP_NE;
            break;
        case GO_WINTERGRASP_FACTORY_BANNER_NW:
            workshopId = BATTLEFIELD_WG_WORKSHOP_NW;
            break;
        case GO_WINTERGRASP_FACTORY_BANNER_SE:
            workshopId = BATTLEFIELD_WG_WORKSHOP_SE;
            break;
        case GO_WINTERGRASP_FACTORY_BANNER_SW:
            workshopId = BATTLEFIELD_WG_WORKSHOP_SW;
            break;
     default:
         return;
    }

    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
    {
        if (WGWorkshop* workshop = (*itr))
        {
            if (workshop->workshopId == workshopId)
            {
                WintergraspCapturePoint* capturePoint = new WintergraspCapturePoint(this, GetAttackerTeam());

                capturePoint->SetCapturePointData(go);
                capturePoint->LinkToWorkshop(workshop);
                AddCapturePoint(capturePoint);
                break;
            }
        }
    }
}

// Called when player kill a unit in wg zone
void BattlefieldWG::HandleKill(Player* killer, Unit* victim)
{
    if (killer == victim)
        return;

    bool again = false;
    TeamId killerTeam = killer->GetTeamId();

    if (victim->IsPlayer())
    {
        if (victim->getLevel() < 70)
            return;

        for (GuidSet::const_iterator itr = m_PlayersInWar[killerTeam].begin(); itr != m_PlayersInWar[killerTeam].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                if (player->GetDistance2d(killer) < 40)
                {
                    player->RewardPlayerAndGroupAtEvent(CRE_PVP_KILL, victim);
                    PromotePlayer(player);
                }
        return;
    }

    switch (victim->GetEntry())
    {
        case NPC_WINTERGRASP_SIEGE_ENGINE_ALLIANCE:
        case NPC_WINTERGRASP_SIEGE_ENGINE_HORDE:
            killer->RewardPlayerAndGroupAtEvent(CRE_PVP_KILL_V, victim);
            PromotePlayer(killer);
            return;
        default:
            break;
    }

    for (GuidSet::const_iterator itr = KeepCreature[MS::Battlegrounds::GetOtherTeamID(killerTeam)].begin(); itr != KeepCreature[MS::Battlegrounds::GetOtherTeamID(killerTeam)].end(); ++itr)
    {
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
            {
                if (victim->GetEntry() == creature->GetEntry() && !again)
                {
                    again = true;
                    for (GuidSet::const_iterator iter = m_PlayersInWar[killerTeam].begin(); iter != m_PlayersInWar[killerTeam].end(); ++iter)
                        if (Player* player = sObjectAccessor->FindPlayer(*iter))
                            if (player->GetDistance2d(killer) < 40.0f)
                            {
                                player->RewardPlayerAndGroupAtEvent(CRE_PVP_KILL, victim);
                                PromotePlayer(player);
                            }
                }
            }
        }
    }
    // TODO:Recent PvP activity worldstate
}

bool BattlefieldWG::FindAndRemoveVehicleFromList(Unit* vehicle)
{
    for (uint8 i = TEAM_ALLIANCE; i < MAX_TEAMS; ++i)
    {
        if (m_vehicles[i].find(vehicle->GetGUID()) != m_vehicles[i].end())
        {
            m_vehicles[i].erase(vehicle->GetGUID());
            if (i == TEAM_HORDE)
                UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_H, -1);
            else
                UpdateData(BATTLEFIELD_WG_DATA_VEHICLE_A, -1);
            return true;
        }
    }
    return false;
}

void BattlefieldWG::OnUnitDeath(Unit* unit)
{
    if (IsWarTime())
        if (unit->IsVehicle())
            if (FindAndRemoveVehicleFromList(unit))
                UpdateVehicleCountWG();
}

// Update rank for player
void BattlefieldWG::PromotePlayer(Player* killer)
{
    if (!m_isActive)
        return;
    // Updating rank of player
    Aura* aur = killer->GetAura(SPELL_RECRUIT);
    if (aur != nullptr)
    {
        if (aur->GetStackAmount() >= 5)
        {
            killer->RemoveAura(SPELL_RECRUIT);
            killer->CastSpell(killer, SPELL_CORPORAL, true);
            SendWarningToPlayer(killer, BATTLEFIELD_WG_TEXT_FIRSTRANK);
        }
        else
            killer->CastSpell(killer, SPELL_RECRUIT, true);
    }
    else
    {
        Aura* aur = killer->GetAura(SPELL_CORPORAL);
        if (aur != nullptr)
        {
            if (aur->GetStackAmount() >= 5)
            {
                killer->RemoveAura(SPELL_CORPORAL);
                killer->CastSpell(killer, SPELL_LIEUTENANT, true);
                SendWarningToPlayer(killer, BATTLEFIELD_WG_TEXT_SECONDRANK);
            }
            else
                killer->CastSpell(killer, SPELL_CORPORAL, true);
        }
    }
}

void BattlefieldWG::RemoveAurasFromPlayer(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_PHASE_NON_BATTLE, player->GetGUID());
    player->RemoveAurasDueToSpell(SPELL_RECRUIT);
    player->RemoveAurasDueToSpell(SPELL_CORPORAL);
    player->RemoveAurasDueToSpell(SPELL_LIEUTENANT);
    player->RemoveAurasDueToSpell(SPELL_TOWER_CONTROL);
    player->RemoveAurasDueToSpell(SPELL_SPIRITUAL_IMMUNITY);
    player->RemoveAurasDueToSpell(SPELL_TENACITY);
    player->RemoveAurasDueToSpell(SPELL_ESSENCE_OF_WINTERGRASP);
    player->RemoveAurasDueToSpell(SPELL_WINTERGRASP_RESTRICTED_FLIGHT_AREA);
}

void BattlefieldWG::OnPlayerJoinWar(Player* player)
{
    RemoveAurasFromPlayer(player);

    player->CastSpell(player, SPELL_RECRUIT, true);
    player->CastSpell(player, SPELL_WINTERGRASP_RESTRICTED_FLIGHT_AREA, true);

    bool onWg = player->GetZoneId() == m_AreaID; 
    player->PlayDirectSound(BG_SOUND_START); // START Battle

    // resurect dead plr
    if(!player->isAlive())
        player->ResurrectPlayer(1.0f);

    if (player->GetTeamId() == GetDefenderTeam())
    {
        if (!onWg || player->GetPositionX() < POS_X_CENTER)
            player->TeleportTo(571, 5345, 2842, 410, 3.14f);
    }
    else if (!onWg || player->GetPositionX() > POS_X_CENTER)
    {
        if (player->GetTeamId() == TEAM_HORDE)
            player->TeleportTo(571, 5025.857422f, 3674.628906f, 362.737122f, 4.135169f);
        else
            player->TeleportTo(571, 5101.284f, 2186.564f, 373.549f, 3.812f);
    }

    UpdateTenacity();

    if (player->GetTeamId() == GetAttackerTeam())
    {
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) < 3)
            player->SetAuraStack(SPELL_TOWER_CONTROL, player, 3 - GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT));
    }
    else
    {
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) > 0)
           player->SetAuraStack(SPELL_TOWER_CONTROL, player, GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT));
    }
}

void BattlefieldWG::OnPlayerLeaveWar(Player* player)
{
    // Remove all aura from WG // TODO: false we can go out of this zone on retail and keep Rank buff, remove on end of WG
    if (!player->GetSession()->PlayerLogout())
    {
        if (Creature* vehicle = player->GetVehicleCreatureBase())   // Remove vehicle of player if he go out.
            vehicle->DespawnOrUnsummon();

        RemoveAurasFromPlayer(player);
    }

    player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROL_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROL_PHASE_SHIFT);

    Battlefield::OnPlayerLeaveWar(player);
}

void BattlefieldWG::OnPlayerLeaveZone(Player* player)
{
    if (!m_isActive)
        RemoveAurasFromPlayer(player);

    player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROLS_FACTORY_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_HORDE_CONTROL_PHASE_SHIFT);
    player->RemoveAurasDueToSpell(SPELL_ALLIANCE_CONTROL_PHASE_SHIFT);
}

void BattlefieldWG::OnPlayerEnterZone(Player* player)
{
    if (!m_isActive)
        RemoveAurasFromPlayer(player);

    if (!IsWarTime())
        player->AddAura(SPELL_PHASE_NON_BATTLE, player);

    player->AddAura(m_DefenderTeam == TEAM_HORDE ? SPELL_HORDE_CONTROL_PHASE_SHIFT : SPELL_ALLIANCE_CONTROL_PHASE_SHIFT, player);
}

uint32 BattlefieldWG::GetData(uint32 data) const
{
    switch (data)
    {
        // Used to determine when the phasing spells must be casted
        // See: SpellArea::IsFitToRequirements
        case AREA_THE_SUNKEN_RING:
        case AREA_THE_BROKEN_TEMPLATE:
        case AREA_WESTPARK_WORKSHOP:
        case AREA_EASTPARK_WORKSHOP:
            // Graveyards and Workshops are controlled by the same team.
            if (m_GraveyardList[GetSpiritGraveyardId(data)])
                return m_GraveyardList[GetSpiritGraveyardId(data)]->GetControlTeamId();
    }

    if (data > BATTLEFIELD_WG_DATA_MAX)
        return 255;   // 0..1 - team fractions. used on some checks.

    return Battlefield::GetData(data);
}

// Method sending worldsate to player
void BattlefieldWG::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_ATTACKER, GetAttackerTeam());
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_DEFENDER, GetDefenderTeam());
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_ACTIVE, IsWarTime()? 0 : 1); // Note: cleanup these two, their names look awkward
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_SHOW_WORLDSTATE, IsWarTime() ? 1 : 0);

    packet.Worldstates.emplace_back(static_cast<WorldStates>(ClockWorldState[0]), IsWarTime() ? (uint32(time(nullptr) + (m_Timer / 1000))) : 0);
    packet.Worldstates.emplace_back(static_cast<WorldStates>(ClockWorldState[1]), !IsWarTime() ? (uint32(time(nullptr) + (m_Timer / 1000))) : 0);

    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_VEHICLE_H));
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_MAX_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H));
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_VEHICLE_A));
    packet.Worldstates.emplace_back(WorldStates::BATTLEFIELD_WG_WORLD_STATE_MAX_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A));

    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState), (*itr)->m_State);

    for (Workshop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        if (*itr)
            packet.Worldstates.emplace_back(static_cast<WorldStates>(WorkshopsData[(*itr)->workshopId].worldstate), (*itr)->state);
}

void BattlefieldWG::BrokenWallOrTower(TeamId team)
{
    // might be some use for this in the future. old code commented out below. KL
    if (team == GetDefenderTeam())
    {
        for (GuidSet::const_iterator itr = m_PlayersInWar[GetAttackerTeam()].begin(); itr != m_PlayersInWar[GetAttackerTeam()].end(); ++itr)
        {
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                player->AreaExploredOrEventHappens(WGQuest[player->GetTeamId()][2]);
                //IncrementQuest(player, WGQuest[player->GetTeamId()][2], true);
        }
    }
}

// Called when a tower is broke
void BattlefieldWG::UpdatedDestroyedTowerCount(TeamId team)
{
    // Destroy an attack tower
    if (team == GetAttackerTeam())
    {
        // Update counter
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, -1);
        UpdateData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT, 1);

        // Remove buff stack on attackers
        for (GuidSet::const_iterator itr = m_PlayersInWar[GetAttackerTeam()].begin(); itr != m_PlayersInWar[GetAttackerTeam()].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                player->RemoveAuraFromStack(SPELL_TOWER_CONTROL);

        // Add buff stack to defenders
        for (GuidSet::const_iterator itr = m_PlayersInWar[GetDefenderTeam()].begin(); itr != m_PlayersInWar[GetDefenderTeam()].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
            {
                player->CastSpell(player, SPELL_TOWER_CONTROL, true);
                DoCompleteOrIncrementAchievement(ACHIEVEMENTS_WG_TOWER_DESTROY, player);
            }

        // If all three south towers are destroyed (ie. all attack towers), remove ten minutes from battle time
        if (GetData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT) == 3)
        {
            if (int32(m_Timer - 600000) < 0)
                m_Timer = 0;
            else
                m_Timer -= 600000;
            SendInitWorldStatesToAll();
        }
    }
    else
    {
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, -1);
        UpdateData(BATTLEFIELD_WG_DATA_BROKEN_TOWER_DEF, 1);
    }
}

void BattlefieldWG::ProcessEvent(WorldObject *obj, uint32 eventId)
{
    if (!obj || !IsWarTime())
        return;

    // We handle only gameobjects here
    GameObject* go = obj->ToGameObject();
    if (!go)
        return;

    // On click on titan relic
    if (go->GetEntry() == GO_WINTERGRASP_TITAN_S_RELIC)
    {
        if (CanInteractWithRelic())
            EndBattle(false);
        else
            GetRelic()->SetRespawnTime(RESPAWN_IMMEDIATELY);
    }

    // if destroy or damage event, search the wall/tower and update worldstate/send warning message
    for (GameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        if (go->GetEntry() == (*itr)->m_Build->GetEntry())
        {
            if ((*itr)->m_Build->GetGOInfo()->destructibleBuilding.DamagedEvent == eventId)
            {
                (*itr)->Damaged();
                switch((*itr)->m_Type)
                {
                    case BATTLEFIELD_WG_OBJECTTYPE_WALL:
                        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                            for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
                                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                                    player->PlayDirectSound(GetDefenderTeam()==TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_ASSAULT_ALLIANCE : BG_SOUND_CAPTURE_POINT_ASSAULT_HORDE) ; // Wintergrasp Fortress under Siege
                        break;
                    case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
                        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                            for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
                                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                                    player->PlayDirectSound(GetDefenderTeam()==TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE : BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE) ; // Wintergrasp Fortress under Siege
                        break;
                }
            }

            if ((*itr)->m_Build->GetGOInfo()->destructibleBuilding.DestroyedEvent == eventId)
            {
                (*itr)->Destroyed();
                switch((*itr)->m_Type)
                {
                    case BATTLEFIELD_WG_OBJECTTYPE_WALL:
                        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                            for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
                                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                                    player->PlayDirectSound(GetDefenderTeam()==TEAM_ALLIANCE ? BG_SOUND_CAPTURE_POINT_CAPTURED_HORDE : BG_SOUND_CAPTURE_POINT_CAPTURED_ALLIANCE) ; // Wintergrasp Fortress under Siege
                        break;
                    case BATTLEFIELD_WG_OBJECTTYPE_TOWER:
                        for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
                            for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
                                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                                    player->PlayDirectSound(GetDefenderTeam()==TEAM_ALLIANCE ? BG_SOUND_FLAG_PLACED_HORDE : BG_SOUND_FLAG_PLACED_ALLIANCE) ; // Wintergrasp Fortress under Siege
                        break;
                }
            }

            break;
        }
    }
}

// Called when a tower is damaged, used for honor reward calcul
void BattlefieldWG::UpdateDamagedTowerCount(TeamId team)
{
    if (team == GetAttackerTeam())
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT, 1);
    else
        UpdateData(BATTLEFIELD_WG_DATA_DAMAGED_TOWER_DEF, 1);
}

// Update vehicle count WorldState to player
void BattlefieldWG::UpdateVehicleCountWG()
{
    SendUpdateWorldState(WorldStates::BATTLEFIELD_WG_WORLD_STATE_VEHICLE_H,     GetData(BATTLEFIELD_WG_DATA_VEHICLE_H));
    SendUpdateWorldState(WorldStates::BATTLEFIELD_WG_WORLD_STATE_MAX_VEHICLE_H, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_H));
    SendUpdateWorldState(WorldStates::BATTLEFIELD_WG_WORLD_STATE_VEHICLE_A,     GetData(BATTLEFIELD_WG_DATA_VEHICLE_A));
    SendUpdateWorldState(WorldStates::BATTLEFIELD_WG_WORLD_STATE_MAX_VEHICLE_A, GetData(BATTLEFIELD_WG_DATA_MAX_VEHICLE_A));
}

void BattlefieldWG::UpdateTenacity()
{
    TeamId team = TEAM_NEUTRAL;
    uint32 alliancePlayers = m_PlayersInWar[TEAM_ALLIANCE].size();
    uint32 hordePlayers = m_PlayersInWar[TEAM_HORDE].size();
    int32 newStack = 0;

    if (alliancePlayers && hordePlayers)
    {
        if (alliancePlayers < hordePlayers)
            newStack = int32((float(hordePlayers / alliancePlayers) - 1) * 4);  // positive, should cast on alliance
        else if (alliancePlayers > hordePlayers)
            newStack = int32((1 - float(alliancePlayers / hordePlayers)) * 4);  // negative, should cast on horde
    }

    if (newStack == int32(m_tenacityStack))
        return;

    if (m_tenacityStack > 0 && newStack <= 0)               // old buff was on alliance
        team = TEAM_ALLIANCE;
    else if (newStack >= 0)                                 // old buff was on horde
        team = TEAM_HORDE;

    m_tenacityStack = newStack;
    // Remove old buff
    if (team != TEAM_NEUTRAL)
    {
        for (GuidSet::const_iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                if (player->getLevel() >= m_MinLevel)
                    player->RemoveAurasDueToSpell(SPELL_TENACITY);

        for (GuidSet::const_iterator itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    creature->RemoveAurasDueToSpell(SPELL_TENACITY_VEHICLE);
    }

    // Apply new buff
    if (newStack)
    {
        team = newStack > 0 ? TEAM_ALLIANCE : TEAM_HORDE;

        if (newStack < 0)
            newStack = -newStack;
        if (newStack > 20)
            newStack = 20;

        uint32 buff_honor = SPELL_GREATEST_HONOR;
        if (newStack < 15)
            buff_honor = SPELL_GREATER_HONOR;
        if (newStack < 10)
            buff_honor = SPELL_GREAT_HONOR;
        if (newStack < 5)
            buff_honor = 0;

        for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(*itr))
                player->SetAuraStack(SPELL_TENACITY, player, newStack);

        for (GuidSet::const_iterator itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
            if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                if (Creature* creature = unit->ToCreature())
                    creature->SetAuraStack(SPELL_TENACITY_VEHICLE, creature, newStack);

        if (buff_honor != 0)
        {
            for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
                if (Player* player = sObjectAccessor->FindPlayer(*itr))
                    player->CastSpell(player, buff_honor, true);
            for (GuidSet::const_iterator itr = m_vehicles[team].begin(); itr != m_vehicles[team].end(); ++itr)
                if (Unit* unit = sObjectAccessor->FindUnit(*itr))
                    if (Creature* creature = unit->ToCreature())
                        creature->CastSpell(creature, buff_honor, true);
        }
    }
}

void BattlefieldWG::RewardMarkOfHonor(Player *plr, uint32 count)
{
    // 'Inactive' this aura prevents the player from gaining honor points and battleground tokens
    if (plr->HasAura(SPELL_BG_AURA_PLAYER_INACTIVE))
        return;
    if (count == 0)
        return;

    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, WG_MARK_OF_HONOR, count, &no_space_count);

    if (msg == EQUIP_ERR_ITEM_NOT_FOUND)
    {
        TC_LOG_ERROR(LOG_FILTER_BATTLEFIELD, "Wintergrasp reward item (Entry %u) not exist in `item_template`.", WG_MARK_OF_HONOR);
        return;
    }

    if (msg != EQUIP_ERR_OK) // convert to possible store amount
        count -= no_space_count;

    if (count != 0 && !dest.empty()) // can add some
        if (Item* item = plr->StoreNewItem(dest, WG_MARK_OF_HONOR, true))
            plr->SendNewItem(item, count, true, false);
}

WintergraspCapturePoint::WintergraspCapturePoint(BattlefieldWG* battlefield, TeamId teamInControl) : BfCapturePoint(battlefield)
{
    m_Bf = battlefield;
    m_team = teamInControl;
    m_Workshop = nullptr;
}

void WintergraspCapturePoint::ChangeTeam(TeamId /*oldTeam*/)
{
    ASSERT(m_Workshop);
    m_Workshop->GiveControlTo(m_team, false);
}

BfGraveyardWG::BfGraveyardWG(BattlefieldWG* battlefield) : BfGraveyard(battlefield), m_GossipTextId(0)
{
    m_Bf = battlefield;
}


void WGWorkshop::GiveControlTo(uint8 team, bool init /* for first call in setup*/)
{
    switch (team)
    {
        case BATTLEFIELD_WG_TEAM_NEUTRAL:
        {
            // Send warning message to all player to inform a faction attack to a workshop
            // alliance / horde attacking a workshop
            bf->SendWarningToAllInZone(teamControl ? WorkshopsData[workshopId].text : WorkshopsData[workshopId].text + 1);
            break;
        }
        case BATTLEFIELD_WG_TEAM_ALLIANCE:
        case BATTLEFIELD_WG_TEAM_HORDE:
        {
            // Updating worldstate
            state = team == BATTLEFIELD_WG_TEAM_ALLIANCE ? BATTLEFIELD_WG_OBJECTSTATE_ALLIANCE_INTACT : BATTLEFIELD_WG_OBJECTSTATE_HORDE_INTACT;
            bf->SendUpdateWorldState(WorkshopsData[workshopId].worldstate, state);

            // Warning message
            if (!init)                              // workshop taken - alliance
                bf->SendWarningToAllInZone(team == BATTLEFIELD_WG_TEAM_ALLIANCE ? WorkshopsData[workshopId].text : WorkshopsData[workshopId].text+1);

            // Found associate graveyard and update it
            if (workshopId < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
                if (bf->GetGraveyardById(workshopId))
                    bf->GetGraveyardById(workshopId)->GiveControlTo(team == BATTLEFIELD_WG_TEAM_ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE);

            teamControl = team;
            break;
        }
    }

    if (!init)
        bf->UpdateCounterVehicle(false);
}

void WGWorkshop::UpdateGraveyardAndWorkshop()
{
    if (workshopId < BATTLEFIELD_WG_WORKSHOP_KEEP_WEST)
    {
        bf->GetGraveyardById(workshopId)->GiveControlTo(bf->GetAttackerTeam());
        GiveControlTo(bf->GetAttackerTeam(), true);
    }
    else
    {
        bf->GetGraveyardById(workshopId)->GiveControlTo(bf->GetDefenderTeam());
        GiveControlTo(bf->GetDefenderTeam(), true);
    }
}
