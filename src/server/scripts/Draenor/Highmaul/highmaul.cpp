////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "highmaul.hpp"

Position const ghargFirstPos = {3466.11f, 7577.58f, 15.203f, 0.8954f};
Position const ghargSecondPos = {3483.23f, 7598.67f, 10.65f, 0.8954f};
Position const ghargTeleportPos = {3475.60f, 7590.64f, 55.30f, 4.062f};
Position const margokTeleport = {3432.25f, 7536.13f, 73.664f, 0.896154f};
Position const kargathPos = {3444.50f, 7550.76f, 55.39f, 0.90f};
Position const ironWarmasterPos = {4155.636719f, 7817.216309f, 0.253316f, 0.514213f};
Position const ironWarmasterJump = {4182.975098f, 7839.367188f, 7.755508f, 5.603590f};
Position const teleporterSpawnPos = {4186.096f, 8574.492f, 425.353f, 3.851739f};

struct npc_highmaul_gharg_arena_master : public ScriptedAI
{
    enum eMove
    {
        MoveFirstPos,
        MoveSecondPos
    };

    enum eAction
    {
        ActionMove
    };

    npc_highmaul_gharg_arena_master(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->SetWalk(true);
        me->SetReactState(REACT_PASSIVE);
        me->SummonGameObject(ArenaElevator, 3466.438f, 7577.974f, 14.94214f, 0.8901166f, 0.0f, 0.0f, 0.4305113f, 0.9025852f, 1000);
        me->GetMotionMaster()->MovePoint(MoveFirstPos, ghargFirstPos);
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DoAction(int32 const action) override
    {
        if (action == ActionMove)
        {
            if (!m_Instance)
                return;

            me->GetMotionMaster()->MovePoint(MoveSecondPos, ghargSecondPos);
            m_Instance->SetData(ElevatorActivated, true);
        }
    }

    void sGossipSelect(Player* player, uint32 /*sender*/, uint32 /*action*/) override
    {
        if (!m_Instance)
            return;

        if (m_Instance->GetData(ElevatorActivated))
            player->NearTeleportTo(ghargTeleportPos);
        else
        {
            me->GetMotionMaster()->MovePoint(MoveSecondPos, ghargSecondPos);
            m_Instance->SetData(ElevatorActivated, true);
        }

        player->PlayerTalkClass->SendCloseGossip();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (!m_Instance || type != MovementGeneratorType::POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case MoveFirstPos:
            {
                me->SetFacingTo(0.8954f);
                me->SetHomePosition(ghargSecondPos);
                break;
            }
            case MoveSecondPos:
            {
                me->SetHomePosition(ghargSecondPos);

                /// Start elevator
                //if (GameObject* l_Elevator = GameObject::GetGameObject(*me, m_Instance->GetGuidData(ArenaElevator)))
                //    l_Elevator->SetTransportState(GO_STATE_TRANSPORT_STOPPED);

                if (GameObject* l_Wall = GameObject::GetGameObject(*me, m_Instance->GetGuidData(CollisionWall)))
                    l_Wall->SetGoState(GO_STATE_READY);
                break;
            }
            default:
                break;
        }
    }
};

struct npc_highmaul_jhorn_the_mad : public MS::AI::CosmeticAI
{
    enum eTalks
    {
        Intro1,
        Intro2,
        Intro3,
        Intro4,
        Intro5,
        Trash1,
        Trash2,
        Kargath1,
        Kargath2
    };

    enum eActions
    {
        StartIntro,
        ContinueIntro,
        VulgorDied
    };

    npc_highmaul_jhorn_the_mad(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case StartIntro:
            {
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(Intro1); });
                AddDelayedEvent(16 * IN_MILLISECONDS, [this]() -> void { Talk(Intro2); });
                AddDelayedEvent(38 * IN_MILLISECONDS, [this]() -> void { Talk(Intro3); });

                AddDelayedEvent(54 * IN_MILLISECONDS, [this]() -> void
                {
                    if (m_Instance)
                    {
                        if (GameObject* l_InnerGate = GameObject::GetGameObject(*me, m_Instance->GetGuidData(GateArenaInner)))
                            l_InnerGate->SetGoState(GO_STATE_ACTIVE);
                    }
                });

                AddDelayedEvent(55 * IN_MILLISECONDS, [this]() -> void
                {
                    Talk(Intro4);

                    if (m_Instance)
                    {
                        if (Creature* l_Vulgor = m_Instance->GetCreature(Vulgor))
                            l_Vulgor->AI()->DoAction(StartIntro);
                    }
                });

                AddDelayedEvent(59 * IN_MILLISECONDS, [this]() -> void
                {
                    if (m_Instance)
                    {
                        if (GameObject* l_InnerGate = GameObject::GetGameObject(*me, m_Instance->GetGuidData(GateArenaInner)))
                            l_InnerGate->SetGoState(GO_STATE_READY);
                    }
                });

                AddDelayedEvent(70 * IN_MILLISECONDS, [this]() -> void { Talk(Intro5); });
                break;
            }
            case ContinueIntro:
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(Trash1); });
                AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void { Talk(Trash2); });
                break;
            case VulgorDied:
            {
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(Kargath1); });

                AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void
                {
                    if (Creature* kargath = m_Instance->GetCreature((KargathBladefist)))
                        kargath->AI()->DoAction(VulgorDied);
                });

                AddDelayedEvent(20 * IN_MILLISECONDS, [this]() -> void { Talk(Kargath2); });

                AddDelayedEvent(21 * IN_MILLISECONDS, [this]() -> void
                {
                    if (Creature* kargath = m_Instance->GetCreature((KargathBladefist)))
                        kargath->SetFacingTo(4.02f);
                });

                break;
            }
            default:
                break;
        }
    }
};

struct npc_highmaul_thoktar_ironskull : public MS::AI::CosmeticAI
{
    enum eTalks
    {
        Intro1,
        Intro2,
        Intro3,
        Trash1,
        Trash2,
        Kargath1
    };

    enum eActions
    {
        StartIntro,
        ContinueIntro,
        VulgorDied
    };

    npc_highmaul_thoktar_ironskull(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case StartIntro:
                AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void { Talk(Intro1); });
                AddDelayedEvent(31 * IN_MILLISECONDS, [this]() -> void { Talk(Intro2); });
                AddDelayedEvent(50 * IN_MILLISECONDS, [this]() -> void { Talk(Intro3); });
                break;
            case ContinueIntro:
                AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void { Talk(Trash1); });
                AddDelayedEvent(17 * IN_MILLISECONDS, [this]() -> void { Talk(Trash2); });
                break;
            case VulgorDied:
                AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void { Talk(Kargath1); });
                break;
            default:
                break;
        }
    }
};


struct npc_highmaul_imperator_margok : public MS::AI::CosmeticAI
{
    enum eTalks
    {
        SorckingEvent12,
        SorckingEvent13
    };

    enum eActions
    {
        VulgorDied = 2,
        KargathLastTalk
    };

    enum eMove
    {
        MoveFrontGate = 1
    };

    enum eSpells
    {
        TeleportIntoArena = 167048,
        TeleportVisual = 167050,
        SitThrone = 88648
    };

    npc_highmaul_imperator_margok(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case VulgorDied:
            {
                AddDelayedEvent(19 * IN_MILLISECONDS, [this]() -> void
                {
                    me->CastSpell(me, TeleportIntoArena, true);
                    me->NearTeleportTo(margokTeleport);
                    me->CastSpell(me, TeleportVisual, true);
                });

                AddDelayedEvent(20 * IN_MILLISECONDS, [this]() -> void
                {
                    me->SetFacingTo(margokTeleport.m_orientation);
                    me->RemoveAura(TeleportIntoArena);
                });

                AddDelayedEvent(28 * IN_MILLISECONDS, [this]() -> void { Talk(SorckingEvent12); });

                AddDelayedEvent(49 * IN_MILLISECONDS, [this]() -> void
                {
                    if (!m_Instance)
                        return;

                    if (Creature* kargath = m_Instance->GetCreature((KargathBladefist)))
                    {
                        kargath->SetWalk(true);
                        kargath->GetMotionMaster()->MovePoint(MoveFrontGate, kargathPos);
                    }
                });

                AddDelayedEvent(39 * IN_MILLISECONDS, [this]() -> void
                {
                    if (!m_Instance)
                        return;

                    if (Creature* kargath = m_Instance->GetCreature((KargathBladefist)))
                        kargath->AI()->DoAction(KargathLastTalk);
                });

                AddDelayedEvent(51 * IN_MILLISECONDS, [this]() -> void { Talk(SorckingEvent13); });
                AddDelayedEvent(52 * IN_MILLISECONDS, [this]() -> void { me->CastSpell(me, SitThrone, true); });
                break;
            }
            default:
                break;
        }
    }
};

struct npc_highmaul_gorian_guardsman : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellBloodyCleave = 166767,

        ChainGripSearcher = 151990,
        ChainGripAura = 152024,
        ViciousSlash = 152043,

        SpellStaggeringBlow = 166779
    };

    enum eEvents
    {
        EventBloodyCleave = 1,
        EventChainGrip,
        EventStaggeringBlow
    };

    npc_highmaul_gorian_guardsman(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventBloodyCleave, urand(4000, 7000));
        m_Events.RescheduleEvent(EventChainGrip, urand(3000, 8000));
        m_Events.RescheduleEvent(EventStaggeringBlow, urand(6000, 10000));
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case ChainGripSearcher:
            {
                ObjectGuid guid = target->GetGUID();
                me->CastSpell(target, ChainGripAura, true);

                AddDelayedEvent(1 * IN_MILLISECONDS, [this, guid]() -> void
                {
                    if (Unit* target = Unit::GetUnit(*me, guid))
                        me->CastSpell(target, ViciousSlash, true);
                });

                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventBloodyCleave:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellBloodyCleave, true);
                m_Events.RescheduleEvent(EventBloodyCleave, urand(10000, 15000));
                break;
            case EventChainGrip:
                me->CastSpell(me, ChainGripSearcher, true);
                m_Events.RescheduleEvent(EventChainGrip, urand(8000, 12000));
                break;
            case EventStaggeringBlow:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellStaggeringBlow, true);
                m_Events.RescheduleEvent(EventStaggeringBlow, urand(15000, 20000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_night_twisted_devout : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellTaintedClaws = 175601,

        SpellDevouringLeap = 175598,
        SpellDevour = 175599
    };

    enum eEvents
    {
        EventTaintedClaws = 1,
        EventDevour
    };

    npc_highmaul_night_twisted_devout(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC) || me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            me->SetReactState(REACT_PASSIVE);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        if (me->HasReactState(REACT_PASSIVE))
        {
            EnterEvadeMode();
            return;
        }

        m_Events.RescheduleEvent(EventTaintedClaws, urand(6000, 9000));
        m_Events.RescheduleEvent(EventDevour, urand(8000, 10000));

        if (Creature* l_IronGrunt = me->FindNearestCreature(IronGrunt, 3.0f))
        {
            me->Kill(l_IronGrunt);
            DoZoneInCombat(me, 40.0f);
        }

        me->SetAnimKitId(0);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventTaintedClaws:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellTaintedClaws, true);
                m_Events.RescheduleEvent(EventTaintedClaws, urand(8000, 11000));
                break;
            }
            case EventDevour:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                {
                    me->CastSpell(target, SpellDevouringLeap, true);

                    ObjectGuid guid = target->GetGUID();
                    AddDelayedEvent(2 * IN_MILLISECONDS, [this, guid]() -> void
                    {
                        if (Unit* target = Unit::GetUnit(*me, guid))
                            me->CastSpell(target, SpellDevour, false);
                    });
                }

                m_Events.RescheduleEvent(EventDevour, urand(8000, 11000));
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_runemaster : public ScriptedAI
{
    enum eSpells
    {
        SpellRuneOfDestruction = 175636,

        SpellRuneOfDisintegration = 175648,

        SpellRuneOfUnmaking = 175899
    };

    enum eEvents
    {
        EventRuneOfDestruction = 1,
        EventRuneOfDisintegration,
        EventRuneOfUnmaking
    };

    npc_highmaul_gorian_runemaster(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventRuneOfDestruction, urand(6000, 9000));
        m_Events.RescheduleEvent(EventRuneOfDisintegration, urand(9000, 12000));
        m_Events.RescheduleEvent(EventRuneOfUnmaking, urand(4000, 6000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventRuneOfDestruction:
                me->CastSpell(me, SpellRuneOfDestruction, false);
                m_Events.RescheduleEvent(EventRuneOfDestruction, urand(9000, 12000));
                break;
            case EventRuneOfDisintegration:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SpellRuneOfDisintegration, false);
                m_Events.RescheduleEvent(EventRuneOfDisintegration, urand(12000, 15000));
                break;
            case EventRuneOfUnmaking:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellRuneOfUnmaking, false);
                m_Events.RescheduleEvent(EventRuneOfUnmaking, urand(6000, 9000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_enforcer : public ScriptedAI
{
    enum eSpell
    {
        SpellMeatGrinder = 175665
    };

    enum eEvent
    {
        EventMeatGrinder = 1
    };

    npc_highmaul_gorian_enforcer(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventMeatGrinder, urand(6000, 8000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventMeatGrinder:
                me->CastSpell(me, SpellMeatGrinder, false);
                m_Events.RescheduleEvent(EventMeatGrinder, urand(12000, 15000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_underbelly_vagrant : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellLooting = 175673,
        SpellPilfer = 175715
    };

    enum eEvent
    {
        EventPilfer = 1
    };

    npc_highmaul_underbelly_vagrant(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { me->CastSpell(me, SpellLooting, false); });
    }

    void LastOperationCalled() override
    {
        AddDelayedEvent(16 * IN_MILLISECONDS, [this]() -> void { me->CastSpell(me, SpellLooting, false); });
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventPilfer, urand(2000, 4000));
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventPilfer:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SpellPilfer, false);
                m_Events.RescheduleEvent(EventPilfer, urand(2000, 4000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_sorcerer : public ScriptedAI
{
    enum eSpells
    {
        SpellArcaneForce = 175848,

        SpellArcaneBolt = 175879
    };

    enum eEvents
    {
        EventArcaneForce = 1,
        EventArcaneBolt
    };

    npc_highmaul_gorian_sorcerer(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventArcaneForce, urand(6000, 8000));
        m_Events.RescheduleEvent(EventArcaneBolt, urand(7000, 10000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventArcaneForce:
                me->CastSpell(me, SpellArcaneForce, false);
                m_Events.RescheduleEvent(EventArcaneForce, urand(20000, 25000));
                break;
            case EventArcaneBolt:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellArcaneBolt, false);
                m_Events.RescheduleEvent(EventArcaneBolt, urand(7000, 10000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};


struct npc_highmaul_night_twisted_brute : public ScriptedAI
{
    enum eSpell
    {
        SpellSurgeOfDarkness = 175763
    };

    enum eEvent
    {
        EventSurgeOfDarkness = 1
    };

    npc_highmaul_night_twisted_brute(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* attacker) override
    {
        m_Events.RescheduleEvent(EventSurgeOfDarkness, urand(8000, 12000));

        std::list<Creature*> l_IronGrunts;
        me->GetCreatureListWithEntryInGrid(l_IronGrunts, IronGrunt, 35.0f);

        if (!l_IronGrunts.empty())
        {
            std::list<Unit*> l_Allies;

            Trinity::AnyFriendlyUnitInObjectRangeCheck l_Check(me, me, 50.0f);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> l_Searcher(me, l_Allies, l_Check);
            me->VisitNearbyObject(50.0f, l_Searcher);

            for (Unit* l_Unit : l_Allies)
            {
                if (l_Unit->ToCreature() != nullptr && l_Unit->ToCreature()->AI())
                    l_Unit->ToCreature()->AI()->AttackStart(attacker);
            }
        }

        for (Creature* creature : l_IronGrunts)
            me->Kill(creature);

        me->SetAnimKitId(0);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventSurgeOfDarkness:
                me->CastSpell(me, SpellSurgeOfDarkness, false);
                m_Events.RescheduleEvent(EventSurgeOfDarkness, urand(8000, 11000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_night_twisted_soothsayer : public ScriptedAI
{
    enum eSpells
    {
        SpellVoidStorm = 167039,

        SpellVoidBolt = 175876
    };

    enum eEvents
    {
        EventVoidStorm = 1,
        EventVoidBolt
    };

    npc_highmaul_night_twisted_soothsayer(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventVoidStorm, urand(6000, 9000));
        m_Events.RescheduleEvent(EventVoidBolt, urand(4000, 7000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventVoidStorm:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SpellVoidStorm, false);
                m_Events.RescheduleEvent(EventVoidStorm, urand(9000, 12000));
                break;
            case EventVoidBolt:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, SpellVoidBolt, false);
                m_Events.RescheduleEvent(EventVoidBolt, urand(8000, 11000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_void_aberration : public ScriptedAI
{
    enum eSpell
    {
        SpellVoidSoul = 175816
    };

    enum eEvent
    {
        EventVoidSoul = 1
    };

    npc_highmaul_void_aberration(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventVoidSoul, urand(8000, 12000));
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventVoidSoul:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, SpellVoidSoul, true);
                m_Events.RescheduleEvent(EventVoidSoul, urand(8000, 11000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_krush : public ScriptedAI
{
    enum eSpells
    {
        SpellBoarsRushSearcher = 166224,
        SpellBoarsRushMissile = 166226
    };

    enum eEvent
    {
        EventBoarsRush = 1
    };

    npc_highmaul_krush(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventBoarsRush, 5000);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case SpellBoarsRushSearcher:
                me->SetFacingToObject(target);
                me->CastSpell(target, SpellBoarsRushMissile, true);
                break;
            default:
                break;
        }
    }

    void JustReachedHome() override
    {
        me->GetMotionMaster()->MoveIdle();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventBoarsRush:
                me->CastSpell(me, SpellBoarsRushSearcher, true);
                me->AddUnitState(UNIT_STATE_ROOT);
                m_Events.RescheduleEvent(EventBoarsRush, 25000);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_iron_flame_technician : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellCorruptedBlood = 174475,
        SpellFlamethrower = 173231,

        SpellUnstoppableChargeSearcher = 174462,
        UnstoppableChargeCharge = 174461,
        UnstoppableChargeDmg = 174465
    };

    enum eEvents
    {
        EventCorruptedBlood = 1,
        EventFlamethrower,
        EventUnstoppableCharge
    };

    enum eCreature
    {
        UnstoppableCharge = 87230
    };

    enum eAction
    {
        DoIntro
    };

    npc_highmaul_iron_flame_technician(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_IsCosmetic = false;
    }

    EventMap m_Events;
    EventMap m_CosmeticEvent;
    ObjectGuid m_ChargeTarget;
    bool m_IsCosmetic;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            if (Creature* boss = me->FindNearestCreature(Brackenspore, 50.0f))
            {
                m_IsCosmetic = true;
                m_CosmeticEvent.RescheduleEvent(EventFlamethrower, urand(4000, 7000));
            }
        });
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        if (m_IsCosmetic)
            return;

        m_Events.RescheduleEvent(EventCorruptedBlood, urand(6000, 9000));
        m_Events.RescheduleEvent(EventFlamethrower, urand(4000, 7000));
        m_Events.RescheduleEvent(EventUnstoppableCharge, urand(9000, 12000));
    }

    void DoAction(int32 const action) override
    {
        if (action == DoIntro)
        {
            m_Events.Reset();
            m_CosmeticEvent.Reset();
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case SpellUnstoppableChargeSearcher:
            {
                if (Creature* l_Charge = me->SummonCreature(UnstoppableCharge, *target, TEMPSUMMON_TIMED_DESPAWN, 2000))
                {
                    l_Charge->SetReactState(REACT_PASSIVE);
                    l_Charge->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);

                    m_ChargeTarget = l_Charge->GetGUID();
                    me->CastSpell(l_Charge, UnstoppableChargeCharge, true);
                    me->CastSpell(l_Charge, UnstoppableChargeDmg, true);
                }
                break;
            }
            default:
                break;
        }
    }

    ObjectGuid GetGUID(int32 /*id*/) override
    {
        return m_ChargeTarget;
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != MovementGeneratorType::POINT_MOTION_TYPE)
            return;

        if (id == 0)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!me->HasUnitState(UNIT_STATE_CASTING))
        {
            m_CosmeticEvent.Update(diff);

            if (m_CosmeticEvent.ExecuteEvent() == EventFlamethrower)
            {
                me->CastSpell(me, SpellFlamethrower, false);
                m_CosmeticEvent.RescheduleEvent(EventFlamethrower, urand(4000, 7000));
            }
        }

        if (!UpdateVictim() || m_IsCosmetic)
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventCorruptedBlood:
                me->CastSpell(me, SpellCorruptedBlood, false);
                m_Events.RescheduleEvent(EventCorruptedBlood, urand(13000, 16000));
                break;
            case EventFlamethrower:
                me->CastSpell(me, SpellFlamethrower, false);
                m_Events.RescheduleEvent(EventFlamethrower, urand(14000, 17000));
                break;
            case EventUnstoppableCharge:
                me->CastSpell(me, SpellUnstoppableChargeSearcher, false);
                m_Events.RescheduleEvent(EventUnstoppableCharge, urand(19000, 22000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_iron_warmaster : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellIronBattleRage = 173238,
        SpellCorruptedBlood = 174475
    };

    enum eEvents
    {
        EventIronBattleRage = 1,
        EventCorruptedBlood
    };

    enum eTalks
    {
        Intro1,
        Intro2
    };

    enum eAction
    {
        DoIntro
    };

    npc_highmaul_iron_warmaster(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_IsCosmetic = false;
        m_IntroDone = false;
    }

    EventMap m_Events;

    /// For Brackenspore event
    bool m_IsCosmetic;
    bool m_IntroDone;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            if (Creature* boss = me->FindNearestCreature(Brackenspore, 50.0f))
                m_IsCosmetic = true;
        });
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        if (m_IsCosmetic)
            return;

        m_Events.RescheduleEvent(EventIronBattleRage, urand(6000, 9000));
        m_Events.RescheduleEvent(EventCorruptedBlood, urand(6000, 9000));
    }

    void DoAction(int32 const action) override
    {
        if (action == DoIntro && !m_IntroDone)
        {
            m_IntroDone = true;
            AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { Talk(Intro1); });

            AddDelayedEvent(6 * IN_MILLISECONDS, [this]() -> void
            {
                Talk(Intro2);

                me->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
                me->GetMotionMaster()->MovePoint(0, ironWarmasterPos);
            });
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != MovementGeneratorType::POINT_MOTION_TYPE)
            return;

        if (id == 0)
            me->GetMotionMaster()->MoveJump(ironWarmasterJump, 10.0f, 30.0f, 1);
        else
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim() || m_IsCosmetic)
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventIronBattleRage:
                me->CastSpell(me, SpellIronBattleRage, true);
                m_Events.RescheduleEvent(EventIronBattleRage, urand(12000, 15000));
                break;
            case EventCorruptedBlood:
                me->CastSpell(me, SpellCorruptedBlood, false);
                m_Events.RescheduleEvent(EventCorruptedBlood, urand(13000, 16000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_iron_blood_mage : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        SpellCorruptedBlood = 174475,
        SpellBloodBolt = 174574,
        SpellCorruptedBloodShield = 174474
    };

    enum eEvents
    {
        EventCorruptedBlood = 1,
        EventBloodBolt,
        EventCorruptedBloodShield,
        EventCheckForIntro
    };

    enum eCreatures
    {
        IronFlameTechnician = 86607,
        IronWarmaster = 86609
    };

    enum eAction
    {
        DoIntro
    };

    npc_highmaul_iron_blood_mage(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            std::list<Creature*> creatureList;
            me->GetCreatureListInGrid(creatureList, 30.0f);

            if (creatureList.empty())
                return;

            creatureList.remove_if([this](Creature* creature) -> bool
            {
                if (creature == nullptr)
                    return true;

                if (creature->GetEntry() != me->GetEntry() && creature->GetEntry() != IronFlameTechnician && creature->GetEntry() != IronWarmaster)
                    return true;

                return false;
            });

            uint8 l_Count = 0;
            for (Creature* creature : creatureList)
            {
                if (Creature* boss = m_Instance->GetCreature((Brackenspore)))
                    if (boss->GetAI())
                        boss->AI()->SetGUID(creature->GetGUID(), l_Count);
            }
        });
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventCorruptedBlood, urand(6000, 9000));
        m_Events.RescheduleEvent(EventBloodBolt, urand(4000, 7000));
        m_Events.RescheduleEvent(EventCorruptedBloodShield, urand(8000, 11000));
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventCorruptedBlood:
                me->CastSpell(me, SpellCorruptedBlood, false);
                m_Events.RescheduleEvent(EventCorruptedBlood, urand(13000, 16000));
                break;
            case EventBloodBolt:
                me->CastSpell(me, SpellBloodBolt, false);
                m_Events.RescheduleEvent(EventBloodBolt, urand(8000, 11000));
                break;
            case EventCorruptedBloodShield:
                if (Unit* target = me->SelectNearbyAlly(me, 15.0f, true))
                    me->CastSpell(target, SpellCorruptedBloodShield, false);
                m_Events.RescheduleEvent(EventCorruptedBloodShield, urand(15000, 18000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_night_twisted_ritualist : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        VoidChannel = 170677,
        VoidTouch = 175581
    };

    enum eEvent
    {
        EventVoidTouch = 1
    };

    enum eCreature
    {
        GreaterAberration = 85246
    };

    enum eAction
    {
        RitualistDied
    };

    npc_highmaul_night_twisted_ritualist(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    ObjectGuid m_Aberration;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            if (Creature* l_Aberration = me->FindNearestCreature(GreaterAberration, 20.0f))
                m_Aberration = l_Aberration->GetGUID();
        });

        me->CastSpell(me, VoidChannel, true);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        me->RemoveAura(VoidChannel);

        m_Events.RescheduleEvent(EventVoidTouch, urand(3000, 6000));
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* l_Aberration = Creature::GetCreature(*me, m_Aberration))
            l_Aberration->AI()->DoAction(RitualistDied);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventVoidTouch:
                me->CastSpell(me, VoidTouch, false);
                m_Events.RescheduleEvent(EventVoidTouch, urand(6000, 9000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_greater_void_aberration : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        VoidCommunion = 175539,
        CallOfTheVoid = 175589
    };

    enum eEvent
    {
        EventCallOfTheVoid = 1
    };

    enum eCreature
    {
        NightTwistedRitualist = 85245
    };

    enum eAction
    {
        RitualistDied
    };

    npc_highmaul_greater_void_aberration(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
        m_Ritualists = 0;
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    uint32 m_Ritualists;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            std::list<Creature*> l_RitualistList;
            me->GetCreatureListWithEntryInGrid(l_RitualistList, NightTwistedRitualist, 20.0f);
            m_Ritualists = (uint32)l_RitualistList.size();

            if (m_Ritualists)
                me->CastSpell(me, VoidCommunion, true);
        });
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventCallOfTheVoid, urand(6000, 9000));
    }

    void DoAction(int32 const action) override
    {
        if (action == RitualistDied && m_Ritualists)
        {
            --m_Ritualists;

            if (!m_Ritualists)
                me->RemoveAura(VoidCommunion);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventCallOfTheVoid:
                me->CastSpell(me, CallOfTheVoid, false);
                m_Events.RescheduleEvent(EventCallOfTheVoid, urand(9000, 12000));
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_highmaul_conscript : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        RendingSlash = 166185,
        ShieldBlocking = 166177,
        ShieldCharge = 166178,
        AtArms = 157739
    };

    enum eEvents
    {
        EventRendingSlash = 1,
        EventShieldBlocking,
        EventShieldCharge
    };

    npc_highmaul_highmaul_conscript(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    ObjectGuid m_ChargeTarget;

    void Reset() override
    {
        m_Events.Reset();

        me->ClearUnitState(UNIT_STATE_ROOT);
        me->CastSpell(me, AtArms, true);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventRendingSlash, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventShieldBlocking, 13 * IN_MILLISECONDS);

        me->RemoveAura(AtArms);
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id != EVENT_CHARGE)
            return;

        me->RemoveAura(AtArms);
        me->RemoveAura(ShieldCharge);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* l_Phemos = m_Instance->GetCreature((Phemos)))
        {
            if (l_Phemos->IsAIEnabled)
            {
                l_Phemos->AI()->SetGUID(me->GetGUID(), 0);
                l_Phemos->AI()->DoAction(0);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventRendingSlash:
            {
                me->CastSpell(me, RendingSlash, false);
                m_Events.RescheduleEvent(EventRendingSlash, 20 * IN_MILLISECONDS);
                break;
            }
            case EventShieldBlocking:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                {
                    m_ChargeTarget = target->GetGUID();
                    me->SetFacingTo(me->GetAngle(target));
                    me->CastSpell(target, ShieldBlocking, false);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                    me->AddUnitState(UNIT_STATE_ROOT);
                }

                AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
                {
                    me->CastSpell(me, AtArms, true);
                });

                m_Events.RescheduleEvent(EventShieldCharge, 6 * IN_MILLISECONDS);
                m_Events.RescheduleEvent(EventShieldBlocking, 60 * IN_MILLISECONDS);
                break;
            }
            case EventShieldCharge:
            {
                if (Unit* target = Unit::GetUnit(*me, m_ChargeTarget))
                {
                    float o = me->GetOrientation();
                    Position l_Pos;

                    me->GetContactPoint(target, l_Pos.m_positionX, l_Pos.m_positionY, l_Pos.m_positionZ);
                    target->GetFirstCollisionPosition(l_Pos, target->GetObjectSize(), o);
                    me->ClearUnitState(UNIT_STATE_ROOT);
                    me->GetMotionMaster()->MoveCharge(l_Pos.m_positionX, l_Pos.m_positionY, l_Pos.m_positionZ + target->GetObjectSize());

                    me->CastSpell(me, ShieldCharge, true);
                }

                break;
            }
            default:
                break;
        }

        if (!me->HasAura(ShieldBlocking))
            DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_ogron_earthshaker : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        IntimidatingRoarJump = 166170,
        EarthdevastatingSlam = 166174,
        EarthdevastatingSlamDmg = 166175
    };

    enum eEvents
    {
        EventIntimidatingRoar = 1,
        EventEarthdevastatingSlam
    };

    enum eAction
    {
        ActionSlam
    };

    npc_highmaul_ogron_earthshaker(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    float m_Orientation;
    uint8 m_SlamCount;

    void Reset() override
    {
        m_Events.Reset();

        m_Orientation = 0.0f;
        m_SlamCount = 0;
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventIntimidatingRoar, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventEarthdevastatingSlam, 17 * IN_MILLISECONDS);
    }

    void DoAction(int32 const action) override
    {
        if (action == ActionSlam)
        {
            me->SetFacingTo(m_Orientation);
            me->CastSpell(me, EarthdevastatingSlamDmg, false);

            m_Orientation += M_PI / 3;
            ++m_SlamCount;

            if (m_SlamCount >= 6)
            {
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                });
            }
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* l_Phemos = m_Instance->GetCreature((Phemos)))
        {
            if (l_Phemos->IsAIEnabled)
            {
                l_Phemos->AI()->SetGUID(me->GetGUID(), 0);
                l_Phemos->AI()->DoAction(0);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventIntimidatingRoar:
            {
                me->CastSpell(me, IntimidatingRoarJump, true);
                m_Events.RescheduleEvent(EventIntimidatingRoar, 25 * IN_MILLISECONDS);
                break;
            }
            case EventEarthdevastatingSlam:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 15.0f))
                {
                    m_Orientation = me->GetAngle(target);
                    m_SlamCount = 0;
                    me->SetFacingTo(m_Orientation);
                    me->CastSpell(target, EarthdevastatingSlam, true);
                    me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
                }

                m_Events.RescheduleEvent(EventEarthdevastatingSlam, 60 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        if (!me->HasAura(EarthdevastatingSlam))
            DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_arcanist : public ScriptedAI
{
    enum eSpells
    {
        ArcaneForceCosmetic = 166289,

        ArcaneBolt = 166204,
        ArcaneVolatility = 166199,
        ArcaneVolatilityAura = 166200,
        ArcaneVolatilityDmg = 166202,
        ArcaneVolatilityBump = 166201,
        ArcaneBarrage = 178023
    };

    enum eEvents
    {
        EventArcaneBolt = 1,
        EventArcaneVolatility,
        EventArcaneBarrage
    };

    enum eCreature
    {
        InvisibleStalker = 15214
    };

    npc_highmaul_gorian_arcanist(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        if (Creature* stalker = me->FindNearestCreature(InvisibleStalker, 20.0f))
            me->CastSpell(stalker, ArcaneForceCosmetic, false);

        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventArcaneBolt, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventArcaneVolatility, 10 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventArcaneBarrage, 8 * IN_MILLISECONDS);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case ArcaneVolatility:
                me->CastSpell(target, ArcaneVolatilityAura, true);
                break;
            case ArcaneVolatilityDmg:
                me->CastSpell(target, ArcaneVolatilityBump, true);
                break;
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* l_Phemos = m_Instance->GetCreature((Phemos)))
        {
            if (l_Phemos->IsAIEnabled)
            {
                l_Phemos->AI()->SetGUID(me->GetGUID(), 0);
                l_Phemos->AI()->DoAction(0);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventArcaneBolt:
                me->CastSpell(me, ArcaneBolt, false);
                m_Events.RescheduleEvent(EventArcaneBolt, 28 * IN_MILLISECONDS);
                break;
            case EventArcaneVolatility:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, ArcaneVolatility, false);
                m_Events.RescheduleEvent(EventArcaneVolatility, 35 * IN_MILLISECONDS);
                break;
            case EventArcaneBarrage:
                me->CastSpell(me, ArcaneBarrage, false);
                m_Events.RescheduleEvent(EventArcaneBarrage, 30 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_ogron_brute : public ScriptedAI
{
    enum eSpell
    {
        Decimate = 166189
    };

    enum eEvent
    {
        EventDecimate = 1
    };

    npc_highmaul_ogron_brute(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventDecimate, 6 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* l_Phemos = m_Instance->GetCreature((Phemos)))
        {
            if (l_Phemos->IsAIEnabled)
            {
                l_Phemos->AI()->SetGUID(me->GetGUID(), 0);
                l_Phemos->AI()->DoAction(0);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventDecimate:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, Decimate, false);
                m_Events.RescheduleEvent(EventDecimate, 15 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_rune_mender : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        AttackPale = 175491,
        Ready2HL = 175155
    };

    npc_highmaul_gorian_rune_mender(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        me->RemoveAura(AttackPale);
        me->CastSpell(me, Ready2HL, true);

        ScheduleNextOperation();
    }

    void LastOperationCalled() override
    {
        ScheduleNextOperation();
    }

    void ScheduleNextOperation()
    {
        AddDelayedEvent(urand(2 * IN_MILLISECONDS, 10 * IN_MILLISECONDS), [this]() -> void
        {
            if (urand(0, 1))
            {
                me->RemoveAura(Ready2HL);
                me->CastSpell(me, AttackPale, true);
            }
            else
            {
                me->RemoveAura(AttackPale);
                me->CastSpell(me, Ready2HL, true);
            }
        });
    }
};

struct npc_highmaul_night_twisted_fanatic : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        Sacrifice = 175430,
        FullHeal = 17683,
        ClearAllDebuffs = 34098,
        FadeOut = 166930
    };

    enum eSteps
    {
        StepSacrifice,
        StepHeal,
        StepReturn
    };

    npc_highmaul_night_twisted_fanatic(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_CosmeticStep = eSteps::StepSacrifice;
    }

    uint8 m_CosmeticStep;

    void Reset() override
    {
        ScheduleNextOperation();
    }

    void LastOperationCalled() override
    {
        ScheduleNextOperation();
    }

    void ScheduleNextOperation()
    {
        uint32 time = 0;
        switch (m_CosmeticStep)
        {
            default:
            case eSteps::StepSacrifice:
                time = urand(5 * IN_MILLISECONDS, 10 * IN_MILLISECONDS);
                break;
            case eSteps::StepHeal:
            case eSteps::StepReturn:
                time = 4 * IN_MILLISECONDS;
                break;
        }

        AddDelayedEvent(time, [this]() -> void
        {
            switch (m_CosmeticStep)
            {
                default:
                case eSteps::StepSacrifice:
                {
                    m_CosmeticStep = eSteps::StepHeal;
                    me->CastSpell(me, Sacrifice, false);
                    break;
                }
                case eSteps::StepHeal:
                {
                    m_CosmeticStep = eSteps::StepReturn;
                    me->CastSpell(me, FullHeal, true);
                    me->CastSpell(me, ClearAllDebuffs, true);
                    me->CastSpell(me, FadeOut, true);
                    break;
                }
                case eSteps::StepReturn:
                {
                    m_CosmeticStep = eSteps::StepSacrifice;
                    me->RemoveAura(Sacrifice);
                    me->RemoveAura(FadeOut);
                    break;
                }
            }
        });
    }
};

struct npc_highmaul_gorian_warden : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        AttackPale = 175491,
        Ready1H = 175387
    };

    npc_highmaul_gorian_warden(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        me->RemoveAura(AttackPale);
        me->CastSpell(me, Ready1H, true);

        ScheduleNextOperation();
    }

    void LastOperationCalled() override
    {
        ScheduleNextOperation();
    }

    void ScheduleNextOperation()
    {
        AddDelayedEvent(5 * IN_MILLISECONDS, [this]() -> void
        {
            if (urand(0, 1))
            {
                me->RemoveAura(Ready1H);
                me->CastSpell(me, AttackPale, true);
            }
            else
            {
                me->RemoveAura(AttackPale);
                me->CastSpell(me, Ready1H, true);
            }
        });
    }
};


struct npc_highmaul_ogron_warbringer : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        AttackPale = 175505,
        ReadyUnarmed = 173366
    };

    npc_highmaul_ogron_warbringer(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        me->RemoveAura(AttackPale);
        me->CastSpell(me, ReadyUnarmed, true);

        ScheduleNextOperation();
    }

    void LastOperationCalled() override
    {
        ScheduleNextOperation();
    }

    void ScheduleNextOperation()
    {
        AddDelayedEvent(5 * IN_MILLISECONDS, [this]() -> void
        {
            if (urand(0, 1))
            {
                me->RemoveAura(ReadyUnarmed);
                me->CastSpell(me, AttackPale, true);
            }
            else
            {
                me->RemoveAura(AttackPale);
                me->CastSpell(me, ReadyUnarmed, true);
            }
        });
    }
};

struct npc_highmaul_warden_thultok : public ScriptedAI
{
    enum eSpells
    {
        DestructiveForce = 175061,
        ArcaneResidue = 175047
    };

    enum eEvents
    {
        EventDestructiveForce = 1,
        EventArcaneResidue
    };

    npc_highmaul_warden_thultok(Creature* creature) : ScriptedAI(creature)
    {
        if (!creature->isAlive())
            me->SummonGameObject(Teleporter, teleporterSpawnPos, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->SummonGameObject(Teleporter, teleporterSpawnPos, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventDestructiveForce, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventArcaneResidue, 10 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventDestructiveForce:
                me->CastSpell(me, DestructiveForce, false);
                m_Events.RescheduleEvent(EventDestructiveForce, 10 * IN_MILLISECONDS);
                break;
            case EventArcaneResidue:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, ArcaneResidue, true);
                m_Events.RescheduleEvent(EventArcaneResidue, 10 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_royal_guardsman : public ScriptedAI
{
    enum eSpells
    {
        /// Pulverize
        PulverizeAura = 174445,
        PulverizeStack = 174446,
        PulverizedStun = 174452,
        /// Rampage
        RampageSearcher = 174468,
        RampageAura = 174469
    };

    enum eEvent
    {
        EventRampage = 1
    };

    npc_highmaul_gorian_royal_guardsman(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        me->CastSpell(me, PulverizeAura, true);

        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventRampage, 10 * IN_MILLISECONDS);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case PulverizeStack:
            {
                if (Aura* aura = target->GetAura(spellInfo->Id, me->GetGUID()))
                {
                    if (aura->GetStackAmount() >= 8)
                    {
                        target->CastSpell(target, PulverizedStun, true);
                        target->RemoveAura(spellInfo->Id);
                    }
                }

                break;
            }
            case RampageSearcher:
            {
                me->CastSpell(me, RampageAura, true);
                me->SetSpeed(MOVE_WALK, 2.0f);
                me->SetSpeed(MOVE_RUN, 2.0f);
                me->GetMotionMaster()->MovePoint(RampageAura, *target);
                break;
            }
            default:
                break;
        }
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id == RampageAura)
        {
            me->RemoveAura(RampageAura);

            me->SetSpeed(MOVE_WALK, 1.0f);
            me->SetSpeed(MOVE_RUN, 1.0f);

            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                me->GetMotionMaster()->MoveChase(target);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventRampage:
                me->CastSpell(me, RampageSearcher, false);
                m_Events.RescheduleEvent(EventRampage, 20 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_gorian_high_sorcerer : public ScriptedAI
{
    enum eSpells
    {
        /// Arcane Blast
        ArcaneBlast = 174442,
        /// Nether Font
        NetherFontSearcher = 174434,
        NetherFontMissile = 174435,
        /// Celerity
        Celerity = 174440
    };

    enum eEvents
    {
        EventArcaneBlast = 1,
        EventNetherFont,
        EventCelerity
    };

    npc_highmaul_gorian_high_sorcerer(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventArcaneBlast, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventNetherFont, 10 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventCelerity, 15 * IN_MILLISECONDS);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case NetherFontSearcher:
                me->CastSpell(target, NetherFontMissile, true);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventArcaneBlast:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, ArcaneBlast, false);
                m_Events.RescheduleEvent(EventArcaneBlast, 15 * IN_MILLISECONDS);
                break;
            case EventNetherFont:
                me->CastSpell(me, NetherFontSearcher, false);
                m_Events.RescheduleEvent(EventNetherFont, 15 * IN_MILLISECONDS);
                break;
            case EventCelerity:
                if (Unit* target = me->SelectNearbyAlly(me, 50.0f, true))
                    me->CastSpell(target, Celerity, false);
                m_Events.RescheduleEvent(EventCelerity, 20 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_ogron_mauler : public ScriptedAI
{
    enum eSpell
    {
        DeafeningRoar = 174477
    };

    enum eEvent
    {
        EventDeafeningRoar = 1
    };

    npc_highmaul_ogron_mauler(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventDeafeningRoar, 5 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventDeafeningRoar:
                me->CastSpell(me, DeafeningRoar, false);
                m_Events.RescheduleEvent(EventDeafeningRoar, 15 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_guard_captain_thag : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        /// Brutal Cleave
        BrutalCleave = 174491,
        /// Ground Stomp
        GroundStomp = 174495,
        /// Rending Throw
        RendingThrow = 174500
    };

    enum eEvents
    {
        EventBrutalCleave = 1,
        EventGroundStomp,
        EventRendingThrow
    };

    enum eAction
    {
        ActionIntro
    };

    npc_highmaul_guard_captain_thag(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    ObjectGuid m_DoorGuid;

    InstanceScript* m_Instance;

    void Reset() override
    {
        m_Events.Reset();

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
        {
            if (GameObject* door = me->FindNearestGameObject(ThroneRoomDoor, 50.0f))
            {
                door->SetGoState(GO_STATE_READY);
                m_DoorGuid = door->GetGUID();
            }
        });

        /// Second equip is a shield
        me->SetCanDualWield(false);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventBrutalCleave, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventGroundStomp, 7 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventRendingThrow, 10 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (GameObject* door = GameObject::GetGameObject(*me, m_DoorGuid))
            door->SetGoState(GO_STATE_ACTIVE);

        if (m_Instance)
        {
            Creature* l_CouncilorMalgris = m_Instance->GetCreature((HighCouncilorMalgris));
            Creature* l_ImperatorMargok = m_Instance->GetCreature((ImperatorMargok));

            if (l_CouncilorMalgris && l_ImperatorMargok && l_CouncilorMalgris->IsAIEnabled && l_ImperatorMargok->IsAIEnabled)
            {
                l_CouncilorMalgris->AI()->DoAction(ActionIntro);
                l_ImperatorMargok->AI()->DoAction(ActionIntro);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventBrutalCleave:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, BrutalCleave, false);
                m_Events.RescheduleEvent(EventBrutalCleave, 7 * IN_MILLISECONDS);
                break;
            case EventGroundStomp:
                me->CastSpell(me, GroundStomp, true);
                m_Events.RescheduleEvent(EventGroundStomp, 6 * IN_MILLISECONDS);
                break;
            case EventRendingThrow:
                me->CastSpell(me, RendingThrow, true);
                m_Events.RescheduleEvent(EventRendingThrow, 6 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_councilor_daglat : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        KneelCosmeticForced = 130491,
        TeleportSearcher = 174536,
        TeleportMove = 174538,
        ArcaneDestruction = 174541
    };

    enum eEvent
    {
        EventArcaneDestruction = 1
    };

    npc_highmaul_councilor_daglat(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, KneelCosmeticForced, true);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventArcaneDestruction, 1 * IN_MILLISECONDS);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case TeleportSearcher:
            {
                me->CastSpell(*target, TeleportMove, true);

                AddDelayedEvent(200, [this]() -> void
                {
                    me->CastSpell(me, ArcaneDestruction, false);
                });

                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventArcaneDestruction:
                me->CastSpell(me, TeleportSearcher, true);
                m_Events.RescheduleEvent(EventArcaneDestruction, 6 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_councilor_magknor : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        KneelCosmeticForced = 130491,
        /// Arcane Torrent
        ArcaneTorrentSummon = 174549,
        ArcaneTorrentAura = 174558
    };

    enum eEvent
    {
        EventArcaneTorrent = 1
    };

    npc_highmaul_councilor_magknor(Creature* creature) : MS::AI::CosmeticAI(creature), m_Summons(creature) { }

    EventMap m_Events;
    SummonList m_Summons;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, KneelCosmeticForced, true);

        m_Summons.DespawnAll();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        me->CastSpell(me, ArcaneTorrentSummon, true);

        m_Events.RescheduleEvent(EventArcaneTorrent, 1 * IN_MILLISECONDS);
    }

    void JustSummoned(Creature* summon) override
    {
        m_Summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        m_Summons.Despawn(summon);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventArcaneTorrent:
                me->CastSpell(me, ArcaneTorrentAura, false);
                m_Events.RescheduleEvent(EventArcaneTorrent, 60 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_arcane_torrent : public MS::AI::CosmeticAI
{
    enum eSpell
    {
        ArcaneTorrentMoveSearcher = 174581
    };

    npc_highmaul_arcane_torrent(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        SetCanSeeEvenInPassiveMode(true);

        me->CastSpell(me, ArcaneTorrentMoveSearcher, true);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case ArcaneTorrentMoveSearcher:
                me->GetMotionMaster()->MovePoint(ArcaneTorrentMoveSearcher, *target);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id == ArcaneTorrentMoveSearcher)
            me->CastSpell(me, ArcaneTorrentMoveSearcher, true);
    }
};

struct npc_highmaul_councilor_gorluk : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        KneelCosmeticForced = 130491,

        ConjurePhantasmalWeapon = 174608
    };

    enum eEvent
    {
        EventPhantasmalWeapon = 1
    };

    npc_highmaul_councilor_gorluk(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, KneelCosmeticForced, true);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventPhantasmalWeapon, 7 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventPhantasmalWeapon:
                me->CastSpell(me, ConjurePhantasmalWeapon, false);
                m_Events.RescheduleEvent(EventPhantasmalWeapon, 7 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_phantasmal_weapon : public ScriptedAI
{
    enum eSpells
    {
        PhantasmalWeaponsSpawn = 174605,
        Focused = 174719,
        Fixated = 174627
    };

    npc_highmaul_phantasmal_weapon(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->CastSpell(me, PhantasmalWeaponsSpawn, true);
        me->CastSpell(me, Focused, true);
        me->CastSpell(me, Fixated, true);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case Fixated:
                AttackStart(target);
                break;
            default:
                break;
        }
    }
};

struct npc_highmaul_councilor_nouk : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        KneelCosmeticForced = 130491,

        TimeStop = 174939
    };

    enum eEvent
    {
        EventTimeStop = 1
    };

    npc_highmaul_councilor_nouk(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    EventMap m_Events;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, KneelCosmeticForced, true);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        me->CastSpell(me, TimeStop, false);

        m_Events.RescheduleEvent(EventTimeStop, 4 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventTimeStop:
                me->CastSpell(me, TimeStop, false);
                m_Events.RescheduleEvent(EventTimeStop, 4 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

struct npc_highmaul_high_councilor_malgris : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        UnstableTempest = 174981,
        UnstableTempestAura = 174976
    };

    enum eEvent
    {
        EventUnstableTempest = 1
    };

    enum eTalks
    {
        Intro1,
        Intro2
    };

    enum eActions
    {
        ActionIntro,
        ActionFinishIntro
    };

    npc_highmaul_high_councilor_malgris(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;

    InstanceScript* m_Instance;

    void Reset() override
    {
        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventUnstableTempest, 50);
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case ActionIntro:
            {
                Talk(Intro1);

                AddDelayedEvent(15 * IN_MILLISECONDS, [this]() -> void
                {
                    Talk(Intro2);
                });

                break;
            }
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* l_Margok = m_Instance->GetCreature((ImperatorMargok)))
        {
            if (l_Margok->IsAIEnabled)
                l_Margok->AI()->DoAction(ActionFinishIntro);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventUnstableTempest:
            {
                CustomSpellValues l_Values;
                if (Aura* aura = me->GetAura(UnstableTempestAura))
                    l_Values.AddSpellMod(SpellValueMod::SPELLVALUE_MAX_TARGETS, aura->GetStackAmount());
                else
                    l_Values.AddSpellMod(SpellValueMod::SPELLVALUE_MAX_TARGETS, 1);

                me->CastCustomSpell(UnstableTempest, l_Values, me, false);
                m_Events.RescheduleEvent(EventUnstableTempest, 50);
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Instance Portal (Raid: Normal, Heroic, Mythic) - 231770
struct go_highmaul_instance_portal : public GameObjectAI
{
    go_highmaul_instance_portal(GameObject* gameObject) : GameObjectAI(gameObject)
    {
        m_CheckTimer = 1000;
    }

    uint32 m_CheckTimer;

    void UpdateAI(uint32 diff) override
    {
        if (m_CheckTimer)
        {
            if (m_CheckTimer <= diff)
            {
                m_CheckTimer = 1000;

                std::list<Player*> playerList;
                go->GetPlayerListInGrid(playerList, 5.0f);

                for (Player* player : playerList)
                    player->TeleportTo(ExitTarget);
            }
            else
                m_CheckTimer -= diff;
        }
    }
};

/// Portal (teleporter to upper/lower city) - 231776
struct go_highmaul_portal : public GameObjectAI
{
    go_highmaul_portal(GameObject* gameObject) : GameObjectAI(gameObject)
    {
        m_CheckTimer = 500;
        m_IsUp = gameObject->GetPositionZ() < 300.0f;
        m_ImperatorsRise = gameObject->GetPositionZ() > 400.0f;
    }

    uint32 m_CheckTimer;
    bool m_IsUp;
    bool m_ImperatorsRise;

    enum eSpell
    {
        Teleport = 160595 ///< Cosmetic effect
    };

    void UpdateAI(uint32 diff) override
    {
        if (m_CheckTimer)
        {
            if (m_CheckTimer <= diff)
            {
                m_CheckTimer = 500;

                std::list<Player*> playerList;
                go->GetPlayerListInGrid(playerList, 5.0f);

                for (Player* player : playerList)
                {
                    player->CastSpell(player, Teleport, true);

                    if (m_ImperatorsRise)
                        player->NearTeleportTo(ImperatorsRise);
                    else
                    {
                        if (m_IsUp)
                            player->NearTeleportTo(PalaceFrontGate);
                        else
                            player->NearTeleportTo(CityBaseTeleporter);
                    }
                }
            }
            else
                m_CheckTimer -= diff;
        }
    }
};

/// Chain Grip - 151990
class spell_highmaul_chain_grip : public SpellScript
{
    PrepareSpellScript(spell_highmaul_chain_grip);

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster(), false));

        WorldObject* l_Object = targets.front();
        targets.clear();
        targets.push_back(l_Object);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_chain_grip::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

/// Chain Grip - 152024
class spell_highmaul_chain_grip_aura : public AuraScript
{
    PrepareAuraScript(spell_highmaul_chain_grip_aura);

    enum eSpell
    {
        ChainGripJumpDest = 151991
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetTarget())
                target->CastSpell(caster, ChainGripJumpDest, true);
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_chain_grip_aura::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Boar's Rush - 166225
class spell_highmaul_boars_rush : public SpellScript
{
    PrepareSpellScript(spell_highmaul_boars_rush);

    enum eSpell
    {
        TargetRestrict = 21373
    };

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        SpellTargetRestrictionsEntry const* restriction = sSpellTargetRestrictionsStore.LookupEntry(TargetRestrict);
        if (restriction == nullptr)
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        float radius = GetSpellInfo()->Effects[0]->CalcRadius(caster);
        targets.remove_if([radius, caster, restriction](WorldObject* object) -> bool
        {
            if (object == nullptr)
                return true;

            if (!object->IsInAxe(caster, restriction->Width, radius))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_boars_rush::CorrectTargets, EFFECT_1, TARGET_UNIT_ENEMY_BETWEEN_DEST);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_boars_rush::CorrectTargets, EFFECT_3, TARGET_UNIT_ENEMY_BETWEEN_DEST);
    }
};

/// Unstoppable Charge - 174465
class spell_highmaul_unstoppable_charge : public SpellScript
{
    PrepareSpellScript(spell_highmaul_unstoppable_charge);

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        if (Creature* ironFlame = caster->ToCreature())
        {
            Unit* target = Unit::GetUnit(*ironFlame, ironFlame->AI()->GetGUID(0));
            if (target == nullptr)
                return;

            float radius = GetSpellInfo()->Effects[0]->CalcRadius(caster);
            targets.remove_if([radius, caster, target](WorldObject* object) -> bool
            {
                if (object == nullptr)
                    return true;

                if (!object->IsInBetween(caster, target, 3.0f))
                    return true;

                return false;
            });
        }
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_unstoppable_charge::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_unstoppable_charge::CorrectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

/// Corrupted Blood Shield - 174474
class spell_highmaul_corrupted_blood_shield : public AuraScript
{
    PrepareAuraScript(spell_highmaul_corrupted_blood_shield);

    enum eSpell
    {
        CorruptedBlood = 174473
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_EXPIRE)
            return;

        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetTarget())
                target->CastSpell(caster, CorruptedBlood, true, nullptr, nullptr, caster->GetGUID());
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_corrupted_blood_shield::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Rending Slash - 166185
class spell_highmaul_rending_slash : public SpellScript
{
    PrepareSpellScript(spell_highmaul_rending_slash);

    enum eSpell
    {
        TargetRestrict = 22561
    };

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        SpellTargetRestrictionsEntry const* restriction = sSpellTargetRestrictionsStore.LookupEntry(TargetRestrict);
        if (restriction == nullptr)
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        float angle = 2 * M_PI / 360 * restriction->ConeDegrees;
        targets.remove_if([caster, angle](WorldObject* object) -> bool
        {
            if (object == nullptr)
                return true;

            if (!object->isInFront(caster, angle))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_rending_slash::CorrectTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_54);
    }
};

/// Shield Charge - 166178
class spell_highmaul_shield_charge : public AuraScript
{
    PrepareAuraScript(spell_highmaul_shield_charge);

    enum eSpells
    {
        ShieldChargeDamage = 166180,
        ShieldChargeBump = 166181
    };

    uint32 m_DamageTimer;

    bool Load() override
    {
        m_DamageTimer = 500;
        return true;
    }

    void OnUpdate(uint32 diff)
    {
        if (m_DamageTimer)
        {
            if (m_DamageTimer <= diff)
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<Unit*> targetList;
                    float radius = 1.0f;

                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck l_Check(caster, caster, radius);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> l_Searcher(caster, targetList, l_Check);
                    caster->VisitNearbyObject(radius, l_Searcher);

                    for (Unit* l_Iter : targetList)
                    {
                        caster->CastSpell(l_Iter, ShieldChargeDamage, true);
                        l_Iter->CastSpell(l_Iter, ShieldChargeBump, true);
                    }
                }

                m_DamageTimer = 500;
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_shield_charge::OnUpdate);
    }
};

/// Earthdevastating Slam - 166174
class spell_highmaul_earthdevastating_slam : public AuraScript
{
    PrepareAuraScript(spell_highmaul_earthdevastating_slam);

    enum eAction
    {
        ActionSlam
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (GetTarget() == nullptr)
            return;

        if (Creature* l_Trash = GetTarget()->ToCreature())
        {
            if (l_Trash->IsAIEnabled)
                l_Trash->AI()->DoAction(ActionSlam);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_earthdevastating_slam::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Earthdevastating Slam (damage) - 166175
class spell_highmaul_earthdevastating_slam_dmg : public SpellScript
{
    PrepareSpellScript(spell_highmaul_earthdevastating_slam_dmg);

    enum eSpell
    {
        TargetRestrict = 21362
    };

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        SpellTargetRestrictionsEntry const* restriction = sSpellTargetRestrictionsStore.LookupEntry(TargetRestrict);
        if (restriction == nullptr)
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        float radius = GetSpellInfo()->Effects[0]->CalcRadius(caster);
        targets.remove_if([radius, caster, restriction](WorldObject* object) -> bool
        {
            if (object == nullptr)
                return true;

            if (!object->IsInAxe(caster, restriction->Width, radius))
                return true;

            if (!object->isInFront(caster))
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_earthdevastating_slam_dmg::CorrectTargets, EFFECT_0, TARGET_UNIT_ENEMY_BETWEEN_DEST);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_earthdevastating_slam_dmg::CorrectTargets, EFFECT_1, TARGET_UNIT_ENEMY_BETWEEN_DEST);
    }
};

/// Arcane Barrage - 178023
class spell_highmaul_arcane_barrage : public SpellScript
{
    enum eSpells
    {
        ArcaneBarrageFirst = 178025,
    };

    PrepareSpellScript(spell_highmaul_arcane_barrage);

    void HandleDummy(SpellEffIndex /*p_EffIndex*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, ArcaneBarrageFirst, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_highmaul_arcane_barrage::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

class spell_highmaul_arcane_barrage_AuraScript : public AuraScript
{
    enum eSpells
    {
        ArcaneBarrageSecond = 178026
    };

    PrepareAuraScript(spell_highmaul_arcane_barrage_AuraScript);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, ArcaneBarrageSecond, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_arcane_barrage_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Decimate - 166189
class spell_highmaul_decimate : public SpellScript
{
    enum eSpells
    {
        DecimateMissile = 166187
    };

    PrepareSpellScript(spell_highmaul_decimate);

    void HandleDummy(SpellEffIndex /*p_EffIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetHitUnit())
                caster->CastSpell(target, DecimateMissile, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_highmaul_decimate::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_highmaul_decimate_AuraScript : public AuraScript
{
    enum eSpells
    {
        DecimateMissile = 166187
    };

    PrepareAuraScript(spell_highmaul_decimate_AuraScript);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetTarget())
                caster->CastSpell(target, DecimateMissile, true);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_decimate_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Unstable Tempest - 174981
class spell_highmaul_unstable_tempest : public SpellScript
{
    PrepareSpellScript(spell_highmaul_unstable_tempest);

    enum eSpell
    {
        UnstableTempestStack = 174976
    };

    void HandleOnCast()
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, UnstableTempestStack, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_highmaul_unstable_tempest::HandleOnCast);
    }
};

/// Time Stop - 174939
class spell_highmaul_time_stop : public AuraScript
{
    enum eSpells
    {
        TimeStopStun = 174961
    };

    PrepareAuraScript(spell_highmaul_time_stop);

    void OnTick(AuraEffect const* auraEffect)
    {
        if (auraEffect->GetTickNumber() % 3)
            return;

        if (Unit* target = GetTarget())
            target->CastSpell(target, TimeStopStun, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_time_stop::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_highmaul()
{
    //RegisterHighmaulCreatureAI(npc_highmaul_gharg_arena_master);
    //RegisterHighmaulCreatureAI(npc_highmaul_jhorn_the_mad);
    //RegisterHighmaulCreatureAI(npc_highmaul_thoktar_ironskull);
    //RegisterHighmaulCreatureAI(npc_highmaul_imperator_margok);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_guardsman);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_devout);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_runemaster);
    //RegisterHighmaulCreatureAI(npc_highmaul_gorian_enforcer);
    RegisterHighmaulCreatureAI(npc_highmaul_underbelly_vagrant);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_sorcerer);
    //RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_brute);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_soothsayer);
    RegisterHighmaulCreatureAI(npc_highmaul_void_aberration);
    RegisterHighmaulCreatureAI(npc_highmaul_krush);
    RegisterHighmaulCreatureAI(npc_highmaul_iron_flame_technician);
    RegisterHighmaulCreatureAI(npc_highmaul_iron_warmaster);
    RegisterHighmaulCreatureAI(npc_highmaul_iron_blood_mage);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_ritualist);
    RegisterHighmaulCreatureAI(npc_highmaul_greater_void_aberration);
    RegisterHighmaulCreatureAI(npc_highmaul_highmaul_conscript);
    RegisterHighmaulCreatureAI(npc_highmaul_ogron_earthshaker);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_arcanist);
    RegisterHighmaulCreatureAI(npc_highmaul_ogron_brute);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_rune_mender);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_fanatic);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_warden);
    RegisterHighmaulCreatureAI(npc_highmaul_ogron_warbringer);
    RegisterHighmaulCreatureAI(npc_highmaul_warden_thultok);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_royal_guardsman);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_high_sorcerer);
    //RegisterHighmaulCreatureAI(npc_highmaul_ogron_mauler);
    RegisterHighmaulCreatureAI(npc_highmaul_guard_captain_thag);
    RegisterHighmaulCreatureAI(npc_highmaul_councilor_daglat);
    RegisterHighmaulCreatureAI(npc_highmaul_councilor_magknor);
    //RegisterHighmaulCreatureAI(npc_highmaul_arcane_torrent);
    RegisterHighmaulCreatureAI(npc_highmaul_councilor_gorluk);
    RegisterHighmaulCreatureAI(npc_highmaul_phantasmal_weapon);
    RegisterHighmaulCreatureAI(npc_highmaul_councilor_nouk);
    RegisterHighmaulCreatureAI(npc_highmaul_high_councilor_malgris);

    RegisterGameObjectAI(go_highmaul_instance_portal);
    RegisterGameObjectAI(go_highmaul_portal);

    RegisterSpellScript(spell_highmaul_chain_grip);
    RegisterAuraScript(spell_highmaul_chain_grip_aura);
    RegisterSpellScript(spell_highmaul_boars_rush);
    RegisterSpellScript(spell_highmaul_unstoppable_charge);
    RegisterAuraScript(spell_highmaul_corrupted_blood_shield);
    RegisterSpellScript(spell_highmaul_rending_slash);
    //RegisterAuraScript(spell_highmaul_shield_charge);
    RegisterAuraScript(spell_highmaul_earthdevastating_slam);
    RegisterSpellScript(spell_highmaul_earthdevastating_slam_dmg);
    RegisterSpellAndAuraScriptPair(spell_highmaul_arcane_barrage, spell_highmaul_arcane_barrage_AuraScript);
    RegisterSpellAndAuraScriptPair(spell_highmaul_decimate, spell_highmaul_decimate_AuraScript);
    RegisterSpellScript(spell_highmaul_unstable_tempest);
    RegisterAuraScript(spell_highmaul_time_stop);
}
