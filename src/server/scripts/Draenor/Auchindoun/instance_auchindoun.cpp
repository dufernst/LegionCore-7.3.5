////////////////////////////////////////////////////////////////////////////////
///
///  MILLENIUM-STUDIO
///  Copyright 2015 Millenium-studio SARL
///  All Rights Reserved.
///
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "auchindoun.hpp"

class instance_auchindoun : public InstanceMapScript
{
public:

    instance_auchindoun() : InstanceMapScript("instance_auchindoun", 1182) { }

    struct instance_auchindoun_InstanceMapScript : InstanceScript
    {
        instance_auchindoun_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            m_KaatharDied = false;
            m_TuulaniSummoned = true;
        }

        InstanceScript* m_Instance = this;
        uint32 m_auiEncounter[4];
        ObjectGuid m_NyamiGuid;
        ObjectGuid m_Tuulani02Guid;
        ObjectGuid m_UniqueGuardGuid;
        ObjectGuid m_TuulaniGuid;
        ObjectGuid m_WardenGuid;
        ObjectGuid m_GromtashGuid;
        ObjectGuid m_DuragGuid;
        ObjectGuid m_GulkoshGuid;
        ObjectGuid m_ElumGuid;
        ObjectGuid m_IruunGuid;
        ObjectGuid m_JoraGuid;
        ObjectGuid m_AssainatingGuardGuid;
        ObjectGuid m_AssainatedGuardGuid;
        ObjectGuid m_KaatharGuid;
        ObjectGuid m_NyamibossGuid;
        ObjectGuid m_AzzakelGuid;
        ObjectGuid m_TeronogorGuid;
        ObjectGuid m_HolyBarrierKathaarObjectGuid;
        ObjectGuid m_CrystalKaatharGuid;
        ObjectGuid m_WindowGuid;
        ObjectGuid m_FelBarrierAzzakelObjectGuid;
        ObjectGuid m_FelPortalGuid;
        ObjectGuid m_SoulTransportStartGuid;
        ObjectGuid m_SoulTransport01Guid;
        ObjectGuid m_SoulTransport02Guid;
        ObjectGuid m_SoulTransport03Guid;
        ObjectGuid m_AuchindounCrystal;
        ObjectGuid m_TriggerBubbleMiddleNyamiGuid;
        ObjectGuid m_TriggerAzzakelFelPortalGuid;
        bool m_KaatharDied;
        bool m_TuulaniSummoned;

        void Initialize() override
        {
            m_KaatharDied = false;

            //instance->SetObjectVisibility(150.0f);

            SetBossNumber(DataMaxBosses);
        }

        void OnGameObjectCreate(GameObject* gameObject) override
        {
            if (gameObject)
            {
                switch (gameObject->GetEntry())
                {
                    case GameObjectAuchindounCrystal:
                        m_AuchindounCrystal = gameObject->GetGUID();
                        break;
                    case GameobjectHolyBarrier:
                        m_HolyBarrierKathaarObjectGuid = gameObject->GetGUID();
                        gameObject->SetLootState(GO_READY);
                        gameObject->UseDoorOrButton(10 * IN_MILLISECONDS, false);
                        break;
                    case GameobjectAuchindounWindow:
                        m_WindowGuid = gameObject->GetGUID();
                        break;
                    case GameobjectFelBarrier:
                        m_FelBarrierAzzakelObjectGuid = gameObject->GetGUID();
                        break;
                    case GameobjectSoulTransportStart:
                        m_SoulTransportStartGuid = gameObject->GetGUID();
                        break;
                    case GameobjectSoulTransport1:
                        if (instance != nullptr)
                        {
                            if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                            {
                                m_SoulTransport01Guid = gameObject->GetGUID();
                            }
                        }
                        break;
                    case GameobjectSoulTransport2:
                        if (instance != nullptr)
                        {
                            if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                            {
                                m_SoulTransport02Guid = gameObject->GetGUID();
                            }
                        }
                        break;
                    case GameobjectSoulTransport3:
                        if (instance != nullptr)
                        {
                            if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                            {
                                m_SoulTransport03Guid = gameObject->GetGUID();
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            if (creature)
            {
                switch (creature->GetEntry())
                {
                    case CreatureSoulBinderTuulani01:
                        m_Tuulani02Guid = creature->GetGUID();
                        break;
                    case CreatureIruun:
                        m_IruunGuid = creature->GetGUID();
                        break;
                    case CreatureJoraa:
                        m_JoraGuid = creature->GetGUID();
                        break;
                    case CreatureDurem:
                        m_ElumGuid = creature->GetGUID();
                        break;
                    case CreatureAuchenaiDefenderUnique:
                        m_UniqueGuardGuid = creature->GetGUID();
                        break;
                    case BossKaathar:
                        m_KaatharGuid = creature->GetGUID();
                        break;
                    case BossNyami:
                        m_NyamibossGuid = creature->GetGUID();
                        creature->SummonCreature(CreatureWardenAzzakael, 1661.218f, 2917.974f, 49.063f, 1.604011f, TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    case BossAzaakel:
                        m_AzzakelGuid = creature->GetGUID();
                        break;
                    case BossTeronogor:
                        m_TeronogorGuid = creature->GetGUID();
                        break;
                    case CreatureSoulBinderTuulani:
                        m_TuulaniGuid = creature->GetGUID();
                        break;
                    case CreatureSoulBinderNyami:
                        m_NyamiGuid = creature->GetGUID();
                        break;
                    case CreatureShieldSpot:
                        m_TriggerBubbleMiddleNyamiGuid = creature->GetGUID();
                        break;
                    case CreatureWardenAzzakael:
                        m_WardenGuid = creature->GetGUID();
                        break;
                    case CreatureDemonsSummoner:
                        m_TriggerAzzakelFelPortalGuid = creature->GetGUID();
                        break;
                    case CreatureGulkosh:
                        m_GulkoshGuid = creature->GetGUID();
                        break;
                    case CreatureGromtashTheDestructor:
                        m_GromtashGuid = creature->GetGUID();
                        break;
                    case CreatureDuragTheDominator:
                        m_DuragGuid = creature->GetGUID();
                        break;
                    case CreatureSargereiAssasinating:
                        m_AssainatingGuardGuid = creature->GetGUID();
                        break;
                    case CreatureAuchenaiAssainated:
                        m_AssainatedGuardGuid = creature->GetGUID();
                        break;
                    default:
                        break;
                }
            }
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (!m_Instance)
                return;

            Creature* creature = unit->ToCreature();
            if (!creature)
                return;

            switch (creature->GetEntry())
            {
                case BossKaathar:
                {
                    if (GameObject* l_Holybarrier = instance->GetGameObject(GetGuidData(DataHolyBarrier)))
                    {
                        l_Holybarrier->Delete();
                        m_KaatharDied = true;
                    }
                    break;
                }
                case BossAzaakel:
                {
                    if (GameObject* l_Felbarrier = instance->GetGameObject(GetGuidData(DataFelBarrier)))
                        l_Felbarrier->Delete();

                    if (instance->GetGameObject(GetGuidData(DataSoulTransportStart)))
                    {
                        if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                        {
                            if (l_Teronogor->IsAIEnabled)
                                l_Teronogor->GetAI()->DoAction(ActionSoulMove1);
                        }
                    }
                    break;
                }
                /// Soul Transport
                case CreatureGromtashTheDestructor:
                    if (instance->GetGameObject(GetGuidData(DataSoulTransport3)))
                    {
                        if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                        {
                            if (l_Teronogor->IsAIEnabled)
                                l_Teronogor->GetAI()->DoAction(ActionSoulMove4);
                        }
                    }
                    break;
                case CreatureGulkosh:
                    if (instance->GetGameObject(GetGuidData(DataSoulTransport2)))
                    {
                        if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                        {
                            if (l_Teronogor->IsAIEnabled)
                                l_Teronogor->GetAI()->DoAction(ActionSoulMove3);
                        }
                    }
                    break;
                case CreatureDuragTheDominator:
                {
                    if (instance != nullptr)
                    {
                        if (instance->GetGameObject(GetGuidData(DataSoulTransport1)))
                        {
                            if (Creature* l_Teronogor = instance->GetCreature(GetGuidData(DataBossTeronogor)))
                            {
                                if (l_Teronogor->IsAIEnabled)
                                    l_Teronogor->GetAI()->DoAction(ActionSoulMove2);
                            }
                        }
                    }
                    break;
                }
                case BossTeronogor:
                {
                    if (creature->GetMap() && creature->GetMap()->IsHeroic())
                        DoCompleteAchievement(AchievementAuchindounHeroic);
                    else
                        DoCompleteAchievement(AchievementAuchindounNormal);

                    /// Curtain flames achievement, No Tags Backs! (9552)
                    UnitList targets;
                    //Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(creature, creature, 100, 153392, true);
                    //Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(creature, targets, u_check);
                    //creature->VisitNearbyObject(100, searcher);

                    if (!targets.empty())
                        DoCompleteAchievement(AchievementNoTagBacks);
                    break;
                }
                default:
                    break;
            }
        }

        ObjectGuid GetGuidData(uint32 p_Data) const override
        {
            switch (p_Data)
            {
                case DataCrystal:
                    return m_AuchindounCrystal;
                case DataTuulani02:
                    return m_Tuulani02Guid;
                case DataGuard:
                    return m_UniqueGuardGuid;
                case DataElum:
                    return m_ElumGuid;
                case DataIruun:
                    return m_IruunGuid;
                case DataJorra:
                    return m_JoraGuid;
                case DataSoulTransportStart:
                    return m_SoulTransportStartGuid;
                case DataSoulTransport1:
                    return m_SoulTransport01Guid;
                case DataSoulTransport2:
                    return m_SoulTransport02Guid;
                case DataSoulTransport3:
                    return m_SoulTransport03Guid;
                case DataHolyBarrier:
                    return m_HolyBarrierKathaarObjectGuid;
                case DataAuchindounWindow:
                    return m_WindowGuid;
                case DataFelBarrier:
                    return m_FelBarrierAzzakelObjectGuid;
                case DataFelPortal:
                    return m_FelPortalGuid;
                case DataBossKathaar:
                    return m_KaatharGuid;
                case DataBossAzzakael:
                    return m_AzzakelGuid;
                case DataBossNyami:
                    return m_NyamibossGuid;
                case DataBossTeronogor:
                    return m_TeronogorGuid;
                case DataNyami:
                    return m_NyamiGuid;
                case DataTuulani:
                    return m_TuulaniGuid;
                case DataWarden:
                    return m_WardenGuid;
                case DataGulkosh:
                    return m_GulkoshGuid;
                case DataGromtash:
                    return m_GromtashGuid;
                case DataDurag:
                    return m_DuragGuid;
                case DataTriggerMiddleNyamiFightBubble:
                    return m_TriggerBubbleMiddleNyamiGuid;
                case DataTriggerAzzakelController:
                    return m_TriggerAzzakelFelPortalGuid;
                case DataAssinatingGuard:
                    return m_AssainatingGuardGuid;
                case DataAssinatedGuard:
                    return m_AssainatedGuardGuid;
                default:
                    break;
            }

            return ObjectGuid::Empty;
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_auchindoun_InstanceMapScript(map);
    }
};

void AddSC_instance_auchindoun()
{
    new instance_auchindoun;
}

