////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2016 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

# include "highmaul.hpp"

uint8 GetEnergyGainFromHealth(float healthPct)
{
    if (healthPct >= 50.0f)
        return 1;
    else if (healthPct >= 20.0f)
        return 2;
    else if (healthPct >= 10.0f)
        return 3;
    else if (healthPct >= 5.0f)
        return 4;
    else
        return 5;
}

void RespawnGuardians(Creature* source, InstanceScript* instance)
{
    if (source == nullptr || instance == nullptr)
        return;

    if (Creature* rokka = Creature::GetCreature(*source, instance->GetGuidData(Rokka)))
    {
        rokka->Respawn();
        rokka->GetMotionMaster()->MoveTargetedHome();
    }

    if (Creature* lokk = Creature::GetCreature(*source, instance->GetGuidData(Lokk)))
    {
        lokk->Respawn();
        lokk->GetMotionMaster()->MoveTargetedHome();
    }

    if (Creature* oro = Creature::GetCreature(*source, instance->GetGuidData(Oro)))
    {
        oro->Respawn();
        oro->GetMotionMaster()->MoveTargetedHome();
    }
}

void StartGuardians(Creature* source, Unit* target)
{
    if (source == nullptr || target == nullptr)
        return;

    if (Creature* rokka = source->FindNearestCreature(Rokka, 100.0f))
        rokka->AI()->AttackStart(target);

    if (Creature* lokk = source->FindNearestCreature(Lokk, 100.0f))
        lokk->AI()->AttackStart(target);

    if (Creature* oro = source->FindNearestCreature(Oro, 100.0f))
        oro->AI()->AttackStart(target);
}

/// Tectus <The Living Mountain> - 78948
/// Shard of Tectus <Shard of the Mountain> - 80551
/// Mote of Tectus <Mote of the Mountain> - 80557
struct boss_tectus : public BossAI
{
    enum eSpells
    {
        /// Misc
        BreakPlayerTargetting = 140562,
        SuicideNoBloodNoLogging = 117624,
        SpawnTectusShards = 169931,
        EncounterEvent = 181089,   ///< Don't know why, maybe useful
        MidsizeTectusDamageReduct = 178193,   ///< -30% damage done
        MoteOfTectusDamageReduct = 178194,   ///< -60% damage done
        EarthFurySpawnDustCloud = 169949,
        Grow = 166306,
        /// Energy Management
        ZeroPowerZeroRegen = 118357,
        /// Periodic dummy of 1s
        TheLivingMountain = 162287,
        ShardOfTheMountain = 162658,
        MoteOfTheMountain = 162674,
        /// Arrow visual on player
        SpellCrystallineBarrage = 162346,
        CrystallineBarrageSummon = 162371,
        CrystallineBarrageDoT = 162370,
        /// +5% damage done
        Accretion = 162288,
        Petrification = 163809,
        /// Fracture
        FractureSearcher = 163214,   ///< Must trigger 163208
        FractureMissile = 163208,
        SpellTectonicUpheaval = 162475,
        /// Loot
        TectusBonus = 177523
    };

    enum eEvents
    {
        EventEnrage = 1,
        EventSpawnBersererk,
        EventSpawnEarthwarper,
        EventCrystallineBarrage,
        EventSpawnCrystalline,
        /// Used at 25 energy
        EventEarthenPillar,
        EventFracture,
        EventAccretion,
        EventTectonicUpheaval
    };

    enum eActions
    {
        GuardianDead,
        ScheduleEarthenPillar,
        ScheduleTectonicUpheaval,
        MoteKilled
    };

    enum eAnimKits
    {
        AnimRise = 6961,
        AnimRise2 = 6958,
        AnimFall = 6918,
        AnimWounded = 2156
    };

    enum eCreatures
    {
        EarthenPillarStalker = 80476,
        ShardOfTectus = 80551,
        MoteOfTectus = 80557,
        NightTwistedBerserker = 80822,
        NightTwistedEarthwarper = 80599
    };

    enum eTalks
    {
        Aggro,
        Shattered,
        TectonicUpheaval,
        TectonicUpheavalCompleted,
        EarthenPillar,
        Slay,
        Death,
        CrystallineBarrage
    };

    enum eMiscs
    {
        ShardSpawnCount = 2,
        MotesSpawnCount = 4,
        InvisDisplay = 11686
    };


    boss_tectus(Creature* creature) : BossAI(creature, BossTectus)
    {
        m_Instance = creature->GetInstanceScript();
    }

    EventMap m_Events;
    InstanceScript* m_Instance;

    ObjectGuid m_CrystallineBarrageTarget;

    Position m_FirstCrystalline;
    Position m_SecondCrystalline;

    bool m_TectonicScheduled;

    uint8 m_MoteKilled;

    void Reset() override
    {
        m_Events.Reset();

        if (me->GetEntry() == Tectus)
            _Reset();

        me->CastSpell(me, ZeroPowerZeroRegen, true);

        me->ClearUnitState(UNIT_STATE_STUNNED);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_PREPARATION | UNIT_FLAG_DISARMED);

        switch (me->GetEntry())
        {
            case Tectus:
            {
                me->CastSpell(me, TheLivingMountain, true);

                if (m_Instance)
                    m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellCrystallineBarrage);

                if (!AllGardiansDead())
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                }
                else
                {
                    me->SetAnimKitId(0);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                }

                break;
            }
            case ShardOfTectus:
            {
                me->CastSpell(me, ShardOfTheMountain, true);
                me->CastSpell(me, Grow, true);
                me->SetDisplayId(InvisDisplay);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                me->AddUnitState(UNIT_STATE_STUNNED);
                break;
            }
            case MoteOfTectus:
            {
                me->CastSpell(me, MoteOfTheMountain, true);
                me->CastSpell(me, Grow, true);
                me->SetDisplayId(InvisDisplay);
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                me->AddUnitState(UNIT_STATE_STUNNED);
                break;
            }
            default:
                break;
        }

        me->RemoveAura(Berserker);

        me->SetPower(POWER_ENERGY, 0);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER | UNIT_FLAG2_DISABLE_TURN);

        m_FirstCrystalline = Position();
        m_SecondCrystalline = Position();

        m_TectonicScheduled = false;

        m_MoteKilled = 0;
    }

    void JustReachedHome() override
    {
        if (!AllGardiansDead())
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }
        else
        {
            me->SetAnimKitId(0);
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
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
            case GuardianDead:
            {
                if (!AllGardiansDead())
                    return;

                me->SetAnimKitId(0);
                me->PlayOneShotAnimKit(AnimRise);

                AddDelayedEvent(4500, [this]() -> void
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                });

                break;
            }
            case ScheduleEarthenPillar:
            {
                if (m_Events.HasEvent(EventEarthenPillar))
                    break;

                m_Events.RescheduleEvent(EventEarthenPillar, 100);
                break;
            }
            case ScheduleTectonicUpheaval:
            {
                if (m_TectonicScheduled || me->HasAura(SpellTectonicUpheaval))
                    break;

                m_TectonicScheduled = true;
                m_Events.RescheduleEvent(EventTectonicUpheaval, 100);
                break;
            }
            case MoteKilled:
            {
                m_Instance->SetData(TectusAchievement, uint32(time(nullptr)));

                ++m_MoteKilled;

                if (m_MoteKilled >= (MotesSpawnCount * 2))
                {
                    me->SetAnimKitId(0);
                    me->PlayOneShotAnimKit(AnimRise);

                    AddDelayedEvent(4 * IN_MILLISECONDS, [this]() -> void
                    {
                        me->SetAnimKitId(AnimWounded);
                        Talk(Death);
                    });

                    AddDelayedEvent(8 * IN_MILLISECONDS, [this]() -> void
                    {
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                        if (Player* player = me->GetMap()->GetPlayers().begin()->getSource())
                            player->CastSpell(me, SuicideNoBloodNoLogging, true);
                    });
                }

                break;
            }
            default:
                break;
        }
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        if (!AllGardiansDead())
        {
            EnterEvadeMode();
            return;
        }

        if (me->GetEntry() == Tectus)
            _EnterCombat();

        m_Events.RescheduleEvent(EventCrystallineBarrage, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventFracture, 8 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventAccretion, 5 * IN_MILLISECONDS);

        switch (me->GetEntry())
        {
            case ShardOfTectus:
                me->CastSpell(me, MidsizeTectusDamageReduct, true);
                if (m_Instance)
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
                break;
            case MoteOfTectus:
                me->CastSpell(me, MoteOfTectusDamageReduct, true);
                if (m_Instance)
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);
                break;
            case Tectus:
                if (m_Instance)
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                Talk(Aggro);
                m_Events.RescheduleEvent(EventEnrage, IsMythicRaid() ? 480 * IN_MILLISECONDS : 600 * IN_MILLISECONDS);
                m_Events.RescheduleEvent(EventSpawnBersererk, 18 * IN_MILLISECONDS);
                m_Events.RescheduleEvent(EventSpawnEarthwarper, 8 * IN_MILLISECONDS);
                break;
            default:
                break;
        }
    }

    void KilledUnit(Unit* killed) override
    {
        if (killed->IsPlayer() && me->GetEntry() == Tectus)
            Talk(Slay);
    }

    void JustDied(Unit* /*killer*/) override
    {
        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellCrystallineBarrage);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(CrystallineBarrageDoT);

        if (me->GetEntry() == Tectus)
        {
            _JustDied();
            CastSpellToPlayers(me->GetMap(), me, TectusBonus, true);
        }

        m_Instance->instance->ApplyOnEveryPlayer([&](Player* player)
        {
            if (!player)
                return;

            /// Hacky but don't know why combat doesn't stop
            if (!player->isAttackingPlayer())
                player->CombatStop();
        });

        std::list<Creature*> creatures;
        me->GetCreatureListWithEntryInGrid(creatures, MoteOfTectus, 150.0f);

        for (Creature* creature : creatures)
        {
            me->Kill(creature);
            creature->DespawnOrUnsummon();
        }

        creatures.clear();

        me->GetCreatureListWithEntryInGrid(creatures, EarthenPillarStalker, 150.0f);

        for (Creature* creature : creatures)
        {
            me->Kill(creature);
            creature->DespawnOrUnsummon();
        }
    }

    void EnterEvadeMode() override
    {
        me->ClearUnitState(UNIT_STATE_STUNNED);

        CreatureAI::EnterEvadeMode();

        m_Instance->SetBossState(BossTectus, FAIL);

        m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        m_Instance->DoRemoveAurasDueToSpellOnPlayers(SpellCrystallineBarrage);
        m_Instance->DoRemoveAurasDueToSpellOnPlayers(CrystallineBarrageDoT);

        std::list<Creature*> motes;
        me->GetCreatureListWithEntryInGrid(motes, MoteOfTectus, 150.0f);

        for (Creature* creature : motes)
        {
            if (creature->IsAIEnabled)
                creature->AI()->Reset();

            creature->DespawnOrUnsummon();
        }
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr)
            return;

        switch (spellInfo->Id)
        {
            case FractureSearcher:
                me->CastSpell(target, FractureMissile, true);
                break;
            case Petrification:
                Talk(TectonicUpheavalCompleted);
                if (me->GetEntry() == Tectus)
                    me->CastSpell(me, TheLivingMountain, true);
                else if (me->GetEntry() == ShardOfTectus)
                    me->CastSpell(me, ShardOfTheMountain, true);
                break;
            case SpellTectonicUpheaval:
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                m_TectonicScheduled = false;
                break;
            case SpawnTectusShards:
            {
                target->PlayOneShotAnimKit(AnimRise2);
                target->RestoreDisplayId();

                ObjectGuid guid = target->GetGUID();
                AddDelayedEvent(4 * IN_MILLISECONDS, [this, guid]() -> void
                {
                    if (Unit* target = Unit::GetUnit(*me, guid))
                        target->CastSpell(target, EarthFurySpawnDustCloud, true);
                });

                AddDelayedEvent(6 * IN_MILLISECONDS, [this, guid]() -> void
                {
                    if (Unit* target = Unit::GetUnit(*me, guid))
                        target->ClearUnitState(UNIT_STATE_STUNNED);
                });

                break;
            }
            default:
                break;
        }
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType dmgType) override
    {
        if (attacker == me)
            return;

        if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
        {
            damage = 0;
            return;
        }

        /// This buff cause Tectus to be unkillable, although he can still be damaged during this time.
        if (damage > me->GetHealth())
        {
            if (me->HasAura(TheLivingMountain) || me->HasAura(ShardOfTheMountain))
            {
                me->SetHealth(1);
                damage = 0;
                return;
            }
            else
            {
                m_Events.Reset();

                me->RemoveAllAuras();
                me->InterruptNonMeleeSpells(true);
                me->SetHealth(1);
                damage = 0;

                me->CastSpell(me, EncounterEvent, true);
                me->CastSpell(me, BreakPlayerTargetting, true);
                me->CastSpell(me, ZeroPowerZeroRegen, true);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);

                me->AddUnitState(UNIT_STATE_STUNNED);
                me->SetAnimKitId(AnimFall);

                if (m_Instance)
                    m_Instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (me->GetEntry() == Tectus)
                {
                    Talk(Shattered);
                    SpawnShards();
                }
                else if (me->GetEntry() == ShardOfTectus)
                    SpawnMotes();
                else if (Creature* tectus = m_Instance->GetCreature((Tectus)))
                    tectus->AI()->DoAction(MoteKilled);
            }
        }
    }

    void RegeneratePower(Powers /*power*/, float& value) override
    {
        /// Tectus only regens by script
        value = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (me->GetDistance(me->GetHomePosition()) >= 70.0f)
        {
            EnterEvadeMode();
            return;
        }

        m_Events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        switch (m_Events.ExecuteEvent())
        {
            case EventEnrage:
            {
                me->CastSpell(me, Berserker, true);

                std::list<Creature*> creature;
                me->GetCreatureListWithEntryInGrid(creature, ShardOfTectus, 150.0f);

                for (Creature* shard : creature)
                    shard->CastSpell(shard, Berserker, true);

                creature.clear();
                me->GetCreatureListWithEntryInGrid(creature, MoteOfTectus, 150.0f);

                for (Creature* mote : creature)
                    mote->CastSpell(mote, Berserker, true);

                break;
            }
            case EventSpawnBersererk:
                SpawnAdd(NightTwistedBerserker);
                m_Events.RescheduleEvent(EventSpawnBersererk, 40 * IN_MILLISECONDS);
                break;
            case EventSpawnEarthwarper:
                SpawnAdd(NightTwistedEarthwarper);
                m_Events.RescheduleEvent(EventSpawnEarthwarper, 40 * IN_MILLISECONDS);
                break;
            case EventCrystallineBarrage:
            {
                /// Crystalline Barrage should not select Main Tank or Off Tank
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -20.0f, true);
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f, true);
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 2, 0.0f, true);

                if (target != nullptr)
                {
                    Talk(CrystallineBarrage, target->GetGUID());

                    me->CastSpell(target, SpellCrystallineBarrage, true);
                    m_CrystallineBarrageTarget = target->GetGUID();

                    AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
                    {
                        float range = 7.0f;
                        float o = me->GetOrientation();
                        float originX = me->GetPositionX();
                        float originY = me->GetPositionY();
                        float z = me->GetPositionZ();

                        float x = originX + (range * cos(o - M_PI / 2));
                        float y = originY + (range * sin(o - M_PI / 2));
                        me->CastSpell(x, y, z, CrystallineBarrageSummon, true);

                        m_FirstCrystalline.m_positionX = x;
                        m_FirstCrystalline.m_positionY = y;
                        m_FirstCrystalline.m_positionZ = z;

                        x = originX + (range * cos(o + M_PI / 2));
                        y = originY + (range * sin(o + M_PI / 2));
                        me->CastSpell(x, y, z, CrystallineBarrageSummon, true);

                        m_SecondCrystalline.m_positionX = x;
                        m_SecondCrystalline.m_positionY = y;
                        m_SecondCrystalline.m_positionZ = z;

                        m_Events.RescheduleEvent(EventSpawnCrystalline, 500);
                    });
                }

                m_Events.RescheduleEvent(EventCrystallineBarrage, 30 * IN_MILLISECONDS);
                break;
            }
            case EventSpawnCrystalline:
            {
                if (Unit* target = Unit::GetUnit(*me, m_CrystallineBarrageTarget))
                {
                    if (!target->HasAura(SpellCrystallineBarrage))
                    {
                        m_CrystallineBarrageTarget.Clear();
                        m_FirstCrystalline = Position();
                        m_SecondCrystalline = Position();
                        break;
                    }

                    float range = 2.0f;
                    float z = me->GetPositionZ();

                    float o = m_FirstCrystalline.GetAngle(target);
                    float x = m_FirstCrystalline.m_positionX + (range * cos(o));
                    float y = m_FirstCrystalline.m_positionY + (range * sin(o));
                    me->CastSpell(x, y, z, CrystallineBarrageSummon, true);

                    m_FirstCrystalline.m_positionX = x;
                    m_FirstCrystalline.m_positionY = y;
                    m_FirstCrystalline.m_positionZ = z;

                    o = m_SecondCrystalline.GetAngle(target);
                    x = m_SecondCrystalline.m_positionX + (range * cos(o));
                    y = m_SecondCrystalline.m_positionY + (range * sin(o));
                    me->CastSpell(x, y, z, CrystallineBarrageSummon, true);

                    m_SecondCrystalline.m_positionX = x;
                    m_SecondCrystalline.m_positionY = y;
                    m_SecondCrystalline.m_positionZ = z;
                }

                m_Events.RescheduleEvent(EventSpawnCrystalline, 500);
                break;
            }
            case EventFracture:
                me->CastSpell(me, FractureSearcher, true);
                m_Events.RescheduleEvent(EventFracture, 6 * IN_MILLISECONDS);
                break;
            case EventEarthenPillar:
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -20.0f, true);
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f, true);
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 2, 0.0f, true);

                if (target != nullptr)
                {
                    float x = target->GetPositionX();
                    float y = target->GetPositionY();
                    me->SummonCreature(EarthenPillarStalker, x, y, me->GetPositionZ());
                }

                Talk(EarthenPillar);
                break;
            }
            case EventAccretion:
                me->CastSpell(me, Accretion, true);
                m_Events.RescheduleEvent(EventAccretion, 5 * IN_MILLISECONDS);
                break;
            case EventTectonicUpheaval:
                /// If Tectus's health is depleted during Tectonic Upheaval, he will Shatter.
                Talk(TectonicUpheaval);
                me->RemoveAura(TheLivingMountain);
                me->RemoveAura(ShardOfTheMountain);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);

                me->CastSpell(me, SpellTectonicUpheaval, false);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }

    bool AllGardiansDead() const
    {
        if (!m_Instance)
            return false;

        if (Creature* rokka = m_Instance->GetCreature((Rokka)))
            if (rokka->isAlive())
                return false;

        if (Creature* lokk = m_Instance->GetCreature((Lokk)))
            if (lokk->isAlive())
                return false;

        if (Creature* oro = m_Instance->GetCreature((Oro)))
            if (oro->isAlive())
                return false;

        return true;
    }

    void SpawnShards()
    {
        float originX = me->GetPositionX();
        float originY = me->GetPositionY();
        float z = me->GetPositionZ();
        float range = 7.0f;

        for (uint8 i = 0; i < ShardSpawnCount; ++i)
        {
            float o = frand(0, 2 * M_PI);
            float x = originX + (range * cos(o));
            float y = originY + (range * sin(o));

            if (Creature* shard = me->SummonCreature(ShardOfTectus, x, y, z))
                me->CastSpell(shard, SpawnTectusShards, true);
        }
    }

    void SpawnMotes()
    {
        float originX = me->GetPositionX();
        float originY = me->GetPositionY();
        float z = me->GetPositionZ();
        float range = 7.0f;

        for (uint8 i = 0; i < MotesSpawnCount; ++i)
        {
            float o = frand(0, 2 * M_PI);
            float x = originX + (range * cos(o));
            float y = originY + (range * sin(o));

            if (Creature* mote = me->SummonCreature(MoteOfTectus, x, y, z))
                me->CastSpell(mote, SpawnTectusShards, true);
        }
    }

    void SpawnAdd(uint32 p_Entry)
    {
        Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, -20.0f, true);
        if (!target)
            target = SelectTarget(SELECT_TARGET_RANDOM, 0, -10.0f, true);
        if (!target)
            target = SelectTarget(SELECT_TARGET_RANDOM, 2, 0.0f, true);

        if (target != nullptr)
        {
            float o = frand(0, 2 * M_PI);
            float range = 5.0f;
            float x = target->GetPositionX() + (range * cos(o));
            float y = target->GetPositionY() + (range * sin(o));

            if (Creature* add = me->SummonCreature(p_Entry, x, y, me->GetPositionZ()))
                if (add->IsAIEnabled)
                    add->AI()->AttackStart(target);
        }
    }
};

/// Night-Twisted Supplicant - 86185
struct npc_highmaul_night_twisted_supplicant : public MS::AI::CosmeticAI
{
    enum eSpell
    {
        NightTwistedCovenant = 172138
    };

    enum eDisplay
    {
        InvisDisplay = 11686
    };

    enum eAction
    {
        Rise
    };

    npc_highmaul_night_twisted_supplicant(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
        SetCanSeeEvenInPassiveMode(true);
    }

    bool CanRespawn() override
    {
        /// This mob is only cosmetic for Rokka, Lokk and Oro spawn
        return false;
    }

    void SpellHitTarget(Unit* target, SpellInfo const* spellInfo) override
    {
        if (target == nullptr || spellInfo->Id != NightTwistedCovenant)
            return;

        target->SetDisplayId(InvisDisplay);
    }

    void MoveInLineOfSight(Unit* mover) override
    {
        if (me->GetDisplayId() == InvisDisplay)
            return;

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        if (!mover->IsPlayer())
            return;

        std::list<Creature*> otherMe;
        me->GetCreatureListWithEntryInGrid(otherMe, me->GetEntry(), 100.0f);

        for (Creature* covenant : otherMe)
            covenant->CastSpell(covenant, NightTwistedCovenant, false);

        AddDelayedEvent(5000, [this]() -> void
        {
            if (Creature* rokka = me->FindNearestCreature(Rokka, 100.0f))
                rokka->AI()->DoAction(Rise);

            if (Creature* lokk = me->FindNearestCreature(Lokk, 100.0f))
                lokk->AI()->DoAction(Rise);

            if (Creature* oro = me->FindNearestCreature(Oro, 100.0f))
                oro->AI()->DoAction(Rise);
        });
    }
};

/// Rokkaa <Heart of Tectus> - 86071
/// Lokk <Hands of Tectus> - 86073
struct npc_highmaul_rokka_and_lokk : public MS::AI::CosmeticAI
{
    enum eAnimKit
    {
        AnimRise = 6961
    };

    enum eActions
    {
        Rise = 0,
        GuardianDead = 0
    };

    enum eSpells
    {
        /// Grow scale, regens 10% of health every second
        Reconstitution = 172116,
        ReconstitutionScale = 172126,
        /// AoE on caster
        EarthenThrust = 172115,
        /// Trigger 172073 missile
        /// If no player hit, casts 172078
        MeteoricEarthspire = 172110
    };

    enum eEvents
    {
        EventEarthenThrust = 1,
        EventMeteoricEarthspire,
        EventReconstitution
    };

    npc_highmaul_rokka_and_lokk(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
        m_Risen = false;
    }

    bool m_Risen;

    EventMap m_Events;
    InstanceScript* m_Instance;

    void Reset() override
    {
        if (!m_Risen)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }
        else
            me->SetAnimKitId(0);

        m_Events.Reset();

        RespawnGuardians(me, m_Instance);
    }

    void JustReachedHome() override
    {
        if (m_Risen)
            me->SetAnimKitId(0);
    }

    bool CanRespawn() override
    {
        return false;
    }

    void EnterCombat(Unit* attacker) override
    {
        StartGuardians(me, attacker);

        m_Events.RescheduleEvent(EventEarthenThrust, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventMeteoricEarthspire, 10 * IN_MILLISECONDS);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (m_Events.HasEvent(EventReconstitution) || me->HasAura(ReconstitutionScale))
            return;

        if (me->HealthBelowPctDamaged(21, damage))
            m_Events.RescheduleEvent(EventReconstitution, 100);
    }

    void DoAction(int32 const action) override
    {
        if (action == Rise && !m_Risen)
        {
            m_Risen = true;
            me->SetAnimKitId(0);
            me->PlayOneShotAnimKit(AnimRise);

            AddDelayedEvent(4500, [this]() -> void
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
            });
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* tectus = me->FindNearestCreature(Tectus, 100.0f))
            tectus->AI()->DoAction(GuardianDead);
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
            case EventEarthenThrust:
                me->CastSpell(me, EarthenThrust, false);
                m_Events.RescheduleEvent(EventEarthenThrust, 12 * IN_MILLISECONDS);
                break;
            case EventMeteoricEarthspire:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, MeteoricEarthspire, false);
                m_Events.RescheduleEvent(EventMeteoricEarthspire, 12 * IN_MILLISECONDS);
                break;
            case EventReconstitution:
                me->CastSpell(me, ReconstitutionScale, true);
                me->CastSpell(me, Reconstitution, false);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Oro <Wrath of Tectus> - 86072
struct npc_highmaul_oro : public MS::AI::CosmeticAI
{
    enum eAnimKit
    {
        AnimRise = 6961
    };

    enum eActions
    {
        Rise = 0,
        GuardianDead = 0
    };

    enum eSpells
    {
        /// Grow scale, regens 10% of health every second
        Reconstitution = 172116,
        ReconstitutionScale = 172126,
        /// Launches Stonebolt at targets further than 5 yards
        StoneboltVolley = 172058,
        /// Poisons a player, inflicting damage to his allies
        RadiatingPoison = 172066
    };

    enum eEvents
    {
        EventStoneboltVolley = 1,
        EventRadiatingPoison,
        EventReconstitution
    };

    npc_highmaul_oro(Creature* creature) : MS::AI::CosmeticAI(creature)
    {
        m_Instance = creature->GetInstanceScript();
        m_Risen = false;
    }

    bool m_Risen;

    EventMap m_Events;
    InstanceScript* m_Instance;

    void Reset() override
    {
        if (!m_Risen)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }
        else
            me->SetAnimKitId(0);

        m_Events.Reset();

        RespawnGuardians(me, m_Instance);
    }

    void JustReachedHome() override
    {
        if (m_Risen)
            me->SetAnimKitId(0);
    }

    bool CanRespawn() override
    {
        return false;
    }

    void EnterCombat(Unit* attacker) override
    {
        StartGuardians(me, attacker);

        m_Events.RescheduleEvent(EventStoneboltVolley, 6 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventRadiatingPoison, 10 * IN_MILLISECONDS);
    }

    void DoAction(int32 const action) override
    {
        if (action == Rise && !m_Risen)
        {
            m_Risen = true;
            me->SetAnimKitId(0);
            me->PlayOneShotAnimKit(AnimRise);

            AddDelayedEvent(4500, [this]() -> void
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
            });
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType) override
    {
        if (m_Events.HasEvent(EventReconstitution) || me->HasAura(ReconstitutionScale))
            return;

        if (me->HealthBelowPctDamaged(21, damage))
            m_Events.RescheduleEvent(EventReconstitution, 100);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Creature* tectus = me->FindNearestCreature(Tectus, 100.0f))
            tectus->AI()->DoAction(GuardianDead);
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
            case EventStoneboltVolley:
                me->CastSpell(me, StoneboltVolley, false);
                m_Events.RescheduleEvent(EventStoneboltVolley, 12 * IN_MILLISECONDS);
                break;
            case EventRadiatingPoison:
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, RadiatingPoison, false);
                m_Events.RescheduleEvent(EventRadiatingPoison, 12 * IN_MILLISECONDS);
                break;
            case EventReconstitution:
                me->CastSpell(me, ReconstitutionScale, true);
                me->CastSpell(me, Reconstitution, false);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Earthen Pillar Stalker - 80476
struct npc_highmaul_earthen_pillar_stalker : public MS::AI::CosmeticAI
{
    npc_highmaul_earthen_pillar_stalker(Creature* creature) : MS::AI::CosmeticAI(creature) { }

    ObjectGuid m_PillarGuid;

    enum eSpells
    {
        EarthenPillarTimer = 166024,
        EarthenPillarKill = 162522
    };

    enum eGameObject
    {
        GoBEarthenPillar = 229649
    };

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

        me->AddUnitState(UNIT_STATE_ROOT);

        me->CastSpell(me, EarthenPillarTimer, true);

        if (GameObject* pillar = me->SummonGameObject(GoBEarthenPillar, *me, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
            m_PillarGuid = pillar->GetGUID();
    }

    void DoAction(int32 const action) override
    {
        if (action)
            return;

        me->CastSpell(me, EarthenPillarKill, true);

        AddDelayedEvent(1 * IN_MILLISECONDS, [this]() -> void
        {
            if (GameObject* pillar = GameObject::GetGameObject(*me, m_PillarGuid))
                pillar->Delete();

            if (GameObject* pillar = me->SummonGameObject(GoBEarthenPillar, *me, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
                m_PillarGuid = pillar->GetGUID();
        });

        AddDelayedEvent(60 * IN_MILLISECONDS, [this]() -> void
        {
            if (GameObject* pillar = GameObject::GetGameObject(*me, m_PillarGuid))
                pillar->Delete();

            me->DespawnOrUnsummon();
        });
    }
};

/// Night-Twisted Berserker - 80822
struct npc_highmaul_night_twisted_berserker : public ScriptedAI
{
    npc_highmaul_night_twisted_berserker(Creature* creature) : ScriptedAI(creature) { }

    enum eSpells
    {
        EncounterSpawn = 181113,
        RavingAssault = 163312
    };

    enum eEvent
    {
        EventRavingAssault = 1
    };

    enum eTalk
    {
        Aggro
    };

    EventMap m_Events;

    void Reset() override
    {
        me->CastSpell(me, EncounterSpawn, true);

        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        Talk(Aggro);

        m_Events.RescheduleEvent(EventRavingAssault, 8 * IN_MILLISECONDS);
    }

    void SpellHitTarget(Unit* victim, SpellInfo const* spellInfo) override
    {
        if (victim == nullptr || victim == me->ToUnit() || spellInfo->Id != RavingAssault)
            return;

        Position pos = *victim;
        me->GetFirstCollisionPosition(pos, me->GetDistance(pos), me->GetRelativeAngle(pos.GetPositionX(), pos.GetPositionY()));
        me->GetMotionMaster()->MoveCharge(pos, SPEED_CHARGE, RavingAssault);
    }

    void MovementInform(uint32 /*type*/, uint32 id) override
    {
        if (id != RavingAssault)
            return;

        me->RemoveAura(RavingAssault);
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
            case EventRavingAssault:
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                    me->CastSpell(target, RavingAssault, false);
                m_Events.RescheduleEvent(EventRavingAssault, 20 * IN_MILLISECONDS);
                break;
            }
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Night-Twisted Earthwarper - 80599
struct npc_highmaul_night_twisted_earthwarper : public ScriptedAI
{
    npc_highmaul_night_twisted_earthwarper(Creature* creature) : ScriptedAI(creature) { }

    enum eSpells
    {
        EncounterSpawn = 181113,
        GiftOfEarth = 162894,
        EarthenFlechettes = 162968
    };

    enum eEvents
    {
        EventEarthenFlechettes = 1,
        EventGiftOfEarth
    };

    enum eTalk
    {
        Aggro
    };

    EventMap m_Events;

    void Reset() override
    {
        me->CastSpell(me, EncounterSpawn, true);

        m_Events.Reset();
    }

    void EnterCombat(Unit* /*attacker*/) override
    {
        Talk(Aggro);

        m_Events.RescheduleEvent(EventEarthenFlechettes, 5 * IN_MILLISECONDS);
        m_Events.RescheduleEvent(EventGiftOfEarth, 12 * IN_MILLISECONDS);
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
            case EventEarthenFlechettes:
                me->CastSpell(me, EarthenFlechettes, false);
                m_Events.RescheduleEvent(EventEarthenFlechettes, 15 * IN_MILLISECONDS);
                break;
            case EventGiftOfEarth:
                me->CastSpell(me, GiftOfEarth, false);
                m_Events.RescheduleEvent(EventGiftOfEarth, 25 * IN_MILLISECONDS);
                break;
            default:
                break;
        }

        DoMeleeAttackIfReady();
    }
};

/// Meteoric Earthspire - 172073
class spell_highmaul_meteoric_earthspire : public SpellScript
{
    PrepareSpellScript(spell_highmaul_meteoric_earthspire);

    enum eSpell
    {
        RupturedEarth = 172078
    };

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (!targets.empty())
            return;

        if (Unit* caster = GetCaster())
            if (WorldLocation const* loc = GetExplTargetDest())
                caster->CastSpell(loc, RupturedEarth, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_meteoric_earthspire::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

/// Stonebolt Volley - 172058
class spell_highmaul_stonebolt_volley : public SpellScript
{
    PrepareSpellScript(spell_highmaul_stonebolt_volley);

    void CorrectTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        Unit* caster = GetCaster();
        if (caster == nullptr)
            return;

        targets.remove_if([this, caster](WorldObject* object) -> bool
        {
            if (object == nullptr)
                return true;

            if (object->GetDistance(caster) <= 5.0f)
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_stonebolt_volley::CorrectTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

/// The Living Mountain - 162287
/// Shard of the Mountain - 162658
/// Mote of the Mountain - 162674
class spell_highmaul_tectus_energy_gain : public AuraScript
{
    PrepareAuraScript(spell_highmaul_tectus_energy_gain);

    enum eActions
    {
        ScheduleEarthenPillar = 1,
        ScheduleTectonicUpheaval
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Creature* target = GetTarget()->ToCreature())
        {
            if (!target->isInCombat())
                return;

            uint32 oldPower = target->GetPower(POWER_ENERGY);
            int32 powerGain = GetEnergyGainFromHealth(target->GetHealthPct());
            target->EnergizeBySpell(target, GetSpellInfo()->Id, powerGain, POWER_ENERGY);
            uint32 newPower = oldPower + powerGain;

            if (target->IsAIEnabled)
            {
                if (newPower >= 100)
                    target->AI()->DoAction(ScheduleTectonicUpheaval);
                else
                {
                    /// On Mythic difficulty, Tectus also uses this ability at 50 Energy.
                    if (target->GetMap()->IsMythicRaid())
                    {
                        if ((oldPower < 25 && newPower >= 25) || (oldPower < 50 && newPower >= 50))
                            target->AI()->DoAction(ScheduleEarthenPillar);
                    }
                    else
                    {
                        if (oldPower < 25 && newPower >= 25)
                            target->AI()->DoAction(ScheduleEarthenPillar);
                    }
                }
            }
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_tectus_energy_gain::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

/// Earthen Pillar (timer) - 166024
class spell_highmaul_earthen_pillar_timer : public AuraScript
{
    PrepareAuraScript(spell_highmaul_earthen_pillar_timer);

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Creature* target = GetTarget()->ToCreature())
        {
            if (target->IsAIEnabled)
                target->AI()->DoAction(0);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_earthen_pillar_timer::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Accretion - 162288
class spell_highmaul_accretion : public AuraScript
{
    PrepareAuraScript(spell_highmaul_accretion);

    uint32 m_DamageTaken;

    bool Load() override
    {
        m_DamageTaken = 0;
        return true;
    }

    void OnProc(AuraEffect const* auraEffect, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        if (Unit* target = GetTarget())
        {
            m_DamageTaken += eventInfo.GetDamageInfo()->GetDamage();

            if (m_DamageTaken >= target->CountPctFromMaxHealth(2))
            {
                auraEffect->GetBase()->ModStackAmount(-1);
                m_DamageTaken = 0;
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_highmaul_accretion::OnProc, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    }
};

/// Tectonic Upheaval - 162475
class spell_highmaul_tectonic_upheaval : public AuraScript
{
    PrepareAuraScript(spell_highmaul_tectonic_upheaval);

    enum eSpell
    {
        Petrification = 163809
    };

    void OnTick(AuraEffect const* /*auraEffect*/)
    {
        if (Unit* target = GetTarget())
            target->EnergizeBySpell(target, GetSpellInfo()->Id, -10, POWER_ENERGY);
    }

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* target = GetTarget())
        {
            AuraRemoveMode removeMode = GetTargetApplication()->GetRemoveMode();
            if (removeMode != AURA_REMOVE_BY_CANCEL)
                target->CastSpell(target, Petrification, true);

            target->SetPower(POWER_ENERGY, 0);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_highmaul_tectonic_upheaval::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_tectonic_upheaval::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Highmaul Raid - Earth Fury - Spawn Dust Cloud - 169949
class spell_highmaul_spawn_dust_cloud : public AuraScript
{
    PrepareAuraScript(spell_highmaul_spawn_dust_cloud);

    void OnRemove(AuraEffect const* /*auraEffect*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTarget() == nullptr)
            return;

        if (Creature* target = GetTarget()->ToCreature())
        {
            target->SetReactState(REACT_AGGRESSIVE);
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_highmaul_spawn_dust_cloud::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/// Earthen Flechettes - 162968
class spell_highmaul_earthen_flechettes : public SpellScript
{
    PrepareSpellScript(spell_highmaul_earthen_flechettes);

    enum eSpell
    {
        TargetRestrict = 22531
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
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_highmaul_earthen_flechettes::CorrectTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
    }
};

/// Petrification - 162892
class spell_highmaul_petrification : public AuraScript
{
    PrepareAuraScript(spell_highmaul_petrification);

    enum eSpell
    {
        Petrification = 163809
    };

    void OnProc(AuraEffect const* /*auraEffect*/, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();

        if (Unit* target = GetTarget())
        {
            if (Unit* attacker = eventInfo.GetActor())
            {
                if (attacker->GetEntry() != Tectus)
                    return;

                attacker->CastSpell(attacker, Petrification, true);
            }
        }
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_highmaul_petrification::OnProc, EFFECT_0, SPELL_AURA_MOD_DECREASE_SPEED);
    }
};

/// Raving Assault - 163312
class spell_highmaul_raving_assault : public AuraScript
{
    PrepareAuraScript(spell_highmaul_raving_assault);

    enum eSpell
    {
        RavingAssaultDamage = 163318
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
                        caster->CastSpell(itr, RavingAssaultDamage, true);
                }

                m_DamageTimer = 500;
            }
            else
                m_DamageTimer -= diff;
        }
    }

    void Register() override
    {
        OnAuraUpdate += AuraUpdateFn(spell_highmaul_raving_assault::OnUpdate);
    }
};

void AddSC_boss_tectus()
{
    RegisterHighmaulCreatureAI(boss_tectus);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_supplicant);
    RegisterHighmaulCreatureAI(npc_highmaul_rokka_and_lokk);
    RegisterHighmaulCreatureAI(npc_highmaul_oro);
    RegisterHighmaulCreatureAI(npc_highmaul_earthen_pillar_stalker);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_berserker);
    RegisterHighmaulCreatureAI(npc_highmaul_night_twisted_earthwarper);

    RegisterSpellScript(spell_highmaul_meteoric_earthspire);
    RegisterSpellScript(spell_highmaul_stonebolt_volley);
    RegisterAuraScript(spell_highmaul_tectus_energy_gain);
    RegisterAuraScript(spell_highmaul_earthen_pillar_timer);
    RegisterAuraScript(spell_highmaul_accretion);
    RegisterAuraScript(spell_highmaul_tectonic_upheaval);
    RegisterAuraScript(spell_highmaul_spawn_dust_cloud);
    RegisterSpellScript(spell_highmaul_earthen_flechettes);
    RegisterAuraScript(spell_highmaul_petrification);
    //RegisterAuraScript(spell_highmaul_raving_assault);
}
