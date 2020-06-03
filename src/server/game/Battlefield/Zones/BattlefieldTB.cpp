/*
 * Copyright (C) 2011-2012 Haloperidolum <http://wow-mig.ru/>
 * This is private source based on TrinityCore
 * for WoW-Mig project
 * This is no GPL code.
 */

#include "BattlefieldTB.h"
#include "WorldStatePackets.h"

void genNumbers(uint32 &a, uint32 &b)
{
    a = b = urand(1, 4);
    b = a; // little hack
    do
        b = urand(1, 4);
    while (a == b);
}

bool BattlefieldTB::SetupBattlefield()
{
    m_TypeId = BATTLEFIELD_TB;                                                           //View enum BattlefieldTypes
    m_BattleId = BATTLEFIELD_BATTLEID_TB;
    m_AreaID = 5095;                                                                     // Tol Barad
    m_MapId = 732;                                                                       // Map X

    m_MaxPlayer = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MAX);
    m_IsEnabled = sWorld->getBoolConfig(CONFIG_TOL_BARAD_ENABLE);
    m_MinPlayer = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MIN);
    m_MinLevel = sWorld->getIntConfig(CONFIG_TOL_BARAD_PLR_MIN_LVL);
    m_BattleTime = sWorld->getIntConfig(CONFIG_TOL_BARAD_BATTLETIME)*60*1000;            // Time of battle (in ms)
    m_NoWarBattleTime = sWorld->getIntConfig(CONFIG_TOL_BARAD_NOBATTLETIME)*60*1000;     //Time between to battle (in ms)

    m_TimeForAcceptInvite = 20;                                                          //in second
    m_StartGroupingTimer = 15*60*1000;                                                   //in ms
    m_StartGrouping=false;
    KickPosition.Relocate(5728.117f, 2714.346f, 697.733f, 0.0f, 0.0f);
    KickPosition.m_mapId = m_MapId;
    RegisterZone(m_AreaID);
    m_Data32.resize(BATTLEFIELD_TB_DATA_MAX);
    m_saveTimer = 60000;
    
    m_Data32[BATTLEFIELD_TB_DATA_CAPTURED] = 0;
  
    // Init GraveYards
    SetGraveyardNumber(BATTLEFIELD_TB_GY_MAX);

    //Load from db
    if ((sWorld->getWorldState((uint32)WorldStates::WS_TB_NEXT_BATTLE_TIMER_ENABLED) == 0) && (sWorld->getWorldState((uint32)WorldStates::WS_TB_HORDE_DEFENCE) == 0) && (sWorld->getWorldState((uint32)ClockBTWorldState[0]) == 0))
    {
        sWorld->setWorldState((uint32)WorldStates::WS_TB_NEXT_BATTLE_TIMER_ENABLED,false);
        sWorld->setWorldState((uint32)WorldStates::WS_TB_HORDE_DEFENCE, urand(0, 1));
        sWorld->setWorldState((uint32)ClockBTWorldState[0],m_NoWarBattleTime);
    }

    if (sWorld->getWorldState(20011) == 0)
        sWorld->setWorldState(20011, time(nullptr) + 86400);

    m_isActive = sWorld->getWorldState((uint32)WorldStates::WS_TB_NEXT_BATTLE_TIMER_ENABLED);
    m_DefenderTeam = (TeamId)sWorld->getWorldState((uint32)WorldStates::WS_TB_HORDE_DEFENCE);
    m_Timer = sWorld->getWorldState((uint32)ClockBTWorldState[0]);

    if(m_isActive)
    {
        m_isActive = false;
        m_Timer = 10 * 60 * 1000;
    }

    InitStalker(43679, -1227.28f, 974.37f, 119.63f, 6.21f);

    for (uint8 i = 0; i < BATTLEFIELD_TB_GY_MAX; i++)
    {
        BfGraveYardTB* gy = new BfGraveYardTB(this);
        gy->Initialize(m_DefenderTeam, TBGraveYard[i].gyid, TBGraveYard[i].type);
        m_GraveyardList[TBGraveYard[i].type] = gy;
    }

    for (uint8 i = 0; i < TB_MAX_WORKSHOP; i++)
    {
        BfTBWorkShopData* ws = new BfTBWorkShopData(this);

        //Init:setup variable
        ws->Init((uint32)TBWorkShopDataBase[i].worldstate, TBWorkShopDataBase[i].type, TBWorkShopDataBase[i].nameid1, TBWorkShopDataBase[i].nameid2);
        ws->ChangeControl(GetDefenderTeam(), true);

        WorkshopsList.insert(ws);
    }

    for (uint8 i = 0; i < TB_MAX_DESTROY_MACHINE_NPC; i++)
        if (Creature* creature = SpawnCreature(TBDestroyMachineNPC[i].entrya, TBDestroyMachineNPC[i].x, TBDestroyMachineNPC[i].y, TBDestroyMachineNPC[i].z, TBDestroyMachineNPC[i].o, GetAttackerTeam()))
        {
            HideNpc(creature);
            Vehicles.insert(creature->GetGUID());
        }

    //Spawning Buiding
    for (uint8 i = 0; i < TB_MAX_OBJ; i++)
    {
        GameObject* go = SpawnGameObject(TBGameObjectBuillding[i].entry, TBGameObjectBuillding[i].x, TBGameObjectBuillding[i].y, TBGameObjectBuillding[i].z, TBGameObjectBuillding[i].o);
        BfTBGameObjectBuilding* b = new BfTBGameObjectBuilding(this);
        b->Init(go,TBGameObjectBuillding[i].type,TBGameObjectBuillding[i].WorldState,TBGameObjectBuillding[i].nameid);
        b->m_State = BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT - (GetDefenderTeam()*3);
        BuildingsInZone.insert(b);
    }

    for (uint8 i = 0; i < 4; i++)
    {
        if (Creature* creature = SpawnCreature(QuestGivers[i].entrya, QuestGivers[i].x, QuestGivers[i].y, QuestGivers[i].z, QuestGivers[i].o, TEAM_ALLIANCE))
        {
            HideNpc(creature);
            creature->setFaction(BfFactions[TEAM_ALLIANCE]);
            questgiversA.insert(creature->GetGUID());
        }

        if (Creature* creature = SpawnCreature(QuestGivers[i].entryh, QuestGivers[i].x, QuestGivers[i].y, QuestGivers[i].z, QuestGivers[i].o, TEAM_HORDE))
        {
            HideNpc(creature);
            creature->setFaction(BfFactions[TEAM_HORDE]);
            questgiversH.insert(creature->GetGUID());
        }
    }

    uint32 a = 0;
    uint32 b = 0;
    genNumbers(a, b);

    uint32 i = 0;
    GuidSet qs = (GetDefenderTeam() == TEAM_ALLIANCE ? questgiversA : questgiversH);
    for (GuidSet::const_iterator itr = qs.begin(); itr != qs.end(); ++itr)
    {
        ++i;
        if (a == i || b == i)
            continue;
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
                ShowNpc(creature, true);
        }
    }

    for (uint8 i = 0; i < 3; i++)
        if (GameObject* go = SpawnGameObject(TBGameobjectsDoor[i].entrya, TBGameobjectsDoor[i].x, TBGameobjectsDoor[i].y, TBGameobjectsDoor[i].z, TBGameobjectsDoor[i].o))
        {
            go->SetLootState(GO_READY);
            go->UseDoorOrButton();
            goDoors.insert(go->GetGUID());
        }

    return true;
}

void BattlefieldTB::OnGameObjectCreate(GameObject* go)
{
    uint8 workshopId = 0;
    switch (go->GetEntry())
    {
        case GAMEOBJECT_TB_NORTH_CAPTURE_POINT_AD:
        case GAMEOBJECT_TB_NORTH_CAPTURE_POINT_HD:
            workshopId = BATTLEFIELD_TB_NOTH_CP;
            break;
        case GAMEOBJECT_TB_EAST_CAPTURE_POINT_AD:
        case GAMEOBJECT_TB_EAST_CAPTURE_POINT_HD:
            workshopId = BATTLEFIELD_TB_EAST_CP;
            break;
        case GAMEOBJECT_TB_WEST_CAPTURE_POINT_AD:
        case GAMEOBJECT_TB_WEST_CAPTURE_POINT_HD:
            workshopId = BATTLEFIELD_TB_WEST_CP;
            break;
     default:
         return;
    }

    for (TbWorkShop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
    {
        if (BfTBWorkShopData* workshop = (*itr))
        {
            if (workshop->m_Type == workshopId)
            {
                BfCapturePointTB *capturePoint = new BfCapturePointTB(this, GetDefenderTeam());

                capturePoint->SetCapturePointData(go);
                capturePoint->LinkToWorkShop(workshop);
                AddCapturePoint(capturePoint);

                CapturePoints.insert(capturePoint);
                break;
            }
        }
    }
}

bool BattlefieldTB::Update(uint32 diff)
{
    bool m_return = Battlefield::Update(diff);

    if (m_saveTimer <= diff)
    {
        sWorld->setWorldState((uint32)WorldStates::WS_TB_NEXT_BATTLE_TIMER_ENABLED, m_isActive);
        sWorld->setWorldState((uint32)WorldStates::WS_TB_HORDE_DEFENCE, m_DefenderTeam);
        sWorld->setWorldState((uint32)ClockBTWorldState[0], m_Timer );
        m_saveTimer = 60 * IN_MILLISECONDS;
    } else m_saveTimer -= diff;

    // Bad code!!!!!!!!!!!!!!!!
    for (GuidSet::const_iterator itr = m_PlayersIsSpellImu.begin(); itr != m_PlayersIsSpellImu.end(); ++itr)
        if (Player* plr = ObjectAccessor::FindPlayer(*itr))
        {
            if (plr->HasAura(SPELL_TB_SPIRITUAL_IMMUNITY))
            {
                const WorldSafeLocsEntry *graveyard = GetClosestGraveYard(plr);
                if (graveyard)
                {
                    if (plr->GetDistance2d(graveyard->Loc.X, graveyard->Loc.Y) > 10.0f)
                    {
                        plr->RemoveAurasDueToSpell(SPELL_TB_SPIRITUAL_IMMUNITY);
                        m_PlayersIsSpellImu.erase(plr->GetGUID());
                    }
                }
            }
        }

    if (m_isActive)
        if (m_Data32[BATTLEFIELD_TB_DATA_CAPTURED] == 3)
            EndBattle(false);

    return m_return;
}

void BattlefieldTB::OnPlayerJoinWar(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_PHASE_TB_NON_BATTLE);
    player->RemoveAurasDueToSpell(SPELL_PHASE_HORDE_CONTROL);
    player->RemoveAurasDueToSpell(SPELL_PHASE_ALLIANCE_CONTROL);

    bool onTb = player->GetZoneId() == m_AreaID; 
    // resurect dead plr
    if(!player->isAlive())
        player->ResurrectPlayer(1.0f);

    if (player->GetTeamId() == GetDefenderTeam())
    {
        if (!onTb || player->GetPositionZ() < POS_Z_TOWER)
            player->TeleportTo(732, -1226.0f, 976.0f, 156.0f, 6.18f);
    }
    else if (!onTb || player->GetPositionX() < POS_X_START)
        player->TeleportTo(732, -807.065f, 1188.05f, 111.038f, 3.154648f);
}

void BattlefieldTB::OnPlayerLeaveWar(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_TB_SPIRITUAL_IMMUNITY);
    player->RemoveAurasDueToSpell(SPELL_TB_VETERAN);

    Battlefield::OnPlayerLeaveWar(player);
}

void BattlefieldTB::OnPlayerEnterZone(Player* player)
{
    if (!IsWarTime())
    {
        player->AddAura(SPELL_PHASE_TB_NON_BATTLE, player);
        player->AddAura(GetDefenderTeam() == TEAM_ALLIANCE ? SPELL_PHASE_ALLIANCE_CONTROL : SPELL_PHASE_HORDE_CONTROL, player);
    }
}

void BattlefieldTB::OnPlayerLeaveZone(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_PHASE_TB_NON_BATTLE);
    player->RemoveAurasDueToSpell(SPELL_PHASE_ALLIANCE_CONTROL);
    player->RemoveAurasDueToSpell(SPELL_PHASE_HORDE_CONTROL);
    player->RemoveAurasDueToSpell(SPELL_TB_SPIRITUAL_IMMUNITY);
    player->RemoveAurasDueToSpell(SPELL_TB_VETERAN);
}

void BattlefieldTB::AddPlayerToResurrectQueue(ObjectGuid npc_guid, ObjectGuid playerGUID)
{
    Battlefield::AddPlayerToResurrectQueue(npc_guid, playerGUID);

    if (IsWarTime())
    {
        if (Player *plr = ObjectAccessor::FindPlayer(playerGUID))
        {
            if (!plr->HasAura(SPELL_TB_SPIRITUAL_IMMUNITY))
            {
                plr->CastSpell(plr, SPELL_TB_SPIRITUAL_IMMUNITY, true);
                m_PlayersIsSpellImu.insert(plr->GetGUID());
            }
        }
    }
}

void BattlefieldTB::OnBattleStart()
{
    m_Data32[BATTLEFIELD_TB_DATA_CAPTURED] = 0;
    m_Data32[BATTLEFIELD_TB_DATA_DESTROYED] = 0;

    for (TbGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        if ((*itr))
            (*itr)->Rebuild();

    for (TbWorkShop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        if ((*itr))
            (*itr)->UpdateWorkshop();

    for (uint8 i = 0; i < BATTLEFIELD_TB_GY_MAX; i++)
    {
        if (i == BATTLEFIELD_TB_GY_BARADIN_HOLD)
            m_GraveyardList[i]->GiveControlTo(GetDefenderTeam());
        else
            m_GraveyardList[i]->GiveControlTo(GetAttackerTeam());
    }

    for (GuidSet::const_iterator itr = goDoors.begin(); itr != goDoors.end(); ++itr)
    {
        if (GameObject* obj = ObjectAccessor::GetObjectInWorld(*itr, (GameObject*)nullptr))
        {
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton();
        }
    }

    for (GuidSet::const_iterator itr = Vehicles.begin(); itr != Vehicles.end(); ++itr)
    {
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
            {
                ShowNpc(creature, false);
                creature->setFaction(BfFactions[GetAttackerTeam()]);
            }
        }
    }

    for (GuidSet::const_iterator itr = questgiversA.begin(); itr != questgiversA.end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = questgiversH.begin(); itr != questgiversH.end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
        {
            if (Player* plr = ObjectAccessor::FindPlayer(*itr))
            {
                plr->RemoveAurasByType(SPELL_AURA_MOUNTED);

                if (plr->getClass() == CLASS_DRUID)
                    plr->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
                if (plr->GetTeamId() == GetDefenderTeam())
                {
                    uint32 k = urand(0, 3);
                    plr->TeleportTo(732, TbDefencerStartPosition[k].m_positionX, TbDefencerStartPosition[k].m_positionY, TbDefencerStartPosition[k].m_positionZ, TbDefencerStartPosition[k].m_orientation);
                    plr->CastSpell(plr, 88473, true);
                }
                else
                    plr->TeleportTo(732, TbDefencerStartPosition[4].m_positionX, TbDefencerStartPosition[4].m_positionY, TbDefencerStartPosition[4].m_positionZ, TbDefencerStartPosition[4].m_orientation);
            }
        }
    }

    SendWarningToAllInZone(BATTLEFIELD_TB_TEXT_START);
    SendInitWorldStatesToAll();
}

void BattlefieldTB::OnBattleEnd(bool endbytimer)
{
    if (endbytimer)
        SendWarningToAllInZone(GetDefenderTeam() == TEAM_ALLIANCE ? BATTLEFIELD_TB_TEXT_ALLIANCE_DEF_TOLBARAD : BATTLEFIELD_TB_TEXT_HORDE_DEF_TOLBARAD);
    else
        SendWarningToAllInZone(GetDefenderTeam() == TEAM_ALLIANCE ? BATTLEFIELD_TB_TEXT_ALLIANCE_TAKEN_TOLBARAD : BATTLEFIELD_TB_TEXT_HORDE_TAKEN_TOLBARAD);

    if (sWorld->getWorldState(20011) > uint64(time(nullptr)))
        sWorld->setWorldState(20011, time(nullptr) + 86400);

    for (TbGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
        if ((*itr))
            (*itr)->Rebuild();

    for (TbWorkShop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
        if ((*itr))
            (*itr)->UpdateWorkshop();

    for (BfCapturePointSet::const_iterator itr = CapturePoints.begin(); itr != CapturePoints.end(); ++itr)
        if ((*itr))
        {
            (*itr)->SetTeam(GetDefenderTeam());
            (*itr)->UpdateCapturePointValue();
        }

    for (uint8 i = 0; i < BATTLEFIELD_TB_GY_MAX; i++)
        m_GraveyardList[i]->GiveControlTo(GetDefenderTeam());

    for (GuidSet::const_iterator itr = goDoors.begin(); itr != goDoors.end(); ++itr)
    {
        if (GameObject* obj = ObjectAccessor::GetObjectInWorld(*itr, (GameObject*)nullptr))
        {
            obj->ToGameObject()->SetLootState(GO_READY);
            obj->ToGameObject()->UseDoorOrButton();
        }
    }

    for (GuidSet::const_iterator itr = Vehicles.begin(); itr != Vehicles.end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
            {
                creature->Kill(creature, false);
                HideNpc(creature);
            }

    for (GuidSet::const_iterator itr = questgiversA.begin(); itr != questgiversA.end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);

    for (GuidSet::const_iterator itr = questgiversH.begin(); itr != questgiversH.end(); ++itr)
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
            if (Creature* creature = unit->ToCreature())
                HideNpc(creature);


    uint32 a = 0;
    uint32 b = 0;
    genNumbers(a, b);

    uint32 i = 0;
    GuidSet qs = (GetDefenderTeam() == TEAM_ALLIANCE ? questgiversA : questgiversH);
    for (GuidSet::const_iterator itr = qs.begin(); itr != qs.end(); ++itr)
    {
        ++i;

        if (a == i || b == i)
            continue;
        if (Unit* unit = sObjectAccessor->FindUnit(*itr))
        {
            if (Creature* creature = unit->ToCreature())
                ShowNpc(creature, true);
        }
    }

    SendInitWorldStatesToAll();

    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
    {
        for (GuidSet::const_iterator itr = m_PlayersInWar[team].begin(); itr != m_PlayersInWar[team].end(); ++itr)
        {
            if (Player* plr = ObjectAccessor::FindPlayer(*itr))
            {
                plr->AddAura(SPELL_PHASE_TB_NON_BATTLE, plr);
                plr->AddAura(GetDefenderTeam() == TEAM_ALLIANCE ? SPELL_PHASE_ALLIANCE_CONTROL : SPELL_PHASE_HORDE_CONTROL, plr);

                plr->RemoveAurasDueToSpell(SPELL_TB_VETERAN);

                if (endbytimer)
                {
                    if (plr->GetTeamId() == GetDefenderTeam())
                    {
                        switch (plr->GetTeamId())
                        {
                            case TEAM_ALLIANCE:
                                plr->CastSpell(plr, SPELL_TB_VICTORY_REWARD_ALLIANCE, true);
                                IncrementQuest(plr, 28882, true);
                            break;
                            case TEAM_HORDE:
                                plr->CastSpell(plr, SPELL_TB_VICTORY_REWARD_HORDE, true);
                                IncrementQuest(plr, 28884, true);
                            break;
                        }
                        if (m_Data32[BATTLEFIELD_TB_DATA_DESTROYED] == 0)
                            plr->CastSpell(plr, SPELL_TB_TOL_BARAD_TOWER_DEFENDED, true);
                    }
                    else
                    {
                        switch (plr->GetTeamId())
                        {
                            case TEAM_ALLIANCE:
                                IncrementQuest(plr, 28882, true);
                            break;
                            case TEAM_HORDE:
                                IncrementQuest(plr, 28884, true);
                            break;
                        }
                        plr->CastSpell(plr, SPELL_TB_LOOSER_REWARD, true);
                        plr->RepopAtGraveyard();
                    }
                }
                else
                {
                    if (plr->GetTeamId() == GetAttackerTeam())
                    {
                        switch (team)
                        {
                            case TEAM_ALLIANCE:
                                plr->CastSpell(plr, SPELL_TB_VICTORY_REWARD_ALLIANCE, true);
                            break;
                            case TEAM_HORDE:
                                plr->CastSpell(plr, SPELL_TB_VICTORY_REWARD_HORDE, true);
                            break;
                        }
                    }
                    else
                    {
                        plr->CastSpell(plr, SPELL_TB_LOOSER_REWARD, true);
                        plr->RepopAtGraveyard();
                    }
                }
            }
        }
        m_PlayersInWar[team].clear();
    }

    m_Data32[BATTLEFIELD_TB_DATA_CAPTURED] = 0;
    m_Data32[BATTLEFIELD_TB_DATA_DESTROYED] = 0;
}

void BattlefieldTB::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(WorldStates::WS_TB_BATTLE_TIMER_ENABLED, IsWarTime() ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::BG_WS_BATTLE_TIMER, uint32(IsWarTime() ? (time(nullptr) + GetTimer() / 1000) : 0));
    packet.Worldstates.emplace_back(WorldStates::WS_TB_COUNTER_BUILDINGS, 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_COUNTER_BUILDINGS_ENABLED, IsWarTime() ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_HORDE_DEFENCE, IsWarTime() ? (GetDefenderTeam() == TEAM_HORDE ? 1 : 0) : 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_ALLIANCE_DEFENCE, uint32(IsWarTime() ? (GetDefenderTeam() == TEAM_ALLIANCE ? 1 : 0) : 0));
    packet.Worldstates.emplace_back(WorldStates::WS_TB_NEXT_BATTLE_TIMER_ENABLED, IsWarTime() ? 0 : 1);
    packet.Worldstates.emplace_back(WorldStates::BG_TB_NEXT_BATTLE_TIMER, uint32(!IsWarTime() ? time(nullptr) + (GetTimer() / 1000) : 0));
    packet.Worldstates.emplace_back(WorldStates::WS_TB_ALLIANCE_ATTACK, IsWarTime() ? (GetAttackerTeam() == TEAM_ALLIANCE ? 1 : 0) : 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_HORDE_ATTACK, IsWarTime() ? (GetAttackerTeam() == TEAM_HORDE ? 1 : 0) : 0);

    if (!IsWarTime())
        packet.Worldstates.emplace_back(WorldStates::BG_TB_NEXT_BATTLE_TIMER, uint32(time(nullptr)+(GetTimer() / 1000)));
    else
        packet.Worldstates.emplace_back(WorldStates::BG_TB_NEXT_BATTLE_TIMER, 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_KEEP_HORDE_DEFENCE, GetDefenderTeam() == TEAM_HORDE ? 1 : 0);
    packet.Worldstates.emplace_back(WorldStates::WS_TB_KEEP_ALLIANCE_DEFENCE, GetDefenderTeam() == TEAM_ALLIANCE ? 1 : 0);

    for (TbWorkShop::const_iterator itr = WorkshopsList.begin(); itr != WorkshopsList.end(); ++itr)
    {
        for (int i = 0; i < MAX_CP_DIFF; i++)
        {
            switch (i)
            {
                case ALLIANCE_DEFENCE:
                    if ((*itr)->m_State == BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT)
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                case HORDE_DEFENCE:
                    if ((*itr)->m_State == BATTLEFIELD_TB_OBJECTSTATE_HORDE_INTACT)
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                /*case ALLIANCE_ATTACK:
                    if ((*itr)->m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE || (*itr)->m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE)
                        packet.Worldstates.emplace_back((*itr)->m_WorldState + ALLIANCE_ATTACK, 1);
                    else
                        packet.Worldstates.emplace_back((*itr)->m_WorldState + ALLIANCE_ATTACK, 0);
                    break;
                case HORDE_ATTACK:
                    if ((*itr)->m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE || (*itr)->m_State == BF_CAPTUREPOINT_OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE)
                        packet.Worldstates.emplace_back((*itr)->m_WorldState + HORDE_ATTACK, 1);
                    else
                        packet.Worldstates.emplace_back((*itr)->m_WorldState + HORDE_ATTACK, 0);
                    break;*/
                default:
                    packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + NEUTRAL), 1);
            }
        }
    }

    for (TbGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        for (int i = 0; i < BUILDING_MAX_DIFF; i++)
        {
            switch (i)
            {
                case BUILDING_HORDE_DEFENCE:
                    if ((*itr)->m_State == (BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT-TEAM_HORDE*3))
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                case BUILDING_HORDE_DEFENCE_DAMAGED:
                    if ((*itr)->m_State == (BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE-TEAM_HORDE*3))
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                case BUILDING_DESTROYED:
                    if ((*itr)->m_State == BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DESTROY-TEAM_HORDE*3)
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                case BUILDING_ALLIANCE_DEFENCE:
                    if ((*itr)->m_State == (BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_INTACT-TEAM_ALLIANCE*3))
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                case BUILDING_ALLIANCE_DEFENCE_DAMAGED:
                    if ((*itr)->m_State == (BATTLEFIELD_TB_OBJECTSTATE_ALLIANCE_DAMAGE-TEAM_ALLIANCE*3))
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 1);
                    else
                        packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + i), 0);
                    break;
                default:
                    packet.Worldstates.emplace_back(static_cast<WorldStates>((*itr)->m_WorldState + BUILDING_DESTROYED), 1);
            }
        }
    }
}

void BattlefieldTB::CapturePoint(uint32 team)
{
    if (team == BATTLEFIELD_TB_TEAM_NEUTRAL)
        return;

    if (team == GetDefenderTeam())
        m_Data32[BATTLEFIELD_TB_DATA_CAPTURED]--;
    else
        m_Data32[BATTLEFIELD_TB_DATA_CAPTURED]++;

    if (int32(m_Data32[BATTLEFIELD_TB_DATA_CAPTURED]) < 0)
        m_Data32[BATTLEFIELD_TB_DATA_CAPTURED] = 0;

    SendUpdateWorldState(WorldStates::WS_TB_COUNTER_BUILDINGS, m_Data32[BATTLEFIELD_TB_DATA_CAPTURED]);
}

void BattlefieldTB::OnDestroyed()
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (GuidSet::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player *plr = ObjectAccessor::FindPlayer(*itr))
                if (plr->GetTeam() == GetAttackerTeam())
                    plr->CastSpell(plr, SPELL_TB_TOL_BARAD_TOWER_DESTROYED, true);

    // Tower destroing incrase battle time
    m_Timer += 10 * 60 * 1000;
    sWorld->setWorldState((uint32)ClockBTWorldState[0], m_Timer );
    SendUpdateWorldState(WorldStates::BG_WS_BATTLE_TIMER, (time(nullptr) + GetTimer() / 1000));
}

void BattlefieldTB::HandleKill(Player *killer, Unit *victim)
{
    if (killer == victim)
        return;

    if (!victim->IsPlayer())
        return;

    if (!IsWarTime())
        return;

    killer->CastSpell(killer, SPELL_TB_VETERAN, true);
}

void BattlefieldTB::OnDamaged()
{
    for (uint8 team = TEAM_ALLIANCE; team < MAX_TEAMS; ++team)
        for (GuidSet::iterator itr = m_players[team].begin(); itr != m_players[team].end(); ++itr)
            if (Player *plr = ObjectAccessor::FindPlayer(*itr))
                if (plr->GetTeam() == GetAttackerTeam())
                    plr->CastSpell(plr, SPELL_TB_TOL_BARAD_TOWER_DAMAGED, true);
}

void BattlefieldTB::ProcessEvent(WorldObject *obj, uint32 eventId)
{
    if (!obj || !IsWarTime())
        return;

    //if destroy or damage event, search the wall/tower and update worldstate/send warning message
    for (TbGameObjectBuilding::const_iterator itr = BuildingsInZone.begin(); itr != BuildingsInZone.end(); ++itr)
    {
        if (obj->GetEntry() == (*itr)->m_Build->GetEntry())
        {
            if ((*itr)->m_Build->GetGOInfo()->destructibleBuilding.DamagedEvent == eventId)
                (*itr)->Damaged();
            
            if ((*itr)->m_Build->GetGOInfo()->destructibleBuilding.DestroyedEvent == eventId)
                (*itr)->Destroyed();

            break;
        }
    }
}

void BattlefieldTB::AddDamagedTower(TeamId team)
{
}

void BattlefieldTB::BrokenWallOrTower(TeamId team)
{
}

void BattlefieldTB::AddBrokenTower(TeamId team)
{
}

void BfCapturePointTB::ChangeTeam(TeamId /*oldTeam*/)
{
    m_WorkShop->ChangeControl(m_team,false);
}

BfCapturePointTB::BfCapturePointTB(BattlefieldTB *bf,TeamId control) : BfCapturePoint(bf)
{
    m_Bf = bf;
    m_team = control;
    m_WorkShop = nullptr;
}

BfGraveYardTB::BfGraveYardTB(BattlefieldTB* bf) : BfGraveyard(bf), m_GossipTextId(0)
{
    m_Bf = bf;
}
