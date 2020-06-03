////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"

float const minAllowedZ = 560.0f;

Position const gorianReaverPo = {4026.755f, 8584.76f, 572.6546f, 3.138298f};
Position const centerPos = {3917.63f, 8590.89f, 565.341f, 0.0f};

Position const volatileAnomalyPos[MaxIntervalles] =
{
    {3885.65f, 8557.80f, 565.34f, 0.747137f}, ///< Rune of Fortification
    {3890.47f, 8628.17f, 565.34f, 5.375480f}  ///< Rune of Replication
};

/// Imperator Mar'gok <Sorcerer King> - 77428
struct boss_imperator_margok : public BossAI
{
    enum eSpells
    {
        /// Cosmetic
        CosmeticSitThrone = 88648,
        TeleportOffThrone = 166090,
        TeleportToDisplacement = 164336,
        TeleportToFortification = 164751,
        TeleportToReplication = 164810,
        EncounterEvent = 181089,   ///< Sniffed, don't know why, related to phases switch ?
        TransitionVisualPhase2 = 176576,
        TransitionVisualPhase3 = 176578,
        TransitionVisualPhase4 = 176579,
        TransitionVisualPeriodic = 176580,
        PowerOfDisplacement = 158013,
        PowerOfFortification = 158012,
        PowerOfReplication = 157964,
        DisplacementTransform = 174020,
        FortificationTransform = 174022,
        DisplacementTransform2 = 174023,
        ArcaneProtection = 174057,
        AwakenRunestone = 157278,
        FortificationBaseVisual = 174043,
        FortificationAchievement = 143809,
        FortificationRuneActive = 166429,
        ReplicationBaseVisual = 174044,
        ReplicationAchievement = 166391,
        ReplicationRuneActive = 174085,
        SummonRuneVisual = 160351,
        SummonWarmage = 160366,
        VolatileAnomalies = 157265,
        ImperatorMargokBonus = 177528,
        /// Mark of Chaos
        MarkOfChaosAura = 158605,
        MarkOfChaosCosmetic = 164161,
        MarkOfChaosDisplacementAura = 164176,
        MarkOfChaosFortificationAura = 164178,
        FetterMarkOfChaosRootAura = 158619,
        MarkOfChaosReplicationAura = 164191,
        OrbsOfChaosDummyAura = 160447,
        /// AcceleratedAssault
        AcceleratedAssault = 159515,
        /// Arcane Aberration
        SummonArcaneAberrationCast = 156471,
        SummonArcaneAberrationCosmetic = 164318,   ///< Sniffed, but I don't know why
        SummonDisplacingArcaneAberration = 164299,
        SummonFortifiedArcaneAberration = 164301,
        SummonReplicatingArcaneAberration = 164303,
        /// Destructive Resonance
        DestructiveResonanceDebuff = 159200,
        DestructiveResonanceSearcher = 156467,
        DestructiveResonanceCosmetic = 164074,   ///< Sniffed, but I don't know why
        DestructiveResonanceSummon = 156734,
        DestructiveResonanceDisplacementSearch = 164075,
        DestructiveResonanceFortificationSearch = 164076,
        DestructiveResonanceReplicationSearch = 164077,
        /// Arcane Wrath
        ArcaneWrathSearcher = 156238,
        ArcaneWrathCosmetic = 163968,   ///< Sniffed, but I don't know why
        ArcaneWrathBranded = 156225,
        ArcaneWrathDisplacementSearcher = 163988,
        ArcaneWrathBrandedDisplacement = 164004,
        ArcaneWrathFortificationSearcher = 163989,
        ArcaneWrathBrandedFortification = 164005,
        ArcaneWrathReplicationSearcher = 163990,
        ArcaneWrathBrandedReplication = 164006,
        /// Force Nova
        ForceNovaCasting = 157349,   ///< CastTime
        ForceNovaScriptEffect = 164227,   ///< Sniffed, but I don't know why
        ForceNovaDummy = 157320,   ///< Visual effect of the nova
        ForceNovaDoT = 157353,
        ForceNovaKnockBack = 157325,
        ForceNovaDisplacement = 164232,   ///< CastTime
        ForceNovaDisplacementDummy = 164252,   ///< Visual effect of the nova
        ForceNovaAreaTrigger = 157327,   ///< Triggers knock back too
        ForceNovaFortificationCasting = 164235,   ///< Cast Time
        ForceNovaFortifiedPeriodicAura = 157323,   ///< 8s periodic for Phase 3
        ForceNovaFortificationDummy = 164253,
        ForceNovaReplicationCasting = 164240,   ///< CastTime
        ForceNovaReplicationDummy = 164254    ///< Visual effect
    };

    enum eEvents
    {
        /// Phase 1
        EventMarkOfChaos = 1,
        EventForceNova,
        EventArcaneWrath,
        EventDestructiveResonance,
        EventArcaneAberration,
        /// Phase 2
        EventMarkOfChaosDisplacement,
        EventForceNovaDisplacement,
        EventArcaneWrathDisplacement,
        EventDestructiveResonanceDisplacement,
        EventArcaneAberrationDisplacement,
        /// Phase 3
        EventMarkOfChaosFortification,
        EventForceNovaFortification,
        EventArcaneWrathFortification,
        EventDestructiveResonanceFortification,
        EventArcaneAberrationFortification,
        /// Phase 4
        EventMarkOfChaosReplication,
        EventForceNovaReplication,
        EventArcaneWrathReplication,
        EventDestructiveResonanceReplication,
        EventArcaneAberrationReplication,
        EventBerserk
    };

    enum eCosmeticEvents
    {
        EventSummonWarmages = 1,
        EventCheckPlayerZ
    };

    enum eActions
    {
        ActionIntro,
        ActionFinishIntro
    };

    enum eCreatures
    {
        SorcererKingVisualPoint = 89081,
        NpcRuneOfDisplacement = 77429,
        SLGGenericMoPLargeAoI = 68553,
        WarmageSummonStalker = 77682,
        GorianWarmage = 78121,
        VolatileAnomaly = 78077,
        GorianReaver = 78549,
        ArcaneRemnant = 79388
    };

    enum eTalks
    {
        Intro1,
        Intro2,
        Intro3,
        Aggro,
        ForceNova,
        MarkOfChaos,
        ArcaneWrath,
        ArcaneAberration,
        TalkRuneOfDisplacement,
        TalkRuneOfFortification1,
        TalkRuneOfFortification2,
        TalkRuneOfFortification3,
        TalkRuneOfFortification4,
        TalkRuneOfFortification5,
        TalkRuneOfReplication1,
        TalkRuneOfReplication2,
        TalkRuneOfReplication3,
        TalkRuneOfReplication4,
        Slay,
        Berserk,
        Death,
        Branded
    };

    enum eAnimKit
    {
        AnimKitFlyingRune = 6420
    };

    enum eMoves
    {
        MoveUp = 1,
        MoveDown
    };

    enum eDatas
    {
        /// Values datas
        BrandedStacks,
        PhaseID,
        OrbOfChaosAngle,
        /// Misc
        MaxVisualPoint = 8,
        MaxNovaPhase3 = 3
    };

    enum ePhases
    {
        MightOfTheCrown = 1,    ///< Phase 1: Might of the Crown
        RuneOfDisplacement,     ///< Phase 2: Rune of Displacement
        DormantRunestones,      ///< Intermission: Dormant Runestones
        RuneOfFortification,    ///< Phase 3: Rune of Fortification
        LineageOfPower,         ///< Intermission: Lineage of Power
        RuneOfReplication,      ///< Phase 4: Rune of Replication
        MaxPhases
    };


    boss_imperator_margok(Creature* creature) : BossAI(creature, BossImperatorMargok), m_Vehicle(creature->GetVehicleKit())
    {
        m_Instance = creature->GetInstanceScript();

        g_SwitchPhasePcts[MightOfTheCrown - 1] = 85;   ///< Phase One lasts from 100 % to 85 % of Mar'gok's health.
        g_SwitchPhasePcts[RuneOfDisplacement - 1] = 55;   ///< Phase Two lasts from 85 % to 55 % of Mar'gok's health.
        g_SwitchPhasePcts[DormantRunestones - 1] = -1;   ///< The First Transition Phase lasts for 1 minute after the end of Phase Two.
        g_SwitchPhasePcts[RuneOfFortification - 1] = 25;   ///< Phase Three lasts from 55 % of Mar'gok's health(end of the First Transition Phase) to 25 % of Mar'gok's health.
        g_SwitchPhasePcts[LineageOfPower - 1] = -1;   ///< The Second transition Phase lasts for 1 minute after the end of Phase Three.
        g_SwitchPhasePcts[RuneOfReplication - 1] = 0;    ///< Phase Four lasts from 25 % of Mar'gok's health(end of the Second Transition Phase) to Mar'gok's death.
    }

    EventMap m_Events;
    EventMap m_CosmeticEvents;

    bool m_InCombat;

    uint8 m_Phase;
    uint8 g_SwitchPhasePcts[MaxPhases - 1];

    ObjectGuid m_MeleeTargetGuid;

    bool m_IsInNova;
    uint32 m_NovaTime;
    Position m_NovaPos;

    /// Force Nova datas for Phase 3: Rune of Fortification
    uint8 m_NovaCount;
    bool m_IsInNovaPhase3[MaxNovaPhase3];
    uint32 m_NovaTimePhase3[MaxNovaPhase3];
    Position m_NovaPosPhase3[MaxNovaPhase3];

    uint8 m_OrbCount;

    Vehicle* m_Vehicle;
    InstanceScript* m_Instance;

    void Reset() override
    {
        KillAllDelayedEvents();

        m_Events.Reset();
        m_CosmeticEvents.Reset();

        summons.DespawnAll();

        m_InCombat = false;

        m_Phase = MightOfTheCrown;
        m_Events.SetPhase(m_Phase);

        m_IsInNova = false;
        m_NovaTime = 0;
        m_NovaPos = Position();
        m_NovaCount = 0;

        for (uint8 i = 0; i < MaxNovaPhase3; ++i)
        {
            m_IsInNovaPhase3[i] = false;
            m_NovaTimePhase3[i] = 0;
            m_NovaPosPhase3[i] = Position();
        }

        m_OrbCount = 0;

        me->RemoveAura(AcceleratedAssault);
        me->RemoveAura(Berserker);

        _Reset();

        AddDelayedEvent(200, [this]() -> void
        {
            me->CastSpell(me, CosmeticSitThrone, true);

            for (uint8 i = 0; i < MaxVisualPoint; ++i)
                if (Creature* creature = me->SummonCreature(SorcererKingVisualPoint, *me))
                    creature->EnterVehicle(me);
        });

        ResetRunes();
        DespawnAdds();
    }

    void JustReachedHome() override
    {
        m_Events.Reset();
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case ActionIntro:
                AddDelayedEvent(7 * IN_MILLISECONDS, [this]() -> void
                {
                    Talk(Intro1);
                });

                AddDelayedEvent(20 * IN_MILLISECONDS, [this]() -> void
                {
                    Talk(Intro2);
                });

                break;
            case ActionFinishIntro:
                Talk(Intro3);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (id)
        {
            case MoveUp:
                if (Creature* rune = me->FindNearestCreature(NpcRuneOfDisplacement, 40.0f))
                    rune->CastSpell(rune, TransitionVisualPeriodic, true);
                break;
            case MoveDown:
                AddDelayedEvent(200, [this]() -> void
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

                    me->SetAnimKitId(0);
                    me->SetAnimTier(0);
                    me->SetDisableGravity(false);
                    me->SetHover(false);
                    me->SetReactState(REACT_AGGRESSIVE);

                    me->RemoveAura(TransitionVisualPhase2);
                    me->RemoveAura(TransitionVisualPhase3);
                    me->RemoveAura(TransitionVisualPhase4);

                    if (Creature* rune = me->FindNearestCreature(NpcRuneOfDisplacement, 40.0f))
                    {
                        rune->RemoveAura(TransitionVisualPeriodic);

                        switch (m_Phase)
                        {
                            case DormantRunestones:
                                rune->CastSpell(rune, FortificationRuneActive, true);
                                break;
                            case LineageOfPower:
                                rune->CastSpell(rune, ReplicationRuneActive, true);
                                break;
                            default:
                                break;
                        }
                    }

                    if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                        AttackStart(target);
                });
                break;
            default:
                break;
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        _EnterCombat();

        Talk(Aggro);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        m_Events.RescheduleEvent(EventMarkOfChaos, 34 * IN_MILLISECONDS, 0, MightOfTheCrown);
        m_Events.RescheduleEvent(EventForceNova, 45 * IN_MILLISECONDS, 0, MightOfTheCrown);
        m_Events.RescheduleEvent(EventArcaneWrath, 6 * IN_MILLISECONDS, 0, MightOfTheCrown);
        m_Events.RescheduleEvent(EventDestructiveResonance, 15 * IN_MILLISECONDS, 0, MightOfTheCrown);
        m_Events.RescheduleEvent(EventArcaneAberration, 25 * IN_MILLISECONDS, 0, MightOfTheCrown);

        if (m_Instance->instance->IsLfr())
            m_Events.RescheduleEvent(EventBerserk, 900 * IN_MILLISECONDS);

        m_CosmeticEvents.RescheduleEvent(EventCheckPlayerZ, 1 * IN_MILLISECONDS);
    }

    void DamageDealt(Unit* victim, uint32& /*damage*/, DamageEffectType damageType) override
    {
        /// Accelerated Assault Icon Accelerated Assault is a stacking buff that Mar'gok applies to himself each time he performs consecutive attacks against the same target.
        /// Each stack of the buff increases Mar'gok's attack speed by 8%.
        if (damageType != DamageEffectType::DIRECT_DAMAGE)
            return;

        /// This is a new target, reset stacks
        if (victim->GetGUID() != m_MeleeTargetGuid)
        {
            me->RemoveAura(AcceleratedAssault);
            m_MeleeTargetGuid = victim->GetGUID();
        }
        /// Still same target, continue stacking
        else
            me->CastSpell(me, AcceleratedAssault, true);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        /// -1 means transition phase, 0 means last phase, until Mar'gok's death
        if (g_SwitchPhasePcts[m_Phase - 1] == 255 || !g_SwitchPhasePcts[m_Phase - 1])
            return;

        if (me->HealthBelowPctDamaged(g_SwitchPhasePcts[m_Phase - 1], damage))
        {
            ++m_Phase;

            switch (m_Phase)
            {
                case RuneOfDisplacement:
                    ScheduleSecondPhase();
                    break;
                case DormantRunestones:
                    ScheduleFirstTransitionPhase();
                    break;
                case LineageOfPower:
                    ScheduleSecondTransitionPhase();
                    break;
                default:
                    break;
            }

            m_Events.SetPhase(m_Phase);
        }
    }

    void KilledUnit(Unit* killed) override
    {
        if (killed->IsPlayer())
            Talk(Slay);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();

        m_Events.Reset();
        m_CosmeticEvents.Reset();

        Talk(Death);

        ResetRunes();
        DespawnAdds();

        summons.DespawnAll();

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosDisplacementAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosFortificationAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosReplicationAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(DestructiveResonanceDebuff);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBranded);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedDisplacement);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedFortification);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedReplication);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(FetterMarkOfChaosRootAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(OrbsOfChaosDummyAura);

        CastSpellToPlayers(me->GetMap(), me, ImperatorMargokBonus, true);
    }

    void EnterEvadeMode() override
    {
        me->SetAnimKitId(0);
        me->SetAnimTier(0);
        me->SetDisableGravity(false);
        me->SetHover(false);
        me->SetReactState(REACT_AGGRESSIVE);

        me->RemoveAllAuras();

        me->InterruptNonMeleeSpells(true);

        /// Just in case, to prevent the fail Return to Home
        me->ClearUnitState(UNIT_STATE_ROOT);
        me->ClearUnitState(UNIT_STATE_DISTRACTED);
        me->ClearUnitState(UNIT_STATE_STUNNED);

        CreatureAI::EnterEvadeMode();

        summons.DespawnAll();

        m_Instance->SetBossState(BossImperatorMargok, FAIL);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosDisplacementAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosFortificationAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfChaosReplicationAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(DestructiveResonanceDebuff);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBranded);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedDisplacement);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedFortification);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ArcaneWrathBrandedReplication);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(FetterMarkOfChaosRootAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(OrbsOfChaosDummyAura);
    }

    uint32 GetData(uint32 id) const override
    {
        switch (id)
        {
            case PhaseID:
                return m_Phase;
            case OrbOfChaosAngle:
                return m_OrbCount;
            default:
                break;
        }

        return 0;
    }

    void SetData(uint32 id, uint32 value) override
    {
        switch (id)
        {
            case OrbOfChaosAngle:
                m_OrbCount = value;
                break;
            default:
                break;
        }
    }

    //void AreaTriggerDespawned(AreaTrigger* p_AreaTrigger) override
    //{
    //    if (p_AreaTrigger->GetSpellId() == ForceNovaAreaTrigger)
    //    {
    //        if (m_Instance)
    //            m_Instance->DoRemoveForcedMovementsOnPlayers(p_AreaTrigger->GetGUID());
    //    }
    //}

    void SpellFinishCast(SpellInfo const* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            case ForceNovaCasting:
            {
                /// This spell should knock back players in melee range
                me->CastSpell(me, ForceNovaKnockBack, true);
                me->CastSpell(me, ForceNovaDummy, true);

                m_IsInNova = true;
                m_NovaTime = 0;
                m_NovaPos = *me;

                /// Force Nova has a radius of 100 yards and moves with a speed of 7 yards per second
                uint32 time = uint32(float(100.0f / 7.0f) * float(IN_MILLISECONDS));
                AddDelayedEvent(time, [this]() -> void
                {
                    m_IsInNova = false;
                    m_NovaTime = 0;
                    m_NovaPos = Position();

                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(ForceNovaDoT);
                });

                break;
            }
            case DestructiveResonanceSearcher:
            case DestructiveResonanceDisplacementSearch:
            case DestructiveResonanceFortificationSearch:
            case DestructiveResonanceReplicationSearch:
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, -10.0f, true);
                if (target == nullptr || me->GetDistance(target) >= 100.0f)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 2, 60.0f, true);

                if (target != nullptr)
                    me->CastSpell(*target, DestructiveResonanceSummon, true);

                break;
            }
            case ArcaneWrathSearcher:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, 60.0f))
                    me->CastSpell(target, ArcaneWrathBranded, true);

                break;
            case ArcaneWrathDisplacementSearcher:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, 60.0f))
                    me->CastSpell(target, ArcaneWrathBrandedDisplacement, true);

                break;
            case ForceNovaDisplacement:
            {
                /// This spell should knock back players in melee range
                me->CastSpell(me, ForceNovaAreaTrigger, true);
                me->CastSpell(me, ForceNovaDisplacementDummy, true);

                m_IsInNova = true;
                m_NovaTime = 0;
                m_NovaPos = *me;

                /// Force Nova has a radius of 100 yards and moves with a speed of 7 yards per second
                uint32 time = uint32(float(100.0f / 7.0f) * float(IN_MILLISECONDS));
                AddDelayedEvent(time, [this]() -> void
                {
                    m_IsInNova = false;
                    m_NovaTime = 0;
                    m_NovaPos = Position();

                    if (m_Instance)
                        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ForceNovaDoT);
                });

                break;
            }
            case AwakenRunestone:
            {
                std::list<Creature*> triggerList;
                me->GetCreatureListWithEntryInGrid(triggerList, SLGGenericMoPLargeAoI, 30.0f);

                /// Should have only two triggers, the base one and the large one
                for (Creature* trigger : triggerList)
                {
                    switch (m_Phase)
                    {
                        case DormantRunestones:
                            /// Small gaze for base visual
                            if (trigger->HasAura(FortificationBaseVisual))
                                continue;

                            /// Big gaze for large visual during activation
                            trigger->CastSpell(trigger, FortificationAchievement, true);
                            trigger->CastSpell(trigger, VolatileAnomalies, true);
                            break;
                        case LineageOfPower:
                            /// Small gaze for base visual
                            if (trigger->HasAura(ReplicationBaseVisual))
                                continue;

                            /// Big gaze for large visual during activation
                            trigger->CastSpell(trigger, ReplicationAchievement, true);
                            trigger->CastSpell(trigger, VolatileAnomalies, true);
                            break;
                        default:
                            break;
                    }

                    break;
                }

                break;
            }
            case ArcaneWrathFortificationSearcher:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, 60.0f))
                    me->CastSpell(target, ArcaneWrathBrandedFortification, true);

                break;
            case ForceNovaFortificationCasting:
                me->CastSpell(me, ForceNovaFortifiedPeriodicAura, true);
                break;
            case ForceNovaFortificationDummy:
            {
                me->CastSpell(me, ForceNovaKnockBack, true);

                m_IsInNovaPhase3[m_NovaCount] = true;
                m_NovaTimePhase3[m_NovaCount] = 0;
                m_NovaPosPhase3[m_NovaCount] = *me;

                /// Force Nova has a radius of 100 yards and moves with a speed of 7 yards per second
                uint32 time = uint32(float(100.0f / 7.0f) * float(IN_MILLISECONDS));
                /// Must save the current value
                uint8 count = m_NovaCount;
                AddDelayedEvent(time, [this, count]() -> void
                {
                    m_IsInNovaPhase3[count] = false;
                    m_NovaTimePhase3[count] = 0;
                    m_NovaPosPhase3[count] = Position();

                    if (m_Instance && count >= (MaxNovaPhase3 - 1))
                    {
                        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ForceNovaDoT);
                        m_NovaCount = 0;
                    }
                });

                ++m_NovaCount;
                break;
            }
            case ArcaneWrathReplicationSearcher:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, 60.0f))
                    me->CastSpell(target, ArcaneWrathBrandedReplication, true);

                break;
            case ForceNovaReplicationCasting:
            {
                /// This spell should knock back players in melee range
                me->CastSpell(me, ForceNovaKnockBack, true);
                me->CastSpell(me, ForceNovaReplicationDummy, true);

                m_IsInNova = true;
                m_NovaTime = 0;
                m_NovaPos = *me;

                /// Force Nova has a radius of 100 yards and moves with a speed of 7 yards per second
                uint32 time = uint32(float(100.0f / 7.0f) * float(IN_MILLISECONDS));
                AddDelayedEvent(time, [this]() -> void
                {
                    m_IsInNova = false;
                    m_NovaTime = 0;
                    m_NovaPos = Position();

                    if (m_Instance)
                        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ForceNovaDoT);
                });
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
            case ArcaneWrathBranded:
            case ArcaneWrathBrandedDisplacement:
            case ArcaneWrathBrandedFortification:
            case ArcaneWrathBrandedReplication:
                Talk(Branded, target->GetGUID());
                break;
            case MarkOfChaosDisplacementAura:
                /// In addition to Mark of Chaos' normal effects, the target is teleported to a random location.
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 2, -10.0f))
                    target->NearTeleportTo(*target);

                break;
            case MarkOfChaosFortificationAura:
                me->CastSpell(target, FetterMarkOfChaosRootAura, true);
                break;
            case MarkOfChaosReplicationAura:
                m_OrbCount = 0;
                me->CastSpell(target, OrbsOfChaosDummyAura, true);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!m_InCombat)
        {
            if (Player* player = me->SelectNearestPlayerNotGM(20.0f))
            {
                me->CastSpell(me, TeleportOffThrone, true);
                m_InCombat = true;

                ObjectGuid guid = player->GetGUID();
                AddDelayedEvent(50, [this, guid]() -> void
                {
                    if (Player* player = Player::GetPlayer(*me, guid))
                        AttackStart(player);
                });
            }
        }

        if (m_IsInNova || (m_NovaCount > 0 && m_IsInNovaPhase3[m_NovaCount - 1]))
            UpdateNovaTargets(diff);

        m_CosmeticEvents.Update(diff);

        switch (m_CosmeticEvents.ExecuteEvent())
        {
            case EventSummonWarmages:
            {
                std::list<Creature*> triggerList;
                me->GetCreatureListWithEntryInGrid(triggerList, WarmageSummonStalker, 150.0f);

                for (Creature* stalker : triggerList)
                    stalker->CastSpell(stalker, SummonWarmage, true);

                break;
            }
            case EventCheckPlayerZ:
            {
                std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
                for (HostileReference* ref : threatList)
                {
                    if (Player* player = Player::GetPlayer(*me, ref->getUnitGuid()))
                        if (player->GetPositionZ() <= minAllowedZ)
                            player->NearTeleportTo(centerPos);
                }

                m_CosmeticEvents.RescheduleEvent(EventCheckPlayerZ, 1 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            /// Phase 1
            case EventMarkOfChaos:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, MarkOfChaosAura, false);
                    me->CastSpell(target, MarkOfChaosCosmetic, true);
                }

                Talk(MarkOfChaos);
                m_Events.RescheduleEvent(EventMarkOfChaos, 51 * IN_MILLISECONDS, 0, MightOfTheCrown);
                break;
            case EventForceNova:
                me->CastSpell(me, ForceNovaCasting, false);
                me->CastSpell(me, ForceNovaScriptEffect, true);

                Talk(ForceNova);

                m_Events.RescheduleEvent(EventForceNova, 45 * IN_MILLISECONDS, 0, MightOfTheCrown);
                break;
            case EventArcaneWrath:
                Talk(ArcaneWrath);
                me->CastSpell(me, ArcaneWrathSearcher, false);
                me->CastSpell(me, ArcaneWrathCosmetic, true);
                m_Events.RescheduleEvent(EventArcaneWrath, 50 * IN_MILLISECONDS, 0, MightOfTheCrown);
                break;
            case EventDestructiveResonance:
                me->CastSpell(me, DestructiveResonanceSearcher, false);
                me->CastSpell(me, DestructiveResonanceCosmetic, true);
                m_Events.RescheduleEvent(EventDestructiveResonance, 15 * IN_MILLISECONDS, 0, MightOfTheCrown);
                break;
            case EventArcaneAberration:
                me->CastSpell(me, SummonArcaneAberrationCast, false);
                me->CastSpell(me, SummonArcaneAberrationCosmetic, true);
                Talk(ArcaneAberration);
                m_Events.RescheduleEvent(EventArcaneAberration, 45 * IN_MILLISECONDS, 0, MightOfTheCrown);
                break;
                /// Phase 2
            case EventMarkOfChaosDisplacement:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, MarkOfChaosDisplacementAura, false);
                    me->CastSpell(target, MarkOfChaosCosmetic, true);
                }

                Talk(MarkOfChaos);
                m_Events.RescheduleEvent(EventMarkOfChaosDisplacement, 51 * IN_MILLISECONDS, 0, RuneOfDisplacement);
                break;
            case EventForceNovaDisplacement:
                me->CastSpell(me, ForceNovaDisplacement, false);
                me->CastSpell(me, ForceNovaScriptEffect, true);

                Talk(ForceNova);

                m_Events.RescheduleEvent(EventForceNovaDisplacement, 45 * IN_MILLISECONDS, 0, RuneOfDisplacement);
                break;
            case EventArcaneWrathDisplacement:
                Talk(ArcaneWrath);
                me->CastSpell(me, ArcaneWrathDisplacementSearcher, false);
                me->CastSpell(me, ArcaneWrathCosmetic, true);
                m_Events.RescheduleEvent(EventArcaneWrathDisplacement, 50 * IN_MILLISECONDS, 0, RuneOfDisplacement);
                break;
            case EventDestructiveResonanceDisplacement:
                me->CastSpell(me, DestructiveResonanceDisplacementSearch, false);
                me->CastSpell(me, DestructiveResonanceCosmetic, true);
                m_Events.RescheduleEvent(EventDestructiveResonanceDisplacement, 15 * IN_MILLISECONDS, 0, RuneOfDisplacement);
                break;
            case EventArcaneAberrationDisplacement:
                me->CastSpell(me, SummonDisplacingArcaneAberration, false);
                me->CastSpell(me, SummonArcaneAberrationCosmetic, true);
                Talk(ArcaneAberration);
                m_Events.RescheduleEvent(EventArcaneAberrationDisplacement, 45 * IN_MILLISECONDS, 0, RuneOfDisplacement);
                break;
                /// Phase 3
            case EventMarkOfChaosFortification:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, MarkOfChaosFortificationAura, false);
                    me->CastSpell(target, MarkOfChaosCosmetic, true);
                }

                Talk(MarkOfChaos);
                m_Events.RescheduleEvent(EventMarkOfChaosFortification, 51 * IN_MILLISECONDS, 0, RuneOfFortification);
                break;
            case EventForceNovaFortification:
                me->CastSpell(me, ForceNovaFortificationCasting, false);
                me->CastSpell(me, ForceNovaScriptEffect, true);

                Talk(ForceNova);

                m_Events.RescheduleEvent(EventForceNovaFortification, 45 * IN_MILLISECONDS, 0, RuneOfFortification);
                break;
            case EventArcaneWrathFortification:
                Talk(ArcaneWrath);
                me->CastSpell(me, ArcaneWrathFortificationSearcher, false);
                me->CastSpell(me, ArcaneWrathCosmetic, true);
                m_Events.RescheduleEvent(EventArcaneWrathFortification, 50 * IN_MILLISECONDS, 0, RuneOfFortification);
                break;
            case EventDestructiveResonanceFortification:
                me->CastSpell(me, DestructiveResonanceFortificationSearch, false);
                me->CastSpell(me, DestructiveResonanceCosmetic, true);
                m_Events.RescheduleEvent(EventDestructiveResonanceFortification, 15 * IN_MILLISECONDS, 0, RuneOfFortification);
                break;
            case EventArcaneAberrationFortification:
                me->CastSpell(me, SummonFortifiedArcaneAberration, false);
                me->CastSpell(me, SummonArcaneAberrationCosmetic, true);
                Talk(ArcaneAberration);
                m_Events.RescheduleEvent(EventArcaneAberrationFortification, 45 * IN_MILLISECONDS, 0, RuneOfFortification);
                break;
                /// Phase 4
            case EventMarkOfChaosReplication:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, MarkOfChaosReplicationAura, false);
                    me->CastSpell(target, MarkOfChaosCosmetic, true);
                }

                Talk(MarkOfChaos);
                m_Events.RescheduleEvent(EventMarkOfChaosReplication, 51 * IN_MILLISECONDS, 0, RuneOfReplication);
                break;
            case EventForceNovaReplication:
                me->CastSpell(me, ForceNovaReplicationCasting, false);
                me->CastSpell(me, ForceNovaScriptEffect, true);

                Talk(ForceNova);

                m_Events.RescheduleEvent(EventForceNovaReplication, 45 * IN_MILLISECONDS, 0, RuneOfReplication);
                break;
            case EventArcaneWrathReplication:
                Talk(ArcaneWrath);
                me->CastSpell(me, ArcaneWrathReplicationSearcher, false);
                me->CastSpell(me, ArcaneWrathCosmetic, true);
                m_Events.RescheduleEvent(EventArcaneWrathReplication, 50 * IN_MILLISECONDS, 0, RuneOfReplication);
                break;
            case EventDestructiveResonanceReplication:
                me->CastSpell(me, DestructiveResonanceReplicationSearch, false);
                me->CastSpell(me, DestructiveResonanceCosmetic, true);
                m_Events.RescheduleEvent(EventDestructiveResonanceReplication, 15 * IN_MILLISECONDS, 0, RuneOfReplication);
                break;
            case EventArcaneAberrationReplication:
                me->CastSpell(me, SummonReplicatingArcaneAberration, false);
                me->CastSpell(me, SummonArcaneAberrationCosmetic, true);
                Talk(ArcaneAberration);
                m_Events.RescheduleEvent(EventArcaneAberrationReplication, 45 * IN_MILLISECONDS, 0, RuneOfReplication);
                break;
            case EventBerserk:
                Talk(Berserk);
                me->CastSpell(me, Berserker, true);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }

    void UpdateNovaTargets(uint32 const diff)
    {
        /// Used for Phase 1 & Phase 2
        if (m_IsInNova)
        {
            m_NovaTime += diff;

            float l_YardsPerMs = 7.0f / (float)IN_MILLISECONDS;
            /// Base min radius is 10 yards, which is increased depending on time passed
            float minRadius = 8.0f;
            float innerRange = 6.0f;

            minRadius += (l_YardsPerMs * m_NovaTime);

            ObjectGuid triggerGuid;
            AreaTrigger* trigger = nullptr;
            //if (trigger = me->GetAreaTrigger(ForceNovaAreaTrigger))
            //    triggerGuid = trigger->GetGUID();

            std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
            for (HostileReference* ref : threatList)
            {
                if (Player* player = Player::GetPlayer(*me, ref->getUnitGuid()))
                {
                    if (player->GetDistance(m_NovaPos) >= (minRadius - innerRange) && player->GetDistance(m_NovaPos) <= minRadius)
                    {
                        if (!player->HasAura(ForceNovaDoT))
                            me->CastSpell(player, ForceNovaDoT, true);

                        /// In addition to Force Nova's normal effects, it now also pushes players away as the nova moves outwards.
                        //if (m_Phase == RuneOfDisplacement && triggerGuid && trigger)
                        //{
                        //    if (!player->HasMovementForce(triggerGuid))
                        //        player->SendApplyMovementForce(triggerGuid, true, *trigger, -5.5f, 1);
                        //}
                    }
                    else
                    {
                        if (player->HasAura(ForceNovaDoT))
                            player->RemoveAura(ForceNovaDoT);

                        /// In addition to Force Nova's normal effects, it now also pushes players away as the nova moves outwards.
                        //if (m_Phase == RuneOfDisplacement && triggerGuid)
                        //{
                        //    if (player->HasMovementForce(triggerGuid))
                        //        player->SendApplyMovementForce(triggerGuid, false, Position());
                        //}
                    }
                }
            }
        }
        /// Phase 3
        else
        {
            std::set<ObjectGuid> affectedPlayers;

            for (uint8 i = 0; i < MaxNovaPhase3; ++i)
            {
                if (m_IsInNovaPhase3[i])
                {
                    m_NovaTimePhase3[i] += diff;

                    float l_YardsPerMs = 7.0f / (float)IN_MILLISECONDS;
                    /// Base min radius is 10 yards, which is increased depending on time passed
                    float minRadius = 8.0f;
                    float innerRange = 6.0f;

                    minRadius += (l_YardsPerMs * m_NovaTimePhase3[i]);

                    std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
                    for (HostileReference* ref : threatList)
                        if (Player* player = Player::GetPlayer(*me, ref->getUnitGuid()))
                            if (player->GetDistance(m_NovaPosPhase3[i]) >= (minRadius - innerRange) && player->GetDistance(m_NovaPosPhase3[i]) <= minRadius)
                                affectedPlayers.insert(player->GetGUID());
                }
            }

            std::list<HostileReference*> threatList = me->getThreatManager().getThreatList();
            for (HostileReference* ref : threatList)
            {
                if (Player* player = Player::GetPlayer(*me, ref->getUnitGuid()))
                {
                    if (affectedPlayers.find(player->GetGUID()) != affectedPlayers.end())
                    {
                        if (!player->HasAura(ForceNovaDoT))
                            me->CastSpell(player, ForceNovaDoT, true);
                    }
                    else
                    {
                        if (player->HasAura(ForceNovaDoT))
                            player->RemoveAura(ForceNovaDoT);
                    }
                }
            }
        }
    }

    void ScheduleSecondPhase()
    {
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

        Talk(TalkRuneOfDisplacement);

        me->CastSpell(me, EncounterEvent, true);
        me->CastSpell(me, TeleportToDisplacement, true);

        AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, TransitionVisualPhase2, true);
            me->SetAnimKitId(AnimKitFlyingRune);

            me->SetAnimTier(3);
            me->SetDisableGravity(true);
            me->SetHover(true);

            Position pos = *me;
            pos.m_positionZ += 16.0f;
            me->GetMotionMaster()->MovePoint(MoveUp, pos);
        });

        AddDelayedEvent(11 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, PowerOfDisplacement, true);
            me->CastSpell(me, DisplacementTransform, true);

            Position pos = *me;
            pos.m_positionZ -= 16.0f;
            me->GetMotionMaster()->MovePoint(MoveDown, pos);

            uint32 time = m_Events.GetEventTime(EventMarkOfChaos);
            m_Events.RescheduleEvent(EventMarkOfChaosDisplacement, time, 0, RuneOfDisplacement);

            time = m_Events.GetEventTime(EventForceNova);
            m_Events.RescheduleEvent(EventForceNovaDisplacement, time, 0, RuneOfDisplacement);

            time = m_Events.GetEventTime(EventArcaneWrath);
            m_Events.RescheduleEvent(EventArcaneWrathDisplacement, time, 0, RuneOfDisplacement);

            time = m_Events.GetEventTime(EventDestructiveResonance);
            m_Events.RescheduleEvent(EventDestructiveResonanceDisplacement, time, 0, RuneOfDisplacement);

            time = m_Events.GetEventTime(EventArcaneAberration) + 13 * IN_MILLISECONDS;
            m_Events.RescheduleEvent(EventArcaneAberrationDisplacement, time, 0, RuneOfDisplacement);
        });
    }

    void ScheduleFirstTransitionPhase()
    {
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

        me->CastSpell(me, EncounterEvent, true);
        me->CastSpell(me, TeleportToFortification, true);

        Talk(TalkRuneOfFortification1);

        AddDelayedEvent(7 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, ArcaneProtection, true);
            me->CastSpell(me, TransitionVisualPhase3, true);

            me->SetAnimKitId(AnimKitFlyingRune);

            me->SetAnimTier(3);
            me->SetDisableGravity(true);
            me->SetHover(true);

            Position pos = *me;
            pos.m_positionZ += 16.0f;
            me->GetMotionMaster()->MovePoint(MoveUp, pos);
        });

        AddDelayedEvent(10 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfFortification2);

            me->CastSpell(me, AwakenRunestone, false);

            std::list<Creature*> triggerList;
            me->GetCreatureListWithEntryInGrid(triggerList, WarmageSummonStalker, 150.0f);

            for (Creature* stalker : triggerList)
                stalker->CastSpell(stalker, SummonRuneVisual, true);

            m_CosmeticEvents.RescheduleEvent(EventSummonWarmages, 1 * IN_MILLISECONDS);
        });

        AddDelayedEvent(25 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfFortification3);
        });

        AddDelayedEvent(56 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfFortification4);
        });

        AddDelayedEvent(70 * IN_MILLISECONDS, [this]() -> void
        {
            me->RemoveAura(AwakenRunestone);

            me->RemoveAura(DisplacementTransform);
            me->CastSpell(me, FortificationTransform, true);

            me->RemoveAura(PowerOfDisplacement);
            me->CastSpell(me, PowerOfFortification, true);

            Position pos = *me;
            pos.m_positionZ -= 16.0f;
            me->GetMotionMaster()->MovePoint(MoveDown, pos);
        });

        AddDelayedEvent(75 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfFortification5);

            ++m_Phase;

            m_Events.SetPhase(m_Phase);

            uint32 time = m_Events.GetEventTime(EventMarkOfChaosDisplacement);
            m_Events.RescheduleEvent(EventMarkOfChaosFortification, time, 0, RuneOfFortification);

            time = m_Events.GetEventTime(EventForceNovaDisplacement);
            m_Events.RescheduleEvent(EventForceNovaFortification, time, 0, RuneOfFortification);

            time = m_Events.GetEventTime(EventArcaneWrathDisplacement);
            m_Events.RescheduleEvent(EventArcaneWrathFortification, time, 0, RuneOfFortification);

            time = m_Events.GetEventTime(EventDestructiveResonanceDisplacement);
            m_Events.RescheduleEvent(EventDestructiveResonanceFortification, time, 0, RuneOfFortification);

            time = m_Events.GetEventTime(EventArcaneAberrationDisplacement);
            m_Events.RescheduleEvent(EventArcaneAberrationFortification, time, 0, RuneOfFortification);
        });
    }

    void ScheduleSecondTransitionPhase()
    {
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

        me->CastSpell(me, EncounterEvent, true);
        me->CastSpell(me, TeleportToReplication, true);

        Talk(TalkRuneOfReplication1);

        AddDelayedEvent(7 * IN_MILLISECONDS, [this]() -> void
        {
            me->CastSpell(me, ArcaneProtection, true);
            me->CastSpell(me, TransitionVisualPhase4, true);

            me->SetAnimKitId(AnimKitFlyingRune);

            me->SetAnimTier(3);
            me->SetDisableGravity(true);
            me->SetHover(true);

            Position pos = *me;
            pos.m_positionZ += 16.0f;
            me->GetMotionMaster()->MovePoint(MoveUp, pos);
        });

        AddDelayedEvent(10 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfReplication2);

            me->CastSpell(me, AwakenRunestone, false);

            std::list<Creature*> triggerList;
            me->GetCreatureListWithEntryInGrid(triggerList, WarmageSummonStalker, 150.0f);

            for (Creature* stalker : triggerList)
                stalker->CastSpell(stalker, SummonRuneVisual, true);

            m_CosmeticEvents.RescheduleEvent(EventSummonWarmages, 1 * IN_MILLISECONDS);

            me->SummonCreature(GorianReaver, gorianReaverPo);
        });

        AddDelayedEvent(41 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfReplication3);
        });

        AddDelayedEvent(70 * IN_MILLISECONDS, [this]() -> void
        {
            me->RemoveAura(AwakenRunestone);

            me->RemoveAura(FortificationTransform);
            me->RemoveAura(ArcaneProtection);
            me->CastSpell(me, DisplacementTransform2, true);

            me->RemoveAura(PowerOfFortification);
            me->CastSpell(me, PowerOfReplication, true);

            Position pos = *me;
            pos.m_positionZ -= 16.0f;
            me->GetMotionMaster()->MovePoint(MoveDown, pos);
        });

        AddDelayedEvent(75 * IN_MILLISECONDS, [this]() -> void
        {
            Talk(TalkRuneOfReplication4);

            ++m_Phase;

            m_Events.SetPhase(m_Phase);

            uint32 time = m_Events.GetEventTime(EventMarkOfChaosFortification);
            m_Events.RescheduleEvent(EventMarkOfChaosReplication, time, 0, RuneOfReplication);

            time = m_Events.GetEventTime(EventForceNovaFortification);
            m_Events.RescheduleEvent(EventForceNovaReplication, time, 0, RuneOfReplication);

            time = m_Events.GetEventTime(EventArcaneWrathFortification);
            m_Events.RescheduleEvent(EventArcaneWrathReplication, time, 0, RuneOfReplication);

            time = m_Events.GetEventTime(EventDestructiveResonanceFortification);
            m_Events.RescheduleEvent(EventDestructiveResonanceReplication, time, 0, RuneOfReplication);

            time = m_Events.GetEventTime(EventArcaneAberrationFortification);
            m_Events.RescheduleEvent(EventArcaneAberrationReplication, time, 0, RuneOfReplication);
        });
    }

    void ResetRunes()
    {
        std::list<Creature*> triggerList;
        me->GetCreatureListWithEntryInGrid(triggerList, SLGGenericMoPLargeAoI, 200.0f);

        for (Creature* trigger : triggerList)
        {
            trigger->RemoveAura(FortificationAchievement);
            trigger->RemoveAura(ReplicationAchievement);
            trigger->RemoveAura(VolatileAnomalies);
        }

        triggerList.clear();

        me->GetCreatureListWithEntryInGrid(triggerList, NpcRuneOfDisplacement, 200.0f);

        for (Creature* trigger : triggerList)
        {
            trigger->RemoveAura(FortificationRuneActive);
            trigger->RemoveAura(ReplicationRuneActive);
        }
    }

    void DespawnAdds()
    {
        std::list<Creature*> creatureList;
        me->GetCreatureListWithEntryInGrid(creatureList, GorianWarmage, 200.0f);

        for (Creature* itr : creatureList)
            itr->DespawnOrUnsummon();

        creatureList.clear();
        me->GetCreatureListWithEntryInGrid(creatureList, VolatileAnomaly, 200.0f);

        for (Creature* itr : creatureList)
            itr->DespawnOrUnsummon();

        creatureList.clear();
        me->GetCreatureListWithEntryInGrid(creatureList, ArcaneRemnant, 200.0f);

        for (Creature* itr : creatureList)
            itr->DespawnOrUnsummon();
    }
};

/// Rune of Displacement - 77429
struct npc_highmaul_rune_of_displacement : public ScriptedAI
{
    npc_highmaul_rune_of_displacement(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override { }

    void UpdateAI(uint32 diff) override { }
};


/// Arcane Aberration - 77809
/// Displacing Arcane Aberration - 77879
/// Fortified Arcane Aberration - 77878
/// Replicating Arcane Aberration - 77877
struct npc_highmaul_arcane_aberration : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        /// Phase 1
        CollapsingEntityAura = 158703,
        CollapsingEntityTrigger = 158705,
        ReverseDeath = 157099,
        /// Phase 2
        DisplacerCharge = 157254,
        ImpactfulPulse = 160367,
        /// Phase  3
        Fortified = 157252,
        /// Phase 4
        Replicator = 157249,
        Replicate = 160341
    };

    enum eCreatures
    {
        ClassicArcaneAberration = 77809,
        DisplacingArcaneAberration = 77879,
        FortifiedArcaneAberration = 77878,
        ReplicatingArcaneAberration = 77877
    };

    enum eVisuals
    {
        ClassicAberration = 42489,
        DisplacingAberration = 45790,
        FortifiedAberration = 45791,
        ReplicatingAberration = 45792
    };

    enum eData
    {
        MaxRemnants = 7
    };

    npc_highmaul_arcane_aberration(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);

        me->CastSpell(me, CollapsingEntityAura, true);
        me->CastSpell(me, ReverseDeath, true);

        switch (me->GetEntry())
        {
            case DisplacingArcaneAberration:
                me->CastSpell(me, DisplacerCharge, true);
                break;
            case FortifiedArcaneAberration:
                /// The Arcane Aberrations are now Fortified Icon Fortified, having 75% more health
                /// And being immune to all crowd-control effects.
                me->CastSpell(me, Fortified, true);
                break;
            case ReplicatingArcaneAberration:
                me->CastSpell(me, Replicator, true);
                break;
            default:
                break;
        }

        AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
        {
            me->SetReactState(REACT_AGGRESSIVE);

            if (Player* player = me->FindNearestPlayer(50.0f))
                AttackStart(player);
        });
    }

    void SpellFinishCast(SpellInfo const* spellInfo) override
    {
        /// I don't know why, but it's sent twice on retail servers
        if (spellInfo->Id == CollapsingEntityTrigger)
        {
            switch (me->GetEntry())
            {
                case ClassicArcaneAberration:
                    me->SendPlaySpellVisualKit(ClassicAberration, 0);
                    me->SendPlaySpellVisualKit(ClassicAberration, 0);
                    break;
                case DisplacingArcaneAberration:
                    me->SendPlaySpellVisualKit(DisplacingAberration, 0);
                    me->SendPlaySpellVisualKit(DisplacingAberration, 0);
                    break;
                case FortifiedArcaneAberration:
                    me->SendPlaySpellVisualKit(FortifiedAberration, 0);
                    me->SendPlaySpellVisualKit(FortifiedAberration, 0);
                    break;
                case ReplicatingArcaneAberration:
                    me->SendPlaySpellVisualKit(ReplicatingAberration, 0);
                    me->SendPlaySpellVisualKit(ReplicatingAberration, 0);
                    break;
                default:
                    break;
            }
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        switch (me->GetEntry())
        {
            case DisplacingArcaneAberration:
                me->CastSpell(me, ImpactfulPulse, true);
                break;
            case ReplicatingArcaneAberration:
                /// Upon death, Replicating Arcane Aberrations split into 7 Arcane Remnants.
                for (uint8 i = 0; i < MaxRemnants; ++i)
                    me->CastSpell(me, Replicate, true);

                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

/// Destructive Resonance - 77637
struct npc_highmaul_destructive_resonance : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        /// Destructive Resonance
        DestructiveResonanceAura = 156639,
        BaseMineVisualAura = 156961,
        DestructiveResonanceDebuff = 159200,
        DestructiveResonanceDamage = 156673,
        /// Destructive Resonance: Displacement
        DestructiveResonanceDisplacementAura = 164086,
        DisplacementMineVisualAura = 156959,
        DisplacementVisualAura = 156983,
        DestructiveResonanceGrowScaleAura = 156941,
        /// Destructive Resonance: Fortification
        DestructiveResonanceFortificationAura = 164088,
        FortificationMineVisualAura = 156958,
        FortificationVisualAura = 156982,
        /// Destructive Resonance: Replication
        DestructiveResonanceReplicationAura = 164096,
        ReplicationMineVisualAura = 156957,
        ReplicationVisualAura = 156977,
        DestructiveResonanceReplicating = 156799
    };

    enum eVisuals
    {
        DestructiveResonanceDespawn = 41168,
        DestructiveResonanceDisplacementDespawn = 45691,
        DestructiveResonanceFortificationDesp = 45692,
        DestructiveResonanceReplicationDespawn = 45693
    };

    enum eDatas
    {
        PhaseID = 1,
        /// Misc
        ReplicationDupliCount = 2
    };

    enum ePhases
    {
        MightOfTheCrown = 1,    ///< Phase 1: Might of the Crown
        RuneOfDisplacement,     ///< Phase 2: Rune of Displacement
        DormantRunestones,      ///< Intermission: Dormant Runestones
        RuneOfFortification,    ///< Phase 3: Rune of Fortification
        LineageOfPower,         ///< Intermission: Lineage of Power
        RuneOfReplication,      ///< Phase 4: Rune of Replication
        MaxPhases
    };

    enum eTalk
    {
        DestructiveResonance
    };


    npc_highmaul_destructive_resonance(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_CanExplode = false;

        if (InstanceScript* l_Script = creature->GetInstanceScript())
        {
            if (Creature* margok = Creature::GetCreature(*creature, l_Script->GetGuidData(ImperatorMargok)))
            {
                if (margok->IsAIEnabled)
                    m_Phase = margok->AI()->GetData(PhaseID);
            }
        }
    }

    bool m_CanExplode;

    uint32 m_SpawnTime;

    uint8 m_Phase;

    void Reset() override
    {
        me->AddUnitState(UNIT_STATE_ROOT);

        me->SetReactState(REACT_PASSIVE);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

        switch (m_Phase)
        {
            case MightOfTheCrown:
            {
                me->CastSpell(me, DestructiveResonanceAura, true);

                AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
                {
                    me->RemoveAura(DestructiveResonanceAura);

                    me->CastSpell(me, BaseMineVisualAura, true);

                    m_CanExplode = true;
                });

                break;
            }
            case RuneOfDisplacement:
            {
                me->CastSpell(me, DestructiveResonanceDisplacementAura, true);

                AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
                {
                    me->RemoveAura(DestructiveResonanceDisplacementAura);

                    me->CastSpell(me, DisplacementMineVisualAura, true);
                    me->CastSpell(me, DisplacementVisualAura, true);

                    me->CastSpell(me, DestructiveResonanceGrowScaleAura, true);
                    me->SetUInt32Value(UNIT_FIELD_SCALE_DURATION, 30 * IN_MILLISECONDS);

                    m_CanExplode = true;
                });

                break;
            }
            case RuneOfFortification:
            {
                me->CastSpell(me, DestructiveResonanceFortificationAura, true);

                AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
                {
                    me->RemoveAura(DestructiveResonanceFortificationAura);

                    me->CastSpell(me, FortificationMineVisualAura, true);
                    me->CastSpell(me, FortificationVisualAura, true);

                    m_CanExplode = true;
                });

                /// The mines caused by Destructive Resonance: Fortification Icon Destructive Resonance: Fortification now last 2 minutes, up from 1 minute.
                AddDelayedEvent(120 * IN_MILLISECONDS, [this]() -> void
                {
                    Despawn();
                });

                return;
            }
            case RuneOfReplication:
            {
                me->CastSpell(me, DestructiveResonanceReplicationAura, true);

                AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
                {
                    me->RemoveAura(DestructiveResonanceReplicationAura);

                    me->CastSpell(me, ReplicationMineVisualAura, true);
                    me->CastSpell(me, ReplicationVisualAura, true);

                    m_CanExplode = true;
                });

                break;
            }
            default:
                break;
        }

        m_SpawnTime = 0;

        /// At the end of the 1 minute, the mine despawns harmlessly.
        AddDelayedEvent(60 * IN_MILLISECONDS, [this]() -> void
        {
            Despawn();
        });
    }

    void UpdateAI(uint32 diff) override
    {
        m_SpawnTime += diff;

        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!m_CanExplode)
            return;

        float radius = 3.0f;

        if (m_Phase == 2)
            AddPct(radius, float((float)m_SpawnTime / float(30 * IN_MILLISECONDS) * 100.0f));

        if (radius > 6.0f)
            radius = 6.0f;

        if (Player* player = me->FindNearestPlayer(radius))
        {
            me->CastSpell(player, DestructiveResonanceDebuff, true);
            me->CastSpell(player, DestructiveResonanceDamage, true);

            switch (m_Phase)
            {
                case MightOfTheCrown:
                    me->SendPlaySpellVisualKit(DestructiveResonanceDespawn, 0);
                    break;
                case RuneOfDisplacement:
                    me->SendPlaySpellVisualKit(DestructiveResonanceDisplacementDespawn, 0);
                    break;
                case RuneOfFortification:
                    me->SendPlaySpellVisualKit(DestructiveResonanceFortificationDesp, 0);
                    break;
                case RuneOfReplication:
                    me->SendPlaySpellVisualKit(DestructiveResonanceReplicationDespawn, 0);
                    break;
                default:
                    break;
            }

            Talk(DestructiveResonance, player->GetGUID());

            Despawn();

            m_CanExplode = false;
        }
    }

    void Despawn()
    {
        me->RemoveAllAuras();
        me->DespawnOrUnsummon();

        /// In addition to Destructive Resonance's normal effects, two additional Destructive Resonances are created nearby when it detonates or expires.
        if (m_Phase == RuneOfReplication)
        {
            if (InstanceScript* instanceScript = me->GetInstanceScript())
            {
                if (Creature* margok = Creature::GetCreature(*me, instanceScript->GetGuidData(ImperatorMargok)))
                {
                    float radius = 8.0f;
                    Position pos = *me;

                    for (uint8 i = 0; i < ReplicationDupliCount; ++i)
                    {
                        float o = frand(0, 2 * M_PI);
                        float x = pos.m_positionX + (radius * cos(o));
                        float y = pos.m_positionY + (radius * sin(o));

                        margok->CastSpell(x, y, pos.m_positionZ, DestructiveResonanceReplicating, true);
                    }
                }
            }
        }
    }
};

/// Destructive Resonance (Replication) - 77681
struct npc_highmaul_destructive_resonance_replication : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        DestructiveResonanceDebuff = 159200,
        DestructiveResonanceDamage = 156673,
        /// Destructive Resonance: Replication
        DestructiveResonanceReplicationAura = 164096,
        ReplicationMineVisualAura = 156957,
        ReplicationVisualAura = 156977,
        DestructiveResonanceReplicating = 156799
    };

    enum eVisual
    {
        DestructiveResonanceReplicationDespawn = 45693
    };

    enum eTalk
    {
        DestructiveResonance
    };

    npc_highmaul_destructive_resonance_replication(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_CanExplode = false;
    }

    bool m_CanExplode;

    void Reset() override
    {
        me->AddUnitState(UNIT_STATE_ROOT);

        me->SetReactState(REACT_PASSIVE);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

        me->CastSpell(me, DestructiveResonanceReplicationAura, true);

        AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
        {
            me->RemoveAura(DestructiveResonanceReplicationAura);

            me->CastSpell(me, ReplicationMineVisualAura, true);
            me->CastSpell(me, ReplicationVisualAura, true);

            m_CanExplode = true;
        });
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!m_CanExplode)
            return;

        float radius = 3.0f;

        if (Player* player = me->FindNearestPlayer(radius))
        {
            me->CastSpell(player, DestructiveResonanceDebuff, true);
            me->CastSpell(player, DestructiveResonanceDamage, true);

            me->SendPlaySpellVisualKit(eVisual::DestructiveResonanceReplicationDespawn, 0);

            Talk(DestructiveResonance, player->GetGUID());

            Despawn();

            m_CanExplode = false;
        }
    }

    void Despawn()
    {
        me->RemoveAllAuras();
        me->DespawnOrUnsummon();
    }
};

/// Gorian Warmage - 78121
struct npc_highmaul_gorian_warmage : public ScriptedAI
{
    enum eSpells
    {
        /// Passive ability that buffs all allies within 25 yards, increasing their damage done by 50% and their attack and casting speed by 50%
        DominanceAura = 174126,
        Fixate = 157763,
        Slow = 157801,
        NetherBlast = 157769,
        /// Misc
        FortificationAchievement = 143809,
        PowerOfFortification = 155040,
        ReplicationAchievement = 166391,
        PowerOfReplication = 166389
    };

    enum eEvents
    {
        EventFixate = 1,
        EventSlow,
        EventNetherBlast
    };

    enum eCosmeticEvent
    {
        EventCheckRune = 1
    };


    npc_highmaul_gorian_warmage(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;

    EventMap m_Events;
    EventMap m_CosmeticEvents;

    ObjectGuid m_FixateTarget;

    void Reset() override
    {
        me->AddUnitState(UNIT_STATE_ROOT);

        me->CastSpell(me, DominanceAura, true);

        if (Player* target = me->SelectNearestPlayerNotGM(50.0f))
            AttackStart(target);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);

        m_Events.Reset();

        m_CosmeticEvents.Reset();

        m_Events.RescheduleEvent(EventFixate, 3 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventSlow, 5 * IN_MILLISECONDS);

        m_CosmeticEvents.RescheduleEvent(EventCheckRune, 1 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        if (me->HasAura(PowerOfFortification) && me->HasAura(PowerOfReplication))
            m_Instance->SetData(ImperatorAchievement, 1);
    }

    void UpdateAI(uint32 diff) override
    {
        m_CosmeticEvents.Update(diff);

        if (m_CosmeticEvents.ExecuteEvent() == EventCheckRune)
        {
            std::list<Creature*> triggerList;
            me->GetCreatureListWithEntryInGrid(triggerList, SLGGenericMoPLargeAoI, 15.0f);

            for (Creature* trigger : triggerList)
            {
                if (trigger->HasAura(FortificationAchievement))
                    me->CastSpell(me, PowerOfFortification, true);
                else if (trigger->HasAura(ReplicationAchievement))
                    me->CastSpell(me, PowerOfReplication, true);
            }

            /// Don't need to do that again if the Warmage already has the two auras
            if (!me->HasAura(PowerOfFortification) || !me->HasAura(PowerOfReplication))
                m_CosmeticEvents.RescheduleEvent(EventCheckRune, 1 * IN_MILLISECONDS);
        }

        if (!UpdateVictim())
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventFixate:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true, -Fixate))
                {
                    m_FixateTarget = target->GetGUID();
                    me->CastSpell(target, Fixate, true);
                }

                m_Events.RescheduleEvent(EventNetherBlast, 100);
                m_Events.RescheduleEvent(EventFixate, 15 * IN_MILLISECONDS);
                break;
            case EventSlow:
                me->CastSpell(me, Slow, true);
                m_Events.RescheduleEvent(EventSlow, 17 * IN_MILLISECONDS);
                break;
            case EventNetherBlast:
                if (Unit* target = Player::GetPlayer(*me, m_FixateTarget))
                    me->CastSpell(target, NetherBlast, false);

                m_Events.RescheduleEvent(EventNetherBlast, 200);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Volatile Anomaly - 78077
struct npc_highmaul_volatile_anomaly : public MS::AI::CosmeticAI
{
    enum eSpells
    {
        Destabilize = 157657,
        AlphaFadeOut = 141608
    };

    enum eVisual
    {
        AnimKit = 5709
    };

    npc_highmaul_volatile_anomaly(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        if (Player* target = me->SelectNearestPlayerNotGM(100.0f))
            AttackStart(target);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->SetAnimKitId(eVisual::AnimKit);

        me->CastSpell(me, Destabilize, true);
        me->CastSpell(me, AlphaFadeOut, true);
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

/// Gorian Reaver - 78549
struct npc_highmaul_gorian_reaver : public ScriptedAI
{
    enum eSpells
    {
        DevastatingShockwave = 158547,
        CrushArmor = 158553,
        KickToTheFace = 158563
    };

    enum eEvents
    {
        EventCrushArmor = 1,
        EventKickToTheFace,
        EventDevastatingShockwave
    };

    npc_highmaul_gorian_reaver(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    InstanceScript* m_Instance;
    EventMap m_Events;

    void Reset() override
    {
        if (Player* target = me->SelectNearestPlayerNotGM(200.0f))
            AttackStart(target);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);

        m_Events.Reset();

        m_Events.RescheduleEvent(EventCrushArmor, 22 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventKickToTheFace, 41 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventDevastatingShockwave, 12 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
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
            case EventCrushArmor:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, CrushArmor, true);

                m_Events.RescheduleEvent(EventCrushArmor, 11 * IN_MILLISECONDS);
                break;
            case EventKickToTheFace:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                {
                    me->CastSpell(target, KickToTheFace, true);
                    me->getThreatManager().modifyThreatPercent(target, -100);
                }

                m_Events.RescheduleEvent(EventKickToTheFace, 25 * IN_MILLISECONDS);
                break;
            case EventDevastatingShockwave:
                me->CastSpell(me, DevastatingShockwave, true);
                m_Events.RescheduleEvent(EventDevastatingShockwave, 12 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Arcane Remnant - 79388
struct npc_highmaul_arcane_remnant : public MS::AI::CosmeticAI
{
    enum eSpell
    {
        ReverseDeath = 157099
    };

    npc_highmaul_arcane_remnant(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        me->CastSpell(me, ReverseDeath, true);

        AddDelayedEvent(3 * IN_MILLISECONDS, [this]() -> void
        {
            if (Player* target = me->SelectNearestPlayerNotGM(50.0f))
                AttackStart(target);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        MS::AI::CosmeticAI::UpdateAI(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

/// Mark of Chaos - 158605
/// Mark of Chaos: Displacement - 164176
/// Mark of Chaos: Fortification - 164178
/// Mark of Chaos: Replication - 164191
class spell_highmaul_mark_of_chaos : public AuraScript
{
    enum eSpells
    {
        MarkOfChaosAoE = 158609,
        FetterMarkOfChaosRootAura = 158619
    };

    PrepareAuraScript(spell_highmaul_mark_of_chaos);

    void AfterAuraRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT)
            return;

        if (Unit* target = GetTarget())
        {
            target->CastSpell(target, MarkOfChaosAoE, true);
            target->RemoveAura(FetterMarkOfChaosRootAura);
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_mark_of_chaos::AfterAuraRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Destructive Resonance - 174116
class spell_highmaul_destructive_resonance : public AuraScript
{
    PrepareAuraScript(spell_highmaul_destructive_resonance);

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        SpellInfo const* spellInfo = eventInfo.GetDamageInfo()->GetSpellInfo();
        if (spellInfo == nullptr)
            return;

        //if (!(spellInfo->SchoolMask & SPELL_SCHOOL_MASK_ARCANE))
        //    return;

        /// Moreover, the player who triggers a Destructive Resonance mine is stunned for 1.5 seconds each time he take Arcane damage.
        if (Unit* target = GetTarget())
            target->CastSpell(target, GetSpellInfo()->Effects[EFFECT_0]->TriggerSpell, true);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_highmaul_destructive_resonance::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

/// Branded - 156225
class spell_highmaul_branded : public AuraScript
{
    enum eSpells
    {
        ArcaneWrathDamage = 156239
    };

    enum eData
    {
        BrandedStacks
    };

    enum eTalk
    {
        Branded = 21
    };

    PrepareAuraScript(spell_highmaul_branded);

    void AfterAuraRemove(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT || GetCaster() == nullptr)
            return;

        /// Caster is Mar'gok
        if (Creature* margok = GetCaster()->ToCreature())
        {
            if (!margok->IsAIEnabled)
                return;

            if (Unit* target = GetTarget())
            {
                if (auto ai = CAST_AI(boss_imperator_margok, margok->GetAI()))
                {
                    ObjectGuid guid = target->GetGUID();
                    ObjectGuid meGuid = margok->GetGUID();

                    uint32 spellID = GetSpellInfo()->Id;
                    uint8 stacks = auraEffect->GetBase()->GetStackAmount();
                    ai->AddDelayedEvent(100, [spellID, stacks, guid, meGuid]() -> void
                    {
                        uint8 stacksCopy = stacks;

                        if (Creature* margok = sObjectAccessor->FindCreature(meGuid))
                        {
                            if (Unit* target = Unit::GetUnit(*margok, guid))
                            {
                                CustomSpellValues values;
                                values.AddSpellMod(SPELLVALUE_AURA_STACK, stacksCopy);

                                margok->CastCustomSpell(ArcaneWrathDamage, values, target, true);

                                /// When Branded expires it inflicts Arcane damage to the wearer and jumps to their closest ally within 200 yards.
                                /// Each time Arcane Wrath jumps, its damage increases by 25% and range decreases by 50%.
                                float jumpRange = 200.0f;
                                for (uint8 i = 0; i < stacks; ++i)
                                    jumpRange -= CalculatePct(jumpRange, 50.0f);

                                if (Player* otherPlayer = target->FindNearestPlayer(jumpRange))
                                {
                                    /// Increase jump count
                                    ++stacksCopy;

                                    if (Aura* aura = margok->AddAura(spellID, otherPlayer))
                                    {
                                        aura->SetStackAmount(stacksCopy);
                                        margok->AI()->Talk(Branded, otherPlayer->GetGUID());
                                    }
                                }
                            }
                        }
                    });
                }
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_branded::AfterAuraRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Branded: Displacement - 164004
class spell_highmaul_branded_displacement : public AuraScript
{
    enum eSpells
    {
        ArcaneWrathDamage = 156239,
        ArcaneWrathZoneVisu = 160369,
        ArcaneWrathTeleport = 160370
    };

    enum eData
    {
        BrandedStacks
    };

    enum eTalk
    {
        Branded = 21
    };

    PrepareAuraScript(spell_highmaul_branded_displacement);

    Position m_MarkPos;

    void OnAuraApply(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
        {
            /// Visual area during 4s to see where you can go without being teleported
            target->CastSpell(target, ArcaneWrathZoneVisu, true);
            m_MarkPos = *target;
        }
    }

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
        {
            /// In addition to Arcane Wrath's normal effects, Branded Players are unable to move more than 10 yards from the location they were marked.
            /// Attempting to leave the marked area will teleport the player back to the location they were originally marked.
            if (target->GetDistance(m_MarkPos) > 10.0f)
                target->CastSpell(m_MarkPos, ArcaneWrathTeleport, true);
        }
    }

    void AfterAuraRemove(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT || GetCaster() == nullptr)
            return;

        /// Caster is Mar'gok
        if (Creature* margok = GetCaster()->ToCreature())
        {
            if (!margok->IsAIEnabled)
                return;

            if (Unit* target = GetTarget())
            {
                if (boss_imperator_margok* ai = CAST_AI(boss_imperator_margok, margok->GetAI()))
                {
                    ObjectGuid guid = target->GetGUID();
                    ObjectGuid meGuid = margok->GetGUID();

                    uint32 spellID = GetSpellInfo()->Id;
                    uint8 stacks = auraEffect->GetBase()->GetStackAmount();
                    ai->AddDelayedEvent(100, [spellID, stacks, guid, meGuid]() -> void
                    {
                        uint8 stackCopy = stacks;

                        if (Creature* margok = sObjectAccessor->FindCreature(meGuid))
                        {
                            if (Unit* target = Unit::GetUnit(*margok, guid))
                            {
                                CustomSpellValues values;
                                values.AddSpellMod(SPELLVALUE_AURA_STACK, stackCopy);

                                margok->CastCustomSpell(ArcaneWrathDamage, values, target, true);

                                /// When Branded expires it inflicts Arcane damage to the wearer and jumps to their closest ally within 200 yards.
                                /// Each time Arcane Wrath jumps, its damage increases by 25% and range decreases by 50%.
                                float jumpRange = 200.0f;
                                for (uint8 i = 0; i < stackCopy; ++i)
                                    jumpRange -= CalculatePct(jumpRange, 50.0f);

                                if (Player* otherPlayer = target->FindNearestPlayer(jumpRange))
                                {
                                    /// Increase jump count
                                    ++stackCopy;

                                    if (Aura* aura = margok->AddAura(spellID, otherPlayer))
                                    {
                                        aura->SetStackAmount(stackCopy);
                                        margok->AI()->Talk(Branded, otherPlayer->GetGUID());
                                    }
                                }
                            }
                        }
                    });
                }
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_highmaul_branded_displacement::OnAuraApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_branded_displacement::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_branded_displacement::AfterAuraRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Branded: Fortification - 164005
class spell_highmaul_branded_fortification : public AuraScript
{
    enum eSpells
    {
        ArcaneWrathDamage = 156239
    };

    enum eData
    {
        BrandedStacks
    };

    enum eTalk
    {
        Branded = 21
    };

    PrepareAuraScript(spell_highmaul_branded_fortification);

    void AfterAuraRemove(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT || GetCaster() == nullptr)
            return;

        /// Caster is Mar'gok
        if (Creature* margok = GetCaster()->ToCreature())
        {
            if (!margok->IsAIEnabled)
                return;

            if (Unit* target = GetTarget())
            {
                if (boss_imperator_margok* ai = CAST_AI(boss_imperator_margok, margok->GetAI()))
                {
                    ObjectGuid guid = target->GetGUID();
                    ObjectGuid meGuid = margok->GetGUID();

                    uint32 spellID = GetSpellInfo()->Id;
                    uint8 stacks = auraEffect->GetBase()->GetStackAmount();
                    ai->AddDelayedEvent(100, [spellID, stacks, guid, meGuid]() -> void
                    {
                        uint8 stacksCopy = stacks;

                        if (Creature* margok = sObjectAccessor->FindCreature(meGuid))
                        {
                            if (Unit* target = Unit::GetUnit(*margok, guid))
                            {
                                CustomSpellValues values;
                                values.AddSpellMod(SPELLVALUE_AURA_STACK, stacksCopy);

                                margok->CastCustomSpell(ArcaneWrathDamage, values, target, true);

                                /// When Branded expires it inflicts Arcane damage to the wearer and jumps to their closest ally within 200 yards.
                                /// Each time Arcane Wrath jumps, its damage increases by 25% and range decreases by 25%.
                                float jumpRange = 200.0f;
                                for (uint8 i = 0; i < stacksCopy; ++i)
                                    jumpRange -= CalculatePct(jumpRange, 25.0f);

                                if (Player* otherPlayer = target->FindNearestPlayer(jumpRange))
                                {
                                    /// Increase jump count
                                    ++stacksCopy;

                                    if (Aura* aura = margok->AddAura(spellID, otherPlayer))
                                    {
                                        aura->SetStackAmount(stacksCopy);
                                        margok->AI()->Talk(Branded, otherPlayer->GetGUID());
                                    }
                                }
                            }
                        }
                    });
                }
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_branded_fortification::AfterAuraRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Branded: Replication - 164006
class spell_highmaul_branded_replication : public AuraScript
{
    enum eSpells
    {
        ArcaneWrathDamage = 156239
    };

    enum eData
    {
        BrandedStacks
    };

    enum eTalk
    {
        Branded = 21
    };

    PrepareAuraScript(spell_highmaul_branded_replication);

    void AfterAuraRemove(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT || GetCaster() == nullptr)
            return;

        /// Caster is Mar'gok
        if (Creature* margok = GetCaster()->ToCreature())
        {
            if (!margok->IsAIEnabled)
                return;

            if (Unit* target = GetTarget())
            {
                if (boss_imperator_margok* ai = CAST_AI(boss_imperator_margok, margok->GetAI()))
                {
                    ObjectGuid guid = target->GetGUID();
                    ObjectGuid meGuid = margok->GetGUID();

                    uint32 spellID = GetSpellInfo()->Id;
                    uint8 stacks = auraEffect->GetBase()->GetStackAmount();
                    ai->AddDelayedEvent(100, [spellID, stacks, guid, meGuid]() -> void
                    {
                        uint8 stacksCopy = stacks;

                        if (Creature* margok = sObjectAccessor->FindCreature(meGuid))
                        {
                            if (Unit* target = Unit::GetUnit(*margok, guid))
                            {
                                CustomSpellValues values;
                                values.AddSpellMod(SPELLVALUE_AURA_STACK, stacksCopy);

                                margok->CastCustomSpell(ArcaneWrathDamage, values, target, true);

                                /// When Branded expires it inflicts Arcane damage to the wearer and jumps to their closest ally within 200 yards.
                                /// Each time Arcane Wrath jumps, its damage increases by 25% and range decreases by 25%.
                                float jumpRange = 200.0f;
                                for (uint8 i = 0; i < stacksCopy; ++i)
                                    jumpRange -= CalculatePct(jumpRange, 25.0f);

                                /// In addition to Arcane Wrath's normal effects, a second player will be Branded the first time Arcane Wrath jumps.
                                if (stacksCopy <= 1)
                                {
                                    std::list<Player*> playerList;
                                    target->GetPlayerListInGrid(playerList, jumpRange);

                                    if (playerList.size() > 2)
                                    {
                                        playerList.sort(Trinity::ObjectDistanceOrderPred(target));
                                        Trinity::Containers::RandomResizeList(playerList, 2);
                                    }

                                    /// Increase jump count
                                    ++stacksCopy;

                                    for (Player* player : playerList)
                                    {
                                        if (Aura* aura = margok->AddAura(spellID, player))
                                        {
                                            aura->SetStackAmount(stacksCopy);
                                            margok->AI()->Talk(Branded, player->GetGUID());
                                        }
                                    }

                                    return;
                                }

                                std::list<Player*> playerList;
                                target->GetPlayerListInGrid(playerList, jumpRange);

                                /// It cannot jumps twice on the same player at the same time
                                if (!playerList.empty())
                                    playerList.remove_if(Trinity::UnitAuraCheck(true, spellID));

                                if (!playerList.empty())
                                {
                                    playerList.sort(Trinity::ObjectDistanceOrderPred(target));

                                    if (Player* otherPlayer = playerList.front())
                                    {
                                        /// Increase jump count
                                        ++stacksCopy;

                                        if (Aura* aura = margok->AddAura(spellID, otherPlayer))
                                        {
                                            aura->SetStackAmount(stacksCopy);
                                            margok->AI()->Talk(Branded, otherPlayer->GetGUID());
                                        }
                                    }
                                }
                            }
                        }
                    });
                }
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_branded_replication::AfterAuraRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Arcane Wrath (damage) - 156239
class spell_highmaul_arcane_wrath_damage : public SpellScript
{
    PrepareSpellScript(spell_highmaul_arcane_wrath_damage);

    void HandleDamage()
    {
        /// When Branded expires it inflicts Arcane damage to the wearer and jumps to their closest ally within 200 yards.
        /// Each time Arcane Wrath jumps, its damage increases by 25% and range decreases by 50%.
        int32 damage = GetHitDamage();
        //uint8 stacks = GetSpell()->GetSpellValue(SPELLVALUE_AURA_STACK);
        //AddPct(damage, int32(25 * stacks));

        SetHitDamage(damage);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_highmaul_arcane_wrath_damage::HandleDamage);
    }
};

/// Transition Visuals - 176580
class spell_highmaul_transition_visuals : public AuraScript
{
    enum eSpells
    {
        TransitionVisualMissileP1 = 176581,
        TransitionVisualMissileP2 = 176582,
        TransitionVisualMissileP3 = 176583
    };

    enum eDatas
    {
        PhaseID = 1,
        MissileCountP1 = 6,
        MissileCountP2 = 3,
        MissileCountP3 = 2
    };

    enum ePhases
    {
        MightOfTheCrown = 1,    ///< Phase 1: Might of the Crown
        RuneOfDisplacement,     ///< Phase 2: Rune of Displacement
        DormantRunestones,      ///< Intermission: Dormant Runestones
        RuneOfFortification,    ///< Phase 3: Rune of Fortification
        LineageOfPower,         ///< Intermission: Lineage of Power
        RuneOfReplication,      ///< Phase 4: Rune of Replication
        MaxPhases
    };

    PrepareAuraScript(spell_highmaul_transition_visuals);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
        {
            if (Creature* margok = target->FindNearestCreature(ImperatorMargok, 40.0f))
            {
                if (!margok->IsAIEnabled)
                    return;

                switch (margok->AI()->GetData(PhaseID))
                {
                    case RuneOfDisplacement:
                        for (uint8 i = 0; i < MissileCountP1; ++i)
                            target->CastSpell(margok, TransitionVisualMissileP1, true);
                        break;
                    case DormantRunestones:
                        for (uint8 i = 0; i < MissileCountP2; ++i)
                            target->CastSpell(margok, TransitionVisualMissileP2, true);
                        break;
                    case LineageOfPower:
                        for (uint8 i = 0; i < MissileCountP3; ++i)
                            target->CastSpell(margok, TransitionVisualMissileP3, true);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_transition_visuals::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

/// Dominance Aura - 174126
class spell_highmaul_dominance_aura : public AuraScript
{
    PrepareAuraScript(spell_highmaul_dominance_aura);

    enum eSpell
    {
        DominanceAuraBuff = 174128
    };

    enum eData
    {
        InvisDisplay = 11686
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
                    std::list<Unit*> targetList;
                    float radius = 40.0f;

                    Trinity::AnyFriendlyUnitInObjectRangeCheck check(caster, caster, radius);
                    Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(caster, targetList, check);
                    caster->VisitNearbyObject(radius, searcher);

                    targetList.remove(caster);

                    for (Unit* itr : targetList)
                    {
                        if (itr->GetDisplayId() == InvisDisplay)
                            continue;

                        if (itr->GetDistance(caster) <= 25.0f)
                        {
                            if (!itr->HasAura(DominanceAuraBuff))
                                caster->CastSpell(itr, DominanceAuraBuff, true);
                        }
                        else
                        {
                            if (itr->HasAura(DominanceAuraBuff))
                                itr->RemoveAura(DominanceAuraBuff);
                        }
                    }
                }

                m_CheckTimer = 500;
            }
            else
                m_CheckTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_dominance_aura::OnUpdate);
    }
};

/// Force Nova: Fortified - 157323
class spell_highmaul_force_nova_fortified : public AuraScript
{
    enum eSpell
    {
        ForceNovaFortificationDummy = 164253
    };

    PrepareAuraScript(spell_highmaul_force_nova_fortified);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, ForceNovaFortificationDummy, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_force_nova_fortified::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Devastating Shockwave - 158547
class spell_highmaul_devastating_shockwave : public SpellScript
{
    PrepareSpellScript(spell_highmaul_devastating_shockwave);

    enum eSpell
    {
        TargetRestrict = 19856
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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_devastating_shockwave::CorrectTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
    }
};

/// Force Nova (DoT) - 157353
class spell_highmaul_force_nova_dot : public AuraScript
{
    enum eSpell
    {
        ForceNovaReplicationAoEDamage = 157357  ///< Damaging spell for allies
    };

    enum eData
    {
        PhaseID = 1
    };

    enum ePhase
    {
        RuneOfReplication = 6
    };

    PrepareAuraScript(spell_highmaul_force_nova_dot);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
        {
            if (Creature* margok = target->FindNearestCreature(ImperatorMargok, 40.0f))
            {
                if (!margok->IsAIEnabled)
                    return;

                uint8 phase = margok->AI()->GetData(PhaseID);
                if (phase != RuneOfReplication)
                    return;

                target->CastSpell(target, ForceNovaReplicationAoEDamage, true);
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_force_nova_dot::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};

/// Orbs of Chaos (Dummy aura) - 160447
class spell_highmaul_orbs_of_chaos_aura : public AuraScript
{
    enum eSpells
    {
        OrbsOfChaosSummoning = 158639
    };

    enum eTalk
    {
        OrbsOfChaos = 22
    };

    enum eData
    {
        OrbOfChaosAngle = 2
    };

    PrepareAuraScript(spell_highmaul_orbs_of_chaos_aura);

    void AfterAuraRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode == AURA_REMOVE_BY_DEFAULT)
            return;

        if (Unit* target = GetTarget())
        {
            target->CastSpell(target, OrbsOfChaosSummoning, true);

            if (Creature* margok = target->FindNearestCreature(ImperatorMargok, 300.0f))
            {
                if (!margok->IsAIEnabled)
                    return;

                margok->AI()->SetData(OrbOfChaosAngle, 0);
                margok->AI()->Talk(OrbsOfChaos, target->GetGUID());

                if (boss_imperator_margok* ai = CAST_AI(boss_imperator_margok, margok->GetAI()))
                {
                    ObjectGuid guid = target->GetGUID();
                    ai->AddDelayedEvent(2 * IN_MILLISECONDS, [ai, guid]() -> void
                    {
                        if (Unit* target = Unit::GetUnit(*ai->me, guid))
                            target->CastSpell(target, OrbsOfChaosSummoning, true);
                    });
                }
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_orbs_of_chaos_aura::AfterAuraRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Volatile Anomalies - 157265
class spell_highmaul_volatile_anomalies : public AuraScript
{
    enum eData
    {
        PhaseID = 1
    };

    enum eSpells
    {
        VolatileAnomalies1 = 158512,
        VolatileAnomalies2 = 159158,
        VolatileAnomalies3 = 159159
    };

    enum ePhases
    {
        DormantRunestones = 3,    ///< Intermission: Dormant Runestones
        LineageOfPower = 5     ///< Intermission: Lineage of Power
    };

    PrepareAuraScript(spell_highmaul_volatile_anomalies);

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
        {
            if (Creature* margok = target->FindNearestCreature(ImperatorMargok, 150.0f))
            {
                if (!margok->IsAIEnabled)
                    return;

                switch (margok->AI()->GetData(PhaseID))
                {
                    case DormantRunestones:
                        target->CastSpell(volatileAnomalyPos[0], VolatileAnomalies1, true);
                        target->CastSpell(volatileAnomalyPos[0], VolatileAnomalies2, true);
                        target->CastSpell(volatileAnomalyPos[0], VolatileAnomalies3, true);
                        break;
                    case LineageOfPower:
                        target->CastSpell(volatileAnomalyPos[1], VolatileAnomalies1, true);
                        target->CastSpell(volatileAnomalyPos[1], VolatileAnomalies2, true);
                        target->CastSpell(volatileAnomalyPos[1], VolatileAnomalies3, true);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_volatile_anomalies::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

void AddSC_boss_imperator_margok()
{
    RegisterHighmaulCreatureAI(boss_imperator_margok);
    RegisterHighmaulCreatureAI(npc_highmaul_rune_of_displacement);
    RegisterHighmaulCreatureAI(npc_highmaul_arcane_aberration);
    RegisterHighmaulCreatureAI(npc_highmaul_destructive_resonance);
    //RegisterHighmaulCreatureAI(npc_highmaul_destructive_resonance_replication);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_warmage);
    RegisterHighmaulCreatureAI(npc_highmaul_volatile_anomaly);
    RegisterHighmaulCreatureAI(npc_highmaul_gorian_reaver);
    RegisterHighmaulCreatureAI(npc_highmaul_arcane_remnant);

    RegisterAuraScript(spell_highmaul_mark_of_chaos);
    RegisterAuraScript(spell_highmaul_destructive_resonance);
    RegisterAuraScript(spell_highmaul_branded);
    RegisterAuraScript(spell_highmaul_branded_displacement);
    RegisterAuraScript(spell_highmaul_branded_fortification);
    RegisterAuraScript(spell_highmaul_branded_replication);
    RegisterSpellScript(spell_highmaul_arcane_wrath_damage);
    RegisterAuraScript(spell_highmaul_transition_visuals);
    //RegisterAuraScript(spell_highmaul_dominance_aura);
    RegisterAuraScript(spell_highmaul_force_nova_fortified);
    RegisterSpellScript(spell_highmaul_devastating_shockwave);
    RegisterAuraScript(spell_highmaul_force_nova_dot);
    RegisterAuraScript(spell_highmaul_orbs_of_chaos_aura);
    RegisterAuraScript(spell_highmaul_volatile_anomalies);
}
