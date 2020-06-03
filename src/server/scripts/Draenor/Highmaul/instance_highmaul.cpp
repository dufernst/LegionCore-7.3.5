////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"
#include "Packets/WorldStatePackets.h"

DoorData const g_DoorData[] =
{
    { GateArenaExit,      BossKargathBladefist,   DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { EarthenPillar,      BossTheButcher,         DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { FungalGiantDoor,    BossKargathBladefist,   DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { WindDoor,           BossKargathBladefist,   DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { WindDoor,           BossBrackenspore,       DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { Earthwall1,         BossTectus,             DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { Earthwall2,         BossTectus,             DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { Earthwall3,         BossTectus,             DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { Earthwall4,         BossTectus,             DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { HighmaulLFRDoor,    BossTectus,             DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { TwinOgronEntrance,  BossTwinOgron,          DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { TwinOgronExit,      BossTwinOgron,          DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { FelBreakerEntrance, BossKoragh,             DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { FelBreakerExitDoor, BossKoragh,             DOOR_TYPE_PASSAGE,    BOUNDARY_NONE },
    { ThroneRoomDoor,     BossImperatorMargok,    DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { StairBlockingDoor,  BossImperatorMargok,    DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { StairBlockingDoor,  BossImperatorMargok,    DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { StairBlockingDoor,  BossImperatorMargok,    DOOR_TYPE_ROOM,       BOUNDARY_NONE },
    { 0,                  0,                      DOOR_TYPE_ROOM,       BOUNDARY_NONE } ///< End
};

ObjectData const creatureData[] =
{
    {GhargArenaMaster,      GhargArenaMaster},
    {KargathBladefist,      KargathBladefist},
    {JhornTheMad,           JhornTheMad},
    {ThoktarIronskull,      ThoktarIronskull},
    {Vulgor,                Vulgor},
    {CrowdAreatrigger,      CrowdAreatrigger},
    {MargokCosmetic,        MargokCosmetic},
    {TheButcher,            TheButcher},
    {Brackenspore,          Brackenspore},
    {Tectus,                Tectus},
    {Phemos,                Phemos},
    {Pol,                   Pol},
    {Koragh,                Koragh},
    {ImperatorMargok,       ImperatorMargok},
    {HighCouncilorMalgris,  HighCouncilorMalgris},
    {Rokka,                 Rokka},
    {Oro,                   Oro},
    {Lokk,                  Lokk},
    { 0,                    0} // END
};

class instance_highmaul : public InstanceMapScript
{
public:
    instance_highmaul() : InstanceMapScript(hScriptName, 1228) { }

    struct instance_highmaul_InstanceMapScript : public InstanceScript
    {
        instance_highmaul_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            m_ArenaElevatorActivated = false;
            m_BrackensporeAchievement = false;
            m_DrunkenBileslingerCount = 0;
            m_ForTests = false;
            m_ImperatorAchievement = false;
            m_Initialized = false;
            m_IronBombersCount = 0;
            m_KargathAchievement = true;
            m_MaggotKilled = 0;
            m_MoteOfTectusTime = 0;
            m_NullificationBarrier = false;
            m_OverflowingEnergies = 0;
            m_PlayerPhases.clear();
            m_TectusAchievement = false;
            m_TwinOgronAchievement = true;
        }

        WorldLocation loc_res_pla;
        std::map<uint32, uint32> m_PlayerPhases;
        ObjectGuid m_ColiseumLFRDoor;
        ObjectGuid m_ArenaElevatorGuid;
        ObjectGuid m_CollisionWallGuid;
        ObjectGuid m_GateArenaInnerGuid;
        ObjectGuid m_RaidGrateGuids[4];
        uint32 m_DungeonID;
        uint32 m_MoteOfTectusTime;
        uint8 m_DrunkenBileslingerCount;
        uint8 m_IronBombersCount;
        uint8 m_MaggotKilled;
        uint8 m_OverflowingEnergies;
        bool m_ArenaElevatorActivated;
        bool m_BrackensporeAchievement;
        bool m_ImperatorAchievement;
        bool m_KargathAchievement;
        bool m_NullificationBarrier;
        bool m_TectusAchievement;
        bool m_TwinOgronAchievement;
        bool m_ForTests;
        bool m_Initialized;

        void Initialize() override
        {
            SetBossNumber(MaxHighmaulBosses);
            //LoadDoorData(g_DoorData);
            //LoadObjectData(creatureData, nullptr);

            //instance->SetObjectVisibility(200.0f);
        }

        void OnCreatureCreate(Creature* creature) override
        {
            /* switch (creature->GetEntry())
            {
                case IronBomberSpawner:
                    creature->SetReactState(REACT_PASSIVE);
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    break;
                case IronBomber:
                    if (!m_IronBombersCount)
                        SendUpdateWorldState(IronBomberEnable, 1);
                    ++m_IronBombersCount;
                    SendUpdateWorldState(IronBomberRemaining, m_IronBombersCount);
                    break;
                case DrunkenBileslinger:
                    if (!m_DrunkenBileslingerCount)
                        SendUpdateWorldState(DrunkenBileslingerEnable, 1);
                    ++m_DrunkenBileslingerCount;
                    SendUpdateWorldState(DrunkenBileslingerRemaining, m_DrunkenBileslingerCount);
                    break;
                case IronGrunt:
                case BlackrockGrunt:
                case LowBatchDeadPale:
                case NightTwistedPaleVis:
                case CosmeticGorianWarr:
                case GorianCivilian:
                case RuneOfNullification:
                    creature->SetReactState(REACT_PASSIVE);
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    break;
                default:
                    break;
            } */
        }

        void OnCreatureRemove(Creature* creature) override
        {
            /* switch (creature->GetEntry())
            {
                case IronBomber:
                {
                    if (!m_IronBombersCount)
                        break;

                    --m_IronBombersCount;
                    SendUpdateWorldState(IronBomberRemaining, m_IronBombersCount);

                    if (!m_IronBombersCount)
                    {
                        SendUpdateWorldState(IronBomberEnable, 0);
                        break;
                    }
                    break;
                }
                case DrunkenBileslinger:
                {
                    if (!m_DrunkenBileslingerCount)
                        break;

                    --m_DrunkenBileslingerCount;
                    SendUpdateWorldState(DrunkenBileslingerRemaining, m_DrunkenBileslingerCount);

                    if (!m_DrunkenBileslingerCount)
                    {
                        SendUpdateWorldState(DrunkenBileslingerEnable, 0);
                        break;
                    }
                    break;
                }
                default:
                    break;
            } */
        }

        void OnGameObjectCreate(GameObject* gameObject) override
        {
            switch (gameObject->GetEntry())
            {
                case GateArenaExit:
                    if (!instance->IsLfr())
                        AddDoor(gameObject, true);
                    break;
                case FungalGiantDoor:
                case EarthenPillar:
                case WindDoor:
                case Earthwall1:
                case Earthwall2:
                case Earthwall3:
                case Earthwall4:
                case TwinOgronEntrance:
                case TwinOgronExit:
                case FelBreakerEntrance:
                case FelBreakerExitDoor:
                case ThroneRoomDoor:
                case StairBlockingDoor:
                case HighmaulLFRDoor:
                    AddDoor(gameObject, true);
                    break;
                /* case ArenaElevator:
                    m_ArenaElevatorGuid = gameObject->GetGUID();
                    gameObject->SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, 0);
                    gameObject->SetGoState(GO_STATE_READY);
                    break;
                case CollisionWall:
                    m_CollisionWallGuid = gameObject->GetGUID();
                    gameObject->SetGoState(GO_STATE_ACTIVE);
                    break;
                case GateArenaInner:
                    m_GateArenaInnerGuid = gameObject->GetGUID();
                    break;
                case RaidGrate1:
                    m_RaidGrateGuids[RaidGrate001] = gameObject->GetGUID();
                    break;
                case RaidGrate2:
                    m_RaidGrateGuids[RaidGrate002] = gameObject->GetGUID();
                    break;
                case RaidGrate3:
                    m_RaidGrateGuids[RaidGrate003] = gameObject->GetGUID();
                    break;
                case RaidGrate4:
                    m_RaidGrateGuids[RaidGrate004] = gameObject->GetGUID();
                    break;
                case HighmaulLFRDoorColiseum:
                    m_ColiseumLFRDoor = gameObject->GetGUID();
                    break; */
                default:
                    break;
            }
        }

        void OnGameObjectRemove(GameObject* gameObject) override
        {
            switch (gameObject->GetEntry())
            {
                case GateArenaExit:
                    if (!instance->IsLfr())
                        AddDoor(gameObject, false);
                    break;
                case FungalGiantDoor:
                case EarthenPillar:
                case WindDoor:
                case Earthwall1:
                case Earthwall2:
                case Earthwall3:
                case Earthwall4:
                case TwinOgronEntrance:
                case TwinOgronExit:
                case FelBreakerEntrance:
                case FelBreakerExitDoor:
                case ThroneRoomDoor:
                case StairBlockingDoor:
                case HighmaulLFRDoor:
                    AddDoor(gameObject, false);
                    break;
                default:
                    break;
            }
        }

        bool SetBossState(uint32 bossID, EncounterState state) override
        {
            if (!InstanceScript::SetBossState(bossID, state))
                return false;

            return true;

            switch (bossID)
            {
                case BossKargathBladefist:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_KargathAchievement = true;
                            break;
                        case DONE:
                            SendUpdateWorldState(DisableCrowdSound, 1);
                            SendUpdateWorldState(UnknownHighmaulWorldState, 0);
                            SendUpdateWorldState(UnknownHighmaulWorldState2, 0);
                            //PlaySceneForPlayers(g_PlayScenePos, 1338);
                            CastSpellToPlayers(instance, nullptr, ChogallNight, true);

                            if (!instance->IsLfr() && m_KargathAchievement)
                                DoCompleteAchievement(FlameOn);
                            break;
                        default:
                            break;
                    }

                    break;
                }
                case BossTheButcher:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_MaggotKilled = 0;
                            break;
                        case DONE:
                            if (!instance->IsLfr() && m_MaggotKilled >= MaxMaggotToKill)
                                DoCompleteAchievement(HurryUpMaggot);
                            break;
                        default:
                            break;
                    }

                    break;
                }
                case BossTectus:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_MoteOfTectusTime = 0;
                            m_TectusAchievement = false;
                            break;
                        case DONE:
                            if (!instance->IsLfr() && m_TectusAchievement)
                                DoCompleteAchievement(MoreLikeWreckedUs);
                            break;
                        default:
                            break;
                    }

                    break;
                }
                case BossBrackenspore:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_BrackensporeAchievement = false;
                            break;
                        case DONE:
                            if (!instance->IsLfr() && m_BrackensporeAchievement)
                                DoCompleteAchievement(AFungusAmongUs);
                            break;
                        default:
                            break;
                    }

                    break;
                }
                case BossTwinOgron:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_TwinOgronAchievement = true;
                            break;
                        case DONE:
                            if (!instance->IsLfr() && m_TwinOgronAchievement)
                                DoCompleteAchievement(BrothersInArms);
                            break;
                        default:
                            break;
                    }
                    break;
                }
                case BossKoragh:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_OverflowingEnergies = 0;
                            m_NullificationBarrier = false;
                            break;
                        case DONE:
                            if (!instance->IsLfr() && m_NullificationBarrier && m_OverflowingEnergies < MaxOverflowingEnergy)
                                DoCompleteAchievement(PairAnnihilation);
                            break;
                        default:
                            break;
                    }

                    break;
                }
                case BossImperatorMargok:
                {
                    switch (state)
                    {
                        case FAIL:
                        case NOT_STARTED:
                            m_ImperatorAchievement = false;
                            break;
                        case DONE:
                        {
                            if (!instance->IsLfr() && m_ImperatorAchievement)
                                DoCompleteAchievement(LineageOfPower);

                            if (instance->IsMythicRaid())
                            {
                                DoCompleteAchievement(AheadOfTheCurve);
                                DoCompleteAchievement(CuttingEdge);
                            }
                            else if (instance->IsHeroic())
                                DoCompleteAchievement(AheadOfTheCurve);

                            break;
                        }
                        default:
                            break;
                    }

                    break;
                }
                default:
                    break;
            }

            return true;
        }

        void SetData(uint32 type, uint32 data) override
        {
            return;

            switch (type)
            {
                case ElevatorActivated:
                    m_ArenaElevatorActivated = data;
                    break;
                case TestsActivated:
                {
                    m_ForTests = data != 0;

                    /// Open all doors for tests
                    if (m_ForTests)
                    {
                        for (uint8 i = 0; i < MaxHighmaulBosses; ++i)
                        {
                            BossInfo* l_BossInfos = GetBossInfo(i);
                            if (l_BossInfos != nullptr)
                                for (uint32 l_Type = 0; l_Type < MAX_DOOR_TYPES; ++l_Type)
                                    for (DoorSet::iterator itr = l_BossInfos->door[l_Type].begin(); itr != l_BossInfos->door[l_Type].end(); ++itr)
                                        (*itr)->SetGoState(GO_STATE_ACTIVE);
                        }
                    }

                    break;
                }
                case KargathAchievement:
                    m_KargathAchievement = false;
                    break;
                case ButcherAchievement:
                    m_MaggotKilled++;
                    break;
                case TectusAchievement:
                    if (!m_MoteOfTectusTime)
                        m_MoteOfTectusTime = data;
                    else
                    {
                        /// Defeat Tectus in Highmaul by destroying all eight Motes of Tectus within 10 seconds of each other on Normal difficulty or higher.
                        if (data > (m_MoteOfTectusTime + 10))
                            m_TectusAchievement = false;
                        else
                            m_TectusAchievement = true;
                    }
                    break;
                case BrackensporeAchievement:
                    m_BrackensporeAchievement = true;
                    break;
                case TwinOgronAchievement:
                    m_TwinOgronAchievement = false;
                    break;
                case KoraghOverflowingEnergy:
                    ++m_OverflowingEnergies;
                    break;
                case KoraghNullificationBarrier:
                    m_NullificationBarrier = true;
                    break;
                case ImperatorAchievement:
                    m_ImperatorAchievement = true;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            return 0;
            switch (type)
            {
                case ElevatorActivated:
                    return m_ArenaElevatorActivated;
                case TestsActivated:
                    return m_ForTests;
                case KoraghAchievement:
                    return !instance->IsLfr() && m_NullificationBarrier && m_OverflowingEnergies < MaxOverflowingEnergy;
                default:
                    return 0;
            }
        }

        ObjectGuid GetGuidData(uint32 type) const
        {
            /* switch (type)
            {
                case ArenaElevator:
                    return m_ArenaElevatorGuid;
                case CollisionWall:
                    return m_CollisionWallGuid;
                case GateArenaInner:
                    return m_GateArenaInnerGuid;
                case RaidGrate1:
                    return m_RaidGrateGuids[RaidGrate001];
                case RaidGrate2:
                    return m_RaidGrateGuids[RaidGrate002];
                case RaidGrate3:
                    return m_RaidGrateGuids[RaidGrate003];
                case RaidGrate4:
                    return m_RaidGrateGuids[RaidGrate004];
                default:
                    break;
            } */

            return ObjectGuid::Empty;
        }

        bool CheckRequiredBosses(uint32 bossID, uint32 entry, Player const* player = nullptr) const override
        {
            return true;

            if (m_ForTests)
                return true;

            if (!InstanceScript::CheckRequiredBosses(bossID, entry, player))
                return false;

            /// Highmaul has 4 main encounters (Kargath Bladefist, Ko'ragh, Twin Ogron, Imperator Mar'gok).
            /// There are also three optional encounters - The Butcher, Brackenspore and Tectus.
            switch (bossID)
            {
                case BossTwinOgron:
                    if (m_DungeonID != ArcaneSanctum && GetBossState(BossKargathBladefist) != DONE)
                        return false;
                    break;
                case BossKoragh:
                    if (GetBossState(bossID - 1) != DONE)
                        return false;
                    break;
                case BossImperatorMargok:
                    if (m_DungeonID != ImperatorsFall && GetBossState(bossID - 1) != DONE)
                        return false;
                    break;
                default:
                    break;
            }

            return true;
        }

        void FillInitialWorldStates(WorldPackets::WorldState::InitWorldStates& packet)
        {
            packet.Worldstates.emplace_back(IronBomberEnable, 0);
            packet.Worldstates.emplace_back(IronBomberRemaining, 0);
            packet.Worldstates.emplace_back(DrunkenBileslingerEnable, 0);
            packet.Worldstates.emplace_back(DrunkenBileslingerRemaining, 0);
        }

        void OnPlayerEnter(Player* player) override
        {
            InstanceScript::OnPlayerEnter(player);

            return;

            bool l_ChogallNight = false;
            if (GetBossState(BossKargathBladefist) == DONE)
            {
                l_ChogallNight = true;

                /// We don't need to update the enter pos if player is summoned by his allies
                //if (!player->IsSummoned())
                {
                    ObjectGuid guid = player->GetGUID();
                    AddDelayedEvent(200, [this, guid]() -> void
                    {
                        if (Player* player = sObjectAccessor->FindPlayer(guid))
                        {
                            if (GetBossState(BossKoragh) == DONE)
                                player->NearTeleportTo(FelBreakerRoom);
                            else if (GetBossState(BossTectus) == DONE)
                                player->NearTeleportTo(PalaceFrontGate);
                            else if (GetBossState(BossTheButcher) == DONE)
                                player->NearTeleportTo(BeachEntrance);
                            else
                                player->NearTeleportTo(KargathDefeated);
                        }
                    });
                }
            }

            /// Disable non available bosses for LFR
            if (!m_Initialized)
            {
                m_Initialized = true;

                //m_DungeonID = player->GetGroup() ? sLFGMgr->GetDungeon(player->GetGroup()->GetGUID()) : 0;

                if (!instance->IsLfr())
                    m_DungeonID = 0;

                switch (m_DungeonID)
                {
                    case WalledCity:
                    {
                        uint32 l_DisabledMask = 0;

                        l_DisabledMask |= (1 << BossTectus);
                        l_DisabledMask |= (1 << BossTwinOgron);
                        l_DisabledMask |= (1 << BossKoragh);
                        l_DisabledMask |= (1 << BossImperatorMargok);

                        SetDisabledBosses(l_DisabledMask);
                        break;
                    }
                    case ArcaneSanctum:
                    {
                        uint32 l_DisabledMask = 0;

                        l_DisabledMask |= (1 << BossKargathBladefist);
                        l_DisabledMask |= (1 << BossTheButcher);
                        l_DisabledMask |= (1 << BossBrackenspore);
                        l_DisabledMask |= (1 << BossImperatorMargok);

                        SetDisabledBosses(l_DisabledMask);

                        if (GameObject* door = sObjectAccessor->FindGameObject(m_ColiseumLFRDoor))
                            door->SetGoState(GO_STATE_ACTIVE);

                        if (Creature* margok = GetCreature(MargokCosmetic))
                            margok->DespawnOrUnsummon();

                        break;
                    }
                    case ImperatorsFall:
                    {
                        uint32 l_DisabledMask = 0;

                        l_DisabledMask |= (1 << BossKargathBladefist);
                        l_DisabledMask |= (1 << BossTheButcher);
                        l_DisabledMask |= (1 << BossBrackenspore);
                        l_DisabledMask |= (1 << BossTectus);
                        l_DisabledMask |= (1 << BossTwinOgron);
                        l_DisabledMask |= (1 << BossKoragh);

                        SetDisabledBosses(l_DisabledMask);
                        break;
                    }
                    default:
                        break;
                }
            }

            switch (m_DungeonID)
            {
                case ArcaneSanctum:
                case ImperatorsFall:
                    l_ChogallNight = true;
                    break;
                default:
                    break;
            }

            if (l_ChogallNight)
                player->CastSpell(player, ChogallNight, true);
            else
            {
                player->RemoveAura(PlayChogallScene);
                player->RemoveAura(ChogallNight);
            }
        }

        void OnPlayerLeave(Player* player) override
        {
            InstanceScript::OnPlayerLeave(player);

            player->RemoveAura(PlayChogallScene);
            player->RemoveAura(ChogallNight);
        }

        void SendUpdateWorldState(uint32 field, uint32 value)
        {
            instance->ApplyOnEveryPlayer([&](Player* player)
            {
                if (!player)
                    return;

                player->SendUpdateWorldState(field, value);
            });
        }

        WorldLocation* GetClosestGraveYard(float x, float y, float z) override
        {
            uint32 graveyardId = 4783;

            bool ok = true;
            for (uint8 i = BossKargathBladefist; i <= BossTectus; ++i)
                if (GetBossState(i) != DONE)
                {
                    ok = false;
                    break;
                }

            if (ok)
                graveyardId = 4785;

            if (WorldSafeLocsEntry const* gy = sWorldSafeLocsStore.LookupEntry(graveyardId))
            {
                loc_res_pla.Relocate(gy->Loc.X, gy->Loc.Y, gy->Loc.Z);
                loc_res_pla.SetMapId(gy->MapID);
            }

            return &loc_res_pla;
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_highmaul_InstanceMapScript(map);
    }
};

void AddSC_instance_highmaul()
{
    new instance_highmaul();
}
