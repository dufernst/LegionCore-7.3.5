////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ASHRAN_MGR_HPP_ASHRAN
# define ASHRAN_MGR_HPP_ASHRAN

# include "OutdoorPvP.h"
# include "OutdoorPvPMgr.h"
# include "AshranDatas.hpp"
# include "ScriptMgr.h"
# include "ScriptedCreature.h"
# include "ScriptedGossip.h"
# include "ScriptedEscortAI.h"
# include "Player.h"
# include "WorldPacket.h"
# include "World.h"
# include "ObjectMgr.h"
# include "Language.h"
# include "CreatureTextMgr.h"
# include "MoveSplineInit.h"
# include "LFGMgr.h"
# include "Group.h"
#include "PrecompiledHeaders/ScriptPCH.h"
# include "MapManager.h"
#include "SpellAuraEffects.h"

class OutdoorPvPAshran;

class OutdoorGraveyardAshran : public OutdoorGraveyard
{
public:
    explicit OutdoorGraveyardAshran(OutdoorPvPAshran* p_OutdoorPvP);
};

class OPvPCapturePoint_Middle : public OPvPCapturePoint
{
public:
    OPvPCapturePoint_Middle(OutdoorPvP* outdoor, eBattleType type, uint8 p_Faction);

    void ChangeState() override;

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void UpdateTowerState();

    bool Update(uint32 p_Diff) override;

    void SpawnFactionGuards(eBattleType p_BattleID, uint8 p_Faction);

    void SetBattleFaction(uint32 p_Faction);
    uint32 GetBattleFaction() const;
    eBattleType GetBattleType() const;

protected:

    eBattleType m_BattleType;
    uint32 m_BattleFaction;
};

class OPvPCapturePoint_Graveyard : public OPvPCapturePoint
{
public:
    explicit OPvPCapturePoint_Graveyard(OutdoorPvP* outdoor);

    void ChangeState() override;

    void SendChangePhase() override;

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void UpdateTowerState();

    bool HandlePlayerEnter(Player* player) override;
    void HandlePlayerLeave(Player* player) override;

    void SpawnFactionFlags(uint8 p_Faction);

    bool Update(uint32 p_Diff) override;
    void ScheduleNextControl(uint32 p_Diff);

    uint8 GetGraveyardState() const;

protected:

    uint8 m_GraveyardState;
    uint32 m_ControlTime;
};

class OutdoorPvPAshran : public OutdoorPvP
{
    using PlayerTimerMap = std::map<ObjectGuid, uint32>;
    using PlayerCurrencyLoot = std::map<ObjectGuid, uint32>;
    using AshranVignettesMap = std::map<uint32, ObjectGuid>;
    using ActiveCaptains = std::set<uint32>;

public:
    OutdoorPvPAshran();

    bool SetupOutdoorPvP() override;

    void Initialize(uint32 zone) override;

    void HandlePlayerEnterMap(ObjectGuid guid, uint32 zoneID) override;
    void HandlePlayerLeaveMap(ObjectGuid guid, uint32 zoneID) override ;
    void HandlePlayerEnterArea(ObjectGuid guid, uint32 areaID) override;
    void HandlePlayerLeaveArea(ObjectGuid guid, uint32 areaID) override;
    void HandlePlayerResurrects(Player* player, uint32 zoneID) override;

    void HandlePlayerKilled(Player* player) override;
    void HandleKill(Player* killer, Unit* killed) override;
    void ResetKillCap(uint8 p_Team);

    static bool IsFactionGuard(Unit* p_Unit);
    void SpawnGladiators(uint8 teamID = TEAM_NEUTRAL, bool p_Spawn = true);

    void FillCustomPvPLoots(Player* looter, Loot& loot, ObjectGuid container) override;

    bool Update(uint32 p_Diff) override;
    void ScheduleNextBattle(uint32 p_Diff);
    void ScheduleEndOfBattle(uint32 p_Diff);
    void ScheduleInitPoints(uint32 p_Diff);
    void ScheduleEventsUpdate(uint32 p_Diff);
    void ScheduleGladiatorRespawn(uint32 p_Diff);

    void StartEvent(uint8 p_EventID);
    void EndEvent(uint8 p_EventID, bool p_ScheduleNext = true);
    void SendEventWarningToPlayers(uint32 p_LangID);
    void SetEventData(uint8 p_EventID, uint8 teamID, uint32 p_Data);

    void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet) override;
    void SendRemoveWorldStates(Player* player) override;

    void HandleBFMGREntryInviteResponse(bool accepted, Player* player) override;
    bool HandleOpenGo(Player* player, ObjectGuid guid) override;
    void HandleArtifactDrop(Unit* p_Unit, uint32 p_Time);

    void OnCreatureCreate(Creature* creature) override;
    void OnCreatureRemove(Creature* creature) override;
    void OnGameObjectCreate(GameObject* gameObject) override;
    void OnGameObjectRemove(GameObject* gameObject) override;
    Creature* GetHerald() const;

    void ResetControlPoints();
    void InitializeControlPoints();
    void InitializeEvents();
    bool IsInitialized() const;

    void SetBattleState(uint32 p_NewState);
    void SetNextBattleTimer(uint32 p_Time);

    void AddGenericMoPGuid(uint8 type, ObjectGuid guid);
    ObjectGuid GetGenericMoPGuid(uint8 type) const;
    ObjectGuid GetFactionGenericMoP(uint8 p_Faction) const;

    uint32 GetCurrentBattleType() const;

    void HandleFactionBossDeath(uint8 p_Faction);
    void HandleCaptainDeath(uint32 type);

    OPvPCapturePoint_Middle* GetCapturePoint(uint8 p_Index) const;

    WorldSafeLocsEntry const* GetClosestGraveyard(Player* player) override;
    static uint8 GetSpiritGraveyardID(uint32 areaID, TeamId p_Team);

    uint32 GetArtifactCollected(uint8 teamID, uint8 type) const;
    void AddCollectedArtifacts(uint8 teamID, uint8 type, uint32 p_Count);
    static void RewardHonorAndReputation(uint32 p_ArtifactCount, Player* player);
    void StartArtifactEvent(uint8 teamID, uint8 type);
    void EndArtifactEvent(uint8 teamID, uint8 type);
    bool IsArtifactEventLaunched(uint8 teamID, uint8 type) const;
    void AnnounceArtifactEvent(uint8 teamID, uint8 type, bool p_Apply);

    template<class T>
    void AddVignetteOnPlayers(T const* object, uint32 vignetteID, uint8 teamID = TEAM_NEUTRAL);
    void RemoveVignetteOnPlayers(uint32 vignetteID, uint8 teamID = TEAM_NEUTRAL);

    uint32 CountPlayersForTeam(uint8 teamID) const;

    void CastSpellOnTeam(Unit* caster, uint8 p_Team, uint32 spellID);

private:

    OPvPCapturePoint_Graveyard* m_GraveYard;
    OPvPCapturePoint_Middle* m_ControlPoints[MaxBattleType];
    ObjectGuid m_GenericMoPGuids[MaxBattleType];
    ObjectGuid m_FactionGenericMoP[TEAM_NEUTRAL];
    uint32 m_InitPointsTimer;
    bool m_IsInitialized;
    bool m_WillBeReset;

    uint64 m_Guid;
    ObjectGuid m_HeraldGuid;
    ObjectGuid m_HighWarlordVolrath;
    ObjectGuid m_GrandMasrhalTremblade;
    uint32 m_WorldPvPAreaId;

    GuidSet m_PlayersInWar[TEAM_NEUTRAL];
    PlayerTimerMap m_InvitedPlayers[TEAM_NEUTRAL];
    PlayerTimerMap m_PlayersWillBeKick[TEAM_NEUTRAL];
    PlayerCurrencyLoot m_PlayerCurrencyLoots;

    uint32 m_EnnemiesKilled[TEAM_NEUTRAL];
    uint32 m_EnnemiesKilledMax[TEAM_NEUTRAL];

    ObjectGuid m_ArtifactsNPCGuids[TEAM_NEUTRAL][MaxArtifactCounts];
    uint32 m_ArtifactsCollected[TEAM_NEUTRAL][MaxArtifactCounts];
    bool m_ArtifactEventsLaunched[TEAM_NEUTRAL][MaxArtifactCounts];

    uint32 m_StadiumRacingLaps[TEAM_NEUTRAL];
    uint32 m_AshranEvents[MaxEvents];
    bool m_AshranEventsWarned[MaxEvents];
    bool m_AshranEventsLaunched[MaxEvents];

    uint32 m_CurrentBattleState;
    uint32 m_NextBattleTimer;
    uint32 m_MaxBattleTime;
    uint32 m_GladiatorRespawnTime;
    uint32 m_AncientArtifactTime;

    AshranVignettesMap m_NeutralVignettes;
    AshranVignettesMap m_FactionVignettes[TEAM_NEUTRAL];
    ActiveCaptains m_ActiveCaptains;
};

#endif ///< ASHRAN_MGR_HPP_ASHRAN
