////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "AshranMgr.hpp"
#include "ScriptedCosmeticAI.h"

/// Jeron Emberfall <Warspear Tower Guardian> - 88178
class npc_jeron_emberfall : public CreatureScript
{
public:
    npc_jeron_emberfall() : CreatureScript("npc_jeron_emberfall") { }

    struct npc_jeron_emberfallAI : public ScriptedAI
    {
        npc_jeron_emberfallAI(Creature* creature) : ScriptedAI(creature)
        {
            m_CheckAroundingPlayersTimer = 0;
            m_PhoenixStrikeTimer = 0;
        }

        enum eSpells
        {
            Fireball = 176652,
            Ignite = 176600,
            CombustionNova = 176605,
            CombustionNovaStun = 176608,
            LivingBomb = 176670,
            SummonLavaFury = 176664,

            TargetedByTheTowerMage = 176076,
            PhoenixStrikeSearcher = 176086,
            PhoenixStrikeMissile = 176066,

            ConjureRefreshment = 176351
        };

        enum eTalk
        {
            TalkAggro,
            TalkSlay,
            TalkDeath,
            TalkSpell,
            TalkLivingBomb
        };

        enum eEvents
        {
            EventFireball = 1,
            EventIgnite,
            EventLivingBomb,
            EventCombustionNova
        };

        EventMap m_Events;

        uint32 m_CheckAroundingPlayersTimer;
        uint32 m_PhoenixStrikeTimer;

        void Reset() override
        {
            m_Events.Reset();

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            Talk(TalkAggro);

            m_Events.ScheduleEvent(EventFireball, 4000);
            m_Events.ScheduleEvent(EventIgnite, 8000);
            m_Events.ScheduleEvent(EventLivingBomb, 12000);
            m_Events.ScheduleEvent(EventCombustionNova, 15000);
        }

        void KilledUnit(Unit* p_Who) override
        {
            if (p_Who->IsPlayer())
                Talk(TalkSlay);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(TalkDeath);
        }

        void SpellHitTarget(Unit* p_Victim, SpellInfo const* p_SpellInfo) override
        {
            if (p_Victim == nullptr)
                return;

            switch (p_SpellInfo->Id)
            {
                case PhoenixStrikeSearcher:
                    me->CastSpell(p_Victim, PhoenixStrikeMissile, false);
                    break;
                case CombustionNova:
                    if (p_Victim->HasAura(Ignite))
                        me->CastSpell(p_Victim, CombustionNovaStun, true);
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
                SchedulePhoenixStrike(p_Diff);
                return;
            }

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventFireball:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM))
                        me->CastSpell(l_Target, Fireball, false);
                    m_Events.ScheduleEvent(EventFireball, 10000);
                    break;
                case EventIgnite:
                    Talk(TalkSpell);
                    me->CastSpell(me, Ignite, true);
                    m_Events.ScheduleEvent(EventIgnite, 9000);
                    break;
                case EventLivingBomb:
                    Talk(TalkLivingBomb);
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_RANDOM))
                        me->CastSpell(l_Target, LivingBomb, false);
                    m_Events.ScheduleEvent(EventLivingBomb, 15000);
                    break;
                case EventCombustionNova:
                    Talk(TalkSpell);
                    me->CastSpell(me, CombustionNova, false);
                    m_Events.ScheduleEvent(EventCombustionNova, 20000);
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

                    if (player->HasAura(TargetedByTheTowerMage))
                        return true;

                    return false;
                });

                for (Player* l_Player : l_PlayerList)
                    l_Player->CastSpell(l_Player, TargetedByTheTowerMage, true, nullptr, nullptr, me->GetGUID());
            }
            else
                m_CheckAroundingPlayersTimer -= p_Diff;
        }

        void SchedulePhoenixStrike(uint32 const p_Diff)
        {
            if (!m_PhoenixStrikeTimer)
                return;

            if (m_PhoenixStrikeTimer <= p_Diff)
            {
                if (!me->isInCombat())
                    me->CastSpell(me, PhoenixStrikeSearcher, true);
                m_PhoenixStrikeTimer = 10000;
            }
            else
                m_PhoenixStrikeTimer -= p_Diff;
        }

        void sGossipSelect(Player* player, uint32 /*p_Sender*/, uint32 /*p_Action*/) override
        {
            player->PlayerTalkClass->SendCloseGossip();
            me->CastSpell(player, ConjureRefreshment, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_jeron_emberfallAI(creature);
    }
};

/// Warspear Shaman - 82438
class npc_ashran_warspear_shaman : public CreatureScript
{
public:
    npc_ashran_warspear_shaman() : CreatureScript("npc_ashran_warspear_shaman") { }

    struct npc_ashran_warspear_shamanAI : public ScriptedAI
    {
        npc_ashran_warspear_shamanAI(Creature* creature) : ScriptedAI(creature) { }

        enum eDatas
        {
            EventCosmetic = 1,
            EarthFury = 82200,
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
                if (Creature* l_EarthFury = me->FindNearestCreature(EarthFury, 15.0f))
                    me->CastSpell(l_EarthFury, NatureChanneling, false);
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_warspear_shamanAI(creature);
    }
};

/// Illandria Belore - 88675
class npc_ashran_illandria_belore : public CreatureScript
{
public:
    npc_ashran_illandria_belore() : CreatureScript("npc_ashran_illandria_belore") { }

    enum eTalks
    {
        First,
        Second,
        Third,
        Fourth,
        Fifth
    };

    enum eData
    {
        RahmFlameheart = 88676,
        ActionInit = 0,
        ActionLoop = 1,
        EventLoop = 1
    };

    struct npc_ashran_illandria_beloreAI : public MS::AI::CosmeticAI
    {
        npc_ashran_illandria_beloreAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;
        EventMap m_Events;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(RahmFlameheart, 15.0f))
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
            AddDelayedEvent(18 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(36 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
            AddDelayedEvent(66 * IN_MILLISECONDS, [this]() -> void { Talk(Fourth); });
            AddDelayedEvent(83 * IN_MILLISECONDS, [this]() -> void { Talk(Fifth); });
        }

        void LastOperationCalled() override
        {
            m_Events.ScheduleEvent(EventLoop, 48 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            CosmeticAI::UpdateAI(p_Diff);

            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == EventLoop)
            {
                if (Creature* l_Creature = me->FindNearestCreature(RahmFlameheart, 15.0f))
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
        return new npc_ashran_illandria_beloreAI(creature);
    }
};

/// Examiner Rahm Flameheart <The Reliquary> - 88676
class npc_ashran_examiner_rahm_flameheart : public CreatureScript
{
public:
    npc_ashran_examiner_rahm_flameheart() : CreatureScript("npc_ashran_examiner_rahm_flameheart") { }

    enum eTalks
    {
        First,
        Second,
        Third,
        Fourth
    };

    enum eData
    {
        IllandriaBelore = 88675,
        ActionInit = 0,
        ActionLoop = 1
    };

    struct npc_ashran_examiner_rahm_flameheartAI : public MS::AI::CosmeticAI
    {
        npc_ashran_examiner_rahm_flameheartAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(IllandriaBelore, 15.0f))
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
            AddDelayedEvent(10 * IN_MILLISECONDS, [this]() -> void { Talk(First); });
            AddDelayedEvent(27 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(75 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
            AddDelayedEvent(92 * IN_MILLISECONDS, [this]() -> void { Talk(Fourth); });
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_examiner_rahm_flameheartAI(creature);
    }
};

/// Centurion Firescream <Warspear Tactician> - 88771
class npc_ashran_centurion_firescream : public CreatureScript
{
public:
    npc_ashran_centurion_firescream() : CreatureScript("npc_ashran_centurion_firescream") { }

    struct npc_ashran_centurion_firescreamAI : public MS::AI::CosmeticAI
    {
        npc_ashran_centurion_firescreamAI(Creature* creature) : CosmeticAI(creature) { }

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
        return new npc_ashran_centurion_firescreamAI(creature);
    }
};

/// Legionnaire Hellaxe <Warspear Strategist> - 88772
class npc_ashran_legionnaire_hellaxe : public CreatureScript
{
public:
    npc_ashran_legionnaire_hellaxe() : CreatureScript("npc_ashran_legionnaire_hellaxe") { }

    struct npc_ashran_legionnaire_hellaxeAI : public MS::AI::CosmeticAI
    {
        npc_ashran_legionnaire_hellaxeAI(Creature* creature) : CosmeticAI(creature) { }

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
        return new npc_ashran_legionnaire_hellaxeAI(creature);
    }
};

/// Kalgan <Warspear Warrior Leader> - 83830
class npc_ashran_kalgan : public CreatureScript
{
public:
    npc_ashran_kalgan() : CreatureScript("npc_ashran_kalgan") { }

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
            if (l_Ashran->IsArtifactEventLaunched(TEAM_HORDE, CountForWarriorPaladin))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::RidersSummoned, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_kalganAI : public ScriptedAI
    {
        npc_ashran_kalganAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->AddCollectedArtifacts(TEAM_HORDE, CountForWarriorPaladin, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_kalganAI(creature);
    }
};

/// Fura <Warspear Mage Leader> - 83995
class npc_ashran_fura : public CreatureScript
{
public:
    npc_ashran_fura() : CreatureScript("npc_ashran_fura") { }

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
            if (l_Ashran->IsArtifactEventLaunched(TEAM_HORDE, CountForMage))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::PortalInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_furaAI : public ScriptedAI
    {
        npc_ashran_furaAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->AddCollectedArtifacts(TEAM_HORDE, CountForMage, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_furaAI(creature);
    }
};

/// Nisstyr <Warspear Warlock Leader> - 83997
class npc_ashran_nisstyr : public CreatureScript
{
public:
    npc_ashran_nisstyr() : CreatureScript("npc_ashran_nisstyr") { }

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
            if (l_Ashran->IsArtifactEventLaunched(TEAM_HORDE, CountForWarlock))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::GatewaysInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_nisstyrAI : public ScriptedAI
    {
        npc_ashran_nisstyrAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->AddCollectedArtifacts(TEAM_HORDE, CountForWarlock, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_nisstyrAI(creature);
    }
};

/// Atomik <Warspear Shaman Leader> - 82204
class npc_ashran_atomik : public CreatureScript
{
public:
    npc_ashran_atomik() : CreatureScript("npc_ashran_atomik") { }

    enum eGossip
    {
        KronusInvoked = 89853
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(creature->GetZoneId());
        if (l_ZoneScript == nullptr)
            return false;

        if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
        {
            if (l_Ashran->IsArtifactEventLaunched(TEAM_HORDE, CountForDruidShaman))
            {
                player->PlayerTalkClass->ClearMenus();
                player->SEND_GOSSIP_MENU(eGossip::KronusInvoked, creature->GetGUID());
                return true;
            }
        }

        return false;
    }

    struct npc_ashran_atomikAI : public ScriptedAI
    {
        npc_ashran_atomikAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->AddCollectedArtifacts(TEAM_HORDE, CountForDruidShaman, artifactCount);
                l_Ashran->RewardHonorAndReputation(artifactCount, player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_atomikAI(creature);
    }
};

/// Zaram Sunraiser <Portal Guardian> - 84468
class npc_ashran_zaram_sunraiser : public CreatureScript
{
public:
    npc_ashran_zaram_sunraiser() : CreatureScript("npc_ashran_zaram_sunraiser") { }

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

    struct npc_ashran_zaram_sunraiserAI : public ScriptedAI
    {
        npc_ashran_zaram_sunraiserAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->EndArtifactEvent(TEAM_HORDE, CountForMage);
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
        return new npc_ashran_zaram_sunraiserAI(creature);
    }
};

/// Gayle Plagueheart <Gateway Guardian> - 84645
/// Ilya Plagueheart <Gateway Guardian> - 84646
class npc_ashran_horde_gateway_guardian : public CreatureScript
{
public:
    npc_ashran_horde_gateway_guardian() : CreatureScript("npc_ashran_horde_gateway_guardian") { }

    enum eSpells
    {
        SpellChaosBolt = 79939,
        SpellFelArmor = 165735,
        SpellImmolate = 79937,
        SpellIncinerate = 79938
    };

    enum eEvents
    {
        EventChaosBolt = 1,
        EventImmolate,
        EventIncinerate
    };

    struct npc_ashran_horde_gateway_guardianAI : public ScriptedAI
    {
        npc_ashran_horde_gateway_guardianAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;

        void Reset() override
        {
            me->CastSpell(me, SpellFelArmor, true);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->CastSpell(me, SpellFelArmor, true);

            m_Events.ScheduleEvent(EventImmolate, 1000);
            m_Events.ScheduleEvent(EventIncinerate, 2000);
            m_Events.ScheduleEvent(EventChaosBolt, 5000);
        }

        void JustDied(Unit* /*killer*/) override
        {
            ZoneScript* l_ZoneScript = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (l_ZoneScript == nullptr)
                return;

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_ZoneScript))
                l_Ashran->EndArtifactEvent(TEAM_HORDE, CountForWarlock);
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
                case EventChaosBolt:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellChaosBolt, false);
                    m_Events.ScheduleEvent(EventChaosBolt, 12000);
                    break;
                case EventImmolate:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellImmolate, false);
                    m_Events.ScheduleEvent(EventImmolate, 9000);
                    break;
                case EventIncinerate:
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, SpellIncinerate, false);
                    m_Events.ScheduleEvent(EventIncinerate, 5000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_horde_gateway_guardianAI(creature);
    }
};

/// Kronus <Horde Guardian> - 82201
class npc_ashran_kronus : public CreatureScript
{
public:
    npc_ashran_kronus() : CreatureScript("npc_ashran_kronus") { }

    enum eSpells
    {
        AshranLaneMobScalingAura = 164310,

        SpellFractureSearcher = 170892,
        SpellFractureMissile = 170893,   ///< Trigger 170894
        SpellGroundPound = 170905,   ///< Periodic Trigger 177605
        SpellRockShield = 175058,
        SpellStoneEmpowermentAura = 170896
    };

    enum eEvents
    {
        EventFracture = 1,
        EventGroundPound,
        EventRockShield,
        EventMove
    };

    struct npc_ashran_kronusAI : public ScriptedAI
    {
        npc_ashran_kronusAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_Events;
        EventMap m_MoveEvent;

        void InitializeAI() override
        {
            m_MoveEvent.ScheduleEvent(EventMove, 1000);

            /// Kronus no longer scales their health based the number of players he's fighting.
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
        }

        void Reset() override
        {
            me->setRegeneratingHealth(false);
            me->CastSpell(me, SpellStoneEmpowermentAura, true);

            me->SetReactState(REACT_AGGRESSIVE);

            m_Events.Reset();
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            Position l_Pos;
            me->GetPosition(&l_Pos);
            me->SetHomePosition(l_Pos);

            m_Events.ScheduleEvent(EventFracture, 5000);
            m_Events.ScheduleEvent(EventGroundPound, 12000);
            m_Events.ScheduleEvent(EventRockShield, 9000);
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
                if (l_Ashran->IsArtifactEventLaunched(TEAM_HORDE, CountForDruidShaman))
                {
                    l_Ashran->CastSpellOnTeam(me, TEAM_ALLIANCE, SpellEventAllianceReward);
                    l_Ashran->EndArtifactEvent(TEAM_HORDE, CountForDruidShaman);
                }
            }
        }

        void SpellHitTarget(Unit* p_Target, SpellInfo const* p_SpellInfo) override
        {
            if (p_Target == nullptr)
                return;

            if (p_SpellInfo->Id == SpellFractureSearcher)
                me->CastSpell(p_Target, SpellFractureMissile, true);
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

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (m_Events.ExecuteEvent())
            {
                case EventFracture:
                    me->CastSpell(me, SpellFractureSearcher, true);
                    m_Events.ScheduleEvent(EventFracture, 16000);
                    break;
                case EventGroundPound:
                    me->CastSpell(me, SpellGroundPound, false);
                    m_Events.ScheduleEvent(EventGroundPound, 43000);
                    break;
                case EventRockShield:
                    me->CastSpell(me, SpellRockShield, true);
                    m_Events.ScheduleEvent(EventRockShield, 39000);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_kronusAI(creature);
    }
};

/// Underpowered Earth Fury <Horde Guardian> - 82200
class npc_ashran_underpowered_earth_fury : public CreatureScript
{
public:
    npc_ashran_underpowered_earth_fury() : CreatureScript("npc_ashran_underpowered_earth_fury") { }

    struct npc_ashran_underpowered_earth_furyAI : public ScriptedAI
    {
        npc_ashran_underpowered_earth_furyAI(Creature* creature) : ScriptedAI(creature) { }

        enum eData
        {
            WarspearShaman = 82438
        };

        EventMap m_Events;

        void Reset() override
        {
            std::list<Creature*> l_WarspearShamans;
            me->GetCreatureListWithEntryInGrid(l_WarspearShamans, WarspearShaman, 20.0f);

            for (Creature* l_Creature : l_WarspearShamans)
            {
                if (l_Creature->AI())
                    l_Creature->AI()->Reset();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_underpowered_earth_furyAI(creature);
    }
};

/// Warspear Gladiator - 85811
class npc_ashran_warspear_gladiator : public CreatureScript
{
public:
    npc_ashran_warspear_gladiator() : CreatureScript("npc_ashran_warspear_gladiator") { }

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

    struct npc_ashran_warspear_gladiatorAI : public ScriptedAI
    {
        npc_ashran_warspear_gladiatorAI(Creature* creature) : ScriptedAI(creature) { }

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
        return new npc_ashran_warspear_gladiatorAI(creature);
    }
};

/// Excavator Rustshiv - 88568
class npc_ashran_excavator_rustshiv : public CreatureScript
{
public:
    npc_ashran_excavator_rustshiv() : CreatureScript("npc_ashran_excavator_rustshiv") { }

    enum eTalks
    {
        First,
        Second,
        Third,
        Fourth,
        Fifth,
        Sixth
    };

    enum eData
    {
        ExcavatorHardtooth = 88567,
        ActionInit = 0,
        ActionLoop = 1,
        EventLoop = 1
    };

    struct npc_ashran_excavator_rustshivAI : public MS::AI::CosmeticAI
    {
        npc_ashran_excavator_rustshivAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;
        EventMap m_Events;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(ExcavatorHardtooth, 15.0f))
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
            AddDelayedEvent(18 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(36 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
            AddDelayedEvent(67 * IN_MILLISECONDS, [this]() -> void { Talk(Fourth); });
            AddDelayedEvent(84 * IN_MILLISECONDS, [this]() -> void { Talk(Fifth); });
            AddDelayedEvent(101 * IN_MILLISECONDS, [this]() -> void { Talk(Sixth); });
        }

        void LastOperationCalled() override
        {
            m_Events.ScheduleEvent(EventLoop, 31 * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            CosmeticAI::UpdateAI(p_Diff);

            m_Events.Update(p_Diff);

            if (m_Events.ExecuteEvent() == EventLoop)
            {
                if (Creature* l_Creature = me->FindNearestCreature(ExcavatorHardtooth, 15.0f))
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
        return new npc_ashran_excavator_rustshivAI(creature);
    }
};

/// Excavator Hardtooth - 88567
class npc_ashran_excavator_hardtooth : public CreatureScript
{
public:
    npc_ashran_excavator_hardtooth() : CreatureScript("npc_ashran_excavator_hardtooth") { }

    enum eTalks
    {
        First,
        Second,
        Third,
        Fourth
    };

    enum eData
    {
        ExcavatorRustshiv = 88568,
        ActionInit = 0,
        ActionLoop = 1
    };

    struct npc_ashran_excavator_hardtoothAI : public MS::AI::CosmeticAI
    {
        npc_ashran_excavator_hardtoothAI(Creature* creature) : CosmeticAI(creature), m_Init{false}
        {
        }

        bool m_Init;

        void Reset() override
        {
            m_Init = false;

            if (Creature* l_Creature = me->FindNearestCreature(ExcavatorRustshiv, 15.0f))
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
            AddDelayedEvent(10 * IN_MILLISECONDS, [this]() -> void { Talk(First); });
            AddDelayedEvent(27 * IN_MILLISECONDS, [this]() -> void { Talk(Second); });
            AddDelayedEvent(76 * IN_MILLISECONDS, [this]() -> void { Talk(Third); });
            AddDelayedEvent(93 * IN_MILLISECONDS, [this]() -> void { Talk(Fourth); });
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_excavator_hardtoothAI(creature);
    }
};

/// Vol'jin's Spear Battle Standard - 85383
class npc_ashran_voljins_spear_battle_standard : public CreatureScript
{
public:
    npc_ashran_voljins_spear_battle_standard() : CreatureScript("npc_ashran_voljins_spear_battle_standard") { }

    struct npc_ashran_voljins_spear_battle_standardAI : public ScriptedAI
    {
        npc_ashran_voljins_spear_battle_standardAI(Creature* creature) : ScriptedAI(creature) { }

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
        return new npc_ashran_voljins_spear_battle_standardAI(creature);
    }
};

/// Warspear Headhunter - 88691
class npc_ashran_warspear_headhunter : public CreatureScript
{
public:
    npc_ashran_warspear_headhunter() : CreatureScript("npc_ashran_warspear_headhunter") { }

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

    struct npc_ashran_warspear_headhunterAI : public ScriptedAI
    {
        npc_ashran_warspear_headhunterAI(Creature* creature) : ScriptedAI(creature)
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
        return new npc_ashran_warspear_headhunterAI(creature);
    }
};

/// Lord Mes <Horde Captain> - 80497
class npc_ashran_lord_mes : public CreatureScript
{
public:
    npc_ashran_lord_mes() : CreatureScript("npc_ashran_lord_mes") { }

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
        Spawn,
        Death
    };

    enum eData
    {
        MountID = 25280
    };

    struct npc_ashran_lord_mesAI : public ScriptedAI
    {
        npc_ashran_lord_mesAI(Creature* creature) : ScriptedAI(creature)
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
            me->Mount(0);
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventPlagueStrike, 2 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathAndDecay, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathCoil, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDeathGrip, 1 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainLordMes);
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
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_lord_mesAI(creature);
    }
};

/// Mindbender Talbadar <Horde Captain> - 80490
class npc_ashran_mindbender_talbadar : public CreatureScript
{
public:
    npc_ashran_mindbender_talbadar() : CreatureScript("npc_ashran_mindbender_talbadar") { }

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

    enum eTalk
    {
        Spawn
    };

    struct npc_ashran_mindbender_talbadarAI : public ScriptedAI
    {
        npc_ashran_mindbender_talbadarAI(Creature* creature) : ScriptedAI(creature)
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
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainMindbenderTalbadar);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_mindbender_talbadarAI(creature);
    }
};

/// Elliott Van Rook <Horde Captain> - 80493
class npc_ashran_elliott_van_rook : public CreatureScript
{
public:
    npc_ashran_elliott_van_rook() : CreatureScript("npc_ashran_elliott_van_rook") { }

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

    enum eTalks
    {
        Slay,
        Death
    };

    enum eData
    {
        MountID = 51048
    };

    struct npc_ashran_elliott_van_rookAI : public ScriptedAI
    {
        npc_ashran_elliott_van_rookAI(Creature* creature) : ScriptedAI(creature) { }

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
            me->Mount(0);
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventBlizzard, 6 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventFrostNova, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventFrostbolt, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventIceLance, 10 * IN_MILLISECONDS);
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
                l_Ashran->HandleCaptainDeath(CaptainElliotVanRook);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_elliott_van_rookAI(creature);
    }
};

/// Vanguard Samuelle <Horde Captain> - 80492
class npc_ashran_vanguard_samuelle : public CreatureScript
{
public:
    npc_ashran_vanguard_samuelle() : CreatureScript("npc_ashran_vanguard_samuelle") { }

    enum eSpells
    {
        Judgment = 162760,
        HammerOfWrath = 162763,
        DivineShield = 164410,
        DivineStorm = 162641,
        HammerOfJustice = 162764,
        AvengingWrath = 164397
    };

    enum eEvents
    {
        EventJudgment = 1,
        EventHammerOfWrath,
        EventDivineShield,
        EventDivineStorm,
        EventHammerOfJustice,
        EventAvengingWrath,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Death
    };

    struct npc_ashran_vanguard_samuelleAI : public ScriptedAI
    {
        npc_ashran_vanguard_samuelleAI(Creature* creature) : ScriptedAI(creature) { }

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

            m_Events.ScheduleEvent(EventJudgment, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHammerOfWrath, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDivineShield, 1 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventDivineStorm, 8 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventHammerOfJustice, 7 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventAvengingWrath, 10 * IN_MILLISECONDS);
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
                l_Ashran->HandleCaptainDeath(CaptainVanguardSamuelle);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
                case EventJudgment:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, Judgment, false);
                    m_Events.ScheduleEvent(EventJudgment, 15 * IN_MILLISECONDS);
                    break;
                }
                case EventHammerOfWrath:
                {
                    if (Unit* l_Target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        me->CastSpell(l_Target, HammerOfWrath, false);
                    m_Events.ScheduleEvent(EventHammerOfWrath, 15 * IN_MILLISECONDS);
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
                case EventAvengingWrath:
                {
                    me->CastSpell(me, AvengingWrath, false);
                    m_Events.ScheduleEvent(EventAvengingWrath, 45 * IN_MILLISECONDS);
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
        return new npc_ashran_vanguard_samuelleAI(creature);
    }
};

/// Elementalist Novo <Horde Captain> - 80491
class npc_ashran_elementalist_novo : public CreatureScript
{
public:
    npc_ashran_elementalist_novo() : CreatureScript("npc_ashran_elementalist_novo") { }

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

    enum eTalk
    {
        Death
    };

    struct npc_ashran_elementalist_novoAI : public ScriptedAI
    {
        npc_ashran_elementalist_novoAI(Creature* creature) : ScriptedAI(creature) { }

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

        void JustDied(Unit* /*killer*/) override
        {
            Talk(Death);

            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainElementalistNovo);
        }

        void JustSummoned(Creature* p_Summon) override
        {
            p_Summon->SetReactState(REACT_PASSIVE);
            p_Summon->AddAura(MagmaTotemAura, p_Summon);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_elementalist_novoAI(creature);
    }
};

/// Captain Hoodrych <Horde Captain> - 79900
class npc_ashran_captain_hoodrych : public CreatureScript
{
public:
    npc_ashran_captain_hoodrych() : CreatureScript("npc_ashran_captain_hoodrych") { }

    enum eSpells
    {
        SpellBladestorm = 164091,
        Shockwave = 164092
    };

    enum eEvents
    {
        EventBladestorm = 1,
        EventShockwave,
        EventMove
    };

    enum eTalks
    {
        Slay,
        Bladestorm,
        Death
    };

    enum eData
    {
        MountID = 38607
    };

    struct npc_ashran_captain_hoodrychAI : public ScriptedAI
    {
        npc_ashran_captain_hoodrychAI(Creature* creature) : ScriptedAI(creature) { }

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
            me->Mount(0);
            me->SetHomePosition(*me);

            m_Events.ScheduleEvent(EventBladestorm, 5 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventShockwave, 10 * IN_MILLISECONDS);
        }

        void KilledUnit(Unit* killed) override
        {
            if (killed->GetTypeId() == TYPEID_PLAYER)
                Talk(Slay);
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
                l_Ashran->HandleCaptainDeath(CaptainCaptainHoodrych);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
                me->SetDefaultMovementType(WAYPOINT_MOTION_TYPE);
                me->GetMotionMaster()->Initialize();
            }

            if (!UpdateVictim())
                return;

            m_Events.Update(p_Diff);

            /// Update position during Bladestorm
            if (me->HasAura(SpellBladestorm))
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
                    Talk(Bladestorm);
                    me->CastSpell(me, SpellBladestorm, false);
                    m_Events.ScheduleEvent(EventBladestorm, 15 * IN_MILLISECONDS);
                    break;
                case EventShockwave:
                    me->CastSpell(me, Shockwave, false);
                    m_Events.ScheduleEvent(EventShockwave, 20 * IN_MILLISECONDS);
                    break;
                default:
                    break;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_captain_hoodrychAI(creature);
    }
};

/// Soulbrewer Nadagast <Horde Captain> - 80489
class npc_ashran_soulbrewer_nadagast : public CreatureScript
{
public:
    npc_ashran_soulbrewer_nadagast() : CreatureScript("npc_ashran_soulbrewer_nadagast") { }

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
        Spawn,
        Slay
    };

    struct npc_ashran_soulbrewer_nadagastAI : public ScriptedAI
    {
        npc_ashran_soulbrewer_nadagastAI(Creature* creature) : ScriptedAI(creature)
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
                m_Spawn = true;
                Talk(Spawn);
            }

            /// Second equip is a Off-hand Frill
            me->SetCanDualWield(false);
        }

        void EnterCombat(Unit* /*p_Attacker*/) override
        {
            me->SetHomePosition(*me);

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
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainSoulbrewerNadagast);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_soulbrewer_nadagastAI(creature);
    }
};

/// Necrolord Azael <Horde Captain> - 80486
class npc_ashran_necrolord_azael : public CreatureScript
{
public:
    npc_ashran_necrolord_azael() : CreatureScript("npc_ashran_necrolord_azael") { }

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
        MountID = 51048
    };

    struct npc_ashran_necrolord_azaelAI : public ScriptedAI
    {
        npc_ashran_necrolord_azaelAI(Creature* creature) : ScriptedAI(creature) { }

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
            me->Mount(0);
            me->SetHomePosition(*me);

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
                l_Ashran->HandleCaptainDeath(CaptainNecrolordAzael);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_necrolord_azaelAI(creature);
    }
};

/// Rifthunter Yoske <Horde Captain> - 80496
class npc_ashran_rifthunter_yoske : public CreatureScript
{
public:
    npc_ashran_rifthunter_yoske() : CreatureScript("npc_ashran_rifthunter_yoske") { }

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

    enum eTalks
    {
        Slay,
        Death
    };

    struct npc_ashran_rifthunter_yoskeAI : public ScriptedAI
    {
        npc_ashran_rifthunter_yoskeAI(Creature* creature) : ScriptedAI(creature) { }

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

            m_Events.ScheduleEvent(EventShoot, 3 * IN_MILLISECONDS);
            m_Events.ScheduleEvent(EventSerpentSting, 5 * IN_MILLISECONDS);
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
                l_Ashran->HandleCaptainDeath(CaptainRifthunterYoske);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_rifthunter_yoskeAI(creature);
    }
};

/// Mor'riz <The Ultimate Troll> - 85133
class npc_ashran_morriz : public CreatureScript
{
public:
    npc_ashran_morriz() : CreatureScript("npc_ashran_morriz") { }

    enum eSpell
    {
        Typhoon = 164337
    };

    enum eEvents
    {
        EventTyphoon = 1,
        EventMove
    };

    struct npc_ashran_morrizAI : public ScriptedAI
    {
        npc_ashran_morrizAI(Creature* creature) : ScriptedAI(creature) { }

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

            m_Events.ScheduleEvent(EventTyphoon, 15 * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(me->GetZoneScript()))
                l_Ashran->HandleCaptainDeath(CaptainMorriz);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
                case EventTyphoon:
                {
                    me->CastSpell(me, Typhoon, false);
                    m_Events.ScheduleEvent(EventTyphoon, 20 * IN_MILLISECONDS);
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
        return new npc_ashran_morrizAI(creature);
    }
};

/// Kaz Endsky <Horde Captain> - 87690
class npc_ashran_kaz_endsky : public CreatureScript
{
public:
    npc_ashran_kaz_endsky() : CreatureScript("npc_ashran_kaz_endsky") { }

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

    enum eData
    {
        MountID = 25280
    };

    struct npc_ashran_kaz_endskyAI : public ScriptedAI
    {
        npc_ashran_kaz_endskyAI(Creature* creature) : ScriptedAI(creature) { }

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
            me->Mount(0);
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
                l_Ashran->HandleCaptainDeath(CaptainKazEndsky);
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
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_kaz_endskyAI(creature);
    }
};

/// Razor Guerra <Horde Captain> - 85138
class npc_ashran_razor_guerra : public CreatureScript
{
public:
    npc_ashran_razor_guerra() : CreatureScript("npc_ashran_razor_guerra") { }

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
        MountID = 51048
    };

    struct npc_ashran_razor_guerraAI : public ScriptedAI
    {
        npc_ashran_razor_guerraAI(Creature* creature) : ScriptedAI(creature) { }

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
            me->Mount(0);
            me->SetHomePosition(*me);

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
                l_Ashran->HandleCaptainDeath(CaptainRazorGuerra);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_razor_guerraAI(creature);
    }
};

/// Jared V. Hellstrike <Horde Captain> - 85131
class npc_ashran_jared_v_hellstrike : public CreatureScript
{
public:
    npc_ashran_jared_v_hellstrike() : CreatureScript("npc_ashran_jared_v_hellstrike") { }

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

    struct npc_ashran_jared_v_hellstrikeAI : public ScriptedAI
    {
        npc_ashran_jared_v_hellstrikeAI(Creature* creature) : ScriptedAI(creature) { }

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
                l_Ashran->HandleCaptainDeath(CaptainJaredVHellstrike);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                me->SetWalk(true);
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_jared_v_hellstrikeAI(creature);
    }
};

/// Kimilyn <Forged in Flame> - 88109
class npc_ashran_kimilyn : public CreatureScript
{
public:
    npc_ashran_kimilyn() : CreatureScript("npc_ashran_kimilyn") { }

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

    enum eData
    {
        MountID = 51048
    };

    struct npc_ashran_kimilynAI : public ScriptedAI
    {
        npc_ashran_kimilynAI(Creature* creature) : ScriptedAI(creature)
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
            me->Mount(0);
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
                l_Ashran->HandleCaptainDeath(CaptainKimilyn);
        }

        void UpdateAI(uint32 p_Diff) override
        {
            m_MoveEvent.Update(p_Diff);

            if (m_MoveEvent.ExecuteEvent() == EventMove)
            {
                /// Use same path as Kronus
                me->LoadPath(Kronus);
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
        return new npc_ashran_kimilynAI(creature);
    }
};

/// Speedy Horde Racer - 82903
class npc_ashran_speedy_horde_racer : public CreatureScript
{
public:
    npc_ashran_speedy_horde_racer() : CreatureScript("npc_ashran_speedy_horde_racer") { }

    enum eSpell
    {
        HordeRacer = 166819
    };

    struct npc_ashran_speedy_horde_racerAI : public MS::AI::CosmeticAI
    {
        npc_ashran_speedy_horde_racerAI(Creature* creature) : CosmeticAI(creature), m_MoveIndex{0}
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
            me->CastSpell(me, HordeRacer, true);
            me->ModifyAuraState(AURA_STATE_UNKNOWN22, true);

            m_MoveIndex = 0;

            AddDelayedEvent(500, [this]() -> void
            {
                if (Creature* l_Rider = me->FindNearestCreature(HordeRider, 10.0f))
                    l_Rider->EnterVehicle(me);
            });

            AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(m_MoveIndex, g_HordeRacingMoves[m_MoveIndex]);
            });
        }

        void MovementInform(uint32 type, uint32 /*p_ID*/) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            ++m_MoveIndex;

            if (m_MoveIndex >= HordeRacingMovesCount)
            {
                m_MoveIndex = 0;
                IncreaseLapCount();
            }

            AddDelayedEvent(100, [this]() -> void
            {
                me->GetMotionMaster()->MovePoint(m_MoveIndex, g_HordeRacingMoves[m_MoveIndex]);
            });
        }

        void IncreaseLapCount()
        {
            OutdoorPvP* l_Outdoor = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(me->GetZoneId());
            if (OutdoorPvPAshran* l_Ashran = static_cast<OutdoorPvPAshran*>(l_Outdoor))
                l_Ashran->SetEventData(EventStadiumRacing, TEAM_HORDE, 1);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ashran_speedy_horde_racerAI(creature);
    }
};

void AddSC_AshranNPCHorde()
{
    new npc_jeron_emberfall();
    new npc_ashran_warspear_shaman();
    new npc_ashran_illandria_belore();
    new npc_ashran_examiner_rahm_flameheart();
    new npc_ashran_centurion_firescream();
    new npc_ashran_legionnaire_hellaxe();
    //new npc_ashran_kalgan();
    //new npc_ashran_fura();
    //new npc_ashran_nisstyr();
    new npc_ashran_atomik();
    new npc_ashran_zaram_sunraiser();
    new npc_ashran_horde_gateway_guardian();
    new npc_ashran_kronus();
    new npc_ashran_underpowered_earth_fury();
    new npc_ashran_warspear_gladiator();
    new npc_ashran_excavator_rustshiv();
    new npc_ashran_excavator_hardtooth();
    new npc_ashran_voljins_spear_battle_standard();
    new npc_ashran_warspear_headhunter();
    new npc_ashran_lord_mes();
    new npc_ashran_mindbender_talbadar();
    new npc_ashran_elliott_van_rook();
    new npc_ashran_vanguard_samuelle();
    new npc_ashran_elementalist_novo();
    new npc_ashran_captain_hoodrych();
    new npc_ashran_soulbrewer_nadagast();
    new npc_ashran_necrolord_azael();
    new npc_ashran_rifthunter_yoske();
    new npc_ashran_morriz();
    new npc_ashran_kaz_endsky();
    new npc_ashran_razor_guerra();
    new npc_ashran_jared_v_hellstrike();
    new npc_ashran_kimilyn();
    new npc_ashran_speedy_horde_racer();
}
