////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "AshranMgr.hpp"
#include "Packets/ChatPackets.h"
#include "Packets/WorldStatePackets.h"

OutdoorGraveyardAshran::OutdoorGraveyardAshran(OutdoorPvPAshran* p_OutdoorPvP) : OutdoorGraveyard(p_OutdoorPvP)
{
    m_OutdoorPvP = p_OutdoorPvP;
}

OPvPCapturePoint_Middle::OPvPCapturePoint_Middle(OutdoorPvP* outdoor, eBattleType type, uint8 p_Faction)
    : OPvPCapturePoint(outdoor), m_BattleType(type), m_BattleFaction(p_Faction)
{
    SetCapturePointData(g_CapturePoint[type]);
    AddCreature(AshranGenericMobTypeID + type, SLGGenericMoPLargeAoI, TEAM_NONE, AshranMapID, g_CapturePoint[type].x, g_CapturePoint[type].y, g_CapturePoint[type].z, M_PI);
    static_cast<OutdoorPvPAshran*>(m_PvP)->AddGenericMoPGuid(type, m_Creatures[AshranGenericMobTypeID + type]);

    if (type == EmberfallTower)
    {
        if (roll_chance_i(25))
            AddCreature(HordeTowerGuardian, g_FactionGuardians[TEAM_HORDE]);
    }
    else if (type == ArchmageOverwatch)
        if (roll_chance_i(25))
            AddCreature(AllianceTowerGuardian, g_FactionGuardians[TEAM_ALLIANCE]);
}

void OPvPCapturePoint_Middle::ChangeState()
{
    uint32 l_UpdateVal = 0;
    switch (m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
            m_BattleFaction = ControlAlliance;
            SpawnFactionGuards(m_BattleType, m_BattleFaction);
            l_UpdateVal = FlagAlliance;
            SendUpdateWorldState(WorldStateEnableTowerProgressBar, WorldStateDisabled);
            break;
        case OBJECTIVESTATE_HORDE:
            m_BattleFaction = ControlHorde;
            SpawnFactionGuards(m_BattleType, m_BattleFaction);
            l_UpdateVal = FlagHorde;
            SendUpdateWorldState(WorldStateEnableTowerProgressBar, WorldStateDisabled);
            break;
        case OBJECTIVESTATE_NEUTRAL:
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
            m_BattleFaction = ControlNeutral;
            SpawnFactionGuards(m_BattleType, m_BattleFaction);
            l_UpdateVal = FlagNeutral;
            break;
        default:
            break;
    }

    if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
        l_Flag->SetGoArtKit(l_UpdateVal);

    UpdateTowerState();
}

void OPvPCapturePoint_Middle::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    packet.Worldstates.emplace_back(g_TowerControlStatus[GetBattleType()], int32(GetBattleFaction()));
}

void OPvPCapturePoint_Middle::UpdateTowerState()
{
    m_PvP->SendUpdateWorldState(g_TowerControlStatus[GetBattleType()], int32(GetBattleFaction()));
}

bool OPvPCapturePoint_Middle::Update(uint32 p_Diff)
{
    if (m_BattleFaction != ControlNeutral)
        return false;

    return OPvPCapturePoint::Update(p_Diff);
}

void OPvPCapturePoint_Middle::SpawnFactionGuards(eBattleType p_BattleID, uint8 p_Faction)
{
    if (p_Faction > ControlAlliance || p_BattleID >= MaxBattleType)
        return;

    bool l_IsInitialized = static_cast<OutdoorPvPAshran*>(m_PvP)->IsInitialized();
    if (l_IsInitialized && p_Faction != ControlNeutral)
    {
        if (Creature* l_GenericMoP = sObjectAccessor->FindCreature(static_cast<OutdoorPvPAshran*>(m_PvP)->GetGenericMoPGuid(p_BattleID)))
            l_GenericMoP->AI()->DoAction(p_Faction == ControlHorde ? AnnounceHordeVictory : AnnounceAllianceVictory);
    }

    bool l_MustChangeKillCap = false;

    switch (p_BattleID)
    {
        case EmberfallTower:
        {
            if (p_Faction == ControlNeutral)
            {
                for (uint8 l_Count = 0; l_Count < EmberfallTowerCreaturesCount; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_EmberfallTowerNeutralSpawns[l_Count], 5);
                }

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);

                DelCreature(EmberfallTowerSpiritHealer);

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(5))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);
            }
            else
            {
                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(p_Faction == ControlHorde ? FlagHorde : FlagAlliance);

                if (l_IsInitialized)
                {
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetNextBattleTimer(AshranTimeForBattle);
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetBattleState(p_Faction == ControlAlliance ? WorldStateHighWarlordVolrath : WorldStateVolrathsAdvanceBattle);
                }

                for (uint8 l_Count = 0; l_Count < EmberfallTowerCreaturesCount; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_EmberfallTowerSpawns[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count], 2 * MINUTE);
                }

                for (uint8 l_Count = EmberfallTowerCreaturesCount; l_Count < EmberfallTowerSpawnsIDs; ++l_Count)
                {
                    DelObject(l_Count);

                    if (p_Faction == ControlAlliance)
                        AddObject(l_Count, g_EmberfallFiresSpawns[l_Count - EmberfallTowerCreaturesCount]);
                }

                DelCreature(HordeTowerGuardian);
                if (roll_chance_i(25) && p_Faction == ControlHorde)
                    AddCreature(HordeTowerGuardian, g_FactionGuardians[TEAM_HORDE], 60 * MINUTE);  ///< Set respawn time at 1 hour to prevent multiple kills

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(5))
                    graveyard->GiveControlTo(p_Faction == ControlHorde ? TEAM_HORDE : TEAM_ALLIANCE);

                DelCreature(EmberfallTowerSpiritHealer);
                if (p_Faction == ControlHorde)
                    AddCreature(EmberfallTowerSpiritHealer, g_EmberfallTowerSpiritHealer[TEAM_HORDE]);
                else
                    AddCreature(EmberfallTowerSpiritHealer, g_EmberfallTowerSpiritHealer[TEAM_ALLIANCE]);

                if (p_Faction == ControlAlliance)
                    l_MustChangeKillCap = true;
            }

            break;
        }
        case VolrathsAdvance:
        {
            if (p_Faction == ControlNeutral)
            {
                uint8 l_CreatureMaxIndex = EmberfallTowerSpawnsIDs + VolrathsAdvanceCreaturesCount;
                for (uint8 l_Count = EmberfallTowerSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_VolrathsAdvanceNeutralSpawns[l_Count - EmberfallTowerSpawnsIDs], 5);
                }

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);
            }
            else
            {
                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(p_Faction == ControlHorde ? FlagHorde : FlagAlliance);

                if (l_IsInitialized)
                {
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetNextBattleTimer(AshranTimeForBattle);
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetBattleState(p_Faction == ControlAlliance ? WorldStateEmberfallTowerBattle : WorldStateTheCrossroadsBattle);
                }

                uint8 l_CreatureMaxIndex = EmberfallTowerSpawnsIDs + VolrathsAdvanceCreaturesCount;
                for (uint8 l_Count = EmberfallTowerSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_VolrathsAdvanceSpawns[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count - EmberfallTowerSpawnsIDs], 2 * MINUTE);
                }

                uint8 l_FireIndex = EmberfallTowerSpawnsIDs + VolrathsAdvanceCreaturesCount;
                for (uint8 l_Index = l_FireIndex; l_Index < VolrathsAdvanceSpawnsIDs; ++l_Index)
                {
                    DelObject(l_Index);

                    if (p_Faction == ControlAlliance)
                        AddObject(l_Index, g_VolrathsAdvanceFires[l_Index - l_FireIndex]);
                }

                if (p_Faction == ControlAlliance)
                    l_MustChangeKillCap = true;
            }

            break;
        }
        case TheCrossroads:
        {
            if (p_Faction == ControlNeutral)
            {
                uint8 l_CreatureMaxIndex = VolrathsAdvanceSpawnsIDs + TheCrossroadsCreaturesCount;
                for (uint8 l_Count = VolrathsAdvanceSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_CrossroadsNeutralSpawns[l_Count - VolrathsAdvanceSpawnsIDs], 5);
                }

                uint8 l_FlagIndex = VolrathsAdvanceSpawnsIDs + TheCrossroadsCreaturesCount;
                for (uint8 l_Count = l_FlagIndex; l_Count < TheCrossroadsSpawnsIDs; ++l_Count)
                    DelObject(l_Count);

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);
            }
            else
            {
                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(p_Faction == ControlHorde ? FlagHorde : FlagAlliance);

                if (l_IsInitialized)
                {
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetNextBattleTimer(AshranTimeForBattle);
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetBattleState(p_Faction == ControlAlliance ? WorldStateVolrathsAdvanceBattle : WorldStateTrembladesVanguardBattle);
                }

                uint8 l_CreatureMaxIndex = VolrathsAdvanceSpawnsIDs + TheCrossroadsCreaturesCount;
                for (uint8 l_Count = VolrathsAdvanceSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_CrossroadSpawns[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count - VolrathsAdvanceSpawnsIDs], 2 * MINUTE);
                }

                uint8 l_FlagIndex = VolrathsAdvanceSpawnsIDs + TheCrossroadsCreaturesCount;
                for (uint8 l_Count = l_FlagIndex; l_Count < TheCrossroadsSpawnsIDs; ++l_Count)
                {
                    DelObject(l_Count);
                    AddObject(l_Count, g_CrossroadsBanners[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count - l_FlagIndex]);
                }
            }

            break;
        }
        case TrembladesVanguard:
        {
            if (p_Faction == ControlNeutral)
            {
                uint8 l_CreatureMaxIndex = TheCrossroadsSpawnsIDs + TrembladesVanguardCreaturesCount;
                for (uint8 l_Count = TheCrossroadsSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_TrembladesVanguardNeutralSpawns[l_Count - TheCrossroadsSpawnsIDs], 5);
                }

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);
            }
            else
            {
                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(p_Faction == ControlHorde ? FlagHorde : FlagAlliance);

                if (l_IsInitialized)
                {
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetNextBattleTimer(AshranTimeForBattle);
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetBattleState(p_Faction == ControlAlliance ? WorldStateTheCrossroadsBattle : WorldStateArchmageOverwatchBattle);
                }

                uint8 l_CreatureMaxIndex = TheCrossroadsSpawnsIDs + TrembladesVanguardCreaturesCount;
                for (uint8 l_Count = TheCrossroadsSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_TrembladesVanguardSpawns[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count - TheCrossroadsSpawnsIDs], 2 * MINUTE);
                }

                uint8 l_FireIndex = TheCrossroadsSpawnsIDs + TrembladesVanguardCreaturesCount;
                for (uint8 l_Index = l_FireIndex; l_Index < TrembladesVanguardSpawnsIDs; ++l_Index)
                {
                    DelObject(l_Index);

                    if (p_Faction == ControlHorde)
                        AddObject(l_Index, g_TrembladesVanguardFires[l_Index - l_FireIndex]);
                }

                if (p_Faction == ControlHorde)
                    l_MustChangeKillCap = true;
            }

            break;
        }
        case ArchmageOverwatch:
        {
            if (p_Faction == ControlNeutral)
            {
                uint8 l_CreatureMaxIndex = TrembladesVanguardSpawnsIDs + ArchmageOverwatchCreaturesCount;
                for (uint8 l_Index = TrembladesVanguardSpawnsIDs; l_Index < l_CreatureMaxIndex; ++l_Index)
                {
                    DelCreature(l_Index);
                    AddCreature(l_Index, g_ArchmageOverwatchNeutral[l_Index - TrembladesVanguardSpawnsIDs], 5);
                }

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);

                DelCreature(ArchmageOverwatchSpiritHealer);

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(4))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);
            }
            else
            {
                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
                    l_Flag->SetGoArtKit(p_Faction == ControlHorde ? FlagHorde : FlagAlliance);

                if (l_IsInitialized)
                {
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetNextBattleTimer(AshranTimeForBattle);
                    static_cast<OutdoorPvPAshran*>(m_PvP)->SetBattleState(p_Faction == ControlAlliance ? WorldStateTrembladesVanguardBattle : WorldStateGrandMarshalTrembladeBattle);
                }

                for (uint8 l_Index = ArchmageOverwatchSpawnsIDs; l_Index < ArchmageOverwatchSpawnsIDs; ++l_Index)
                    DelCreature(l_Index);

                uint8 l_CreatureMaxIndex = TrembladesVanguardSpawnsIDs + ArchmageOverwatchCreaturesCount;
                for (uint8 l_Count = TrembladesVanguardSpawnsIDs; l_Count < l_CreatureMaxIndex; ++l_Count)
                {
                    DelCreature(l_Count);
                    AddCreature(l_Count, g_ArchmageOverwatchSpawns[p_Faction == ControlAlliance ? TEAM_ALLIANCE : TEAM_HORDE][l_Count - TrembladesVanguardSpawnsIDs], 2 * MINUTE);
                }

                uint8 l_FireIndex = TrembladesVanguardSpawnsIDs + ArchmageOverwatchCreaturesCount;
                for (uint8 l_Index = l_FireIndex; l_Index < ArchmageOverwatchSpawnsIDs; ++l_Index)
                {
                    DelObject(l_Index);

                    if (p_Faction == ControlHorde)
                        AddObject(l_Index, g_ArchmageOverwatchFires[l_Index - l_FireIndex]);
                }

                DelCreature(AllianceTowerGuardian);
                if (roll_chance_i(25) && p_Faction == ControlAlliance)
                    AddCreature(AllianceTowerGuardian, g_FactionGuardians[TEAM_ALLIANCE], 60 * MINUTE);  ///< Set respawn time at 1 hour to prevent multiple kills

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(4))
                    graveyard->GiveControlTo(p_Faction == ControlHorde ? TEAM_HORDE : TEAM_ALLIANCE);

                DelCreature(ArchmageOverwatchSpiritHealer);
                if (p_Faction == ControlHorde)
                    AddCreature(ArchmageOverwatchSpiritHealer, g_ArchmageOverwatchSpiritHealer[TEAM_HORDE]);
                else
                    AddCreature(ArchmageOverwatchSpiritHealer, g_ArchmageOverwatchSpiritHealer[TEAM_ALLIANCE]);

                if (p_Faction == ControlHorde)
                    l_MustChangeKillCap = true;
            }

            break;
        }
        default:
            break;
    }

    if (l_MustChangeKillCap)
        static_cast<OutdoorPvPAshran*>(m_PvP)->ResetKillCap(p_Faction == ControlAlliance ? TEAM_HORDE : TEAM_ALLIANCE);
}

void OPvPCapturePoint_Middle::SetBattleFaction(uint32 p_Faction)
{
    m_BattleFaction = p_Faction;
}

uint32 OPvPCapturePoint_Middle::GetBattleFaction() const
{
    return m_BattleFaction;
}

eBattleType OPvPCapturePoint_Middle::GetBattleType() const
{
    return m_BattleType;
}

OPvPCapturePoint_Graveyard::OPvPCapturePoint_Graveyard(OutdoorPvP* outdoor) : OPvPCapturePoint(outdoor)
{
    m_GraveyardState = ControlNeutral;
    m_ControlTime = 0;

    SetCapturePointData(g_GraveyardBanner_N);
}

void OPvPCapturePoint_Graveyard::ChangeState()
{
    uint32 l_UpdateVal = 0;
    switch (m_State)
    {
        case OBJECTIVESTATE_ALLIANCE:
        {
            m_GraveyardState = ControlAlliance;
            SpawnFactionFlags(m_GraveyardState);
            l_UpdateVal = FlagAlliance;
            m_ControlTime = 15 * MINUTE * IN_MILLISECONDS;

            if (Creature* l_Herald = static_cast<OutdoorPvPAshran*>(m_PvP)->GetHerald())
                l_Herald->AI()->DoAction(AnnounceAllianceGraveyard);

            SendUpdateWorldState(WorldStateEnableGraveyardProgressBar, WorldStateDisabled);
            break;
        }
        case OBJECTIVESTATE_HORDE:
        {
            m_GraveyardState = ControlHorde;
            SpawnFactionFlags(m_GraveyardState);
            l_UpdateVal = FlagHorde;
            m_ControlTime = 15 * MINUTE * IN_MILLISECONDS;

            if (Creature* l_Herald = static_cast<OutdoorPvPAshran*>(m_PvP)->GetHerald())
                l_Herald->AI()->DoAction(AnnounceHordeGraveyard);

            SendUpdateWorldState(WorldStateEnableGraveyardProgressBar, WorldStateDisabled);
            break;
        }
        case OBJECTIVESTATE_NEUTRAL:
        case OBJECTIVESTATE_NEUTRAL_ALLIANCE_CHALLENGE:
        case OBJECTIVESTATE_NEUTRAL_HORDE_CHALLENGE:
        case OBJECTIVESTATE_ALLIANCE_HORDE_CHALLENGE:
        case OBJECTIVESTATE_HORDE_ALLIANCE_CHALLENGE:
        {
            m_GraveyardState = ControlNeutral;
            SpawnFactionFlags(m_GraveyardState);
            l_UpdateVal = FlagNeutral;

            if (Creature* l_Herald = static_cast<OutdoorPvPAshran*>(m_PvP)->GetHerald())
                l_Herald->AI()->DoAction(AnnounceMarketplaceGraveyard);

            break;
        }
        default:
            break;
    }

    if (GameObject* l_Flag = sObjectAccessor->FindGameObject(m_capturePointGUID))
        l_Flag->SetGoArtKit(l_UpdateVal);

    UpdateTowerState();
}

void OPvPCapturePoint_Graveyard::SendChangePhase()
{
    SendUpdateWorldState(WorldStateEnableGraveyardProgressBar, WorldStateEnabled);
    SendUpdateWorldState(WorldStateGraveyardProgressBar, static_cast<uint32>(ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f)));
    SendUpdateWorldState(WorldStateGraveyardProgressBarGreyPct, m_neutralValuePct);
}

void OPvPCapturePoint_Graveyard::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    switch (m_GraveyardState)
    {
        case ControlNeutral:
            packet.Worldstates.emplace_back(WorldStateGraveyardStatusForAlliance, int32(WorldStateDisabled));
            packet.Worldstates.emplace_back(WorldStateGraveyardStatusForHorde, int32(WorldStateDisabled));
            break;
        case ControlAlliance:
            packet.Worldstates.emplace_back(WorldStateGraveyardStatusForAlliance, int32(WorldStateEnabled));
            break;
        case ControlHorde:
            packet.Worldstates.emplace_back(WorldStateGraveyardStatusForHorde, int32(WorldStateEnabled));
            break;
        default:
            break;
    }
}

void OPvPCapturePoint_Graveyard::UpdateTowerState()
{
    if (m_PvP == nullptr)
        return;

    switch (m_GraveyardState)
    {
        case ControlNeutral:
            m_PvP->SendUpdateWorldState(WorldStateGraveyardStatusForAlliance, WorldStateDisabled);
            m_PvP->SendUpdateWorldState(WorldStateGraveyardStatusForHorde, WorldStateDisabled);
            break;
        case ControlAlliance:
            m_PvP->SendUpdateWorldState(WorldStateGraveyardStatusForAlliance, WorldStateEnabled);
            break;
        case ControlHorde:
            m_PvP->SendUpdateWorldState(WorldStateGraveyardStatusForHorde, WorldStateEnabled);
            break;
        default:
            break;
    }
}

bool OPvPCapturePoint_Graveyard::HandlePlayerEnter(Player* player)
{
    if (player == nullptr)
        return false;

    if (OPvPCapturePoint::HandlePlayerEnter(player))
    {
        player->SendUpdateWorldState(WorldStateEnableGraveyardProgressBar, WorldStateEnabled);
        player->SendUpdateWorldState(WorldStateGraveyardProgressBar, static_cast<uint32>(ceil((m_value + m_maxValue) / (2 * m_maxValue) * 100.0f)));
        player->SendUpdateWorldState(WorldStateGraveyardProgressBarGreyPct, m_neutralValuePct);
        return true;
    }

    return false;
}

void OPvPCapturePoint_Graveyard::HandlePlayerLeave(Player* player)
{
    if (player == nullptr)
        return;

    player->SendUpdateWorldState(WorldStateEnableGraveyardProgressBar, WorldStateDisabled);
    OPvPCapturePoint::HandlePlayerLeave(player);
}

void OPvPCapturePoint_Graveyard::SpawnFactionFlags(uint8 p_Faction)
{
    for (uint8 l_Index = GraveyardBanner0; l_Index < GraveyardMaxBanner; ++l_Index)
    {
        switch (p_Faction)
        {
            case ControlAlliance:
                AddObject(l_Index, g_GraveyardBanner_A[l_Index]);

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(2))
                    graveyard->GiveControlTo(TEAM_ALLIANCE);
                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(3))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);

                DelCreature(MarketplaceGraveyardSpiritHealer);
                AddCreature(MarketplaceGraveyardSpiritHealer, g_MarketplaceGraveyardSpirits[TEAM_ALLIANCE]);
                break;
            case ControlHorde:
                AddObject(l_Index, g_GraveyardBanner_H[l_Index]);

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(2))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);
                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(3))
                    graveyard->GiveControlTo(TEAM_HORDE);

                DelCreature(MarketplaceGraveyardSpiritHealer);
                AddCreature(MarketplaceGraveyardSpiritHealer, g_MarketplaceGraveyardSpirits[TEAM_HORDE]);
                break;
            case ControlNeutral:
                DelObject(l_Index);

                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(2))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);
                if (OutdoorGraveyard* graveyard = m_PvP->GetGraveyardById(3))
                    graveyard->GiveControlTo(TEAM_NEUTRAL);

                DelCreature(MarketplaceGraveyardSpiritHealer);
                break;
            default:
                break;
        }
    }
}

bool OPvPCapturePoint_Graveyard::Update(uint32 p_Diff)
{
    ScheduleNextControl(p_Diff);

    if (m_State == OBJECTIVESTATE_ALLIANCE || m_State == OBJECTIVESTATE_HORDE)
        return false;

    return OPvPCapturePoint::Update(p_Diff);
}

void OPvPCapturePoint_Graveyard::ScheduleNextControl(uint32 p_Diff)
{
    if (!m_ControlTime)
        return;

    if (m_ControlTime <= p_Diff)
    {
        m_value = 0;
        m_State = OBJECTIVESTATE_NEUTRAL;
        m_ControlTime = 0;
    }
    else
        m_ControlTime -= p_Diff;
}

uint8 OPvPCapturePoint_Graveyard::GetGraveyardState() const
{
    return m_GraveyardState;
}

OutdoorPvPAshran::OutdoorPvPAshran()
{
    m_TypeId = OUTDOOR_PVP_ASHRAN;
    m_WorldPvPAreaId = AshranPvPAreaID;
    m_InitPointsTimer = 0;
    m_IsInitialized = false;
    m_WillBeReset = false;
    m_CurrentBattleState = WorldStateTheCrossroadsBattle;
    m_NextBattleTimer = AshranTimeForBattle * IN_MILLISECONDS;
    m_MaxBattleTime = 0;
    m_GladiatorRespawnTime = 0;
    m_AncientArtifactTime = 0;

    m_PlayerCurrencyLoots.clear();
    m_NeutralVignettes.clear();
    m_ActiveCaptains.clear();

    m_Guid = ObjectGuid::Create<HighGuid::PVPQueueGroup>(AshranMapID, m_WorldPvPAreaId, 0).GetLowPart();
    m_Guid |= BattlefieldWorldPvP;

    for (uint8 l_Team = 0; l_Team < TEAM_NEUTRAL; ++l_Team)
    {
        m_PlayersInWar[l_Team].clear();
        m_InvitedPlayers[l_Team].clear();
        m_PlayersWillBeKick[l_Team].clear();
        m_FactionVignettes[l_Team].clear();

        m_EnnemiesKilled[l_Team] = 0;
        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap2;
        m_StadiumRacingLaps[l_Team] = 0;

        for (uint8 l_I = 0; l_I < MaxArtifactCounts; ++l_I)
        {
            m_ArtifactsCollected[l_Team][l_I] = 0;
            m_ArtifactEventsLaunched[l_Team][l_I] = false;
        }
    }

    for (uint8 l_Index = 0; l_Index < MaxEvents; ++l_Index)
    {
        m_AshranEvents[l_Index] = 0;
        m_AshranEventsWarned[l_Index] = false;
        m_AshranEventsLaunched[l_Index] = false;
    }

    AddCreature(AllianceFactionBoss, g_FactionBossesSpawn[0], 5 * MINUTE);
    AddCreature(AllianceMarshalKarshStormforge, g_FactionBossesGuardians[0], 5 * MINUTE);
    AddCreature(AllianceMarshalGabriel, g_FactionBossesGuardians[1], 5 * MINUTE);
    AddCreature(HordeFactionBoss, g_FactionBossesSpawn[3], 5 * MINUTE);
    AddCreature(HordeGeneralAevd, g_FactionBossesGuardians[6], 5 * MINUTE);
    AddCreature(HordeWarlordNoktyn, g_FactionBossesGuardians[7], 5 * MINUTE);
}

bool OutdoorPvPAshran::SetupOutdoorPvP()
{
    TC_LOG_ERROR(LOG_FILTER_GENERAL, "OutdoorPvPAshran: SetupOutdoorPvP");

    RegisterZone(AshranZoneID);

    return true;
}

void OutdoorPvPAshran::Initialize(uint32 zone)
{
    for (uint8 i = EmberfallTower; i < MaxBattleType; ++i)
    {
        if (g_MiddleBattlesEntries[i] == m_CurrentBattleState)
        {
            m_ControlPoints[i] = new OPvPCapturePoint_Middle(this, eBattleType(i), ControlNeutral);
            m_ControlPoints[i]->SetState(OBJECTIVESTATE_NEUTRAL);
        }
        else
        {
            if (i < TheCrossroads)
            {
                m_ControlPoints[i] = new OPvPCapturePoint_Middle(this, eBattleType(i), ControlHorde);
                m_ControlPoints[i]->SetState(OBJECTIVESTATE_HORDE);
            }
            else
            {
                m_ControlPoints[i] = new OPvPCapturePoint_Middle(this, eBattleType(i), ControlAlliance);
                m_ControlPoints[i]->SetState(OBJECTIVESTATE_ALLIANCE);
            }
        }

        AddCapturePoint(m_ControlPoints[i]);

        TC_LOG_ERROR(LOG_FILTER_GENERAL, "OutdoorPvPAshran: SetupOutdoorPvP:: AddCapturePoint %u", i);
    }

    m_GraveYard = new OPvPCapturePoint_Graveyard(this);
    AddCapturePoint(m_GraveYard);

    SetGraveyardNumber(TotalGraveyards);
    for (uint8 x = 0; x < TotalGraveyards; ++x)
    {
        OutdoorGraveyardAshran* graveyard = new OutdoorGraveyardAshran(this);
        graveyard->Initialize(g_AshranGraveyards[x].m_StartTeam, g_AshranGraveyards[x].m_ID);
        m_GraveyardList[x] = graveyard;
    }

    for (uint8 l_TeamID = TEAM_ALLIANCE; l_TeamID <= TEAM_HORDE; ++l_TeamID)
    {
        AddCreature(AllianceBaseSpiritHealer + l_TeamID, g_BasesSpiritHealers[l_TeamID]);
        //AddAreaTrigger(g_HallowedGroundEntries[l_TeamID], 1, AshranHallowedGroundID, g_HallowedGroundPos[l_TeamID], 0, sMapMgr->CreateBaseMap(AshranMapID));
    }

    AddCreature(AllianceGuardian, g_AllianceGuardian);
    AddCreature(HordeGuardian, g_HordeGuardian);
    AddObject(AncientArtifactSpawn, g_AncientArtifactPos[urand(0, AncientArtifactCount - 1)]);
}

void OutdoorPvPAshran::HandlePlayerEnterMap(ObjectGuid guid, uint32 zoneID)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    if (!player || player->GetTeamId() >= MAX_TEAMS)
        return;

    if (player->getLevel() < PlayerMinLevel)
    {
        if (m_PlayersWillBeKick[player->GetTeamId()].count(player->GetGUID()) == 0)
            m_PlayersWillBeKick[player->GetTeamId()][player->GetGUID()] = uint32(time(nullptr)) + 10;
        return;
    }

    if (m_PlayersInWar[player->GetTeamId()].count(player->GetGUID()) || m_InvitedPlayers[player->GetTeamId()].count(player->GetGUID()))
        return;

    m_InvitedPlayers[player->GetTeamId()][player->GetGUID()] = uint32(time(nullptr)) + AshranTimeForInvite;
    sLFGMgr->LeaveLfg(player->GetGUID());

    player->CastSpell(player, SpellLootable, true);

    if (player->GetTeamId() == TEAM_ALLIANCE)
        player->CastSpell(player, WelcomeToAshranAlliance, true);
    else
        player->CastSpell(player, WelcomeToAshranHorde, true);

    if (!m_IsInitialized && !m_InitPointsTimer)
    {
        player->GetMap()->LoadAllGrids(3700.0f, 5100.0f, -5050.0f, -3510.0f, player);
        m_InitPointsTimer = 2000;
    }

    for (auto itr : m_NeutralVignettes)
        if (auto vignette = sVignetteStore.LookupEntry(itr.first))
            if (auto creature = Creature::GetCreature(*player, itr.second))
                player->GetVignetteMgr().CreateAndAddVignette(vignette, AshranMapID, Vignette::Type::SourceScript, creature->GetPosition(), creature->GetAreaId(), itr.second);

    if (player->GetTeamId() < TEAM_NEUTRAL)
        for (auto itr : m_FactionVignettes[player->GetTeamId()])
            if (auto vignette = sVignetteStore.LookupEntry(itr.first))
                if (auto creature = Creature::GetCreature(*player, itr.second))
                    player->GetVignetteMgr().CreateAndAddVignette(vignette, AshranMapID, Vignette::Type::SourceScript, creature->GetPosition(), creature->GetAreaId(), itr.second);
}

void OutdoorPvPAshran::HandlePlayerLeaveMap(ObjectGuid guid, uint32 mapID)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    if (player->GetTeamId() < TEAM_NEUTRAL)
    {
        m_InvitedPlayers[player->GetTeamId()].erase(player->GetGUID());
        m_PlayersInWar[player->GetTeamId()].erase(player->GetGUID());
        m_PlayersWillBeKick[player->GetTeamId()].erase(player->GetGUID());

        GuidSet::iterator itrSet = m_players[player->GetTeamId()].find(player->GetGUID());
        if (itrSet != m_players[player->GetTeamId()].end())
            m_players[player->GetTeamId()].erase(itrSet);
    }

    SendRemoveWorldStates(player);

    player->RemoveAura(SpellLootable);

    if (m_PlayerCurrencyLoots.find(player->GetGUID()) != m_PlayerCurrencyLoots.end())
        m_PlayerCurrencyLoots.erase(player->GetGUID());

    for (auto itr : m_NeutralVignettes)
        if (auto vignette = sVignetteStore.LookupEntry(itr.first))
            player->GetVignetteMgr().DestroyAndRemoveVignetteByEntry(vignette);

    if (player->GetTeamId() < TEAM_NEUTRAL)
        for (auto itr : m_FactionVignettes[player->GetTeamId()])
            if (auto  vignette = sVignetteStore.LookupEntry(itr.first))
                player->GetVignetteMgr().DestroyAndRemoveVignetteByEntry(vignette);
}

void OutdoorPvPAshran::HandlePlayerEnterArea(ObjectGuid guid, uint32 areaID)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    if (areaID == AshranPreAreaHorde || areaID == AshranPreAreaAlliance)
    {
        ObjectGuid guid = player->GetGUID();
        player->AddDelayedEvent(Seconds(5).count(), [guid]() -> void
        {
            if (auto player2 = sObjectAccessor->FindPlayer(guid))
                player2->SafeTeleport(AshranNeutralMapID, player2);
        });
    }

    if (areaID == AshranHordeBase && player->GetTeamId() == TEAM_HORDE || areaID == AshranAllianceBase && player->GetTeamId() == TEAM_ALLIANCE)
        player->CastSpell(player, SpellHoldYourGround, true);
    else if (areaID == EmberfallTowerAreaID && player->GetTeamId() == TEAM_HORDE || areaID == ArchmageOverwatchAreaID && player->GetTeamId() == TEAM_ALLIANCE)
        player->CastSpell(player, SpellTowerDefense, true);
    else if (areaID == VolrathsAdvanceAreaID && player->GetTeamId() == TEAM_HORDE || areaID == TrembladesVanguardAreaID && player->GetTeamId() == TEAM_ALLIANCE)
        player->CastSpell(player, SpellStandFast, true);
}

void OutdoorPvPAshran::HandlePlayerLeaveArea(ObjectGuid guid, uint32 areaID)
{
    Player* player = ObjectAccessor::GetObjectInMap(guid, m_map, (Player*)nullptr);
    if (!player)
        return;

    if (player->GetMapId() == AshranNeutralMapID)
    {
        if (areaID == AshranPreAreaHorde || areaID == AshranPreAreaAlliance)
        {
            ObjectGuid guid = player->GetGUID();
            player->AddDelayedEvent(Seconds(1).count(), [guid]() -> void
            {
                if (auto player2 = sObjectAccessor->FindPlayer(guid))
                    player2->SafeTeleport(AshranMapID, player2);
                return;
            });
        }
    }
    else
    {
        if (areaID == AshranHordeBase || areaID == AshranAllianceBase)
            player->RemoveAura(SpellHoldYourGround);
        else if (areaID == EmberfallTowerAreaID || areaID == ArchmageOverwatchAreaID)
            player->RemoveAura(SpellTowerDefense);
        else if (areaID == VolrathsAdvanceAreaID || areaID == TrembladesVanguardAreaID)
            player->RemoveAura(SpellStandFast);
    }
}

void OutdoorPvPAshran::HandlePlayerResurrects(Player* player, uint32 /*zoneID*/)
{
    if (m_PlayerCurrencyLoots.find(player->GetGUID()) != m_PlayerCurrencyLoots.end())
        m_PlayerCurrencyLoots.erase(player->GetGUID());
}

void OutdoorPvPAshran::HandlePlayerKilled(Player* player)
{
    if (!player)
        return;

    if (uint32 artifactCount = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT))
    {
        artifactCount /= 2;
        player->ModifyCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT, -int32(artifactCount));
        m_PlayerCurrencyLoots.insert(std::make_pair(player->GetGUID(), artifactCount / CURRENCY_PRECISION));
    }

    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

void OutdoorPvPAshran::HandleKill(Player* killer, Unit* killed)
{
    std::string l_Str = killer->GetSession()->GetTrinityString(LangDisplaySlainCounter);
    uint8 l_SlayCount = 0;

    if (killed->GetTypeId() == TYPEID_PLAYER)
        l_SlayCount = KillCountForPlayer;
    else if (IsFactionGuard(killed))  ///< Only for Road of Glory
        l_SlayCount = KillCountForFactionGuard;

    if (l_SlayCount > 0)
    {
        WorldPackets::Chat::WorldText packet;
        packet.Guid = killed->GetGUID();
        packet.Text = l_Str;
        killer->SendDirectMessage(packet.Write());

        TeamId l_Team = killer->GetTeamId();
        if (l_Team < TEAM_NEUTRAL)
        {
            if (m_EnnemiesKilled[l_Team] + l_SlayCount < m_EnnemiesKilledMax[l_Team])
            {
                m_EnnemiesKilled[l_Team] += l_SlayCount;

                if (l_Team == TEAM_ALLIANCE)
                    SendUpdateWorldState(WorldStateEnnemiesSlainAlliance, m_EnnemiesKilled[l_Team]);
                else
                    SendUpdateWorldState(WorldStateEnnemiesSlainHorde, m_EnnemiesKilled[l_Team]);
            }
            else
            {
                l_SlayCount -= m_EnnemiesKilledMax[l_Team] - m_EnnemiesKilled[l_Team];

                m_EnnemiesKilled[l_Team] = l_SlayCount;

                switch (m_EnnemiesKilledMax[l_Team])
                {
                    case EnnemiesSlainCap1:
                        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap2;
                        break;
                    case EnnemiesSlainCap2:
                        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap3;
                        break;
                    case EnnemiesSlainCap3:
                        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap4;
                        break;
                    case EnnemiesSlainCap4:
                        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap5;
                        break;
                    case EnnemiesSlainCap5:
                        m_EnnemiesKilledMax[l_Team] = EnnemiesSlainCap6;
                        break;
                    case EnnemiesSlainCap6:
                    default:
                        break;
                }

                if (l_Team == TEAM_ALLIANCE)
                {
                    SendUpdateWorldState(WorldStateEnnemiesSlainAlliance, m_EnnemiesKilled[l_Team]);
                    SendUpdateWorldState(WorldStateEnnemiesSlainAllianceMax, m_EnnemiesKilledMax[l_Team]);
                }
                else
                {
                    SendUpdateWorldState(WorldStateEnnemiesSlainHorde, m_EnnemiesKilled[l_Team]);
                    SendUpdateWorldState(WorldStateEnnemiesSlainHordeMax, m_EnnemiesKilledMax[l_Team]);
                }

                creature_type l_Spawn;
                AshranCaptain l_Captain;
                uint8 l_MaxLoop = 255;

                do
                {
                    --l_MaxLoop;

                    if (l_Team == TEAM_ALLIANCE)
                    {
                        l_Captain = g_AshranCaptains[urand(0, MaxAllianceCaptains - 1)];
                        l_Spawn.teamval = ALLIANCE;
                        l_Spawn.x = g_FactionBossesSpawn[2].x;
                        l_Spawn.y = g_FactionBossesSpawn[2].y;
                        l_Spawn.z = g_FactionBossesSpawn[2].z;
                        l_Spawn.o = g_FactionBossesSpawn[2].o;
                    }
                    else
                    {
                        l_Captain = g_AshranCaptains[urand(MaxAllianceCaptains, MaxAshranCaptains - 1)];
                        l_Spawn.teamval = HORDE;
                        l_Spawn.x = g_FactionBossesSpawn[5].x;
                        l_Spawn.y = g_FactionBossesSpawn[5].y;
                        l_Spawn.z = g_FactionBossesSpawn[5].z;
                        l_Spawn.o = g_FactionBossesSpawn[5].o;
                    }

                    l_Spawn.entry = l_Captain.Entry;
                    l_Spawn.map = AshranMapID;

                    if (m_ActiveCaptains.find(l_Captain.Type) != m_ActiveCaptains.end())
                        l_Captain = AshranCaptain();

                } while (!l_Captain.Entry || l_MaxLoop > 0);

                m_ActiveCaptains.insert(l_Captain.Type);
                AddCreature(l_Captain.Type, l_Spawn);
            }
        }
    }
}

void OutdoorPvPAshran::ResetKillCap(uint8 p_Team)
{
    if (p_Team >= TEAM_NEUTRAL)
        return;

    while (m_EnnemiesKilled[p_Team] >= EnnemiesSlainCap1)
    {
        m_EnnemiesKilled[p_Team] -= EnnemiesSlainCap1;

        creature_type l_Spawn;
        AshranCaptain l_Captain;
        if (p_Team == TEAM_ALLIANCE)
        {
            l_Captain = g_AshranCaptains[urand(0, MaxAllianceCaptains - 1)];
            l_Spawn.teamval = ALLIANCE;
            l_Spawn.x = g_FactionBossesSpawn[2].x;
            l_Spawn.y = g_FactionBossesSpawn[2].y;
            l_Spawn.z = g_FactionBossesSpawn[2].z;
            l_Spawn.o = g_FactionBossesSpawn[2].o;
        }
        else
        {
            l_Captain = g_AshranCaptains[urand(MaxAllianceCaptains, MaxAshranCaptains - 1)];
            l_Spawn.teamval = HORDE;
            l_Spawn.x = g_FactionBossesSpawn[5].x;
            l_Spawn.y = g_FactionBossesSpawn[5].y;
            l_Spawn.z = g_FactionBossesSpawn[5].z;
            l_Spawn.o = g_FactionBossesSpawn[5].o;
        }

        l_Spawn.entry = l_Captain.Entry;
        l_Spawn.map = AshranMapID;

        AddCreature(l_Captain.Type, l_Spawn);
    }

    m_EnnemiesKilledMax[p_Team] = EnnemiesSlainCap1;

    if (p_Team == TEAM_ALLIANCE)
    {
        SendUpdateWorldState(WorldStateEnnemiesSlainAlliance, m_EnnemiesKilled[p_Team]);
        SendUpdateWorldState(WorldStateEnnemiesSlainAllianceMax, m_EnnemiesKilledMax[p_Team]);
    }
    else
    {
        SendUpdateWorldState(WorldStateEnnemiesSlainHorde, m_EnnemiesKilled[p_Team]);
        SendUpdateWorldState(WorldStateEnnemiesSlainHordeMax, m_EnnemiesKilledMax[p_Team]);
    }
}

bool OutdoorPvPAshran::IsFactionGuard(Unit* p_Unit)
{
    switch (p_Unit->GetEntry())
    {
        case StormshieldVanguard:
        case StormshieldKnight:
        case StormshieldSentinel:
        case StormshieldFootman:
        case StormshieldPriest:
        case GrandMarshalTremblade:
        case RylaiCrestfall:
        case WarspearBloodGuard:
        case WarspearRaptorRider:
        case WarspearHeadhunter:
        case WarspearGrunt:
        case WarspearPriest:
        case HighWarlordVolrath:
        case JeronEmberfall:
            return true;
        default:
            break;
    }

    return false;
}

void OutdoorPvPAshran::SpawnGladiators(uint8 teamID /*= TEAM_NEUTRAL*/, bool p_Spawn /*= true*/)
{
    if (teamID == TEAM_NEUTRAL)
    {
        for (uint32 l_I = WarspearGladiatorsSpawnsIDs; l_I < StormshieldGladiatorsSpawnsIDs; ++l_I)
        {
            DelCreature(l_I);

            if (p_Spawn)
                AddCreature(l_I, g_StormshieldGladiators[l_I - WarspearGladiatorsSpawnsIDs], 2 * MINUTE);
        }

        for (uint32 l_I = ArchmageOverwatchSpawnsIDs; l_I < WarspearGladiatorsSpawnsIDs; ++l_I)
        {
            DelCreature(l_I);

            if (p_Spawn)
                AddCreature(l_I, g_WarspearGladiators[l_I - ArchmageOverwatchSpawnsIDs], 2 * MINUTE);
        }
    }
    else
    {
        if (teamID == TEAM_ALLIANCE)
        {
            for (uint32 l_I = WarspearGladiatorsSpawnsIDs; l_I < StormshieldGladiatorsSpawnsIDs; ++l_I)
            {
                DelCreature(l_I);

                if (p_Spawn)
                    AddCreature(l_I, g_StormshieldGladiators[l_I - WarspearGladiatorsSpawnsIDs], 2 * MINUTE);
            }
        }
        else
        {
            for (uint32 l_I = ArchmageOverwatchSpawnsIDs; l_I < WarspearGladiatorsSpawnsIDs; ++l_I)
            {
                DelCreature(l_I);

                if (p_Spawn)
                    AddCreature(l_I, g_WarspearGladiators[l_I - ArchmageOverwatchSpawnsIDs], 2 * MINUTE);
            }
        }
    }
}

void OutdoorPvPAshran::FillCustomPvPLoots(Player* looter, Loot& loot, ObjectGuid container)
{
    if (m_PlayerCurrencyLoots.find(container) == m_PlayerCurrencyLoots.end())
        return;

    loot.AddItem(LootStoreItem(CURRENCY_TYPE_ARTIFACT_FRAGMENT, LOOT_ITEM_TYPE_CURRENCY, 100.0f, 0, 0, m_PlayerCurrencyLoots[container], m_PlayerCurrencyLoots[container]));
    loot.FillCurrencyLoot(looter);
}

bool OutdoorPvPAshran::Update(uint32 p_Diff)
{
    PlayerTimerMap l_TempList[MAX_TEAMS];

    for (uint8 l_Team = 0; l_Team < 2; ++l_Team)
    {
        l_TempList[l_Team] = m_InvitedPlayers[l_Team];

        for (PlayerTimerMap::iterator l_Iter = l_TempList[l_Team].begin(); l_Iter != l_TempList[l_Team].end(); ++l_Iter)
        {
            if ((*l_Iter).second <= time(NULL))
            {
                if (Player* l_Player = sObjectAccessor->FindPlayer((*l_Iter).first))
                {
                    if (l_Player->GetTeamId() == TEAM_HORDE)
                        l_Player->TeleportTo(AshranNeutralMapID, g_HordeTeleportPos.m_positionX, g_HordeTeleportPos.m_positionY, g_HordeTeleportPos.m_positionZ, g_HordeTeleportPos.m_orientation);
                    else
                        l_Player->TeleportTo(AshranNeutralMapID, g_AllianceTeleportPos.m_positionX, g_AllianceTeleportPos.m_positionY, g_AllianceTeleportPos.m_positionZ, g_AllianceTeleportPos.m_orientation);
                }
            }
        }

        l_TempList[l_Team] = m_PlayersWillBeKick[l_Team];

        for (PlayerTimerMap::iterator l_Iter = l_TempList[l_Team].begin(); l_Iter != l_TempList[l_Team].end(); ++l_Iter)
        {
            if ((*l_Iter).second <= time(NULL))
            {
                if (Player* l_Player = sObjectAccessor->FindPlayer((*l_Iter).first))
                {
                    if (l_Player->GetTeamId() == TEAM_HORDE)
                        l_Player->TeleportTo(AshranNeutralMapID, g_HordeTeleportPos.m_positionX, g_HordeTeleportPos.m_positionY, g_HordeTeleportPos.m_positionZ, g_HordeTeleportPos.m_orientation);
                    else
                        l_Player->TeleportTo(AshranNeutralMapID, g_AllianceTeleportPos.m_positionX, g_AllianceTeleportPos.m_positionY, g_AllianceTeleportPos.m_positionZ, g_AllianceTeleportPos.m_orientation);
                }
            }
        }
    }

    ScheduleNextBattle(p_Diff);
    ScheduleEndOfBattle(p_Diff);
    ScheduleInitPoints(p_Diff);
    ScheduleEventsUpdate(p_Diff);
    ScheduleGladiatorRespawn(p_Diff);

    return OutdoorPvP::Update(p_Diff);
}

void OutdoorPvPAshran::ScheduleNextBattle(uint32 p_Diff)
{
    if (!m_NextBattleTimer)
        return;

    if (m_NextBattleTimer <= p_Diff)
    {
        m_NextBattleTimer = 0;

        if (m_WillBeReset)
        {
            SetBattleState(m_CurrentBattleState);
            ResetControlPoints();
        }

        SendUpdateWorldState(WorldStateNextBattleEnabled, WorldStateDisabled);

        bool l_Found = false;
        uint8 l_Count = 0;
        for (uint32 l_BattleIndex : g_MiddleBattlesEntries)
        {
            if (m_CurrentBattleState == l_BattleIndex)
                l_Found = true;
            else if (!l_Found)
                ++l_Count;
        }

        if (l_Found)
        {
            SendUpdateWorldState(WorldStateControlTheFlag, WorldStateEnabled);

            if (OPvPCapturePoint_Middle* l_ControlPoint = m_ControlPoints[l_Count])
            {
                l_ControlPoint->SetBattleFaction(ControlNeutral);
                l_ControlPoint->SetValue(0.0f);
                l_ControlPoint->SetState(OBJECTIVESTATE_NEUTRAL);
                l_ControlPoint->UpdateTowerState();

                if (GameObject* l_Flag = sObjectAccessor->FindGameObject(l_ControlPoint->m_capturePointGUID))
                    l_Flag->SetGoArtKit(FlagNeutral);
            }
        }
        else
        {
            m_MaxBattleTime = 10 * MINUTE * IN_MILLISECONDS;
            SendUpdateWorldState(WorldStateTimeRemainingForBoss, uint32(time(nullptr)) + m_MaxBattleTime / IN_MILLISECONDS);

            if (m_CurrentBattleState == WorldStateHighWarlordVolrath)
            {
                SendUpdateWorldState(WorldStateSlayVolrath, WorldStateEnabled);
                SendUpdateWorldState(WorldStateHighWarlordVolrath, WorldStateEnabled);
                SendUpdateWorldState(WorldStateWarspearOutpostStatus, ControlNeutral);

                if (Creature* l_Volrath = sObjectAccessor->FindCreature(m_HighWarlordVolrath))
                    l_Volrath->AI()->DoAction(WarspearOutpostInFight);

                SpawnGladiators(TEAM_HORDE, false);
            }
            else
            {
                SendUpdateWorldState(WorldStateSlayTremblade, WorldStateEnabled);
                SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, WorldStateEnabled);
                SendUpdateWorldState(WorldStateStormshieldStrongholdStatus, ControlNeutral);

                if (Creature* l_Tremblade = sObjectAccessor->FindCreature(m_GrandMasrhalTremblade))
                    l_Tremblade->AI()->DoAction(StormshieldStrongholdInFight);

                SpawnGladiators(TEAM_ALLIANCE, false);
            }
        }
    }
    else
        m_NextBattleTimer -= p_Diff;
}

void OutdoorPvPAshran::ScheduleEndOfBattle(uint32 p_Diff)
{
    if (!m_MaxBattleTime)
        return;

    if (m_MaxBattleTime <= p_Diff)
    {
        m_MaxBattleTime = 0;
        SetNextBattleTimer(AshranTimeForBattle);

        if (m_CurrentBattleState == WorldStateHighWarlordVolrath)
        {
            SendUpdateWorldState(WorldStateSlayVolrath, WorldStateDisabled);
            SendUpdateWorldState(WorldStateHighWarlordVolrath, WorldStateDisabled);
            SendUpdateWorldState(WorldStateWarspearOutpostStatus, ControlHorde);
            SetBattleState(WorldStateEmberfallTowerBattle);

            if (Creature* l_Volrath = sObjectAccessor->FindCreature(m_HighWarlordVolrath))
                l_Volrath->AI()->DoAction(WarspearVictory);

            SpawnGladiators(TEAM_HORDE);
        }
        else
        {
            SendUpdateWorldState(WorldStateSlayTremblade, WorldStateDisabled);
            SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, WorldStateDisabled);
            SendUpdateWorldState(WorldStateStormshieldStrongholdStatus, ControlAlliance);
            SetBattleState(WorldStateArchmageOverwatchBattle);

            if (Creature* l_Tremblade = sObjectAccessor->FindCreature(m_GrandMasrhalTremblade))
                l_Tremblade->AI()->DoAction(StormshieldVictory);

            SpawnGladiators(TEAM_ALLIANCE);
        }
    }
    else
        m_MaxBattleTime -= p_Diff;
}

void OutdoorPvPAshran::ScheduleInitPoints(uint32 p_Diff)
{
    if (!m_InitPointsTimer || m_IsInitialized)
        return;

    if (m_InitPointsTimer <= p_Diff)
    {
        m_InitPointsTimer = 0;
        InitializeControlPoints();
        InitializeEvents();
        m_IsInitialized = true;
    }
    else
        m_InitPointsTimer -= p_Diff;
}

void OutdoorPvPAshran::ScheduleEventsUpdate(uint32 p_Diff)
{
    if (!m_IsInitialized)
        return;

    uint32 l_TimeForWarn = AshranEventWarning * MINUTE * IN_MILLISECONDS;
    for (uint8 l_Index = 0; l_Index < MaxEvents; ++l_Index)
    {
        if (!m_AshranEvents[l_Index])
            continue;

        if (m_AshranEvents[l_Index] <= l_TimeForWarn && !m_AshranEventsWarned[l_Index])
        {
            m_AshranEventsWarned[l_Index] = true;
            SendEventWarningToPlayers(g_EventWarnTexts[l_Index]);
        }

        if (m_AshranEvents[l_Index] <= p_Diff)
        {
            m_AshranEvents[l_Index] = 0;
            m_AshranEventsWarned[l_Index] = false;
            StartEvent(l_Index);
        }
        else
            m_AshranEvents[l_Index] -= p_Diff;
    }
}

void OutdoorPvPAshran::ScheduleGladiatorRespawn(uint32 p_Diff)
{
    if (!m_GladiatorRespawnTime)
        return;

    if (m_GladiatorRespawnTime <= p_Diff)
    {
        m_GladiatorRespawnTime = 0;
        SpawnGladiators();
    }
    else
        m_GladiatorRespawnTime -= p_Diff;
}

void OutdoorPvPAshran::StartEvent(uint8 p_EventID)
{
    if (p_EventID >= MaxEvents)
        return;

    m_AshranEventsLaunched[p_EventID] = true;

    switch (p_EventID)
    {
        case EventKorlokTheOgreKing:
        {
            SendEventWarningToPlayers(LangKorlokIsAwakening);
            AddCreature(NeutralKorlokTheOgreKing, g_Korlok, 5 * MINUTE);
            AddCreature(OgreAllianceChampion, g_AllianceChapion, 5 * MINUTE);
            AddCreature(OgreHordeChapion, g_HordeChampion, 5 * MINUTE);
            break;
        }
        case EventStadiumRacing:
        {
            SendEventWarningToPlayers(LangStadiumRacingHasBegun);

            for (uint8 l_I = 0; l_I < MaxRacingFlags; ++l_I)
                AddObject(AllianceRacingFlagSpawn1 + l_I, g_RacingFlagsPos[l_I]);

            for (uint8 l_I = 0; l_I < MaxRacingCreatures; ++l_I)
                AddCreature(SpeedyHordeRacerSpawn + l_I, g_RacingCreaturesPos[l_I]);

            SendUpdateWorldState(WorldStateEnableLapsEvent, WorldStateEnabled);
            SendUpdateWorldState(WorldStateLapsAlliance, m_StadiumRacingLaps[TEAM_ALLIANCE]);
            SendUpdateWorldState(WorldStateLapsHorde, m_StadiumRacingLaps[TEAM_HORDE]);
            break;
        }
        case MaxEvents:
        default:
            break;
    }
}

void OutdoorPvPAshran::EndEvent(uint8 p_EventID, bool p_ScheduleNext /*= true*/)
{
    if (p_EventID >= MaxEvents)
        return;

    if (p_ScheduleNext)
        m_AshranEvents[p_EventID] = AshranEventTimer * MINUTE * IN_MILLISECONDS;

    switch (p_EventID)
    {
        case EventKorlokTheOgreKing:
        {
            if (p_ScheduleNext)
            {
                DelCreature(NeutralKorlokTheOgreKing);
                break;
            }

            DelCreature(OgreAllianceChampion);
            DelCreature(OgreHordeChapion);
            break;
        }
        case EventStadiumRacing:
        {
            for (uint8 l_I = 0; l_I < MaxRacingFlags; ++l_I)
                DelObject(AllianceRacingFlagSpawn1 + l_I);

            for (uint8 l_I = 0; l_I < MaxRacingCreatures; ++l_I)
                DelCreature(SpeedyHordeRacerSpawn + l_I);

            SendUpdateWorldState(WorldStateLapsAlliance, 0);
            SendUpdateWorldState(WorldStateLapsHorde, 0);
            SendUpdateWorldState(WorldStateEnableLapsEvent, WorldStateDisabled);
            break;
        }
        case MaxEvents:
        default:
            break;
    }

    m_AshranEventsLaunched[p_EventID] = false;
}

void OutdoorPvPAshran::SendEventWarningToPlayers(uint32 p_LangID)
{
    for (uint8 l_I = 0; l_I < MAX_TEAMS; ++l_I)
    {
        for (ObjectGuid guid : m_PlayersInWar[l_I])
        {
            //if (Player* l_Player = sObjectAccessor->FindPlayer(guid))
            //{
            //    WorldPacket l_Data;
            //    l_Player->BuildPlayerChat(&l_Data, 0, CHAT_MSG_TEXT_EMOTE, l_Player->GetSession()->GetTrinityString(p_LangID).c_str(), LANG_UNIVERSAL);
            //    l_Player->GetSession()->SendPacket(&l_Data);
            //}

            /// Send announces to invitees too
            for (auto l_Invitee : m_InvitedPlayers[l_I])
            {
                if (Player* l_Player = sObjectAccessor->FindPlayer(l_Invitee.first))
                {
                    //            WorldPacket l_Data;
                    //            l_Player->BuildPlayerChat(&l_Data, 0, CHAT_MSG_TEXT_EMOTE, l_Player->GetSession()->GetTrinityString(p_LangID).c_str(), LANG_UNIVERSAL);
                    //            l_Player->GetSession()->SendPacket(&l_Data);
                }
            }
        }
    }
}

void OutdoorPvPAshran::SetEventData(uint8 p_EventID, uint8 teamID, uint32 p_Data)
{
    if (p_EventID >= MaxEvents)
        return;

    if (!m_AshranEventsLaunched[p_EventID])
        return;

    switch (p_EventID)
    {
        case EventStadiumRacing:
        {
            if (teamID >= TEAM_NEUTRAL)
                break;

            if (m_StadiumRacingLaps[teamID] + p_Data >= MaxStadiumRacingLaps)
            {
                EndEvent(EventStadiumRacing);

                if (Creature* l_Herald = GetHerald())
                {
                    if (l_Herald->IsAIEnabled)
                    {
                        if (teamID == TEAM_ALLIANCE)
                            l_Herald->AI()->Talk(AllianceVictorious, ObjectGuid::Empty);
                        else
                            l_Herald->AI()->Talk(HordeVictorious, ObjectGuid::Empty);
                    }
                }
            }
            else
            {
                m_StadiumRacingLaps[teamID] += p_Data;

                if (teamID == TEAM_ALLIANCE)
                    SendUpdateWorldState(WorldStateLapsAlliance, m_StadiumRacingLaps[teamID]);
                else
                    SendUpdateWorldState(WorldStateLapsHorde, m_StadiumRacingLaps[teamID]);
            }

            break;
        }
        case EventKorlokTheOgreKing:
        default:
            break;
    }
}

void OutdoorPvPAshran::FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
{
    
    TC_LOG_ERROR(LOG_FILTER_GENERAL, "OutdoorPvPAshran: FillInitialWorldStates");

    packet.Worldstates.emplace_back(WorldStateEnnemiesSlainAlliance, int32(m_EnnemiesKilled[TEAM_ALLIANCE]));
    packet.Worldstates.emplace_back(WorldStateEnnemiesSlainHorde, int32(m_EnnemiesKilled[TEAM_HORDE]));

    packet.Worldstates.emplace_back(WorldStateEnnemiesSlainAllianceMax, int32(m_EnnemiesKilledMax[TEAM_ALLIANCE]));
    packet.Worldstates.emplace_back(WorldStateEnnemiesSlainHordeMax, int32(m_EnnemiesKilledMax[TEAM_HORDE]));

    packet.Worldstates.emplace_back(WorldStateActiveStage, int32(-1));

    packet.Worldstates.emplace_back(WorldStateEnableLapsEvent, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateLapsAlliance, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateLapsHorde, int32(WorldStateDisabled));

    packet.Worldstates.emplace_back(WorldStateOreCollectedAlliance, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateOreCollectedHorde, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateEnableOreCollection, int32(WorldStateDisabled));

    packet.Worldstates.emplace_back(WorldStateFiresScoringAlliance, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateFiresScoringHorde, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateFiresScoringEnabled, int32(WorldStateDisabled));

    packet.Worldstates.emplace_back(WorldStateRisenSpiritsCapturedAlliance, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateRisenSpiritsCapturedHorde, int32(WorldStateDisabled));
    packet.Worldstates.emplace_back(WorldStateRisenSpiritsCaptureEnabled, int32(WorldStateDisabled));

    if (m_GraveYard)
    {
        switch (m_GraveYard->GetGraveyardState())
        {
            case ControlNeutral:
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForAlliance, int32(WorldStateDisabled));
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForHorde, int32(WorldStateDisabled));
                break;
            case ControlAlliance:
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForHorde, int32(WorldStateDisabled));
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForAlliance, int32(WorldStateEnabled));
                break;
            case ControlHorde:
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForAlliance, int32(WorldStateDisabled));
                packet.Worldstates.emplace_back(WorldStateGraveyardStatusForHorde, int32(WorldStateEnabled));
                break;
            default:
                break;
        }
    }

    if (m_CurrentBattleState == WorldStateGrandMarshalTrembladeBattle)
    {
        packet.Worldstates.emplace_back(WorldStateTimeRemainingForBoss, int32(time(NULL) + m_MaxBattleTime / IN_MILLISECONDS));
        packet.Worldstates.emplace_back(WorldStateSlayVolrath, int32(WorldStateDisabled));
        packet.Worldstates.emplace_back(WorldStateSlayTremblade, int32(WorldStateEnabled));
    }
    else if (m_CurrentBattleState == WorldStateHighWarlordVolrath)
    {
        packet.Worldstates.emplace_back(WorldStateTimeRemainingForBoss, int32(time(NULL) + m_MaxBattleTime / IN_MILLISECONDS));
        packet.Worldstates.emplace_back(WorldStateSlayVolrath, int32(WorldStateEnabled));
        packet.Worldstates.emplace_back(WorldStateSlayTremblade, int32(WorldStateDisabled));
    }
    else
    {
        packet.Worldstates.emplace_back(WorldStateSlayVolrath, int32(WorldStateDisabled));
        packet.Worldstates.emplace_back(WorldStateSlayTremblade, int32(WorldStateDisabled));
    }

    packet.Worldstates.emplace_back(WorldStateWarspearOutpostStatus, int32(ControlHorde));

    for (uint32 l_BattleIndex : g_MiddleBattlesEntries)
    {
        if (m_CurrentBattleState == l_BattleIndex)
        {
            packet.Worldstates.emplace_back(m_CurrentBattleState, int32(WorldStateEnabled));

            if (m_NextBattleTimer)
            {
                packet.Worldstates.emplace_back(WorldStateNextBattleEnabled, int32(WorldStateEnabled));
                packet.Worldstates.emplace_back(WorldStateNextBattleTimestamp, int32(time(NULL) + m_NextBattleTimer / IN_MILLISECONDS));
                packet.Worldstates.emplace_back(WorldStateControlTheFlag, int32(WorldStateDisabled));
            }
            else
            {
                packet.Worldstates.emplace_back(WorldStateNextBattleEnabled, int32(WorldStateDisabled));
                packet.Worldstates.emplace_back(WorldStateControlTheFlag, int32(WorldStateEnabled));
            }
        }
        else
            packet.Worldstates.emplace_back(l_BattleIndex, int32(WorldStateDisabled));
    }

    packet.Worldstates.emplace_back(WorldStateStormshieldStrongholdStatus, int32(ControlAlliance));

    packet.Worldstates.emplace_back(WorldStateAllianceMageArtifactCount, int32(m_ArtifactsCollected[TEAM_ALLIANCE][CountForMage]));
    packet.Worldstates.emplace_back(WorldStateAllianceWarlockArtifactCount, int32(m_ArtifactsCollected[TEAM_ALLIANCE][CountForWarlock]));
    packet.Worldstates.emplace_back(WorldStateAllianceWarriorArtifactCount, int32(m_ArtifactsCollected[TEAM_ALLIANCE][CountForWarriorPaladin]));
    packet.Worldstates.emplace_back(WorldStateAllianceShamanArtifactCount, int32(m_ArtifactsCollected[TEAM_ALLIANCE][CountForDruidShaman]));

    packet.Worldstates.emplace_back(WorldStateHordeMageArtifactCount, int32(m_ArtifactsCollected[TEAM_HORDE][CountForMage]));
    packet.Worldstates.emplace_back(WorldStateHordeWarlockArtifactCount, int32(m_ArtifactsCollected[TEAM_HORDE][CountForWarlock]));
    packet.Worldstates.emplace_back(WorldStateHordeWarriorArtifactCount, int32(m_ArtifactsCollected[TEAM_HORDE][CountForWarriorPaladin]));
    packet.Worldstates.emplace_back(WorldStateHordeShamanArtifactCount, int32(m_ArtifactsCollected[TEAM_HORDE][CountForDruidShaman]));

    packet.Worldstates.emplace_back(WorldStateMageArtifactMaxCount, int32(MaxCountForMage));
    packet.Worldstates.emplace_back(WorldStateWarlockArtifactMaxCount, int32(MaxCountForWarlock));
    packet.Worldstates.emplace_back(WorldStateWarriorArtifactMaxCount, int32(MaxCountForWarriorPaladin));
    packet.Worldstates.emplace_back(WorldStateShamanArtifactMaxCount, int32(MaxCountForDruidShaman));

    for (auto v : m_capturePoints)
        v.second->FillInitialWorldStates(packet);
}

void OutdoorPvPAshran::SendRemoveWorldStates(Player* player)
{
    player->SendUpdateWorldState(WorldStateEnnemiesSlainAlliance, 0);
    player->SendUpdateWorldState(WorldStateEnnemiesSlainHorde, 0);
    player->SendUpdateWorldState(WorldStateEnnemiesSlainAllianceMax, 0);
    player->SendUpdateWorldState(WorldStateEnnemiesSlainHordeMax, 0);
    player->SendUpdateWorldState(WorldStateActiveStage, 0);
    player->SendUpdateWorldState(WorldStateControlTheFlag, 0);
    player->SendUpdateWorldState(WorldStateEnableLapsEvent, 0);
    player->SendUpdateWorldState(WorldStateLapsAlliance, 0);
    player->SendUpdateWorldState(WorldStateLapsHorde, 0);
    player->SendUpdateWorldState(WorldStateOreCollectedAlliance, 0);
    player->SendUpdateWorldState(WorldStateOreCollectedHorde, 0);
    player->SendUpdateWorldState(WorldStateEnableOreCollection, 0);
    player->SendUpdateWorldState(WorldStateFiresScoringAlliance, 0);
    player->SendUpdateWorldState(WorldStateFiresScoringHorde, 0);
    player->SendUpdateWorldState(WorldStateFiresScoringEnabled, 0);
    player->SendUpdateWorldState(WorldStateRisenSpiritsCapturedAlliance, 0);
    player->SendUpdateWorldState(WorldStateRisenSpiritsCapturedHorde, 0);
    player->SendUpdateWorldState(WorldStateRisenSpiritsCaptureEnabled, 0);
    player->SendUpdateWorldState(WorldStateNextBattleTimestamp, 0);
    player->SendUpdateWorldState(WorldStateNextBattleEnabled, 0);
    player->SendUpdateWorldState(WorldStateTimeRemainingForBoss, 0);
    player->SendUpdateWorldState(WorldStateSlayVolrath, 0);
    player->SendUpdateWorldState(WorldStateSlayTremblade, 0);
    player->SendUpdateWorldState(WorldStateEmberfallTowerBattle, 0);
    player->SendUpdateWorldState(WorldStateVolrathsAdvanceBattle, 0);
    player->SendUpdateWorldState(WorldStateTheCrossroadsBattle, 0);
    player->SendUpdateWorldState(WorldStateTrembladesVanguardBattle, 0);
    player->SendUpdateWorldState(WorldStateArchmageOverwatchBattle, 0);
    player->SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, 0);
}

void OutdoorPvPAshran::HandleBFMGREntryInviteResponse(bool accepted, Player* player)
{
    if (accepted)
    {
        m_PlayersInWar[player->GetTeamId()].insert(player->GetGUID());
        m_InvitedPlayers[player->GetTeamId()].erase(player->GetGUID());
        m_players[player->GetTeamId()].emplace(player->GetGUID());

        player->GetSession()->SendBfEntered(m_Guid, false, false);
    }
    else
    {
        if (player->GetTeamId() == TEAM_HORDE)
            player->TeleportTo(AshranNeutralMapID, g_HordeTeleportPos.m_positionX, g_HordeTeleportPos.m_positionY, g_HordeTeleportPos.m_positionZ, g_HordeTeleportPos.m_orientation);
        else
            player->TeleportTo(AshranNeutralMapID, g_AllianceTeleportPos.m_positionX, g_AllianceTeleportPos.m_positionY, g_AllianceTeleportPos.m_positionZ, g_AllianceTeleportPos.m_orientation);
    }
}

bool OutdoorPvPAshran::HandleOpenGo(Player* player, ObjectGuid guid)
{
    if (m_Objects[AncientArtifactSpawn] == guid)
    {
        if (Aura* aura = player->AddAura(SpellAncientArtifact, player))
        {
            if (m_AncientArtifactTime > 0)
            {
                aura->SetDuration(m_AncientArtifactTime);
                aura->SetMaxDuration(m_AncientArtifactTime);
            }
            else
                m_AncientArtifactTime = AncientArtifactMaxTime;
        }

        if (Creature* l_Herald = GetHerald())
        {
            if (l_Herald->IsAIEnabled)
            {
                if (player->GetTeamId() == TEAM_ALLIANCE)
                    l_Herald->AI()->Talk(ArtifactLootedByAlliance, player->GetGUID());
                else
                    l_Herald->AI()->Talk(ArtifactLootedByHorde, player->GetGUID());
            }
        }

        DelObject(AncientArtifactSpawn);
        return true;
    }

    return false;
}

void OutdoorPvPAshran::HandleArtifactDrop(Unit* p_Unit, uint32 p_Time)
{
    if (!p_Time)
    {
        m_AncientArtifactTime = 0;
        AddObject(AncientArtifactSpawn, g_AncientArtifactPos[urand(0, AncientArtifactCount - 1)]);
    }
    else
    {
        if (p_Unit)
        {
            go_type const l_GoType = {AncientArtifact, AshranMapID, p_Unit->m_positionX, p_Unit->m_positionY, p_Unit->m_positionZ, p_Unit->m_orientation, 0.0f, 0.0f, 0.0f, 0.0f};
            AddObject(AncientArtifactSpawn, l_GoType);
            m_AncientArtifactTime = p_Time;
        }
    }
}

void OutdoorPvPAshran::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case AshranHerald:
            m_HeraldGuid = creature->GetGUID();
            break;
        case HighWarlordVolrath:
            m_HighWarlordVolrath = creature->GetGUID();
            AddCreature(SLGGenericMoPLargeAoI + TEAM_HORDE, SLGGenericMoPLargeAoI, TEAM_OTHER, AshranMapID, creature->m_positionX, creature->m_positionY, creature->m_positionZ, M_PI);
            m_FactionGenericMoP[TEAM_HORDE] = m_Creatures[SLGGenericMoPLargeAoI + TEAM_HORDE];
            break;
        case GrandMarshalTremblade:
            m_GrandMasrhalTremblade = creature->GetGUID();
            AddCreature(SLGGenericMoPLargeAoI + TEAM_ALLIANCE, SLGGenericMoPLargeAoI, TEAM_OTHER, AshranMapID, creature->m_positionX, creature->m_positionY, creature->m_positionZ, M_PI);
            m_FactionGenericMoP[TEAM_ALLIANCE] = m_Creatures[SLGGenericMoPLargeAoI + TEAM_ALLIANCE];
            break;
        case AllianceSpiritGuide:
        case HordeSpiritGuide:
        {
            TeamId l_TeamID = creature->GetEntry() == HordeSpiritGuide ? TEAM_HORDE : TEAM_ALLIANCE;
            uint8 l_GraveyardID = GetSpiritGraveyardID(creature->GetAreaId(), l_TeamID);
            if (m_GraveyardList[l_GraveyardID])
                m_GraveyardList[l_GraveyardID]->SetSpirit(creature, l_TeamID);
            break;
        }
        case KorlokTheOgreKing:
            AddVignetteOnPlayers(creature, VignetteKorlok);
            break;
        case VignetteDummyA:
            AddVignetteOnPlayers(creature, VignetteStormshieldPortal, TEAM_ALLIANCE);
            break;
        case VignetteDummyH:
            AddVignetteOnPlayers(creature, VignetteWarspearPortal, TEAM_HORDE);
            break;
        case Nisstyr:
            m_ArtifactsNPCGuids[TEAM_HORDE][CountForWarlock] = creature->GetGUID();
            break;
        case Fura:
            m_ArtifactsNPCGuids[TEAM_HORDE][CountForMage] = creature->GetGUID();
            break;
        case Kalgan:
            m_ArtifactsNPCGuids[TEAM_HORDE][CountForWarriorPaladin] = creature->GetGUID();
            break;
        case Atomik:
            m_ArtifactsNPCGuids[TEAM_HORDE][CountForDruidShaman] = creature->GetGUID();
            break;
        case Marketa:
            m_ArtifactsNPCGuids[TEAM_ALLIANCE][CountForWarlock] = creature->GetGUID();
            break;
        case Ecilam:
            m_ArtifactsNPCGuids[TEAM_ALLIANCE][CountForMage] = creature->GetGUID();
            break;
        case ValantBrightsworn:
            m_ArtifactsNPCGuids[TEAM_ALLIANCE][CountForWarriorPaladin] = creature->GetGUID();
            break;
        case Anenga:
            m_ArtifactsNPCGuids[TEAM_ALLIANCE][CountForDruidShaman] = creature->GetGUID();
            break;
        case Frangraal:
            AddVignetteOnPlayers(creature, VignetteFangraal, TEAM_ALLIANCE);
            break;
        case Kronus:
            AddVignetteOnPlayers(creature, VignetteKronus, TEAM_HORDE);
            break;
        default:
            break;
    }
}

void OutdoorPvPAshran::OnCreatureRemove(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case KorlokTheOgreKing:
            RemoveVignetteOnPlayers(VignetteKorlok);
            break;
        case VignetteDummyA:
            RemoveVignetteOnPlayers(VignetteStormshieldPortal, TEAM_ALLIANCE);
            break;
        case VignetteDummyH:
            RemoveVignetteOnPlayers(VignetteWarspearPortal, TEAM_HORDE);
            break;
        case Frangraal:
            RemoveVignetteOnPlayers(VignetteFangraal, TEAM_ALLIANCE);
            break;
        case Kronus:
            RemoveVignetteOnPlayers(VignetteKronus, TEAM_HORDE);
            break;
        case HighWarlordVolrath:
            DelCreature(SLGGenericMoPLargeAoI + TEAM_HORDE);
            break;
        case GrandMarshalTremblade:
            DelCreature(SLGGenericMoPLargeAoI + TEAM_ALLIANCE);
            break;
        default:
            break;
    }
}

void OutdoorPvPAshran::OnGameObjectCreate(GameObject* gameObject)
{
    switch (gameObject->GetEntry())
    {
        case HordeGateway1:
            AddVignetteOnPlayers(gameObject, VignetteWarlockGateway1, TEAM_HORDE);
            break;
        case HordeGateway2:
            AddVignetteOnPlayers(gameObject, VignetteWarlockGateway2, TEAM_HORDE);
            break;
        case AllianceGateway1:
            AddVignetteOnPlayers(gameObject, VignetteWarlockGateway1, TEAM_ALLIANCE);
            break;
        case AllianceGateway2:
            AddVignetteOnPlayers(gameObject, VignetteWarlockGateway2, TEAM_ALLIANCE);
            break;
        default:
            break;
    }

    OutdoorPvP::OnGameObjectCreate(gameObject);
}

void OutdoorPvPAshran::OnGameObjectRemove(GameObject* gameObject)
{
    switch (gameObject->GetEntry())
    {
        case HordeGateway1:
            RemoveVignetteOnPlayers(VignetteWarlockGateway1, TEAM_HORDE);
            break;
        case HordeGateway2:
            RemoveVignetteOnPlayers(VignetteWarlockGateway2, TEAM_HORDE);
            break;
        case AllianceGateway1:
            RemoveVignetteOnPlayers(VignetteWarlockGateway1, TEAM_ALLIANCE);
            break;
        case AllianceGateway2:
            RemoveVignetteOnPlayers(VignetteWarlockGateway2, TEAM_ALLIANCE);
            break;
        default:
            break;
    }

    OutdoorPvP::OnGameObjectRemove(gameObject);
}

Creature* OutdoorPvPAshran::GetHerald() const
{
    return sObjectAccessor->FindCreature(m_HeraldGuid);
}

void OutdoorPvPAshran::ResetControlPoints()
{
    if (!m_WillBeReset)
        return;

    m_IsInitialized = false;

    for (uint8 l_I = 0; l_I < MaxTaxiToBases; ++l_I)
    {
        DelCreature(AllianceTaxiToBase1 + l_I);
        DelCreature(HordeTaxiToBase1 + l_I);
    }

    for (uint8 l_BattleIndex = EmberfallTower; l_BattleIndex < MaxBattleType; ++l_BattleIndex)
    {
        if (OPvPCapturePoint_Middle* l_CapturePoint = m_ControlPoints[l_BattleIndex])
        {
            if (g_MiddleBattlesEntries[l_BattleIndex] == m_CurrentBattleState)
            {
                l_CapturePoint->SetBattleFaction(ControlNeutral);
                l_CapturePoint->SetValue(0.0f);
                l_CapturePoint->SetState(OBJECTIVESTATE_NEUTRAL);
                l_CapturePoint->UpdateTowerState();
            }
            else
            {
                l_CapturePoint->SetBattleFaction(l_BattleIndex < 2 ? ControlHorde : ControlAlliance);
                l_CapturePoint->SetState(l_BattleIndex < 2 ? OBJECTIVESTATE_HORDE : OBJECTIVESTATE_ALLIANCE);
                l_CapturePoint->UpdateTowerState();
            }

            l_CapturePoint->SpawnFactionGuards(l_CapturePoint->GetBattleType(), l_CapturePoint->GetBattleFaction());
        }
    }

    SendUpdateWorldState(WorldStateWarspearOutpostStatus, ControlHorde);
    SendUpdateWorldState(WorldStateStormshieldStrongholdStatus, ControlAlliance);

    m_WillBeReset = false;
    m_IsInitialized = true;
}

void OutdoorPvPAshran::InitializeControlPoints()
{
    for (uint8 l_BattleId = EmberfallTower; l_BattleId < MaxBattleType; ++l_BattleId)
        if (OPvPCapturePoint_Middle* l_CapturePoint = m_ControlPoints[l_BattleId])
            l_CapturePoint->SpawnFactionGuards(l_CapturePoint->GetBattleType(), l_CapturePoint->GetBattleFaction());

    SpawnGladiators();
}

void OutdoorPvPAshran::InitializeEvents()
{
    uint32 l_Timer = 0;
    uint32 l_TimerInterval = AshranEventTimer * MINUTE * IN_MILLISECONDS / MaxEvents;
    for (uint8 l_Index = 0; l_Index < MaxEvents; ++l_Index)
    {
        if (l_Index != EventKorlokTheOgreKing && l_Index != EventStadiumRacing)
            break;

        l_Timer += l_TimerInterval;
        m_AshranEvents[l_Index] = l_Timer;
    }
}

bool OutdoorPvPAshran::IsInitialized() const
{
    return m_IsInitialized;
}

void OutdoorPvPAshran::SetBattleState(uint32 p_NewState)
{
    if (!m_IsInitialized)
        return;

    m_CurrentBattleState = p_NewState;

    if (!m_WillBeReset)
    {
        switch (m_CurrentBattleState)
        {
            case WorldStateEmberfallTowerBattle:
                DelCreature(HordeFactionBoss);
                DelCreature(HordeGeneralAevd);
                DelCreature(HordeWarlordNoktyn);
                AddCreature(HordeFactionBoss, g_FactionBossesSpawn[5], 5 * MINUTE);
                AddCreature(HordeGeneralAevd, g_FactionBossesGuardians[10], 5 * MINUTE);
                AddCreature(HordeWarlordNoktyn, g_FactionBossesGuardians[11], 5 * MINUTE);
                break;
            case WorldStateVolrathsAdvanceBattle:
                DelCreature(HordeFactionBoss);
                DelCreature(HordeGeneralAevd);
                DelCreature(HordeWarlordNoktyn);
                AddCreature(HordeFactionBoss, g_FactionBossesSpawn[4], 5 * MINUTE);
                AddCreature(HordeGeneralAevd, g_FactionBossesGuardians[8], 5 * MINUTE);
                AddCreature(HordeWarlordNoktyn, g_FactionBossesGuardians[9], 5 * MINUTE);
                break;
            case WorldStateTheCrossroadsBattle:
                DelCreature(AllianceFactionBoss);
                DelCreature(HordeFactionBoss);
                DelCreature(AllianceMarshalKarshStormforge);
                DelCreature(AllianceMarshalGabriel);
                DelCreature(HordeGeneralAevd);
                DelCreature(HordeWarlordNoktyn);
                AddCreature(AllianceFactionBoss, g_FactionBossesSpawn[0], 5 * MINUTE);
                AddCreature(AllianceMarshalKarshStormforge, g_FactionBossesGuardians[0], 5 * MINUTE);
                AddCreature(AllianceMarshalGabriel, g_FactionBossesGuardians[1], 5 * MINUTE);
                AddCreature(HordeFactionBoss, g_FactionBossesSpawn[3], 5 * MINUTE);
                AddCreature(HordeGeneralAevd, g_FactionBossesGuardians[6], 5 * MINUTE);
                AddCreature(HordeWarlordNoktyn, g_FactionBossesGuardians[7], 5 * MINUTE);
                break;
            case WorldStateTrembladesVanguardBattle:
                DelCreature(AllianceFactionBoss);
                DelCreature(AllianceMarshalKarshStormforge);
                DelCreature(AllianceMarshalGabriel);
                AddCreature(AllianceFactionBoss, g_FactionBossesSpawn[1], 5 * MINUTE);
                AddCreature(AllianceMarshalKarshStormforge, g_FactionBossesGuardians[2], 5 * MINUTE);
                AddCreature(AllianceMarshalGabriel, g_FactionBossesGuardians[3], 5 * MINUTE);
                break;
            case WorldStateArchmageOverwatchBattle:
                DelCreature(AllianceFactionBoss);
                DelCreature(AllianceMarshalKarshStormforge);
                DelCreature(AllianceMarshalGabriel);
                AddCreature(AllianceFactionBoss, g_FactionBossesSpawn[2], 5 * MINUTE);
                AddCreature(AllianceMarshalKarshStormforge, g_FactionBossesGuardians[4], 5 * MINUTE);
                AddCreature(AllianceMarshalGabriel, g_FactionBossesGuardians[5], 5 * MINUTE);
                break;
            default:
                break;
        }
    }

    for (uint32 l_BattleIndex : g_MiddleBattlesEntries)
    {
        if (m_CurrentBattleState == l_BattleIndex)
            SendUpdateWorldState(m_CurrentBattleState, WorldStateEnabled);
        else
            SendUpdateWorldState(l_BattleIndex, WorldStateDisabled);
    }

    if (m_CurrentBattleState == WorldStateHighWarlordVolrath)
        SendUpdateWorldState(WorldStateHighWarlordVolrath, WorldStateEnabled);
    else if (m_CurrentBattleState == WorldStateGrandMarshalTrembladeBattle)
        SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, WorldStateEnabled);
    else
    {
        SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, WorldStateDisabled);
        SendUpdateWorldState(WorldStateHighWarlordVolrath, WorldStateDisabled);
    }

    SendUpdateWorldState(WorldStateNextBattleTimestamp, uint32(time(nullptr)) + m_NextBattleTimer / IN_MILLISECONDS);
    SendUpdateWorldState(WorldStateNextBattleEnabled, WorldStateEnabled);
    SendUpdateWorldState(WorldStateControlTheFlag, WorldStateDisabled);
}

void OutdoorPvPAshran::SetNextBattleTimer(uint32 p_Time)
{
    m_NextBattleTimer = p_Time * IN_MILLISECONDS;
}

void OutdoorPvPAshran::AddGenericMoPGuid(uint8 type, ObjectGuid guid)
{
    m_GenericMoPGuids[type] = guid;
}

ObjectGuid OutdoorPvPAshran::GetGenericMoPGuid(uint8 type) const
{
    return m_GenericMoPGuids[type];
}

ObjectGuid OutdoorPvPAshran::GetFactionGenericMoP(uint8 p_Faction) const
{
    return m_FactionGenericMoP[p_Faction];
}

uint32 OutdoorPvPAshran::GetCurrentBattleType() const
{
    switch (m_CurrentBattleState)
    {
        case WorldStateEmberfallTowerBattle:
            return EmberfallTower;
        case WorldStateVolrathsAdvanceBattle:
            return VolrathsAdvance;
        case WorldStateTheCrossroadsBattle:
            return TheCrossroads;
        case WorldStateTrembladesVanguardBattle:
            return TrembladesVanguard;
        case WorldStateArchmageOverwatchBattle:
            return ArchmageOverwatch;
        default:
            return TheCrossroads;
    }
}

void OutdoorPvPAshran::HandleFactionBossDeath(uint8 p_Faction)
{
    if (m_CurrentBattleState == WorldStateHighWarlordVolrath)
    {
        SendUpdateWorldState(WorldStateSlayVolrath, WorldStateDisabled);
        SendUpdateWorldState(WorldStateHighWarlordVolrath, WorldStateDisabled);

        if (Creature* l_Tremblade = sObjectAccessor->FindCreature(m_GrandMasrhalTremblade))
            l_Tremblade->AI()->DoAction(StormshieldVictory);
    }
    else if (m_CurrentBattleState == WorldStateGrandMarshalTrembladeBattle)
    {
        SendUpdateWorldState(WorldStateSlayTremblade, WorldStateDisabled);
        SendUpdateWorldState(WorldStateGrandMarshalTrembladeBattle, WorldStateDisabled);

        if (Creature* l_Volrath = sObjectAccessor->FindCreature(m_HighWarlordVolrath))
            l_Volrath->AI()->DoAction(WarspearVictory);
    }
    else
        return;

    SetNextBattleTimer(10 * MINUTE);

    m_GladiatorRespawnTime = 3 * MINUTE * IN_MILLISECONDS;

    m_WillBeReset = true;
    SetBattleState(WorldStateTheCrossroadsBattle);

    for (uint8 l_I = 0; l_I < MaxTaxiToBases; ++l_I)
    {
        DelCreature(AllianceTaxiToBase1 + l_I);
        DelCreature(HordeTaxiToBase1 + l_I);

        if (p_Faction == TEAM_ALLIANCE)
            AddCreature(AllianceTaxiToBase1 + l_I, g_FactionTaxisToBase[p_Faction][l_I], 5 * MINUTE);
        else if (p_Faction == TEAM_HORDE)
            AddCreature(HordeTaxiToBase1 + l_I, g_FactionTaxisToBase[p_Faction][l_I], 5 * MINUTE);
        else
            return;
    }
}

void OutdoorPvPAshran::HandleCaptainDeath(uint32 type)
{
    DelCreature(type);

    if (m_ActiveCaptains.find(type) != m_ActiveCaptains.end())
        m_ActiveCaptains.erase(type);
}

OPvPCapturePoint_Middle* OutdoorPvPAshran::GetCapturePoint(uint8 p_Index) const
{
    return m_ControlPoints[p_Index];
}

WorldSafeLocsEntry const* OutdoorPvPAshran::GetClosestGraveyard(Player* player)
{
    WorldSafeLocsEntry const* graveyard = nullptr;

    float l_PosX = player->GetPositionX();
    float l_PosY = player->GetPositionY();
    float l_MinDist = 1000000.0f;

    uint8 l_TeamID = player->GetTeamId();
    for (uint8 l_I = 0; l_I < MaxGraveyards; ++l_I)
    {
        if (g_GraveyardIDs[l_TeamID][l_I] == AllianceCenter || g_GraveyardIDs[l_TeamID][l_I] == HordeCenter)
        {
            if (m_GraveYard)
            {
                uint8 l_State = m_GraveYard->GetGraveyardState();
                if (l_State == ControlNeutral || l_State == ControlAlliance && l_TeamID != TEAM_ALLIANCE ||
                    l_State == ControlHorde && l_TeamID != TEAM_HORDE)
                    continue;
            }
        }
        else if (g_GraveyardIDs[l_TeamID][l_I] == TowerAlliance)
        {
            if (OPvPCapturePoint_Middle* l_CapturePoint = m_ControlPoints[ArchmageOverwatch])
            {
                uint32 l_State = l_CapturePoint->GetBattleFaction();
                if (l_State == ControlNeutral || l_State == ControlAlliance && l_TeamID != TEAM_ALLIANCE ||
                    l_State == ControlHorde && l_TeamID != TEAM_HORDE)
                    continue;
            }
        }
        else if (g_GraveyardIDs[l_TeamID][l_I] == TowerHorde)
        {
            if (OPvPCapturePoint_Middle* l_CapturePoint = m_ControlPoints[EmberfallTower])
            {
                uint32 l_State = l_CapturePoint->GetBattleFaction();
                if (l_State == ControlNeutral || l_State == ControlHorde && l_TeamID != TEAM_HORDE ||
                    l_State == ControlAlliance && l_TeamID != TEAM_ALLIANCE)
                    continue;
            }
        }

        if (WorldSafeLocsEntry const* l_SafeLoc = sWorldSafeLocsStore.LookupEntry(g_GraveyardIDs[l_TeamID][l_I]))
        {
            float l_Dist = (l_SafeLoc->Loc.X - l_PosX) * (l_SafeLoc->Loc.X - l_PosX) + (l_SafeLoc->Loc.Y - l_PosY) * (l_SafeLoc->Loc.Y - l_PosY);
            if (l_MinDist > l_Dist)
            {
                l_MinDist = l_Dist;
                graveyard = l_SafeLoc;
            }
        }
    }

    return graveyard;
}

uint8 OutdoorPvPAshran::GetSpiritGraveyardID(uint32 areaID, TeamId p_Team)
{
    switch (areaID)
    {
        case AshranAllianceBase:
            return 0;
        case AshranHordeBase:
            return 1;
        case KingsRestAreaID:
        {
            if (p_Team == TEAM_ALLIANCE)
                return 2;
            else
                return 3;
        }
        case ArchmageOverwatchAreaID:
            return 4;
        case EmberfallTowerAreaID:
            return 5;
        default:
            break;
    }

    return 0;
}

uint32 OutdoorPvPAshran::GetArtifactCollected(uint8 teamID, uint8 type) const
{
    return m_ArtifactsCollected[teamID][type];
}

void OutdoorPvPAshran::AddCollectedArtifacts(uint8 teamID, uint8 type, uint32 p_Count)
{
    if (teamID > TEAM_HORDE || type >= MaxArtifactCounts || p_Count == 0)
        return;

    while (m_ArtifactsCollected[teamID][type] + p_Count >= g_MaxArtifactsToCollect[type])
    {
        p_Count -= g_MaxArtifactsToCollect[type] - m_ArtifactsCollected[teamID][type];
        m_ArtifactsCollected[teamID][type] = 0;
        StartArtifactEvent(teamID, type);
    };

    m_ArtifactsCollected[teamID][type] += p_Count;
    SendUpdateWorldState(g_ArtifactsWorldStates[teamID][type], m_ArtifactsCollected[teamID][type]);
}

void OutdoorPvPAshran::StartArtifactEvent(uint8 teamID, uint8 type)
{
    if (type >= MaxArtifactCounts || teamID > TEAM_HORDE)
        return;

    if (m_ArtifactEventsLaunched[teamID][type])
        return;

    m_ArtifactEventsLaunched[teamID][type] = true;
    AnnounceArtifactEvent(teamID, type, true);

    if (teamID == TEAM_ALLIANCE)
    {
        switch (type)
        {
            case CountForMage:
                AddCreature(AllianceMagePortal1, g_MagePortalsSpawns[teamID][0]);
                AddCreature(AllianceMagePortal2, g_MagePortalsSpawns[teamID][1]);
                AddCreature(AllianceVignetteDummy, g_MagePortalsSpawns[teamID][2]);
                AddCreature(AllianceKauper, g_MagePortalsSpawns[teamID][3]);
                AddObject(AlliancePortalToStormshield, g_MagePortalsGob[teamID]);
                break;
            case CountForWarlock:
                AddCreature(AllianceFalconAtherton, g_WarlockGatewaysSpawns[teamID][0]);
                AddCreature(AllianceDeckerWatts, g_WarlockGatewaysSpawns[teamID][1]);
                AddObject(AllianceWarlockGateway1, g_WarlockGatewaysGob[teamID][0]);
                AddObject(AllianceWarlockGateway2, g_WarlockGatewaysGob[teamID][1]);
                break;
            case CountForWarriorPaladin:
                m_ArtifactEventsLaunched[teamID][type] = false;
                break;
            case CountForDruidShaman:
                DelCreature(AllianceGuardian);
                AddCreature(AllianceFangraal, g_AllianceFangraal);
                break;
        }
    }
    else if (teamID == TEAM_HORDE)
    {
        switch (type)
        {
            case CountForMage:
                AddCreature(HordeMagePortal1, g_MagePortalsSpawns[teamID][0]);
                AddCreature(HordeMagePortal2, g_MagePortalsSpawns[teamID][1]);
                AddCreature(HordeVignetteDummy, g_MagePortalsSpawns[teamID][2]);
                AddCreature(HordeZaramSunraiser, g_MagePortalsSpawns[teamID][3]);
                AddObject(HordePortalToWarspear, g_MagePortalsGob[teamID]);
                break;
            case CountForWarlock:
                AddCreature(HordeGaylePlagueheart, g_WarlockGatewaysSpawns[teamID][0]);
                AddCreature(HordeIlyaPlagueheart, g_WarlockGatewaysSpawns[teamID][1]);
                AddObject(HordeWarlockGateway1, g_WarlockGatewaysGob[teamID][0]);
                AddObject(HordeWarlockGateway2, g_WarlockGatewaysGob[teamID][1]);
                break;
            case CountForWarriorPaladin:
                m_ArtifactEventsLaunched[teamID][type] = false;
                break;
            case CountForDruidShaman:
                DelCreature(HordeGuardian);
                AddCreature(HordeKronus, g_HordeKronus);
                break;
        }
    }
}

void OutdoorPvPAshran::EndArtifactEvent(uint8 teamID, uint8 type)
{
    if (type >= MaxArtifactCounts || teamID > TEAM_HORDE)
        return;

    if (!m_ArtifactEventsLaunched[teamID][type])
        return;

    m_ArtifactEventsLaunched[teamID][type] = false;
    AnnounceArtifactEvent(teamID, type, false);

    if (teamID == TEAM_ALLIANCE)
    {
        switch (type)
        {
            case CountForMage:
                DelCreature(AllianceMagePortal1);
                DelCreature(AllianceMagePortal2);
                DelCreature(AllianceVignetteDummy);
                DelCreature(AllianceKauper);
                DelObject(AlliancePortalToStormshield);
                break;
            case CountForWarlock:
                DelCreature(AllianceFalconAtherton);
                DelCreature(AllianceDeckerWatts);
                DelObject(AllianceWarlockGateway1);
                DelObject(AllianceWarlockGateway2);
                break;
            case CountForWarriorPaladin:
                break;
            case CountForDruidShaman:
                AddCreature(AllianceGuardian, g_AllianceGuardian);
                DelCreature(AllianceFangraal);
                break;
            default:
                break;
        }
    }
    else if (teamID == TEAM_HORDE)
    {
        switch (type)
        {
            case CountForMage:
                DelCreature(HordeMagePortal1);
                DelCreature(HordeMagePortal2);
                DelCreature(HordeVignetteDummy);
                DelCreature(HordeZaramSunraiser);
                DelObject(HordePortalToWarspear);
                break;
            case CountForWarlock:
                DelCreature(HordeGaylePlagueheart);
                DelCreature(HordeIlyaPlagueheart);
                DelObject(HordeWarlockGateway1);
                DelObject(HordeWarlockGateway2);
                break;
            case CountForWarriorPaladin:
                break;
            case CountForDruidShaman:
                AddCreature(HordeGuardian, g_HordeGuardian);
                DelCreature(HordeKronus);
                break;
            default:
                break;
        }
    }
}

bool OutdoorPvPAshran::IsArtifactEventLaunched(uint8 teamID, uint8 type) const
{
    if (type >= MaxArtifactCounts || teamID > TEAM_HORDE)
        return false;

    return m_ArtifactEventsLaunched[teamID][type];
}

void OutdoorPvPAshran::AnnounceArtifactEvent(uint8 teamID, uint8 type, bool p_Apply)
{
    if (type >= MaxArtifactCounts || teamID > TEAM_HORDE)
        return;

    if (!m_ArtifactsNPCGuids[teamID][type])
        return;

    Creature* l_Creature = sObjectAccessor->FindCreature(m_ArtifactsNPCGuids[teamID][type]);
    if (l_Creature == nullptr || l_Creature->GetAI() == nullptr)
        return;

    l_Creature->AI()->Talk(p_Apply ? 0 : 1);
}

void OutdoorPvPAshran::RewardHonorAndReputation(uint32 p_ArtifactCount, Player* player)
{
    if (player == nullptr)
        return;

    player->PlayerTalkClass->SendCloseGossip();

    player->RewardHonor(nullptr, 1, p_ArtifactCount * HonorConversionRate * CURRENCY_PRECISION);

    FactionEntry const* l_Faction = sFactionStore.LookupEntry(player->GetTeamId() == TEAM_ALLIANCE ? WrynnsVanguard : VoljinsSpear);
    if (l_Faction == nullptr)
        return;

    player->GetReputationMgr().ModifyReputation(l_Faction, p_ArtifactCount * ReputationConversionRate);
}

template <class T>
void OutdoorPvPAshran::AddVignetteOnPlayers(T const* object, uint32 vignetteID, uint8 teamID /*= TeamId::TEAM_NEUTRAL*/)
{
    VignetteEntry const* vignette = sVignetteStore.LookupEntry(vignetteID);
    if (!vignette)
        return;

    ObjectGuid guid = object->GetGUID();

    if (teamID == TEAM_NEUTRAL)
        m_NeutralVignettes.insert(std::make_pair(vignetteID, guid));
    else
        m_FactionVignettes[teamID].insert(std::make_pair(vignetteID, guid));

    for (uint8 l_Team = 0; l_Team < MAX_TEAMS; ++l_Team)
    {
        if (teamID != TEAM_NEUTRAL && l_Team != teamID)
            continue;

        for (auto v : m_PlayersInWar[l_Team])
            if (auto player = sObjectAccessor->FindPlayer(v))
                player->GetVignetteMgr().CreateAndAddVignette(vignette, AshranMapID, Vignette::Type::SourceScript, object->GetPosition(), object->GetAreaId(), v);
    }
}

template void OutdoorPvPAshran::AddVignetteOnPlayers(Creature const* /*creature*/, uint32 /*vignetteID*/, uint8 /*teamID*/ /*= TEAM_NEUTRAL*/);
template void OutdoorPvPAshran::AddVignetteOnPlayers(GameObject const* /*go*/, uint32 /*vignetteID*/, uint8 /*teamID*/ /*= TEAM_NEUTRAL*/);

void OutdoorPvPAshran::RemoveVignetteOnPlayers(uint32 vignetteID, uint8 teamID /*= TEAM_NEUTRAL*/)
{
    VignetteEntry const* vignette = sVignetteStore.LookupEntry(vignetteID);
    if (!vignette)
        return;

    if (teamID == TEAM_NEUTRAL)
        m_NeutralVignettes.erase(vignetteID);
    else
        m_FactionVignettes[teamID].erase(vignetteID);

    for (uint8 l_Team = 0; l_Team < MAX_TEAMS; ++l_Team)
    {
        if (teamID != TEAM_NEUTRAL && l_Team != teamID)
            continue;

        for (auto guid : m_PlayersInWar[l_Team])
            if (auto player = sObjectAccessor->FindPlayer(guid))
                player->GetVignetteMgr().DestroyAndRemoveVignetteByEntry(vignette);
    }
}

uint32 OutdoorPvPAshran::CountPlayersForTeam(uint8 teamID) const
{
    if (teamID > TEAM_HORDE)
        return 0;

    return static_cast<uint32>(m_PlayersInWar[teamID].size());
}

void OutdoorPvPAshran::CastSpellOnTeam(Unit* caster, uint8 p_Team, uint32 spellID)
{
    if (p_Team > TEAM_HORDE)
        return;

    for (ObjectGuid guid : m_PlayersInWar[p_Team])
    {
        //if (!caster->getThreatManager().HaveInThreatList(guid))
        //    continue;

        if (Player* l_Player = Player::GetPlayer(*caster, guid))
            caster->CastSpell(l_Player, spellID, true);
    }
}

class OutdoorPvP_Ashran : public OutdoorPvPScript
{
public:

    OutdoorPvP_Ashran() : OutdoorPvPScript("outdoorpvp_ashran") { }

    OutdoorPvP* GetOutdoorPvP() const override
    {
        return new OutdoorPvPAshran();
    }
};

void AddSC_AshranMgr()
{
    //new OutdoorPvP_Ashran();
}
