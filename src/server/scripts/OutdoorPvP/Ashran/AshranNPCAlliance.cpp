////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "AshranMgr.hpp"
#include "ScriptedCosmeticAI.h"

/// Rylai Crestfall <Stormshield Tower Guardian> - 88224
class npc_rylai_crestfall : public CreatureScript
{
public:
    npc_rylai_crestfall() : CreatureScript("npc_rylai_crestfall") { }

    struct npc_rylai_crestfallAI : public ScriptedAI
    {
        npc_rylai_crestfallAI(Creature* creature) : ScriptedAI(creature)
        {
            m_CheckAroundingPlayersTimer = 0;
            m_FreezingFieldTimer = 0;
        }

        enum eSpells
        {
            Frostbolt = 176268,
            FrostboltVolley = 176273,
            IceBlock = 176269,
            Hypotermia = 41425,
            MassPolymorph = 176204,
            FrostNovaCasted = 176327,
            NorthrendWinds = 176267,
            FrostNova = 176276,
            DeepFreeze = 176278,
            SummonIceShard = 177599,   ///< @TODO

            TowerMageTargetingAura = 176162,   ///< Put on ennemy players around 200 yards
            FreezingFieldSearcher = 176163,   ///< Launch frost missile on one player targeted
            FreezingFieldMissile = 176165,

            ConjureRefreshment = 176351
        };

        enum eTalk
        {
            TalkAggro,
            TalkSlay,
            TalkDeath,
            TalkSpell
        };

        enum eEvents
        {
            EventFrostbolt = 1,
            EventFrostboltVolley,
            EventMassPolymorph,
            EventFrostNova,
            EventNorthrendWinds
        };

        EventMap m_Events;

        uint32 m_CheckAroundingPlayersTimer;
        uint32 m_FreezingFieldTimer;

        void Reset() override
        {
            m_Events.Reset();

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);

            m_CheckAroundingPlayersTimer = 2000;
            m_FreezingFieldTimer = 10000;
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            Talk(TalkAggro);

            m_Events.ScheduleEvent(EventFrostbolt, 4000);
            m_Events.ScheduleEvent(EventMassPolymorph, 6000);
            m_Events.ScheduleEvent(EventFrostboltVolley, 9000);
            m_Events.ScheduleEvent(EventFrostNova, 12500);
            m_Events.ScheduleEvent(EventNorthrendWinds, 15000);
        }

        void KilledUnit(Unit* p_Who) override
        {
            if (p_Who->GetTypeId() == TYPEID_PLAYER)
                Talk(TalkSlay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(TalkDeath);
        }

        void DamageTaken(Unit* /*p_Attacker*/, uint32& p_Damage, DamageEffectType dmgType) override
        {
            if (me->HealthBelowPctDamaged(20, p_Damage) && !me->HasAura(Hypotermia))
            {
                me->CastSpell(me, IceBlock, true);
                me->CastSpell(me, Hypotermia, true);
            }
        }

        void SpellHitTarget(Unit* p_Victim, SpellInfo const* p_SpellInfo) override
        {
            if (p_Victim == nullptr)
                return;

            switch (p_SpellInfo->Id)
            {
                case FreezingFieldSearcher:
                    me->CastSpell(p_Victim, FreezingFieldMissile, false);
                    break;
                case NorthrendWinds:
                    if (p_Victim->HasAura(FrostNova))
                        me->CastSpell(p_Victim, DeepFreeze, true);
                    else
                        me->CastSpell(p_Victim, FrostNova, true);
                    break;
                default:
                    break;
            }
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
            {
                ScheduleTargetingPlayers(p_Diff);
                ScheduleFreezingField(p_Diff);
                return;
            }

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventFrostbolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM))
                        me->CastSpell(l_Target, Frostbolt, false);
                    m_Events.ScheduleEvent(EventFrostbolt, 10000);
                    break;
                case EventFrostboltVolley:
                    Talk(TalkSpell);
                    me->CastSpell(me, FrostboltVolley, false);
                    m_Events.ScheduleEvent(EventFrostboltVolley, 20000);
                    break;
                case EventMassPolymorph:
                    me->CastSpell(me, MassPolymorph, false);
                    m_Events.ScheduleEvent(EventMassPolymorph, 25000);
                    break;
                case EventFrostNova:
                    me->CastSpell(me, FrostNovaCasted, false);
                    m_Events.ScheduleEvent(EventFrostNova, 27500);
                    break;
                case EventNorthrendWinds:
                    Talk(TalkSpell);
                    me->CastSpell(me, NorthrendWinds, false);
                    m_Events.ScheduleEvent(EventNorthrendWinds, 30000);
                    break;
                default:
                    break;
            }

            EnterEvadeIfOutOfCombatArea(p_Diff);
            DoMeleeAttackIfReady();
        }

        void ScheduleTargetingPlayers(uint32 const p_Diff)
        {
            if (!m_CheckAroundingPlayersTimer)
                return;

            if (m_CheckAroundingPlayersTimer <= p_Diff)
            {
                m_CheckAroundingPlayersTimer = 2500;

                std::list<Player*> l_PlayerList;
                me->GetPlayerListInGrid(l_PlayerList, 200.0f);

                l_PlayerList.remove_if([this](Player* player) -> bool
                {
                    if (player == nullptr)
                        return true;

                    if (!me->IsValidAttackTarget(player))
                        return true;

                    if (player->HasAura(TowerMageTargetingAura))
                        return true;

                    return false;
                });

                for (Player* l_Player : l_PlayerList)
                    l_Player->CastSpell(l_Player, TowerMageTargetingAura, true, nullptr, nullptr, me->GetGUID());
            }
            else
                m_CheckAroundingPlayersTimer -= p_Diff;
        }

        void ScheduleFreezingField(uint32 const p_Diff)
        {
            if (!m_FreezingFieldTimer)
                return;

            if (m_FreezingFieldTimer <= p_Diff)
            {
                if (!me->isInCombat())
                    me->CastSpell(me, FreezingFieldSearcher, true);
                m_FreezingFieldTimer = 10000;
            }
            else
                m_FreezingFieldTimer -= p_Diff;
        }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 /*p_Action*/) override
        {
            player->PlayerTalkClass->SendCloseGossip();
            me->CastSpell(player, ConjureRefreshment, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_rylai_crestfallAI(creature);
    }
};

/// Grimnir Sternhammer <Explorer's League> - 88679
class npc_ashran_grimnir_sternhammer : public CreatureScript
{
public:
    npc_ashran_grimnir_sternhammer() : CreatureScript("npc_ashran_grimnir_sternhammer") { }

    enum eTalks
    {
        First,
        Second,
        Third,
        Fourth
    };

    enum eData
    {
        MisirinStouttoe = 88682,
        ActionInit = 0,
        ActionLoop = 1,
        EventLoop = 1
    };

    struct npc_ashran_grimnir_sternhammerAI : public MS::AI::CosmeticAI
    {
        npc_ashran_grimnir_sternhammerAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;
        EventMap m_Events;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(MisirinStouttoe, 15.0f))
            {
                if (l_Creature->AI())
                {
                    m_Init = true;
                    l_Creature->AI()->DoAction(ActionInit);
                    ScheduleAllTalks();
                }
            }
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case ActionInit:
                    if (m_Init)
                        break;
                    m_Init = true;
                    ScheduleAllTalks();
                    break;
                default:
                    break;
            }
        }

        void ScheduleAllTalks()
        {
            AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(First); });
            AddDelayedEvent(13 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(42 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
            AddDelayedEvent(54 * IN_MILLISECONDS, [this]() -> void { Talk(Fourth); });
        }

        void LastOperationCalled() override
        {
            m_Events.ScheduleEvent(EventLoop, 33 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            CosmeticAI::UpdateAI(p_Diff);

            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == EventLoop)
            {
                if (Creature* l_Creature = me->FindNearestCreature(MisirinStouttoe, 15.0f))
                {
                    if (l_Creature->AI())
                    {
                        l_Creature->AI()->DoAction(ActionLoop);
                        ScheduleAllTalks();
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_grimnir_sternhammerAI(creature);
    }
};

/// Misirin Stouttoe <Explorer's League> - 88682
class npc_ashran_misirin_stouttoe : public CreatureScript
{
public:
    npc_ashran_misirin_stouttoe() : CreatureScript("npc_ashran_misirin_stouttoe") { }

    enum eTalks
    {
        First,
        Second,
        Third
    };

    enum eData
    {
        GrimnirSternhammer = 88679,
        ActionInit = 0,
        ActionLoop = 1
    };

    struct npc_ashran_misirin_stouttoeAI : public MS::AI::CosmeticAI
    {
        npc_ashran_misirin_stouttoeAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(GrimnirSternhammer, 15.0f))
            {
                if (l_Creature->AI())
                {
                    m_Init = true;
                    l_Creature->AI()->DoAction(ActionInit);
                    ScheduleAllTalks();
                }
            }
        }

        void DoAction(int32 const p_Action) override
        {
            switch (p_Action)
            {
                case ActionInit:
                    m_Init = true;
                    ScheduleAllTalks();
                    break;
                case ActionLoop:
                    ScheduleAllTalks();
                    break;
                default:
                    break;
            }
        }

        void ScheduleAllTalks()
        {
            AddDelayedEvent(7 * IN_MILLISECONDS, [this]() -> void { Talk(First); });
            AddDelayedEvent(48 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(60 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_misirin_stouttoeAI(creature);
    }
};

/// Stormshield Druid - 81887
class npc_ashran_stormshield_druid : public CreatureScript
{
public:
    npc_ashran_stormshield_druid() : CreatureScript("npc_ashran_stormshield_druid") { }

    struct npc_ashran_stormshield_druidAI : public ScriptedAI
    {
        npc_ashran_stormshield_druidAI(Creature* creature) : ScriptedAI(creature) { }

        enum eDatas
        {
            EventCosmetic = 1,
            AncientOfWar = 81883,
            NatureChanneling = 164850
        };

        EventMap m_Events;

        void Reset() override
        {
            m_Events.ScheduleEvent(EventCosmetic, 5000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == EventCosmetic)
            {
                if (Creature* l_EarthFury = me->FindNearestCreature(AncientOfWar, 20.0f))
                    me->CastSpell(l_EarthFury, NatureChanneling, false);
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_stormshield_druidAI(creature);
    }
};

/// Officer Rumsfeld - 88696
class npc_ashran_officer_rumsfeld : public CreatureScript
{
public:
    npc_ashran_officer_rumsfeld() : CreatureScript("npc_ashran_officer_rumsfeld") { }

    struct npc_ashran_officer_rumsfeldAI : public MS::AI::CosmeticAI
    {
        npc_ashran_officer_rumsfeldAI(Creature* creature) : CosmeticAI(creature) { }

        void Reset() override
        {
            AddDelayedEvent(20 * IN_MILLISECONDS, [this]() -> void { Talk(0); });
        }

        void LastOperationCalled() override
        {
            Reset();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_officer_rumsfeldAI(creature);
    }
};

/// Officer Ironore - 88697
class npc_ashran_officer_ironore : public CreatureScript
{
public:
    npc_ashran_officer_ironore() : CreatureScript("npc_ashran_officer_ironore") { }

    struct npc_ashran_officer_ironoreAI : public MS::AI::CosmeticAI
    {
        npc_ashran_officer_ironoreAI(Creature* creature) : CosmeticAI(creature) { }

        void Reset() override
        {
            AddDelayedEvent(30 * IN_MILLISECONDS, [this]() -> void { Talk(0); });
        }

        void LastOperationCalled() override
        {
            Reset();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_officer_ironoreAI(creature);
    }
};

/// Marketa <Stormshield Warlock Leader> - 82660
class npc_ashran_marketa : public CreatureScript
{
public:
    npc_ashran_marketa() : CreatureScript("npc_ashran_marketa") { }

    enum eGossip
    {
        GatewaysInvoked = 85463
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
        if (l_ZoneScript == nullptr)
            return false;

        if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
        {
            if (l_Ashran->IsArtifactEventLaunched(TEAM_ALLIANCE, CountForWarlock))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::GatewaysInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_marketaAI : public ScriptedAI
    {
        npc_ashran_marketaAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 p_Action) override
        {
            /// "Take all of my Artifact Fragments" is always 0
            if (p_Action)
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            uint32 artifactCount = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT);
            player->ModifyCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT, -int32(artifactCount * CURRENCY_PRECISION), false);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
            {
                l_Ashran->AddCollectedArtifacts(TEAM_ALLIANCE, CountForWarlock, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_marketaAI(creature);
    }
};

/// Ecilam <Stormshield Mage Leader> - 82966
class npc_ashran_ecilam : public CreatureScript
{
public:
    npc_ashran_ecilam() : CreatureScript("npc_ashran_ecilam") { }

    enum eGossip
    {
        PortalInvoked = 84919
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
        if (l_ZoneScript == nullptr)
            return false;

        if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
        {
            if (l_Ashran->IsArtifactEventLaunched(TEAM_ALLIANCE, CountForMage))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::PortalInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_ecilamAI : public ScriptedAI
    {
        npc_ashran_ecilamAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 p_Action) override
        {
            /// "Take all of my Artifact Fragments" is always 0
            if (p_Action)
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            uint32 artifactCount = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT);
            player->ModifyCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT, -int32(artifactCount * CURRENCY_PRECISION), false);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
            {
                l_Ashran->AddCollectedArtifacts(TEAM_ALLIANCE, CountForMage, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_ecilamAI(creature);
    }
};

/// Valant Brightsworn <Stormshield Paladin Leader> - 82893
class npc_ashran_valant_brightsworn : public CreatureScript
{
public:
    npc_ashran_valant_brightsworn() : CreatureScript("npc_ashran_valant_brightsworn") { }

    enum eGossip
    {
        RidersSummoned = 84923
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
        if (l_ZoneScript == nullptr)
            return false;

        if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
        {
            if (l_Ashran->IsArtifactEventLaunched(TEAM_ALLIANCE, CountForWarriorPaladin))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::RidersSummoned, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_valant_brightswornAI : public ScriptedAI
    {
        npc_ashran_valant_brightswornAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 p_Action) override
        {
            /// "Take all of my Artifact Fragments" is always 0
            if (p_Action)
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            uint32 artifactCount = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT);
            player->ModifyCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT, -int32(artifactCount * CURRENCY_PRECISION), false);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
            {
                l_Ashran->AddCollectedArtifacts(TEAM_ALLIANCE, CountForWarriorPaladin, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_valant_brightswornAI(creature);
    }
};

/// Anenga <Stormshield Druid Leader> - 81870
class npc_ashran_anenga : public CreatureScript
{
public:
    npc_ashran_anenga() : CreatureScript("npc_ashran_anenga") { }

    enum eGossip
    {
        FangraalInvoked = 83895
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
        if (l_ZoneScript == nullptr)
            return false;

        if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
        {
            if (l_Ashran->IsArtifactEventLaunched(TEAM_ALLIANCE, CountForDruidShaman))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::FangraalInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_anengaAI : public ScriptedAI
    {
        npc_ashran_anengaAI(Creature* creature) : ScriptedAI(creature) { }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 p_Action) override
        {
            /// "Take all of my Artifact Fragments" is always 0
            if (p_Action)
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            uint32 artifactCount = player->GetCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT);
            player->ModifyCurrency(CURRENCY_TYPE_ARTIFACT_FRAGMENT, -int32(artifactCount * CURRENCY_PRECISION), false);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
            {
                l_Ashran->AddCollectedArtifacts(TEAM_ALLIANCE, CountForDruidShaman, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_anengaAI(creature);
    }
};

/// Kauper <Portal Guardian> - 84466
class npc_ashran_kauper : public CreatureScript
{
public:
    npc_ashran_kauper() : CreatureScript("npc_ashran_kauper") { }

    enum eSpells
    {
        SpellMoltenArmor = 79849,
        SpellFlamestrike = 79856,
        SpellFireball = 79854,
        SpellBlastWave = 79857
    };

    enum eEvents
    {
        EventFlamestrike = 1,
        EventFireball,
        EventBlastWave
    };

    struct npc_ashran_kauperAI : public ScriptedAI
    {
        npc_ashran_kauperAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        void Reset() override
        {
            me->CastSpell(me, SpellMoltenArmor, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->CastSpell(me, SpellMoltenArmor, true);

            m_Events.ScheduleEvent(EventFlamestrike, 7000);
            m_Events.ScheduleEvent(EventFireball, 3000);
            m_Events.ScheduleEvent(EventBlastWave, 9000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
                l_Ashran->EndArtifactEvent(TEAM_ALLIANCE, CountForMage);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventFlamestrike:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM))
                        me->CastSpell(l_Target, SpellFlamestrike, false);
                    m_Events.ScheduleEvent(EventFlamestrike, 15000);
                    break;
                case EventFireball:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellFireball, false);
                    m_Events.ScheduleEvent(EventFireball, 10000);
                    break;
                case EventBlastWave:
                    me->CastSpell(me, SpellBlastWave, false);
                    m_Events.ScheduleEvent(EventBlastWave, 20000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_kauperAI(creature);
    }
};

/// Decker Watts <Gateway Guardian> - 84651
/// Falcon Atherton <Gateway Guardian> - 84652
class npc_ashran_alliance_gateway_guardian : public CreatureScript
{
public:
    npc_ashran_alliance_gateway_guardian() : CreatureScript("npc_ashran_alliance_gateway_guardian") { }

    enum eSpells
    {
        SpellCurseOfTheElements = 79956,
        SpellRainOfFire = 165757,
        SpellShadowBolt = 79932
    };

    enum eEvents
    {
        EventShadowBolt = 1,
        EventRainOfFire
    };

    struct npc_ashran_alliance_gateway_guardianAI : public ScriptedAI
    {
        npc_ashran_alliance_gateway_guardianAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        void Reset() override
        {
            m_Events.Reset();
        }

        void EnterCombat(Unit* p_Attacker) override
        {
            me->CastSpell(p_Attacker, SpellCurseOfTheElements, true);

            m_Events.ScheduleEvent(EventShadowBolt, 1000);
            m_Events.ScheduleEvent(EventRainOfFire, 6000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
                l_Ashran->EndArtifactEvent(TEAM_ALLIANCE, CountForWarlock);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventShadowBolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellShadowBolt, false);
                    m_Events.ScheduleEvent(EventShadowBolt, 4000);
                    break;
                case EventRainOfFire:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM))
                        me->CastSpell(l_Target, SpellRainOfFire, false);
                    m_Events.ScheduleEvent(EventRainOfFire, 20000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_alliance_gateway_guardianAI(creature);
    }
};

/// Fangraal <Alliance Guardian> - 81859
class npc_ashran_fangraal : public CreatureScript
{
public:
    npc_ashran_fangraal() : CreatureScript("npc_ashran_fangraal") { }

    enum eSpells
    {
        AshranLaneMobScalingAura = 164310,

        SpellWildGrowth = 168247,
        SpellEntanglingRootSearcher = 168248,
        SpellEntanglingRootMissile = 177607    ///< Trigger 177606
    };

    enum eEvents
    {
        EventWildGrowth = 1,
        EventEntanglingRoots,
        EventAwake1,
        EventAwake2,
        EventMove
    };

    enum eTalks
    {
        TalkAwake1,
        TalkAwake2
    };

    struct npc_ashran_fangraalAI : public ScriptedAI
    {
        npc_ashran_fangraalAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_TalkEvents;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1000);

            /// Fangraal no longer scales their health based the number of players he's fighting.
            /// Each faction guardian's health now scales based on the number of enemy players active at the time when they're summoned.
            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            uint32 l_PlayerCount = 0;
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
                l_PlayerCount = l_Ashran->CountPlayersForTeam(TEAM_ALLIANCE);

            if (Aura* l_Scaling = me->AddAura(AshranLaneMobScalingAura, me))
            {
                if (AuraEffect* l_Damage = l_Scaling->GetEffect(EFFECT_0))
                    l_Damage->ChangeAmount(HealthPCTAddedByHostileRef * l_PlayerCount);
                if (AuraEffect* l_Health = l_Scaling->GetEffect(EFFECT_1))
                    l_Health->ChangeAmount(HealthPCTAddedByHostileRef * l_PlayerCount);
            }

            m_TalkEvents.ScheduleEvent(EventAwake1, 1000);
            m_TalkEvents.ScheduleEvent(EventAwake2, 5000);
        }

        void Reset() override
        {
            me->setRegeneratingHealth(false);

            me->SetReactState(REACT_AGGRESSIVE);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            Position l_Pos;
            me->GetPosition(&l_Pos);
            me->SetHomePosition(l_Pos);

            m_Events.ScheduleEvent(EventWildGrowth, 5000);
            m_Events.ScheduleEvent(EventEntanglingRoots, 10000);
        }

        void DamageTaken(Unit* p_Attacker, uint32& p_Damage, DamageEffectType dmgType) override
        {
            if (p_Damage < me->GetHealth())
                return;

            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
            {
                if (l_Ashran->IsArtifactEventLaunched(TEAM_ALLIANCE, CountForDruidShaman))
                {
                    l_Ashran->CastSpellOnTeam(me, TEAM_HORDE, SpellEventAllianceReward);
                    l_Ashran->EndArtifactEvent(TEAM_ALLIANCE, CountForDruidShaman);
                }
            }
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            if (p_SpellInfo->Id == SpellEntanglingRootSearcher)
                me->CastSpell(p_Target, SpellEntanglingRootMissile, true);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                me->SetWalk(true);
                me->LoadPath(me->GetEntry());
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            m_TalkEvents.Update(p_Diff);

            switch (m_TalkEvents.ExecuteEvent())
            {
                case EventAwake1:
                    Talk(TalkAwake1);
                    break;
                case EventAwake2:
                    Talk(TalkAwake2);
                    break;
                default:
                    break;
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventWildGrowth:
                    me->CastSpell(me, SpellWildGrowth, true);
                    m_Events.ScheduleEvent(EventWildGrowth, 38000);
                    break;
                case EventEntanglingRoots:
                    me->CastSpell(me, SpellEntanglingRootSearcher, true);
                    m_Events.ScheduleEvent(EventEntanglingRoots, 37000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_fangraalAI(creature);
    }
};

/// Lifeless Ancient <Alliance Guardian> - 81883
class npc_ashran_lifeless_ancient : public CreatureScript
{
public:
    npc_ashran_lifeless_ancient() : CreatureScript("npc_ashran_lifeless_ancient") { }

    struct npc_ashran_lifeless_ancientAI : public ScriptedAI
    {
        npc_ashran_lifeless_ancientAI(Creature* creature) : ScriptedAI(creature) { }

        enum eData
        {
            StormshieldDruid = 81887
        };

        EventMap m_Events;

        void Reset() override
        {
            std::list<Creature*> l_StormshieldDruids;
            me->GetCreatureListWithEntryInGrid(l_StormshieldDruids, StormshieldDruid, 20.0f);

            for (Creature* l_Creature : l_StormshieldDruids)
            {
                if (l_Creature->AI())
                    l_Creature->AI()->Reset();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_lifeless_ancientAI(creature);
    }
};

/// Stormshield Stormcrow - 82895
class npc_ashran_stormshield_stormcrow : public CreatureScript
{
public:
    npc_ashran_stormshield_stormcrow() : CreatureScript("npc_ashran_stormshield_stormcrow") { }

    enum eEvent
    {
        MoveCircle = 1
    };

    struct npc_ashran_stormshield_stormcrowAI : public ScriptedAI
    {
        npc_ashran_stormshield_stormcrowAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        void Reset() override
        {
            m_Events.ScheduleEvent(MoveCircle, 2000);
        }

        void FillCirclePath(Position const& p_Center, float p_Radius, float p_Z, Movement::PointsArray& p_Path, bool p_Clockwise)
        {
            float l_Step = p_Clockwise ? -M_PI / 8.0f : M_PI / 8.0f;
            float l_Angle = p_Center.GetAngle(me->GetPositionX(), me->GetPositionY());

            for (uint8 l_Iter = 0; l_Iter < 16; l_Angle += l_Step, ++l_Iter)
            {
                G3D::Vector3 l_Point;
                l_Point.x = p_Center.GetPositionX() + p_Radius * cosf(l_Angle);
                l_Point.y = p_Center.GetPositionY() + p_Radius * sinf(l_Angle);
                l_Point.z = p_Z;
                p_Path.push_back(l_Point);
            }
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == MoveCircle)
            {
                if (Creature* l_Creature = me->FindNearestCreature(LifelessAncient, 20.0f))
                {
                    Position l_Pos;
                    l_Pos.m_positionX = l_Creature->GetPositionX();
                    l_Pos.m_positionY = l_Creature->GetPositionY();
                    l_Pos.m_positionZ = l_Creature->GetPositionZ();
                    l_Pos.m_orientation = l_Creature->GetOrientation();

                    /// Creating the circle path from the center
                    Movement::MoveSplineInit l_Init(*me);
                    FillCirclePath(l_Pos, 10.0f, me->GetPositionZ(), l_Init.Path(), true);
                    l_Init.SetWalk(true);
                    l_Init.SetCyclic();
                    l_Init.Launch();
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_stormshield_stormcrowAI(creature);
    }
};

/// Stormshield Gladiator - 85812
class npc_ashran_stormshield_gladiator : public CreatureScript
{
public:
    npc_ashran_stormshield_gladiator() : CreatureScript("npc_ashran_stormshield_gladiator") { }

    enum eSpells
    {
        SpellCleave = 119419,
        SpellDevotionAura = 165712,
        SpellMortalStrike = 19643,
        SpellNet = 81210,
        SpellSnapKick = 15618
    };

    enum eEvents
    {
        EventCleave = 1,
        EventMortalStrike,
        EventNet,
        EventSnapKick
    };

    struct npc_ashran_stormshield_gladiatorAI : public ScriptedAI
    {
        npc_ashran_stormshield_gladiatorAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        void Reset() override
        {
            m_Events.Reset();

            me->CastSpell(me, SpellDevotionAura, true);
        }

        void EnterCombat(Unit* p_Attacker) override
        {
            me->CastSpell(me, SpellDevotionAura, true);
            me->CastSpell(p_Attacker, SpellNet, true);

            m_Events.ScheduleEvent(EventCleave, 3000);
            m_Events.ScheduleEvent(EventMortalStrike, 5000);
            m_Events.ScheduleEvent(EventNet, 8000);
            m_Events.ScheduleEvent(EventSnapKick, 9000);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventCleave:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellCleave, true);
                    m_Events.ScheduleEvent(EventCleave, 15000);
                    break;
                case EventMortalStrike:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellMortalStrike, true);
                    m_Events.ScheduleEvent(EventMortalStrike, 10000);
                    break;
                case EventNet:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellNet, true);
                    m_Events.ScheduleEvent(EventNet, 20000);
                    break;
                case EventSnapKick:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellSnapKick, true);
                    m_Events.ScheduleEvent(EventSnapKick, 20000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_stormshield_gladiatorAI(creature);
    }
};

/// Wrynn's Vanguard Battle Standard - 85382
class npc_ashran_wrynns_vanguard_battle_standard : public CreatureScript
{
public:
    npc_ashran_wrynns_vanguard_battle_standard() : CreatureScript("npc_ashran_wrynns_vanguard_battle_standard") { }

    struct npc_ashran_wrynns_vanguard_battle_standardAI : public ScriptedAI
    {
        npc_ashran_wrynns_vanguard_battle_standardAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_wrynns_vanguard_battle_standardAI(creature);
    }
};

/// Stormshield Sentinel - 86767
class npc_ashran_stormshield_sentinel : public CreatureScript
{
public:
    npc_ashran_stormshield_sentinel() : CreatureScript("npc_ashran_stormshield_sentinel") { }

    enum eSpells
    {
        SpellShoot = 163921,
        SpellKillShot = 173642,
        SpellHeadhuntersMark = 177203,
        SpellConcussiveShot = 17174
    };

    enum eEvents
    {
        SearchTarget = 1,
        EventShoot,
        EventKillShot,
        EventHeadhuntersMark,
        EventConcussiveShot,
        EventClearEvade
    };

    struct npc_ashran_stormshield_sentinelAI : public ScriptedAI
    {
        npc_ashran_stormshield_sentinelAI(Creature* creature) : ScriptedAI(creature)
        {
            m_CosmeticEvent.Reset();
        }

        EventMap m_Events;
        EventMap m_CosmeticEvent;

        void Reset() override
        {
            m_Events.Reset();

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->AddUnitState(UNIT_STATE_ROOT);

            m_CosmeticEvent.ScheduleEvent(SearchTarget, 1 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            m_Events.ScheduleEvent(EventShoot, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventKillShot, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHeadhuntersMark, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventConcussiveShot, 10 * IN_MILLISECONDS);
        }

        void EnterEvadeMode() override
        {
            me->ClearUnitState(UNIT_STATE_ROOT);

            CreatureAI::EnterEvadeMode();

            m_CosmeticEvent.ScheduleEvent(EventClearEvade, 500);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_CosmeticEvent.Update(p_Diff);

            switch (m_CosmeticEvent.ExecuteEvent())
            {
                case SearchTarget:
                {
                    if (Player* l_Player = me->FindNearestPlayer(40.0f))
                        AttackStart(l_Player);
                    else
                        m_CosmeticEvent.ScheduleEvent(SearchTarget, 1 * IN_MILLISECONDS);

                    break;
                }
                case EventClearEvade:
                {
                    me->ClearUnitState(UNIT_STATE_EVADE);
                    break;
                }
                default:
                    break;
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventShoot:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellShoot, false);
                    m_Events.ScheduleEvent(EventShoot, 5 * IN_MILLISECONDS);
                    break;
                }
                case EventKillShot:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    {
                        if (l_Target->HealthBelowPct(20))
                        {
                            me->CastSpell(l_Target, SpellKillShot, false);
                            m_Events.ScheduleEvent(EventKillShot, 10 * IN_MILLISECONDS);
                            break;
                        }
                    }

                    m_Events.ScheduleEvent(EventKillShot, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventHeadhuntersMark:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellHeadhuntersMark, false);
                    m_Events.ScheduleEvent(EventHeadhuntersMark, 18 * IN_MILLISECONDS);
                    break;
                }
                case EventConcussiveShot:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellConcussiveShot, false);
                    m_Events.ScheduleEvent(EventConcussiveShot, 10 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_stormshield_sentinelAI(creature);
    }
};

/// Avenger Turley <Alliance Captain> - 80499
class npc_ashran_avenger_turley : public CreatureScript
{
public:
    npc_ashran_avenger_turley() : CreatureScript("npc_ashran_avenger_turley") { }

    enum eSpells
    {
        AvengersShield = 162638,
        Consecration = 162642,
        DivineShield = 164410,
        DivineStorm = 162641,
        HammerOfJustice = 162764
    };

    enum eEvents
    {
        EventAvengerShield = 1,
        EventConsecration,
        EventDivineShield,
        EventDivineStorm,
        EventHammerOfJustice,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Death
    };

    enum eData
    {
        MountID = 14584
    };

    struct npc_ashran_avenger_turleyAI : public ScriptedAI
    {
        npc_ashran_avenger_turleyAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            /// Second equip is a shield
            me->SetCanDualWield(false);

            me->Mount(MountID);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventAvengerShield, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventConsecration, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDivineShield, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDivineStorm, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHammerOfJustice, 7 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainAvengerTurley);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventAvengerShield:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, AvengersShield, false);
                    m_Events.ScheduleEvent(EventAvengerShield, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventConsecration:
                {
                    me->CastSpell(me, Consecration, false);
                    m_Events.ScheduleEvent(EventConsecration, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventDivineShield:
                {
                    if (me->HealthBelowPct(50))
                        me->CastSpell(me, DivineShield, true);
                    else
                        m_Events.ScheduleEvent(EventDivineShield, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventDivineStorm:
                {
                    me->CastSpell(me, DivineStorm, false);
                    m_Events.ScheduleEvent(EventDivineStorm, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventHammerOfJustice:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, HammerOfJustice, false);
                    m_Events.ScheduleEvent(EventHammerOfJustice, 15 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_avenger_turleyAI(creature);
    }
};

/// Jackson Bajheera <Alliance Captain> - 80484
class npc_ashran_jackson_bajheera : public CreatureScript
{
public:
    npc_ashran_jackson_bajheera() : CreatureScript("npc_ashran_jackson_bajheera") { }

    enum eSpells
    {
        Bladestorm = 164091
    };

    enum eEvents
    {
        EventBladestorm = 1,
        EventMove
    };

    enum eTalks
    {
        Spawn,
        Death
    };

    enum eData
    {
        MountID = 38668
    };

    struct npc_ashran_jackson_bajheeraAI : public ScriptedAI
    {
        npc_ashran_jackson_bajheeraAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Spawn = false;
        }

        EventMap m_Events;
        EventMap m_MoveEvent;
        bool m_Spawn;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            if (!m_Spawn)
            {
                Talk(Spawn);
                m_Spawn = true;
            }

            me->Mount(MountID);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventBladestorm, 5 * IN_MILLISECONDS);
        }

        void EnterEvadeMode() override
        {
            me->InterruptNonMeleeSpells(true);

            CreatureAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainJacksonBajheera);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            /// Update position during Bladestorm
            if (me->HasAura(Bladestorm))
            {
                if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->GetMotionMaster()->MovePoint(0, *l_Target);
                    return;
                }
            }

            /// Update target movements here to avoid some movements problems
            if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
            {
                if (!me->IsWithinMeleeRange(l_Target))
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveChase(l_Target);
                }
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventBladestorm:
                    me->CastSpell(me, Bladestorm, false);
                    m_Events.ScheduleEvent(EventBladestorm, 15 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_jackson_bajheeraAI(creature);
    }
};

/// John Swifty <Alliance Captain> - 79902
class npc_ashran_john_swifty : public CreatureScript
{
public:
    npc_ashran_john_swifty() : CreatureScript("npc_ashran_john_swifty") { }

    enum eSpells
    {
        Bladestorm = 164091
    };

    enum eEvents
    {
        EventBladestorm = 1,
        EventMove
    };

    enum eTalks
    {
        Spawn,
        Death
    };

    enum eData
    {
        MountID = 38668
    };

    struct npc_ashran_john_swiftyAI : public ScriptedAI
    {
        npc_ashran_john_swiftyAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Spawn = false;
        }

        EventMap m_Events;
        EventMap m_MoveEvent;
        bool m_Spawn;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            if (!m_Spawn)
            {
                Talk(Spawn);
                m_Spawn = true;
            }

            me->Mount(MountID);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventBladestorm, 5 * IN_MILLISECONDS);
        }

        void EnterEvadeMode() override
        {
            me->InterruptNonMeleeSpells(true);

            CreatureAI::EnterEvadeMode();
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainJohnSwifty);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            /// Update position during Bladestorm
            if (me->HasAura(Bladestorm))
            {
                if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->GetMotionMaster()->MovePoint(0, *l_Target);
                    return;
                }
            }

            /// Update target movements here to avoid some movements problems
            if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
            {
                if (!me->IsWithinMeleeRange(l_Target))
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveChase(l_Target);
                }
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventBladestorm:
                    me->CastSpell(me, Bladestorm, false);
                    m_Events.ScheduleEvent(EventBladestorm, 15 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_john_swiftyAI(creature);
    }
};

/// Tosan Galaxyfist <Alliance Captain> - 80494
class npc_ashran_tosan_galaxyfist : public CreatureScript
{
public:
    npc_ashran_tosan_galaxyfist() : CreatureScript("npc_ashran_tosan_galaxyfist") { }

    enum eSpells
    {
        BlackoutKick = 164394,
        LegSweep = 164392,
        RisingSunKick = 127734,
        SpinningCraneKick = 162759
    };

    enum eEvents
    {
        EventBlackoutKick = 1,
        EventLegSweep,
        EventRisingSunKick,
        EventSpinningCraneKick,
        EventMove
    };

    struct npc_ashran_tosan_galaxyfistAI : public ScriptedAI
    {
        npc_ashran_tosan_galaxyfistAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventBlackoutKick, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventLegSweep, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventRisingSunKick, 10 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventSpinningCraneKick, 12 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainTosanGalaxyfist);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventBlackoutKick:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, BlackoutKick, false);
                    m_Events.ScheduleEvent(EventBlackoutKick, 10 * IN_MILLISECONDS);
                    break;
                case EventLegSweep:
                    me->CastSpell(me, LegSweep, false);
                    m_Events.ScheduleEvent(EventLegSweep, 25 * IN_MILLISECONDS);
                    break;
                case EventRisingSunKick:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, RisingSunKick, false);
                    m_Events.ScheduleEvent(EventRisingSunKick, 15 * IN_MILLISECONDS);
                    break;
                case EventSpinningCraneKick:
                    me->CastSpell(me, SpinningCraneKick, false);
                    m_Events.ScheduleEvent(EventSpinningCraneKick, 20 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_tosan_galaxyfistAI(creature);
    }
};

/// Brock the Crazed <Alliance Captain> - 80498
class npc_ashran_brock_the_crazed : public CreatureScript
{
public:
    npc_ashran_brock_the_crazed() : CreatureScript("npc_ashran_brock_the_crazed") { }

    enum eSpells
    {
        DevouringPlague = 164452,
        Dispersion = 164444,
        MindBlast = 164448,
        MindSear = 177402,
        PsychicScream = 164443,
        ShadowWordPain = 164446
    };

    enum eEvents
    {
        EventDevouringPlague = 1,
        EventDispersion,
        EventMindBlast,
        EventMindSear,
        EventPsychicScream,
        EventShadowWordPain,
        EventMove
    };

    enum eTalks
    {
        Death
    };

    enum eData
    {
        MountID = 28918
    };

    struct npc_ashran_brock_the_crazedAI : public ScriptedAI
    {
        npc_ashran_brock_the_crazedAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            me->Mount(MountID);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventDevouringPlague, 10 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDispersion, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventMindBlast, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventMindSear, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventPsychicScream, 15 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventShadowWordPain, 1 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainBrockTheCrazed);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventDevouringPlague:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, DevouringPlague, false);
                    m_Events.ScheduleEvent(EventDevouringPlague, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventDispersion:
                {
                    if (me->HealthBelowPct(50))
                        me->CastSpell(me, Dispersion, true);
                    else
                        m_Events.ScheduleEvent(EventDispersion, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventMindBlast:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, MindBlast, false);
                    m_Events.ScheduleEvent(EventMindBlast, 10 * IN_MILLISECONDS);
                    break;
                }
                case EventMindSear:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, MindSear, false);
                    m_Events.ScheduleEvent(EventMindSear, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventPsychicScream:
                {
                    me->CastSpell(me, PsychicScream, false);
                    m_Events.ScheduleEvent(EventPsychicScream, 30 * IN_MILLISECONDS);
                    break;
                }
                case EventShadowWordPain:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, ShadowWordPain, false);
                    m_Events.ScheduleEvent(EventShadowWordPain, 12 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_brock_the_crazedAI(creature);
    }
};

/// Alune Windmane <Alliance Captain> - 80488
class npc_ashran_alune_windmane : public CreatureScript
{
public:
    npc_ashran_alune_windmane() : CreatureScript("npc_ashran_alune_windmane") { }

    enum eSpells
    {
        Moonfire = 162623,
        Starfire = 162622,
        Starsurge = 164087,
        Sunfire = 162628,
        Typhoon = 164337,
        Wrath = 162621
    };

    enum eEvents
    {
        EventMoonfire = 1,
        EventStarfire,
        EventStarsurge,
        EventSunfire,
        EventTyphoon,
        EventWrath,
        EventMove
    };

    enum eTalks
    {
        Spawn,
        Slay
    };

    struct npc_ashran_alune_windmaneAI : public ScriptedAI
    {
        npc_ashran_alune_windmaneAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Spawn = false;
        }

        EventMap m_Events;
        EventMap m_MoveEvent;
        bool m_Spawn;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            if (!m_Spawn)
            {
                Talk(Spawn);
                m_Spawn = true;
            }
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventMoonfire, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventStarfire, 3 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventStarsurge, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventSunfire, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventTyphoon, 15 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventWrath, 6 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainAluneWindmane);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventMoonfire:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Moonfire, false);
                    m_Events.ScheduleEvent(EventMoonfire, 12 * IN_MILLISECONDS);
                    break;
                }
                case EventStarfire:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Starfire, false);
                    m_Events.ScheduleEvent(EventStarfire, 10 * IN_MILLISECONDS);
                    break;
                }
                case EventStarsurge:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Starsurge, false);
                    m_Events.ScheduleEvent(EventStarsurge, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventSunfire:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Sunfire, false);
                    m_Events.ScheduleEvent(EventSunfire, 12 * IN_MILLISECONDS);
                    break;
                }
                case EventTyphoon:
                {
                    me->CastSpell(me, Typhoon, false);
                    m_Events.ScheduleEvent(EventTyphoon, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventWrath:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Wrath, false);
                    m_Events.ScheduleEvent(EventWrath, 5 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_alune_windmaneAI(creature);
    }
};

/// Chani Malflame <Alliance Captain> - 85129
class npc_ashran_chani_malflame : public CreatureScript
{
public:
    npc_ashran_chani_malflame() : CreatureScript("npc_ashran_chani_malflame") { }

    enum eSpells
    {
        ChaosBolt = 178076,
        RainOfFire = 178069
    };

    enum eEvents
    {
        EventChaosBolt = 1,
        EventRainOfFire,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Death
    };

    enum eData
    {
        MountID = 25159
    };

    struct npc_ashran_chani_malflameAI : public ScriptedAI
    {
        npc_ashran_chani_malflameAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            me->Mount(MountID);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventChaosBolt, 3 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventRainOfFire, 6 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainChaniMalflame);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventChaosBolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, ChaosBolt, false);
                    m_Events.ScheduleEvent(EventChaosBolt, 15 * IN_MILLISECONDS);
                    break;
                case EventRainOfFire:
                    me->CastSpell(me, RainOfFire, false);
                    m_Events.ScheduleEvent(EventRainOfFire, 20 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_chani_malflameAI(creature);
    }
};

/// Hildie Hackerguard <Alliance Captain> - 80495
class npc_ashran_hildie_hackerguard : public CreatureScript
{
public:
    npc_ashran_hildie_hackerguard() : CreatureScript("npc_ashran_hildie_hackerguard") { }

    enum eSpells
    {
        Blind = 178058,
        CloakOfShadows = 178055,
        Eviscerate = 178054,
        FanOfKnives = 178053,
        Hemorrhage = 178052,
        Shadowstep = 178056,
        WoundPoison = 178050
    };

    enum eEvents
    {
        EventBlind = 1,
        EventCloakOfShadows,
        EventEviscerate,
        EventFanOfKnives,
        EventHemorrhage,
        EventShadowStep,
        EventWoundPoison,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Death
    };

    enum eData
    {
        MountID = 22720
    };

    struct npc_ashran_hildie_hackerguardAI : public ScriptedAI
    {
        npc_ashran_hildie_hackerguardAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            me->Mount(MountID);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventBlind, 15 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventCloakOfShadows, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventEviscerate, 10 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventFanOfKnives, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHemorrhage, 2 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventShadowStep, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventWoundPoison, 5 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainHildieHackerguard);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventBlind:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Blind, false);
                    m_Events.ScheduleEvent(EventBlind, 30 * IN_MILLISECONDS);
                    break;
                }
                case EventCloakOfShadows:
                {
                    if (me->HealthBelowPct(50))
                        me->CastSpell(me, CloakOfShadows, true);
                    else
                        m_Events.ScheduleEvent(EventCloakOfShadows, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventEviscerate:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Eviscerate, false);
                    m_Events.ScheduleEvent(EventEviscerate, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventFanOfKnives:
                {
                    me->CastSpell(me, FanOfKnives, false);
                    m_Events.ScheduleEvent(EventFanOfKnives, 10 * IN_MILLISECONDS);
                    break;
                }
                case EventHemorrhage:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Hemorrhage, false);
                    m_Events.ScheduleEvent(EventHemorrhage, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventShadowStep:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Shadowstep, false);
                    m_Events.ScheduleEvent(EventShadowStep, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventWoundPoison:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, WoundPoison, false);
                    m_Events.ScheduleEvent(EventWoundPoison, 12 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_hildie_hackerguardAI(creature);
    }
};

/// Taylor Dewland <Alliance Captain> - 80500
class npc_ashran_taylor_dewland : public CreatureScript
{
public:
    npc_ashran_taylor_dewland() : CreatureScript("npc_ashran_taylor_dewland") { }

    enum eSpells
    {
        ChainLightning = 178060,
        Hex = 178064,
        LavaBurst = 178091,
        LightningBolt = 178059,
        MagmaTotem = 178063,
        MagmaTotemAura = 178062
    };

    enum eEvents
    {
        EventChainLightning = 1,
        EventHex,
        EventLavaBurst,
        EventLightningBolt,
        EventMagmaTotem,
        EventMove
    };

    struct npc_ashran_taylor_dewlandAI : public ScriptedAI
    {
        npc_ashran_taylor_dewlandAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            /// Second equip is a shield
            me->SetCanDualWield(false);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventChainLightning, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHex, 10 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventLavaBurst, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventLightningBolt, 2 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventMagmaTotem, 15 * IN_MILLISECONDS);
        }

        void JustSummoned(Creature* p_Summon) override
        {
            p_Summon->SetReactState(REACT_PASSIVE);
            p_Summon->AddAura(MagmaTotemAura, p_Summon);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainTaylorDewland);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventChainLightning:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, ChainLightning, false);
                    m_Events.ScheduleEvent(EventChainLightning, 8 * IN_MILLISECONDS);
                    break;
                case EventHex:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Hex, false);
                    m_Events.ScheduleEvent(EventHex, 30 * IN_MILLISECONDS);
                    break;
                case EventLavaBurst:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, LavaBurst, false);
                    m_Events.ScheduleEvent(EventLavaBurst, 15 * IN_MILLISECONDS);
                    break;
                case EventLightningBolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, LightningBolt, false);
                    m_Events.ScheduleEvent(EventLightningBolt, 6 * IN_MILLISECONDS);
                    break;
                case EventMagmaTotem:
                    me->CastSpell(me, MagmaTotem, false);
                    m_Events.ScheduleEvent(EventMagmaTotem, 40 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_taylor_dewlandAI(creature);
    }
};

/// Malda Brewbelly <Alliance Captain> - 85122
class npc_ashran_malda_brewbelly : public CreatureScript
{
public:
    npc_ashran_malda_brewbelly() : CreatureScript("npc_ashran_malda_brewbelly") { }

    enum eSpells
    {
        Shoot = 164095,
        SerpentSting = 162754
    };

    enum eEvents
    {
        EventShoot = 1,
        EventSerpentSting,
        EventMove
    };

    enum eData
    {
        MountID = 44837
    };

    struct npc_ashran_malda_brewbellyAI : public ScriptedAI
    {
        npc_ashran_malda_brewbellyAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            me->Mount(MountID);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventShoot, 3 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventSerpentSting, 5 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainMaldaBrewbelly);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventShoot:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Shoot, false);
                    m_Events.ScheduleEvent(EventShoot, 5 * IN_MILLISECONDS);
                    break;
                case EventSerpentSting:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SerpentSting, false);
                    m_Events.ScheduleEvent(EventSerpentSting, 12 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_malda_brewbellyAI(creature);
    }
};

/// Shani Freezewind <Alliance Captain> - 80485
class npc_ashran_shani_freezewind : public CreatureScript
{
public:
    npc_ashran_shani_freezewind() : CreatureScript("npc_ashran_shani_freezewind") { }

    enum eSpells
    {
        Blizzard = 162610,
        FrostNova = 164067,
        Frostbolt = 162608,
        IceLance = 162609
    };

    enum eEvents
    {
        EventBlizzard = 1,
        EventFrostNova,
        EventFrostbolt,
        EventIceLance,
        EventMove
    };

    enum eData
    {
        MountID = 38668
    };

    struct npc_ashran_shani_freezewindAI : public ScriptedAI
    {
        npc_ashran_shani_freezewindAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            me->Mount(MountID);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            me->Mount(0);

            m_Events.ScheduleEvent(EventBlizzard, 6 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventFrostNova, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventFrostbolt, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventIceLance, 10 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainShaniFreezewind);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventBlizzard:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Blizzard, false);
                    m_Events.ScheduleEvent(EventBlizzard, 20 * IN_MILLISECONDS);
                    break;
                case EventFrostNova:
                    me->CastSpell(me, FrostNova, false);
                    m_Events.ScheduleEvent(EventFrostNova, 20 * IN_MILLISECONDS);
                    break;
                case EventFrostbolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Frostbolt, false);
                    m_Events.ScheduleEvent(EventFrostbolt, 5 * IN_MILLISECONDS);
                    break;
                case EventIceLance:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, IceLance, false);
                    m_Events.ScheduleEvent(EventIceLance, 10 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_shani_freezewindAI(creature);
    }
};

/// Anne Otther <Alliance Captain> - 85140
class npc_ashran_anne_otther : public CreatureScript
{
public:
    npc_ashran_anne_otther() : CreatureScript("npc_ashran_anne_otther") { }

    enum eSpells
    {
        PlagueStrike = 164063,
        DeathAndDecay = 164334,
        DeathCoil = 164064,
        DeathGrip = 79894,
        DeathGripJump = 168563
    };

    enum eEvents
    {
        EventPlagueStrike = 1,
        EventDeathAndDecay,
        EventDeathCoil,
        EventDeathGrip,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Death
    };

    struct npc_ashran_anne_ottherAI : public ScriptedAI
    {
        npc_ashran_anne_ottherAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventPlagueStrike, 2 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathAndDecay, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathCoil, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathGrip, 1 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainAnneOtther);
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            if (p_SpellInfo->Id == DeathGrip)
                p_Target->CastSpell(static_cast<Position>(*me), DeathGripJump, true);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventPlagueStrike:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, PlagueStrike, false);
                    m_Events.ScheduleEvent(EventPlagueStrike, 8 * IN_MILLISECONDS);
                    break;
                }
                case EventDeathAndDecay:
                {
                    me->CastSpell(me, DeathAndDecay, false);
                    m_Events.ScheduleEvent(EventDeathAndDecay, 12 * IN_MILLISECONDS);
                    break;
                }
                case EventDeathCoil:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, DeathCoil, false);
                    m_Events.ScheduleEvent(EventDeathCoil, 10 * IN_MILLISECONDS);
                    break;
                }
                case EventDeathGrip:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, DeathGrip, false);
                    m_Events.ScheduleEvent(EventDeathGrip, 20 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_anne_ottherAI(creature);
    }
};

/// Mathias Zunn <Alliance Captain> - 85137
class npc_ashran_mathias_zunn : public CreatureScript
{
public:
    npc_ashran_mathias_zunn() : CreatureScript("npc_ashran_mathias_zunn") { }

    enum eSpells
    {
        DevouringPlague = 164452,
        Dispersion = 164444,
        MindBlast = 164448,
        MindSear = 177402,
        PsychicScream = 164443,
        ShadowWordPain = 164446
    };

    enum eEvents
    {
        EventDevouringPlague = 1,
        EventDispersion,
        EventMindBlast,
        EventMindSear,
        EventPsychicScream,
        EventShadowWordPain,
        EventMove
    };

    enum eTalks
    {
        Spawn,
        Death
    };

    struct npc_ashran_mathias_zunnAI : public ScriptedAI
    {
        npc_ashran_mathias_zunnAI(Creature* creature) : ScriptedAI(creature)
        {
            m_Spawn = false;
        }

        EventMap m_Events;
        EventMap m_MoveEvent;
        bool m_Spawn;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1 * IN_MILLISECONDS);

            Reset();
        }

        void Reset() override
        {
            m_Events.Reset();

            if (!m_Spawn)
            {
                Talk(Spawn);
                m_Spawn = true;
            }
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventDevouringPlague, 10 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDispersion, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventMindBlast, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventMindSear, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventPsychicScream, 15 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventShadowWordPain, 1 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainMathiasZunn);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Frangraal
                me->LoadPath(Frangraal);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventDevouringPlague:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, DevouringPlague, false);
                    m_Events.ScheduleEvent(EventDevouringPlague, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventDispersion:
                {
                    if (me->HealthBelowPct(50))
                        me->CastSpell(me, Dispersion, true);
                    else
                        m_Events.ScheduleEvent(EventDispersion, 1 * IN_MILLISECONDS);
                    break;
                }
                case EventMindBlast:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, MindBlast, false);
                    m_Events.ScheduleEvent(EventMindBlast, 10 * IN_MILLISECONDS);
                    break;
                }
                case EventMindSear:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, MindSear, false);
                    m_Events.ScheduleEvent(EventMindSear, 20 * IN_MILLISECONDS);
                    break;
                }
                case EventPsychicScream:
                {
                    me->CastSpell(me, PsychicScream, false);
                    m_Events.ScheduleEvent(EventPsychicScream, 30 * IN_MILLISECONDS);
                    break;
                }
                case EventShadowWordPain:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, ShadowWordPain, false);
                    m_Events.ScheduleEvent(EventShadowWordPain, 12 * IN_MILLISECONDS);
                    break;
                }
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_mathias_zunnAI(creature);
    }
};

/// Ex Alliance Racer - 82884
class npc_ashran_ex_alliance_racer : public CreatureScript
{
public:
    npc_ashran_ex_alliance_racer() : CreatureScript("npc_ashran_ex_alliance_racer") { }

    enum eSpell
    {
        AllianceRacer = 166784
    };

    struct npc_ashran_ex_alliance_racerAI : public MS::AI::CosmeticAI
    {
        npc_ashran_ex_alliance_racerAI(Creature* creature) : CosmeticAI(creature), m_MoveIndex{0}
        {
            m_CheckCooldown = uint32(time(nullptr) + 5);
        }

        uint8 m_MoveIndex;
        uint32 m_CheckCooldown;

        void InitializeAI() override
        {
            Reset();
        }

        void Reset() override
        {
            me->CastSpell(me, AllianceRacer, true);
            me->ModifyAuraState(AURA_STATE_UNKNOWN22, true);

            m_MoveIndex = 0;

            AddDelayedEvent(500, [this]() -> void
            {
                if (Creature* l_Rider = me->FindNearestCreature(AllianceRider, 10.0f))
                    l_Rider->EnterVehicle(me);
            });

            AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(m_MoveIndex, g_AllianceRacingMoves[m_MoveIndex]);
            });
        }

        void MovementInform(uint32 type, uint32 /*p_ID*/) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            ++m_MoveIndex;

            if (m_MoveIndex >= AllianceRacingMovesCount)
            {
                m_MoveIndex = 0;
                IncreaseLapCount();
            }

            AddDelayedEvent(100, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(m_MoveIndex, g_AllianceRacingMoves[m_MoveIndex]);
            });
        }

        void IncreaseLapCount()
        {
            OutdoorPvP* l_Outdoor = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_Outdoor))
                l_Ashran->SetEventData(EventStadiumRacing, TEAM_ALLIANCE, 1);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_ex_alliance_racerAI(creature);
    }
};
void AddSC_AshranNPCAlliance()
{
    new npc_rylai_crestfall();
    new npc_ashran_grimnir_sternhammer();
    new npc_ashran_misirin_stouttoe();
    new npc_ashran_stormshield_druid();
    new npc_ashran_officer_rumsfeld();
    new npc_ashran_officer_ironore();
    new npc_ashran_marketa();
    new npc_ashran_ecilam();
    new npc_ashran_valant_brightsworn();
    new npc_ashran_anenga();
    new npc_ashran_kauper();
    new npc_ashran_alliance_gateway_guardian();
    new npc_ashran_fangraal();
    new npc_ashran_lifeless_ancient();
    new npc_ashran_stormshield_stormcrow();
    new npc_ashran_stormshield_gladiator();
    new npc_ashran_wrynns_vanguard_battle_standard();
    new npc_ashran_stormshield_sentinel();
    new npc_ashran_avenger_turley();
    new npc_ashran_jackson_bajheera();
    new npc_ashran_john_swifty();
    new npc_ashran_tosan_galaxyfist();
    new npc_ashran_brock_the_crazed();
    new npc_ashran_alune_windmane();
    new npc_ashran_chani_malflame();
    new npc_ashran_hildie_hackerguard();
    new npc_ashran_taylor_dewland();
    new npc_ashran_malda_brewbelly();
    new npc_ashran_shani_freezewind();
    new npc_ashran_anne_otther();
    new npc_ashran_mathias_zunn();
    new npc_ashran_ex_alliance_racer();
}
