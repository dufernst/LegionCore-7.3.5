////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"

Position const centerPos = {3903.39f, 8608.15f, 364.71f, 5.589f};

/// Ko'ragh <Breaker of Magic> - 79015
struct boss_koragh : public BossAI
{
    enum eSpells
    {
        /// Cosmetic
        RuneChargingPermanent = 174415,
        RuneChargingTemporary = 160721,
        NullificationRuneEmpowered = 166482,
        CausticEnergyAreaTrigger = 160720,
        /// Nullification Barrier
        NullificationBarrierPower = 163612,
        NullificationBarrierAbsorb = 156803,
        BreakersStrength = 162569,
        NullificationBarrierAbsorb2 = 163134,   ///< For Players
        MarkOfNullification = 172886,
        VulnerabilityAura = 160734,
        KnockbackForRecharge = 174856,
        VolatileAnomaliesAura = 161378,
        /// Expel Magic: Fire
        ExpelMagicFireDoT = 162185,
        /// Expel Magic: Arcane
        ExpelMagicArcaneAura = 162186,
        /// Expel Magic: Shadow
        ExpelMagicShadow = 162184,
        /// Expel Magic: Frost
        ExpelMagicFrostAreaTrigger = 172747,
        ExpelMagicFrostDamage = 161411,
        ExpelMagicFrostAura = 172813,
        /// Suppression Field
        SuppressionFieldAura = 161328,
        SuppressionFieldMissile = 161331,   ///< Triggers 161330 (AreaTrigger) and 161358 (Damage)
        SuppressionFieldDoT = 161345,
        SuppressionFieldSilence = 162595,
        /// Overflowing Energy
        OverflowingEnergySpawn = 161574,
        /// Loot
        KoraghBonus = 177526
    };

    enum eEvents
    {
        EventExpelMagicFire = 1,
        EventExpelMagicArcane,
        EventExpelMagicFrost,
        EventExpelMagicShadow,
        EventSuppressionField,
        EventOverflowingEnergy
    };

    enum eCosmeticEvents
    {
        EventBreakersStrength = 1,
        EventEndOfCharging
    };

    enum eActions
    {
        CancelBreakersStrength,
        ActionSuppressionField
    };

    enum eCreatures
    {
        BreakerOfFel = 86330,
        BreakerOfFire = 86329,
        BreakerOfFrost = 86326
    };

    enum eTalks
    {
        Intro,
        Aggro,
        BarrierShattered,
        ExpelMagic,
        SuppressionField,
        DominatingPower,
        Berserk,
        Slay,
        Death,
        CausticEnergyWarn,
        ExpelMagicFelWarn,
        ExpelMagicArcaneWarn
    };

    enum eAnimKit
    {
        AnimWaiting = 7289
    };

    enum eMove
    {
        MoveToCenter
    };

    enum eDatas
    {
        DataMaxRunicPlayers,
        DataRunicPlayersCount
    };


    boss_koragh(Creature* creature) : BossAI(creature, BossKoragh)
    {
        m_Instance = creature->GetInstanceScript();
        m_Init = false;
    }

    EventMap m_Events;
    EventMap m_CosmeticEvents;
    InstanceScript* m_Instance;
    std::set<ObjectGuid> m_RitualistGuids;
    bool m_Init;
    ObjectGuid m_FlyingRune;
    ObjectGuid m_FloorRune;
    bool m_Charging;
    uint8 m_RunicPlayers;
    uint8 m_RunicPlayersCount;
    ObjectGuid m_SuppressionFieldTarget;

    void Reset() override
    {
        m_Events.Reset();
        m_CosmeticEvents.Reset();

        me->RemoveAura(BreakersStrength);

        _Reset();

        if (m_RitualistGuids.empty() && m_Init)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            me->SetAnimKitId(0);
        }
        else
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            me->SetAnimKitId(AnimWaiting);
        }

        summons.DespawnAll();

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
        {
            if (m_RitualistGuids.empty() && !m_Init)
            {
                std::list<Creature*> breakerList;
                me->GetCreatureListWithEntryInGrid(breakerList, BreakerOfFel, 30.0f);

                for (Creature* breaker : breakerList)
                    if (breaker->isAlive())
                        m_RitualistGuids.insert(breaker->GetGUID());

                breakerList.clear();
                me->GetCreatureListWithEntryInGrid(breakerList, BreakerOfFire, 30.0f);

                for (Creature* breaker : breakerList)
                    if (breaker->isAlive())
                        m_RitualistGuids.insert(breaker->GetGUID());

                breakerList.clear();
                me->GetCreatureListWithEntryInGrid(breakerList, BreakerOfFrost, 30.0f);

                for (Creature* breaker : breakerList)
                    if (breaker->isAlive())
                        m_RitualistGuids.insert(breaker->GetGUID());

                m_Init = true;
            }

            std::list<Creature*> runeList;
            me->GetCreatureListWithEntryInGrid(runeList, RuneOfNullification, 50.0f);

            for (Creature* rune : runeList)
            {
                if (rune->GetPositionZ() > me->GetPositionZ() + 10.0f)
                    m_FlyingRune = rune->GetGUID();
                else
                    m_FloorRune = rune->GetGUID();
            }

            if (m_RitualistGuids.empty() && m_Init)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                me->SetAnimKitId(0);
            }
            else
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                me->SetAnimKitId(AnimWaiting);
            }
        });

        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
        {
            if (me->GetReactState() == REACT_AGGRESSIVE)
                return;

            if (Creature* flying = Creature::GetCreature(*me, m_FlyingRune))
            {
                flying->CastSpell(me, RuneChargingPermanent, true);

                flying->SetReactState(REACT_PASSIVE);
                flying->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
        });

        /// One player may enter the rune with Ko'ragh, absorbing a portion of its energy.
        /// In Mythic difficulty, two players may enter the rune per phase of Charging.
        /// In Raid Finder difficulty, five players may enter the rune per phase of Charging.
        m_RunicPlayers = IsMythicRaid() ? 2 : (m_Instance->instance->IsLfr() ? 5 : 1);
        m_RunicPlayersCount = 0;

        m_SuppressionFieldTarget.Clear();

        m_Charging = false;
    }

    void JustReachedHome() override
    {
        if (m_RitualistGuids.empty() && m_Init)
        {
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            me->SetAnimKitId(0);
        }
        else
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            me->SetAnimKitId(AnimWaiting);
        }

        m_Events.Reset();

        summons.DespawnAll();
    }

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        m_RitualistGuids.erase(guid);

        if (m_RitualistGuids.empty())
        {
            if (Creature* flying = Creature::GetCreature(*me, m_FlyingRune))
                flying->RemoveAura(RuneChargingPermanent);

            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            me->SetAnimKitId(0);

            Talk(Intro);
        }
    }

    bool CanRespawn() override
    {
        return false;
    }

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case CancelBreakersStrength:
                m_CosmeticEvents.CancelEvent(EventBreakersStrength);

                me->InterruptNonMeleeSpells(true);

                me->RemoveAura(BreakersStrength);
                me->RemoveAura(SuppressionFieldAura);

                /// When the Ko'ragh's Nullification Barrier is removed, he begins to recharge. After 20 sec, the barrier is restored.
                Talk(BarrierShattered);
                m_Charging = true;

                me->SetReactState(REACT_PASSIVE);
                me->ClearAllUnitState();

                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(MoveToCenter, centerPos);

                m_CosmeticEvents.RescheduleEvent(EventEndOfCharging, 15 * IN_MILLISECONDS);
                break;
            case ActionSuppressionField:
                if (Unit* target = Unit::GetUnit(*me, m_SuppressionFieldTarget))
                {
                    if (me->GetDistance(target) > 5.0f)
                    {
                        float o = me->GetAngle(target);
                        float radius = me->GetDistance(target) - 4.0f;
                        float x = me->GetPositionX() + (radius * cos(o));
                        float y = me->GetPositionY() + (radius * sin(o));

                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveCharge(x, y, target->GetPositionZ(), 10.0f, SuppressionFieldAura);
                        m_SuppressionFieldTarget.Clear();
                    }
                    else
                    {
                        if (Unit* target = Unit::GetUnit(*me, m_SuppressionFieldTarget))
                            me->SetFacingTo(me->GetAngle(target));

                        me->RemoveAura(SuppressionFieldAura);
                        me->CastSpell(me, SuppressionFieldMissile, true);

                        AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                            {
                                me->GetMotionMaster()->Clear();
                                me->GetMotionMaster()->MoveChase(target);
                            }
                        });

                        m_SuppressionFieldTarget.Clear();
                    }
                }
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        switch (id)
        {
            case MoveToCenter:
            {
                if (type != POINT_MOTION_TYPE)
                    break;

                if (Creature* flying = Creature::GetCreature(*me, m_FlyingRune))
                    flying->CastSpell(flying, RuneChargingTemporary, true);

                if (Creature* grounding = Creature::GetCreature(*me, m_FloorRune))
                {
                    grounding->CastSpell(grounding, CausticEnergyAreaTrigger, true, nullptr, nullptr, me->GetGUID());
                    grounding->CastSpell(grounding, VolatileAnomaliesAura, true, nullptr, nullptr, me->GetGUID());
                }

                me->SetAnimKitId(AnimWaiting);
                me->AddUnitState(UNIT_STATE_STUNNED);

                me->CastSpell(me, KnockbackForRecharge, true);
                me->CastSpell(me, VulnerabilityAura, true);

                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

                m_Events.DelayEvent(EventOverflowingEnergy, 20 * IN_MILLISECONDS);

                m_CosmeticEvents.CancelEvent(EventEndOfCharging);
                m_CosmeticEvents.RescheduleEvent(EventEndOfCharging, 20 * IN_MILLISECONDS);
                break;
            }
            case SuppressionFieldAura:
            {
                if (Unit* target = Unit::GetUnit(*me, m_SuppressionFieldTarget))
                    me->SetFacingTo(me->GetAngle(target));

                me->RemoveAura(SuppressionFieldAura);
                me->CastSpell(me, SuppressionFieldMissile, true);

                AddDelayedEvent(2 * IN_MILLISECONDS, [this]() -> void
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    {
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveChase(target);
                    }
                });

                break;
            }
            default:
                break;
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        _EnterCombat();

        Talk(Aggro);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        if (Creature* flying = Creature::GetCreature(*me, m_FlyingRune))
            flying->CastSpell(me, NullificationRuneEmpowered, true);

        me->CastSpell(me, NullificationBarrierPower, true);
        me->CastSpell(me, NullificationBarrierAbsorb, true);

        me->SetPower(POWER_ALTERNATE, 100);

        /// While Ko'ragh maintains a Nullification Barrier, his damage done is increased by 6% every 10 seconds.
        /// This effect stacks. When the barrier expires, Breaker's Strength is removed.
        m_CosmeticEvents.RescheduleEvent(EventBreakersStrength, 10 * IN_MILLISECONDS);

        m_Events.RescheduleEvent(EventExpelMagicFire, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventExpelMagicArcane, 30 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventExpelMagicFrost, 40 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventExpelMagicShadow, 55 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventSuppressionField, 15 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventOverflowingEnergy, 36 * IN_MILLISECONDS);
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

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(NullificationBarrierPower);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(NullificationBarrierAbsorb2);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfNullification);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicFireDoT);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicArcaneAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicShadow);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SuppressionFieldDoT);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SuppressionFieldSilence);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicFrostAura);

        CastSpellToPlayers(me->GetMap(), me, KoraghBonus, true);

        Talk(Death);
    }

    void EnterEvadeMode() override
    {
        CreatureAI::EnterEvadeMode();

        m_Instance->SetBossState(BossKoragh, FAIL);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(NullificationBarrierPower);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(NullificationBarrierAbsorb2);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(MarkOfNullification);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicFireDoT);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicArcaneAura);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicShadow);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SuppressionFieldDoT);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SuppressionFieldSilence);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(ExpelMagicFrostAura);

        summons.DespawnAll();
    }

    uint32 GetData(uint32 id) const override
    {
        switch (id)
        {
            case DataMaxRunicPlayers:
                return m_RunicPlayers;
            case DataRunicPlayersCount:
                return m_RunicPlayersCount;
            default:
                break;
        }

        return 0;
    }

    void SetData(uint32 id, uint32 value) override
    {
        switch (id)
        {
            case DataRunicPlayersCount:
                m_RunicPlayersCount = value;
                m_Instance->SetData(KoraghNullificationBarrier, 1);
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
            case ExpelMagicArcaneAura:
                Talk(ExpelMagicArcaneWarn, target->GetGUID());
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->GetDistance(me->GetHomePosition()) >= 60.0f)
        {
            EnterEvadeMode();
            return;
        }

        m_CosmeticEvents.Update(diff);

        switch (m_CosmeticEvents.ExecuteEvent())
        {
            case EventBreakersStrength:
                me->CastSpell(me, BreakersStrength, true);
                m_CosmeticEvents.RescheduleEvent(EventBreakersStrength, 10 * IN_MILLISECONDS);
                break;
            case EventEndOfCharging:
                me->SetAnimKitId(0);
                me->ClearUnitState(UNIT_STATE_STUNNED);

                m_Charging = false;

                me->SetPower(POWER_ALTERNATE, 100);
                me->CastSpell(me, NullificationBarrierAbsorb, true);

                me->RemoveAura(VulnerabilityAura);
                me->SetReactState(REACT_AGGRESSIVE);

                me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

                me->GetMotionMaster()->Clear();

                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->GetMotionMaster()->MoveChase(target);

                if (Creature* grounding = Creature::GetCreature(*me, m_FloorRune))
                    grounding->RemoveAura(VolatileAnomaliesAura);

                m_RunicPlayersCount = 0;
                m_CosmeticEvents.RescheduleEvent(EventBreakersStrength, 10 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        if (!UpdateVictim() || m_Charging)
            return;

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventExpelMagicFire:
                me->CastSpell(me, ExpelMagicFireDoT, false);
                Talk(ExpelMagic);
                m_Events.RescheduleEvent(EventExpelMagicFire, 60 * IN_MILLISECONDS);
                break;
            case EventExpelMagicArcane:
                if (Unit* target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                    me->CastSpell(target, ExpelMagicArcaneAura, false);
                Talk(ExpelMagic);
                m_Events.RescheduleEvent(EventExpelMagicArcane, 30 * IN_MILLISECONDS);
                break;
            case EventExpelMagicFrost:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f))
                {
                    me->CastSpell(target, ExpelMagicFrostAreaTrigger, false);

                    AddDelayedEvent(1500, [this]() -> void
                    {
                        me->CastSpell(me, ExpelMagicFrostDamage, true);
                    });
                }

                Talk(ExpelMagic);
                m_Events.RescheduleEvent(EventExpelMagicFrost, 60 * IN_MILLISECONDS);
                break;
            case EventExpelMagicShadow:
                me->CastSpell(me, ExpelMagicShadow, false);
                Talk(ExpelMagic);
                m_Events.RescheduleEvent(EventExpelMagicShadow, 60 * IN_MILLISECONDS);
                break;
            case EventSuppressionField:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f))
                {
                    m_SuppressionFieldTarget = target->GetGUID();
                    me->CastSpell(me, SuppressionFieldAura, false);
                }
                else if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                {
                    m_SuppressionFieldTarget = target->GetGUID();
                    me->CastSpell(me, SuppressionFieldAura, false);
                }

                Talk(SuppressionField);
                m_Events.RescheduleEvent(EventSuppressionField, 17 * IN_MILLISECONDS);
                break;
            }
            case EventOverflowingEnergy:
            {
                me->CastSpell(me, OverflowingEnergySpawn, true);
                m_Events.RescheduleEvent(EventOverflowingEnergy, 30 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Breaker Ritualist <Breaker of Fel> - 86330
struct npc_highmaul_breaker_of_fel : public ScriptedAI
{
    enum eSpells
    {
        /// Cosmetic
        FelBreakerFelChannel = 172664,

        /// Damaging Spells
        FelBlast = 174422,
        FelNova = 174403
    };

    enum eEvent
    {
        EventFelNova = 1
    };

    npc_highmaul_breaker_of_fel(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    InstanceScript* m_Instance;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, FelBreakerFelChannel, false);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventFelNova, 12 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* boss = m_Instance->GetCreature((Koragh)))
            if (boss->IsAIEnabled)
                boss->AI()->SetGUID(me->GetGUID());
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
            case EventFelNova:
                me->CastSpell(me, FelNova, false);
                m_Events.RescheduleEvent(EventFelNova, 25 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoSpellAttackIfReady(FelBlast);
    }
};


/// Breaker Ritualist <Breaker of Fire> - 86329
struct npc_highmaul_breaker_of_fire : public ScriptedAI
{
    enum eSpells
    {
        /// Cosmetic
        FelBreakerFireChannel = 172455,

        /// Damaging Spells
        WildFlamesSearcher = 173763,
        WildFlamesMissile = 173618
    };

    enum eEvent
    {
        EventWildFlames = 1
    };

    npc_highmaul_breaker_of_fire(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    InstanceScript* m_Instance;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, FelBreakerFireChannel, false);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventWildFlames, 4 * IN_MILLISECONDS);
    }

    void EnterEvadeMode() override
    {
        CreatureAI::EnterEvadeMode();
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case WildFlamesSearcher:
                me->CastSpell(target, WildFlamesMissile, true);
                break;
            default:
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* boss = m_Instance->GetCreature((Koragh)))
            if (boss->IsAIEnabled)
                boss->AI()->SetGUID(me->GetGUID());
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
            case EventWildFlames:
                me->CastSpell(me, WildFlamesSearcher, false);
                m_Events.RescheduleEvent(EventWildFlames, 11 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Wild Flames - 86875
struct npc_highmaul_wild_flames : public ScriptedAI
{
    enum eSpells
    {
        WildFlamesAreaTrigger = 173616,
        WildFlamesSearcher = 173799,
        WildFlamesMissile = 173823
    };

    npc_highmaul_wild_flames(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        me->CastSpell(me, WildFlamesAreaTrigger, true);

        SetCanSeeEvenInPassiveMode(true);
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case WildFlamesSearcher:
                me->CastSpell(target, WildFlamesMissile, true);
                break;
            default:
                break;
        }
    }
};

/// Breaker Ritualist <Breaker of Frost> - 86326
struct npc_highmaul_breaker_of_frost : public ScriptedAI
{
    enum eSpells
    {
        /// Cosmetic
        FelBreakerFrostChannel = 172448,

        /// Damaging Spells
        FrozenCore = 174404
    };

    enum eEvent
    {
        EventFrozenCore = 1
    };

    npc_highmaul_breaker_of_frost(Creature* creature) : ScriptedAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    InstanceScript* m_Instance;

    void Reset() override
    {
        m_Events.Reset();

        me->CastSpell(me, FelBreakerFrostChannel, false);
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        m_Events.RescheduleEvent(EventFrozenCore, 4 * IN_MILLISECONDS);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_Instance)
            return;

        if (Creature* boss = m_Instance->GetCreature((Koragh)))
            if (boss->IsAIEnabled)
                boss->AI()->SetGUID(me->GetGUID());
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
            case EventFrozenCore:
                me->CastSpell(me, FrozenCore, false);
                m_Events.RescheduleEvent(EventFrozenCore, 13 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Volatile Anomaly - 79956
struct npc_highmaul_koragh_volatile_anomaly : public ScriptedAI
{
    enum eSpells
    {
        AlphaFadeOut = 141608,
        Destabilize = 163466,
        SuppressionFieldSilence = 162595
    };

    npc_highmaul_koragh_volatile_anomaly(Creature* creature) : ScriptedAI(creature)
    {
        m_Exploded = false;
    }

    bool m_Exploded;

    void Reset() override
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);

        if (Player* target = me->FindNearestPlayer(30.0f))
            AttackStart(target);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (m_Exploded)
            return;

        if (damage > me->GetHealth() && !me->HasAura(SuppressionFieldSilence))
        {
            m_Exploded = true;
            me->CastSpell(me, AlphaFadeOut, true);
            me->CastSpell(me, Destabilize, true);
        }
    }
};

/// Chain - 233127
struct go_highmaul_chain : public GameObjectAI
{
    go_highmaul_chain(GameObject* gameObject) : GameObjectAI(gameObject) { }

    enum eAnimID
    {
        AnimKit = 947
    };

    void Reset() override
    {
        go->SetAnimKitId(AnimKit);
    }
};

/// Frozen Core - 174404
class spell_highmaul_frozen_core : public SpellScript
{
    PrepareSpellScript(spell_highmaul_frozen_core);

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        if (Unit* caster = GetCaster())
            targets.remove_if(Trinity::UnitAuraCheck(true, GetSpellInfo()->Id, caster->GetGUID()));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_frozen_core::CorrectTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_frozen_core::CorrectTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_frozen_core::CorrectTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

class spell_highmaul_frozen_core_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_highmaul_frozen_core_AuraScript);

    enum eSpell
    {
        FrozenCoreAura = 174405
    };

    uint32 m_DamageTimer;

    bool Load() override
    {
        m_DamageTimer = 200;
        return true;
    }

    void OnUpdate(uint32 diff)
    {
        if (m_DamageTimer)
        {
            if (m_DamageTimer <= diff)
            {
                m_DamageTimer = 200;

                if (Unit* target = GetUnitOwner())
                {
                    std::list<Unit*> targetList;
                    Trinity::AnyFriendlyUnitInObjectRangeCheck check(target, target, 8.0f);
                    Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(target, targetList, check);
                    target->VisitNearbyObject(8.0f, searcher);

                    for (Unit* l_Unit : targetList)
                        target->CastSpell(l_Unit, FrozenCoreAura, true);
                }
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_frozen_core_AuraScript::OnUpdate);
    }
};

/// Wild Flames (AreaTrigger) - 173616
class spell_highmaul_wild_flames_areatrigger : public AuraScript
{
    PrepareAuraScript(spell_highmaul_wild_flames_areatrigger);

    enum eSpells
    {
        WildFlamesSearcher = 173799,
        WildFlamesDoT = 173827
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
                if (Unit* target = GetUnitOwner())
                {
                    std::list<Unit*> targetList;
                    float radius = 6.0f;

                    Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(target, target, radius);
                    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, targetList, check);
                    target->VisitNearbyObject(radius, searcher);

                    for (Unit* itr : targetList)
                    {
                        if (itr->GetDistance(target) <= 3.0f)
                            itr->CastSpell(itr, WildFlamesDoT, true);
                        else
                            itr->RemoveAura(WildFlamesDoT);
                    }
                }

                m_DamageTimer = 500;
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, WildFlamesSearcher, true);
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_wild_flames_areatrigger::OnUpdate);
        AfterEffectRemove += AuraEffectRemoveFn(spell_highmaul_wild_flames_areatrigger::OnRemove, EFFECT_0, SPELL_AURA_CREATE_AREATRIGGER, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Nullification Barrier - 156803
class spell_highmaul_nullification_barrier : public AuraScript
{
    PrepareAuraScript(spell_highmaul_nullification_barrier);

    enum eSpell
    {
        BreakersStrength = 162569
    };

    enum eAction
    {
        CancelBreakersStrength
    };

    int32 m_AbsorbAmount;

    bool Load() override
    {
        m_AbsorbAmount = 0;
        return true;
    }

    void AfterApply(AuraEffect const* auraEffect, AuraEffectHandleModes /*mode*/)
    {
        m_AbsorbAmount = auraEffect->GetAmount();
    }

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (m_AbsorbAmount <= 0)
            return;

        if (Unit* target = GetTarget())
        {
            if (AuraEffect* absorbAura = target->GetAuraEffect(GetSpellInfo()->Id, EFFECT_0, target->GetGUID()))
            {
                int32 pct = int32(((float)absorbAura->GetAmount() / (float)m_AbsorbAmount) * 100.0f);
                target->SetPower(POWER_ALTERNATE, pct);
            }
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetUnitOwner() == nullptr)
            return;

        if (Creature* boss = GetUnitOwner()->ToCreature())
        {
            boss->SetPower(POWER_ALTERNATE, 0);

            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (boss->IsAIEnabled && removeMode != AURA_REMOVE_BY_DEATH)
                boss->AI()->DoAction(CancelBreakersStrength);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_highmaul_nullification_barrier::AfterApply, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_nullification_barrier::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_nullification_barrier::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Caustic Energy - 160720
class spell_highmaul_caustic_energy : public AuraScript
{
    PrepareAuraScript(spell_highmaul_caustic_energy);

    enum eSpell
    {
        CausticEnergyDoT = 161242
    };

    enum eDatas
    {
        DataMaxRunicPlayers,
        DataRunicPlayersCount
    };

    enum eTalk
    {
        CausticEnergyWarn = 9
    };

    enum eCreature
    {
        VolatileAnomaly = 79956
    };

    uint32 m_DamageTimer;

    bool Load() override
    {
        m_DamageTimer = 200;
        return true;
    }

    void OnUpdate(uint32 diff)
    {
        if (m_DamageTimer)
        {
            if (m_DamageTimer <= diff)
            {
                if (Unit* target = GetUnitOwner())
                {
                    std::list<Unit*> targetList;
                    float radius = 10.0f;

                    Trinity::AnyUnitInObjectRangeCheck check(target, radius);
                    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(target, targetList, check);
                    target->VisitNearbyObject(radius, searcher);

                    if (targetList.empty())
                        return;

                    targetList.remove_if([this](Unit* p_Unit) -> bool
                    {
                        if (p_Unit == nullptr || p_Unit->ToCreature())
                            return true;

                        return false;
                    });

                    bool l_CanChargePlayer = false;
                    Creature* boss = nullptr;
                    uint8 l_ChargingCount = 0;

                    if (InstanceScript* instance = target->GetInstanceScript())
                    {
                        if (boss = Creature::GetCreature(*target, instance->GetGuidData(Koragh)))
                        {
                            if (boss->IsAIEnabled)
                            {
                                l_ChargingCount = boss->AI()->GetData(DataRunicPlayersCount);
                                l_CanChargePlayer = l_ChargingCount < boss->AI()->GetData(DataMaxRunicPlayers);
                            }
                        }
                    }

                    if (boss == nullptr || !boss->IsAIEnabled)
                    {
                        m_DamageTimer = 200;
                        return;
                    }

                    for (Unit* itr : targetList)
                    {
                        if (itr->GetDistance(target) <= 7.0f)
                        {
                            if (!itr->HasAura(CausticEnergyDoT) && l_CanChargePlayer)
                            {
                                itr->CastSpell(itr, CausticEnergyDoT, true);

                                if (boss != nullptr && boss->IsAIEnabled)
                                {
                                    boss->MonsterTextEmote(CausticEnergyWarn, itr->GetGUID());
                                    boss->AI()->SetData(DataRunicPlayersCount, l_ChargingCount + 1);
                                }
                            }
                        }
                        else
                        {
                            if (itr->HasAura(CausticEnergyDoT))
                                itr->RemoveAura(CausticEnergyDoT);
                        }

                        /// Must be updated
                        l_ChargingCount = boss->AI()->GetData(DataRunicPlayersCount);
                        l_CanChargePlayer = l_ChargingCount < boss->AI()->GetData(DataMaxRunicPlayers);
                    }
                }

                m_DamageTimer = 200;
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetUnitOwner())
        {
            std::list<Unit*> targetList;
            float radius = 10.0f;

            Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(target, target, radius);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(target, targetList, check);
            target->VisitNearbyObject(radius, searcher);

            for (Unit* itr : targetList)
                itr->RemoveAura(CausticEnergyDoT);
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_caustic_energy::OnUpdate);
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_caustic_energy::OnRemove, EFFECT_0, SPELL_AURA_CREATE_AREATRIGGER, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Caustic Energy (DoT) - 161242
class spell_highmaul_caustic_energy_dot : public AuraScript
{
    PrepareAuraScript(spell_highmaul_caustic_energy_dot);

    enum eSpells
    {
        NullificationBarrierPower = 163612,
        NullificationBarrierAbsorb = 163134,
        MarkOfNullification = 172886
    };

    enum eDatas
    {
        DataRunicPlayersCount = 1
    };

    void OnApply(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
        {
            target->CastSpell(target, NullificationBarrierPower, true);
            target->CastSpell(target, MarkOfNullification, true);
            target->SetPower(POWER_ALTERNATE, 0);
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
        {
            if (Unit* target = GetTarget())
            {
                target->CastSpell(target, NullificationBarrierAbsorb, true);
                target->RemoveAura(MarkOfNullification);

                if (InstanceScript* instance = caster->GetInstanceScript())
                {
                    if (Creature* boss = Creature::GetCreature(*target, instance->GetGuidData(Koragh)))
                    {
                        if (boss->IsAIEnabled)
                        {
                            uint8 actualCount = boss->AI()->GetData(DataRunicPlayersCount);
                            if (actualCount > 0)
                                boss->AI()->SetData(DataRunicPlayersCount, actualCount - 1);
                        }
                    }
                }

                if (AuraEffect* absorb = target->GetAuraEffect(NullificationBarrierAbsorb, EFFECT_0))
                {
                    /// The Nullification Barrier received by players can absorb up to 15000000 Magic damage.
                    int32 maxAmount = 15000000;
                    int32 pct = target->GetPower(POWER_ALTERNATE);
                    /// When the rune powers down, the player receives their own Nullification Barrier, proportional to the amount of time spent inside the rune.
                    absorb->ChangeAmount(CalculatePct(maxAmount, pct));
                }
            }
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_highmaul_caustic_energy_dot::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_caustic_energy_dot::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Expel Magic: Fire - 162185
class spell_highmaul_expel_magic_fire : public AuraScript
{
    PrepareAuraScript(spell_highmaul_expel_magic_fire);

    enum eSpell
    {
        ExpelMagicFireAoE = 172685
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, ExpelMagicFireAoE, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_expel_magic_fire::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Expel Magic: Arcane - 162186
class spell_highmaul_expel_magic_arcane : public AuraScript
{
    PrepareAuraScript(spell_highmaul_expel_magic_arcane);

    enum eSpell
    {
        ExpelMagicArcaneMissile = 162398
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
            target->CastSpell(target, ExpelMagicArcaneMissile, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_expel_magic_arcane::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Nullification Barrier (Absorb) - 163134
class spell_highmaul_nullification_barrier_player : public AuraScript
{
    PrepareAuraScript(spell_highmaul_nullification_barrier_player);

    enum eSpell
    {
        NullificationBarrierPower = 163612
    };

    void OnAbsorb(AuraEffect* auraEffect, DamageInfo& /*p_DmgInfo*/, float& /*p_AbsorbAmount*/)
    {
        if (Unit* target = GetTarget())
        {
            /// The Nullification Barrier received by players can absorb up to 15000000 Magic damage.
            target->SetPower(POWER_ALTERNATE, int32(((float)auraEffect->GetAmount() / (float)15000000) * 100.0f));
        }
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetUnitOwner())
            target->RemoveAura(NullificationBarrierPower);
    }

    void Register() override
    {
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_highmaul_nullification_barrier_player::OnAbsorb, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_nullification_barrier_player::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Expel Magic: Frost - 172813
class spell_highmaul_expel_magic_frost_aura : public AuraScript
{
    PrepareAuraScript(spell_highmaul_expel_magic_frost_aura);

    enum eSpell
    {
        ExpelMagicFrostAreaTrigger = 172747
    };

    void OnTick(AuraEffect const* auraEffect)
    {
        //if (Unit* target = GetTarget())
        //{
        //    if (AreaTrigger* areaTrigger = target->FindNearestAreaTrigger(ExpelMagicFrostAreaTrigger, 30.0f))
        //    {
        //        float distance = target->GetDistance(areaTrigger);
        //        float l_MinDistance = 5.0f;
        //        if (distance <= l_MinDistance)
        //            return;

        //        float l_MaxDistance = 25.0f;
        //        int32 maxAmount = 0;

        //        if (AuraEffect* l_SpeedEffect = auraEffect->GetBase()->GetEffect(EFFECT_0))
        //        {
        //            maxAmount = l_SpeedEffect->GetBaseAmount();
        //            if (!maxAmount)
        //                return;

        //            int32 l_NewAmount = int32(maxAmount * (distance / l_MaxDistance));
        //            l_SpeedEffect->ChangeAmount(l_NewAmount);
        //        }
        //    }
        //    else
        //        auraEffect->GetBase()->Remove();
        //}
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_expel_magic_frost_aura::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Suppression Field (aura) - 161328
class spell_highmaul_suppression_field_aura : public AuraScript
{
    PrepareAuraScript(spell_highmaul_suppression_field_aura);

    enum eAction
    {
        SuppressionField = 1
    };

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
        if (removeMode != AURA_REMOVE_BY_EXPIRE || GetTarget() == nullptr)
            return;

        if (Creature* target = GetTarget()->ToCreature())
            if (target->IsAIEnabled)
                target->AI()->DoAction(SuppressionField);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_suppression_field_aura::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Pair Annihilation - 8976
class achievement_highmaul_pair_annihilation : public AchievementCriteriaScript
{
public:
    achievement_highmaul_pair_annihilation() : AchievementCriteriaScript("achievement_highmaul_pair_annihilation") { }

    bool OnCheck(Player* source, Unit* /*target*/) override
    {
        if (!source || !source->GetInstanceScript())
            return false;

        if (source->GetInstanceScript()->GetData(KoraghAchievement))
            return true;

        return false;
    }
};

void AddSC_boss_koragh()
{
    RegisterHighmaulCreatureAI(boss_koragh);
    RegisterHighmaulCreatureAI(npc_highmaul_breaker_of_fel);
    RegisterHighmaulCreatureAI(npc_highmaul_breaker_of_fire);
    RegisterHighmaulCreatureAI(npc_highmaul_wild_flames);
    RegisterHighmaulCreatureAI(npc_highmaul_breaker_of_frost);
    RegisterHighmaulCreatureAI(npc_highmaul_koragh_volatile_anomaly);

    RegisterGameObjectAI(go_highmaul_chain);

    //RegisterSpellAndAuraScriptPair(spell_highmaul_frozen_core, spell_highmaul_frozen_core_AuraScript);
    //RegisterAuraScript(spell_highmaul_wild_flames_areatrigger);
    //RegisterAuraScript(spell_highmaul_nullification_barrier);
    //RegisterAuraScript(spell_highmaul_caustic_energy);
    //RegisterAuraScript(spell_highmaul_caustic_energy_dot);
    RegisterAuraScript(spell_highmaul_expel_magic_fire);
    RegisterAuraScript(spell_highmaul_expel_magic_arcane);
    RegisterAuraScript(spell_highmaul_nullification_barrier_player);
    RegisterAuraScript(spell_highmaul_expel_magic_frost_aura);
    RegisterAuraScript(spell_highmaul_suppression_field_aura);

    //new achievement_highmaul_pair_annihilation();
}
