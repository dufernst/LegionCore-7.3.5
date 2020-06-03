////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"

float const gorthenonFloor = 330.0f;
float const circleToCenterDist = 30.0f;
float const blazeDistToCenter = 136.0f;

Position const polJumpPos = {4043.08f, 8500.94f, 322.226f, 5.63415f};
Position const phemosJumpPos = {4028.90f, 8485.51f, 322.226f, 5.63415f};
Position const centerPos = {4062.38f, 8470.91f, 322.226f, 0.0f};

Position const polMovePos[3] =
{
    {4031.884f, 8587.818f, 343.6748f, 4.55423f},
    {4028.196f, 8568.284f, 341.8094f, 4.51496f},
    {4012.667f, 8523.617f, 327.2006f, 5.63415f}
};

Position const phemosMovePos[3] =
{
    {3949.489f, 8484.431f, 343.7032f, 0.31307f},
    {3974.303f, 8497.139f, 335.4293f, 0.40340f},
    {4002.597f, 8510.825f, 327.1879f, 5.63415f}
};


void RespawnOgrons(Creature* source, InstanceScript* instance)
{
    if (source == nullptr || instance == nullptr)
        return;

    if (Creature* other = Creature::GetCreature(*source, (source->GetEntry() == Phemos) ? instance->GetGuidData(Pol) : instance->GetGuidData(Phemos)))
    {
        other->Respawn();
        other->GetMotionMaster()->MoveTargetedHome();
    }
}

void StartOgrons(Creature* source, Unit* target)
{
    if (source == nullptr || target == nullptr)
        return;

    if (Creature* other = source->FindNearestCreature((source->GetEntry() == Phemos) ? Pol : Phemos, 30.0f))
        other->AI()->AttackStart(target);
}

/// Pol - 78238
struct boss_twin_ogron_pol : public BossAI
{
    enum eSpells
    {
        WarmingUp = 173425,
        AggressiveDisposition = 157951,
        Disposition = 154172,
        VenomshadeCopyDmgAura = 154349,
        /// Pulverize
        PulverizeAura = 158385,
        PulverizeFirstAoE = 157952,
        PulverizeFirstAoEDmg = 158336,
        PulverizeSecondAoE = 158415,
        PulverizeSecondAoEDmg = 158417,
        PulverizeThirdAoE = 158419,
        PulverizeThirdAoEDmg = 158420,
        /// Interrupting Shout
        SpellInterruptingShout = 158093,
        InterruptingShoutDmg = 158102,
        /// Shield Charge
        SpellShieldCharge = 158134,
        ShieldChargeEndingAoE = 158157,
        InjuredDoT = 155569,

        ShieldBash = 143834,

        /// Loot
        TwinOgronBonus = 177525
    };

    enum eEvents
    {
        EventBerserker = 1,
        EventShieldBash
    };

    enum eActions
    {
        SchedulePulverize,
        ScheduleInterruptingShout,
        ScheduleShieldCharge
    };

    enum eTalks
    {
        Intro,
        Aggro,
        InterruptingShout,
        ShieldCharge,
        Pulverize,
        Slay,
        ShieldChargeWarn
    };

    enum eMoves
    {
        MoveFirst = 10,
        MoveSecond,
        MoveThird,
        MoveJump,

        MoveShieldCharge = 158134
    };

    enum eVisuals
    {
        PulverizeVisual = 37673,
        PulverizeSecond = 37699,
        PulverizeLast = 44116,
        BigPulverize = 37702,
        PulverizeFinal = 40527
    };

    boss_twin_ogron_pol(Creature* creature) : BossAI(creature, BossTwinOgron)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    InstanceScript* m_Instance;

    bool m_ShieldChargeScheduled;
    bool m_InterruptingShoutScheduled;

    void Reset() override
    {
        m_Events.Reset();

        _Reset();

        me->SetPower(POWER_ENERGY, 0);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

        me->SetCanDualWield(false);

        me->CastSpell(me, AggressiveDisposition, true);
        me->CastSpell(me, Disposition, true);

        m_ShieldChargeScheduled = false;
        m_InterruptingShoutScheduled = false;

        me->CancelSpellVisualKit(PulverizeLast);
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case SchedulePulverize:
            {
                Talk(Pulverize);

                m_InterruptingShoutScheduled = false;
                m_ShieldChargeScheduled = false;

                me->SetPower(POWER_ENERGY, 0);

                me->AddUnitState(UNIT_STATE_ROOT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                me->CastSpell(me, PulverizeAura, true);

                AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
                {
                    me->CastSpell(me, PulverizeFirstAoE, false);
                });

                AddDelayedEvent(5 * IN_MILLISECONDS, [this]() -> void
                {
                    me->CastSpell(me, PulverizeSecondAoE, false);
                });

                AddDelayedEvent(9 * IN_MILLISECONDS, [this]() -> void
                {
                    me->SendPlaySpellVisualKit(PulverizeLast, 0, 0);
                    me->CastSpell(me, PulverizeThirdAoE, false);
                });

                AddDelayedEvent(15 * IN_MILLISECONDS, [this]() -> void
                {
                    Position pos;
                    me->GetRandomNearPosition(pos, 30.0f);
                    me->CastSpell(pos, PulverizeThirdAoEDmg, true);

                    me->ClearUnitState(UNIT_STATE_ROOT);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                });

                break;
            }
            case ScheduleInterruptingShout:
            {
                if (m_InterruptingShoutScheduled)
                    break;

                Talk(InterruptingShout);
                m_InterruptingShoutScheduled = true;
                me->CastSpell(me, SpellInterruptingShout, false);
                break;
            }
            case ScheduleShieldCharge:
            {
                if (m_ShieldChargeScheduled)
                    break;

                Talk(ShieldCharge);
                Talk(ShieldChargeWarn);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -20.0f, true))
                    me->CastSpell(target, SpellShieldCharge, false);
                else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f, true))
                    me->CastSpell(target, SpellShieldCharge, false);

                m_ShieldChargeScheduled = true;
                break;
            }
            default:
                break;
        }
    }

    void EnterCombat(Unit* attacker) override
    {
        Talk(Aggro);

        me->CastSpell(me, WarmingUp, true);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        m_Events.RescheduleEvent(EventBerserker, IsMythicRaid() ? 420 * IN_MILLISECONDS : 480 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventShieldBash, 22 * IN_MILLISECONDS);

        StartOgrons(me, attacker);

        if (Creature* other = me->FindNearestCreature(Phemos, 150.0f))
            me->AddAura(VenomshadeCopyDmgAura, other);

        _EnterCombat();
    }

    void KilledUnit(Unit* killed) override
    {
        if (killed->IsPlayer())
            Talk(Slay);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(InjuredDoT);
        CastSpellToPlayers(me->GetMap(), me, TwinOgronBonus, true);
    }

    void EnterEvadeMode() override
    {
        me->ClearUnitState(UNIT_STATE_ROOT);

        m_Instance->SetBossState(BossTwinOgron, FAIL);
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(InjuredDoT);

        RespawnOgrons(me, m_Instance);

        CreatureAI::EnterEvadeMode();
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        switch (id)
        {
            case MoveFirst:
                me->GetMotionMaster()->MovePoint(MoveSecond, polMovePos[1]);
                break;
            case MoveSecond:
                me->GetMotionMaster()->MovePoint(MoveThird, polMovePos[2]);
                break;
            case MoveThird:
                me->SetHomePosition(*me);
                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void { me->SetFacingTo(polMovePos[2].m_orientation); });
                break;
            case MoveJump:
                me->SetHomePosition(*me);
                break;
            case MoveShieldCharge:
                me->RemoveAura(SpellShieldCharge);
                me->CastSpell(me, ShieldChargeEndingAoE, true);
                break;
            default:
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case SpellInterruptingShout:
            {
                me->CastSpell(target, InterruptingShoutDmg, true);
                break;
            }
            case PulverizeFirstAoE:
            {
                me->CastSpell(target, PulverizeFirstAoEDmg, true);
                break;
            }
            case PulverizeSecondAoE:
            {
                Position pos = *target;
                AddDelayedEvent(3 * IN_MILLISECONDS, [this, pos]() -> void
                {
                    me->CastSpell(pos, PulverizeSecondAoEDmg, true);
                });

                break;
            }
            default:
                break;
        }
    }

    void RegeneratePower(Powers power, float& value) override
    {
        /// Pol only regens by script
        if (power == POWER_ENERGY)
            value = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->GetDistance(me->GetHomePosition()) >= 110.0f)
        {
            EnterEvadeMode();
            return;
        }

        if (!UpdateVictim())
            return;

        if (!me->HasAura(WarmingUp))
            m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventBerserker:
            {
                me->CastSpell(me, Berserker, true);
                break;
            }
            case EventShieldBash:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, ShieldBash, false);
                m_Events.RescheduleEvent(EventShieldBash, 22 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Phemos - 78237
struct boss_twin_ogron_phemos : public BossAI
{
    enum eSpells
    {
        /// Misc
        WarmingUp = 173425,
        AggressiveDisposition = 157951,
        Disposition = 154172,
        VenomshadeCopyDmgAura = 154349,
        DespawnAreaTriggers = 115905,
        /// Quake
        SpellQuake = 158200,
        BlazeWeaponVisual = 158206,
        BlazeFirstSpawn = 162901,
        /// Enfeebling Roar
        SpellEnfeeblingRoar = 158057,
        EnfeeblingRoarDebuff = 158026,
        /// Whirlwind
        SpellWhirlwind = 157943,
        WeakenedDefenses = 159709,

        DoubleSlashMainHand = 158521,
        DoubleSlashOffHand = 167198,
        BlazeDoT = 158241
    };

    enum eEvents
    {
        EventCheckPlayer = 1,
        EventBerserker,
        EventDoubleSlash,
        EventSpawnFirstBlaze,
        EventSpawnSecondBlaze
    };

    enum eActions
    {
        ActionMoveHome,
        ScheduleQuake,
        ScheduleEnfeeblingRoar,
        ScheduleWhirlwind
    };

    enum eCreature
    {
        InvisibleStalker = 15214
    };

    enum eTalks
    {
        Intro,
        Aggro,
        Whirlwind,
        Quake,
        EnfeeblingRoar,
        Berserk,
        Slay,
        Wipe,
        WhirlwindWarn
    };

    enum eMoves
    {
        MoveFirst = 10,
        MoveSecond,
        MoveThird,
        MoveJump
    };

    enum eVisuals
    {
        QuakeVisualID = 41369,
        QuakeSpellVisual = 37816
    };

    enum eMiscs
    {
        EnfeeblingCounter,
        BlazeFirstSpawnCounter = 3,
        BlazeWaveSpawnCounter = 6,
        BlazeTotalWaveCount = 5
    };

    boss_twin_ogron_phemos(Creature* creature) : BossAI(creature, BossTwinOgron)
    {
        m_Instance = creature->GetInstanceScript();
        m_Init = false;
        m_HomeChanged = false;
    }

    EventMap m_CosmeticEvents;
    EventMap m_Events;
    InstanceScript* m_Instance;

    std::set<ObjectGuid> m_TrashsMobs;
    bool m_Init;
    bool m_HomeChanged;

    bool m_WhirlwindScheduled;
    bool m_EnfeeblingRoarScheduled;

    std::set<ObjectGuid> m_EnfeeblingRoarTargets;

    Position m_FirstBlazePos;
    uint8 m_FirstWaveCounter;
    Position m_SecondBlazePos;
    uint8 m_SecondWaveCounter;

    void Reset() override
    {
        if (!m_HomeChanged)
            m_CosmeticEvents.RescheduleEvent(EventCheckPlayer, 1 * IN_MILLISECONDS);

        m_Events.Reset();

        _Reset();

        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 98);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);

        me->CastSpell(me, DespawnAreaTriggers, true);
        me->CastSpell(me, AggressiveDisposition, true);
        me->CastSpell(me, Disposition, true);

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            if (m_TrashsMobs.empty() && !m_Init)
            {
                std::list<Creature*> trashList;
                me->GetCreatureListInGrid(trashList, 250.0f);

                if (!trashList.empty())
                {
                    trashList.remove_if([this](Creature* creature) -> bool
                    {
                        if (creature == nullptr || creature->GetPositionZ() > gorthenonFloor)
                            return true;

                        if (creature->GetEntry() == InvisibleStalker)
                            return true;

                        if (!creature->isAlive())
                            return true;

                        return false;
                    });
                }

                for (Creature* creature : trashList)
                    m_TrashsMobs.insert(creature->GetGUID());

                m_Init = true;
            }
        });

        m_WhirlwindScheduled = false;
        m_EnfeeblingRoarScheduled = false;

        m_EnfeeblingRoarTargets.clear();

        m_FirstBlazePos = Position();
        m_FirstWaveCounter = 0;
        m_SecondBlazePos = Position();
        m_SecondWaveCounter = 0;

        me->CancelSpellVisualKit(QuakeVisualID);
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case ActionMoveHome:
            {
                DoMoveHome();
                break;
            }
            case ScheduleQuake:
            {
                Talk(Quake);

                me->SetPower(POWER_ENERGY, 0);
                me->CastSpell(me, SpellQuake, false);

                m_EnfeeblingRoarScheduled = false;
                m_WhirlwindScheduled = false;

                float o = frand(0, 2 * M_PI);
                Position pos = {(centerPos.m_positionX + (circleToCenterDist * cos(o))), (centerPos.m_positionY + (circleToCenterDist * sin(o))), centerPos.m_positionZ, 0.0f};

                pos.m_orientation = pos.GetAngle(&centerPos);
                m_FirstBlazePos = pos;

                /// Total: 4 waves
                /// First
                ++m_FirstWaveCounter;
                SpawnBlazeWave(pos);
                /// Second, third and fourth
                m_CosmeticEvents.RescheduleEvent(EventSpawnFirstBlaze, 5 * IN_MILLISECONDS);

                me->CastSpell(pos, BlazeWeaponVisual, true);
                me->SendPlaySpellVisualKit(QuakeVisualID, 0, 0);

                o += M_PI;
                pos = {(centerPos.m_positionX + (circleToCenterDist * cos(o))), (centerPos.m_positionY + (circleToCenterDist * sin(o))), centerPos.m_positionZ, 0.0f};

                pos.m_orientation = pos.GetAngle(&centerPos);

                m_SecondBlazePos = pos;
                m_CosmeticEvents.RescheduleEvent(EventSpawnSecondBlaze, 1500);

                AddDelayedEvent(1500, [this, pos]() -> void
                {
                    me->CastSpell(pos, BlazeWeaponVisual, true);
                    me->SendPlaySpellVisualKit(QuakeVisualID, 0, 0);
                });

                break;
            }
            case ScheduleEnfeeblingRoar:
            {
                if (m_EnfeeblingRoarScheduled)
                    break;

                m_EnfeeblingRoarTargets.clear();
                Talk(EnfeeblingRoar);
                me->CastSpell(me, SpellEnfeeblingRoar, false);
                m_EnfeeblingRoarScheduled = true;

                /// Cast time of Enfeebling roar + 100ms timer
                AddDelayedEvent(3100, [this]() -> void
                {
                    for (ObjectGuid guid : m_EnfeeblingRoarTargets)
                    {
                        if (Unit* target = Unit::GetUnit(*me, guid))
                            me->CastSpell(target, EnfeeblingRoarDebuff, true);
                    }
                });

                break;
            }
            case ScheduleWhirlwind:
            {
                if (m_WhirlwindScheduled)
                    break;

                Talk(Whirlwind);
                Talk(WhirlwindWarn);
                me->CastSpell(me, SpellWhirlwind, false);
                m_WhirlwindScheduled = true;
                break;
            }
            default:
                break;
        }
    }

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        m_TrashsMobs.erase(guid);
    }

    void EnterCombat(Unit* attacker) override
    {
        Talk(Aggro);

        me->CastSpell(me, WarmingUp, true);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        m_Events.RescheduleEvent(EventBerserker, IsMythicRaid() ? 420 * IN_MILLISECONDS : 480 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventDoubleSlash, 26 * IN_MILLISECONDS);

        StartOgrons(me, attacker);

        if (Creature* other = me->FindNearestCreature(Pol, 150.0f))
            me->AddAura(VenomshadeCopyDmgAura, other);

        _EnterCombat();
    }

    void KilledUnit(Unit* killed) override
    {
        if (killed->IsPlayer())
            Talk(Slay);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(WeakenedDefenses);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(EnfeeblingRoarDebuff);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(BlazeDoT);
    }

    void EnterEvadeMode() override
    {
        Talk(Wipe);

        m_Instance->SetBossState(BossTwinOgron, FAIL);
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(WeakenedDefenses);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(EnfeeblingRoarDebuff);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(BlazeDoT);

        RespawnOgrons(me, m_Instance);

        CreatureAI::EnterEvadeMode();
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        switch (id)
        {
            case MoveFirst:
                me->GetMotionMaster()->MovePoint(MoveSecond, phemosMovePos[1]);
                break;
            case MoveSecond:
                me->GetMotionMaster()->MovePoint(MoveThird, phemosMovePos[2]);
                break;
            case MoveThird:
            {
                me->SetHomePosition(*me);
                m_HomeChanged = true;

                AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
                {
                    me->SetFacingTo(phemosMovePos[2].m_orientation);
                });

                if (m_TrashsMobs.empty())
                {
                    AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
                    {
                        DoMoveHome();
                    });
                }

                break;
            }
            case MoveJump:
            {
                me->SetHomePosition(*me);

                me->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);

                if (Creature* pos = m_Instance->GetCreature((Pol)))
                    pos->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);

                break;
            }
            default:
                break;
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case SpellEnfeeblingRoar:
            {
                m_EnfeeblingRoarTargets.insert(target->GetGUID());
                break;
            }
            default:
                break;
        }
    }

    void RegeneratePower(Powers power, float& value) override
    {
        /// Phemos only regens by script
        if (power == POWER_ENERGY)
            value = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->GetDistance(me->GetHomePosition()) >= 110.0f)
        {
            EnterEvadeMode();
            return;
        }

        m_CosmeticEvents.Update(diff);

        switch (m_CosmeticEvents.ExecuteEvent())
        {
            case EventCheckPlayer:
            {
                if (me->FindNearestPlayer(150.0f))
                    DoFirstMove();
                else
                    m_CosmeticEvents.RescheduleEvent(EventCheckPlayer, 1 * IN_MILLISECONDS);
                break;
            }
            case EventSpawnFirstBlaze:
            {
                ++m_FirstWaveCounter;
                SpawnBlazeWave(m_FirstBlazePos);
                if (m_FirstWaveCounter <= BlazeTotalWaveCount)
                    m_CosmeticEvents.RescheduleEvent(EventSpawnFirstBlaze, 5 * IN_MILLISECONDS);
                break;
            }
            case EventSpawnSecondBlaze:
            {
                ++m_SecondWaveCounter;
                SpawnBlazeWave(m_SecondBlazePos);
                if (m_SecondWaveCounter <= BlazeTotalWaveCount)
                    m_CosmeticEvents.RescheduleEvent(EventSpawnSecondBlaze, 5 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        if (!UpdateVictim())
            return;

        if (!me->HasAura(WarmingUp))
            m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventBerserker:
            {
                me->CastSpell(me, Berserker, true);
                Talk(Berserk);
                break;
            }
            case EventDoubleSlash:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, DoubleSlashMainHand, false);

                    ObjectGuid guid = target->GetGUID();
                    AddDelayedEvent(1 * IN_MILLISECONDS, [this, guid]() -> void
                    {
                        if (Unit* target = Unit::GetUnit(*me, guid))
                            me->CastSpell(target, DoubleSlashOffHand, true);
                    });
                }

                m_Events.RescheduleEvent(EventDoubleSlash, 27 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }

    uint32 GetData(uint32 id) const override
    {
        if (id == EnfeeblingCounter)
            return m_EnfeeblingRoarTargets.size();

        return 0;
    }

    void DoFirstMove()
    {
        Talk(Intro);
        me->GetMotionMaster()->MovePoint(MoveFirst, phemosMovePos[0]);

        if (Creature* pos = m_Instance->GetCreature((Pol)))
        {
            pos->GetMotionMaster()->MovePoint(MoveFirst, polMovePos[0]);

            ObjectGuid guid = pos->GetGUID();
            AddDelayedEvent(5 * IN_MILLISECONDS, [this, guid]() -> void
            {
                if (Creature* boss = Creature::GetCreature(*me, guid))
                {
                    if (boss->IsAIEnabled)
                        boss->AI()->Talk(Intro);
                }
            });
        }
    }

    void DoMoveHome()
    {
        if (!m_TrashsMobs.empty())
            return;

        me->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        me->GetMotionMaster()->MoveJump(phemosJumpPos, 30.0f, 20.0f, MoveJump);

        if (Creature* pos = m_Instance->GetCreature((Pol)))
        {
            pos->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            pos->GetMotionMaster()->MoveJump(polJumpPos, 30.0f, 20.0f, MoveJump);
        }
    }

    void SpawnBlazeWave(Position pos)
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(BlazeFirstSpawn))
        {
            float oStep = (pos.m_orientation - (M_PI / 2.0f)) + frand(0.0f, (M_PI / 3.0f));

            for (uint8 i = 0; i < BlazeFirstSpawnCounter; ++i)
            {
                uint32 timer = 0;
                float o = oStep;

                for (uint8 j = 0; j < BlazeWaveSpawnCounter; ++j)
                {
                    Position dest = pos;
                    dest.m_positionX = pos.m_positionX + (blazeDistToCenter * cos(o));
                    dest.m_positionY = pos.m_positionY + (blazeDistToCenter * sin(o));

                    if (timer)
                    {
                        AddDelayedEvent(timer, [this, pos, dest]() -> void
                        {
                            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(BlazeFirstSpawn))
                            {
                                AreaTrigger* areaTrigger = new AreaTrigger;
                                if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), spellInfo->Id, me, spellInfo, pos, dest))
                                    delete areaTrigger;
                            }
                        });
                    }
                    else
                    {
                        AreaTrigger* areaTrigger = new AreaTrigger;
                        if (!areaTrigger->CreateAreaTrigger(sObjectMgr->GetGenerator<HighGuid::AreaTrigger>()->Generate(), spellInfo->Id, me, spellInfo, pos, dest))
                            delete areaTrigger;
                    }

                    /// Is it enough?
                    o += 0.05f;

                    timer += 500;
                }

                oStep += M_PI / 3.0f;
            }
        }
    }
};

/// Warming Up - 173425
class spell_highmaul_warming_up : public AuraScript
{
    PrepareAuraScript(spell_highmaul_warming_up);

    enum eSpell
    {
        Disposition = 157953
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, Disposition, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_warming_up::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Disposition - 157953
class spell_highmaul_disposition : public AuraScript
{
    PrepareAuraScript(spell_highmaul_disposition);

    uint8 tickCount;

    enum eActions
    {
        /// Phemos
        ScheduleQuake = 1,
        ScheduleEnfleeblingRoar = 2,
        ScheduleWhirlwind = 3,
        /// Pol
        SchedulePulverize = 0,
        ScheduleInterruptingShout = 1,
        ScheduleShieldCharge = 2
    };

    bool Load() override
    {
        tickCount = 0;
        return true;
    }

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (GetTarget() == nullptr)
            return;

        if (Creature* boss = GetTarget()->ToCreature())
        {
            float energyGain = 1.0f;
            energyGain *= 1.0f + (float)boss->GetPower(POWER_ALTERNATE) / 100.0f;

            /// Pol energizes 25% faster than Phemos
            if (boss->GetEntry() == Pol)
            {
                ++tickCount;
                energyGain *= 1.25f;

                if ((int32)energyGain == 1 && !(tickCount % 2))
                {
                    tickCount = 0;
                    energyGain += 1.0f;
                }
            }

            boss->EnergizeBySpell(boss, GetSpellInfo()->Id, (int32)energyGain, POWER_ENERGY);

            if (!boss->IsAIEnabled)
                return;

            switch (boss->GetEntry())
            {
                case Phemos:
                {
                    if (boss->GetPower(POWER_ENERGY) >= 100)
                        boss->AI()->DoAction(ScheduleQuake);
                    else if (boss->GetPower(POWER_ENERGY) >= 66 && boss->GetPower(POWER_ENERGY) <= 80)
                        boss->AI()->DoAction(ScheduleEnfleeblingRoar);
                    else if (boss->GetPower(POWER_ENERGY) >= 33 && boss->GetPower(POWER_ENERGY) <= 50)
                        boss->AI()->DoAction(ScheduleWhirlwind);

                    break;
                }
                case Pol:
                {
                    if (boss->GetPower(POWER_ENERGY) >= 100)
                        boss->AI()->DoAction(SchedulePulverize);
                    else if (boss->GetPower(POWER_ENERGY) >= 66 && boss->GetPower(POWER_ENERGY) <= 80)
                        boss->AI()->DoAction(ScheduleInterruptingShout);
                    else if (boss->GetPower(POWER_ENERGY) >= 33 && boss->GetPower(POWER_ENERGY) <= 50)
                        boss->AI()->DoAction(ScheduleShieldCharge);

                    break;
                }
                default:
                    break;
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_disposition::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Enfeebling Roar - 158026
class spell_highmaul_enfeebling_roar_AuraScript : public AuraScript
{
    enum eMisc
    {
        EnfeeblingCounter
    };

    PrepareAuraScript(spell_highmaul_enfeebling_roar_AuraScript);

    void AfterApply(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        if (GetCaster() == nullptr)
            return;

        if (Creature* phemos = GetCaster()->ToCreature())
        {
            if (!phemos->IsAIEnabled)
                return;

            Aura* aura = auraEffect->GetBase();
            if (!aura)
                return;

            uint32 count = phemos->AI()->GetData(EnfeeblingCounter);
            if (!count)
                count = 1;

            uint32 maxDuration = aura->GetDuration() * 10;
            int32 amount = auraEffect->GetAmount() * 10;

            aura->SetDuration(maxDuration / count);
            aura->SetMaxDuration(maxDuration / count);
            aura->GetEffect(EFFECT_1)->ChangeAmount(amount / count);
            aura->SetNeedClientUpdateForTargets();
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_highmaul_enfeebling_roar_AuraScript::AfterApply, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_highmaul_enfeebling_roar : public SpellScript
{
    enum eMiscs
    {
        EnfeeblingCounter,
    };

    PrepareSpellScript(spell_highmaul_enfeebling_roar);

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (GetCaster() == nullptr)
            return;

        if (Creature* phemos = GetCaster()->ToCreature())
        {
            if (!phemos->IsAIEnabled)
                return;

            uint32 count = phemos->AI()->GetData(EnfeeblingCounter);
            if (!count)
                count = 1;

            SetHitDamage(GetHitDamage() * 10 / count);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_highmaul_enfeebling_roar::HandleDamage, EFFECT_2, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

/// Shield Charge - 158134
class spell_highmaul_pol_shield_charge : public SpellScript
{
    PrepareSpellScript(spell_highmaul_pol_shield_charge);

    WorldLocation m_Location;

    void HandleBeforeCast()
    {
        if (WorldLocation const* loc = GetExplTargetDest())
            m_Location = *loc;
    }

    void HandleAfterCast()
    {
        if (Unit* caster = GetCaster())
            caster->GetMotionMaster()->MoveCharge(m_Location.m_positionX, m_Location.m_positionY, m_Location.m_positionZ, SPEED_CHARGE, GetSpellInfo()->Id);
    }

    void Register() override
    {
        BeforeCast += SpellCastFn(spell_highmaul_pol_shield_charge::HandleBeforeCast);
        AfterCast += SpellCastFn(spell_highmaul_pol_shield_charge::HandleAfterCast);
    }
};

class spell_highmaul_pol_shield_charge_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_highmaul_pol_shield_charge_AuraScript);

    enum eSpell
    {
        ShieldChargeDamage = 158159
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

                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(caster, caster, radius);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(caster, targetList, check);
                    caster->VisitNearbyObject(radius, searcher);

                    for (Unit* itr : targetList)
                        caster->CastSpell(itr, ShieldChargeDamage, true);
                }

                m_DamageTimer = 500;
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_pol_shield_charge_AuraScript::OnUpdate);
    }
};

/// Aggressive Disposition - 157951
/// Fierce Disposition - 158016
/// Savage Disposition - 158017
class spell_highmaul_twin_ogron_dispositions : public AuraScript
{
    PrepareAuraScript(spell_highmaul_twin_ogron_dispositions);

    enum eSpells
    {
        AggressiveDisposition = 157951,
        FierceDisposition = 158016,
        SavageDisposition = 158017
    };

    uint32 m_CheckTimer;

    bool Load() override
    {
        m_CheckTimer = 200;
        return true;
    }

    void OnUpdate(uint32 diff)
    {
        if (m_CheckTimer)
        {
            if (m_CheckTimer <= diff)
            {
                if (Unit* caster = GetCaster())
                {
                    uint32 entry = caster->GetEntry() == Pol ? Phemos : Pol;
                    if (Creature* other = caster->FindNearestCreature(entry, 150.0f))
                    {
                        float distance = std::min(caster->GetDistance(other), 100.0f);
                        uint32 spellID = 0;

                        if (distance >= 50.0f)
                            spellID = SavageDisposition;
                        else if (distance >= 20.0f)
                            spellID = FierceDisposition;
                        else
                            spellID = AggressiveDisposition;

                        if ((int32)distance > DispositionPCT)
                            if (InstanceScript* instance = other->GetInstanceScript())
                                instance->SetData(TwinOgronAchievement, 1);

                        if (!caster->HasAura(spellID))
                            caster->CastSpell(caster, spellID, true);

                        if (!other->HasAura(spellID))
                            other->CastSpell(other, spellID, true);

                        if (Aura* casterAura = caster->GetAura(spellID))
                        {
                            if (AuraEffect* firstEffect = casterAura->GetEffect(EFFECT_0))
                                firstEffect->ChangeAmount((int32)distance);

                            if (AuraEffect* secondEffect = casterAura->GetEffect(EFFECT_1))
                                secondEffect->ChangeAmount((int32)distance);
                        }

                        if (Aura* otherAura = other->GetAura(spellID))
                        {
                            if (AuraEffect* firstEffect = otherAura->GetEffect(EFFECT_0))
                                firstEffect->ChangeAmount((int32)distance);

                            if (AuraEffect* secondEffect = otherAura->GetEffect(EFFECT_1))
                                secondEffect->ChangeAmount((int32)distance);
                        }

                        caster->SetPower(POWER_ALTERNATE, (int32)distance);
                        other->SetPower(POWER_ALTERNATE, (int32)distance);
                    }
                }

                m_CheckTimer = 200;
            }
            else
                m_CheckTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_twin_ogron_dispositions::OnUpdate);
    }
};

/// Pulverize (Wave 3) - 158420
class spell_highmaul_pulverize_third_wave : public SpellScript
{
    PrepareSpellScript(spell_highmaul_pulverize_third_wave);

    void HandleDamage(SpellEffIndex /*effectIndex*/)
    {
        if (WorldLocation const* loc = GetExplTargetDest())
        {
            if (Unit* target = GetHitUnit())
            {
                int32 damage = GetHitDamage();
                SetHitDamage(damage - CalculatePct(damage, (int32)std::min(100.0f, target->GetDistance(*loc))));
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_highmaul_pulverize_third_wave::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

/// Whirlwind - 157943
class spell_highmaul_phemos_whirlwind : public AuraScript
{
    PrepareAuraScript(spell_highmaul_phemos_whirlwind);

    enum eSpell
    {
        WeakenedDefenses = 159709
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, WeakenedDefenses, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_phemos_whirlwind::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_boss_twin_ogron()
{
    RegisterHighmaulCreatureAI(boss_twin_ogron_pol);
    RegisterHighmaulCreatureAI(boss_twin_ogron_phemos);

    RegisterAuraScript(spell_highmaul_warming_up);
    RegisterAuraScript(spell_highmaul_disposition);
    RegisterSpellAndAuraScriptPair(spell_highmaul_enfeebling_roar, spell_highmaul_enfeebling_roar_AuraScript);
    //RegisterSpellAndAuraScriptPair(spell_highmaul_pol_shield_charge, spell_highmaul_pol_shield_charge_AuraScript);
    RegisterAuraScript(spell_highmaul_twin_ogron_dispositions);
    RegisterSpellScript(spell_highmaul_pulverize_third_wave);
    RegisterAuraScript(spell_highmaul_phemos_whirlwind);
}
